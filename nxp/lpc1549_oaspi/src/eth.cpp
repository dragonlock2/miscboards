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
} data;

/* private helpers */
struct Eth::Helper {

static void int_handler(void *arg) {
    Eth &dev = *reinterpret_cast<Eth*>(arg);
    BaseType_t woke = pdFALSE;
    vTaskNotifyGiveIndexedFromISR(dev._task.handle, configNOTIF_ETH, &woke);
    portYIELD_FROM_ISR(woke);
}

static void fill_tx_chunk(Eth &dev, OASPI::tx_chunk &chunk, bool add) {
    // >21MHz SPI cancels out worst case overhead of 65-byte packets, no need for SWO complexity.
    chunk.header.fill(0);
    if (add && (dev._tx.pkt != nullptr) && (dev._tx.free_chunks != 0)) {
        chunk.DV  = 1;
        chunk.SV  = dev._tx.start;
        chunk.SWO = 0;
        if (dev._tx.len <= chunk.data.size()) {
            chunk.EV  = 1;
            chunk.EBO = dev._tx.len - 1;
            std::memcpy(chunk.data.data(), &dev._tx.pkt->_buf[dev._tx.idx], dev._tx.len);
            dev._tx.len = 0;
        } else {
            std::memcpy(chunk.data.data(), &dev._tx.pkt->_buf[dev._tx.idx], chunk.data.size());
            dev._tx.idx += chunk.data.size();
            dev._tx.len -= chunk.data.size();
        }
        if (dev._tx.len == 0) {
            dev.pkt_free(dev._tx.pkt);
            dev._tx.pkt = nullptr;
            volatile auto tx_cb = dev._callbacks.tx_cb;
            if (tx_cb) {
                tx_cb();
            }
            dev._tx.total++;
        }
        dev._tx.start = false;
        dev._tx.free_chunks--;
    }
    chunk.DNC = 1;
    chunk.P   = OASPI::parity(chunk.header);
}

static void add_rx_buffer(Eth &dev, std::span<uint8_t> buffer, bool start, bool end) {
    // set state
    if (start) {
        dev._rx.len = 0;
    } else if (dev._rx.len == 0) {
        return; // unexpected non-start chunk
    }

    // copy buffer
    if ((dev._rx.len + buffer.size()) <= dev._rx.pkt->_buf.size()) {
        std::memcpy(&dev._rx.pkt->_buf[dev._rx.len], buffer.data(), buffer.size());
        dev._rx.len += buffer.size();
    } else {
        dev._rx.len = 0; // too long, drop
        return;
    }

    // callback if needed
    if (end) {
        if (dev._rx.len >= (Packet::HDR_LEN + 4)) {
            dev._rx.pkt->_len = dev._rx.len - Packet::HDR_LEN - 4;
            if (OASPI::fcs_check(*dev._rx.pkt)) {
                Packet *rx_new = dev.pkt_alloc(false);
                if (rx_new) {
                    bool taken = false;
                    xSemaphoreTake(dev._callbacks.lock, portMAX_DELAY);
                    auto &[cb, arg] = dev._callbacks.rx_cb;
                    if (cb && cb(dev._rx.pkt, arg)) {
                        taken = true;
                    }
                    xSemaphoreGive(dev._callbacks.lock);
                    if (!taken) {
                        dev.pkt_free(dev._rx.pkt);
                    }
                    dev._rx.pkt = rx_new;
                    dev._rx.total++;
                } else {
                    dev._rx.drops++;
                }
            }
        }
        dev._rx.len = 0;
    }
}

static bool process_rx_chunk(Eth &dev, OASPI::rx_chunk &chunk) {
    // validate chunk
    auto handle_error = [&dev]() {
        // drop tx/rx packets on error
        dev._task.error = true;
        if (dev._tx.pkt != nullptr) {
            dev.pkt_free(dev._tx.pkt);
            dev._tx.pkt = nullptr;
            volatile auto tx_cb = dev._callbacks.tx_cb;
            if (tx_cb) {
                tx_cb();
            }
            dev._tx.drops++;
        }
        dev._rx.len = 0;
    };
    auto handle_reset = [&dev]() {
        dev._task.error = true;
        dev._oaspi.reset();
        dev._tx.free_chunks = 1; // assuming at least one chunk free after reset
        dev._rx.len = 0;
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
        dev._task.error = false;
    }

    // process rx data
    bool start = chunk.SV;
    bool end = chunk.EV;
    size_t start_idx = 4 * chunk.SWO;
    size_t end_idx = chunk.EBO + 1;
    std::span<uint8_t> chunk_data(chunk.data);
    if (chunk.DV && (start_idx < chunk.data.size()) && (end_idx <= chunk.data.size())) {
        if (!start && !end) {
            add_rx_buffer(dev, chunk_data, false, false);
        } else if (start && !end) {
            add_rx_buffer(dev, chunk_data.subspan(start_idx), true, false);
        } else if (!start && end) {
            if (chunk.FD) {
                dev._rx.len = 0;
            } else {
                add_rx_buffer(dev, chunk_data.subspan(0, end_idx), false, true);
            }
        } else { // start && end
            if (start_idx < end_idx) {
                if (chunk.FD) {
                    dev._rx.len = 0;
                } else {
                    add_rx_buffer(dev, chunk_data.subspan(start_idx, end_idx - start_idx), true, true);
                }
            } else { // start_idx >= end_idx
                if (chunk.FD) {
                    dev._rx.len = 0;
                } else {
                    add_rx_buffer(dev, chunk_data.subspan(0, end_idx), false, true);
                }
                add_rx_buffer(dev, chunk_data.subspan(start_idx), true, false);
            }
        }
    }
    dev._tx.free_chunks = chunk.TXC;
    dev._rx.pend_chunks = chunk.RCA;
    return true;
}

static void task(void *arg) {
    configASSERT(arg != nullptr);
    Eth &dev = *reinterpret_cast<Eth*>(arg);
    bool wait = false;
    dev._oaspi.reset();
    while (true) {
        if (wait) {
            // fixed wait time to quickly detect unintended resets
            ulTaskNotifyTakeIndexed(configNOTIF_ETH, true, pdMS_TO_TICKS(100));
            wait = false;
        }

        // pull next packet
        if (dev._tx.pkt == nullptr) {
            if (xQueueReceive(dev._tx.reqs, &dev._tx.pkt, 0) == pdTRUE) {
                dev._tx.start = true;
                dev._tx.idx   = 0;
                dev._tx.len   = Packet::HDR_LEN + dev._tx.pkt->_len + 4; // include CRC
            }
        }

        // compute number of chunks
        auto chunk_size = dev._tx.chunks[0].data.size();
        size_t tx_prefer = (dev._tx.pkt == nullptr) ? 0 : ((dev._tx.len + chunk_size - 1) / chunk_size);
#ifdef CONFIG_ETH_MIN_LATENCY
        bool tx_add = dev._tx.free_chunks >= tx_prefer;
        size_t tx_chunks = tx_add ? tx_prefer : 0; // only TX if enough chunks for entire packet
        size_t rx_chunks = 1; // to minimize TX latency can only read one chunk at a time
#else
        bool tx_add = true;
        size_t tx_chunks = std::min(dev._tx.free_chunks, tx_prefer);
        size_t rx_chunks = dev._rx.pend_chunks;
#endif
        size_t num_chunks = std::clamp<size_t>(std::max(tx_chunks, rx_chunks), 1, MAX_CHUNKS);

        // setup tx chunks
        for (size_t i = 0; i < num_chunks; i++) {
            fill_tx_chunk(dev, dev._tx.chunks[i], tx_add);
        }

        // perform data transfer
        dev._oaspi.data_transfer(std::span(dev._tx.chunks).subspan(0, num_chunks), std::span(dev._rx.chunks).subspan(0, num_chunks));

        // process rx chunks
        for (size_t i = 0; i < num_chunks; i++) {
            if (!process_rx_chunk(dev, dev._rx.chunks[i])) {
                break;
            }
        }

        // wait if both tx/rx want wait
        if ((uxQueueMessagesWaiting(dev._tx.reqs) == 0) && // no queued tx
            ((dev._tx.pkt == nullptr) || (dev._tx.free_chunks == 0)) && // no current tx or tx buffer full
            (dev._rx.pend_chunks == 0)) { // no rx
            wait = true;
        }
    }
}

}; // Eth::Helper

