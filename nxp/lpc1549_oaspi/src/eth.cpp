#include <array>
#include <cstdio>
#include <cstring>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <tusb.h>
#include <chip.h>
#include "oaspi.h"
#include "eth.h"

/* private data */
struct eth_tx_req {
    const uint8_t* pkt;
    size_t len;
};

static struct {
    SemaphoreHandle_t event;
    struct {
        QueueHandle_t reqs;
        bool start;
        std::array<uint8_t, OASPI_MAX_PKT_LEN> pkt;
        size_t idx, len;
        oaspi_tx_chunk chunk;
    } tx;
    struct {
        std::array<uint8_t, OASPI_MAX_PKT_LEN> pkt;
        size_t len;
        oaspi_rx_chunk chunk;
    } rx;
} data;

/* private helpers */
static void eth_int_handler(void) {
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
    BaseType_t woke = pdFALSE;
    xSemaphoreGiveFromISR(data.event, &woke);
    portYIELD_FROM_ISR(woke);
}

static void eth_task(void*) {
    bool wait = false;
    while (true) {
        if (wait) {
            xSemaphoreTake(data.event, portMAX_DELAY);
            wait = false;
        }

        // setup tx chunk
        data.tx.chunk.header.fill(0);
        if (data.rx.chunk.TXC != 0) {
            if (data.tx.len == 0) {
                eth_tx_req req;
                if (xQueueReceive(data.tx.reqs, &req, 0) == pdTRUE) {
                    if (req.len <= (data.tx.pkt.size() - 4)) {
                        std::memcpy(data.tx.pkt.data(), req.pkt, req.len);
                        data.tx.start = true;
                        data.tx.idx   = 0;
                        data.tx.len   = req.len;
                        if (data.tx.len < 60) { // short packet, need zero pad
                            std::memset(&data.tx.pkt[data.tx.len], 0, 60 - data.tx.len);
                            data.tx.len = 60;
                        }
                        oaspi_fcs_add(data.tx.pkt, data.tx.len);
                        data.tx.len += 4;
                    }
                    tud_network_recv_renew(); // free buffer
                }
            }
            if (data.tx.len) {
                data.tx.chunk.DV  = 1;
                data.tx.chunk.SV  = data.tx.start;
                data.tx.chunk.SWO = 0;
                if (data.tx.len <= 64) {
                    data.tx.chunk.EV  = 1;
                    data.tx.chunk.EBO = data.tx.len - 1;
                    std::memcpy(data.tx.chunk.data.data(), &data.tx.pkt[data.tx.idx], data.tx.len);
                    data.tx.len = 0;
                } else {
                    std::memcpy(data.tx.chunk.data.data(), &data.tx.pkt[data.tx.idx], 64);
                    data.tx.idx += 64;
                    data.tx.len -= 64;
                }
                data.tx.start = false;
            }
        }
        data.tx.chunk.DNC = 1;
        data.tx.chunk.P   = oaspi_parity(data.tx.chunk.header);

        // perform data transfer
        // TODO consider multiple chunks to reduce startup and context switch penalties
        oaspi_data_transfer(data.tx.chunk, data.rx.chunk);
        if (data.rx.chunk.HDRB || oaspi_parity(data.rx.chunk.footer)) { // drop tx/rx packets on error
            data.tx.len = 0;
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
            if ((data.rx.len + len) < data.rx.pkt.size()) {
                std::memcpy(&data.rx.pkt[data.rx.len], data.rx.chunk.data.data(), len);
                data.rx.len += len;
            } else {
                data.rx.len = 0;
            }
        }
        if (data.rx.chunk.EV && data.rx.len >= 4 && oaspi_fcs_check(data.rx.pkt, data.rx.len)) {
            tud_network_xmit(data.rx.pkt.data(), data.rx.len - 4);
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
    data.event   = xSemaphoreCreateBinary();
    data.tx.reqs = xQueueCreate(2, sizeof(eth_tx_req));
    configASSERT(data.event && data.tx.reqs);

    Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 12);
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 12, IOCON_MODE_PULLUP);

    Chip_PININT_Init(LPC_GPIO_PIN_INT);
    Chip_INMUX_PinIntSel(0, 0, 12);
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(0));
    Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(0));

    NVIC_SetVector(PIN_INT0_IRQn, reinterpret_cast<uint32_t>(eth_int_handler));
    NVIC_SetPriority(PIN_INT0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS));
    NVIC_EnableIRQ(PIN_INT0_IRQn);

    xTaskCreate(eth_task, "eth_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
}

void tud_network_init_cb(void) {}

bool tud_network_recv_cb(const uint8_t* src, uint16_t size) {
    eth_tx_req req = {
        .pkt = src,
        .len = size,
    };
    if (size && xQueueSend(data.tx.reqs, &req, 0) == pdTRUE) {
        xSemaphoreGive(data.event);
    }
    return false;
}

uint16_t tud_network_xmit_cb(uint8_t* dst, void* ref, uint16_t arg) {
    uint8_t* src = static_cast<uint8_t*>(ref);
    uint16_t len = arg;
    std::memcpy(dst, src, len);
    return len;
}
