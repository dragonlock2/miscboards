#include <algorithm>
#include <cstdio>
#include <cstring>
#include "usr.h"
#include "eth.h"

namespace eth {

/* private data */
static struct {
    std::array<Packet, Eth::POOL_SIZE> pool_items;
    std::array<Packet*, Eth::POOL_SIZE> pool_buf;
    StaticQueue_t pool_data;
    QueueHandle_t pool;
} data_;

/* private helpers */
struct Eth::Helper {

static void int_handler(void *arg) {
    Eth &dev = *reinterpret_cast<Eth*>(arg);
    BaseType_t woke = pdFALSE;
    vTaskNotifyGiveIndexedFromISR(dev.task_.handle, configNOTIF_ETH, &woke);
    portYIELD_FROM_ISR(woke);
}

static void fill_tx_chunk(Eth &dev, OASPI::tx_chunk_t &chunk, bool add) {
    // >21MHz SPI cancels out worst case overhead of 65-byte packets, no need for SWO complexity.
    chunk.header.fill(0);
    if (add && (dev.tx_.pkt != nullptr) && (dev.tx_.free_chunks != 0)) {
        chunk.DV  = 1;
        chunk.SV  = dev.tx_.start;
        chunk.SWO = 0;
        if (dev.tx_.start) {
            chunk.TSC = dev.tx_.pkt->timestamp_id.value_or(0);
        }
        if (dev.tx_.len <= chunk.data.size()) {
            chunk.EV  = 1;
            chunk.EBO = dev.tx_.len - 1;
            std::memcpy(chunk.data.data(), &dev.tx_.pkt->buf_[dev.tx_.idx], dev.tx_.len);
            dev.tx_.len = 0;
        } else {
            std::memcpy(chunk.data.data(), &dev.tx_.pkt->buf_[dev.tx_.idx], chunk.data.size());
            dev.tx_.idx += chunk.data.size();
            dev.tx_.len -= chunk.data.size();
        }
        if (dev.tx_.len == 0) {
            dev.tx_.packets.fetch_add(1);
            dev.tx_.bytes.fetch_add(Packet::HDR_LEN + dev.tx_.pkt->len_);
            dev.pkt_free(dev.tx_.pkt);
            dev.tx_.pkt = nullptr;
            xSemaphoreTake(dev.callbacks_.lock, portMAX_DELAY);
            for (auto &[cb, arg]: dev.callbacks_.tx_cb) {
                if (cb != nullptr) {
                    cb(arg);
                }
            }
            xSemaphoreGive(dev.callbacks_.lock);
        }
        dev.tx_.start = false;
        dev.tx_.free_chunks--;
    }
    chunk.DNC = 1;
    chunk.P   = OASPI::parity(chunk.header);
}

static void add_rx_buffer(Eth &dev, std::span<uint8_t> buffer, bool start, bool end) {
    // set state
    if (start) {
        dev.rx_.len = 0;
    }

    // copy buffer
    if ((dev.rx_.len + buffer.size()) <= dev.rx_.pkt->buf_.size()) {
        std::memcpy(&dev.rx_.pkt->buf_[dev.rx_.len], buffer.data(), buffer.size());
        dev.rx_.len += buffer.size();
    } else {
        dev.rx_.len = 0; // too long, drop
        return;
    }

    // check timestamp if needed
    bool time64 = dev.oaspi_.ts_time64();
    if (dev.rx_.ts_expect && (dev.rx_.len >= (time64 ? 8 : 4))) {
        std::span<uint8_t, 4> hi(&dev.rx_.pkt->buf_[0], 4);
        std::span<uint8_t, 4> lo(&dev.rx_.pkt->buf_[4], 4);
        if (time64) {
            if ((OASPI::parity(hi) ^ OASPI::parity(lo)) != dev.rx_.ts_parity) {
                uint32_t secs  = (hi[0] << 24) | (hi[1] << 16) | (hi[2] << 8) | hi[3];
                uint32_t nsecs = (lo[0] << 24) | (lo[1] << 16) | (lo[2] << 8) | lo[3];
                dev.rx_.pkt->timestamp = { secs, nsecs & 0x3FFFFFFF };
            }
            dev.rx_.len -= 8;
            std::memmove(&dev.rx_.pkt->buf_[0], &dev.rx_.pkt->buf_[8], dev.rx_.len);
        } else {
            if (OASPI::parity(hi) == dev.rx_.ts_parity) {
                uint32_t raw = (hi[0] << 24) | (hi[1] << 16) | (hi[2] << 8) | hi[3];
                dev.rx_.pkt->timestamp = { (raw >> 30) & 0x03, raw & 0x3FFFFFFF };
            }
            dev.rx_.len -= 4;
            std::memmove(&dev.rx_.pkt->buf_[0], &dev.rx_.pkt->buf_[4], dev.rx_.len);
        }
        dev.rx_.ts_expect = false;
    }

    // callback if needed
    if (end) {
        if (dev.rx_.len >= (Packet::HDR_LEN + 4)) {
            dev.rx_.pkt->len_ = dev.rx_.len - Packet::HDR_LEN - 4;
            if (OASPI::fcs_check(*dev.rx_.pkt)) {
                Packet *rx_new = dev.pkt_alloc(false);
                if (rx_new) {
                    dev.rx_.packets.fetch_add(1);
                    dev.rx_.bytes.fetch_add(Packet::HDR_LEN + dev.rx_.pkt->len_);
                    bool taken = false;
                    xSemaphoreTake(dev.callbacks_.lock, portMAX_DELAY);
                    for (auto &[cb, arg]: dev.callbacks_.rx_cb) {
                        if ((cb != nullptr) && cb(dev.rx_.pkt, arg)) {
                            taken = true; // ownership taken, not available to others
                            break;
                        }
                    }
                    xSemaphoreGive(dev.callbacks_.lock);
                    if (!taken) {
                        dev.pkt_free(dev.rx_.pkt);
                    }
                    dev.rx_.pkt = rx_new;
                }
            }
        }
        dev.rx_.len = 0;
    }
}

static bool process_rx_chunk(Eth &dev, OASPI::rx_chunk_t &chunk) {
    // validate chunk
    auto handle_error = [&dev]() {
        // drop tx/rx packets on error
        dev.task_.error = true;
        if (dev.tx_.pkt != nullptr) {
            dev.pkt_free(dev.tx_.pkt);
            dev.tx_.pkt = nullptr;
            xSemaphoreTake(dev.callbacks_.lock, portMAX_DELAY);
            for (auto &[cb, arg]: dev.callbacks_.tx_cb) {
                if (cb != nullptr) {
                    cb(arg);
                }
            }
            xSemaphoreGive(dev.callbacks_.lock);
        }
        dev.rx_.len = 0;
    };
    auto handle_reset = [&dev]() {
        dev.task_.error = true;
        dev.oaspi_.reset();
        dev.tx_.free_chunks = 1; // assuming at least one chunk free after reset
        dev.rx_.len = 0;
    };
    if (OASPI::parity(chunk.footer)) {
        handle_error(); // likely random bit error
        return false;
    } else if (chunk.SYNC == 0) {
        handle_reset(); // likely reset
        return false;
    } else if (chunk.EXST) {
        handle_reset(); // assume any unmasked status bit means error
        return false;
    } else if (chunk.HDRB) {
        handle_error(); // likely random bit error, also set on reset
        return false;
    } else {
        dev.task_.error = false;
    }

    // process rx data
    bool start = chunk.SV;
    bool end = chunk.EV;
    size_t start_idx = 4 * chunk.SWO;
    size_t end_idx = chunk.EBO + 1;
    std::span<uint8_t> chunk_data(chunk.data);
    if (chunk.DV && (start_idx < chunk.data.size()) && (end_idx <= chunk.data.size())) {
        auto ts_reset = [&dev, &chunk]() {
            dev.rx_.pkt->timestamp = std::nullopt;
            dev.rx_.ts_expect = chunk.RTSA;
            dev.rx_.ts_parity = chunk.RTSP;
        };
        if (!start && !end) {
            add_rx_buffer(dev, chunk_data, false, false);
        } else if (start && !end) {
            ts_reset();
            add_rx_buffer(dev, chunk_data.subspan(start_idx), true, false);
        } else if (!start && end) {
            if (chunk.FD) {
                dev.rx_.len = 0;
            } else {
                add_rx_buffer(dev, chunk_data.subspan(0, end_idx), false, true);
            }
        } else { // start && end
            if (start_idx < end_idx) {
                if (chunk.FD) {
                    dev.rx_.len = 0;
                } else {
                    ts_reset();
                    add_rx_buffer(dev, chunk_data.subspan(start_idx, end_idx - start_idx), true, true);
                }
            } else { // start_idx >= end_idx
                if (chunk.FD) {
                    dev.rx_.len = 0;
                } else {
                    add_rx_buffer(dev, chunk_data.subspan(0, end_idx), false, true);
                }
                ts_reset();
                add_rx_buffer(dev, chunk_data.subspan(start_idx), true, false);
            }
        }
    }
    dev.tx_.free_chunks = chunk.TXC;
    dev.rx_.pend_chunks = chunk.RCA;
    return true;
}

static void task(void *arg) {
    configASSERT(arg != nullptr);
    Eth &dev = *reinterpret_cast<Eth*>(arg);
    bool wait = false;
    dev.oaspi_.reset();
    while (true) {
        if (wait) {
            // fixed wait time to quickly detect unintended resets
            ulTaskNotifyTakeIndexed(configNOTIF_ETH, true, pdMS_TO_TICKS(100));
            wait = false;
        }

        // pull next packet
        if (dev.tx_.pkt == nullptr) {
            if (xQueueReceive(dev.tx_.reqs, &dev.tx_.pkt, 0) == pdTRUE) {
                dev.tx_.start = true;
                dev.tx_.idx   = 0;
                dev.tx_.len   = Packet::HDR_LEN + dev.tx_.pkt->len_ + 4; // include CRC
            }
        }

        // compute number of chunks
        auto chunk_size = dev.tx_.chunks[0].data.size();
        size_t tx_prefer = (dev.tx_.pkt == nullptr) ? 0 : ((dev.tx_.len + chunk_size - 1) / chunk_size);
#ifdef CONFIG_ETH_MIN_LATENCY
        bool tx_add = dev.tx_.free_chunks >= tx_prefer;
        size_t tx_chunks = tx_add ? tx_prefer : 0; // only TX if enough chunks for entire packet
        size_t rx_chunks = 1; // to minimize TX latency can only read one chunk at a time
#else
        bool tx_add = true;
        size_t tx_chunks = std::min(dev.tx_.free_chunks, tx_prefer);
        size_t rx_chunks = dev.rx_.pend_chunks;
#endif
        size_t num_chunks = std::clamp<size_t>(std::max(tx_chunks, rx_chunks), 1, MAX_CHUNKS);

        // setup tx chunks
        for (size_t i = 0; i < num_chunks; i++) {
            fill_tx_chunk(dev, dev.tx_.chunks[i], tx_add);
        }

        // perform data transfer
        dev.oaspi_.data_transfer(std::span(dev.tx_.chunks).subspan(0, num_chunks), std::span(dev.rx_.chunks).subspan(0, num_chunks));

        // process rx chunks
        for (size_t i = 0; i < num_chunks; i++) {
            if (!process_rx_chunk(dev, dev.rx_.chunks[i])) {
                break;
            }
        }

        // wait if both tx/rx want wait
        if ((uxQueueMessagesWaiting(dev.tx_.reqs) == 0) && // no queued tx
            ((dev.tx_.pkt == nullptr) || (dev.tx_.free_chunks == 0)) && // no current tx or tx buffer full
            (dev.rx_.pend_chunks == 0)) { // no rx
            wait = true;
        }
    }
}

}; // Eth::Helper

/* public functions */
Eth::Eth(OASPI &oaspi, int_set_callback_t int_set) : oaspi_(oaspi), int_set_(int_set) {
    if (data_.pool == nullptr) {
        data_.pool = xQueueCreateStatic(data_.pool_buf.size(), sizeof(Packet*),
            reinterpret_cast<uint8_t*>(data_.pool_buf.data()), &data_.pool_data);
        configASSERT(data_.pool);
        for (auto &pkt: data_.pool_items) {
            Packet *ptr = &pkt;
            configASSERT(xQueueSend(data_.pool, &ptr, 0) == pdTRUE);
        }
    }

    callbacks_.lock = xSemaphoreCreateMutexStatic(&callbacks_.lock_buffer);
    tx_.reqs = xQueueCreateStatic(tx_.reqs_buf.size(), sizeof(Packet*),
        reinterpret_cast<uint8_t*>(tx_.reqs_buf.data()), &tx_.reqs_data);
    rx_.pkt = pkt_alloc();
    configASSERT(callbacks_.lock && tx_.reqs && rx_.pkt);

    task_.handle = xTaskCreateStatic(Helper::task, "eth_task",
        task_.stack.size(), this, configETH_PRIORITY, task_.stack.data(), &task_.buffer);
    configASSERT(task_.handle);

    int_set_(Helper::int_handler, this);
}

Eth::~Eth() {
    vTaskDelete(task_.handle);
    int_set_(nullptr, nullptr);
    vQueueDelete(tx_.reqs);
    vSemaphoreDelete(callbacks_.lock);
}

Packet *Eth::pkt_alloc(bool wait) {
    Packet *pkt = nullptr;
    xQueueReceive(data_.pool, &pkt, wait ? portMAX_DELAY : 0);
    if (pkt) {
        pkt->timestamp_id = std::nullopt;
        pkt->timestamp = std::nullopt;
        pkt->len_ = 0;
    }
    return pkt;
}

void Eth::pkt_free(Packet *pkt) {
    configASSERT(pkt != nullptr);
    configASSERT(xQueueSend(data_.pool, &pkt, 0) == pdTRUE); // should be immediate
}

size_t Eth::add_tx_cb(tx_callback_t cb, void *arg) {
    xSemaphoreTake(callbacks_.lock, portMAX_DELAY);
    bool found = false;
    size_t id;
    for (id = 0; id < callbacks_.tx_cb.size(); id++) {
        if (callbacks_.tx_cb[id] == std::make_tuple(nullptr, nullptr)) {
            callbacks_.tx_cb[id] = { cb, arg };
            found = true;
            break;
        }
    }
    xSemaphoreGive(callbacks_.lock);
    configASSERT(found);
    return id;
}

size_t Eth::add_rx_cb(rx_callback_t cb, void *arg) {
    xSemaphoreTake(callbacks_.lock, portMAX_DELAY);
    bool found = false;
    size_t id;
    for (id = 0; id < callbacks_.rx_cb.size(); id++) {
        if (callbacks_.rx_cb[id] == std::make_tuple(nullptr, nullptr)) {
            callbacks_.rx_cb[id] = { cb, arg };
            found = true;
            break;
        }
    }
    xSemaphoreGive(callbacks_.lock);
    configASSERT(found);
    return id;
}

void Eth::remove_tx_cb(size_t id) {
    configASSERT(id < callbacks_.tx_cb.size());
    xSemaphoreTake(callbacks_.lock, portMAX_DELAY);
    callbacks_.tx_cb[id] = { nullptr, nullptr };
    xSemaphoreGive(callbacks_.lock);
}

void Eth::remove_rx_cb(size_t id) {
    configASSERT(id < callbacks_.rx_cb.size());
    xSemaphoreTake(callbacks_.lock, portMAX_DELAY);
    callbacks_.rx_cb[id] = { nullptr, nullptr };
    xSemaphoreGive(callbacks_.lock);
}

bool Eth::send(Packet *pkt, bool wait) {
    configASSERT(pkt != nullptr);
    configASSERT(pkt->len_ <= Packet::MTU);
    if ((Packet::HDR_LEN + pkt->len_) < 60) { // short packet, need zero pad
        std::memset(&pkt->buf_[Packet::HDR_LEN + pkt->len_], 0, 60 - pkt->len_ - Packet::HDR_LEN);
        pkt->len_ = 60 - Packet::HDR_LEN;
    }
    OASPI::fcs_add(*pkt);
    if (xQueueSend(tx_.reqs, &pkt, wait ? portMAX_DELAY : 0) == pdTRUE) {
        xTaskNotifyGiveIndexed(task_.handle, configNOTIF_ETH);
        return true;
    } else {
        pkt_free(pkt);
        return false;
    }
}

std::tuple<uint32_t, uint32_t> Eth::get_packets() {
    return { tx_.packets.exchange(0), rx_.packets.exchange(0) };
}

std::tuple<uint32_t, uint32_t> Eth::get_bytes() {
    return { tx_.bytes.exchange(0), rx_.bytes.exchange(0) };
}

bool Eth::error() {
    return task_.error;
}

std::optional<Time> Eth::timestamp_read(uint32_t id) {
    return oaspi_.ts_read(static_cast<OASPI::TTSC>(id));
}

};
