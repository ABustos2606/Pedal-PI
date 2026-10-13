#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
# Save deadlock: the same request with and without a NUL after the message.
. "$(dirname "$0")/env.sh"
cd "$(dirname "$0")"
for mode in nul newline; do
start_engine || exit 1
python3 - "$mode" <<'PY'
import os, subprocess, sys, time
sys.path.insert(0, os.getcwd())
from ingenprobe import Client
mode = sys.argv[1]
bundle = "/tmp/pedalpi-spike/%s.ingen" % mode; subprocess.run(["rm", "-rf", bundle])
c = Client(terminator=b"\n\0" if mode == "nul" else b"\n")
print("%-8s put block: %s" % (mode, c.put("ingen:/main/amp", "a ingen:Block ;\n\t\tlv2:prototype <http://lv2plug.in/plugins/eg-amp>")[0]))
start = time.time()
status = c.copy("ingen:/main", "file://" + bundle, timeout=4)[0]
print("%-8s save reply: %s after %.2f s; files: %s" % (mode, status, time.time() - start, sorted(os.listdir(bundle)) if os.path.isdir(bundle) else "none"))
print("%-8s next request: %s" % (mode, c.get("ingen:/main", timeout=3)[0]), flush=True)
PY
done
stop_engine
