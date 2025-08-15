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

        // setup tx chunk
        dev._tx.chunk.header.fill(0);
        if (dev._rx.chunk.TXC != 0) {
            if (dev._tx.len == 0) {
                if (xQueueReceive(dev._tx.reqs, &dev._tx.pkt, 0) == pdTRUE) {
                    dev._tx.start = true;
                    dev._tx.idx   = 0;
                    dev._tx.len   = Packet::HDR_LEN + dev._tx.pkt->_len + 4; // include CRC
                }
            }
            if (dev._tx.len) {
                dev._tx.chunk.DV  = 1;
                dev._tx.chunk.SV  = dev._tx.start;
                dev._tx.chunk.SWO = 0;
                if (dev._tx.len <= 64) {
                    dev._tx.chunk.EV  = 1;
                    dev._tx.chunk.EBO = dev._tx.len - 1;
                    std::memcpy(dev._tx.chunk.data.data(), &dev._tx.pkt->_buf[dev._tx.idx], dev._tx.len);
                    dev._tx.len = 0;
                } else {
                    std::memcpy(dev._tx.chunk.data.data(), &dev._tx.pkt->_buf[dev._tx.idx], 64);
                    dev._tx.idx += 64;
                    dev._tx.len -= 64;
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
            }
        }
        dev._tx.chunk.DNC = 1;
        dev._tx.chunk.P   = OASPI::parity(dev._tx.chunk.header);

        // perform data transfer (one chunk only to minimize latency and memory overhead)
        dev._oaspi.data_transfer(dev._tx.chunk, dev._rx.chunk);
        
        // process rx chunk
        auto handle_error = [&dev]() {
            // drop tx/rx packets on error
            dev._task.error = true;
            if (dev._tx.len) {
                dev._tx.len = 0;
                dev.pkt_free(dev._tx.pkt);
                dev._tx.pkt = nullptr;
                dev._tx.drops++;
                volatile auto tx_cb = dev._callbacks.tx_cb;
                if (tx_cb) {
                    tx_cb();
                }
            }
            dev._rx.len = 0;
        };
        auto handle_reset = [&dev]() {
            dev._task.error = true;
            dev._oaspi.reset();
            dev._rx.chunk.TXC = 31;
        };
        if (OASPI::parity(dev._rx.chunk.footer)) {
            handle_error(); // likely random bit error
            continue;
        } else if (dev._rx.chunk.SYNC == 0) {
            handle_reset(); // likely reset
            continue;
        } else if (dev._rx.chunk.EXST) {
            handle_reset(); // all status masked, shouldn't happen
            continue;
        } else if (dev._rx.chunk.HDRB) {
            handle_error(); // likely random bit error, also set on reset
            continue;
        } else {
            dev._task.error = false;
        }

        bool rx_copy = false;
        if (dev._rx.chunk.SV && dev._rx.chunk.DV) {
            rx_copy = true;
            dev._rx.len = 0;
        } else if (dev._rx.chunk.DV && dev._rx.len) {
            rx_copy = true;
        }
        if (rx_copy) {
            size_t len = dev._rx.chunk.EV ? dev._rx.chunk.EBO + 1 : 64;
            if ((dev._rx.len + len) <= dev._rx.pkt->_buf.size()) {
                std::memcpy(&dev._rx.pkt->_buf[dev._rx.len], dev._rx.chunk.data.data(), len);
                dev._rx.len += len;
            } else {
                dev._rx.len = 0;
            }
        }
        if (dev._rx.chunk.EV && dev._rx.len >= (Packet::HDR_LEN + 4)) {
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
                } else {
                    dev._rx.drops++;
                }
                dev._rx.total++;
            }
            dev._rx.len = 0;
        }

        // wait if both tx/rx want wait
        if ((uxQueueMessagesWaiting(dev._tx.reqs) == 0) && // no queued tx
            (dev._tx.len == 0 || dev._rx.chunk.TXC == 0) && // no current tx or tx buffer full
            (dev._rx.chunk.RCA == 0)) { // no rx
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

    _int_set(Helper::int_handler, this);
    _task.handle = xTaskCreateStatic(Helper::task, "eth_task",
        _task.stack.size(), this, configMAX_PRIORITIES - 1, _task.stack.data(), &_task.buffer);
    configASSERT(_task.handle);
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
