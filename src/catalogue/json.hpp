// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#pragma once
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Just enough JSON for the project's own generated files: strict, bounded in
// depth, and with no dependency to pin on two platforms.
namespace pedal::json {
struct Value {
    enum class Kind { Null, Boolean, Number, String, Array, Object };
    Kind kind = Kind::Null;
    bool boolean = false;
    double number = 0;
    std::string text;
    std::vector<Value> items;
    // In file order; a repeated key is an error when parsing.
    std::vector<std::pair<std::string, Value>> members;
    const Value* get(std::string_view key) const;
};
// Returns an empty string, or what was wrong and where.
std::string parse(std::string_view text, Value& out);
std::string quote(std::string_view text);
// Shortest text that reads back as the same double.
std::string number(double value);
}
