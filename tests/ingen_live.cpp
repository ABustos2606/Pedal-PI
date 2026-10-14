// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
// The Ingen adapter against a running engine. Needs the LV2 example amplifier
// and replaces whatever patch the engine holds:
//   pedal_ingen_live unix:///tmp/pedalpi-ingen.sock
// It leaves IN (left) -> amp at +6.02 dB -> OUT (left) for a level measurement.
// With --survive-restart it instead waits for the engine to be stopped and
// started again, and reports whether the adapter found its way back.
#include "engine/ingen/ingen_engine.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

using namespace pedal;
namespace {
using Clock = std::chrono::steady_clock;
const std::string amp = "http://lv2plug.in/plugins/eg-amp";
int failures = 0;
bool check(bool ok, const std::string& name, const std::string& detail = {}) {
    std::cout << (ok ? "PASS  " : "FAIL  ") << name << (detail.empty() ? "" : " -- " + detail) << std::endl;
    if (!ok) ++failures;
    return ok;
}
struct Client {
    explicit Client(const std::string& address) : engine(ingen::Options{address}) {}
    std::vector<EngineEvent> pump(unsigned milliseconds) {
        std::vector<EngineEvent> all;
        const auto until = Clock::now() + std::chrono::milliseconds(milliseconds);
        while (Clock::now() < until) {
            for (auto& event : step()) all.push_back(std::move(event));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return all;
    }
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
    EngineEvent result(Command command, double* took = nullptr) {
        const auto start = Clock::now();
        const auto id = engine.submit(std::move(command));
        while (Clock::now() - start < std::chrono::seconds(6)) {
            for (auto& event : step()) if (event.request == id) {
                if (took) *took = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
                return event;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
        return {id, false, "no result within six seconds", engine.state(), {}, {}};
    }
    bool ok(Command command, const std::string& name) {
        const auto event = result(std::move(command));
        return check(event.accepted, name, event.accepted ? "" : event.message);
    }
    const Graph& graph() const { return engine.snapshot(); }
    IngenEngine engine;
    Clock::time_point last = Clock::now();
};
bool has_cable(const Graph& graph, const Connection& cable) { return std::find(graph.connections.begin(), graph.connections.end(), cable) != graph.connections.end(); }
double gain_of(const Graph& graph, InstanceId id) { const auto* m = find_module(graph, id); return m && !m->parameters.empty() ? m->parameters[0].value : std::nan(""); }
}
int main(int argc, char** argv) {
    const std::string address = argc > 1 ? argv[1] : "unix:///tmp/pedalpi-ingen.sock";
    register_definitions(ingen_development_catalogue());
    auto a = std::make_unique<Client>(address);
    a->engine.connect();
    if (!check(a->ready(), "connect, read the graph and create the audio endpoints", address)) return 1;
    if (argc > 2 && std::string(argv[2]) == "--survive-restart") {
        a->ok(NewPatch{"Before restart"}, "edit before the engine goes away");
        std::cout << "WAITING for the engine to stop" << std::endl;
        std::vector<EngineEvent> seen;
        const auto until = Clock::now() + std::chrono::seconds(20);
        while (a->engine.state() == EngineState::Ready && Clock::now() < until) for (auto& e : a->pump(20)) seen.push_back(std::move(e));
        check(a->engine.state() == EngineState::Connecting && !seen.empty() && !seen.back().message.empty(), "a stopped engine is noticed and reported",
              seen.empty() ? "" : seen.back().message);
        check(!a->result(RenamePatch{"While away"}).accepted && a->graph().name == "Before restart", "edits are refused and the last graph kept while the engine is away");
        check(a->ready(20000), "the adapter reconnects by itself when the engine returns");
        check(a->graph().name == "Untitled" && a->graph().connections.empty(), "the graph shown is the new engine's, not the old one remembered");
        a->ok(RenamePatch{"After restart"}, "edits work again");
        std::cout << "SUMMARY " << (failures ? std::to_string(failures) + " failed" : std::string("engine restart survived")) << std::endl;
        return failures ? 1 : 0;
    }

    a->ok(NewPatch{"Live check"}, "new patch replaces what the engine held");
    check(a->graph().modules.size() == 2 && a->graph().connections.size() == 2 && a->graph().name == "Live check", "new patch is IN wired to OUT on both sides");
    auto event = a->result(AddModule{amp, 184, 132});
    check(event.accepted && a->graph().modules.size() == 3, "add a plugin", event.message);
    if (a->graph().modules.size() < 3) return 1;
    const auto first = a->graph().modules[2];
    check(first.uri == amp && first.x == 184 && first.y == 132 && first.ports.size() == 2 && first.parameters.size() == 1 && first.parameters[0].value == 0,
          "the new module carries the engine's ports and value", "id " + std::to_string(first.id) + ", " + std::to_string(first.ports.size()) + " ports");
    check(!a->result(AddModule{"urn:pedalpi:demo:drive", 24, 24}).accepted, "a plugin the engine lacks is refused");
    a->ok(Disconnect{{1, 2, "out", "in"}}, "remove the direct left cable");
    a->ok(Connect{{1, first.id, "out", "in"}}, "cable IN to the module");
    a->ok(Connect{{first.id, 2, "out", "in"}}, "cable the module to OUT");
    check(!a->result(Connect{{first.id, first.id, "out", "in"}}).accepted, "feedback is refused before it reaches the engine");
    check(a->graph().connections.size() == 3 && has_cable(a->graph(), {1, first.id, "out", "in"}) && has_cable(a->graph(), {1, 2, "out_r", "in_r"}), "cables read back from the engine");
    a->ok(SetParameter{first.id, "gain", 6.0206}, "set a value");
    check(std::abs(gain_of(a->graph(), first.id) - 6.0206) < 1e-6 && !a->result(SetParameter{first.id, "gain", 9999}).accepted, "value applied; out-of-range value refused");
    a->ok(SetBypass{first.id, true}, "bypass");
    a->ok(MoveModule{first.id, 312, 144}, "move");
    event = a->result(DuplicateModule{first.id, 456, 144});
    const auto copy = a->graph().modules.back();
    check(event.accepted && a->graph().modules.size() == 4 && copy.id != first.id && copy.bypassed && std::abs(copy.parameters[0].value - 6.0206) < 1e-4 && copy.x == 456 && copy.ports.size() == 2,
          "duplicate keeps value and bypass at the new position", event.message);
    const std::string awkward = "Live \"check\" \xc3\xa9 .";
    a->ok(RenamePatch{awkward}, "rename the patch");

    // A second client reads the same graph and each hears the other.
    Client b(address);
    b.engine.connect();
    check(b.ready() && b.graph().modules.size() == 4 && b.graph().name == awkward && b.graph().connections.size() == 3, "a second client reads the same graph");
    const auto* seen = find_module(b.graph(), first.id);
    check(seen && seen->bypassed && seen->x == 312 && seen->y == 144 && std::abs(gain_of(b.graph(), first.id) - 6.0206) < 1e-4, "values, bypass and positions were in the engine, not only in the first client");
    b.ok(SetParameter{copy.id, "gain", -12}, "second client sets a value");
    a->pump(100);
    check(std::abs(gain_of(a->graph(), copy.id) + 12) < 1e-6, "the first client hears the second client's change");
    b.ok(RemoveModule{copy.id}, "second client removes a module");
    a->pump(100);
    check(a->graph().modules.size() == 3 && !find_module(a->graph(), copy.id), "the first client sees the removal");

    // Session snapshot, then rebuild the whole patch through the engine.
    a->ok(SetBypass{first.id, false}, "bypass off");
    a->ok(SaveGraph{}, "session snapshot");
    const auto saved = a->graph();
    a->ok(NewPatch{"Scratch"}, "new patch");
    check(a->graph().modules.size() == 2, "engine graph emptied");
    event = a->result(LoadSession{});
    const auto* restored = find_module(a->graph(), first.id);
    check(event.accepted && restored && a->graph().name == awkward && a->graph().connections.size() == saved.connections.size() && restored->x == 312 && !restored->bypassed &&
          std::abs(restored->parameters[0].value - 6.0206) < 1e-6, "session snapshot rebuilt in the engine", event.message);
    b.pump(150);
    check(b.graph().modules.size() == 3 && std::abs(gain_of(b.graph(), first.id) - 6.0206) < 1e-4 && b.graph().connections.size() == saved.connections.size(),
          "the second client followed the rebuild, including values it was never sent directly");

    std::vector<double> times;
    for (int i = 0; i < 200; ++i) { double took = 0; a->result(SetParameter{first.id, "gain", 6.0206 - 0.001 * (i % 2)}, &took); times.push_back(took); }
    a->ok(SetParameter{first.id, "gain", 6.0206}, "restore the value");
    std::sort(times.begin(), times.end());
    std::cout << "INFO  edit round trip through the adapter -- median " << times[100] << " ms, p95 " << times[189] << " ms, max " << times.back() << " ms (200 edits)" << std::endl;

    // The interface dies without a word; a new one finds everything as it was.
    const auto running = a->graph();
    a.reset();
    Client restarted(address);
    restarted.engine.connect();
    check(restarted.ready() && restarted.graph().modules.size() == running.modules.size() && restarted.graph().name == running.name &&
          restarted.graph().connections.size() == running.connections.size() && find_module(restarted.graph(), first.id) &&
          std::abs(gain_of(restarted.graph(), first.id) - 6.0206) < 1e-4, "a restarted interface recovers the graph under the same identities");
    const auto load = restarted.engine.telemetry().cpu_percent;
    restarted.pump(300);
    std::cout << "INFO  engine load reading -- " << (restarted.engine.telemetry().cpu_percent ? std::to_string(*restarted.engine.telemetry().cpu_percent) + " %" : std::string("none yet"))
              << (load ? "" : " (none at connect)") << std::endl;
    restarted.ok(SetParameter{first.id, "gain", 6.0206}, "the restarted interface can edit");
    std::cout << "SUMMARY " << (failures ? std::to_string(failures) + " failed" : std::string("all adapter checks passed")) << std::endl;
    return failures ? 1 : 0;
}
