// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
#pragma once
#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace peepo {
inline constexpr std::size_t euclidean_tracks = 4;
inline constexpr int euclidean_max_steps = 32;
using EuclideanSteps = std::array<std::optional<int>, euclidean_tracks>;

inline bool euclidean_uri(const std::string& uri) {
    return uri == "http://polyeffects.com/lv2/euclidean";
}

// Euclidean rhythm distribution matching Poly Euclidean (poly_euclidean.c):
// distributes `beats` across `steps` (0..32) and rotates clockwise by `shift`.
inline std::vector<bool> euclidean_pattern(int steps, int beats, int shift) {
    steps = std::clamp(steps, 0, euclidean_max_steps);
    beats = std::clamp(beats, 0, euclidean_max_steps);
    shift = std::clamp(shift, 0, euclidean_max_steps);
    if (steps <= 0) return {};
    std::vector<bool> raw(static_cast<std::size_t>(steps), false);
    int bucket = 0;
    for (int i = 0; i < steps; ++i) {
        bucket += beats;
        if (bucket >= steps) {
            bucket -= steps;
            raw[static_cast<std::size_t>(steps - 1 - i)] = true;
        }
    }
    std::vector<bool> shifted(static_cast<std::size_t>(steps), false);
    for (int i = 0; i < steps; ++i) {
        shifted[static_cast<std::size_t>((i + shift) % steps)] = raw[static_cast<std::size_t>(i)];
    }
    return shifted;
}
}
