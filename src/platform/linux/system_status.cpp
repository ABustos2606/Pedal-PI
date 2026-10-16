// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
#include "platform/linux/system_status.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace peepo::linux_system {
namespace {
std::string contents(const std::filesystem::path& path) {
    std::ifstream file(path);
    std::ostringstream text; text << file.rdbuf();
    return text.str();
}
std::optional<double> number(const std::string& text) {
    std::istringstream in(text);
    double value = 0;
    if (!(in >> value) || !std::isfinite(value)) return {};
    return value;
}
}

std::optional<std::uint64_t> meminfo_bytes(const std::string& text, const std::string& field) {
    std::istringstream lines(text);
    for (std::string line; std::getline(lines, line);) {
        std::istringstream fields(line);
        std::string name, unit;
        std::uint64_t value = 0;
        if (!(fields >> name >> value) || name != field + ":") continue;
        fields >> unit;
        return unit == "kB" ? value * 1024 : value;
    }
    return {};
}
std::optional<double> celsius(const std::string& millidegrees) {
    const auto value = number(millidegrees);
    // Anything outside what a working sensor reports is no reading at all.
    if (!value || *value < -40000 || *value > 150000) return {};
    return *value / 1000;
}
std::optional<double> uptime_seconds(const std::string& text) {
    const auto value = number(text);
    if (!value || *value < 0) return {};
    return value;
}
Reading read(const std::string& sys, const std::string& proc) {
    namespace fs = std::filesystem;
    Reading r;
    std::error_code error;
    // The CPU's zone; on the Pi it is the only one.
    for (const auto& zone : fs::directory_iterator(fs::path(sys) / "class/thermal", error)) {
        if (zone.path().filename().string().rfind("thermal_zone", 0) != 0) continue;
        if (const auto t = celsius(contents(zone.path() / "temp")); t && (!r.temperature_c || contents(zone.path() / "type").rfind("cpu", 0) == 0)) r.temperature_c = t;
    }
    for (const auto& monitor : fs::directory_iterator(fs::path(sys) / "class/hwmon", error)) {
        if (contents(monitor.path() / "name").rfind("rpi_volt", 0) != 0) continue;
        const auto alarm = number(contents(monitor.path() / "in0_lcrit_alarm"));
        if (alarm) r.under_voltage = *alarm != 0;
    }
    const auto memory = contents(fs::path(proc) / "meminfo");
    r.memory_total = meminfo_bytes(memory, "MemTotal");
    r.memory_available = meminfo_bytes(memory, "MemAvailable");
    r.uptime_s = uptime_seconds(contents(fs::path(proc) / "uptime"));
    return r;
}
}
