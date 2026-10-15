// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
// Runs only against an isolated test engine. A JACK tone generator reads
// the frequency file; see tools/ingen-spike/17-tuner.sh.
#include "catalogue/catalogue.hpp"
#include "core/tuning.hpp"
#include "engine/ingen/ingen_engine.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <stdexcept>
using namespace pedal;
int main(int argc, char** argv) {
    try {
        if (argc != 4) throw std::runtime_error("Expected engine address, catalogue and tone-control file");
        std::vector<ModuleDefinition> modules;
        const auto issue = load_catalogue(argv[2], "", modules);
        if (!issue.empty()) throw std::runtime_error(issue);
        replace_catalogue(std::move(modules));
        IngenEngine engine(ingen::Options{argv[1]});
        auto step = [&] { engine.advance(10); std::this_thread::sleep_for(std::chrono::milliseconds(10)); };
        auto command = [&](Command c) {
            const auto id = engine.submit(std::move(c));
            for (int i = 0; i < 600; ++i) {
                for (const auto& event : engine.advance(10)) if (event.request == id) {
                    if (!event.accepted) throw std::runtime_error(event.message);
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            throw std::runtime_error("Command timed out");
        };
        engine.connect();
        for (int i = 0; i < 600 && engine.state() != EngineState::Ready; ++i) step();
        if (engine.state() != EngineState::Ready) throw std::runtime_error("Engine did not connect");
        command(NewPatch{"Tuner test"});
        command(AddModule{"http://gareus.org/oss/lv2/tuna#mod", 184, 132});
        const auto id = engine.snapshot().modules.back().id;
        command(Connect{{1, id, "out", "in"}});
        command(Connect{{id, 2, "out", "in"}});
        engine.watch_tuner(id);
        struct Case { double frequency, reference, cents; const char* note; };
        const Case cases[] = {{440, 440, 0, "A4"}, {440 * std::pow(2., -23. / 1200), 440, -23, "A4"},
            {440 * std::pow(2., 19. / 1200), 440, 19, "A4"}, {82.4069, 440, 0, "E2"}, {442, 442, 0, "A4"}, {0, 442, 0, ""}};
        unsigned case_number = 0;
        for (const auto& test : cases) {
            command(SetParameter{id, "tuning", test.reference});
            std::ofstream(argv[3]) << test.frequency;
            // Warm up the tracker and hold the signal long enough to test
            // changed-only notifications plus periodic reads.
            for (int i = 0; i < 250; ++i) step();
            const auto frequency = engine.tuner_frequency(id);
            const auto tuning = tuning_from_frequency(frequency, test.reference);
            const bool ok = test.frequency == 0 ? !tuning && frequency && *frequency == 0 :
                tuning && tuning->note == test.note && std::abs(tuning->cents - test.cents) < 2;
            std::cout << (ok ? "PASS " : "FAIL ") << test.frequency << " Hz at A=" << test.reference;
            std::cout << " (raw " << (frequency ? std::to_string(*frequency) : "unknown") << ")";
            if (tuning) std::cout << " -> " << tuning->note << ", " << tuning->cents << " cents";
            std::cout << std::endl;
            if (!ok) throw std::runtime_error("Live tuner reading incorrect");
            // A tuner can be tapped off the input without routing its audio
            // output into the musical chain.
            if (case_number++ == 0) command(Disconnect{{id, 2, "out", "in"}});
        }
        engine.watch_tuner(std::nullopt);
        if (engine.tuner_frequency(id)) throw std::runtime_error("Reading retained after leaving tuner");
        command(NewPatch{"Tuner test complete"});
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
