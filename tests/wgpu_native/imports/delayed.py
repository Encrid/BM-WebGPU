# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import struct
import sys

RVA = 0x1000
RAW = 0x200
OPTIONAL = 0x58
SECTION = OPTIONAL + 0xF0
IMPORTS = int(sys.argv[2], 16) if len(sys.argv) > 2 else RVA


def section():
    data = bytearray(0x100)
    struct.pack_into('<5I', data, 0x00, 0, 0, 0, RVA + 0x80, 0)
    struct.pack_into('<8I', data, 0x28, 1, RVA + 0x90, 0, 0, 0, 0, 0, 0)
    data[0x80:0x8C] = b'kernel32.dll'
    data[0x90:0x99] = b'other.dll'
    return bytes(data)


def headers():
    data = bytearray(RAW)
    data[0:2] = b'MZ'
    struct.pack_into('<I', data, 0x3C, 0x40)
    data[0x40:0x44] = b'PE\0\0'
    struct.pack_into('<HHIIIHH', data, 0x44, 0x8664, 1, 0, 0, 0, 0xF0, 0x2022)
    struct.pack_into('<H', data, OPTIONAL, 0x20B)
    struct.pack_into('<II', data, OPTIONAL + 32, 0x1000, 0x200)
    struct.pack_into('<II', data, OPTIONAL + 56, 0x2000, RAW)
    struct.pack_into('<I', data, OPTIONAL + 108, 16)
    struct.pack_into('<II', data, OPTIONAL + 112 + 8, IMPORTS, 40)
    struct.pack_into('<II', data, OPTIONAL + 112 + 13 * 8, RVA + 0x28, 64)
    data[SECTION:SECTION + 6] = b'.rdata'
    struct.pack_into('<IIII', data, SECTION + 8, 0x100, RVA, 0x200, RAW)
    struct.pack_into('<I', data, SECTION + 36, 0x40000040)
    return bytes(data)


with open(sys.argv[1], 'wb') as out:
    out.write(headers() + section() + bytes(0x100))
