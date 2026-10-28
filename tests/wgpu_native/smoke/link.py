# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import os
import subprocess
import sys

for name in ('ZIG_GLOBAL_CACHE_DIR', 'ZIG_LOCAL_CACHE_DIR'):
    os.environ[name] = os.path.abspath(os.environ[name])
command = [os.path.abspath(arg) if os.path.exists(arg) else arg for arg in sys.argv[2:]]
os.chdir(sys.argv[1])
sys.exit(subprocess.call(command))
