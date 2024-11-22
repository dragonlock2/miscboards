#include <array>
#include <cstdio>
#include <cstring>
#include <tuple>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <tusb.h>
#include <chip.h>
#include "oaspi.h"
#include "eth.h"

/* private data */
static struct {
    TaskHandle_t handle;
    struct {
        std::array<eth_pkt, ETH_POOL_SIZE> buf;
        QueueHandle_t pool;
    } pool;
    struct {
        SemaphoreHandle_t lock;
        std::array<std::tuple<eth_callback, void*>, 2> cbs;
    } callbacks;
    struct {
        QueueHandle_t reqs;
        bool start;
        eth_pkt *pkt;
        size_t idx, len;
        oaspi_tx_chunk chunk;
        uint32_t drops;
    } tx;
    struct {
        eth_pkt *pkt;
        size_t len;
        oaspi_rx_chunk chunk;
        uint32_t drops;
    } rx;
} data;

/* private helpers */
static void eth_int_handler(void) {
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
    BaseType_t woke = pdFALSE;
    xTaskNotifyIndexedFromISR(data.handle, 0, 1 << configNOTIF_ETH, eSetBits, &woke);
    portYIELD_FROM_ISR(woke);
}

static void eth_task(void*) {
    bool wait = false;
    while (true) {
        if (wait) {
            xTaskNotifyWaitIndexed(0, 0, 1 << configNOTIF_ETH, NULL, portMAX_DELAY);
            wait = false;
        }

        // setup tx chunk
        data.tx.chunk.header.fill(0);
        if (data.rx.chunk.TXC != 0) {
            if (data.tx.len == 0) {
                if (xQueueReceive(data.tx.reqs, &data.tx.pkt, 0) == pdTRUE) {
                    data.tx.start = true;
                    data.tx.idx   = 0;
                    data.tx.len   = ETH_HDR_LEN + data.tx.pkt->len + 4; // include CRC
                }
            }
            if (data.tx.len) {
                data.tx.chunk.DV  = 1;
                data.tx.chunk.SV  = data.tx.start;
                data.tx.chunk.SWO = 0;
                if (data.tx.len <= 64) {
                    data.tx.chunk.EV  = 1;
                    data.tx.chunk.EBO = data.tx.len - 1;
                    std::memcpy(data.tx.chunk.data.data(), &data.tx.pkt->buf[data.tx.idx], data.tx.len);
                    data.tx.len = 0;
                } else {
                    std::memcpy(data.tx.chunk.data.data(), &data.tx.pkt->buf[data.tx.idx], 64);
                    data.tx.idx += 64;
                    data.tx.len -= 64;
                }
                if (data.tx.len == 0) {
                    eth_pkt_free(data.tx.pkt);
                    data.tx.pkt = NULL;
                }
                data.tx.start = false;
            }
        }
        data.tx.chunk.DNC = 1;
        data.tx.chunk.P   = oaspi_parity(data.tx.chunk.header);

        // perform data transfer (one chunk only to minimize latency and memory overhead)
        oaspi_data_transfer(data.tx.chunk, data.rx.chunk);
        if (data.rx.chunk.HDRB || oaspi_parity(data.rx.chunk.footer)) { // drop tx/rx packets on error
            if (data.tx.len) {
                data.tx.len = 0;
                eth_pkt_free(data.tx.pkt);
                data.tx.pkt = NULL;
            }
            data.rx.len = 0;
            continue;
        }

        // process rx chunk
        if (data.rx.chunk.SYNC == 0) {
            oaspi_configure();
            data.rx.chunk.TXC = 31;
            continue;
        } else if (data.rx.chunk.EXST) {
            configASSERT(false); // all status masked, not needed yet
        }

        bool rx_copy = false;
        if (data.rx.chunk.SV && data.rx.chunk.DV) {
            rx_copy = true;
            data.rx.len = 0;
        } else if (data.rx.chunk.DV && data.rx.len) {
            rx_copy = true;
        }
        if (rx_copy) {
            size_t len = data.rx.chunk.EV ? data.rx.chunk.EBO + 1 : 64;
            if ((data.rx.len + len) <= data.rx.pkt->buf.size()) {
                std::memcpy(&data.rx.pkt->buf[data.rx.len], data.rx.chunk.data.data(), len);
                data.rx.len += len;
            } else {
                data.rx.len = 0;
            }
        }
        if (data.rx.chunk.EV && data.rx.len >= (ETH_HDR_LEN + 4)) {
            data.rx.pkt->len = data.rx.len - ETH_HDR_LEN - 4;
            if (oaspi_fcs_check(*data.rx.pkt)) {
                eth_pkt *rx_new = eth_pkt_alloc(false);
                if (rx_new) {
                    bool taken = false;
                    xSemaphoreTake(data.callbacks.lock, portMAX_DELAY);
                    for (auto &[cb, arg]: data.callbacks.cbs) {
                        if (cb == NULL) {
                            continue;
                        }
                        if (cb(data.rx.pkt, arg)) {
                            taken = true;
                            break;
                        }
                    }
                    xSemaphoreGive(data.callbacks.lock);
                    if (!taken) {
                        eth_pkt_free(data.rx.pkt);
                    }
                    data.rx.pkt = rx_new;
                } else {
                    data.rx.drops++;
                }
            }
            data.rx.len = 0;
        }

        // wait if both tx/rx want wait
        if ((uxQueueMessagesWaiting(data.tx.reqs) == 0) && // no queued tx
            (data.tx.len == 0 || data.rx.chunk.TXC == 0) && // no current tx or tx buffer full
            (data.rx.chunk.RCA == 0)) { // no rx
            wait = true;
        }
    }
}

