#include <string.h>
#include "mcc_generated_files/system/system.h"
#include "lsm303ah.h"

/* private defines */
#define ACCEL_ADDR (0x1D)
#define MAG_ADDR   (0x1E)

typedef enum {
    LSM303AH_STATE_BUF0_ACCEL,
    LSM303AH_STATE_BUF0_MAG,
    LSM303AH_STATE_BUF1_ACCEL,
    LSM303AH_STATE_BUF1_MAG,
} lsm303ah_state_E;

typedef struct {
    lsm303ah_state_E state;
    lsm303ah_result_S buf[2];
} lsm303ah_data_S;
static lsm303ah_data_S lsm303ah_data;

static const uint8_t ACCEL_OUT_REG = 0x28;
static const uint8_t MAG_OUT_REG   = 0x68;

/* private helpers */
static void lsm303ah_write_reg(bool accel, uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    TWI0_Write(accel ? ACCEL_ADDR : MAG_ADDR, buf, sizeof buf);
    while (TWI0_IsBusy());
}

static uint8_t lsm303ah_read_reg(bool accel, uint8_t reg) {
    uint8_t ret;
    TWI0_WriteRead(accel ? ACCEL_ADDR : MAG_ADDR, &reg, 1, &ret, 1);
    while (TWI0_IsBusy());
    return ret;
}

/* public functions */
void lsm303ah_init() {
    if (lsm303ah_read_reg(true, 0x0F) != 0x43 || lsm303ah_read_reg(false, 0x4F) != 0x40) {
        printf("wrong whoami!\r\n");
    }
    lsm303ah_write_reg(true,  0x20, 0x79); // ODR=800Hz,14-bit, FS=4g, HF_ODR=0, BDU=1
    lsm303ah_write_reg(true,  0x23, 0x20); // INT1_WU=1
    lsm303ah_write_reg(true,  0x33, 0x03); // SLEEP_ON=0, WU_THS=3 (hard shake wakes up)
    lsm303ah_write_reg(true,  0x34, 0x2F); // WU_DUR=1, SLEEP_DUR=15
    lsm303ah_write_reg(false, 0x60, 0x8C); // COMP_TEMP_EN=1, LP=0, ODR=100Hz, MD=00
    lsm303ah_data.state = LSM303AH_STATE_BUF0_ACCEL;
}

void lsm303ah_read(lsm303ah_result_S *result) {
    // AVR is little-endian so no byte flip!
    if (!TWI0_IsBusy()) {
        switch (lsm303ah_data.state) {
            case LSM303AH_STATE_BUF0_ACCEL:
                TWI0_WriteRead(ACCEL_ADDR, (uint8_t*) &ACCEL_OUT_REG, 1, (uint8_t*) &lsm303ah_data.buf[0].ax, 6);
                lsm303ah_data.state = LSM303AH_STATE_BUF0_MAG;
                break;
                
            case LSM303AH_STATE_BUF0_MAG:
                TWI0_WriteRead(MAG_ADDR, (uint8_t*) &MAG_OUT_REG, 1, (uint8_t*) &lsm303ah_data.buf[0].mx, 6);
                lsm303ah_data.state = LSM303AH_STATE_BUF1_ACCEL;
                break;
                
            case LSM303AH_STATE_BUF1_ACCEL:
                TWI0_WriteRead(ACCEL_ADDR, (uint8_t*) &ACCEL_OUT_REG, 1, (uint8_t*) &lsm303ah_data.buf[1].ax, 6);
                lsm303ah_data.state = LSM303AH_STATE_BUF1_MAG;
                break;
                
            case LSM303AH_STATE_BUF1_MAG:
                TWI0_WriteRead(MAG_ADDR, (uint8_t*) &MAG_OUT_REG, 1, (uint8_t*) &lsm303ah_data.buf[1].mx, 6);
                lsm303ah_data.state = LSM303AH_STATE_BUF0_ACCEL;
                break;
        }
    }
    
    switch (lsm303ah_data.state) { // indicates action pending, not started
        case LSM303AH_STATE_BUF0_ACCEL:
            memcpy(&result->ax, &lsm303ah_data.buf[1].ax, 6);
            memcpy(&result->mx, &lsm303ah_data.buf[0].mx, 6);
            break;
            
        case LSM303AH_STATE_BUF0_MAG:
            memcpy(&result->ax, &lsm303ah_data.buf[1].ax, 6);
            memcpy(&result->mx, &lsm303ah_data.buf[1].mx, 6);
            break;
            
        case LSM303AH_STATE_BUF1_ACCEL:
            memcpy(&result->ax, &lsm303ah_data.buf[0].ax, 6);
            memcpy(&result->mx, &lsm303ah_data.buf[1].mx, 6);
            break;
            
        case LSM303AH_STATE_BUF1_MAG:
            memcpy(&result->ax, &lsm303ah_data.buf[0].ax, 6);
            memcpy(&result->mx, &lsm303ah_data.buf[0].mx, 6);
            break;
    }
}
