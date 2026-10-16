// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
// The Diagnostics readings: the kernel's text as the Pi 5 wrote it, a copy of
// its files in a temporary tree, and this machine's own files.
#include "platform/linux/system_status.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace peepo::linux_system;
namespace {
void check(bool result, const char* why) { if (!result) throw std::runtime_error(why); }
void write(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path) << text;
}
}
int main() {
    try {
        const std::string meminfo = "MemTotal:        2053120 kB\nMemFree:          785776 kB\nMemAvailable:    1675728 kB\n";
        check(meminfo_bytes(meminfo, "MemTotal") == 2053120ull * 1024, "MemTotal not read in bytes");
        check(meminfo_bytes(meminfo, "MemAvailable") == 1675728ull * 1024, "MemAvailable not read");
        check(!meminfo_bytes(meminfo, "Mem") && !meminfo_bytes("", "MemTotal"), "a missing field gave a value");
        check(celsius("47950\n") == 47.95, "temperature not converted");
        check(!celsius("") && !celsius("garbage") && !celsius("999999"), "an impossible temperature was accepted");
        check(uptime_seconds("11628.07 45373.47\n") == 11628.07, "uptime not read");
        check(!uptime_seconds("-1") && !uptime_seconds(""), "an impossible uptime was accepted");

        // The Pi 5's files, with the supply too low.
        const auto root = std::filesystem::temp_directory_path() / "peepo-system-test";
        std::filesystem::remove_all(root);
        write(root / "sys/class/thermal/thermal_zone0/type", "cpu-thermal\n");
        write(root / "sys/class/thermal/thermal_zone0/temp", "47950\n");
        write(root / "sys/class/hwmon/hwmon0/name", "cpu_thermal\n");
        write(root / "sys/class/hwmon/hwmon3/name", "rpi_volt\n");
        write(root / "sys/class/hwmon/hwmon3/in0_lcrit_alarm", "1\n");
        write(root / "proc/meminfo", meminfo);
        write(root / "proc/uptime", "11628.07 45373.47\n");
        auto r = read((root / "sys").string(), (root / "proc").string());
        check(r.temperature_c == 47.95 && r.under_voltage == true && r.memory_total == 2053120ull * 1024 && r.uptime_s == 11628.07, "the Pi's files were not read");
        write(root / "sys/class/hwmon/hwmon3/in0_lcrit_alarm", "0\n");
        check(read((root / "sys").string(), (root / "proc").string()).under_voltage == false, "a recovered supply still read as low");
        // A machine without the Pi's voltage monitor says nothing about it.
        std::filesystem::remove_all(root / "sys/class/hwmon/hwmon3");
        check(!read((root / "sys").string(), (root / "proc").string()).under_voltage, "a missing voltage monitor gave a reading");
        std::filesystem::remove_all(root);
        r = read((root / "sys").string(), (root / "proc").string());
        check(!r.temperature_c && !r.memory_total && !r.uptime_s && !r.under_voltage, "missing files gave readings");

        // This machine's own files must read without failing.
        r = read();
        check(r.memory_total && r.uptime_s, "this machine's memory and uptime were not read");
        std::cout << "System status checks passed.\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
