// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
// A module's file (NAM model, impulse response) against a running engine,
// through the interface's adapter. tools/pi/check-engine.sh measures what is
// heard between the steps:
//   peepo_file_live ADDRESS catalogue.json overlay.json STEP [ARGUMENT]
//   setup URI      a new graph: IN (left) -> the module -> OUT (left)
//   file PATH      give the module that file; a second client must see it
//   reload DIR     save the patch into a store at DIR and load it back
//   duplicate      replace the module with a duplicate of it, wired the same way
#include "catalogue/catalogue.hpp"
#include "engine/ingen/ingen_engine.hpp"
#include "storage/patch_store.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

using namespace peepo;
namespace {
using Clock = std::chrono::steady_clock;
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
        while (Clock::now() - start < std::chrono::seconds(10))
            for (const auto& e : tick()) if (e.request == id) return check(e.accepted, what, e.message);
        return check(false, what, "no answer");
    }
    void settle(int ms) { const auto until = Clock::now() + std::chrono::milliseconds(ms); while (Clock::now() < until) tick(); }
};
const Module* the_module(const Graph& g) {
    const auto it = std::find_if(g.modules.begin(), g.modules.end(), [](const Module& m) { return !m.fixed; });
    return it == g.modules.end() ? nullptr : &*it;
}
const Port* first(const Module& m, Direction direction) {
    for (const auto& p : module_ports(m)) if (p.type == PortType::Audio && p.direction == direction) return &p;
    return nullptr;
}
bool wire(Client& c, const Module& m) {
    const auto* in = first(m, Direction::Input);
    const auto* out = first(m, Direction::Output);
    if (!check(in && out, "the module has an audio input and output")) return false;
    return c.command(Connect{{1, m.id, "out", in->symbol}}, "IN left -> " + m.name) && c.command(Connect{{m.id, 2, out->symbol, "in"}}, m.name + " -> OUT left");
}
}

int main(int argc, char** argv) {
    if (argc < 5) { std::cerr << "usage: peepo_file_live ADDRESS catalogue.json overlay.json setup URI|file PATH|reload DIR|duplicate\n"; return 2; }
    const std::string step = argv[4], argument = argc > 5 ? argv[5] : "";
    std::vector<ModuleDefinition> definitions;
    if (const auto issue = load_catalogue(argv[2], argv[3], definitions); !issue.empty()) { std::cerr << issue << "\n"; return 1; }
    replace_catalogue(std::move(definitions));
    Client c(argv[1]);
    if (!check(c.engine.state() == EngineState::Ready, "connect to the engine")) return 1;

    if (step == "setup") {
        const auto* known = definition(argument);
        if (!check(known && !known->file.property.empty() && known->availability == Availability::Available, "the catalogue offers " + argument + " with a file to choose")) return 1;
        auto graph = empty_graph("File check");
        graph.connections.clear();
        c.command(LoadGraph{graph}, "load an empty graph");
        c.command(AddModule{argument, 312, 132}, "add " + known->name);
        const auto* m = the_module(c.engine.snapshot());
        if (!check(m && m->file.empty(), "the new module has no file yet")) return 1;
        wire(c, *m);
    } else if (step == "file") {
        const auto* m = the_module(c.engine.snapshot());
        if (!check(m != nullptr, "the module is there")) return 1;
        const auto id = m->id;
        c.command(SetFile{id, argument}, "give it " + argument);
        m = the_module(c.engine.snapshot());
        check(m && m->file == argument, "the engine reports the file back");
        // Another client, such as a restarted interface, sees it too.
        Client second(argv[1]);
        second.settle(500);
        const auto* seen = the_module(second.engine.snapshot());
        check(seen && seen->file == argument, "a second client sees the file");
        // A path that is not absolute never reaches the engine.
        const auto refused = c.engine.submit(SetFile{id, "relative/model.nam"});
        bool said = false;
        for (int i = 0; i < 500 && !said; ++i) for (const auto& e : c.tick()) if (e.request == refused) { said = true; check(!e.accepted, "a relative path is refused", e.message); }
    } else if (step == "reload") {
        PatchStore store(argument);
        check(store.open().empty(), "open a scratch patch store");
        const auto* m = the_module(c.engine.snapshot());
        if (!check(m && !m->file.empty(), "the module has a file")) return 1;
        const auto file = m->file;
        const auto id = store.new_id();
        std::string issue;
        const auto staging = store.stage(id, issue);
        if (!check(staging.has_value(), "stage a patch", issue)) return 1;
        c.command(SaveGraph{SaveScope::Persistent, true, staging->location, id}, "save the patch with plugin state");
        check(store.publish(*staging).empty(), "publish the saved patch");
        const auto stored = store.find(id);
        if (!check(stored && stored->issue.empty(), "the saved patch reads back")) return 1;
        std::string missing;
        for (const auto& f : stored->missing_files) missing += (missing.empty() ? "" : ", ") + f;
        check(stored->missing_files.empty(), "the saved state names no missing file", missing);
        const auto disk = bundle_graph(stored->bundle);
        const auto* saved = the_module(disk);
        check(saved && saved->file == file, "the bundle names the file");
        c.command(LoadPatch{stored->location, saved_blocks(stored->bundle), id}, "load the saved patch back");
        m = the_module(c.engine.snapshot());
        check(m && m->file == file, "after loading, the module names the same file");
    } else if (step == "duplicate") {
        const auto* m = the_module(c.engine.snapshot());
        if (!check(m && !m->file.empty(), "the module has a file")) return 1;
        const auto original = m->id;
        const auto file = m->file;
        c.command(DuplicateModule{original, 312, 276}, "duplicate it");
        c.command(RemoveModule{original}, "remove the original");
        m = the_module(c.engine.snapshot());
        if (!check(m && m->id != original && m->file == file, "the duplicate names the same file")) return 1;
        wire(c, *m);
    } else { std::cerr << "unknown step " << step << "\n"; return 2; }
    std::cout << (failures ? "SUMMARY failed " : "SUMMARY passed ") << step << std::endl;
    return failures ? 1 : 0;
}
