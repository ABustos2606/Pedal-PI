# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
"""Exploration: what does the engine send, and when?"""
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ingenprobe import Client, show

t = []
c = Client(transcript=t)
print("== unsolicited messages on connect")
c.drain(0.5); show(t)
print("== get root")
print("status", c.get("ingen:/")[0]); c.drain(); show(t)
print("== get main")
print("status", c.get("ingen:/main")[0]); c.drain(); show(t)
print("== put a block")
print("status", c.put("ingen:/main/amp", "a ingen:Block ;\n\t\tlv2:prototype <http://lv2plug.in/plugins/eg-amp>")[0]); c.drain(); show(t)
print("== put a block with an unknown plugin")
print("status", c.put("ingen:/main/nope", "a ingen:Block ;\n\t\tlv2:prototype <urn:does:not:exist>")[0]); c.drain(); show(t)
print("== set gain")
print("status", c.set("ingen:/main/amp/gain", "ingen:value", "6.0")[0]); c.drain(); show(t)
print("== set out of range / wrong type")
print("status", c.set("ingen:/main/amp/gain", "ingen:value", "\"loud\"")[0]); c.drain(); show(t)
print("== get block")
print("status", c.get("ingen:/main/amp")[0]); c.drain(); show(t)
