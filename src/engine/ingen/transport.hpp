// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace peepo::ingen {
// A byte stream to the engine whose blocking work happens elsewhere: no call
// here waits on the network, so the thread that draws never does either.
class Transport {
public:
    struct Event {
        enum class Kind { Opened, Data, Closed };
        Kind kind;
        // Received bytes, or why the connection ended.
        std::string data;
    };
    virtual ~Transport() = default;
    // Start connecting; Opened or Closed reports the outcome.
    virtual void open() = 0;
    // Drop the connection. Nothing more is reported for it.
    virtual void close() = 0;
    virtual void send(std::string) = 0;
    virtual std::vector<Event> poll() = 0;
};
// unix:///path (not on Windows) or tcp://host:port. A reader that falls
// `max_buffered` bytes behind ends the connection instead of growing without
// bound. The engine writes to its clients from one loop, so a client that
// stops reading delays every other client's replies: the reader thread never
// waits for the interface to catch up.
std::unique_ptr<Transport> socket_transport(std::string address, std::size_t max_buffered);
}
