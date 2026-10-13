# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
"""Minimal raw-socket client for probing Ingen's Turtle protocol.

Deliberately dumb: it frames messages, keeps a transcript, and correlates
replies by patch:sequenceNumber. No RDF library, so what is printed is exactly
what crosses the socket.
"""
import re
import socket
import time

SOCKET_PATH = "/tmp/pedalpi-ingen.sock"
PREFIXES = """@prefix atom: <http://lv2plug.in/ns/ext/atom#> .
@prefix ingen: <http://drobilla.net/ns/ingen#> .
@prefix lv2: <http://lv2plug.in/ns/lv2core#> .
@prefix patch: <http://lv2plug.in/ns/ext/patch#> .
@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .
@prefix pedalpi: <urn:pedalpi:ns#> .
"""


RESPONSE = re.compile(r'a patch:Response ;\s*patch:sequenceNumber "(\d+)"\^\^xsd:int ;(?:\s*patch:subject <[^>]*> ;)?\s*patch:body "(-?\d+)"')


class Client:
    def __init__(self, path=SOCKET_PATH, transcript=None, terminator=b"\n"):
        self.terminator = terminator
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.connect(path)
        self.buffer = b""
        self.seq = 0
        self.transcript = transcript
        self.sent_prefixes = False

    def close(self):
        self.sock.close()

    def log(self, direction, text):
        if self.transcript is not None:
            self.transcript.append((direction, text))

    def send_raw(self, text):
        if not self.sent_prefixes:
            text = PREFIXES + text
            self.sent_prefixes = True
        self.log(">", text)
        # No NUL after the message. The engine's reader holds its RDF lock while
        # it waits for the byte after whatever it last consumed; a trailing NUL
        # leaves it parked there with the lock held, and a save then deadlocks
        # the whole event pipeline. Exactly one byte after the final '.' avoids it.
        self.sock.sendall(text.rstrip().encode() + self.terminator)

    def read_message(self, timeout=2.0):
        """One NUL-terminated message, or None on timeout."""
        deadline = time.monotonic() + timeout
        while b"\0" not in self.buffer:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                return None
            self.sock.settimeout(remaining)
            try:
                chunk = self.sock.recv(65536)
            except socket.timeout:
                return None
            if not chunk:
                raise ConnectionError("engine closed the socket")
            self.buffer += chunk
        message, self.buffer = self.buffer.split(b"\0", 1)
        text = message.decode(errors="replace")
        self.log("<", text)
        return text

    def drain(self, quiet=0.3):
        out = []
        while (m := self.read_message(quiet)) is not None:
            out.append(m)
        return out

    def request(self, body, timeout=3.0):
        """Send one patch message; return (status, reply_text, other_messages).

        `body` is the predicate list after `[] a patch:X ;`, without the final dot.
        status is the integer patch:body of the patch:Response, or None on timeout.
        """
        self.seq += 1
        self.send_raw("[]\n\t%s ;\n\tpatch:sequenceNumber \"%d\"^^xsd:int .\n" % (body.strip().rstrip(";."), self.seq))
        others = []
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            m = self.read_message(deadline - time.monotonic())
            if m is None:
                break
            # One socket message can bundle several replies, and the engine's own
            # sequence numbers share the predicate, so match Response blocks only.
            statuses = [int(status) for number, status in RESPONSE.findall(m) if int(number) == self.seq]
            if statuses:
                return max(statuses, key=abs), m, others
            others.append(m)
        return None, "", others

    # Convenience wrappers ------------------------------------------------
    def get(self, subject, **kw):
        return self.request("a patch:Get ;\n\tpatch:subject <%s>" % subject, **kw)

    def put(self, subject, body, **kw):
        return self.request("a patch:Put ;\n\tpatch:subject <%s> ;\n\tpatch:body [\n\t\t%s\n\t]" % (subject, body), **kw)

    def set(self, subject, key, value, **kw):
        return self.request("a patch:Set ;\n\tpatch:subject <%s> ;\n\tpatch:property %s ;\n\tpatch:value %s" % (subject, key, value), **kw)

    def connect(self, tail, head, parent="ingen:/main/", **kw):
        return self.request("a patch:Put ;\n\tpatch:subject <%s> ;\n\tpatch:body [\n\t\ta ingen:Arc ;\n\t\tingen:tail <%s> ;\n\t\tingen:head <%s>\n\t]" % (parent, tail, head), **kw)

    def disconnect(self, tail, head, **kw):
        return self.request("a patch:Delete ;\n\tpatch:body [\n\t\ta ingen:Arc ;\n\t\tingen:tail <%s> ;\n\t\tingen:head <%s>\n\t]" % (tail, head), **kw)

    def delete(self, subject, **kw):
        return self.request("a patch:Delete ;\n\tpatch:subject <%s>" % subject, **kw)

    def copy(self, subject, destination, **kw):
        return self.request("a patch:Copy ;\n\tpatch:subject <%s> ;\n\tpatch:destination <%s>" % (subject, destination), **kw)


def show(transcript, limit=4000):
    for direction, text in transcript:
        body = text if len(text) <= limit else text[:limit] + "\n... (%d more bytes)" % (len(text) - limit)
        for line in body.rstrip().splitlines():
            print(direction, line)
        print(direction, "<NUL>")
    transcript.clear()
