// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#include "engine/simulated_engine.hpp"
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <unordered_set>
#include <limits>

namespace pedal {
RequestId SimulatedEngine::submit(Command c) {
    const auto id = next_request_++;
    if (state_ != EngineState::Ready) {
        events_.push_back({id, false, "Engine is unavailable", state_, graph_}); return id;
    }
    pending_.push_back({id, std::move(c), clock_ + 100});
    return id;
}
void SimulatedEngine::set_state(EngineState state) {
    connect_due_.reset();
    state_ = state;
    // A connection loss cancels requests; reconnect never applies stale edits.
    if (state != EngineState::Ready) {
        for (const auto& p : pending_) events_.push_back({p.id, false, "Engine disconnected", state, {}});
        pending_.clear();
    }
    events_.push_back({0, true, "", state, graph_});
}
void SimulatedEngine::connect() {
    set_state(EngineState::Connecting); connect_due_ = clock_ + 100;
}
bool SimulatedEngine::simulate_missing_module() {
    if (state_ != EngineState::Ready || !pending_.empty()) return false;
    graph_ = missing_module_graph();
    events_.push_back({0, true, "Missing module fixture loaded. Orange cables are inactive.", state_, graph_});
    return true;
}
std::vector<EngineEvent> SimulatedEngine::advance(unsigned milliseconds) {
    clock_ += milliseconds;
    if (connect_due_ && clock_ >= *connect_due_) set_state(EngineState::Ready);
    while (!pending_.empty() && pending_.front().due <= clock_) {
        auto p = std::move(pending_.front()); pending_.pop_front();
        std::string error;
        if (state_ != EngineState::Ready) error = "Engine is unavailable";
        else if (reject_next_) { error = "Simulated command rejection"; reject_next_ = false; }
        else error = apply(p.command);
        EngineEvent event{p.id, error.empty(), error, state_, graph_};
        if (error.empty() && std::holds_alternative<SaveGraph>(p.command)) {
            event.saved_session = saved_session_;
            event.message = "Session snapshot saved in memory. Closing the app clears it.";
        }
        events_.push_back(std::move(event));
    }
    auto result = std::move(events_); events_.clear(); return result;
}
std::string SimulatedEngine::apply(const Command& command) {
    auto error = std::visit([&](const auto& c) -> std::string {
        using T = std::decay_t<decltype(c)>;
        if constexpr (std::is_same_v<T, AddModule>) {
            const auto* d = definition(c.uri);
            if (!d || d->category == "Endpoint" || d->availability != Availability::Available) return "Module is unavailable or untested";
            if (next_instance_ == std::numeric_limits<InstanceId>::max()) return "Instance ID space exhausted";
            graph_.modules.push_back({next_instance_++, d->uri, d->name, std::clamp(c.x, module_min_x, module_max_x),
                std::clamp(c.y, module_min_y, module_max_y), false, false, d->parameters, d->ports});
        } else if constexpr (std::is_same_v<T, Connect>) {
            auto issue = validate_connection(graph_, c.connection);
            if (!issue.empty()) return issue;
            graph_.connections.push_back(c.connection);
        } else if constexpr (std::is_same_v<T, Disconnect>) {
            const auto count = std::erase(graph_.connections, c.connection);
            if (!count) return "Cable is missing";
        } else if constexpr (std::is_same_v<T, LoadGraph>) {
            if (auto issue = validate_graph(c.graph); !issue.empty()) return issue;
            graph_ = c.graph;
            for (const auto& m : graph_.modules) next_instance_ = std::max(next_instance_, m.id + 1);
        } else if constexpr (std::is_same_v<T, NewPatch>) {
            if (auto issue = validate_patch_name(c.name); !issue.empty()) return issue;
            graph_ = empty_graph(c.name);
        } else if constexpr (std::is_same_v<T, RenamePatch>) {
            if (auto issue = validate_patch_name(c.name); !issue.empty()) return issue;
            graph_.name = c.name;
        } else if constexpr (std::is_same_v<T, SaveGraph>) {
            if (c.scope != SaveScope::Session) return "Unsupported capability: persistent patch storage";
            if (c.include_plugin_state) return "Unsupported capability: plugin state capture";
            saved_session_ = graph_; saved_session_->dirty = false; graph_.dirty = false;
        } else if constexpr (std::is_same_v<T, LoadSession>) {
            if (!saved_session_) return "No session snapshot saved";
            graph_ = *saved_session_;
        } else {
            auto* m = find_module(graph_, c.id);
            if (!m) return "Module is missing";
            if constexpr (std::is_same_v<T, RemoveModule>) {
                if (m->fixed) return "Endpoints cannot be deleted";
                std::erase_if(graph_.connections, [&](const Connection& edge) { return edge.from == c.id || edge.to == c.id; });
                std::erase_if(graph_.assignments, [&](const Assignment& a) { return a.module == c.id; });
                std::erase_if(graph_.modules, [&](const Module& item) { return item.id == c.id; });
            } else if constexpr (std::is_same_v<T, MoveModule>) {
                if (m->fixed) return "Endpoints are fixed";
                m->x = std::clamp(c.x, module_min_x, module_max_x);
                m->y = std::clamp(c.y, module_min_y, module_max_y);
            } else if constexpr (std::is_same_v<T, DuplicateModule>) {
                if (m->fixed) return "Endpoints cannot be duplicated";
                if (!module_available(*m)) return "Unavailable module cannot be duplicated";
                if (next_instance_ == std::numeric_limits<InstanceId>::max()) return "Instance ID space exhausted";
                auto copy = *m;
                copy.id = next_instance_++;
                copy.x = std::clamp(c.x, module_min_x, module_max_x);
                copy.y = std::clamp(c.y, module_min_y, module_max_y);
                graph_.modules.push_back(std::move(copy));
            } else if constexpr (std::is_same_v<T, SetBypass>) {
                if (m->fixed) return "Endpoints cannot be bypassed";
                if (!module_available(*m)) return "Module is unavailable";
                m->bypassed = c.value;
            } else if constexpr (std::is_same_v<T, SetParameter>) {
                if (!module_available(*m)) return "Module is unavailable";
                auto it = std::find_if(m->parameters.begin(), m->parameters.end(), [&](const Parameter& p) { return p.symbol == c.symbol; });
                if (it == m->parameters.end()) return "Parameter is missing";
                if (!std::isfinite(c.value) || c.value < it->minimum || c.value > it->maximum) return "Value is outside the parameter range";
                if (it->kind != ParameterKind::Continuous && c.value != std::round(c.value)) return "Parameter requires a discrete value";
                it->value = c.value;
            }
        }
        return {};
    }, command);
    if (error.empty() && !std::holds_alternative<LoadGraph>(command) && !std::holds_alternative<SaveGraph>(command) &&
        !std::holds_alternative<LoadSession>(command)) graph_.dirty = true;
    return error;
}
}
