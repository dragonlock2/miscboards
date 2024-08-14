import os
import crc

POLYNOMIAL = 0xEDB88320
CHECK      = 0x2144DF1C

def crc_table():
    table = [0] * 256
    for b in range(256):
        c = b
        for _ in range(8):
            if c & 1:
                c = POLYNOMIAL ^ (c >> 1)
            else:
                c = c >> 1
        table[b] = c
    return table

def crc_fast(table, data):
    c = 0xFFFFFFFF
    for b in data:
        c = table[(b ^ c) & 0xFF] ^ (c >> 8)
    return (c & 0xFFFFFFFF) ^ 0xFFFFFFFF

def crc_lib(data):
    return crc.Calculator(crc.Crc32.CRC32).checksum(data)

if __name__ == "__main__":
    table = crc_table()
    pkt = os.urandom(256)
    assert(crc_lib(pkt) == crc_fast(table, pkt))
    pkt += crc_lib(pkt).to_bytes(4, "little")
    assert(crc_fast(table, pkt) == CHECK)

    print("static std::array<uint32_t, 256> FCS_TABLE {")
    for i in range(0, 256, 8):
        print("   ", end="")
        for j in range(8):
            print(f" 0x{table[i + j]:08X},", end="")
        print()
    print("};")
