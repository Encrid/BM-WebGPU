# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re
import sys
from pathlib import Path

DECLARED = re.compile(r'^\s*(?:WGPU_EXPORT\s+)?(?P<result>[\w \t*]+?)\s*\b(?P<name>wgpu[A-Z]\w*)'
                      r'\((?P<parameters>[^)]*)\)(?:\s*WGPU_FUNCTION_ATTRIBUTE)?\s*;\s*$')
DEFINED = re.compile(r'^\w[\w \t*]*?\b(wgpu[A-Z]\w*)\(', re.MULTILINE)
LAST_NAME = re.compile(r'(\w+)\s*$')


def declarations(headers):
    for header in headers:
        for line in Path(header).read_text(encoding='utf-8').splitlines():
            found = DECLARED.match(line)
            if found:
                yield found.group('result').strip(), found.group('name'), found.group('parameters').strip()


def call(result, name, parameters):
    names = [] if parameters == 'void' else [LAST_NAME.search(part).group(1) for part in parameters.split(',')]
    real = '((%s (*)(%s))find_real("%s"))(%s)' % (result, parameters, name, ', '.join(names))
    body = '    %s;' % real if result == 'void' else '    return (%s);' % real
    return '%s %s(%s)\n{\n%s\n}\n' % (result, name, parameters, body)


def main(output, inputs):
    sources = [path for path in inputs if path.endswith('.c')]
    headers = [path for path in inputs if path.endswith('.h')]
    written = {name for path in sources for name in DEFINED.findall(Path(path).read_text(encoding='utf-8'))}
    parts = ['#include "%s"\n' % Path(path).resolve().as_posix() for path in sources]
    for result, name, parameters in declarations(headers):
        if name not in written:
            written.add(name)
            parts.append(call(result, name, parameters))
    Path(output).write_text('\n'.join(parts), encoding='utf-8', newline='\n')


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2:])