/* public functions */
Eth::Eth(OASPI &oaspi, int_set_callback int_set) : _oaspi(oaspi), _int_set(int_set) {
    if (data.pool == nullptr) {
        data.pool = xQueueCreateStatic(data.pool_buf.size(), sizeof(Packet*),
            reinterpret_cast<uint8_t*>(data.pool_buf.data()), &data.pool_data);
        configASSERT(data.pool);
        for (auto &pkt: data.pool_items) {
            Packet *ptr = &pkt;
            configASSERT(xQueueSend(data.pool, &ptr, 0) == pdTRUE);
        }
    }

    _callbacks.lock = xSemaphoreCreateMutexStatic(&_callbacks.lock_buffer);
    _tx.reqs = xQueueCreateStatic(_tx.reqs_buf.size(), sizeof(Packet*),
        reinterpret_cast<uint8_t*>(_tx.reqs_buf.data()), &_tx.reqs_data);
    _rx.pkt = pkt_alloc();
    configASSERT(_callbacks.lock && _tx.reqs && _rx.pkt);

    _task.handle = xTaskCreateStatic(Helper::task, "eth_task",
        _task.stack.size(), this, configMAX_PRIORITIES - 2, _task.stack.data(), &_task.buffer);
    configASSERT(_task.handle);

    _int_set(Helper::int_handler, this);
}

