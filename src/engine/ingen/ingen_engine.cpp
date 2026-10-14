// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#include "engine/ingen/ingen_engine.hpp"

namespace pedal {
IngenEngine::IngenEngine(ingen::Options options, std::unique_ptr<ingen::Transport> transport)
    : options_(options), session_(options),
      transport_(transport ? std::move(transport) : ingen::socket_transport(options.address, options.max_message_bytes * 2)) {}
IngenEngine::~IngenEngine() { transport_->close(); }
void IngenEngine::connect() {
    wanted_ = true;
    if (open_ || connecting_) return;
    reported_ = false;
    session_.opening();
    attempt();
}
void IngenEngine::attempt() {
    connecting_ = true;
    transport_->open();
}
void IngenEngine::disconnect() {
    wanted_ = open_ = connecting_ = false;
    transport_->close();
    session_.closed("", false);
}
void IngenEngine::lost(std::string reason) {
    open_ = connecting_ = false;
    retry_in_ = options_.reconnect_ms;
    if (!options_.reconnect_ms) { wanted_ = false; session_.closed(std::move(reason), false); }
    // Say once why the engine is away; the attempts that follow are silent.
    else if (!reported_) { reported_ = true; session_.closed(std::move(reason), true); }
}
void IngenEngine::flush() {
    for (auto& message : session_.take_outgoing()) if (open_) transport_->send(std::move(message));
}
RequestId IngenEngine::submit(Command command) {
    const auto id = session_.submit(std::move(command));
    flush();
    return id;
}
std::vector<EngineEvent> IngenEngine::advance(unsigned milliseconds) {
    for (auto& event : transport_->poll()) {
        if (!wanted_) break;
        switch (event.kind) {
        case ingen::Transport::Event::Kind::Opened:
            connecting_ = false; open_ = true; reported_ = false;
            session_.opened();
            break;
        case ingen::Transport::Event::Kind::Data:
            if (open_) session_.received(event.data);
            break;
        case ingen::Transport::Event::Kind::Closed:
            lost(std::move(event.data));
            break;
        }
    }
    auto events = session_.advance(milliseconds);
    if (auto fault = session_.take_fault()) {
        // The stream cannot be trusted any more: replace it and read the graph again.
        transport_->close();
        lost(std::move(*fault));
        for (auto& event : session_.advance(0)) events.push_back(std::move(event));
    }
    if (wanted_ && !open_ && !connecting_) {
        if (retry_in_ > milliseconds) retry_in_ -= milliseconds;
        else attempt();
    }
    flush();
    return events;
}
std::vector<ModuleDefinition> ingen_development_catalogue() {
    // Ranges from lv2/plugins/eg-amp.lv2/amp.ttl, which every LV2 installation ships.
    return {{"http://lv2plug.in/plugins/eg-amp", "Amp", "Utility", "LV2 example amplifier / engine development", 0xa6c8ec,
             {{"in", Direction::Input}, {"out", Direction::Output}},
             {{"gain", "Gain", "dB", ParameterKind::Continuous, -90, 24, 0}}}};
}
}
