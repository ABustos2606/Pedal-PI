// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
// Does the exported catalogue describe what the engine actually builds? Loads
// every available module in a running Ingen through the adapter, compares the
// engine's ports with the catalogue's, moves every control across its range
// and removes the module again. Replaces whatever patch the engine holds.
//   peepo_catalogue_live unix:///tmp/peepo-ingen.sock catalogue.json [overlay.json [keep-uri]]
// With a fourth argument, that module is added once more at the end and left
// cabled between IN and OUT on the left side, for a sound measurement.
#include "catalogue/catalogue.hpp"
#include "engine/ingen/ingen_engine.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <set>
#include <thread>
#include <tuple>

using namespace peepo;
namespace {
using Clock = std::chrono::steady_clock;
struct Client {
    explicit Client(const std::string& address) : engine(ingen::Options{address}) {}
    std::vector<EngineEvent> step() {
        const auto now = Clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
        if (elapsed > 0) last = now;
        return engine.advance(static_cast<unsigned>(elapsed));
    }
    bool ready(unsigned milliseconds = 5000) {
        const auto until = Clock::now() + std::chrono::milliseconds(milliseconds);
        while (engine.state() != EngineState::Ready && Clock::now() < until) { step(); std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
        return engine.state() == EngineState::Ready;
    }
    EngineEvent result(Command command) {
        const auto start = Clock::now();
        const auto id = engine.submit(std::move(command));
        while (Clock::now() - start < std::chrono::seconds(6)) {
            for (auto& event : step()) if (event.request == id) return event;
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
        return {id, false, "no result within six seconds", engine.state(), {}, {}};
    }
    IngenEngine engine;
    Clock::time_point last = Clock::now();
};
using Connector = std::tuple<std::string, Direction, PortType>;
std::set<Connector> connectors(const std::vector<Port>& ports) {
    std::set<Connector> all;
    for (const auto& port : ports) all.emplace(port.symbol, port.direction, port.type);
    return all;
}
std::string describe(const Connector& c) {
    static const char* const types[] = {"audio", "control", "midi", "cv"};
    return std::get<0>(c) + (std::get<1>(c) == Direction::Input ? " (input, " : " (output, ") + types[static_cast<int>(std::get<2>(c))] + ")";
}
}
int main(int argc, char** argv) {
    if (argc < 3) { std::cerr << "usage: peepo_catalogue_live address catalogue.json [overlay.json]\n"; return 2; }
    std::vector<ModuleDefinition> offered;
    if (const auto issue = load_catalogue(argv[2], argc > 3 ? argv[3] : "", offered); !issue.empty()) { std::cerr << issue << std::endl; return 2; }
    const auto modules = offered;
    replace_catalogue(std::move(offered));
    Client client(argv[1]);
    client.engine.connect();
    if (!client.ready()) { std::cout << "FAIL  connect to the engine -- " << argv[1] << std::endl; return 1; }
    if (const auto event = client.result(NewPatch{"Catalogue check"}); !event.accepted) { std::cout << "FAIL  clear the engine graph -- " << event.message << std::endl; return 1; }

    unsigned passed = 0, failed = 0, withheld = 0;
    for (const auto& d : modules) {
        if (d.availability != Availability::Available) {
            ++withheld;
            std::cout << "SKIP  " << d.name << " <" << d.uri << "> -- " << d.availability_reason << std::endl;
            continue;
        }
        std::string problem, note;
        const auto added = client.result(AddModule{d.uri, 240, 240});
        const Module* made = nullptr;
        for (const auto& m : client.engine.snapshot().modules) if (!m.fixed && m.uri == d.uri) made = &m;
        if (!added.accepted || !made) problem = "engine did not load it: " + added.message;
        else {
            const auto id = made->id;
            const auto expected = connectors(d.ports), actual = connectors(made->ports);
            for (const auto& c : expected) if (!actual.contains(c)) problem += (problem.empty() ? "" : "; ") + std::string("engine lacks ") + describe(c);
            for (const auto& c : actual) if (!expected.contains(c)) problem += (problem.empty() ? "" : "; ") + std::string("catalogue lacks ") + describe(c);
            if (made->bypassed) problem += (problem.empty() ? "" : "; ") + std::string("it starts switched off");
            if (made->parameters.size() != d.parameters.size())
                problem += (problem.empty() ? "" : "; ") + std::to_string(d.parameters.size()) + " controls in the catalogue, " + std::to_string(made->parameters.size()) + " in the engine";
            unsigned moved = 0;
            for (std::size_t i = 0; i < d.parameters.size() && i < made->parameters.size(); ++i)
                if (std::abs(made->parameters[i].value - d.parameters[i].value) > 1e-4 * std::max(1.0, d.parameters[i].maximum - d.parameters[i].minimum)) ++moved;
            if (moved) note = "; " + std::to_string(moved) + " controls start away from the catalogue default";
            for (const auto& parameter : d.parameters) {
                if (!problem.empty()) break;
                for (const double value : {parameter.minimum, parameter.maximum, parameter.value}) {
                    const auto set = client.result(SetParameter{id, parameter.symbol, value});
                    if (!set.accepted) { problem = "control " + parameter.symbol + " refused " + std::to_string(value) + ": " + set.message; break; }
                }
            }
            if (problem.empty())
                for (const bool bypassed : {true, false}) {
                    const auto event = client.result(SetBypass{id, bypassed});
                    const auto* now = find_module(client.engine.snapshot(), id);
                    if (!event.accepted) problem = "bypass refused: " + event.message;
                    else if (!now || now->bypassed != bypassed) problem = "bypass did not read back";
                }
            if (client.engine.state() == EngineState::Ready) {
                if (const auto removed = client.result(RemoveModule{id}); !removed.accepted && problem.empty()) problem = "engine did not remove it: " + removed.message;
            }
        }
        if (client.engine.state() != EngineState::Ready) problem += (problem.empty() ? "" : "; ") + std::string("the engine went away");
        ++(problem.empty() ? passed : failed);
        if (!d.enable_symbol.empty()) note += "; switched on through its own control";
        std::cout << (problem.empty() ? "PASS  " : "FAIL  ") << d.name << " <" << d.uri << "> -- "
                  << (problem.empty() ? std::to_string(d.ports.size()) + " connectors, " + std::to_string(d.parameters.size()) + " controls" + note : problem) << std::endl;
        // A plugin that took the engine down ends the run: the rest cannot be judged.
        if (client.engine.state() != EngineState::Ready && !client.ready(3000)) break;
    }
    if (argc > 4 && client.engine.state() == EngineState::Ready) {
        const auto* d = definition(argv[4]);
        std::string in, out;
        if (d) for (const auto& port : d->ports) if (port.type == PortType::Audio) (port.direction == Direction::Input ? in : out).empty() ? void((port.direction == Direction::Input ? in : out) = port.symbol) : void();
        bool kept = d && !in.empty() && !out.empty() && client.result(AddModule{d->uri, 312, 132}).accepted;
        if (kept) {
            const auto id = client.engine.snapshot().modules.back().id;
            for (const auto& cable : client.engine.snapshot().connections) if (cable.output == "out") client.result(Disconnect{cable});
            kept = client.result(Connect{{1, id, "out", in}}).accepted && client.result(Connect{{id, 2, out, "in"}}).accepted;
        }
        std::cout << (kept ? "PASS  " : "FAIL  ") << "left for measurement: IN -> " << (d ? d->name : std::string(argv[4])) << " -> OUT on the left side" << std::endl;
        if (!kept) ++failed;
    }
    std::cout << "SUMMARY " << passed << " modules load and match, " << failed << " do not, " << withheld << " withheld by the catalogue" << std::endl;
    return failed ? 1 : 0;
}
