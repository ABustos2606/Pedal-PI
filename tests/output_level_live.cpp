// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
// The general Output against a running engine with patch 0008. Replaces the
// engine's patch; tools/ingen-spike/18-output-level.sh measures what is heard
// after each step:
//   peepo_output_level_live ADDRESS catalogue.json STEP
//   setup      IN (left) -> example amp at 0 dB with Output -6 dB -> OUT (left); right direct
//   reload     save the patch to /tmp/peepo-output-patches, change Output, load it back
//   duplicate  remove the amp, put a duplicate of it in its place
//   bypass     bypass the duplicate: the input passes unchanged
// The example amp has a gain control of its own; with no overlay it is treated
// like any module without one, which is what this needs.
#include "catalogue/catalogue.hpp"
#include "engine/ingen/ingen_engine.hpp"
#include "storage/patch_store.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

using namespace peepo;
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
    IngenEngine engine;
    explicit Client(const std::string& address) : engine(ingen::Options{address}) {
        engine.connect();
        const auto until = Clock::now() + std::chrono::seconds(5);
        while (engine.state() != EngineState::Ready && Clock::now() < until) tick();
    }
    std::vector<EngineEvent> tick() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); return engine.advance(1); }
    bool command(Command c, const std::string& what) {
        const auto id = engine.submit(std::move(c));
        const auto start = Clock::now();
        while (Clock::now() - start < std::chrono::seconds(6))
            for (const auto& e : tick()) if (e.request == id) return check(e.accepted, what, e.message);
        return check(false, what, "no answer");
    }
};
const Module* by_uri(const Graph& g) {
    const auto it = std::find_if(g.modules.begin(), g.modules.end(), [](const Module& m) { return m.uri == amp; });
    return it == g.modules.end() ? nullptr : &*it;
}
double output_of(const Module& m) {
    for (const auto& p : m.parameters) if (p.symbol == general_output_symbol) return p.value;
    return std::nan("");
}
}

int main(int argc, char** argv) {
    if (argc != 4) { std::cerr << "usage: peepo_output_level_live ADDRESS catalogue.json setup|reload|duplicate|bypass\n"; return 2; }
    const std::string step = argv[3];
    std::vector<ModuleDefinition> definitions;
    if (const auto issue = load_catalogue(argv[2], "", definitions); !issue.empty()) { std::cerr << issue << "\n"; return 1; }
    // An overlay with no rules: every module with audio outputs and no named
    // output control of its own gets the general Output.
    if (const auto issue = apply_catalogue_overlay(R"({"format": "peepo-catalogue-overlay", "version": 1, "rules": []})", definitions); !issue.empty()) {
        std::cerr << issue << "\n"; return 1;
    }
    replace_catalogue(std::move(definitions));
    const auto* known = definition(amp);
    if (!check(known && known->parameters.size() == 2 && known->parameters.back().symbol == general_output_symbol, "the example amp is offered with the general Output"))
        return 1;
    Client c(argv[1]);
    if (!check(c.engine.state() == EngineState::Ready, "connect to the engine")) return 1;

    if (step == "setup") {
        auto graph = empty_graph("Output level");
        graph.connections = {{1, 2, "out_r", "in_r"}};
        c.command(LoadGraph{graph}, "load a graph with only the right channel wired");
        c.command(AddModule{amp, 184, 132}, "add the example amp");
        const auto* m = by_uri(c.engine.snapshot());
        if (!check(m && output_of(*m) == 0, "a new module's Output starts at 0 dB")) return 1;
        const auto id = m->id;
        c.command(Connect{{1, id, "out", "in"}}, "IN left -> amp");
        c.command(Connect{{id, 2, "out", "in"}}, "amp -> OUT left");
        c.command(SetParameter{id, general_output_symbol, -6.0206}, "set Output to -6.02 dB");
        m = by_uri(c.engine.snapshot());
        check(m && std::abs(output_of(*m) + 6.0206) < 1e-4, "the engine reports Output -6.02 dB back");
        const auto beyond = c.engine.submit(SetParameter{id, general_output_symbol, 20});
        bool refused = false;
        for (int i = 0; i < 500 && !refused; ++i) for (const auto& e : c.tick()) if (e.request == beyond) refused = !e.accepted;
        check(refused, "an Output beyond +12 dB is refused");
    } else if (step == "reload") {
        PatchStore store("/tmp/peepo-output-patches");
        check(store.open().empty(), "open a scratch patch store");
        const auto id = store.new_id();
        std::string issue;
        const auto staging = store.stage(id, issue);
        if (!check(staging.has_value(), "stage a patch", issue)) return 1;
        c.command(SaveGraph{SaveScope::Persistent, true, staging->location, id}, "save the patch with its Output");
        check(store.publish(*staging).empty(), "publish the saved patch");
        const auto stored = store.find(id);
        if (!check(stored && stored->issue.empty(), "the saved patch reads back")) return 1;
        const auto disk = bundle_graph(stored->bundle);
        const auto* saved = by_uri(disk);
        check(saved && std::abs(output_of(*saved) + 6.0206) < 1e-4, "the bundle on disk holds Output -6.02 dB");
        const auto* m = by_uri(c.engine.snapshot());
        c.command(SetParameter{m->id, general_output_symbol, 0}, "change Output to 0 dB before reloading");
        c.command(LoadPatch{stored->location, saved_blocks(stored->bundle), id}, "load the saved patch back");
        m = by_uri(c.engine.snapshot());
        check(m && std::abs(output_of(*m) + 6.0206) < 1e-4, "reloading restores Output -6.02 dB");
    } else if (step == "duplicate") {
        const auto* m = by_uri(c.engine.snapshot());
        if (!check(m != nullptr, "the amp is there")) return 1;
        const auto original = m->id;
        c.command(DuplicateModule{original, 184, 276}, "duplicate the amp");
        c.command(RemoveModule{original}, "remove the original");
        m = by_uri(c.engine.snapshot());
        if (!check(m && m->id != original && std::abs(output_of(*m) + 6.0206) < 1e-4, "the duplicate shows Output -6.02 dB")) return 1;
        c.command(Connect{{1, m->id, "out", "in"}}, "IN left -> duplicate");
        c.command(Connect{{m->id, 2, "out", "in"}}, "duplicate -> OUT left");
    } else if (step == "bypass") {
        const auto* m = by_uri(c.engine.snapshot());
        if (!check(m != nullptr, "the amp is there")) return 1;
        c.command(SetBypass{m->id, true}, "bypass the amp");
    } else { std::cerr << "unknown step " << step << "\n"; return 2; }
    std::cout << (failures ? "SUMMARY failed " : "SUMMARY passed ") << step << std::endl;
    return failures ? 1 : 0;
}
