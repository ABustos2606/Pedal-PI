// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
#pragma once
#include "controls/controls.hpp"
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

// The physical controls on Linux, read from the kernel's input devices. On the
// Pi the kernel's own drivers make them: each rotary-encoder overlay reports
// relative turns on its own axis, and each gpio-key overlay a key code of its
// own (see config/pi/config.txt). Bindings name those codes, so nothing
// depends on device numbering, and a device that comes and goes is found again.
namespace peepo::linux_input {
struct Binding {
    std::string control;
    enum class Kind { Turn, Key } kind = Kind::Key;
    unsigned code = 0;
    bool reverse = false;
    // Only this device, by its kernel name; empty for any.
    std::string device;
};
// One binding per line: `control turn|key CODE [reverse] [device=NAME]`, with
// `#` comments. CODE is a number or a name such as REL_X or KEY_F13.
std::string parse_bindings(std::string_view text, std::vector<Binding>&);
std::optional<ControlEvent> translate(const std::vector<Binding>&, const std::string& device, unsigned type, unsigned code, int value);

class EvdevControls {
public:
    // `exclude`: device paths never opened here, such as the touch screen.
    explicit EvdevControls(std::vector<Binding>, std::string directory = "/dev/input", std::set<std::string> exclude = {});
    ~EvdevControls();
    EvdevControls(const EvdevControls&) = delete;
    EvdevControls& operator=(const EvdevControls&) = delete;
    // Read whatever has arrived, without waiting. Looks for new devices every
    // two seconds, and drops one that has gone.
    std::vector<ControlEvent> poll(unsigned milliseconds);
    // Names of the devices in use.
    std::vector<std::string> devices() const;
    // Forces the next poll to look for devices at once.
    void rescan() { since_scan_ = rescan_ms; }
    static constexpr unsigned rescan_ms = 2000;
private:
    struct Device { int fd; std::string path, name; };
    void scan();
    std::vector<Binding> bindings_;
    std::string directory_;
    std::set<std::string> exclude_;
    std::vector<Device> devices_;
    unsigned since_scan_ = rescan_ms;
};
// The first device under `directory` that reports absolute touch positions.
std::optional<std::string> find_touchscreen(const std::string& directory = "/dev/input");
}
