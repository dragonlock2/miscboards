#include <string.h>
#include "mcc_generated_files/system/system.h"
#include "rf24.h"

/* private defines */
#define RF24_CONFIG     0x00
#define RF24_EN_RXADDR  0x02
#define RF24_SETUP_RETR 0x04
#define RF24_CH         0x05
#define RF24_SETUP      0x06
#define RF24_STATUS     0x07
#define RF24_RX_ADDR_P0 0x0A
#define RF24_RX_ADDR_P1 0x0B
#define RF24_RX_ADDR_P2 0x0C
#define RF24_RX_ADDR_P3 0x0D
#define RF24_RX_ADDR_P4 0x0E
#define RF24_RX_ADDR_P5 0x0F
#define RF24_TX_ADDR    0x10
#define RF24_RX_PW_P0   0x11
#define RF24_RX_PW_P1   0x12
#define RF24_RX_PW_P2   0x13
#define RF24_RX_PW_P3   0x14
#define RF24_RX_PW_P4   0x15
#define RF24_RX_PW_P5   0x16
#define RF24_DYNPD      0x1C
#define RF24_FEATURE    0x1D

#define RF24_R_RX_PAYLOAD 0x61
#define RF24_W_TX_PAYLOAD 0xA0
#define RF24_R_RX_PL_WID  0x60

typedef struct {
    uint8_t address[5];
    bool address_update;
} rf24_data_S;
static rf24_data_S rf24_data;

/* private helpers */
static void rf24_transfer_done_cb() {
    CSN_SetHigh();
    SPI0_RegisterTransferDoneCallback(NULL);
}

static inline uint8_t rf24_read(uint8_t cmd, uint8_t *data, uint8_t data_len) {
    CSN_SetLow();
    uint8_t status = SPI0_ByteExchange(cmd);
    if (data_len) {
        SPI0_BufferRead(data, data_len);
        while (SPI0_StatusBusy());
    }
    CSN_SetHigh();
    return status;
}

static inline uint8_t rf24_write(uint8_t cmd, uint8_t *data, uint8_t data_len) {
    CSN_SetLow();
    uint8_t status = SPI0_ByteExchange(cmd);
    if (data_len) {
        SPI0_BufferWrite(data, data_len);
        while (SPI0_StatusBusy());
    }
    CSN_SetHigh();
    return status;
}

static inline uint8_t rf24_read_reg_one(uint8_t reg, uint8_t *data) {
    return rf24_read(0x00 | reg, data, 1);
}

static inline uint8_t rf24_write_reg_one(uint8_t reg, uint8_t data) {
    return rf24_write(0x20 | reg, &data, 1);
}

static inline uint8_t rf24_read_reg_many(uint8_t reg, uint8_t *data, uint8_t data_len) {
    return rf24_read(0x00 | reg, data, data_len);
}

static inline uint8_t rf24_write_reg_many(uint8_t reg, uint8_t *data, uint8_t data_len) {
    return rf24_write(0x20 | reg, data, data_len);
}

/* public functions */
void rf24_init(uint8_t address[5], uint8_t channel) {
    // MCC Melody forgot a volatile, make sure to add back when reconfiguring...
    SPI0_Open(0);
    rf24_write_reg_one(RF24_CONFIG,     0x0E); // all ISR enabled, 2-byte CRC enabled, power up!
    rf24_write_reg_one(RF24_EN_RXADDR,  0x01); // enable pipe 0 only
    rf24_write_reg_one(RF24_SETUP_RETR, 0x01); // 0.25ms retransmit delay, 1 retry
    rf24_write_reg_one(RF24_CH,         channel & 0x7F);
    rf24_write_reg_one(RF24_SETUP,      0x06); // 1Mbps, 0dBm
    rf24_write_reg_one(RF24_RX_PW_P0,   32);
    rf24_write_reg_one(RF24_RX_PW_P1,   32);
    rf24_write_reg_one(RF24_RX_PW_P2,   32);
    rf24_write_reg_one(RF24_RX_PW_P3,   32);
    rf24_write_reg_one(RF24_RX_PW_P4,   32);
    rf24_write_reg_one(RF24_RX_PW_P5,   32);
    rf24_write_reg_one(RF24_DYNPD,      0x3F); // enable dynamic payload on all pipes
    rf24_write_reg_one(RF24_FEATURE,    0x04); // enable dynamic payload, no payload in ack, always expect ack

    rf24_write_reg_many(RF24_RX_ADDR_P0, address, 5);
    memcpy(rf24_data.address, address, 5);
    rf24_data.address_update = false;
    CE_SetHigh();
}

