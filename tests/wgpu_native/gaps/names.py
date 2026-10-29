# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re
import sys

FUNCTION = re.compile(r'pub (?:unsafe )?extern "C" fn (\w+)')

for path in sys.argv[1:]:
    text = open(path, encoding='utf-8').read()
    found = list(FUNCTION.finditer(text))
    for at, match in enumerate(found):
        end = found[at + 1].start() if at + 1 < len(found) else len(text)
        if 'unimplemented!' in text[match.end():end]:
            print(match.group(1))
