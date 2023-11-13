import os
import serial
import sys
import time

class Flash:
    def __init__(self, port):
        self.ser = serial.Serial(port, 115200)
        self.ser.read_all()

    # public functions
    def erase(self, addr, length):
        self._cmd(0x01, addr.to_bytes(4, 'big') + length.to_bytes(4, 'big'))

    def write(self, addr, data):
        if len(data) % 256 != 0:
            data += b'\xff' * (256 - len(data) % 256)
        for a in range(addr, addr + len(data), 256):
            self._cmd(0x02, a.to_bytes(4, 'big') + data[:256])
            data = data[256:]

    def read(self, addr, length):
        d = b''
        for a in range(addr, addr + length, 256):
            d += self._cmd(0x03, a.to_bytes(4, 'big'))
        return d[:length]

    # private helpers
    def _cmd(self, type, data):
        # TODO write a proper rpc
        self.ser.write(b'\x69' + bytes([type]) + bytes(data))
        t = self.ser.read(1)
        if t[0] == 0x69:
            l = int.from_bytes(self.ser.read(4), 'big')
            return self.ser.read(l)
        raise Exception('retcode error')

if __name__ == '__main__':
    f = Flash('/dev/tty.usbmodem7DE58F0602C22')

    if len(sys.argv) == 2:
        with open(sys.argv[1], 'rb') as r:
            d = r.read()
            print(f'writing length {len(d)} file')
            f.erase(0, len(d))
            f.write(0, d)
            print(f.read(0, len(d)) == d)
    else:
        print('pass in a file!')
