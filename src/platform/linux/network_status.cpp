// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
#include "platform/linux/network_status.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ifaddrs.h>
// The kernel's own if.h comes with wireless.h; glibc's net/if.h would clash with it.
#include <linux/wireless.h>
#include <map>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace peepo::linux_network {
namespace {
std::string signal_word(int dbm) { return dbm >= -60 ? "good" : dbm >= -70 ? "fair" : "weak"; }
// The joined Wi-Fi network through the wireless extensions; empty if none.
std::string ssid(int fd, const std::string& interface) {
    char name[IW_ESSID_MAX_SIZE + 1] = {};
    iwreq request{};
    std::strncpy(request.ifr_name, interface.c_str(), IFNAMSIZ - 1);
    request.u.essid.pointer = name;
    request.u.essid.length = IW_ESSID_MAX_SIZE;
    if (ioctl(fd, SIOCGIWESSID, &request) != 0) return {};
    return std::string(name, std::min<std::size_t>(request.u.essid.length, IW_ESSID_MAX_SIZE)).c_str();
}
}

std::optional<int> wireless_level(const std::string& proc_text, const std::string& interface) {
    std::istringstream lines(proc_text);
    for (std::string line; std::getline(lines, line);) {
        std::istringstream fields(line);
        std::string name, status, quality, level;
        if (!(fields >> name >> status >> quality >> level) || name != interface + ":") continue;
        try {
            const int value = std::stoi(level);
            // Older drivers report the level as an unsigned byte.
            return value > 63 ? value - 256 : value;
        } catch (const std::exception&) { return std::nullopt; }
    }
    return std::nullopt;
}

std::string describe(const std::vector<Link>& links) {
    if (links.empty()) return "Not connected";
    std::string text;
    for (const auto& link : links) {
        if (!text.empty()) text += "\n";
        text += link.wireless ? "Wi-Fi " + (link.ssid.empty() ? link.interface : link.ssid) : "Wired " + link.interface;
        text += " / " + (link.address.empty() ? std::string("no address yet") : link.address);
        if (link.wireless && link.level_dbm) text += " / signal " + signal_word(*link.level_dbm) + " (" + std::to_string(*link.level_dbm) + " dBm)";
    }
    return text;
}

std::vector<Link> links() {
    std::map<std::string, std::string> addresses;
    if (ifaddrs* list = nullptr; getifaddrs(&list) == 0) {
        for (auto* a = list; a; a = a->ifa_next) {
            if (!a->ifa_addr || a->ifa_addr->sa_family != AF_INET || (a->ifa_flags & IFF_LOOPBACK)) continue;
            char address[INET_ADDRSTRLEN] = {};
            inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(a->ifa_addr)->sin_addr, address, sizeof address);
            addresses.emplace(a->ifa_name, address);
        }
        freeifaddrs(list);
    }
    std::ifstream proc("/proc/net/wireless");
    std::ostringstream wireless; wireless << proc.rdbuf();
    const int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    std::vector<Link> found;
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator("/sys/class/net", error)) {
        Link link;
        link.interface = entry.path().filename().string();
        if (link.interface == "lo") continue;
        link.wireless = std::filesystem::exists(entry.path() / "wireless", error);
        if (const auto it = addresses.find(link.interface); it != addresses.end()) link.address = it->second;
        if (link.wireless && fd >= 0) link.ssid = ssid(fd, link.interface);
        if (link.wireless && !link.ssid.empty()) link.level_dbm = wireless_level(wireless.str(), link.interface);
        if (!link.address.empty() || !link.ssid.empty()) found.push_back(std::move(link));
    }
    if (fd >= 0) close(fd);
    std::sort(found.begin(), found.end(), [](const Link& a, const Link& b) { return a.interface < b.interface; });
    return found;
}
}
