# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import sys

DELAY_DIRECTORY = {0x10B: 200, 0x20B: 216}
ENTRY = 32


def word(data, at):
    return int.from_bytes(data[at:at + 2], 'little')


def dword(data, at):
    return int.from_bytes(data[at:at + 4], 'little')


def file_offset(data, table, count, rva):
    for at in range(table, table + count * 40, 40):
        virtual_size, virtual_address = dword(data, at + 8), dword(data, at + 12)
        if virtual_address <= rva < virtual_address + virtual_size:
            return rva - virtual_address + dword(data, at + 20)
    sys.exit('no section holds the RVA %#x' % rva)


def delayed_names(data):
    signature = dword(data, 0x3C)
    if data[signature:signature + 4] != b'PE\0\0':
        sys.exit('no PE signature')
    coff = signature + 4
    optional = coff + 20
    table = optional + word(data, coff + 16)
    count = word(data, coff + 2)
    directory = dword(data, optional + DELAY_DIRECTORY[word(data, optional)])
    names = []
    at = file_offset(data, table, count, directory) if directory else None
    while at is not None and any(data[at:at + ENTRY]):
        name = file_offset(data, table, count, dword(data, at + 4))
        names.append(data[name:data.index(b'\0', name)].decode('ascii'))
        at += ENTRY
    return names


for name in delayed_names(open(sys.argv[1], 'rb').read()):
    print(name)
