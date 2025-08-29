from pathlib import Path
import struct
import binascii
import enum
import argparse


MAGIC1 = B'APPS'
MAGIC2 = 0x4FB80AB4
VERSION = 1


class Attribute(enum.Flag):
    DISABLE_NEXT_OSDSYS_HOOK = enum.auto()


apps = [
    ('PS2BBL',  Path('ps2bbl/bin/COMPRESSED_PS2BBL.ELF'),            Attribute(0)),
]


def pack_entry(data: bytes, attrs: int):
    entry = bytearray()
    entry += struct.pack('<II', len(data), attrs)
    entry += data
    entry += struct.pack('<I', binascii.crc32(data))

    return entry


def main(out_path: Path):
    entries = []

    app_names = bytearray()
    for (name, _, _) in apps:
        name_encoded = name.encode('ascii')
        app_name = struct.pack(f'< B {len(name_encoded)}s', len(name_encoded), name_encoded)
        app_names += app_name
    app_names = struct.pack('<B', len(apps)) + app_names
    entries.append(pack_entry(app_names, 0))

    for (_, path, attrs) in apps:
        app_data = path.read_bytes()
        entries.append(pack_entry(app_data, attrs.value))

    hdr = struct.pack('< 4s I I I', MAGIC1, MAGIC2, VERSION, len(entries))

    lookup_table_len = 2 * 4 * len(entries)
    offset = len(hdr) + lookup_table_len
    lookup_table = bytearray()
    for entry in entries:
        lookup_table += struct.pack('<II', offset, len(entry))
        offset += len(entry)

    with open(out_path, 'wb') as out:
        out.write(hdr)
        out.write(lookup_table)
        for entry in entries:
            out.write(entry)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('out_path', type=Path)
    args = parser.parse_args()

    main(args.out_path)
