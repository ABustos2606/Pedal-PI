// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#pragma once
#include "core/model.hpp"
#include <optional>
#include <variant>

namespace pedal {
enum class EngineState { Ready, Disconnected, Error, Connecting };
struct EngineCapabilities {
    bool simulation = false;
    bool session_save = false;
    bool persistent_save = false;
    bool plugin_state = false;
    bool audio_processing = false;
};
struct EngineTelemetry {
    // Global tempo is independent of the displayed module or active graph.
    // Missing readings remain unknown, rather than displaying zero or stale load.
    std::optional<double> tempo_bpm;
    std::optional<double> cpu_percent;
};
using RequestId = std::uint64_t;
struct AddModule { std::string uri; int x, y; };
struct RemoveModule { InstanceId id; };
struct MoveModule { InstanceId id; int x, y; };
struct DuplicateModule { InstanceId id; int x, y; };
struct SetParameter { InstanceId id; std::string symbol; double value; };
struct SetBypass { InstanceId id; bool value; };
struct Connect { Connection connection; };
struct Disconnect { Connection connection; };
struct LoadGraph { Graph graph; };
struct NewPatch { std::string name; };
struct RenamePatch { std::string name; };
enum class SaveScope { Session, Persistent };
struct SaveGraph { SaveScope scope = SaveScope::Session; bool include_plugin_state = false; };
struct LoadSession {};
using Command = std::variant<AddModule, RemoveModule, MoveModule, DuplicateModule, SetParameter, SetBypass, Connect, Disconnect,
                             LoadGraph, NewPatch, RenamePatch, SaveGraph, LoadSession>;
struct EngineEvent {
    RequestId request = 0;
    bool accepted = true;
    std::string message;
    EngineState state = EngineState::Ready;
    std::optional<Graph> snapshot;
    std::optional<Graph> saved_session;
};
class Engine {
public:
    virtual ~Engine() = default;
    virtual RequestId submit(Command) = 0;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual std::vector<EngineEvent> advance(unsigned milliseconds) = 0;
    virtual EngineState state() const = 0;
    virtual const Graph& snapshot() const = 0;
    virtual EngineCapabilities capabilities() const = 0;
    virtual EngineTelemetry telemetry() const { return {}; }
    // Optional development controls; real adapters keep these unsupported.
    virtual bool simulate_state(EngineState) { return false; }
    virtual bool simulate_rejection() { return false; }
    virtual bool simulate_missing_module() { return false; }
};
}
