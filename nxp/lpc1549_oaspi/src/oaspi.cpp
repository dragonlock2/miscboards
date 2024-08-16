#include <cstdio>
#include <cstring>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <chip.h>
#include "spi.h"
#include "oaspi.h"

/* private constants */
static std::array<uint32_t, 256> FCS_TABLE {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};

/* private data */
static struct {
    SemaphoreHandle_t mdio_lock;
} data;

/* private helpers */
static uint32_t oaspi_fcs(std::array<uint8_t, OASPI_MAX_PKT_LEN>& pkt, size_t len) {
    uint32_t fcs = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        fcs = FCS_TABLE[(pkt[i] ^ fcs) & 0xFF] ^ (fcs >> 8);
    }
    return ~fcs;
}

static bool oaspi_control_check(std::array<uint8_t, 16>& rx) {
    std::array<uint8_t, 4> hdr {rx[4], rx[5], rx[6], rx[7]};
    if ((rx[4] & 0x40) || oaspi_parity(hdr)) {
        return false; // HDRB or parity error
    }
    if ((*reinterpret_cast<uint32_t*>(&rx[8]) ^ *reinterpret_cast<uint32_t*>(&rx[12])) != 0xFFFFFFFF) {
        return false; // protection error
    }
    return true;
}

static uint16_t oaspi_mdio_cmd(uint32_t cmd) {
    uint32_t ret;
    while (true) {
        oaspi_reg_write(oaspi_mms::STANDARD, 0x20, cmd);
        while (((ret = oaspi_reg_read(oaspi_mms::STANDARD, 0x20)) & 0x80000000) == 0);
        if (!(ret & 0x40000000)) { // turnaround error
            return ret & 0xFFFF;
        }
    }
}

static void oaspi_mac_filter(uint8_t slot, std::array<uint8_t, 6>& addr, std::array<uint8_t, 6>& mask) {
    uint8_t offset = slot * 2;
    uint32_t val;
    val = 0x40090000 | (addr[0] << 8) | (addr[1] << 0);
    oaspi_reg_write(oaspi_mms::MAC, 0x50 + offset, val);
    val = (addr[2] << 24) | (addr[3] << 16) | (addr[4] << 8) | (addr[5] << 0);
    oaspi_reg_write(oaspi_mms::MAC, 0x51 + offset, val);
    if (offset > 2) {
        return; // only 2 mask registers, rest need exact match
    }
    val = (mask[0] << 8) | (mask[1] << 0);
    oaspi_reg_write(oaspi_mms::MAC, 0x70 + offset, val);
    val = (mask[2] << 24) | (mask[3] << 16) | (mask[4] << 8) | (mask[5] << 0);
    oaspi_reg_write(oaspi_mms::MAC, 0x71 + offset, val);
}

/* public functions */
void oaspi_init(void) {
    data.mdio_lock = xSemaphoreCreateMutex();
    configASSERT(data.mdio_lock);

    // hardware reset
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 7);
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    Chip_GPIO_SetPinState(LPC_GPIO, 0, 7, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // software reset
    oaspi_reg_write(oaspi_mms::STANDARD, 0x03, 0x00000001);
    while (oaspi_reg_read(oaspi_mms::STANDARD, 0x01) != 0x0283BC91);
    oaspi_reg_write(oaspi_mms::STANDARD, 0x08, 0x00000040);
}

void oaspi_configure(void) {
    // MAC settings
    uint32_t t0;
    t0 = oaspi_reg_read(oaspi_mms::STANDARD, 0x04);
    oaspi_reg_write(oaspi_mms::STANDARD, 0x04, t0 | 0x5000); // TXFCSVE=1, ZARFE=1

    // MAC filter
    t0 = oaspi_reg_read(oaspi_mms::STANDARD, 0x06);
    oaspi_reg_write(oaspi_mms::STANDARD, 0x06, t0 | 0x0004); // P1_FWD_UNK2HOST=1
    (void) oaspi_mac_filter;
    // std::array<uint8_t, 6> bcast_addr {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    // oaspi_mac_filter(2, bcast_addr, bcast_addr);
    // std::array<uint8_t, 6> mcast_addr {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    // oaspi_mac_filter(0, mcast_addr, mcast_addr);
    // std::array<uint8_t, 6> ucast_addr;
    // extern uint8_t tud_network_mac_address[6];
    // std::memcpy(ucast_addr.data(), tud_network_mac_address, 6);
    // std::array<uint8_t, 6> ucast_mask {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    // oaspi_mac_filter(1, ucast_addr, ucast_mask);

    // PHY led setting
    uint16_t t1;
    t1 = oaspi_mdio_c45_read(0x1E, 0x8C56);
    oaspi_mdio_c45_write(0x1E, 0x8C56, t1 & ~0x000E); // enable LED1
    t1 = oaspi_mdio_c45_read(0x1E, 0x8C83);
    oaspi_mdio_c45_write(0x1E, 0x8C83, t1 | 0x0005); // active-high
    t1 = oaspi_mdio_c45_read(0x1E, 0x8C82);
    oaspi_mdio_c45_write(0x1E, 0x8C82, (t1 & ~(0x1F1F)) | 0x0304); // LED0: act, LED1: link

    // PHY turn on
    oaspi_mdio_c45_write(0x1E, 0x8812, 0x0000);
    while (oaspi_mdio_c45_read(0x1E, 0x8818) & 0x0002);

    // mark configured
    t0 = oaspi_reg_read(oaspi_mms::STANDARD, 0x04);
    oaspi_reg_write(oaspi_mms::STANDARD, 0x04, t0 | 0x8000); // SYNC=1
}

