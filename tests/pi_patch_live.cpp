// SPDX-License-Identifier: GPL-3.0-or-later
// Check the current Pi patch using the same adapter and store as the UI.
// Never removes an existing patch. save and save-load create new identities.
#include "catalogue/catalogue.hpp"
#include "engine/ingen/ingen_engine.hpp"
#include "storage/patch_store.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <tuple>
using namespace pedal;
using Clock = std::chrono::steady_clock;
void require(bool ok, const std::string& message) {
    if (!ok) throw std::runtime_error(message);
    std::cout << "PASS " << message << std::endl;
}
struct Client {
    IngenEngine engine{ingen::Options{"unix:///run/pedalpi/ingen.sock"}};
    Client() {
        engine.connect();
        const auto until = Clock::now() + std::chrono::seconds(10);
        while (engine.state() != EngineState::Ready && Clock::now() < until) tick();
        require(engine.state() == EngineState::Ready, "connect to the running Pi engine");
    }
    std::vector<EngineEvent> tick() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return engine.advance(1);
    }
    void command(Command command, const std::string& description) {
        const auto start = Clock::now();
        const auto id = engine.submit(std::move(command));
        while (Clock::now() - start < std::chrono::seconds(10))
            for (const auto& event : tick()) if (event.request == id) {
                require(event.accepted, description + (event.accepted ? "" : ": " + event.message));
                std::cout << "TIME " << description << ": " << std::chrono::duration<double, std::milli>(Clock::now()-start).count() << " ms\n";
                return;
            }
        throw std::runtime_error("timeout: " + description);
    }
};
std::string canonical(Graph graph) {
    // Ingen can give a reloaded block a new client-local instance number,
    // particularly for blocks created outside the UI (such as /clouds).
    // Compare the musical graph independently of those transient numbers.
    std::vector<Module*> effects;
    for (auto& module : graph.modules) if (!module.fixed) effects.push_back(&module);
    std::sort(effects.begin(), effects.end(), [](const auto* a, const auto* b) {
        return std::tie(a->uri,a->name,a->x,a->y,a->id) < std::tie(b->uri,b->name,b->x,b->y,b->id);
    });
    std::map<InstanceId,InstanceId> ids{{1,1},{2,2}};
    InstanceId next = 3;
    for (const auto* module : effects) ids[module->id] = next++;
    for (auto& module : graph.modules) module.id = ids.at(module.id);
    for (auto& cable : graph.connections) { cable.from = ids.at(cable.from); cable.to = ids.at(cable.to); }
    for (auto& assignment : graph.assignments) assignment.module = ids.at(assignment.module);
    std::sort(graph.modules.begin(), graph.modules.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    std::sort(graph.connections.begin(), graph.connections.end(), [](const auto& a, const auto& b) {
        return std::tie(a.from,a.to,a.output,a.input) < std::tie(b.from,b.to,b.output,b.input);
    });
    for (auto& module : graph.modules)
    {
        for (auto& p : module.parameters) p.value = std::round(p.value * 1e6) / 1e6;
        std::sort(module.parameters.begin(), module.parameters.end(), [](const auto& a, const auto& b) { return a.symbol < b.symbol; });
    }
    return write_main_ttl(graph);
}
int main(int argc, char** argv) try {
    const std::string mode = argc > 1 ? argv[1] : "inspect";
    if (mode != "inspect" && mode != "save" && mode != "save-load" && mode != "verify" && mode != "audit" && mode != "resume") throw std::runtime_error("usage: pedal_pi_patch_live [inspect|save|save-load|verify|audit|resume]");
    std::vector<ModuleDefinition> definitions;
    const auto issue = load_catalogue("/opt/pedalpi/share/pedalpi/catalogue.json", "/opt/pedalpi/share/pedalpi/catalogue-overlay.json", definitions);
    require(issue.empty(), "load the installed catalogue and overlay" + (issue.empty() ? "" : ": " + issue));
    replace_catalogue(std::move(definitions));
    Client client;
    const auto original = client.engine.snapshot();
    std::cout << "CURRENT " << original.name << ", id=" << original.patch_id << ", modules=" << original.modules.size() << ", cables=" << original.connections.size() << ", dirty=" << original.dirty << '\n';
    for (const auto& module : original.modules) if (!module.fixed)
        for (const auto& p : module.parameters) std::cout << "CONTROL " << module.name << '/' << p.symbol << '=' << p.value << '\n';
    if (mode == "inspect") return 0;
    PatchStore store("/var/lib/pedalpi/patches");
    require(store.open().empty(), "open the Pi's real patch store");
    const std::string baseline = "/tmp/pedalpi-patch-restart-baseline.ttl";
    if (mode == "audit") {
        const auto stored = store.find(original.patch_id);
        require(stored.has_value(), "find the patch saved in this session");
        auto expected = bundle_graph(stored->bundle);
        expected.dirty = false;
        std::ofstream("/tmp/pi-patch-expected.ttl") << canonical(expected);
        std::ofstream("/tmp/pi-patch-observed.ttl") << canonical(original);
        require(canonical(expected) == canonical(original), "saved bundle matches the restored engine graph (controls to 1e-6)");
        return 0;
    }
    if (mode == "verify") {
        std::ifstream input(baseline);
        const std::string expected((std::istreambuf_iterator<char>(input)), {});
        require(!expected.empty() && canonical(original) == expected, "name, identity, modules, cables, controls, layout and dirty flag unchanged after UI restart");
        const auto stored = store.find(original.patch_id);
        require(stored && stored->issue.empty() && stored->missing_files.empty(), "saved patch still lists cleanly in the Pi patch store");
        return 0;
    }
    if (mode == "save") {
        const auto id = store.new_id();
        std::string staging_issue;
        const auto staging = store.stage(id, staging_issue);
        require(staging.has_value(), "stage a new copy of the live patch: " + staging_issue);
        client.command(SaveGraph{SaveScope::Persistent, true, staging->location, id}, "save the live patch without changing controls or rebuilding audio");
        require(store.publish(*staging).empty(), "publish the saved live patch atomically");
        auto expected = original;
        expected.patch_id = id;
        expected.dirty = false;
        require(canonical(expected) == canonical(client.engine.snapshot()), "saving preserves all modules, cables, controls, layout and assignments");
        const auto stored = store.find(id);
        require(stored && stored->issue.empty() && stored->missing_files.empty(), "saved live patch is complete and readable");
        auto disk = bundle_graph(stored->bundle);
        disk.dirty = false;
        require(canonical(expected) == canonical(disk), "saved bundle matches the live graph (controls to 1e-6)");
        std::cout << "SAVED " << stored->name << " id=" << id << " path=" << stored->path << '\n';
        return 0;
    }
    require(original.modules.size() > 2, "the current musical patch contains an effect");
    const auto id = mode == "resume" ? original.patch_id : store.new_id();
    if (mode != "resume") {
    std::string staging_issue;
    const auto staging = store.stage(id, staging_issue);
    require(staging.has_value(), "stage a new patch without replacing existing saved patches");
    client.command(SaveGraph{SaveScope::Persistent, true, staging->location, id}, "save current patch with plugin state");
    require(store.publish(*staging).empty(), "validate and publish revision 1 atomically");
    }
    const auto stored = store.find(id);
    require(stored && stored->revision == 1 && stored->issue.empty() && stored->missing_files.empty(), "saved patch is listed with a complete readable bundle");
    std::cout << "SAVED " << stored->name << " id=" << id << " path=" << stored->path << '\n';
    const auto saved = client.engine.snapshot();
    require(saved.patch_id == id && !saved.dirty, "engine knows the saved identity and clears unsaved edits");
    // Change only a small amount of blend, so the reload must restore a value.
    bool changed = false;
    for (const auto& module : saved.modules) if (!module.fixed && !changed)
        for (const auto& p : module.parameters) if (p.symbol == "blend_param") {
            const double delta = (p.maximum-p.minimum) * 0.02;
            const double value = p.value + delta <= p.maximum ? p.value + delta : p.value - delta;
            client.command(SetParameter{module.id,p.symbol,value}, "change blend temporarily before reload");
            changed = true;
            break;
        }
    require(changed && client.engine.snapshot().dirty, "the running patch differs from the saved revision");
    client.command(LoadPatch{stored->location,saved_blocks(stored->bundle),id}, "reload the saved patch from the Pi card");
    auto restored = client.engine.snapshot();
    std::ofstream("/tmp/pi-patch-before-load.ttl") << canonical(saved);
    std::ofstream("/tmp/pi-patch-after-load.ttl") << canonical(restored);
    require(canonical(saved) == canonical(restored), "saved identity, name, modules, cables, every control, positions and assignments restored exactly");
    // A frozen Clouds buffer has no LV2 state. Keep the exact original as
    // revision 1, then publish a live-input revision for the playing test.
    bool thawed = false;
    for (const auto& module : restored.modules) if (!module.fixed)
        for (const auto& p : module.parameters) if (p.symbol == "freeze_param" && p.value == 1) {
            std::cout << "NOTE Clouds' captured buffer is not saved; preserving frozen controls in revision 1\n";
            client.command(SetParameter{module.id,p.symbol,0}, "turn Freeze off for live guitar input");
            thawed = true;
        }
    if (thawed) {
        std::string staging_issue;
        const auto next = store.stage(id, staging_issue);
        require(next.has_value(), "stage revision 2 while keeping revision 1");
        client.command(SaveGraph{SaveScope::Persistent,true,next->location,id}, "save the live-input revision");
        require(store.publish(*next).empty(), "publish revision 2 with original revision 1 retained");
        const auto live_saved = client.engine.snapshot();
        const auto current = store.find(id);
        require(current && current->revision == 2, "the Pi patch manager lists revision 2");
        client.command(LoadPatch{current->location,saved_blocks(current->bundle),id}, "reload the live-input revision");
        restored = client.engine.snapshot();
        require(canonical(live_saved) == canonical(restored), "live-input revision restores exactly with Freeze off");
        std::cout << "SAVED live-input revision path=" << current->path << '\n';
    }
    std::ofstream output(baseline);
    output << canonical(restored);
    output.close();
    require(bool(output), "record a baseline for the interface-only restart");
    std::cout << "SUMMARY patch save and reload passed on the Pi; restored patch left playing\n";
    return 0;
} catch (const std::exception& e) { std::cerr << "FAIL " << e.what() << std::endl; return 1; }
