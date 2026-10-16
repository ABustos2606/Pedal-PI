// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
// The network line in Settings: the Wi-Fi signal read from the kernel's text,
// how links are described, and reading this machine's links without failing.
#include "platform/linux/network_status.hpp"
#include <iostream>
#include <stdexcept>

using namespace peepo::linux_network;
namespace {
void check(bool result, const char* why) { if (!result) throw std::runtime_error(why); }
}
int main() {
    try {
        // As the Pi 5 reported it on the guest Wi-Fi.
        const std::string proc =
            "Inter-| sta-|   Quality        |   Discarded packets               | Missed | WE\n"
            " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n"
            " wlan0: 0000   55.  -55.  -256        0      0      0      0      0        0\n"
            " wlan1: 0000   20.  190.  0           0      0      0      0      0        0\n";
        check(wireless_level(proc, "wlan0") == -55, "the Pi's signal level was not read");
        check(wireless_level(proc, "wlan1") == -66, "an unsigned level was not converted to dBm");
        check(!wireless_level(proc, "wlan2") && !wireless_level(proc, "wlan") && !wireless_level("", "wlan0"), "a missing interface gave a level");

        check(describe({}) == "Not connected", "no links not reported");
        check(describe({{"wlan0", "192.168.10.157", "Camarones-invitades-2.4", true, -55}}) ==
              "Wi-Fi Camarones-invitades-2.4 / 192.168.10.157 / signal good (-55 dBm)", "joined Wi-Fi described wrongly");
        check(describe({{"wlan0", "", "Home", true, -75}}) == "Wi-Fi Home / no address yet / signal weak (-75 dBm)", "Wi-Fi without an address described wrongly");
        check(describe({{"eth0", "10.0.0.2", "", false, {}}, {"wlan0", "10.0.0.3", "Home", true, -65}}) ==
              "Wired eth0 / 10.0.0.2\nWi-Fi Home / 10.0.0.3 / signal fair (-65 dBm)", "two links described wrongly");

        // Whatever this machine has, reading it must not fail or list loopback.
        for (const auto& link : links()) check(link.interface != "lo" && (!link.address.empty() || !link.ssid.empty()), "a listed link has no address or network");
        std::cout << "Network status checks passed.\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
