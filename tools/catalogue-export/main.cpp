// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
// Export the installed LV2 plugins as a Pedal PI catalogue file. Linux only.
// Uses Lilv, as the engine does, so discovery follows the same LV2_PATH,
// manifests and plugin classes. Metadata only: no plugin is loaded or run.
//
//   LV2_PATH=/usr/lib/lv2 pedal_catalogue_export -o catalogue.json
//   pedal_catalogue_export --exclude eg-scope --sample-rate 48000 > catalogue.json
#include "catalogue/catalogue.hpp"
#include <lilv/lilv.h>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sys/utsname.h>

using namespace pedal;
namespace {
constexpr char lv2[] = "http://lv2plug.in/ns/lv2core#";
constexpr char pprops[] = "http://lv2plug.in/ns/ext/port-props#";
constexpr char units[] = "http://lv2plug.in/ns/extensions/units#";
// What the engine offers a plugin: reference/upstream/ingen src/World.cpp and
// src/server/Engine.cpp. Ingen ignores any plugin that requires something else.
constexpr const char* engine_features[] = {
    "http://lv2plug.in/ns/ext/urid#map", "http://lv2plug.in/ns/ext/urid#unmap", "http://lv2plug.in/ns/ext/instance-access",
    "http://lv2plug.in/ns/ext/data-access", "http://lv2plug.in/ns/ext/log#log", "http://lv2plug.in/ns/ext/worker#schedule",
    "http://lv2plug.in/ns/ext/options#options", "http://lv2plug.in/ns/ext/buf-size#powerOf2BlockLength",
    "http://lv2plug.in/ns/ext/buf-size#fixedBlockLength", "http://lv2plug.in/ns/ext/buf-size#boundedBlockLength",
    "http://lv2plug.in/ns/ext/state#loadDefaultState"};
// LV2 plugin classes to the interface's categories. A class not listed takes
// its parent's; the overlay can still move any module.
constexpr std::pair<const char*, const char*> categories[] = {
    {"DelayPlugin", "Delay"}, {"ReverbPlugin", "Reverb"}, {"DistortionPlugin", "Drive"}, {"WaveshaperPlugin", "Drive"},
    {"SimulatorPlugin", "Amp"}, {"AmplifierPlugin", "Utility"}, {"ModulatorPlugin", "Modulation"}, {"ChorusPlugin", "Modulation"}, {"FlangerPlugin", "Modulation"},
    {"PhaserPlugin", "Modulation"}, {"FilterPlugin", "Filter"}, {"EQPlugin", "Filter"}, {"DynamicsPlugin", "Dynamics"},
    {"CompressorPlugin", "Dynamics"}, {"LimiterPlugin", "Dynamics"}, {"GatePlugin", "Dynamics"}, {"ExpanderPlugin", "Dynamics"},
    {"PitchPlugin", "Pitch"}, {"GeneratorPlugin", "Sound source"}, {"InstrumentPlugin", "Sound source"}, {"OscillatorPlugin", "Sound source"}};
constexpr std::pair<const char*, const char*> unit_symbols[] = {
    {"db", "dB"}, {"hz", "Hz"}, {"khz", "kHz"}, {"mhz", "MHz"}, {"s", "s"}, {"ms", "ms"}, {"min", "min"}, {"pc", "%"}, {"coef", ""},
    {"bpm", "BPM"}, {"semitone12TET", "st"}, {"cent", "ct"}, {"oct", "oct"}, {"degree", "deg"}, {"midiNote", "note"}, {"bar", "bars"}, {"beat", "beats"}};

struct Free { void operator()(LilvNode* node) const { lilv_node_free(node); } void operator()(LilvNodes* nodes) const { lilv_nodes_free(nodes); } };
using Node = std::unique_ptr<LilvNode, Free>;
using Nodes = std::unique_ptr<LilvNodes, Free>;
struct Terms {
    explicit Terms(LilvWorld* world) : world(world) {}
    Node uri(const std::string& text) const { return Node(lilv_new_uri(world, text.c_str())); }
    LilvWorld* world;
    Node input = uri(std::string(lv2) + "InputPort"), output = uri(std::string(lv2) + "OutputPort"), audio = uri(std::string(lv2) + "AudioPort"),
         control = uri(std::string(lv2) + "ControlPort"), cv = uri(std::string(lv2) + "CVPort"), atom = uri("http://lv2plug.in/ns/ext/atom#AtomPort"),
         optional = uri(std::string(lv2) + "connectionOptional"), toggled = uri(std::string(lv2) + "toggled"), integer = uri(std::string(lv2) + "integer"),
         enumeration = uri(std::string(lv2) + "enumeration"), sample_rate = uri(std::string(lv2) + "sampleRate"), designation = uri(std::string(lv2) + "designation"),
         enabled = uri(std::string(lv2) + "enabled"), free_wheeling = uri(std::string(lv2) + "freeWheeling"), logarithmic = uri(std::string(pprops) + "logarithmic"),
         not_on_gui = uri(std::string(pprops) + "notOnGUI"), trigger = uri(std::string(pprops) + "trigger"), unit = uri(std::string(units) + "unit"),
         unit_symbol = uri(std::string(units) + "symbol");
};
std::string text_of(const LilvNode* node) { return node && lilv_node_as_string(node) ? lilv_node_as_string(node) : ""; }
// The number as the plugin's metadata writes it. Lilv's own accessors return
// 32-bit floats, which would turn 0.9 into 0.8999999761581421 in the file.
double number_of(const LilvNode* node) {
    const auto text = text_of(node);
    double value = std::nan("");
    if (node && (lilv_node_is_float(node) || lilv_node_is_int(node))) {
        const auto* begin = text.data() + (text.starts_with('+') ? 1 : 0);
        if (std::from_chars(begin, text.data() + text.size(), value).ec != std::errc{}) value = std::nan("");
    }
    return value;
}
std::string local_name(const std::string& uri) { return uri.substr(uri.find_last_of("#/") + 1); }
// Cut to a byte limit without splitting a UTF-8 character, and without control characters.
std::string tidy(std::string text, std::size_t limit) {
    std::erase_if(text, [](unsigned char c) { return c < 0x20 || c == 0x7f; });
    if (text.size() > limit) {
        text.resize(limit);
        while (!text.empty() && (static_cast<unsigned char>(text.back()) & 0xc0) == 0x80) text.pop_back();
        if (!text.empty() && static_cast<unsigned char>(text.back()) >= 0xc0) text.pop_back();
    }
    return text;
}
bool usable_symbol(const std::string& text) {
    if (text.empty() || text.size() > 256 || (text[0] >= '0' && text[0] <= '9')) return false;
    return std::all_of(text.begin(), text.end(), [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; });
}
// The category of the nearest listed class, and whether any class on the way
// up is the instrument class.
std::string category_of(const Terms& terms, const LilvPlugin* plugin, bool& instrument) {
    const auto* classes = lilv_world_get_plugin_classes(terms.world);
    const auto* current = lilv_plugin_get_class(plugin);
    std::string found;
    for (int depth = 0; current && depth < 8; ++depth) {
        const auto name = local_name(text_of(lilv_plugin_class_get_uri(current)));
        instrument = instrument || name == "InstrumentPlugin";
        for (const auto& [lv2_class, category] : categories) if (found.empty() && name == lv2_class) found = category;
        const auto* parent = lilv_plugin_class_get_parent_uri(current);
        current = parent ? lilv_plugin_classes_get_by_uri(classes, parent) : nullptr;
    }
    return found.empty() ? "Utility" : found;
}
std::string unit_of(const Terms& terms, const LilvPlugin* plugin, const LilvPort* port) {
    const Nodes found(lilv_port_get_value(plugin, port, terms.unit.get()));
    if (!found || lilv_nodes_size(found.get()) == 0) return {};
    const auto* unit = lilv_nodes_get_first(found.get());
    if (lilv_node_is_uri(unit)) {
        const auto name = local_name(text_of(unit));
        for (const auto& [lv2_unit, symbol] : unit_symbols) if (name == lv2_unit) return symbol;
    }
    // A unit the table does not know, or one the plugin defines itself.
    const Node symbol(lilv_world_get(terms.world, unit, terms.unit_symbol.get(), nullptr));
    return tidy(text_of(symbol.get()), 16);
}
struct Exported { ModuleDefinition definition; std::string origin; unsigned hidden = 0; };
Exported export_plugin(const Terms& terms, const LilvPlugin* plugin, double rate) {
    Exported out;
    auto& d = out.definition;
    d.uri = text_of(lilv_plugin_get_uri(plugin));
    const Node name(lilv_plugin_get_name(plugin));
    d.name = tidy(text_of(name.get()), 96);
    if (d.name.empty()) d.name = tidy(local_name(d.uri), 96);
    bool instrument = false;
    d.category = category_of(terms, plugin, instrument);
    d.color = category_color(d.category);
    d.reference = true;
    auto label = text_of(lilv_plugin_class_get_label(lilv_plugin_get_class(plugin)));
    if (label.ends_with(" Plugin")) label.resize(label.size() - 7);
    const Node author(lilv_plugin_get_author_name(plugin));
    d.description = tidy((label.empty() || label == "Plugin" ? std::string("LV2 plugin") : label) + (author ? " by " + text_of(author.get()) : ""), 200);
    out.origin = tidy("class " + (label.empty() ? std::string("Plugin") : label) + "; bundle " + text_of(lilv_plugin_get_bundle_uri(plugin)), 2000);

    std::string blocked;
    const Nodes required(lilv_plugin_get_required_features(plugin));
    LILV_FOREACH (nodes, i, required.get()) {
        const auto feature = text_of(lilv_nodes_get(required.get(), i));
        if (std::none_of(std::begin(engine_features), std::end(engine_features), [&](const char* known) { return feature == known; }) && blocked.empty())
            blocked = "Needs an LV2 feature the engine does not provide: " + local_name(feature);
    }
    const auto count = lilv_plugin_get_num_ports(plugin);
    std::vector<Port> audio_in, audio_out, others;
    for (uint32_t index = 0; index < count; ++index) {
        const auto* port = lilv_plugin_get_port_by_index(plugin, index);
        auto is = [&](const Node& kind) { return lilv_port_is_a(plugin, port, kind.get()); };
        auto has = [&](const Node& property) { return lilv_port_has_property(plugin, port, property.get()); };
        const auto symbol = text_of(lilv_port_get_symbol(plugin, port));
        if (!usable_symbol(symbol)) { if (blocked.empty()) blocked = "A port has a symbol the engine cannot address."; continue; }
        const bool input = is(terms.input);
        const auto direction = input ? Direction::Input : Direction::Output;
        // One connector per audio port, as the plugin declares them: left and
        // right are separate ports and can be patched separately.
        if (is(terms.audio)) (input ? audio_in : audio_out).push_back({symbol, direction, 1, PortType::Audio, {}});
        else if (is(terms.cv)) others.push_back({symbol, direction, 1, PortType::Cv, {}});
        else if (is(terms.atom)) others.push_back({symbol, direction, 1, PortType::Midi, {}});
        else if (!is(terms.control)) { if (!has(terms.optional) && blocked.empty()) blocked = "Has a port of a type the engine does not handle: " + symbol; }
        else if (input) {
            const Nodes designations(lilv_port_get_value(plugin, port, terms.designation.get()));
            const bool switched = designations && (lilv_nodes_contains(designations.get(), terms.enabled.get()) || lilv_nodes_contains(designations.get(), terms.free_wheeling.get()));
            // The enable control is the module's bypass, not a page parameter;
            // a control marked as not for interfaces is left to the plugin.
            const bool enables = (designations && lilv_nodes_contains(designations.get(), terms.enabled.get())) || symbol == "enabled";
            if (enables && d.enable_symbol.empty()) d.enable_symbol = symbol;
            if (switched || enables || has(terms.not_on_gui)) { ++out.hidden; continue; }
            Parameter parameter;
            parameter.symbol = symbol;
            const Node port_name(lilv_port_get_name(plugin, port));
            parameter.name = text_of(port_name.get());
            if (parameter.name.empty()) parameter.name = symbol;
            std::replace(parameter.name.begin(), parameter.name.end(), '_', ' ');
            if (parameter.name[0] >= 'a' && parameter.name[0] <= 'z') parameter.name[0] = static_cast<char>(parameter.name[0] - 'a' + 'A');
            parameter.name = tidy(parameter.name, 96);
            parameter.unit = unit_of(terms, plugin, port);
            LilvNode *default_node = nullptr, *minimum_node = nullptr, *maximum_node = nullptr;
            lilv_port_get_range(plugin, port, &default_node, &minimum_node, &maximum_node);
            const Node owned_default(default_node), owned_minimum(minimum_node), owned_maximum(maximum_node);
            const double stated_low = number_of(minimum_node), stated_high = number_of(maximum_node), stated_default = number_of(default_node);
            double low = std::isfinite(stated_low) ? stated_low : 0.0, high = std::isfinite(stated_high) ? stated_high : 1.0;
            if (has(terms.sample_rate)) { low *= rate; high *= rate; }
            if (high < low) std::swap(low, high);
            double value = std::isfinite(stated_default) ? stated_default * (has(terms.sample_rate) ? rate : 1.0) : low;
            std::vector<std::pair<double, std::string>> points;
            if (auto* scale = lilv_port_get_scale_points(plugin, port)) {
                LILV_FOREACH (scale_points, i, scale) {
                    const auto* point = lilv_scale_points_get(scale, i);
                    points.emplace_back(number_of(lilv_scale_point_get_value(point)), tidy(text_of(lilv_scale_point_get_label(point)), 96));
                }
                lilv_scale_points_free(scale);
            }
            std::sort(points.begin(), points.end());
            bool counted = !points.empty();
            for (std::size_t i = 0; i < points.size(); ++i) counted = counted && points[i].first == static_cast<double>(i) && !points[i].second.empty();
            if (has(terms.toggled) || has(terms.trigger)) { parameter.kind = ParameterKind::Toggle; low = 0; high = 1; }
            else if (has(terms.enumeration) && counted) {
                parameter.kind = ParameterKind::Enumeration; low = 0; high = static_cast<double>(points.size() - 1);
                for (auto& point : points) parameter.choices.push_back(std::move(point.second));
            } else if (has(terms.integer) || has(terms.enumeration)) { parameter.kind = ParameterKind::Integer; low = std::round(low); high = std::round(high); }
            if (parameter.kind != ParameterKind::Continuous) value = std::round(value);
            parameter.minimum = low; parameter.maximum = high;
            parameter.value = std::clamp(value, low, high);
            parameter.logarithmic = has(terms.logarithmic) && low > 0 && high > low;
            // The reader refuses the whole file for one bad parameter, so a
            // plugin with unusable metadata is marked here, not exported broken.
            if (auto issue = validate_parameter(parameter); !issue.empty()) { if (blocked.empty()) blocked = "Control " + symbol + " has unusable metadata."; continue; }
            d.parameters.push_back(std::move(parameter));
        }
    }
    const bool patchable = !audio_in.empty() || !audio_out.empty() || std::any_of(others.begin(), others.end(), [](const Port& p) { return p.type == PortType::Cv; });
    d.ports = std::move(audio_in);
    d.ports.insert(d.ports.end(), audio_out.begin(), audio_out.end());
    d.ports.insert(d.ports.end(), others.begin(), others.end());
    if (blocked.empty() && !patchable) blocked = "This module has no audio or control connectors to patch.";
    if (!blocked.empty()) { d.availability = Availability::Unavailable; d.availability_reason = tidy(blocked, 500); }
    // Loadable, but silent: cables carry audio and control, not notes yet.
    else if (instrument) { d.availability = Availability::Untested; d.availability_reason = "Played by MIDI notes, which cannot be routed yet."; }
    return out;
}
}
int main(int argc, char** argv) {
    std::string output;
    std::vector<std::string> excluded;
    double rate = 48000;
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "-o" && i + 1 < argc) output = argv[++i];
        else if (option == "--exclude" && i + 1 < argc) excluded.push_back(argv[++i]);
        else if (option == "--sample-rate" && i + 1 < argc) rate = std::atof(argv[++i]);
        else { std::cerr << "usage: pedal_catalogue_export [-o file] [--exclude uri-substring]... [--sample-rate hz]\n"; return 2; }
    }
    if (!(rate >= 8000 && rate <= 768000)) { std::cerr << "sample rate out of range\n"; return 2; }
    std::unique_ptr<LilvWorld, void (*)(LilvWorld*)> world(lilv_world_new(), lilv_world_free);
    lilv_world_load_all(world.get());
    const Terms terms(world.get());
    CatalogueDocument document;
    unsigned available = 0, hidden = 0, skipped = 0;
    const auto* plugins = lilv_world_get_all_plugins(world.get());
    LILV_FOREACH (plugins, i, plugins) {
        const auto* plugin = lilv_plugins_get(plugins, i);
        const auto uri = text_of(lilv_plugin_get_uri(plugin));
        // A file: address names one machine's path (Ingen's own graph
        // templates are such); it would mean nothing on another.
        const bool local = uri.starts_with("file:");
        if (local || std::any_of(excluded.begin(), excluded.end(), [&](const std::string& part) { return uri.find(part) != std::string::npos; })) {
            ++skipped;
            std::cerr << "skipped " << uri << (local ? " (machine-local address)" : " (excluded)") << '\n';
            continue;
        }
        auto exported = export_plugin(terms, plugin, rate);
        hidden += exported.hidden;
        if (exported.definition.availability == Availability::Available) ++available;
        else std::cerr << "withheld " << uri << ": " << exported.definition.availability_reason << '\n';
        document.origins[uri] = std::move(exported.origin);
        document.modules.push_back(std::move(exported.definition));
    }
    std::sort(document.modules.begin(), document.modules.end(), [](const ModuleDefinition& a, const ModuleDefinition& b) { return a.uri < b.uri; });
    utsname machine{};
    uname(&machine);
    const char* path = std::getenv("LV2_PATH");
    document.provenance = {{"tool", "pedal_catalogue_export"}, {"lilv", PEDAL_LILV_VERSION}, {"machine", machine.machine},
                           {"lv2_path", path ? path : "(Lilv default)"}, {"sample_rate", std::to_string(static_cast<long>(rate))}};
    const auto text = write_catalogue(document);
    // What is written must be what the interface will accept.
    CatalogueDocument check;
    if (auto issue = read_catalogue(text, check); !issue.empty()) { std::cerr << "export does not read back: " << issue << '\n'; return 1; }
    if (output.empty()) std::cout << text;
    else if (!(std::ofstream(output, std::ios::binary) << text)) { std::cerr << "cannot write " << output << '\n'; return 1; }
    std::cerr << document.modules.size() << " modules exported, " << available << " available, " << document.modules.size() - available
              << " withheld; " << hidden << " controls left to the plugin, " << skipped << " plugins skipped\n";
    return 0;
}