bool oaspi_parity(std::array<uint8_t, 4>& hdr) {
    uint32_t val = *reinterpret_cast<uint32_t*>(hdr.data()); // endian-independent
    val = val ^ (val >> 1);
    val = val ^ (val >> 2);
    val = val ^ (val >> 4);
    val = val ^ (val >> 8);
    val = val ^ (val >> 16);
    return !(val & 1);
}

void oaspi_fcs_add(std::array<uint8_t, OASPI_MAX_PKT_LEN>& pkt, size_t len) {
    uint32_t fcs = oaspi_fcs(pkt, len);
    pkt[len + 0] = (fcs >> 0)  & 0xFF;
    pkt[len + 1] = (fcs >> 8)  & 0xFF;
    pkt[len + 2] = (fcs >> 16) & 0xFF;
    pkt[len + 3] = (fcs >> 24) & 0xFF;
}

bool oaspi_fcs_check(std::array<uint8_t, OASPI_MAX_PKT_LEN>& pkt, size_t len) {
    return oaspi_fcs(pkt, len) == 0x2144DF1C;
}

void oaspi_data_transfer(oaspi_tx_chunk& tx, oaspi_rx_chunk& rx) {
    spi_transceive(reinterpret_cast<uint8_t*>(&tx), reinterpret_cast<uint8_t*>(&rx), sizeof(oaspi_tx_chunk));
}

void oaspi_reg_write(oaspi_mms mms, uint16_t reg, uint32_t val) {
    while (true) {
        std::array<uint8_t, 16> tx, rx;
        tx[0]  = 0x20 | static_cast<uint8_t>(mms);
        tx[1]  = (reg >> 8) & 0xFF;
        tx[2]  = reg & 0xFF;
        tx[3]  = 0x00;
        std::array<uint8_t, 4> hdr {tx[0], tx[1], tx[2], tx[3]};
        tx[3] |= oaspi_parity(hdr);
        tx[4]  = (val >> 24) & 0xFF;
        tx[5]  = (val >> 16) & 0xFF;
        tx[6]  = (val >> 8)  & 0xFF;
        tx[7]  = (val >> 0)  & 0xFF;
        tx[8]  = ~tx[4];
        tx[9]  = ~tx[5];
        tx[10] = ~tx[6];
        tx[11] = ~tx[7];
        spi_transceive(tx.data(), rx.data(), tx.size());
        if (oaspi_control_check(rx)) {
            break;
        }
    }
}

uint32_t oaspi_reg_read(oaspi_mms mms, uint16_t reg) {
    while (true) {
        std::array<uint8_t, 16> tx, rx;
        tx[0] = 0x00 | static_cast<uint8_t>(mms);
        tx[1] = (reg >> 8) & 0xFF;
        tx[2] = reg & 0xFF;
        tx[3] = 0;
        std::array<uint8_t, 4> hdr {tx[0], tx[1], tx[2], tx[3]};
        tx[3] |= oaspi_parity(hdr);
        spi_transceive(tx.data(), rx.data(), 16);
        if (oaspi_control_check(rx)) {
            return (rx[8] << 24) | (rx[9] << 16) | (rx[10] << 8) | (rx[11] << 0);
        }
    }
}

void oaspi_mdio_write(uint8_t reg, uint16_t val) {
    xSemaphoreTake(data.mdio_lock, portMAX_DELAY);
    uint32_t cmd = 0x14200000;
    cmd |= (reg & 0x1F) << 16;
    cmd |= val;
    oaspi_mdio_cmd(cmd);
    xSemaphoreGive(data.mdio_lock);
}

uint16_t oaspi_mdio_read(uint8_t reg) {
    xSemaphoreTake(data.mdio_lock, portMAX_DELAY);
    uint32_t cmd = 0x18200000;
    cmd |= (reg & 0x1F) << 16;
    uint16_t ret = oaspi_mdio_cmd(cmd);
    xSemaphoreGive(data.mdio_lock);
    return ret;
}

void oaspi_mdio_c45_write(uint8_t devad, uint16_t reg, uint16_t val) {
    xSemaphoreTake(data.mdio_lock, portMAX_DELAY);
    uint32_t cmd = 0x00200000;
    cmd |= (devad & 0x1F) << 16;
    cmd |= reg;
    oaspi_mdio_cmd(cmd);
    cmd  = 0x04200000;
    cmd |= (devad & 0x1F) << 16;
    cmd |= val;
    oaspi_mdio_cmd(cmd);
    xSemaphoreGive(data.mdio_lock);
}

uint16_t oaspi_mdio_c45_read(uint8_t devad, uint16_t reg) {
    xSemaphoreTake(data.mdio_lock, portMAX_DELAY);
    uint32_t cmd = 0x00200000;
    cmd |= (devad & 0x1F) << 16;
    cmd |= reg;
    oaspi_mdio_cmd(cmd);
    cmd  = 0x0c200000;
    cmd |= (devad & 0x1F) << 16;
    uint16_t ret = oaspi_mdio_cmd(cmd);
    xSemaphoreGive(data.mdio_lock);
    return ret;
}
