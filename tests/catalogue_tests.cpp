// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
// Catalogue files: what is written reads back identically, a damaged or
// hostile file is refused whole, and the overlay can restrict but not grant.
//   pedal_catalogue_tests <folder holding the exported fixture and overlay>
#include "catalogue/catalogue.hpp"
#include "catalogue/json.hpp"
#include "engine/simulated_engine.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace pedal;
namespace {
void check(bool result, const char* why) { if (!result) throw std::runtime_error(why); }
std::string replaced(std::string text, const std::string& from, const std::string& to) {
    const auto at = text.find(from);
    check(at != std::string::npos, "test fixture text changed");
    return text.replace(at, from.size(), to);
}
void json_reading() {
    json::Value v;
    check(json::parse(R"( {"a": [1, -2.5e3, true, null], "b": "x\n\"\u00e9\ud83c\udfb8", "c": {}} )", v).empty(), "valid JSON refused");
    check(v.get("a")->items.size() == 4 && v.get("a")->items[1].number == -2500 && v.get("a")->items[2].boolean, "values misread");
    check(v.get("b")->text == "x\n\"\xc3\xa9\xf0\x9f\x8e\xb8" && v.get("c")->kind == json::Value::Kind::Object && !v.get("d"), "strings or members misread");
    for (const char* bad : {"{\"a\": 1,}", "[1 2]", "{\"a\": 1, \"a\": 2}", "\"open", "{\"a\": nan}", "[1] x", "\"\\ud83c\"", "\"tab\there\"", ""})
        check(!json::parse(bad, v).empty(), "invalid JSON accepted");
    check(!json::parse(std::string(100, '[') + std::string(100, ']'), v).empty(), "unbounded nesting accepted");
    check(json::quote("a\"b\\\n\x01") == "\"a\\\"b\\\\\\n\\u0001\"" && json::number(0.1) == "0.1" && json::number(-90) == "-90", "writer text wrong");
}
void round_trip() {
    CatalogueDocument written;
    for (const auto& d : demo_catalogue()) if (d.category != "Endpoint") written.modules.push_back(d);
    for (const auto& d : reference_catalogue()) written.modules.push_back(d);
    written.provenance = {{"tool", "test"}, {"lv2_path", "/usr/lib/lv2:\"quoted\""}};
    written.origins[written.modules[0].uri] = "class Distortion; bundle file:///x/";
    const auto text = write_catalogue(written);
    CatalogueDocument read;
    const auto issue = read_catalogue(text, read);
    check(issue.empty(), issue.c_str());
    check(read.modules.size() == written.modules.size() && read.provenance == written.provenance && read.origins == written.origins, "document changed in the file");
    for (std::size_t i = 0; i < read.modules.size(); ++i) {
        const auto& a = written.modules[i]; const auto& b = read.modules[i];
        check(a.uri == b.uri && a.name == b.name && a.category == b.category && a.description == b.description && a.color == b.color &&
              a.availability == b.availability && a.availability_reason == b.availability_reason && b.reference, "module changed in the file");
        check(a.ports.size() == b.ports.size() && a.parameters.size() == b.parameters.size(), "ports or parameters lost");
        for (std::size_t p = 0; p < a.ports.size(); ++p)
            check(a.ports[p].symbol == b.ports[p].symbol && a.ports[p].direction == b.ports[p].direction && a.ports[p].type == b.ports[p].type, "port changed in the file");
        for (std::size_t p = 0; p < a.parameters.size(); ++p) {
            const auto& x = a.parameters[p]; const auto& y = b.parameters[p];
            check(x.symbol == y.symbol && x.name == y.name && x.unit == y.unit && x.kind == y.kind && x.minimum == y.minimum && x.maximum == y.maximum &&
                  x.value == y.value && x.logarithmic == y.logarithmic && x.choices == y.choices, "parameter changed in the file");
        }
    }
    check(write_catalogue(read) == text, "the same catalogue wrote different bytes");

    // One bad entry refuses the file, and says which.
    CatalogueDocument untouched = read;
    auto refused = [&](const std::string& damaged, const char* why) {
        const auto message = read_catalogue(damaged, read);
        check(!message.empty() && read.modules.size() == untouched.modules.size(), why);
        return message;
    };
    refused(replaced(text, "\"pedalpi-catalogue\"", "\"something-else\""), "foreign file accepted");
    refused(replaced(text, "\"version\": 1", "\"version\": 2"), "newer file accepted");
    refused(replaced(text, "\"default\": 32", "\"default\": 320"), "default outside its range accepted");
    refused(replaced(text, "\"symbol\": \"drive\"", "\"symbol\": \"drive> ; x\""), "unusable symbol accepted");
    refused(replaced(text, "\"symbol\": \"drive\"", "\"symbol\": \"in\""), "control sharing a connector's symbol accepted");
    refused(replaced(text, "\"category\": \"Drive\"", "\"category\": \"Endpoint\""), "reserved category accepted");
    refused(replaced(text, "\"kind\": \"continuous\"", "\"kind\": \"wobbly\""), "unknown kind accepted");
    refused(replaced(text, "\"uri\": \"urn:pedalpi:demo:clouds\"", "\"uri\": \"urn:pedalpi:demo:drive\""), "repeated module accepted");
    refused(replaced(text, "\"minimum\": 0,", "\"minimum\": \"0\","), "mistyped number accepted");
    check(refused(replaced(text, "\"name\": \"Drive\",", ""), "module without a name accepted").find("urn:pedalpi:demo:drive") != std::string::npos, "fault not attributed to its module");
    refused(text.substr(0, text.size() / 2), "truncated file accepted");
}
void overlay() {
    CatalogueDocument document;
    for (const auto& d : reference_catalogue()) document.modules.push_back(d);
    document.modules[0].availability = Availability::Unavailable; document.modules[0].availability_reason = "export said no";
    auto modules = document.modules;
    const auto first = modules[0].uri, second = modules[1].uri;
    const std::string rules = R"({"format": "pedalpi-catalogue-overlay", "version": 1, "rules": [
        {"match": "polyclouds", "category": "Texture", "description": "Granular", "strip_prefix": "Mist "},
        {"match": "polyclouds", "color": "#102030"},
        {"uri": ")" + second + R"(", "name": "Renamed", "availability": "untested", "reason": "Not heard yet."},
        {"uri": ")" + first + R"(", "availability": "untested", "reason": "weaker than the export's"},
        {"uri": "urn:not:installed", "name": "Ghost"}]})";
    check(apply_catalogue_overlay(rules, modules).empty() && modules.size() == document.modules.size(), "valid overlay refused or added a module");
    for (const auto& d : modules) if (d.uri.find("polyclouds") != std::string::npos) check(d.category == "Texture" && d.color == 0x102030 && d.description == "Granular", "match rule not applied");
    check(modules[1].name == "Renamed" && modules[2].name == "Looping Delay" && modules[0].name == "Beat Repeat" && modules[1].availability == Availability::Untested && modules[1].availability_reason == "Not heard yet.", "exact rule not applied");
    check(modules[0].availability == Availability::Unavailable && modules[0].availability_reason == "export said no", "overlay weakened the export's verdict");
    const auto before = modules;
    for (const char* bad : {R"({"format": "pedalpi-catalogue-overlay", "version": 1, "rules": [{"match": "poly", "availability": "available"}]})",
                            R"({"format": "pedalpi-catalogue-overlay", "version": 1, "rules": [{"name": "No selector"}]})",
                            R"({"format": "pedalpi-catalogue-overlay", "version": 1, "rules": [{"match": "poly", "name": "Applied"}, {"match": "poly", "color": "blue"}]})",
                            R"({"format": "pedalpi-catalogue-overlay", "version": 1, "rules": [{"match": "poly", "category": "Endpoint"}]})",
                            R"({"format": "pedalpi-catalogue", "version": 1, "rules": []})"}) {
        check(!apply_catalogue_overlay(bad, modules).empty(), "invalid overlay accepted");
        for (std::size_t i = 0; i < modules.size(); ++i) check(modules[i].name == before[i].name && modules[i].availability == before[i].availability, "refused overlay was half applied");
    }
    check(category_color("Delay") == 0x7bbaed && category_color("Something new") == 0x93a4b5, "category colours changed");
}
void fixture(const std::string& folder) {
    std::vector<ModuleDefinition> offered;
    const auto issue = load_catalogue(folder + "/wsl-debian.json", folder + "/overlay.json", offered);
    check(issue.empty(), issue.c_str());
    check(!load_catalogue(folder + "/missing.json", "", offered).empty() && !offered.empty(), "missing file accepted or cleared the catalogue");
    const ModuleDefinition* amp = nullptr;
    for (const auto& d : offered) {
        if (d.uri == "http://lv2plug.in/plugins/eg-amp") amp = &d;
        bool patchable = false;
        for (const auto& port : d.ports) patchable = patchable || port.type == PortType::Audio || port.type == PortType::Cv;
        check(d.availability != Availability::Available || patchable, "an entry with nothing to patch is offered");
        check(d.availability == Availability::Available || !d.availability_reason.empty(), "a withheld entry gives no reason");
    }
    check(amp && amp->availability == Availability::Available && amp->ports.size() == 2 && amp->parameters.size() == 1, "example amplifier missing from the exported fixture");
    const auto& gain = amp->parameters[0];
    check(gain.symbol == "gain" && gain.minimum == -90 && gain.maximum == 24 && gain.value == 0 && gain.unit == "dB" && gain.kind == ParameterKind::Continuous, "exported range or unit wrong");

    // Two independent readings of the same plugins must agree: the reference
    // entries come from the upstream Turtle files through rdflib, the fixture
    // from the installed bundles through Lilv.
    unsigned compared = 0;
    for (const auto& reference : reference_catalogue()) {
        const ModuleDefinition* exported = nullptr;
        for (const auto& d : offered) if (d.uri == reference.uri) exported = &d;
        if (!exported) continue;
        ++compared;
        // The overlay renames the one module upstream misnames.
        check((exported->name == reference.name || reference.uri.ends_with("polyrings")) && exported->category == reference.category, "exported name or category differs from the reference reading");
        check(exported->ports.size() == reference.ports.size() && exported->parameters.size() == reference.parameters.size(), "exported port or control count differs from the reference reading");
        for (std::size_t i = 0; i < reference.ports.size(); ++i)
            check(exported->ports[i].symbol == reference.ports[i].symbol && exported->ports[i].direction == reference.ports[i].direction && exported->ports[i].type == reference.ports[i].type,
                  "exported connector differs from the reference reading");
        for (std::size_t i = 0; i < reference.parameters.size(); ++i) {
            const auto& x = exported->parameters[i]; const auto& y = reference.parameters[i];
            check(x.symbol == y.symbol && x.name == y.name && x.kind == y.kind && x.minimum == y.minimum && x.maximum == y.maximum && x.logarithmic == y.logarithmic &&
                  x.choices == y.choices && std::abs(x.value - y.value) < 1e-6,
                  ("exported control differs from the reference reading: " + reference.uri + " " + y.symbol + " (" + x.name + " / " + y.name + ", " +
                   std::to_string(x.minimum) + ".." + std::to_string(x.maximum) + " / " + std::to_string(y.minimum) + ".." + std::to_string(y.maximum) + ", default " +
                   std::to_string(x.value) + " / " + std::to_string(y.value) + ", kind " + std::to_string(int(x.kind)) + " / " + std::to_string(int(y.kind)) + ")").c_str());
        }
    }
    check(compared == reference_catalogue().size(), "the fixture lacks some of the Mutable-derived modules");

    // A real engine offers its own catalogue: no demo fixtures, endpoints kept.
    const auto count = offered.size();
    const auto exported = offered;
    replace_catalogue(std::move(offered));
    check(catalogue().size() == count + 2 && !definition("urn:pedalpi:demo:drive") && !definition("urn:pedalpi:demo:nam") && definition("http://lv2plug.in/plugins/eg-amp"),
          "replaced catalogue kept fixtures or lost entries");
    check(validate_graph(empty_graph("Still valid")).empty() && module_available(empty_graph("x").modules[0]), "endpoints lost with the fixtures");

    // The simulation offers the exported modules beside its demo ones, and
    // its sample patches still work.
    std::vector<ModuleDefinition> both;
    for (const auto& d : demo_catalogue()) if (d.category != "Endpoint") both.push_back(d);
    both.insert(both.end(), exported.begin(), exported.end());
    replace_catalogue(std::move(both));
    check(definition("urn:pedalpi:demo:drive") && definition("http://polyeffects.com/lv2/polyclouds#granular") && catalogue().size() == count + demo_catalogue().size(), "simulation catalogue incomplete");
    check(validate_graph(demo_graph()).empty() && validate_graph(parallel_demo_graph()).empty(), "demo patches broken by the exported catalogue");
    SimulatedEngine engine;
    engine.submit(AddModule{"http://polyeffects.com/lv2/polyclouds#granular", 240, 240});
    check(engine.advance(100)[0].accepted, "simulation could not add an exported module");
    const auto& clouds = engine.snapshot().modules.back();
    check(clouds.ports.size() == 16 && clouds.parameters.size() == 12, "exported module lost its connectors or controls in the simulation");
    engine.submit(SetParameter{clouds.id, clouds.parameters[0].symbol, clouds.parameters[0].maximum});
    engine.submit(Connect{{3, clouds.id, "out", "l_in"}});
    const auto events = engine.advance(100);
    check(events.size() == 2 && events[0].accepted && events[1].accepted, "exported module could not be edited or cabled in the simulation");
    engine.submit(AddModule{"http://drobilla.net/plugins/mda/DX10", 24, 24});
    check(!engine.advance(100)[0].accepted, "a withheld instrument was added in the simulation");
}
}
int main(int argc, char** argv) {
    try {
        json_reading();
        round_trip();
        overlay();
        fixture(argc > 1 ? argv[1] : "config/catalogue");
        std::cout << "Catalogue files: JSON reading, round trip, refusals, overlay rules and the exported fixture passed.\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
