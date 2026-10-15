// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#pragma once
#include <cmath>
#include <optional>
#include <string>

namespace pedal {
inline bool tuner_uri(const std::string& uri) {
    return uri == "http://gareus.org/oss/lv2/tuna#mod" || uri == "http://gareus.org/oss/lv2/tuna#one" ||
           uri == "http://gareus.org/oss/lv2/tuna#two";
}
struct Tuning { std::string note; double cents, frequency; };
inline std::optional<Tuning> tuning_from_frequency(std::optional<double> frequency, double reference) {
    if (!frequency || !std::isfinite(*frequency) || *frequency < 20 || *frequency > 10000 ||
        !std::isfinite(reference) || reference < 220 || reference > 880) return {};
    const double pitch = 69 + 12 * std::log2(*frequency / reference);
    const int midi = static_cast<int>(std::round(pitch));
    if (midi < 0 || midi > 127) return {};
    static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return Tuning{std::string(names[midi % 12]) + std::to_string(midi / 12 - 1), (pitch - midi) * 100, *frequency};
}
}