Eth::~Eth() {
    vTaskDelete(_task.handle);
    _int_set(nullptr, nullptr);
    vQueueDelete(_tx.reqs);
    vSemaphoreDelete(_callbacks.lock);
}

Packet *Eth::pkt_alloc(bool wait) {
    Packet *pkt = nullptr;
    xQueueReceive(data.pool, &pkt, wait ? portMAX_DELAY : 0);
    if (pkt) {
        pkt->_len = 0;
    }
    return pkt;
}

void Eth::pkt_free(Packet *pkt) {
    configASSERT(pkt != nullptr);
    configASSERT(xQueueSend(data.pool, &pkt, 0) == pdTRUE); // should be immediate
}

void Eth::set_tx_cb(tx_callback cb) {
    xSemaphoreTake(_callbacks.lock, portMAX_DELAY);
    _callbacks.tx_cb = cb;
    xSemaphoreGive(_callbacks.lock);
}

void Eth::set_rx_cb(rx_callback cb, void *arg) {
    xSemaphoreTake(_callbacks.lock, portMAX_DELAY);
    _callbacks.rx_cb = {cb, arg};
    xSemaphoreGive(_callbacks.lock);
}

bool Eth::send(Packet *pkt, bool wait) {
    configASSERT(pkt != nullptr);
    configASSERT(pkt->_len <= Packet::MTU);
    if ((Packet::HDR_LEN + pkt->_len) < 60) { // short packet, need zero pad
        std::memset(&pkt->_buf[Packet::HDR_LEN + pkt->_len], 0, 60 - pkt->_len - Packet::HDR_LEN);
        pkt->_len = 60 - Packet::HDR_LEN;
    }
    OASPI::fcs_add(*pkt);
    if (xQueueSend(_tx.reqs, &pkt, wait ? portMAX_DELAY : 0) == pdTRUE) {
        xTaskNotifyGiveIndexed(_task.handle, configNOTIF_ETH);
        return true;
    } else {
        pkt_free(pkt);
        return false;
    }
}

std::tuple<uint32_t, uint32_t> Eth::get_drops() {
    // not important, no locks
    return {_tx.drops, _rx.drops};
}

std::tuple<uint32_t, uint32_t> Eth::get_total() {
    // not important, no locks
    return {_tx.total, _rx.total};
}

bool Eth::error() {
    return _task.error;
}

};