bool rf24_irq_valid() {
    return IRQ_GetValue() == 0;
}

void rf24_start_rx() {
    // switch to PRX
    uint8_t reg;
    rf24_read_reg_one(RF24_CONFIG, &reg);
    rf24_write_reg_one(RF24_CONFIG, reg | 0x01);
    if (rf24_data.address_update) {
        rf24_open_rx_pipe(0, rf24_data.address);
        rf24_data.address_update = false;
    }
}

void rf24_open_rx_pipe(uint8_t pipe, uint8_t address[5]) {
    if (pipe == 0) {
        memcpy(rf24_data.address, address, 5);
    }
    switch (pipe) {
        case 0: rf24_write_reg_many(RF24_RX_ADDR_P0, address, 5); break;
        case 1: rf24_write_reg_many(RF24_RX_ADDR_P1, address, 5); break;
        case 2: rf24_write_reg_one( RF24_RX_ADDR_P2, address[4]); break; // ignores 1st 4 bytes
        case 3: rf24_write_reg_one( RF24_RX_ADDR_P3, address[4]); break;
        case 4: rf24_write_reg_one( RF24_RX_ADDR_P4, address[4]); break;
        case 5: rf24_write_reg_one( RF24_RX_ADDR_P5, address[4]); break;
    }
    uint8_t reg;
    rf24_read_reg_one(RF24_EN_RXADDR, &reg);
    rf24_write_reg_one(RF24_EN_RXADDR, reg | (1 << pipe));
}

uint8_t rf24_recv(uint8_t *data, uint8_t *data_len) {
    uint8_t status = rf24_write_reg_one(RF24_STATUS, 0x40); // clear interrupt
    uint8_t pipe = (status >> 1) & 0x07;
    rf24_read(RF24_R_RX_PL_WID, data_len, 1);
    rf24_read(RF24_R_RX_PAYLOAD, data, *data_len);
    return pipe;
}

void rf24_start_tx(uint8_t address[5]) {
    // switch to PTX
    uint8_t reg;
    rf24_read_reg_one(RF24_CONFIG, &reg);
    rf24_write_reg_one(RF24_CONFIG, reg & ~0x01);
    // open new address
    bool diff = false;
    for (int i = 0; i < 5; i++) {
        if (address[i] != rf24_data.address[i]) {
            diff = true;
            break;
        }
    }
    if (diff) {
        rf24_data.address_update = true;
    }
    rf24_open_rx_pipe(0, address);
    rf24_write_reg_many(RF24_TX_ADDR, address, 5);
    rf24_read_reg_one(RF24_EN_RXADDR, &reg);
    rf24_write_reg_one(RF24_EN_RXADDR, reg | (1 << 0)); // enable pipe 0
}

void rf24_send(uint8_t *data, uint8_t data_len) {
    CSN_SetLow();
    SPI0_ByteExchange(RF24_W_TX_PAYLOAD);
    SPI0_RegisterTransferDoneCallback(rf24_transfer_done_cb);
    SPI0_BufferWrite(data, data_len); // non-blocking!
}

bool rf24_tx_status() {
    uint8_t status = rf24_write_reg_one(RF24_STATUS, 0x30); // clear interrupts
    return status & 0x20; // checking TX_DS, can also do MAX_RT
}
