import jabi
import time
import struct
from pathlib import PosixPath
from math import ceil

class SD:
    BLOCK_LEN = 512

    def __init__(self, dev, id, cs):
        self.dev = dev
        self.id  = id
        self.cs  = cs

        self.dev.gpio_set_mode(self.cs, jabi.GPIODir.OUTPUT)
        self.dev.spi_set_bitorder(True, self.id) # MSB first
        self.dev.spi_set_freq(400000, self.id) # init 100-400kHz
        self.dev.spi_set_mode(0, self.id)

        # init SD in SPI mode
        self.dev.gpio_write(self.cs, True)
        self.dev.spi_write([0xFF]*10, self.id)
        self.dev.gpio_write(self.cs, False)

        # CMD0
        self.CMD(0, crc=0x95)
        self.check_r1(self.R1())
        
        # CMD8
        self.CMD(8, [0x00, 0x00, 0x01, 0xAA], crc=0x87)
        if (r7 := self.R7())[3] != 0xAA:
            raise Exception(f'bad CMD8 response {r7}')

        # CMD59 - disable crc
        self.CMD(59, crc=0x91)
        self.check_r1(self.R1())

        # ACMD41 - initialize card
        sup_acmd41 = False
        for i in range(10):
            self.ACMD(41, [0x40, 0x00, 0x00, 0x00])
            if self.check_r1(self.R1(), False):
                sup_acmd41 = True
                break
        if not sup_acmd41:
            raise Exception('card failed ACMD41')
        timeout_ctr = 0
        while True:
            self.ACMD(41, [0x40, 0x00, 0x00, 0x00])
            if not self.R1()[0] & 0x01:
                break
            if timeout_ctr >= 50:
                raise Exception('init took longer than 500ms!')
            timeout_ctr += 1
            time.sleep(0.01)
        
        # bump up clock
        self.dev.spi_set_freq(25000000, self.id)
        
        # CMD58 - read OCR
        self.CMD(58)
        self.sdhc = bool(self.R3()[0] & 0x40)

        # CMD16 - set block length to BLOCK_LEN
        self.CMD(16, list(self.BLOCK_LEN.to_bytes(4, 'big')))
        self.check_r1(self.R1())

        # CMD9 - read CSD
        self.CMD(9)
        self.check_r1(self.R1())
        csd = self.data_response(16)

        # CMD10 - read CID
        self.CMD(10)
        self.check_r1(self.R1())
        cid = self.data_response(16)

        # decode card info
        print('SDHC detected' if self.sdhc else 'SD detected')

        print(f'MID: {cid[0]:#02x}')
        print(f'OID: "{chr(cid[1]) + chr(cid[2])}"')
        print(f'PNM: "{"".join([chr(c) for c in cid[3:8]])}"')
        print(f'PRV: {cid[8]>>4}.{cid[8]&0x0F}')
        print(f'PSN: {struct.unpack(">I", bytes(cid[9:13]))[0]:#08x}')
        print(f'mfg date: {cid[13]&0x0F}/{2000+((cid[13]&0x0F)<<4|(cid[14]&0xF0)>>4)}')

        csd = struct.unpack(">QQ", bytes(csd))
        csd = csd[0] << 64 | csd[1]
        csd_structure = (csd >> 126) & 0x03
        if csd_structure == 0:
            block_len = 2 ** ((csd >> 80) & 0x0F)
            mult = 2 ** ((csd >> 47) & 0x07 + 2)
            blocknr = (((csd >> 62) & 0x03FF) + 1) * mult
            print(f'block length: {block_len}')
            print(f'num blocks: {blocknr}')
            print(f'total size: {blocknr * block_len / (2 ** 20)} MiB')
        elif csd_structure == 1:
            c_size = (csd >> 48) & 0x3FFFFF
            print(f'num blocks: {(c_size + 1) * (2 ** 19) // self.BLOCK_LEN}')
            print(f'total size: {(c_size + 1) * (2 ** 19) / (2 ** 30)} GiB')
        else:
            raise Exception('unknown CSD version')

    # SD high-level commands
    def read_block(self, idx):
        if not self.sdhc:
            idx *= self.BLOCK_LEN
        self.CMD(17, list(idx.to_bytes(4, 'big')))
        self.check_r1(self.R1())
        return self.data_response(self.BLOCK_LEN)

    def read_blocks(self, idx, num):
        if not self.sdhc:
            idx *= self.BLOCK_LEN
        self.CMD(18, list(idx.to_bytes(4, 'big')))
        self.check_r1(self.R1())
        blocks = [self.data_response(self.BLOCK_LEN) for _ in range(num)]
        self.CMD(12)
        self.R1b()
        return blocks

    # SD low-level commands
    def CMD(self, cmd, arg=[0x00]*4, crc=0x01):
        self.dev.spi_write([0x40 | cmd] + arg + [crc], self.id)

    def ACMD(self, cmd, arg=[0x00]*4):
        self.CMD(55) # APP_CMD
        self.R1()
        self.CMD(cmd, arg)

    def R1(self):
        return self.get_resp(1)

    def R1b(self):
        # note not checking/returning R1, got 127 for some reason
        self.get_resp(1)
        ctr = 0
        while (b := self.dev.spi_transceive([0xFF], self.id)) == 0x00:
            ctr += 1
            if ctr >= 100:
                raise Exception('busy for too long')
            time.sleep(0.001)

    def R3(self):
        return self.check_r1(self.get_resp(5))[1:]

    def R7(self):
        return self.check_r1(self.get_resp(5))[1:]

    def data_response(self, length):
        ctr = 0
        while (b := self.dev.spi_transceive([0xFF], self.id)[0]) != 0xFE:
            ctr += 1
            if ctr >= 100:
                raise Exception('no data response')
            time.sleep(0.001)
        return self.dev.spi_transceive([0xFF]*(length+2), self.id)[:-2] # ignore CRC16

    # helpers
    def check_r1(self, buf, exception=True):
        if buf[0] & 0x7C:
            if exception:
                raise Exception(f'bad R1 {buf[0]}')
            else:
                return None
        return buf

    def get_resp(self, length):
        ctr = 0
        while (b := self.dev.spi_transceive([0xFF], self.id)[0]) == 0xFF:
            ctr += 1
            if ctr > 8:
                raise Exception('no response')
        return [b] + self.dev.spi_transceive([0xFF]*(length-1), self.id)

