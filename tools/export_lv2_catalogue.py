# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
"""Export LV2 plugin metadata from Turtle files into a C++ catalogue fixture.

Reads only port/parameter metadata (URIs, symbols, names, ranges). No upstream
code is copied and nothing is built or loaded. Requires rdflib (pip install rdflib).

    python tools/export_lv2_catalogue.py reference/upstream/lenticular_lv2 \
        > src/core/reference_catalogue.cpp
"""
import json
import pathlib
import re
import sys

import rdflib
from rdflib import RDF, Namespace

LV2 = Namespace("http://lv2plug.in/ns/lv2core#")
DOAP = Namespace("http://usefulinc.com/ns/doap#")
ATOM = Namespace("http://lv2plug.in/ns/ext/atom#")
UNITS = Namespace("http://lv2plug.in/ns/extensions/units#")
PPROPS = Namespace("http://lv2plug.in/ns/ext/port-props#")
RDFS = Namespace("http://www.w3.org/2000/01/rdf-schema#")

# Project-owned presentation defaults, keyed by a substring of the plugin URI.
FAMILIES = [
    ("polyclouds", "Texture", 0xBFA1ED),
    ("polywarps", "Modulation", 0xF0AA72),
    ("polyrings", "Resonator", 0x81C9B8),
    ("polyplaits", "Sound source", 0x7BBAED),
]
FALLBACK = ("Utility", 0x93A4B5)
NO_CONNECTORS = "This module has no audio or control connectors to patch."


def cstr(value):
    return json.dumps(str(value), ensure_ascii=True)


def number(node, fallback):
    try:
        return float(node)
    except (TypeError, ValueError):
        return fallback


def export_plugin(g, plugin):
    uri = str(plugin)
    category, color = next(((c, k) for needle, c, k in FAMILIES if needle in uri), FALLBACK)
    name = re.sub(r"^Poly\s+", "", str(g.value(plugin, DOAP.name) or uri.rsplit("/", 1)[-1]))
    audio = {"Input": [], "Output": []}
    others, parameters, symbols = [], [], set()
    for port in sorted(g.objects(plugin, LV2.port), key=lambda p: number(g.value(p, LV2["index"]), 0)):
        types = set(g.objects(port, RDF.type))
        symbol = str(g.value(port, LV2.symbol))
        symbols.add(symbol)
        direction = "Input" if LV2.InputPort in types else "Output"
        if LV2.AudioPort in types:
            audio[direction].append(symbol)
        elif LV2.CVPort in types:
            others.append((symbol, direction, "Cv"))
        elif ATOM.AtomPort in types:
            others.append((symbol, direction, "Midi"))
        elif LV2.ControlPort in types and direction == "Input":
            # The LV2 enable control is the module's software bypass, not a page parameter.
            if symbol == "enabled" or g.value(port, LV2.designation) == LV2.enabled:
                continue
            properties = set(g.objects(port, LV2.portProperty))
            low, high = number(g.value(port, LV2.minimum), 0.0), number(g.value(port, LV2.maximum), 1.0)
            if high < low:
                low, high = high, low
            value = min(max(number(g.value(port, LV2.default), low), low), high)
            points = sorted((number(g.value(p, RDF.value), 0.0), str(g.value(p, RDFS.label))) for p in g.objects(port, LV2.scalePoint))
            kind, choices = "Continuous", []
            if LV2.toggled in properties:
                kind, low, high = "Toggle", 0.0, 1.0
            elif LV2.enumeration in properties and points and [v for v, _ in points] == [float(i) for i in range(len(points))]:
                kind, low, high, choices = "Enumeration", 0.0, float(len(points) - 1), [label for _, label in points]
            elif LV2.integer in properties or LV2.enumeration in properties:
                kind, low, high = "Integer", float(round(low)), float(round(high))
            if kind != "Continuous":
                value = float(min(max(round(value), low), high))
            unit = g.value(port, UNITS.unit)
            unit = str(unit).rsplit("#", 1)[-1] if unit is not None else ""
            log = "true" if PPROPS.logarithmic in properties and low > 0 and high > low else "false"
            label = str(g.value(port, LV2.name) or symbol).replace("_", " ")
            label = label[:1].upper() + label[1:]
            parameters.append(
                f'{{{cstr(symbol)}, {cstr(label)}, {cstr(unit)}, ParameterKind::{kind}, {low!r}, {high!r}, {value!r}, {log}, '
                f'{{{", ".join(cstr(c) for c in choices)}}}}}')
    # One connector per audio port, exactly as the plugin declares them. Left and
    # right are separate ports and can be patched separately.
    ports = []
    for direction in ("Input", "Output"):
        for symbol in audio[direction]:
            ports.append(f'{{{cstr(symbol)}, Direction::{direction}, 1, PortType::Audio, {{}}}}')
    for symbol, direction, kind in others:
        ports.append(f'{{{cstr(symbol)}, Direction::{direction}, 1, PortType::{kind}, {{}}}}')
    patchable = audio["Input"] or audio["Output"] or any(kind == "Cv" for _, _, kind in others)
    availability = "Available" if patchable else "Unavailable"
    reason = "" if patchable else NO_CONNECTORS
    fragment = uri.rsplit("/", 1)[-1]
    lines = [f'        {{{cstr(uri)}, {cstr(name)}, {cstr(category)}, {cstr(fragment + " / reference metadata, not host-verified")}, {color:#08x},']
    lines.append("         {" + ",\n          ".join(ports) + "},")
    lines.append("         {" + ",\n          ".join(parameters) + "},")
    lines.append(f'         Availability::{availability}, {cstr(reason)}, true}}')
    return "\n".join(lines)


def main(roots):
    entries, sources = [], []
    for root in roots:
        for path in sorted(pathlib.Path(root).glob("*.ttl")):
            if path.name == "manifest.ttl":
                continue
            g = rdflib.Graph()
            g.parse(path, format="turtle")
            plugins = sorted(set(g.subjects(RDF.type, LV2.Plugin)))
            if plugins:
                sources.append(path.as_posix())
            entries += [export_plugin(g, plugin) for plugin in plugins]
    print("// SPDX-License-Identifier: GPL-3.0-or-later")
    print("// Copyright (C) 2026 Pedal PI contributors")
    print("// GENERATED by tools/export_lv2_catalogue.py - do not edit by hand.")
    print("// Port and parameter metadata read from unchanged upstream Turtle files:")
    for source in sources:
        print(f"//   {source}")
    print("// Metadata only. Nothing here has been built, loaded in a host or heard.")
    print('#include "core/model.hpp"')
    print()
    print("namespace pedal {")
    print("const std::vector<ModuleDefinition>& reference_catalogue() {")
    print("    static const std::vector<ModuleDefinition> catalogue = {")
    print(",\n".join(entries))
    print("    };")
    print("    return catalogue;")
    print("}")
    print("}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    sys.stdout.reconfigure(newline="\n")
    main(sys.argv[1:])
