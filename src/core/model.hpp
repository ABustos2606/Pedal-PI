// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace peepo {
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
    // silent until that control is set, so it stays set; bypass is the
    // engine's, which passes the input through.
    std::string enable_symbol;
    // Controls whose default the project chose over the plugin's own. The
    // plugin starts at its own value, so adding the module sets these.
    std::vector<std::string> project_defaults;
    // Keep legacy identities loadable without offering duplicates in Add.
    bool hidden = false;
    // The plugin's own control for its output level, shown as Output. A module
    // with audio outputs and none of its own gets the general Output instead;
    // "none" leaves a pass-through module, such as a tuner, without either.
    std::string output_symbol;
};
// The general Output: not a plugin port but the block's own level, which the
// engine applies to its audio outputs (urn:peepo:ns#outputLevel, in dB).
inline constexpr const char* general_output_symbol = "peepo_output";
Parameter general_output_parameter();
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
// A physical control pinned to something in this patch: an encoder to a
// parameter, a stomp switch to a parameter or to a module's software bypass.
enum class ControlAction { Parameter, Bypass };
// How a stomp switch drives a parameter: flip it on each press, or hold the
// top of the range only while pressed.
enum class SwitchMode { Toggle, Momentary };
struct Assignment {
    std::string control;
    InstanceId module = 0;
    std::string parameter;
    // The part of the parameter's range the control covers.
    double minimum = 0, maximum = 1;
    bool inverted = false;
    ControlAction action = ControlAction::Parameter;
    SwitchMode mode = SwitchMode::Toggle;
    // The target's plugin when assigned. A different module that later holds
    // the same identity is never controlled by mistake.
    std::string plugin;
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
// Physical control identities, such as enc1 or stomp3: the same characters.
bool valid_control_id(const std::string&);
// The module an assignment controls, or null when it is orphaned: its module
// is gone, holds another plugin, or lacks the parameter.
const Module* assignment_target(const Graph&, const Assignment&);
const Parameter* assignment_parameter(const Graph&, const Assignment&);
// An assignment as one engine property value, naming its block by symbol (m3).
std::string encode_assignment(const Assignment&, const std::string& block);
// Back again; the module is left 0 and the block symbol returned through `block`.
bool decode_assignment(const std::string& control, const std::string& text, Assignment&, std::string& block);
std::string validate_parameter(const Parameter&);
std::string validate_graph(const Graph&);
std::string validate_connection(const Graph&, const Connection&, bool retain_unavailable = false);
RoutingPlan expand_connection(const Graph&, const Connection&);
double normalized_value(const Parameter&);
double value_from_normalized(const Parameter&, double);
std::string format_value(const Parameter&);
}