class FAT:
    BYTES_PER_SECTOR = 512

    def __init__(self, sd):
        self.sd = sd

        p1 = self.sd.read_block(0)[446:446+16]
        if p1[4] not in [0x0B, 0x0C, 0xE]:
            raise Exception(f'unknown filesystem type: {p1[4]}')
        self.volume_id = struct.unpack('<I', bytes(p1[8:8+4]))[0]
        boot_sect = self.sd.read_block(self.volume_id)

        print(f'formatting OS: {"".join(chr(c) for c in boot_sect[3:3+8]).strip()}')
        self.sectors_per_cluster = struct.unpack('<B', bytes(boot_sect[13:13+1]))[0]
        self.rsvd_sectors = struct.unpack('<H', bytes(boot_sect[14:14+2]))[0]
        self.num_fat = struct.unpack('<B', bytes(boot_sect[16:16+1]))[0]
        self.sectors_per_fat = struct.unpack('<H', bytes(boot_sect[22:22+2]))[0]
        if self.sectors_per_fat: # FAT16
            self.is_fat32 = False

            print(f'drive name: {"".join(chr(c) for c in boot_sect[43:43+11]).strip()}')
            self.num_root_entries = struct.unpack('<H', bytes(boot_sect[17:17+2]))[0]
            self.num_sectors = struct.unpack('<H', bytes(boot_sect[19:19+2]))[0]
            if self.num_sectors == 0:
                self.num_sectors = struct.unpack('<I', bytes(boot_sect[32:32+4]))[0]

            # compute some stuff
            self.fat_sector = self.volume_id + self.rsvd_sectors
            self.root_dir_sector = self.fat_sector + self.num_fat * self.sectors_per_fat
            self.num_root_sectors = ceil(self.num_root_entries * 32 / self.BYTES_PER_SECTOR)
            self.data_sector = self.root_dir_sector + self.num_root_sectors - 2 * self.sectors_per_cluster
        else: # FAT32
            self.is_fat32 = True

            print(f'drive name: {"".join(chr(c) for c in boot_sect[71:71+11]).strip()}')
            self.rsvd_sectors = struct.unpack('<H', bytes(boot_sect[14:14+2]))[0]
            self.sectors_per_fat = struct.unpack('<I', bytes(boot_sect[36:36+4]))[0]

            self.fat_sector = self.volume_id + self.rsvd_sectors
            self.data_sector = self.fat_sector + self.num_fat * self.sectors_per_fat - 2 * self.sectors_per_cluster

    def ls(self, name):
        def ls_helper(dirs, cluster):
            files = []
            found = False
            e = self.read_file_from_cluster(cluster)
            for d in [e[i:i+32] for i in range(0, len(e), 32)]:
                dirent = self.parse_dirent(d)
                if dirent['end']:
                    break
                elif dirent['volume_name'] or dirent['hidden'] or dirent['free']:
                    continue
                elif len(dirs) == 0: # ls of root dir
                    files.append({
                        'name': dirent['name'],
                        'dir': dirent['dir'],
                        'size': dirent['size'],
                        'first_cluster': dirent['first_cluster']
                    })
                elif dirent['dir'] and dirs[0] == dirent['name']:
                    cluster = dirent['first_cluster']
                    found = True
                    break
            if len(dirs) == 0:
                return files
            elif found:
                return ls_helper(dirs[1:], cluster)
            else:
                return None

        start_cluster = 2 # root sector for FAT32
        dirs = [d.upper() for d in PosixPath(name).parts[1:]]
        if not self.is_fat32: # FAT16 has fixed root directory
            files = []
            end, found = False, False
            for i in range(self.num_root_sectors): # quite a few, faster if one at a time
                e = self.sd.read_block(self.root_dir_sector + i)
                for d in [e[i:i+32] for i in range(0, len(e), 32)]:
                    dirent = self.parse_dirent(d)
                    if dirent['end']:
                        end = True
                        break
                    elif dirent['volume_name'] or dirent['hidden'] or dirent['free']:
                        continue
                    elif len(dirs) == 0: # ls of root dir
                        files.append({
                            'name': dirent['name'],
                            'dir': dirent['dir'],
                            'size': dirent['size'],
                            'first_cluster': dirent['first_cluster']
                        })
                    elif dirent['dir'] and dirs[0] == dirent['name']:
                        start_cluster = dirent['first_cluster']
                        end, found = True, True
                if end:
                    break
            if len(dirs) == 0:
                return files
            elif not found:
                return None
            dirs = dirs[1:]

        return ls_helper(dirs, start_cluster)

    def read(self, name):
        filename = PosixPath(name).parts[-1].upper()
        dirs = str(PosixPath(name).parent)
        dirents = self.ls(dirs)
        if not dirents:
            return None
        for dirent in dirents:
            if dirent['name'] == filename:
                return self.read_file_from_cluster(dirent['first_cluster'])[:dirent['size']]
        return None

    # low-level helpers
    def read_file_from_cluster(self, cluster):
        data = self.read_cluster(cluster)
        while cluster := self.next_cluster(cluster):
            data.extend(self.read_cluster(cluster))
        return bytes(data)

    def read_cluster(self, cluster):
        data = []
        for d in self.sd.read_blocks(self.data_sector + cluster * self.sectors_per_cluster, self.sectors_per_cluster):
            data.extend(d)
        return data

    def next_cluster(self, cluster):
        bpe = 4 if self.is_fat32 else 2
        eps = self.BYTES_PER_SECTOR // bpe
        s = cluster // eps
        i = (cluster % eps) * bpe
        e = struct.unpack('<I' if self.is_fat32 else '<H', bytes(self.sd.read_block(self.fat_sector + s)[i:i+bpe]))[0]
        if self.is_fat32:
            # upper 4 bits reserved
            e = e & 0x0FFFFFFF
            return e if e < 0x0FFFFFF6 else None
        else:
            return e if e < 0xFFF7 else None

    def parse_dirent(self, dirent):
        ext = ''.join(chr(c) for c in dirent[8:11]).strip()
        first_cluster = struct.unpack('<H', bytes(dirent[26:28]))[0]
        if self.is_fat32:
            first_cluster += struct.unpack('<H', bytes(dirent[20:22]))[0] << 16
        return {
            'end': dirent[0] == 0x00,
            'volume_name': bool(dirent[11] & 0x08),
            'dir': bool(dirent[11] & 0x10),
            'hidden': bool(dirent[11] & 0x02),
            'free': bool(dirent[0] == 0xe5),
            'name': ''.join(chr(c) for c in dirent[0:8]).strip() + ('.' + ext if ext else ''),
            'size': struct.unpack('<I', bytes(dirent[28:32]))[0],
            'first_cluster': first_cluster,
        }

if __name__ == '__main__':
    # for k66f_breakout
    CS_PIN  = 7
    SPI_IDX = 1

    dev = jabi.USBInterface.list_devices()[0]
    sd = SD(dev, SPI_IDX, CS_PIN)
    fat = FAT(sd)

    print(fat.ls('/'))
    print(fat.read('/a/b/c/cluster.md'))
