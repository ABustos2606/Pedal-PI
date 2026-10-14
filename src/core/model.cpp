// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#include "core/model.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <unordered_set>
#include <limits>

namespace pedal {
const std::vector<ModuleDefinition>& demo_catalogue() {
    static const std::vector<ModuleDefinition> catalogue = {
        {"urn:pedalpi:demo:input", "IN", "Endpoint", "Audio input", 0x93a4b5,
         {{"out", Direction::Output}, {"out_r", Direction::Output}}, {}},
        {"urn:pedalpi:demo:output", "OUT", "Endpoint", "Audio output", 0x93a4b5,
         {{"in", Direction::Input}, {"in_r", Direction::Input}}, {}},
        {"urn:pedalpi:demo:drive", "Drive", "Drive", "Warm saturation and tone", 0xf0aa72,
         {{"in", Direction::Input}, {"in_r", Direction::Input}, {"out", Direction::Output}, {"out_r", Direction::Output}},
         {{"drive", "Drive", "%", ParameterKind::Continuous, 0, 100, 32},
          {"tone", "Tone", "Hz", ParameterKind::Continuous, 200, 8000, 2400, true},
          {"level", "Level", "dB", ParameterKind::Continuous, -24, 12, -3}}},
        {"urn:pedalpi:demo:clouds", "Clouds", "Texture", "Granular texture playground", 0xbfa1ed,
         {{"in", Direction::Input}, {"in_r", Direction::Input}, {"out", Direction::Output}, {"out_r", Direction::Output}},
         {{"position", "Position", "%", ParameterKind::Continuous, 0, 100, 45},
          {"size", "Size", "ms", ParameterKind::Continuous, 10, 1000, 180, true},
          {"mix", "Mix", "%", ParameterKind::Continuous, 0, 100, 40},
          {"freeze", "Freeze", "", ParameterKind::Toggle, 0, 1, 0},
          {"density", "Density", "%", ParameterKind::Continuous, 0, 100, 50},
          {"pitch", "Pitch", "st", ParameterKind::Integer, -24, 24, 0}}},
        {"urn:pedalpi:demo:delay", "Delay", "Delay", "Echoes with space to breathe", 0x7bbaed,
         {{"in", Direction::Input}, {"in_r", Direction::Input}, {"out", Direction::Output}, {"out_r", Direction::Output}},
         {{"time", "Time", "ms", ParameterKind::Continuous, 20, 2000, 380, true},
          {"feedback", "Feedback", "%", ParameterKind::Continuous, 0, 95, 35},
          {"mix", "Mix", "%", ParameterKind::Continuous, 0, 100, 28},
          {"mode", "Mode", "", ParameterKind::Enumeration, 0, 2, 0, false, {"Clean", "Tape", "Ping pong"}}}},
        {"urn:pedalpi:demo:reverb", "Reverb", "Reverb", "Room, hall and a little shimmer", 0x81c9b8,
         {{"in", Direction::Input}, {"in_r", Direction::Input}, {"out", Direction::Output}, {"out_r", Direction::Output}},
         {{"decay", "Decay", "s", ParameterKind::Continuous, 0.1, 15, 3.2, true},
          {"mix", "Mix", "%", ParameterKind::Continuous, 0, 100, 25},
          {"predelay", "Pre-delay", "ms", ParameterKind::Integer, 0, 200, 20}}},
        {"urn:pedalpi:demo:mono", "Mono utility", "Utility", "Mono routing fixture / no audio", 0xa6c8ec,
         {{"in", Direction::Input, 1}, {"out", Direction::Output, 1}},
         {{"gain", "Gain", "dB", ParameterKind::Continuous, -24, 12, 0}}},
        {"urn:pedalpi:demo:nam", "NAM fixture", "Amp", "Untested catalogue example", 0xf0aa72,
         {{"in", Direction::Input, 1}, {"out", Direction::Output, 1}}, {}, Availability::Untested,
         "Simulator fixture: compatibility has not been established."},
        {"urn:pedalpi:demo:cabinet", "Cabinet fixture", "Cabinet", "Unavailable catalogue example", 0x93a4b5,
         {{"in", Direction::Input, 1}, {"out", Direction::Output, 1}}, {}, Availability::Unavailable,
         "Simulator fixture: required cabinet asset is unavailable."}
    };
    return catalogue;
}
namespace {
// A deque: registering an entry must not move the ones already handed out.
std::deque<ModuleDefinition>& entries() {
    static std::deque<ModuleDefinition> merged = [] {
        std::deque<ModuleDefinition> all(demo_catalogue().begin(), demo_catalogue().end());
        const auto& reference = reference_catalogue();
        all.insert(all.end(), reference.begin(), reference.end());
        return all;
    }();
    return merged;
}
}
const std::deque<ModuleDefinition>& catalogue() { return entries(); }
void register_definitions(std::vector<ModuleDefinition> added) {
    for (auto& d : added) if (!definition(d.uri)) entries().push_back(std::move(d));
}
void replace_catalogue(std::vector<ModuleDefinition> offered) {
    std::erase_if(entries(), [](const ModuleDefinition& d) { return d.category != "Endpoint"; });
    register_definitions(std::move(offered));
}
const ModuleDefinition* definition(const std::string& uri) {
    for (const auto& d : catalogue()) if (d.uri == uri) return &d;
    return nullptr;
}
const Module* find_module(const Graph& g, InstanceId id) {
    for (const auto& m : g.modules) if (m.id == id) return &m;
    return nullptr;
}
Module* find_module(Graph& g, InstanceId id) {
    for (auto& m : g.modules) if (m.id == id) return &m;
    return nullptr;
}
Graph demo_graph() {
    Graph g;
    const int xs[] = {32, 664, 184, 352, 520};
    const int ys[] = {132, 132, 132, 132, 132};
    for (std::size_t i = 0; i < 5; ++i) {
        const auto& d = demo_catalogue()[i];
        g.modules.push_back({i + 1, d.uri, d.name, xs[i], ys[i], i < 2, false, d.parameters, d.ports});
    }
    // Left lane first, then the right lane: every cable is one channel.
    g.connections = {{1, 3}, {3, 4}, {4, 5}, {5, 2}};
    for (std::size_t i = 0; i < 4; ++i) g.connections.push_back({g.connections[i].from, g.connections[i].to, "out_r", "in_r"});
    return g;
}
Graph empty_graph(std::string name) {
    auto g = demo_graph(); g.name = std::move(name); g.modules.resize(2); g.connections = {{1, 2}, {1, 2, "out_r", "in_r"}}; g.dirty = true;
    return g;
}
Graph parallel_demo_graph() {
    auto g = demo_graph();
    g.name = "Parallel texture"; g.dirty = true;
    const int xs[] = {24, 672, 192, 408, 408};
    const int ys[] = {132, 132, 132, 24, 240};
    for (std::size_t i = 0; i < g.modules.size(); ++i) {
        g.modules[i].x = xs[i]; g.modules[i].y = ys[i];
    }
    g.connections = {{1, 3}, {3, 4}, {3, 5}, {4, 2}, {5, 2}};
    for (std::size_t i = 0; i < 5; ++i) g.connections.push_back({g.connections[i].from, g.connections[i].to, "out_r", "in_r"});
    return g;
}
Graph missing_module_graph() {
    auto g = demo_graph();
    auto* m = find_module(g, 4); m->uri = "urn:pedalpi:missing:texture"; m->name = "Missing texture";
    g.name = "Missing module example";
    return g;
}
const std::vector<Port>& module_ports(const Module& m) {
    if (!m.ports.empty()) return m.ports;
    if (const auto* d = definition(m.uri)) return d->ports;
    static const std::vector<Port> empty; return empty;
}
bool module_available(const Module& m) {
    const auto* d = definition(m.uri);
    return d && d->availability == Availability::Available;
}
std::string validate_patch_name(const std::string& name) {
    if (name.empty() || name.find_first_not_of(' ') == std::string::npos) return "Enter a patch name.";
    if (name.size() > 96) return "Patch name is too long (maximum 96 UTF-8 bytes).";
    for (unsigned char c : name) if (c < 32 || c == 127) return "Patch name contains a control character.";
    return {};
}
bool valid_patch_id(const std::string& id) {
    return !id.empty() && id.size() <= 64 && std::all_of(id.begin(), id.end(), [](char c) { return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-'; });
}
std::string validate_parameter(const Parameter& p) {
    if (p.symbol.empty() || !std::isfinite(p.minimum) || !std::isfinite(p.maximum) || !std::isfinite(p.value) ||
        p.minimum > p.maximum || p.value < p.minimum || p.value > p.maximum) return "Invalid parameter range or value";
    if (p.logarithmic && (p.minimum <= 0 || p.maximum <= p.minimum)) return "Invalid logarithmic range";
    if (p.kind != ParameterKind::Continuous && (p.value != std::round(p.value) || p.minimum != std::round(p.minimum) ||
        p.maximum != std::round(p.maximum) || std::abs(p.minimum) > 9007199254740991.0 || std::abs(p.maximum) > 9007199254740991.0))
        return "Parameter requires exact discrete values";
    if (p.kind == ParameterKind::Toggle && (p.minimum != 0 || p.maximum != 1)) return "Invalid toggle range";
    if (p.kind == ParameterKind::Enumeration && (p.choices.empty() || p.minimum != 0 || p.maximum != p.choices.size() - 1))
        return "Invalid enumeration choices";
    return {};
}
std::string validate_connection(const Graph& g, const Connection& c, bool retain_unavailable) {
    const auto* from = find_module(g, c.from);
    const auto* to = find_module(g, c.to);
    if (!from || !to) return "Module is missing";
    if (c.from == c.to) return "Feedback loops are disabled";
    if (!retain_unavailable && (!module_available(*from) || !module_available(*to))) return "Module is unavailable; cable is inactive";
    auto port = [](const Module& m, const std::string& symbol, Direction dir) -> const Port* {
        for (const auto& p : module_ports(m)) if (p.symbol == symbol && p.direction == dir) return &p;
        return nullptr;
    };
    const auto* a = port(*from, c.output, Direction::Output); const auto* b = port(*to, c.input, Direction::Input);
    if (!a || !b) return "Connect an output to an input";
    // Audio joins audio and control joins control. The engine would also mix
    // the two; the interface keeps them apart until that is a deliberate feature.
    if (a->type != b->type) return "Audio and control connectors cannot be joined";
    if (a->type != PortType::Audio && a->type != PortType::Cv) return "Only audio and control cables are supported";
    if (a->channels < 1 || a->channels > 2 || b->channels < 1 || b->channels > 2) return "Only mono and stereo ports are supported";
    for (const auto* p : {a, b}) if (!p->channel_symbols.empty()) {
        if (p->channel_symbols.size() != p->channels) return "Channel symbols do not match channel count";
        std::unordered_set<std::string> symbols;
        for (const auto& s : p->channel_symbols) if (s.empty() || !symbols.insert(s).second) return "Invalid channel symbol";
    }
    if (std::find(g.connections.begin(), g.connections.end(), c) != g.connections.end())
        return "Cable already exists";
    std::unordered_set<InstanceId> seen;
    std::vector<InstanceId> todo{c.to};
    while (!todo.empty()) {
        const auto id = todo.back(); todo.pop_back();
        if (id == c.from) return "Feedback loops are disabled";
        if (!seen.insert(id).second) continue;
        for (const auto& edge : g.connections) if (edge.from == id) todo.push_back(edge.to);
    }
    return {};
}
std::string validate_graph(const Graph& g) {
    if (auto issue = validate_patch_name(g.name); !issue.empty()) return issue;
    std::unordered_set<InstanceId> ids;
    unsigned inputs = 0, outputs = 0;
    for (const auto& m : g.modules) {
        if (!m.id || m.id == std::numeric_limits<InstanceId>::max() || !ids.insert(m.id).second || m.uri.empty()) return "Invalid module identity";
        const auto* d = definition(m.uri);
        const bool endpoint = d && d->category == "Endpoint";
        if (m.fixed != endpoint || (m.fixed && m.bypassed)) return "Invalid fixed endpoint";
        if (m.uri == "urn:pedalpi:demo:input") ++inputs;
        if (m.uri == "urn:pedalpi:demo:output") ++outputs;
        if (m.x < 0 || m.x > canvas_width - module_size || m.y < 0 || m.y > canvas_height - module_size) return "Module outside canvas";
        std::unordered_set<std::string> symbols;
        for (const auto& p : module_ports(m)) {
            if (p.symbol.empty() || !symbols.insert(p.symbol).second || p.channels < 1 || p.channels > 2) return "Invalid port metadata";
            if (!p.channel_symbols.empty()) {
                if (p.channel_symbols.size() != p.channels) return "Channel symbols do not match channel count";
                std::unordered_set<std::string> channels;
                for (const auto& s : p.channel_symbols) if (s.empty() || !channels.insert(s).second) return "Invalid channel symbol";
            }
        }
        symbols.clear();
        for (const auto& p : m.parameters) {
            if (!symbols.insert(p.symbol).second) return "Duplicate parameter symbol";
            if (auto issue = validate_parameter(p); !issue.empty()) return issue;
        }
    }
    if (inputs != 1 || outputs != 1) return "Patch must contain one IN and one OUT endpoint";
    Graph checked = g; checked.connections.clear();
    for (const auto& c : g.connections) {
        if (auto issue = validate_connection(checked, c, true); !issue.empty()) return issue;
        checked.connections.push_back(c);
    }
    for (const auto& a : g.assignments) {
        const auto* m = find_module(g, a.module);
        if (!m || a.control.empty() || !std::isfinite(a.minimum) || !std::isfinite(a.maximum) || a.minimum > a.maximum) return "Invalid control assignment";
        const auto it = std::find_if(m->parameters.begin(), m->parameters.end(), [&](const Parameter& p) { return p.symbol == a.parameter; });
        if (it == m->parameters.end() || a.minimum < it->minimum || a.maximum > it->maximum) return "Invalid assignment parameter range";
    }
    return {};
}
RoutingPlan expand_connection(const Graph& g, const Connection& c) {
    // Existing cables may be expanded; ignore just this edge for duplicate checking.
    auto probe = g; std::erase(probe.connections, c);
    if (auto issue = validate_connection(probe, c); !issue.empty()) return {{}, issue};
    const auto& a = module_ports(*find_module(g, c.from)); const auto& b = module_ports(*find_module(g, c.to));
    const auto& output = *std::find_if(a.begin(), a.end(), [&](const Port& p) { return p.symbol == c.output; });
    const auto& input = *std::find_if(b.begin(), b.end(), [&](const Port& p) { return p.symbol == c.input; });
    auto symbol = [](const Port& p, unsigned channel) {
        if (!p.channel_symbols.empty()) return p.channel_symbols[channel];
        return p.channels == 1 ? p.symbol : p.symbol + (channel == 0 ? "_L" : "_R");
    };
    RoutingPlan result;
    const unsigned count = std::max(output.channels, input.channels);
    for (unsigned i = 0; i < count; ++i)
        result.connections.push_back({c.from, c.to, symbol(output, output.channels == 1 ? 0 : i), symbol(input, input.channels == 1 ? 0 : i)});
    return result;
}
double normalized_value(const Parameter& p) {
    if (p.maximum <= p.minimum) return 0;
    const auto v = std::clamp(p.value, p.minimum, p.maximum);
    if (p.logarithmic && p.minimum > 0)
        return std::log(v / p.minimum) / std::log(p.maximum / p.minimum);
    return (v - p.minimum) / (p.maximum - p.minimum);
}
double value_from_normalized(const Parameter& p, double n) {
    n = std::clamp(n, 0.0, 1.0);
    auto value = p.logarithmic && p.minimum > 0
        ? p.minimum * std::pow(p.maximum / p.minimum, n)
        : p.minimum + n * (p.maximum - p.minimum);
    if (p.kind != ParameterKind::Continuous) value = std::round(value);
    return std::clamp(value, p.minimum, p.maximum);
}
std::string format_value(const Parameter& p) {
    if (p.kind == ParameterKind::Toggle) return p.value >= 0.5 ? "On" : "Off";
    if (p.kind == ParameterKind::Enumeration && p.value >= 0 && p.value < p.choices.size())
        return p.choices[static_cast<std::size_t>(p.value)];
    std::ostringstream out;
    // Narrow ranges, such as 0-1 plugin controls, need decimals to show movement.
    const int digits = p.unit == "s" ? 1 : p.kind == ParameterKind::Continuous && p.maximum - p.minimum <= 10 ? 2 : 0;
    out << std::fixed << std::setprecision(digits) << p.value;
    if (!p.unit.empty()) out << " " << p.unit;
    return out.str();
}
}
