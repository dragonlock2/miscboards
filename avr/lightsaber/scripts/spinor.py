import jabi
import time
import random

class SPINOR:
    def __init__(self, dev, id, cs):
        self.dev = dev
        self.id  = id
        self.cs  = cs

        self.dev.gpio_set_mode(self.cs, jabi.GPIODir.OUTPUT, init_val=1)
        self.dev.spi_set_freq(32000000, self.id)
        self.dev.spi_set_mode(0, self.id)
        self.dev.spi_set_bitorder(True, self.id)

        # parse SFDP header
        self.dev.gpio_write(self.cs, 0)
        self.dev.spi_write([0x5a, 0x00, 0x00, 0x00, 0xFF], self.id)
        buf = self.dev.spi_read(8, self.id)
        if buf[0:4] != [0x53, 0x46, 0x44, 0x50]:
            raise Exception('invalid SFDP header!')
        self.rev = str(buf[5]) + '.' + str(buf[4]) # TODO use
        self.nph = buf[6] + 1
        self.access_protocol = buf[7] # TODO use
        self.parameter_headers = []
        for i in range(self.nph):
            buf = self.dev.spi_read(8, self.id)
            self.parameter_headers.append({
                'id_lsb': buf[0],
                'id_msb': buf[7], # TODO use
                'rev'   : str(buf[2]) + '.' + str(buf[1]), # TODO use
                'len'   : buf[3] * 4,
                'ptp'   : int.from_bytes(bytes(buf[4:7]), 'little'),
            })
        self.dev.gpio_write(self.cs, 1)

        # parse SFDP parameter tables
        for ph in self.parameter_headers:
            self.dev.gpio_write(self.cs, 0)
            self.dev.spi_write([0x5A] + list(ph['ptp'].to_bytes(3, 'big')) + [0xFF], self.id)
            buf = self.dev.spi_read(ph['len'], self.id)
            self.dev.gpio_write(self.cs, 1)
            if ph['id_lsb'] == 0x00:
                self.wren = bool(buf[0] & 0x08)
                self.wren_code = 0x06 if buf[0] & 0x10 else 0x50

                if (addr_bytes := (buf[2] >> 1) & 0b11) in [0b00, 0b01]:
                    self.addr_len = 3 # TODO support 4-byte address enable command
                elif addr_bytes == 0b10:
                    self.addr_len = 4
                else:
                    raise Exception('unknown address length!')

                if (density := int.from_bytes(bytes(buf[4:8]), 'little')) & 0x80000000:
                    self.flash_size = (2 ** (self.flash_size & 0x7FFFFFFF)) // 8
                else:
                    self.flash_size = (density + 1) // 8

                # only use first erase type size
                self.erase_size = 2 ** buf[28]
                self.erase_code = buf[29]

                if len(buf) >= 41:
                    self.page_size = 2 ** ((buf[40] >> 4) & 0x0F)
                else:
                    self.page_size = 256
                self.page_size = min(self.page_size, self.dev.req_max_size())

                self.read_size = self.dev.resp_max_size()

    def read(self, addr, length):
        if addr + length > self.flash_size:
            raise Exception('read past end of flash!')
        self.dev.gpio_write(self.cs, 0)
        self.dev.spi_write([0x03] + list(addr.to_bytes(self.addr_len, 'big')), self.id)
        data = []
        while length > 0:
            data.extend(self.dev.spi_read(min(length, self.read_size), self.id))
            length -= self.read_size
        self.dev.gpio_write(self.cs, 1)
        return data

    def erase(self, addr, length):
        if addr + length > self.flash_size:
            raise Exception('erase past end of flash!')
        if addr % self.erase_size != 0:
            raise Exception(f'address must be sector ({self.erase_size}-byte) aligned!')
        for _ in range((length + self.erase_size - 1) // self.erase_size):
            # enable write
            self.dev.gpio_write(self.cs, 0)
            self.dev.spi_write([0x06], self.id)
            self.dev.gpio_write(self.cs, 1)
            # sector erase
            self.dev.gpio_write(self.cs, 0)
            self.dev.spi_write([self.erase_code] + list(addr.to_bytes(self.addr_len, 'big')), self.id)
            self.dev.gpio_write(self.cs, 1)
            # wait to finish
            self.dev.gpio_write(self.cs, 0)
            while self.dev.spi_transceive([0x05, 0xFF], self.id)[1] & 0x01:
                self.dev.gpio_write(self.cs, 1)
                time.sleep(0.001)
                self.dev.gpio_write(self.cs, 0)
            self.dev.gpio_write(self.cs, 1)

            addr += self.erase_size
        
    def write(self, addr, data):
        if addr + len(data) > self.flash_size:
            raise Exception('write past end of flash!')
        if addr % self.page_size != 0:
            raise Exception(f'address must be page ({self.page_size}-byte) aligned!')
        pages = [data[i:i+self.page_size] for i in range(0,len(data),self.page_size)]
        for p in pages:
            # enable write
            self.dev.gpio_write(self.cs, 0)
            self.dev.spi_write([0x06], self.id)
            self.dev.gpio_write(self.cs, 1)
            # page write
            self.dev.gpio_write(self.cs, 0)
            self.dev.spi_write([0x02] + list(addr.to_bytes(self.addr_len, 'big')) + p, self.id)
            self.dev.gpio_write(self.cs, 1)
            # wait to finish
            self.dev.gpio_write(self.cs, 0)
            while self.dev.spi_transceive([0x05, 0xFF], self.id)[1] & 0x01:
                self.dev.gpio_write(self.cs, 1)
                time.sleep(0.001)
                self.dev.gpio_write(self.cs, 0)
            self.dev.gpio_write(self.cs, 1)

            addr += self.page_size

    def __len__(self):
        return self.flash_size

class LightFS: # filesystem for lightsabers!
    def __init__(self, flash, freq=25000, read=False):
        self.flash = flash
        self.freq  = freq

if __name__ == '__main__':
    # for k66f_breakout
    flash = SPINOR(jabi.USBInterface.list_devices()[0], 0, 8)

    addr = 0x000000
    data = list(random.randbytes(256))
    flash.erase(addr, len(data))
    flash.write(addr, data)
    assert(flash.read(0, len(data)) == data)

