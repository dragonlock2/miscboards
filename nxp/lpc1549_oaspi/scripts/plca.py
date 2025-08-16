import hid # pip install hidapi
import struct

TIMEOUT = 10 # ms

def reg_write(mms, reg, val):
    dev.write(b"\x00" + b"\x00" + struct.pack(">BHI", mms, reg, val))
    assert(dev.read(64, TIMEOUT)[0] == 0)

def reg_read(mms, reg):
    dev.write(b"\x00" + b"\x01" + struct.pack(">BH", mms, reg))
    resp = dev.read(64, TIMEOUT)
    assert(resp[0] == 0)
    return int.from_bytes(resp[1:5], "big")

def set_plca(enable=True, leader=True, node_id=0, node_cnt=8):
    assert(node_id <= 0xFF and node_cnt <= 0xFF)
    assert(reg_read(0, 0x0001) == 0xbc0189a1) # only works on NCN26010
    MMS = 4
    reg_write(MMS, 0x8002, 0x0003 if leader else 0x0002)
    reg_write(MMS, 0xCA01, bool(enable) << 15)
    reg_write(MMS, 0xCA02, (node_cnt << 8) | (node_id << 0))

if __name__ == "__main__":
    dev = hid.device()
    dev.open_path(hid.enumerate(0xcafe, 0x0069)[0]["path"])
    print(f"id={hex(reg_read(0, 0x0001))}")

    set_plca()
    # set_plca(leader=False, node_id=1)
    # set_plca(False)
