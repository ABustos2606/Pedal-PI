# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
"""Delete the last block of a plugin and load it again at once (patch 0005).

The engine creates blocks in its pre-processor and frees deleted ones in its
post-processor, after the delete has been answered. Freeing the last instance
of a plugin closes its library; without patch 0005 that could happen while a
block of the same plugin was being created, and the engine crashed. Loading a
patch that uses the plugins of the one it replaces does exactly this, so each
round deletes a block and, without waiting, loads a saved bundle holding the
same plugin. Needs a running engine (env.sh):
    python3 16-instance-race.py [rounds]
"""
import os
import shutil
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ingenprobe import Client

PLUGINS = ["http://polyeffects.com/lv2/basic_quantizer", "http://drobilla.net/plugins/mda/Overdrive",
           "http://polyeffects.com/lv2/polyclouds#granular", "http://lv2plug.in/plugins/eg-amp"]
WORK = "/tmp/peepo-race"


def alive():
    return subprocess.run(["pgrep", "-x", "ingen"], capture_output=True).returncode == 0


def main():
    rounds = int(sys.argv[1]) if len(sys.argv) > 1 else 200
    shutil.rmtree(WORK, ignore_errors=True)
    os.makedirs(WORK)
    c = Client()
    # One saved graph per plugin, holding a single block of it.
    for i, uri in enumerate(PLUGINS):
        c.put("ingen:/main/x", "a ingen:Block ;\n\t\tlv2:prototype <%s>" % uri)
        c.copy("ingen:/main", "file://%s/%d.ingen" % (WORK, i), timeout=10)
        c.delete("ingen:/main/x")
    start = time.monotonic()
    for n in range(rounds):
        i = n % len(PLUGINS)
        c.put("ingen:/main/x", "a ingen:Block ;\n\t\tlv2:prototype <%s>" % PLUGINS[i])
        # No waiting from here: the delete's instance is freed while the load
        # is already instantiating the same plugin.
        c.send_raw("[] a patch:Delete ; patch:subject <ingen:/main/x> .\n")
        c.send_raw("[] a patch:Copy ; patch:subject <file://%s/%d.ingen> ; patch:destination <ingen:/main/r> .\n" % (WORK, i))
        time.sleep(0.05)
        c.delete("ingen:/main/r")
        c.drain(0.01)
        if not alive():
            print("FAIL  the engine crashed in round %d of %d (%s)" % (n + 1, rounds, PLUGINS[i]))
            sys.exit(1)
    time.sleep(1.0)
    ok = alive() and c.get("ingen:/main")[0] == 0
    print("%s  %d rounds of deleting a block and loading the same plugin from a bundle in %.1f s; the engine %s"
          % ("PASS" if ok else "FAIL", rounds, time.monotonic() - start, "still answers" if ok else "did not survive"))
    sys.exit(0 if ok else 1)


main()