/* public functions */
void eth_init(void) {
    data.pool.pool      = xQueueCreate(data.pool.buf.size(), sizeof(eth_pkt*));
    data.callbacks.lock = xSemaphoreCreateMutex();
    data.tx.reqs        = xQueueCreate(data.pool.buf.size() / 2, sizeof(eth_pkt*));
    configASSERT(data.pool.pool && data.callbacks.lock && data.tx.reqs);
    for (auto &pkt: data.pool.buf) {
        eth_pkt *ptr = &pkt;
        configASSERT(xQueueSend(data.pool.pool, &ptr, 0) == pdTRUE);
    }
    data.rx.pkt = eth_pkt_alloc();

#ifdef CONFIG_ADIN1110
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 12);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 12, IOCON_MODE_PULLUP);
    Chip_INMUX_PinIntSel(0, 0, 12);
#elifdef CONFIG_NCN26010
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 18);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, IOCON_MODE_PULLUP);
    Chip_INMUX_PinIntSel(0, 0, 18);
#endif

    Chip_PININT_Init(LPC_GPIO_PIN_INT);
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(0));
    Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(0));

    NVIC_SetVector(PIN_INT0_IRQn, reinterpret_cast<uint32_t>(eth_int_handler));
    NVIC_SetPriority(PIN_INT0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    NVIC_EnableIRQ(PIN_INT0_IRQn);

    configASSERT(xTaskCreate(eth_task, "eth_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &data.handle) == pdPASS);
}

eth_pkt *eth_pkt_alloc(bool wait) {
    eth_pkt *pkt = NULL;
    xQueueReceive(data.pool.pool, &pkt, wait ? portMAX_DELAY : 0);
    if (pkt) {
        pkt->len = 0;
    }
    return pkt;
}

void eth_pkt_free(eth_pkt *pkt) {
    configASSERT(pkt != NULL);
    configASSERT(xQueueSend(data.pool.pool, &pkt, 0) == pdTRUE); // should be immediate
}

void eth_set_cb(size_t idx, eth_callback cb, void *arg) {
    configASSERT(idx < data.callbacks.cbs.size());
    xSemaphoreTake(data.callbacks.lock, portMAX_DELAY);
    data.callbacks.cbs[idx] = {cb, arg};
    xSemaphoreGive(data.callbacks.lock);
}

void eth_send(eth_pkt *pkt) {
    configASSERT(pkt != NULL);
    configASSERT(pkt->len <= ETH_MTU);
    if ((ETH_HDR_LEN + pkt->len) < 60) { // short packet, need zero pad
        std::memset(&pkt->buf[ETH_HDR_LEN + pkt->len], 0, 60 - pkt->len - ETH_HDR_LEN);
        pkt->len = 60 - ETH_HDR_LEN;
    }
    oaspi_fcs_add(*pkt);
    if (xQueueSend(data.tx.reqs, &pkt, 0) == pdTRUE) {
        xTaskNotifyIndexed(data.handle, 0, 1 << configNOTIF_ETH, eSetBits);
    } else {
        eth_pkt_free(pkt);
        data.tx.drops++;
    }
}

void eth_get_error(uint32_t &tx_drop, uint32_t &rx_drop) {
    // not important, no locks
    tx_drop = data.tx.drops;
    rx_drop = data.rx.drops;
}
