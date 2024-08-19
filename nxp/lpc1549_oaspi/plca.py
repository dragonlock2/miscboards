import hid

PLCA_ENABLE   = 1
PLCA_LEADER   = 1
PLCA_NODE_ID  = 0 # if 0, always leader
PLCA_NODE_CNT = 8

if __name__ == "__main__":
    dev = hid.Device(0xcafe, 0x0069)
    dev.write(bytes([
        (bool(PLCA_LEADER) << 1) | (bool(PLCA_ENABLE) << 0),
        PLCA_NODE_ID,
        PLCA_NODE_CNT,
    ]))
