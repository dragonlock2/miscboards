import struct
import sys
import usb

PAGE_SIZE = 256

class Flash:
    def __init__(self, vid=0x0069, pid=0x0421):
        self.dev = usb.core.find(idVendor=vid, idProduct=pid)
        self.dev.set_configuration()
        self.off()

    # public functions
    def off(self):
        self._cmd(0)

    def on(self):
        self._cmd(1)

    def erase(self, addr, size):
        self._cmd(2, addr=addr, size=size)

    def write(self, addr, data):
        while data:
            data += b'\xff' * max(0, PAGE_SIZE - len(data))
            self._cmd(3, addr=addr, data=data[:PAGE_SIZE])
            addr += PAGE_SIZE
            data = data[PAGE_SIZE:]

    def read(self, addr, size):
        data = b''
        while size > 0:
            data += self._cmd(4, addr=addr)[2]
            addr += PAGE_SIZE
            size -= PAGE_SIZE
        return data[:size]

    # private helpers
    def _cmd(self, id, addr=0, size=0, data=b'\xff'*PAGE_SIZE):
        assert(len(data) == PAGE_SIZE)
        self.dev.write(0x01,
            id.to_bytes(4, 'little') +
            addr.to_bytes(4, 'little') +
            size.to_bytes(4, 'little') +
            data
        )
        ret, addr, size, data = struct.unpack('<III256s', self.dev.read(0x81, 12+PAGE_SIZE))
        assert(ret == 0)
        return addr, size, data

if __name__ == '__main__':
    f = Flash()

    if len(sys.argv) == 2:
        with open(sys.argv[1], 'rb') as r:
            d = r.read()
            print(f'writing length {len(d)} file')
            f.erase(0, len(d))
            f.write(0, d)
            print("match?", f.read(0, len(d)) == d)
            f.on()
    else:
        print('pass in a file!')
