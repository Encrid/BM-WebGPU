# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import sys
from pathlib import Path

SYSTEM = {'bcryptprimitives.dll', 'combase.dll', 'd3d12.dll', 'dxgi.dll', 'gdi32.dll', 'kernel32.dll', 'ntdll.dll',
          'oleaut32.dll', 'opengl32.dll', 'rpcrt4.dll', 'setupapi.dll', 'user32.dll', 'userenv.dll', 'ws2_32.dll'}
DIRECTORIES = {0x10B: 96, 0x20B: 112}
TABLES = ((1, 20, 12), (13, 32, 4))
USAGE = 'usage: python tools/imports.py <library> [<needed> ...]'


def read_imports(path):
    data = Path(path).read_bytes() if Path(path).is_file() else b''

    def number(at, size=4):
        return int.from_bytes(data[at:at + size], 'little')
    pe = number(0x3C)
    if data[:2] != b'MZ' or data[pe:pe + 4] != b'PE\0\0':
        return None
    optional = pe + 24
    first = optional + number(pe + 20, 2)
    sections = [(number(at + 8), number(at + 12), number(at + 20))
                for at in range(first, first + 40 * number(pe + 6, 2), 40)]

    def offset(rva):
        return next(raw + rva - start for length, start, raw in sections if start <= rva < start + length)
    directories = optional + DIRECTORIES[number(optional, 2)]
    names = []
    for index, size, field in TABLES:
        rva = number(directories + 8 * index)
        at = offset(rva) if rva else None
        while at is not None and number(at + field):
            name = offset(number(at + field))
            names.append(data[name:data.index(b'\0', name)].decode('ascii').lower())
            at += size
    return list(dict.fromkeys(names))


def is_system(name):
    return name in SYSTEM or name.startswith('api-ms-win-')


def main():
    if len(sys.argv) < 2:
        print(USAGE)
        return 2
    library, needed = sys.argv[1], {name.lower() for name in sys.argv[2:]}
    names = read_imports(library)
    if names is None:
        print('imports: %s: no PE file' % library)
        return 1
    other = [name for name in names if name not in needed and not is_system(name)]
    for name in other:
        print('imports: %s: %s is not a library of the system' % (library, name))
    if other:
        return 1
    for name in names:
        if name in needed:
            print('imports: %s: %s is a stated need' % (library, name))
    print('imports: %s: %d libraries of the system' % (library, len(set(names) - needed)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
