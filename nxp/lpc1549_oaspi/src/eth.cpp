#include <cstdio>
#include <cstring>
#include "usr.h"
#include "eth.h"

namespace eth {

/* private constants */
static constexpr size_t TX_REQ_SIZE = POOL_SIZE / 2; // can adjust

/* private data */
static struct {
    std::array<Packet, POOL_SIZE> pool_buf;
    QueueHandle_t pool;
} data;

/* private helpers */
struct EthHelper {

static void int_handler(void *arg) {
    Eth &dev = *reinterpret_cast<Eth*>(arg);
    BaseType_t woke = pdFALSE;
    vTaskNotifyGiveIndexedFromISR(dev.handle, configNOTIF_ETH, &woke);
    portYIELD_FROM_ISR(woke);
}

static void task(void *arg) {
    configASSERT(arg != nullptr);
    Eth &dev = *reinterpret_cast<Eth*>(arg);
    bool wait = false;
    while (true) {
        if (wait) {
            ulTaskNotifyTakeIndexed(configNOTIF_ETH, true, portMAX_DELAY);
            wait = false;
        }

        // setup tx chunk
        dev.tx.chunk.header.fill(0);
        if (dev.rx.chunk.TXC != 0) {
            if (dev.tx.len == 0) {
                if (xQueueReceive(dev.tx.reqs, &dev.tx.pkt, 0) == pdTRUE) {
                    dev.tx.start = true;
                    dev.tx.idx   = 0;
                    dev.tx.len   = Packet::HDR_LEN + dev.tx.pkt->_len + 4; // include CRC
                }
            }
            if (dev.tx.len) {
                dev.tx.chunk.DV  = 1;
                dev.tx.chunk.SV  = dev.tx.start;
                dev.tx.chunk.SWO = 0;
                if (dev.tx.len <= 64) {
                    dev.tx.chunk.EV  = 1;
                    dev.tx.chunk.EBO = dev.tx.len - 1;
                    std::memcpy(dev.tx.chunk.data.data(), &dev.tx.pkt->_buf[dev.tx.idx], dev.tx.len);
                    dev.tx.len = 0;
                } else {
                    std::memcpy(dev.tx.chunk.data.data(), &dev.tx.pkt->_buf[dev.tx.idx], 64);
                    dev.tx.idx += 64;
                    dev.tx.len -= 64;
                }
                if (dev.tx.len == 0) {
                    Eth::pkt_free(dev.tx.pkt);
                    dev.tx.pkt = nullptr;
                }
                dev.tx.start = false;
            }
        }
        dev.tx.chunk.DNC = 1;
        dev.tx.chunk.P   = OASPI::parity(dev.tx.chunk.header);

        // perform data transfer (one chunk only to minimize latency and memory overhead)
        dev.oaspi.data_transfer(dev.tx.chunk, dev.rx.chunk);
        if (dev.rx.chunk.HDRB || OASPI::parity(dev.rx.chunk.footer)) { // drop tx/rx packets on error
            if (dev.tx.len) {
                dev.tx.len = 0;
                Eth::pkt_free(dev.tx.pkt);
                dev.tx.pkt = nullptr;
            }
            dev.rx.len = 0;
            continue;
        }

        // process rx chunk
        if (dev.rx.chunk.SYNC == 0) {
            dev.oaspi.configure();
            dev.rx.chunk.TXC = 31;
            continue;
        } else if (dev.rx.chunk.EXST) {
            configASSERT(false); // all status masked, not needed yet
        }

        bool rx_copy = false;
        if (dev.rx.chunk.SV && dev.rx.chunk.DV) {
            rx_copy = true;
            dev.rx.len = 0;
        } else if (dev.rx.chunk.DV && dev.rx.len) {
            rx_copy = true;
        }
        if (rx_copy) {
            size_t len = dev.rx.chunk.EV ? dev.rx.chunk.EBO + 1 : 64;
            if ((dev.rx.len + len) <= dev.rx.pkt->_buf.size()) {
                std::memcpy(&dev.rx.pkt->_buf[dev.rx.len], dev.rx.chunk.data.data(), len);
                dev.rx.len += len;
            } else {
                dev.rx.len = 0;
            }
        }
        if (dev.rx.chunk.EV && dev.rx.len >= (Packet::HDR_LEN + 4)) {
            dev.rx.pkt->_len = dev.rx.len - Packet::HDR_LEN - 4;
            if (OASPI::fcs_check(*dev.rx.pkt)) {
                Packet *rx_new = Eth::pkt_alloc(false);
                if (rx_new) {
                    bool taken = false;
                    xSemaphoreTake(dev.callbacks.lock, portMAX_DELAY);
                    for (auto &[cb, arg]: dev.callbacks.cbs) {
                        if (cb == nullptr) {
                            continue;
                        }
                        if (cb(dev.rx.pkt, arg)) {
                            taken = true;
                            break;
                        }
                    }
                    xSemaphoreGive(dev.callbacks.lock);
                    if (!taken) {
                        Eth::pkt_free(dev.rx.pkt);
                    }
                    dev.rx.pkt = rx_new;
                } else {
                    dev.rx.drops++;
                }
            }
            dev.rx.len = 0;
        }

        // wait if both tx/rx want wait
        if ((uxQueueMessagesWaiting(dev.tx.reqs) == 0) && // no queued tx
            (dev.tx.len == 0 || dev.rx.chunk.TXC == 0) && // no current tx or tx buffer full
            (dev.rx.chunk.RCA == 0)) { // no rx
            wait = true;
        }
    }
}

}; // Eth::Helper

/* public functions */
Eth::Eth(OASPI &oaspi, int_set_callback cb) : oaspi(oaspi), int_set_cb(cb) {
    if (data.pool == nullptr) {
        data.pool = xQueueCreate(data.pool_buf.size(), sizeof(Packet*));
        configASSERT(data.pool);
        for (auto &pkt: data.pool_buf) {
            Packet *ptr = &pkt;
            configASSERT(xQueueSend(data.pool, &ptr, 0) == pdTRUE);
        }
    }

    callbacks.lock = xSemaphoreCreateMutex();
    tx.reqs        = xQueueCreate(TX_REQ_SIZE, sizeof(Packet*));
    rx.pkt         = pkt_alloc();
    configASSERT(callbacks.lock && tx.reqs && rx.pkt);

    int_set_cb(EthHelper::int_handler, this);
    configASSERT(xTaskCreate(EthHelper::task, "eth_task", configMINIMAL_STACK_SIZE, this, configMAX_PRIORITIES - 1, &handle) == pdPASS);
}

Eth::~Eth() {
    vTaskDelete(handle);
    int_set_cb(nullptr, nullptr);
    vQueueDelete(tx.reqs);
    vSemaphoreDelete(callbacks.lock);
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

void Eth::set_cb(size_t idx, rx_callback cb, void *arg) {
    configASSERT(idx < callbacks.cbs.size());
    xSemaphoreTake(callbacks.lock, portMAX_DELAY);
    callbacks.cbs[idx] = {cb, arg};
    xSemaphoreGive(callbacks.lock);
}

void Eth::send(Packet *pkt) {
    configASSERT(pkt != nullptr);
    configASSERT(pkt->_len <= Packet::MTU);
    if ((Packet::HDR_LEN + pkt->_len) < 60) { // short packet, need zero pad
        std::memset(&pkt->_buf[Packet::HDR_LEN + pkt->_len], 0, 60 - pkt->_len - Packet::HDR_LEN);
        pkt->_len = 60 - Packet::HDR_LEN;
    }
    OASPI::fcs_add(*pkt);
    if (xQueueSend(tx.reqs, &pkt, 0) == pdTRUE) {
        xTaskNotifyGiveIndexed(handle, configNOTIF_ETH);
    } else {
        pkt_free(pkt);
        tx.drops++;
    }
}

std::tuple<uint32_t, uint32_t> Eth::get_error(void) {
    // not important, no locks
    return {tx.drops, rx.drops};
}

};
