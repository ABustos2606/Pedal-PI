// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
// What the Diagnostics page shows about the machine: temperature, memory,
// the supply voltage and uptime, read from the kernel's files without
// running a program. A reading the kernel does not offer stays unknown.
#pragma once
#include <cstdint>
#include <optional>
#include <string>

namespace peepo::linux_system {
struct Reading {
    std::optional<double> temperature_c;
    std::optional<std::uint64_t> memory_total, memory_available;
    // The Pi's under-voltage alarm: true while the supply is too low.
    std::optional<bool> under_voltage;
    std::optional<double> uptime_s;
};
// One field of /proc/meminfo, in bytes.
std::optional<std::uint64_t> meminfo_bytes(const std::string& text, const std::string& field);
// A thermal zone's millidegrees as degrees Celsius.
std::optional<double> celsius(const std::string& millidegrees);
// The first number of /proc/uptime.
std::optional<double> uptime_seconds(const std::string& text);
// The machine's readings now. `sys` and `proc` are the roots, for tests.
Reading read(const std::string& sys = "/sys", const std::string& proc = "/proc");
}
