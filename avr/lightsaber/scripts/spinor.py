import jabi
import sys
import time
import wave
import samplerate
import numpy as np
import sounddevice as sd
from pathlib import Path
from tqdm import tqdm

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

"""
LightFS is a lightweight "filesystem" for lightsabers. Since it's for personal use, it's
designed to be just functional enough. It all starts with the header at address 0x00000000.
All fields are stored big endian.

struct header {
    uint32_t sample_frequency; // Hz
    uint32_t font_addr[8]; // unused entries set to 0xFF
};

Each font_addr is an address pointing to the following struct.

struct font {
    uint32_t boot;
    uint32_t hum;
    uint32_t off;
    uint32_t on;
    uint32_t swing[10]; // unused entries set to 0xFF
    uint32_t clash[10]; // unused entries set to 0xFF
};

Each entry in the font struct is an address pointing to the following struct.

struct file {
    uint32_t len;
    uint8_t  data[len]; // 8-bit audio
};
"""
class LightFS:
    def __init__(self, flash, fs=25000, read=False):
        self.flash = flash
        self.fs    = fs

        if read:
            hdr = self.flash.read(0, 36)
            self.fs = self._get(hdr, 0, 4)
            self.fonts = []
            while (f := self._get(hdr, 4+len(self.fonts)*4, 4)) != 0xFFFFFFFF:
                font = self.flash.read(f, 96)
                d = {
                    'boot': [self._get(font, 0,  4)],
                    'hum' : [self._get(font, 4,  4)],
                    'off' : [self._get(font, 8,  4)],
                    'on'  : [self._get(font, 12, 4)],
                    'swing' : [],
                    'clash' : [],
                }
                while (a := self._get(font, 16+len(d['swing'])*4, 4)) != 0xFFFFFFFF:
                    d['swing'].append(a)
                while (a := self._get(font, 56+len(d['clash'])*4, 4)) != 0xFFFFFFFF:
                    d['clash'].append(a)
                self.fonts.append(d)

    # high-level functions
    def write(self, src):
        # compiler
        fonts = []
        for p in src.iterdir():
            if p.is_dir():
                fonts.append({
                    'boot' : self._resample(p / 'boot.wav'),
                    'hum'  : self._resample(p / 'hum.wav'),
                    'off'  : self._resample(p / 'off.wav'),
                    'on'   : self._resample(p / 'on.wav'),
                    'swing': [self._resample(f) for f in p.glob('swing*.wav')],
                    'clash': [self._resample(f) for f in p.glob('clash*.wav')],
                })

        # linker
        self.mem = []
        hdr_addr = self._alloc(36)
        self._set(hdr_addr, self.fs.to_bytes(4, 'big'))
        for i, f in enumerate(fonts):
            f['swing'] = f['swing'][:10]
            f['clash'] = f['clash'][:10]
            font_addr = self._alloc(96)
            self._set(hdr_addr+4+i*4, font_addr.to_bytes(4, 'big'))

            for j, t in enumerate(['boot', 'hum', 'off', 'on']):
                a = self._alloc(4+len(f[t]))
                self._set(font_addr+j*4, a.to_bytes(4, 'big'))
                self._set(a, len(f[t]).to_bytes(4, 'big'))
                self._set(a+4, f[t])

            for j, d in enumerate(f['swing']):
                a = self._alloc(4+len(d))
                self._set(font_addr+16+j*4, a.to_bytes(4, 'big'))
                self._set(a, len(d).to_bytes(4, 'big'))
                self._set(a+4, d)

            for j, d in enumerate(f['clash']):
                a = self._alloc(4+len(d))
                self._set(font_addr+56+j*4, a.to_bytes(4, 'big'))
                self._set(a, len(d).to_bytes(4, 'big'))
                self._set(a+4, d)

        if len(self.mem) > len(self.flash):
            raise Exception(f'too much sound! {self.mem} > {self.flash}')

        # erase
        pb = tqdm(range(0,len(self.mem),self.flash.erase_size))
        pb.set_description('erase')
        for i in pb:
            self.flash.erase(i, self.flash.erase_size)

        # write
        pb = tqdm([self.mem[i:i+self.flash.page_size] for i in range(0,len(self.mem),self.flash.page_size)])
        pb.set_description('write')
        for i, p in enumerate(pb):
            self.flash.write(i*self.flash.page_size, p)

        # verify
        VERIFY_LEN = 1024
        pb = tqdm([self.mem[i:i+VERIFY_LEN] for i in range(0,len(self.mem),VERIFY_LEN)])
        pb.set_description('verify')
        for i, p in enumerate(pb):
            if p != self.flash.read(i*VERIFY_LEN, min(VERIFY_LEN, len(p))):
                raise Exception(f'mismatch at {i*VERIFY_LEN}!')

    def play(self, t, font=0, idx=0):
        length = self._get(self.flash.read(self.fonts[font][t][idx], 4), 0, 4)
        data = np.array(self.flash.read(self.fonts[font][t][idx]+4, length))
        sd.play((data - 128) / 128, self.fs, blocking=True)

    def num_fonts(self):
        return len(self.fonts)

    def num(self, t, font=0):
        return len(self.fonts[font][t])

    # low-level helpers
    def _resample(self, file):
        file = file.open('rb')
        w = wave.open(file)
        sw = w.getsampwidth()
        rd = w.readframes(w.getnframes())
        rd = np.array([int.from_bytes(rd[i:i+sw], 'little', signed=sw>1) for i in range(0,len(rd),sw)])
        rd = samplerate.resample(rd[::w.getnchannels()], self.fs / w.getframerate(), 'sinc_best')
        if sw > 1: # signed, convert to 8-bit unsigned
            rd = rd / (2 ** (8*sw-1)) * 256
            rd += 128
        file.close()
        return rd.round().clip(0, 255).astype(np.uint8)

    def _alloc(self, length):
        addr = len(self.mem)
        self.mem.extend([0xFF]*length)
        return addr

    def _set(self, addr, data):
        self.mem[addr:addr+len(data)] = data

    def _get(self, data, idx, length):
        return int.from_bytes(bytes(data[idx:idx+length]), 'big')

if __name__ == '__main__':
    # for k66f_breakout
    flash = SPINOR(jabi.USBInterface.list_devices()[0], 0, 8)

    # write fonts
    light = LightFS(flash)
    light.write(sys.argv[1] if len(sys.argv) == 2 else Path(__file__).parents[1] / 'fonts/')

    # read back fonts
    light = LightFS(flash, read=True)
    for i in range(light.num_fonts()):
        light.play('boot', font=i)
        light.play('hum', font=i)
        light.play('on', font=i)
        light.play('off', font=i)
        for j in range(light.num('swing', font=i)):
            light.play('swing', font=i, idx=j)
        for j in range(light.num('clash', font=i)):
            light.play('clash', font=i, idx=j)
