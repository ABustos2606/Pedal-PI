// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
// How the instrument is reached: its network links, their addresses and the
// Wi-Fi network and signal, read from the kernel without running a program
// or waiting on the network.
#pragma once
#include <optional>
#include <string>
#include <vector>

namespace peepo::linux_network {
struct Link {
    std::string interface, address, ssid;
    bool wireless = false;
    std::optional<int> level_dbm;
};
// The signal level of one interface in /proc/net/wireless, in dBm.
std::optional<int> wireless_level(const std::string& proc_text, const std::string& interface);
// One line per link, or "Not connected" when there is none.
std::string describe(const std::vector<Link>&);
// Links that have an address or have joined a Wi-Fi network.
std::vector<Link> links();
}
