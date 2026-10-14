// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace pedal {
using InstanceId = std::uint64_t;
// Logical canvas extent; viewport movement never changes module positions.
inline constexpr int canvas_width = 4096, canvas_height = 2048;
inline constexpr int module_size = 96;
inline constexpr int module_min_x = 24, module_min_y = 24;
inline constexpr int module_max_x = 3984, module_max_y = 1920;
enum class ParameterKind { Continuous, Integer, Toggle, Enumeration };
struct Parameter {
    std::string symbol, name, unit;
    ParameterKind kind = ParameterKind::Continuous;
    double minimum = 0, maximum = 1, value = 0;
    bool logarithmic = false;
    std::vector<std::string> choices;
};
enum class Direction { Input, Output };
enum class PortType { Audio, Control, Midi, Cv };
struct Port {
    std::string symbol;
    Direction direction;
    unsigned channels = 1;
    PortType type = PortType::Audio;
    std::vector<std::string> channel_symbols;
};
enum class Availability { Available, Untested, Unavailable };
struct ModuleDefinition {
    std::string uri, name, category, description;
    std::uint32_t color;
    std::vector<Port> ports;
    std::vector<Parameter> parameters;
    Availability availability = Availability::Available;
    std::string availability_reason;
    // Generated from upstream LV2 metadata, rather than an invented demo fixture.
    bool reference = false;
    // The plugin's own on/off control, when it has one. Such a module is
    // silent until that control is set, and bypass means clearing it.
    std::string enable_symbol;
};
struct Module {
    InstanceId id;
    std::string uri, name;
    int x, y;
    bool fixed = false, bypassed = false;
    std::vector<Parameter> parameters;
    // Retain a snapshot of ports, including when the plugin is unavailable.
    std::vector<Port> ports;
};
struct Connection {
    InstanceId from, to;
    std::string output = "out", input = "in";
    bool operator==(const Connection&) const = default;
};
struct Assignment {
    std::string control;
    InstanceId module;
    std::string parameter;
    double minimum, maximum;
};
struct Graph {
    std::string name = "First light";
    // The stored patch this graph was loaded from or last saved as; empty
    // when it has never been saved. Independent of the editable name.
    std::string patch_id;
    std::vector<Module> modules;
    std::vector<Connection> connections;
    std::vector<Assignment> assignments;
    bool dirty = false;
};
struct PhysicalConnection {
    InstanceId from, to;
    std::string output, input;
    bool operator==(const PhysicalConnection&) const = default;
};
struct RoutingPlan { std::vector<PhysicalConnection> connections; std::string error; };
const std::vector<ModuleDefinition>& demo_catalogue();
const std::vector<ModuleDefinition>& reference_catalogue();
// Demo fixtures, the generated reference entries, then registered ones.
const std::deque<ModuleDefinition>& catalogue();
// Add entries an engine adapter can vouch for. Call before the interface
// starts; an entry whose URI is already known is ignored.
void register_definitions(std::vector<ModuleDefinition>);
// Offer exactly these modules, as a real engine's exported catalogue does:
// the demo and reference entries go, the IN and OUT endpoints stay. Call
// before the interface starts.
void replace_catalogue(std::vector<ModuleDefinition>);
const ModuleDefinition* definition(const std::string& uri);
const Module* find_module(const Graph&, InstanceId);
Module* find_module(Graph&, InstanceId);
Graph demo_graph();
Graph parallel_demo_graph();
Graph empty_graph(std::string name);
Graph missing_module_graph();
const std::vector<Port>& module_ports(const Module&);
bool module_available(const Module&);
std::string validate_patch_name(const std::string&);
// Stored patch identities: 1-64 lowercase letters, digits and hyphens.
bool valid_patch_id(const std::string&);
std::string validate_parameter(const Parameter&);
std::string validate_graph(const Graph&);
std::string validate_connection(const Graph&, const Connection&, bool retain_unavailable = false);
RoutingPlan expand_connection(const Graph&, const Connection&);
double normalized_value(const Parameter&);
double value_from_normalized(const Parameter&, double);
std::string format_value(const Parameter&);
}
