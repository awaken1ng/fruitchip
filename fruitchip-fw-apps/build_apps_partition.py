from pathlib import Path
import struct
import enum
import argparse
import math

SECTOR_SIZE = 4096
SECTOR_ERASE_VALUE = b'\xFF'

MAGIC1 = B'APPS'
MAGIC2 = 0x4FB80AB4
VERSION = 2


class Attribute(enum.Flag):
    DISABLE_NEXT_OSDSYS_HOOK = enum.auto()


apps = [
    ('PS2BBL',  Path('../ps2bbl/bin/COMPRESSED_PS2BBL.ELF'),            Attribute(0)),
]


crc32_lut = [
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
]


def crc32(data):
    crc = 0xFFFFFFFF

    for i in range(len(data)):
        crc ^= data[i] << 24
        crc = (crc << 4) ^ crc32_lut[crc >> 28]
        crc &= 0xFFFFFFFF
        crc = (crc << 4) ^ crc32_lut[crc >> 28]
        crc &= 0xFFFFFFFF

    return crc


def calculate_padding_for_len(data_len: int):
    padding_len = 0
    if data_len % SECTOR_SIZE != 0:
        padding_len = (math.ceil(data_len / SECTOR_SIZE) * SECTOR_SIZE) - data_len

    return padding_len


def pack_entry(data: bytes, attrs: int):
    entry = bytearray()
    entry += struct.pack('<II', len(data), attrs)
    entry += data
    entry += struct.pack('<I', crc32(data))

    entry_len = len(entry)
    entry_padding_len = calculate_padding_for_len(entry_len)
    entry.extend(SECTOR_ERASE_VALUE * entry_padding_len)

    return entry


def main(out_path: Path):
    entries = []

    app_names = bytearray()
    for (name, _, _) in apps:
        name_encoded = name.encode('ascii')
        app_name = struct.pack(f'< B {len(name_encoded)}s', len(name_encoded), name_encoded)
        app_names += app_name
    app_names = struct.pack('<B', len(apps)) + app_names
    app_names_entry = pack_entry(app_names, 0)
    entries.append(app_names_entry)

    for (_, path, attrs) in apps:
        app_data = path.read_bytes()
        entries.append(pack_entry(app_data, attrs.value))

    hdr = struct.pack('< 4s I I I', MAGIC1, MAGIC2, VERSION, len(entries))

    lookup_table_len = 2 * 4 * len(entries)
    hdr_len = len(hdr) + lookup_table_len
    hdr_padding_len = calculate_padding_for_len(hdr_len)

    offset = hdr_len + hdr_padding_len
    lookup_table = bytearray()
    for entry in entries:
        lookup_table += struct.pack('<II', offset, len(entry))
        offset += len(entry)

    with open(out_path, 'wb') as out:
        out.write(hdr)
        out.write(lookup_table)
        out.write(SECTOR_ERASE_VALUE * hdr_padding_len)
        for entry in entries:
            out.write(entry)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('out_path', type=Path)
    args = parser.parse_args()

    main(args.out_path)
