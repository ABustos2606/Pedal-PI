// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#include "ui/application.hpp"
// v9.5.0 exposes the hit-test payload through this header. Keep this usage
// isolated and recheck it when upgrading the pinned LVGL dependency.
#include "src/core/lv_obj_event_private.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace pedal {
namespace {
constexpr std::uint32_t background = 0x10171f, surface = 0x1a2530, border = 0x2b3b4a;
constexpr std::uint32_t text = 0xe9eef2, muted = 0x94a7b8, accent = 0x8edbc2;
lv_color_t color(std::uint32_t c) { return lv_color_hex(c); }
lv_obj_t* panel(lv_obj_t* parent, int x, int y, int w, int h, std::uint32_t fill = surface) {
    auto* o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_pos(o, x, y); lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, color(fill), 0); lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(o, 12, 0);
    lv_obj_set_style_border_width(o, 1, 0); lv_obj_set_style_border_color(o, color(border), 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    return o;
}
lv_obj_t* label(lv_obj_t* parent, const std::string& value, int x, int y, int w = 0,
                std::uint32_t ink = text, const lv_font_t* font = &lv_font_montserrat_18) {
    auto* l = lv_label_create(parent); lv_label_set_text(l, value.c_str());
    lv_obj_set_pos(l, x, y); if (w) lv_obj_set_width(l, w);
    lv_obj_set_style_text_font(l, font, 0); lv_obj_set_style_text_color(l, color(ink), 0);
    lv_obj_remove_flag(l, LV_OBJ_FLAG_CLICKABLE);
    return l;
}
int snap(int value) { return static_cast<int>(std::round(value / 24.0)) * 24; }
std::string metric_text(std::optional<double> reading, bool cpu) {
    if (!reading || !std::isfinite(*reading) || (cpu ? *reading < 0 || *reading > 100 : *reading <= 0)) return "--";
    std::ostringstream value;
    value << std::fixed << std::setprecision(std::abs(*reading - std::round(*reading)) < 0.05 ? 0 : 1) << *reading;
    if (cpu) value << '%';
    return value.str();
}
enum class ModuleLayout { Generic, Clouds, Delay };
ModuleLayout module_layout(const Module& m) {
    // Match by stable URI and parameter schema, never by the editable name.
    // An unexpected/extended schema must keep every control in the generic view.
    const auto* d = definition(m.uri);
    if (!d || m.parameters.size() != d->parameters.size()) return ModuleLayout::Generic;
    for (const auto& expected : d->parameters) {
        const auto it = std::find_if(m.parameters.begin(), m.parameters.end(), [&](const Parameter& p) { return p.symbol == expected.symbol; });
        if (it == m.parameters.end() || it->kind != expected.kind || it->choices != expected.choices || !validate_parameter(*it).empty())
            return ModuleLayout::Generic;
    }
    if (m.uri == "urn:pedalpi:demo:clouds") return ModuleLayout::Clouds;
    if (m.uri == "urn:pedalpi:demo:delay") return ModuleLayout::Delay;
    return ModuleLayout::Generic;
}
// Vertical placement of one connector on its edge of the square. Every audio
// channel has its own connector, so a stereo port shows two (left above
// right). A single connector stays centred; several share the edge in equal
// touch bands.
struct Jack { int centre, height; };
std::optional<Jack> jack(const Module& m, const std::string& symbol, Direction direction, unsigned channel = 0) {
    int count = 0, index = -1;
    for (const auto& p : module_ports(m)) {
        if (p.type != PortType::Audio || p.direction != direction) continue;
        if (p.symbol == symbol && channel < p.channels) index = count + static_cast<int>(channel);
        count += static_cast<int>(p.channels);
    }
    if (index < 0) return {};
    return Jack{(2 * index + 1) * module_size / (2 * count), std::min(44, module_size / count)};
}
const Parameter& parameter(const Module& m, const std::string& symbol) {
    return *std::find_if(m.parameters.begin(), m.parameters.end(), [&](const Parameter& p) { return p.symbol == symbol; });
}
}
Application::Application(Engine& engine) : engine_(engine), graph_(engine.snapshot()) { rebuild(); }
Application::~Application() { if (screen_) lv_obj_delete(screen_); }
void Application::message(std::string value) {
    message_ = std::move(value);
    if (footer_) lv_label_set_text(footer_, message_.c_str());
}
void Application::navigate(Page page) { page_ = page; rebuild_ = true; }
RequestId Application::submit(Command command) {
    if (engine_.state() != EngineState::Ready) {
        message("Engine unavailable. Reconnect in Settings."); rebuild_ = true; return 0;
    }
    auto kind = std::holds_alternative<NewPatch>(command) || std::holds_alternative<LoadSession>(command) || std::holds_alternative<LoadGraph>(command) ? PendingKind::Replace :
        std::holds_alternative<RenamePatch>(command) ? PendingKind::Rename : PendingKind::Edit;
    const auto id = engine_.submit(std::move(command));
    pending_[id] = kind;
    message("Waiting for simulated engine...");
    return id;
}
void Application::stream_parameter(InstanceId id, const std::string& symbol, double value) {
    // A moving control must be heard while it moves. Send as fast as the
    // engine acknowledges, and never queue more than the newest value.
    if (stream_ && (stream_->id != id || stream_->symbol != symbol)) {
        if (stream_->sent != stream_->latest) submit(SetParameter{stream_->id, stream_->symbol, stream_->latest});
        stream_.reset();
    }
    if (!stream_) stream_ = Stream{id, symbol};
    stream_->latest = value;
    if (!stream_->in_flight) send_stream();
}
void Application::send_stream() {
    if (stream_->sent == stream_->latest) return;
    stream_->sent = stream_->latest;
    stream_->in_flight = submit(SetParameter{stream_->id, stream_->symbol, stream_->latest});
    if (!stream_->in_flight) stream_.reset();
}
void Application::tick(unsigned ms) {
    for (auto& e : engine_.advance(ms)) {
        const auto it = pending_.find(e.request);
        const auto kind = it == pending_.end() ? PendingKind::Edit : it->second;
        pending_.erase(e.request);
        if (stream_ && e.request && e.request == stream_->in_flight) { stream_->in_flight = 0; send_stream(); }
        if (e.snapshot) graph_ = *e.snapshot;
        if (e.saved_session) { saved_ = *e.saved_session; has_saved_ = true; }
        if (e.accepted && e.request && kind == PendingKind::Replace) {
            pan_x_ = pan_y_ = 0; selected_cable_.reset(); parameter_page_ = 0; navigate(Page::Patch);
        } else if (e.accepted && e.request && kind == PendingKind::Rename) navigate(Page::Patches);
        if (selected_cable_ && std::find(graph_.connections.begin(), graph_.connections.end(), *selected_cable_) == graph_.connections.end())
            selected_cable_.reset();
        if (!e.accepted) message(e.message);
        else if (!e.message.empty()) message(e.message);
        else if (e.request) message("Edit accepted by engine.");
        rebuild_ = true;
    }
    // Rebuilding during a held key can deliver its release to a new button.
    // Keep both pointer and keypad targets alive until their gesture ends.
    const bool pressed = (pointer_input_ && lv_indev_get_state(pointer_input_) == LV_INDEV_STATE_PRESSED) ||
        (keypad_input_ && lv_indev_get_state(keypad_input_) == LV_INDEV_STATE_PRESSED);
    if (stream_ && !stream_->in_flight && !pressed) stream_.reset();
    if (rebuild_ && !dragging_ && !panning_ && !cable_source_ && !pressed) rebuild();
    update_metrics();
}
void Application::update_metrics() {
    if (!tempo_value_ || !cpu_value_) return;
    const auto readings = engine_.telemetry();
    const auto tempo = metric_text(readings.tempo_bpm, false), cpu = metric_text(readings.cpu_percent, true);
    if (tempo != lv_label_get_text(tempo_value_)) lv_label_set_text(tempo_value_, tempo.c_str());
    if (cpu != lv_label_get_text(cpu_value_)) lv_label_set_text(cpu_value_, cpu.c_str());
    const auto tempo_ink = color(tempo == "--" ? muted : accent);
    const auto cpu_ink = color(cpu == "--" ? muted : *readings.cpu_percent >= 85 ? 0xf0aa72 : text);
    if (!lv_color_eq(lv_obj_get_style_text_color(tempo_value_, LV_PART_MAIN), tempo_ink))
        lv_obj_set_style_text_color(tempo_value_, tempo_ink, 0);
    if (!lv_color_eq(lv_obj_get_style_text_color(cpu_value_, LV_PART_MAIN), cpu_ink))
        lv_obj_set_style_text_color(cpu_value_, cpu_ink, 0);
}
void Application::bind(lv_obj_t* obj, Action action, InstanceId id, std::string symbol, int number, lv_obj_t* value) {
    bindings_.push_back({this, action, id, std::move(symbol), number, value});
    lv_obj_add_event_cb(obj, event, LV_EVENT_ALL, &bindings_.back());
}
lv_obj_t* Application::button(lv_obj_t* parent, const std::string& title, int x, int y, int w,
                              Action action, InstanceId id, std::string symbol, int number) {
    auto* b = lv_button_create(parent); lv_obj_remove_style_all(b);
    lv_obj_set_pos(b, x, y); lv_obj_set_size(b, w, 48);
    lv_obj_set_style_bg_color(b, color(surface), 0); lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(b, color(border), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(b, 1, 0); lv_obj_set_style_border_color(b, color(border), 0);
    lv_obj_set_style_radius(b, 10, 0);
    auto* l = label(b, title, 0, 0); lv_obj_center(l);
    bind(b, action, id, std::move(symbol), number);
    if (auto* group = lv_group_get_default()) lv_group_add_obj(group, b);
    return b;
}
void Application::event(lv_event_t* e) {
    auto* b = static_cast<Binding*>(lv_event_get_user_data(e));
    b->app->handle(*b, e);
}
void Application::handle(Binding& b, lv_event_t* e) {
    if (auto* input = lv_indev_active()) {
        if (lv_indev_get_type(input) == LV_INDEV_TYPE_POINTER) pointer_input_ = input;
        else if (lv_indev_get_type(input) == LV_INDEV_TYPE_KEYPAD) keypad_input_ = input;
    }
    const auto code = lv_event_get_code(e);
    auto* target = static_cast<lv_obj_t*>(lv_event_get_target(e));
    if (b.action == Action::NameInput) {
        if (code == LV_EVENT_VALUE_CHANGED) {
            draft_name_ = lv_textarea_get_text(target);
            message(renaming_ ? "Rename keeps modules, values and cables." : "New patch starts with IN connected to OUT.");
        } else if (code == LV_EVENT_READY) commit_name();
        return;
    }
    if (b.action == Action::Keyboard) {
        // The default keyboard forwards READY to its textarea. Commit there
        // once, which also handles physical Enter and the on-screen return key.
        if (code == LV_EVENT_CANCEL) navigate(Page::Patches);
        return;
    }
    if (b.action == Action::SelectCable && code == LV_EVENT_HIT_TEST) {
        auto* hit = static_cast<lv_hit_test_info_t*>(lv_event_get_param(e));
        hit->res = false;
        if (b.number < 0 || static_cast<std::size_t>(b.number) >= cables_.size()) return;
        const auto& cable = cables_[b.number];
        const double px = hit->point->x, py = hit->point->y - 72;
        for (int i = 0; i < 3; ++i) {
            const double ax = cable.points[i].x, ay = cable.points[i].y;
            const double dx = cable.points[i + 1].x - ax, dy = cable.points[i + 1].y - ay;
            const double length = dx * dx + dy * dy;
            const double t = length ? std::clamp(((px - ax) * dx + (py - ay) * dy) / length, 0.0, 1.0) : 0;
            const double ex = px - (ax + t * dx), ey = py - (ay + t * dy);
            if (ex * ex + ey * ey <= 144) { hit->res = true; break; }
        }
        return;
    }
    if (b.action == Action::Pan) {
        auto* indev = lv_indev_active(); if (!indev) return;
        lv_point_t point; lv_indev_get_point(indev, &point);
        if (code == LV_EVENT_PRESSED) {
            press_ = point; start_x_ = pan_x_; start_y_ = pan_y_; panning_ = true;
        } else if (code == LV_EVENT_PRESSING && panning_) {
            pan_x_ = std::clamp(start_x_ + press_.x - point.x, 0, canvas_width - 800);
            pan_y_ = std::clamp(start_y_ + press_.y - point.y, 0, canvas_height - 360);
            update_viewport();
        } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
            if (code == LV_EVENT_RELEASED && panning_) {
                pan_x_ = std::clamp(start_x_ + press_.x - point.x, 0, canvas_width - 800);
                pan_y_ = std::clamp(start_y_ + press_.y - point.y, 0, canvas_height - 360);
                update_viewport();
            }
            panning_ = false;
        } else if (code == LV_EVENT_CLICKED && std::abs(point.x - press_.x) + std::abs(point.y - press_.y) <= 8) {
            selected_cable_.reset(); rebuild_ = true;
        }
        return;
    }
    if (b.action == Action::Module || b.action == Action::Port) {
        auto* indev = lv_indev_active();
        if (!indev) return;
        lv_point_t point; lv_indev_get_point(indev, &point);
        const auto* m = find_module(graph_, b.id);
        if (!m) return;
        if (code == LV_EVENT_PRESSED) {
            press_ = point; start_x_ = m->x; start_y_ = m->y; dragging_ = false; long_pressed_ = false;
            if (b.action == Action::Port && (b.number & 1)) {
                cable_source_ = b.id; cable_port_ = b.symbol;
                const auto from = jack(*m, b.symbol, Direction::Output, static_cast<unsigned>(b.number >> 1));
                message("Cable from " + m->name + " / " + b.symbol + ". Drop it on an input jack.");
                preview_ = lv_line_create(canvas_); lv_obj_remove_flag(preview_, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_set_style_line_color(preview_, color(accent), 0); lv_obj_set_style_line_width(preview_, 3, 0);
                preview_points_[0] = {static_cast<lv_value_precise_t>(m->x + 96 - pan_x_), static_cast<lv_value_precise_t>(m->y + (from ? from->centre : 48) - pan_y_)};
                indicate_targets(true);
            }
        } else if (code == LV_EVENT_LONG_PRESSED && b.action == Action::Module && !m->fixed && !dragging_) {
            long_pressed_ = true; selected_ = b.id; navigate(Page::ModuleActions);
        } else if (code == LV_EVENT_PRESSING) {
            if (b.action == Action::Module && !m->fixed && !long_pressed_) {
                const int dx = point.x - press_.x, dy = point.y - press_.y;
                if (std::abs(dx) + std::abs(dy) > 8) dragging_ = true;
                if (dragging_) {
                    lv_obj_set_pos(target, std::clamp(start_x_ + dx, module_min_x, module_max_x) - pan_x_,
                        std::clamp(start_y_ + dy, module_min_y, module_max_y) - pan_y_);
                    update_cables();
                }
            } else if (cable_source_ && preview_) {
                preview_points_[1] = {static_cast<lv_value_precise_t>(point.x), static_cast<lv_value_precise_t>(point.y - 72)};
                lv_line_set_points(preview_, preview_points_, 2);
            }
        } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
            if (cable_source_) {
                // The nearest input jack under the finger wins; a module can have several.
                InstanceId destination = 0; std::string input; int nearest = 0;
                if (point.x >= 0 && point.x < 800 && point.y >= 72 && point.y < 432)
                    for (const auto& item : graph_.modules) for (const auto& port : module_ports(item)) for (unsigned channel = 0; channel < port.channels; ++channel) {
                        const auto drop = jack(item, port.symbol, Direction::Input, channel);
                        if (!drop || std::abs(point.x + pan_x_ - item.x) > 24) continue;
                        const int distance = std::abs(point.y - 72 + pan_y_ - item.y - drop->centre);
                        if (distance > drop->height / 2 + 2 || (destination && distance >= nearest)) continue;
                        destination = item.id; input = port.symbol; nearest = distance;
                    }
                if (destination && code == LV_EVENT_RELEASED) submit(Connect{{cable_source_, destination, cable_port_, input}});
                else message("Drop the cable on an input jack.");
                if (preview_) lv_obj_delete(preview_);
                preview_ = nullptr; cable_source_ = 0; indicate_targets(false);
            } else if (dragging_) {
                // Release can contain a final pointer sample without a preceding
                // PRESSING event. Commit that point, rather than the last frame.
                if (code == LV_EVENT_RELEASED)
                    submit(MoveModule{b.id, snap(start_x_ + point.x - press_.x), snap(start_y_ + point.y - press_.y)});
                rebuild_ = true;
                dragging_ = false;
            }
        } else if (code == LV_EVENT_CLICKED && b.action == Action::Module && !m->fixed) {
            // A completed drag is followed by CLICKED; distinguish it using displacement.
            if (!long_pressed_ && std::abs(point.x - press_.x) + std::abs(point.y - press_.y) <= 8) {
                selected_ = b.id; parameter_page_ = 0; generic_parameters_ = false; navigate(Page::Parameters);
            }
        }
        return;
    }
    if (b.action == Action::Parameter) {
        const auto* m = find_module(graph_, b.id); if (!m) return;
        auto it = std::find_if(m->parameters.begin(), m->parameters.end(), [&](const Parameter& p) { return p.symbol == b.symbol; });
        if (it == m->parameters.end()) return;
        auto p = *it;
        if (code == LV_EVENT_VALUE_CHANGED || code == LV_EVENT_RELEASED) {
            if (p.kind == ParameterKind::Toggle) p.value = lv_obj_has_state(target, LV_STATE_CHECKED) ? 1 : 0;
            else if (p.kind == ParameterKind::Enumeration) p.value = lv_dropdown_get_selected(target);
            else p.value = value_from_normalized(p, lv_slider_get_value(target) / 1000.0);
            if (b.value_label) lv_label_set_text(b.value_label, format_value(p).c_str());
            const bool discrete = p.kind == ParameterKind::Toggle || p.kind == ParameterKind::Enumeration;
            if (discrete) { if (code == LV_EVENT_VALUE_CHANGED) submit(SetParameter{b.id, b.symbol, p.value}); }
            // Release sends the final position; a touch that moved nothing is not an edit.
            else if (code == LV_EVENT_VALUE_CHANGED || stream_ || p.value != it->value) stream_parameter(b.id, b.symbol, p.value);
        }
        return;
    }
    if (code != LV_EVENT_CLICKED) return;
    switch (b.action) {
    case Action::Navigate: navigate(static_cast<Page>(b.number)); break;
    case Action::Add:
    case Action::Duplicate: {
        if (const auto slot = vacant_slot()) {
            const auto [x, y] = *slot;
            if (b.action == Action::Add) submit(AddModule{b.symbol, x, y});
            else submit(DuplicateModule{b.id, x, y});
            reveal(x, y); navigate(Page::Patch);
        } else message("No free module slot remains on this canvas.");
        break;
    }
    case Action::SelectCable:
        if (b.number >= 0 && static_cast<std::size_t>(b.number) < cables_.size()) {
            selected_cable_ = cables_[b.number].connection; rebuild_ = true;
            message("Cable selected. Use Delete cable to remove it.");
        }
        break;
    case Action::DeleteCable: if (selected_cable_) submit(Disconnect{*selected_cable_}); break;
    case Action::ClearCable: selected_cable_.reset(); rebuild_ = true; break;
    case Action::Home: pan_x_ = pan_y_ = 0; rebuild_ = true; break;
    case Action::OpenActions: selected_ = b.id; navigate(Page::ModuleActions); break;
    case Action::Edit: selected_ = b.id; parameter_page_ = 0; generic_parameters_ = false; navigate(Page::Parameters); break;
    case Action::Bypass: {
        const auto* m = find_module(graph_, b.id); if (m) submit(SetBypass{b.id, !m->bypassed}); break;
    }
    case Action::Remove: submit(RemoveModule{b.id}); navigate(Page::Patch); break;
    case Action::SaveSession:
        if (pending()) { message("Wait for pending edits before saving."); break; }
        submit(SaveGraph{}); break;
    case Action::LoadSession:
        if (has_saved_) replace_patch(LoadSession{}, Page::Patches);
        else message("Save a session snapshot first.");
        break;
    case Action::ParallelDemo: replace_patch(LoadGraph{parallel_demo_graph()}, Page::Patches); break;
    case Action::ParameterLayout: generic_parameters_ = !generic_parameters_; parameter_page_ = 0; rebuild_ = true; break;
    case Action::ParameterChoice: submit(SetParameter{b.id, b.symbol, static_cast<double>(b.number)}); break;
    case Action::State:
        if (static_cast<EngineState>(b.number) == EngineState::Ready) engine_.connect();
        else if (static_cast<EngineState>(b.number) == EngineState::Disconnected) engine_.disconnect();
        else if (!engine_.simulate_state(EngineState::Error)) message("Unsupported capability: simulated engine error");
        break;
    case Action::Reject:
        message(engine_.simulate_rejection() ? "The next engine edit will be rejected." : "Unsupported capability: simulated rejection"); break;
    case Action::MissingFixture:
        if (engine_.simulate_missing_module()) navigate(Page::Patch);
        else message("Missing-module fixture requires a ready simulator with no pending edit.");
        break;
    case Action::NameEntry:
        if (pending()) { message("Wait for pending edits before naming a patch."); break; }
        renaming_ = b.number == 1; draft_name_ = renaming_ ? graph_.name : "";
        message(renaming_ ? "Rename keeps modules, values and cables." : "New patch starts with IN connected to OUT.");
        navigate(Page::NamePatch); break;
    case Action::CommitName: commit_name(); break;
    case Action::Confirm:
        if (replacement_ && !pending()) submit(*replacement_);
        else message("Wait for pending edits before replacing the patch.");
        break;
    case Action::CancelReplace: replacement_.reset(); navigate(confirm_origin_); break;
    case Action::ParameterPage: parameter_page_ = static_cast<std::size_t>(b.number); rebuild_ = true; break;
    case Action::BrowserPage: browser_page_ = static_cast<std::size_t>(b.number); rebuild_ = true; break;
    case Action::AvailabilityInfo:
        if (const auto* d = definition(b.symbol)) message(d->availability_reason);
        break;
    default: break;
    }
}
void Application::commit_name() {
    if (pending()) { message("Wait for pending edits before naming a patch."); return; }
    if (auto issue = validate_patch_name(draft_name_); !issue.empty()) { message(issue); return; }
    if (renaming_) submit(RenamePatch{draft_name_});
    else replace_patch(NewPatch{draft_name_}, Page::NamePatch);
}
void Application::replace_patch(Command command, Page origin) {
    if (pending()) { message("Wait for pending edits before replacing the patch."); return; }
    if (graph_.dirty) {
        replacement_ = std::move(command); confirm_origin_ = origin;
        message("Choose Replace patch to discard unsaved edits, or Cancel."); navigate(Page::ConfirmReplace);
    }
    else submit(std::move(command));
}
void Application::rebuild() {
    rebuild_ = false;
    if (!screen_) {
        screen_ = lv_obj_create(nullptr);
        lv_obj_remove_style_all(screen_); lv_obj_set_size(screen_, 800, 480);
        lv_obj_set_style_bg_color(screen_, color(background), 0); lv_obj_set_style_bg_opa(screen_, LV_OPA_COVER, 0);
        lv_screen_load(screen_);
    } else lv_obj_clean(screen_);
    bindings_.clear(); module_widgets_.clear(); input_jacks_.clear(); dots_.clear(); cables_.clear(); canvas_ = nullptr; footer_ = nullptr;
    tempo_value_ = cpu_value_ = nullptr;
    auto* header = panel(screen_, 0, 0, 800, 72, background);
    lv_obj_set_style_radius(header, 0, 0);
    const bool patch = page_ == Page::Patch;
    if (patch) {
        label(header, "PEDAL / PI", 24, 10, 0, accent, &lv_font_montserrat_14);
        auto* title = label(header, graph_.name + (graph_.dirty ? " *" : ""), 24, 32, 200, text, &lv_font_montserrat_24);
        lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
        lv_obj_set_height(title, 28);
        button(header, "Home", 244, 12, 64, Action::Home);
        const std::string status = engine_.state() == EngineState::Ready ? (engine_.capabilities().simulation ? "SIMULATED" : "READY") :
            engine_.state() == EngineState::Disconnected ? "DISCONNECTED" : engine_.state() == EngineState::Connecting ? "CONNECTING" : "ENGINE ERROR";
        label(header, status, 318, 28, 0, engine_.state() == EngineState::Ready ? accent : 0xf0aa72, &lv_font_montserrat_14);
        button(header, "+ Add", 448, 12, 96, Action::Navigate, 0, {}, static_cast<int>(Page::Browser));
        button(header, "Patches", 556, 12, 104, Action::Navigate, 0, {}, static_cast<int>(Page::Patches));
        button(header, "Settings", 672, 12, 104, Action::Navigate, 0, {}, static_cast<int>(Page::Settings));
    } else {
        button(header, "< Back", 16, 12, 104, Action::Navigate, 0, {}, static_cast<int>(page_ == Page::NamePatch ? Page::Patches : Page::Patch));
        std::string title = page_ == Page::Browser ? "Add a module" : page_ == Page::Patches ? "Session patches" : page_ == Page::Settings ? "Simulator settings" : "Parameters";
        if (page_ == Page::NamePatch) title = renaming_ ? "Rename patch" : "New patch";
        if (page_ == Page::ConfirmReplace) title = "Replace current patch?";
        if (page_ == Page::Parameters || page_ == Page::ModuleActions)
            if (const auto* m = find_module(graph_, selected_)) title = m->name + (page_ == Page::ModuleActions ? " actions" : "");
        auto* heading = label(header, title, 144, 23, 480, text, &lv_font_montserrat_24);
        lv_label_set_long_mode(heading, LV_LABEL_LONG_DOT);
        lv_obj_set_height(heading, 30);
        if (page_ == Page::NamePatch) button(header, renaming_ ? "Rename" : "Create", 640, 12, 136, Action::CommitName);
        else label(header, engine_.capabilities().simulation ? "SIMULATED" : "ENGINE", 660, 28, 0, accent, &lv_font_montserrat_14);
    }
    auto* content = panel(screen_, 0, 72, 800, 360, background);
    lv_obj_set_style_border_width(content, 0, 0); lv_obj_set_style_radius(content, 0, 0);
    switch (page_) {
    case Page::Patch: patch_view(content); break;
    case Page::Browser: browser(content); break;
    case Page::Parameters: parameters(content); break;
    case Page::Patches: patches(content); break;
    case Page::Settings: settings(content); break;
    case Page::ModuleActions: module_actions(content); break;
    case Page::NamePatch: name_patch(content); break;
    case Page::ConfirmReplace: confirm_replace(content); break;
    }
    auto* foot = panel(screen_, 0, 432, 800, 48, background); lv_obj_set_style_radius(foot, 0, 0);
    footer_ = label(foot, message_, 24, 7, patch && selected_cable_ ? 240 : 568, muted, &lv_font_montserrat_14);
    lv_obj_set_height(footer_, 34); lv_label_set_long_mode(footer_, LV_LABEL_LONG_DOT);
    if (patch && selected_cable_) {
        button(foot, "Deselect", 280, 0, 128, Action::ClearCable);
        button(foot, "Delete cable", 424, 0, 168, Action::DeleteCable);
    }
    auto* metrics = panel(foot, 608, 4, 168, 40);
    lv_obj_remove_flag(metrics, LV_OBJ_FLAG_CLICKABLE);
    tempo_value_ = label(metrics, "--", 12, 0, 68, accent, &lv_font_montserrat_20);
    cpu_value_ = label(metrics, "--", 88, 0, 72, text, &lv_font_montserrat_20);
    for (auto* value : {tempo_value_, cpu_value_}) {
        lv_obj_set_height(value, 23); lv_label_set_long_mode(value, LV_LABEL_LONG_DOT);
    }
    label(metrics, "BPM", 12, 23, 68, muted, &lv_font_montserrat_14);
    label(metrics, engine_.capabilities().simulation ? "CPU SIM" : "CPU", 88, 23, 72, muted, &lv_font_montserrat_14);
    update_metrics();
    lv_obj_update_layout(screen_);
}
void Application::patch_view(lv_obj_t* parent) {
    canvas_ = parent;
    lv_obj_add_flag(parent, LV_OBJ_FLAG_PRESS_LOCK);
    bind(parent, Action::Pan);
    for (int y = 12; y < 350; y += 24) for (int x = 24; x < 790; x += 24) {
        auto* dot = panel(parent, x, y, 2, 2, border); lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_remove_flag(dot, LV_OBJ_FLAG_CLICKABLE);
        dots_.push_back(dot);
    }
    for (const auto& edge : graph_.connections) {
        const bool selected = selected_cable_ && *selected_cable_ == edge;
        const auto routing = expand_connection(graph_, edge);
        // Draw every channel: stereo to stereo is two lines, mono to stereo
        // fans out to both connectors, stereo to mono lands on one.
        unsigned out_channels = 1, in_channels = 1;
        if (const auto* source = find_module(graph_, edge.from)) for (const auto& p : module_ports(*source))
            if (p.symbol == edge.output && p.direction == Direction::Output) out_channels = p.channels;
        if (const auto* sink = find_module(graph_, edge.to)) for (const auto& p : module_ports(*sink))
            if (p.symbol == edge.input && p.direction == Direction::Input) in_channels = p.channels;
        for (unsigned channel = 0; channel < std::max(out_channels, in_channels); ++channel) {
        auto* line = lv_line_create(parent);
        lv_obj_set_style_line_color(line, color(selected || !routing.error.empty() ? 0xf0aa72 : accent), 0);
        lv_obj_set_style_line_width(line, selected ? 6 : 4, 0);
        lv_obj_set_style_line_rounded(line, true, 0);
        lv_obj_add_flag(line, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(line, LV_OBJ_FLAG_ADV_HITTEST);
        lv_obj_set_ext_click_area(line, 12);
        bind(line, Action::SelectCable, 0, {}, static_cast<int>(cables_.size()));
        cables_.push_back({line, edge, {}, routing, out_channels == 1 ? 0 : channel, in_channels == 1 ? 0 : channel});
        }
    }
    for (const auto& m : graph_.modules) {
        const auto* d = definition(m.uri);
        const auto ink = module_available(m) && d ? d->color : 0xf0aa72;
        auto* card = panel(parent, m.x - pan_x_, m.y - pan_y_, 96, 96);
        lv_obj_set_style_border_color(card, color(ink), 0); lv_obj_set_style_border_width(card, 2, 0);
        lv_obj_set_style_bg_color(card, color(m.bypassed ? background : surface), 0);
        lv_obj_set_style_shadow_width(card, 14, 0); lv_obj_set_style_shadow_opa(card, LV_OPA_20, 0);
        lv_obj_set_style_shadow_color(card, color(0x000000), 0);
        const bool long_name = m.name.size() > 10;
        auto* name = label(card, m.name, 8, long_name ? 16 : 30, 80, ink, long_name ? &lv_font_montserrat_18 : &lv_font_montserrat_20);
        lv_label_set_long_mode(name, LV_LABEL_LONG_DOT); lv_obj_set_height(name, long_name ? 48 : 28);
        lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
        const auto& ports = module_ports(m);
        auto* sub = label(card, !module_available(m) ? "MISSING" : m.fixed ? "AUDIO" : m.bypassed ? "BYPASS" : "", 0, 72, 96, muted, &lv_font_montserrat_14);
        lv_obj_set_style_text_align(sub, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_add_flag(card, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        lv_obj_add_flag(card, LV_OBJ_FLAG_PRESS_LOCK);
        bind(card, Action::Module, m.id);
        module_widgets_[m.id] = card;
        for (const auto& port : ports) {
            if (port.type != PortType::Audio) continue;
            const bool output = port.direction == Direction::Output;
            for (unsigned channel = 0; channel < port.channels; ++channel) {
            auto* hit = lv_obj_create(card); lv_obj_remove_style_all(hit);
            const auto place = *jack(m, port.symbol, port.direction, channel);
            lv_obj_set_size(hit, 44, place.height); lv_obj_set_pos(hit, output ? 74 : -22, place.centre - place.height / 2);
            lv_obj_remove_flag(hit, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(hit, LV_OBJ_FLAG_PRESS_LOCK);
            auto* dot = panel(hit, 15, place.height / 2 - 7, 14, 14, background);
            lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(dot, 3, 0); lv_obj_set_style_border_color(dot, color(ink), 0);
            lv_obj_remove_flag(dot, LV_OBJ_FLAG_CLICKABLE);
            // The connector's channel rides above the direction bit.
            bind(hit, Action::Port, m.id, port.symbol, (output ? 1 : 0) | static_cast<int>(channel << 1));
            if (!output) input_jacks_.emplace(std::pair{m.id, port.symbol}, dot);
            }
        }
    }
    update_viewport();
}
void Application::update_viewport() {
    for (const auto& m : graph_.modules) {
        const auto it = module_widgets_.find(m.id);
        if (it != module_widgets_.end()) lv_obj_set_pos(it->second, m.x - pan_x_, m.y - pan_y_);
    }
    std::size_t i = 0;
    for (int y = 12; y < 350; y += 24) for (int x = 24; x < 790; x += 24)
        lv_obj_set_pos(dots_[i++], x - pan_x_ % 24, y - pan_y_ % 24);
    update_cables();
}
void Application::indicate_targets(bool active) {
    for (const auto& [key, jack] : input_jacks_) {
        const auto& [id, symbol] = key;
        const auto* m = find_module(graph_, id); if (!m) continue;
        const auto* d = definition(m->uri);
        const bool valid = active && validate_connection(graph_, {cable_source_, id, cable_port_, symbol}).empty();
        lv_obj_set_style_border_color(jack, color(active ? (valid ? accent : border) : module_available(*m) && d ? d->color : 0xf0aa72), 0);
        lv_obj_set_style_outline_color(jack, color(accent), 0);
        lv_obj_set_style_outline_width(jack, valid ? 4 : 0, 0);
        lv_obj_set_style_outline_opa(jack, LV_OPA_COVER, 0);
    }
}
std::optional<std::pair<int, int>> Application::vacant_slot() const {
    // Prefer the current viewport, then search the larger canvas.
    for (int visible = 1; visible >= 0; --visible)
        for (int y = module_min_y; y <= module_max_y; y += 120)
            for (int x = module_min_x; x <= module_max_x; x += 144) {
                if (visible && (x < pan_x_ + 24 || x + 96 > pan_x_ + 776 || y < pan_y_ + 12 || y + 96 > pan_y_ + 360)) continue;
                const bool occupied = std::any_of(graph_.modules.begin(), graph_.modules.end(), [&](const Module& m) {
                    return std::abs(m.x - x) < 120 && std::abs(m.y - y) < 110;
                });
                if (!occupied) return std::pair{x, y};
            }
    return {};
}
void Application::reveal(int x, int y) {
    if (x < pan_x_ + 24 || x + 96 > pan_x_ + 776) pan_x_ = std::clamp(x - 24, 0, canvas_width - 800);
    if (y < pan_y_ + 12 || y + 96 > pan_y_ + 360) pan_y_ = std::clamp(y - 12, 0, canvas_height - 360);
}
void Application::update_cables() {
    // LVGL positions are lazy; resolve the cards before reading coordinates.
    if (canvas_) lv_obj_update_layout(canvas_);
    for (auto& c : cables_) {
        auto a = module_widgets_.find(c.connection.from), b = module_widgets_.find(c.connection.to);
        if (a == module_widgets_.end() || b == module_widgets_.end()) continue;
        const auto* source = find_module(graph_, c.connection.from); const auto* sink = find_module(graph_, c.connection.to);
        const auto out = source ? jack(*source, c.connection.output, Direction::Output, c.out_channel) : std::nullopt;
        const auto in = sink ? jack(*sink, c.connection.input, Direction::Input, c.in_channel) : std::nullopt;
        const int ax = lv_obj_get_x(a->second) + 96, ay = lv_obj_get_y(a->second) + (out ? out->centre : 48);
        const int bx = lv_obj_get_x(b->second), by = lv_obj_get_y(b->second) + (in ? in->centre : 48);
        // Keep the two channels of a pair from sharing a vertical run or crossing.
        const int lane = !out || out->centre == module_size / 2 ? 0 : out->centre < module_size / 2 ? 6 : -6;
        const int mid = (ax + bx) / 2 + (by > ay ? lane : by < ay ? -lane : 0);
        c.points[0] = {static_cast<lv_value_precise_t>(ax), static_cast<lv_value_precise_t>(ay)};
        c.points[1] = {static_cast<lv_value_precise_t>(mid), static_cast<lv_value_precise_t>(ay)};
        c.points[2] = {static_cast<lv_value_precise_t>(mid), static_cast<lv_value_precise_t>(by)};
        c.points[3] = {static_cast<lv_value_precise_t>(bx), static_cast<lv_value_precise_t>(by)};
        lv_line_set_points(c.line, c.points, 4);
    }
}
void Application::browser(lv_obj_t* parent) {
    std::vector<const ModuleDefinition*> entries;
    for (const auto& d : catalogue()) if (d.category != "Endpoint") entries.push_back(&d);
    const auto count = (entries.size() + 3) / 4;
    browser_page_ = std::min(browser_page_, count - 1);
    label(parent, "Demo + reference metadata / no plugin discovery", 24, 12, 440, muted, &lv_font_montserrat_14);
    auto* previous = button(parent, "<", 472, 0, 48, Action::BrowserPage, 0, {}, static_cast<int>(browser_page_ ? browser_page_ - 1 : 0));
    if (!browser_page_) lv_obj_add_state(previous, LV_STATE_DISABLED);
    label(parent, std::to_string(browser_page_ + 1) + " / " + std::to_string(count), 536, 14, 80, muted, &lv_font_montserrat_14);
    auto* next = button(parent, ">", 632, 0, 48, Action::BrowserPage, 0, {}, static_cast<int>(std::min(browser_page_ + 1, count - 1)));
    if (browser_page_ + 1 >= count) lv_obj_add_state(next, LV_STATE_DISABLED);
    int y = 44;
    for (std::size_t i = browser_page_ * 4; i < std::min(entries.size(), browser_page_ * 4 + 4); ++i) {
        const auto& d = *entries[i];
        auto* row = panel(parent, 24, y, 752, 66);
        if (d.availability != Availability::Available) bind(row, Action::AvailabilityInfo, 0, d.uri);
        auto* mark = panel(row, 14, 14, 5, 38, d.color); lv_obj_set_style_border_width(mark, 0, 0);
        label(row, d.name, 34, 8, 280, d.color, &lv_font_montserrat_20);
        auto* description = label(row, d.category + " / " + d.description, 34, 36, 560, muted, &lv_font_montserrat_14);
        lv_label_set_long_mode(description, LV_LABEL_LONG_DOT);
        label(row, d.availability == Availability::Available ? (d.reference ? "REFERENCE / simulated" : "DEMO / available") : d.availability == Availability::Untested ? "Untested" : "Unavailable",
              400, 10, 225, d.availability == Availability::Available ? accent : 0xf0aa72, &lv_font_montserrat_14);
        auto* add = button(row, d.availability == Availability::Available ? "+ Add" : "Disabled", 638, 9, 96, Action::Add, 0, d.uri);
        if (d.availability != Availability::Available) { lv_obj_add_state(add, LV_STATE_DISABLED); lv_obj_set_style_opa(add, LV_OPA_50, 0); }
        y += 76;
    }
}
void Application::parameters(lv_obj_t* parent) {
    const auto* m = find_module(graph_, selected_);
    if (!m) { label(parent, "Module no longer exists", 24, 24); return; }
    if (!module_available(*m)) {
        button(parent, "Delete module", 24, 12, 200, Action::Remove, m->id);
        label(parent, "Module unavailable", 24, 92, 0, 0xf0aa72, &lv_font_montserrat_24);
        label(parent, "Saved values and cables are retained for repair.\nParameters, bypass and new connections are disabled.\nYou can move or delete the placeholder.", 24, 136, 752, muted);
        auto* uri = label(parent, m->uri, 24, 244, 752, muted, &lv_font_montserrat_14);
        lv_label_set_long_mode(uri, LV_LABEL_LONG_DOT); return;
    }
    const auto layout = module_layout(*m);
    if (!generic_parameters_ && layout == ModuleLayout::Clouds) { clouds_parameters(parent, *m); return; }
    if (!generic_parameters_ && layout == ModuleLayout::Delay) { delay_parameters(parent, *m); return; }
    auto* bypass = button(parent, m->bypassed ? "Enable" : "Bypass", 24, 12, 104, Action::Bypass, m->id);
    (void)bypass;
    button(parent, "Delete", 140, 12, 104, Action::Remove, m->id);
    const auto page_count = std::max<std::size_t>(1, (m->parameters.size() + 3) / 4);
    parameter_page_ = std::min(parameter_page_, page_count - 1);
    if (page_count > 1) {
        auto* prev = button(parent, "<", 272, 12, 48, Action::ParameterPage, 0, {}, static_cast<int>(parameter_page_ ? parameter_page_ - 1 : 0));
        if (!parameter_page_) lv_obj_add_state(prev, LV_STATE_DISABLED);
        label(parent, std::to_string(parameter_page_ + 1) + " / " + std::to_string(page_count), 348, 27, 104, muted, &lv_font_montserrat_14);
        auto* next = button(parent, ">", 472, 12, 48, Action::ParameterPage, 0, {}, static_cast<int>(std::min(parameter_page_ + 1, page_count - 1)));
        if (parameter_page_ + 1 >= page_count) lv_obj_add_state(next, LV_STATE_DISABLED);
        if (layout == ModuleLayout::Generic) label(parent, "Software\nbypass", 536, 18, 100, muted, &lv_font_montserrat_14);
    } else label(parent, "Demo values", 272, 27, 0, muted, &lv_font_montserrat_14);
    if (layout != ModuleLayout::Generic) button(parent, "Custom", 536, 12, 96, Action::ParameterLayout);
    button(parent, "Actions", 640, 12, 136, Action::OpenActions, m->id);
    const auto begin = parameter_page_ * 4, end = std::min(m->parameters.size(), begin + 4);
    for (std::size_t i = begin; i < end; ++i) {
        const auto& p = m->parameters[i];
        const auto slot = i - begin;
        const int x = 24 + static_cast<int>(slot % 2) * 384, y = 76 + static_cast<int>(slot / 2) * 136;
        parameter_tile(parent, *m, p, x, y);
    }
}
void Application::parameter_tile(lv_obj_t* parent, const Module& m, const Parameter& p, int x, int y, int height, std::uint32_t ink) {
    auto* tile = panel(parent, x, y, 368, height);
    label(tile, p.name, 20, 12, 150);
    auto* value = label(tile, format_value(p), 180, 12, 168, ink, &lv_font_montserrat_20);
    lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_t* control = nullptr;
    if (p.kind == ParameterKind::Toggle) {
        control = lv_switch_create(tile); lv_obj_set_pos(control, 20, 54); lv_obj_set_size(control, 80, 44);
        lv_obj_set_style_bg_color(control, color(border), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(control, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_radius(control, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_bg_color(control, color(text), LV_PART_KNOB);
        lv_obj_set_style_bg_opa(control, LV_OPA_COVER, LV_PART_KNOB);
        lv_obj_set_style_radius(control, LV_RADIUS_CIRCLE, LV_PART_KNOB);
        lv_obj_set_style_pad_all(control, -4, LV_PART_KNOB);
        if (p.value >= 0.5) lv_obj_add_state(control, LV_STATE_CHECKED);
        const auto checked_indicator = static_cast<lv_style_selector_t>(LV_PART_INDICATOR) | static_cast<lv_style_selector_t>(LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(control, color(ink), checked_indicator);
        lv_obj_set_style_bg_opa(control, LV_OPA_COVER, checked_indicator);
        lv_obj_set_style_radius(control, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    } else if (p.kind == ParameterKind::Enumeration) {
        control = lv_dropdown_create(tile); lv_obj_set_pos(control, 20, 54); lv_obj_set_size(control, 328, 48);
        std::string options; for (const auto& choice : p.choices) { if (!options.empty()) options += '\n'; options += choice; }
        lv_dropdown_set_options(control, options.c_str()); lv_dropdown_set_selected(control, static_cast<unsigned>(p.value));
        lv_obj_set_style_text_font(control, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(control, color(text), 0); lv_obj_set_style_bg_color(control, color(border), 0);
        lv_obj_set_style_bg_opa(control, LV_OPA_COVER, 0); lv_obj_set_style_pad_all(control, 12, 0);
        auto* list = lv_dropdown_get_list(control); lv_obj_set_style_bg_color(list, color(surface), 0); lv_obj_set_style_text_color(list, color(text), 0);
    } else {
        control = lv_slider_create(tile); lv_obj_set_pos(control, 30, height < 110 ? 48 : 70); lv_obj_set_size(control, 308, 12);
        lv_slider_set_range(control, 0, 1000); lv_slider_set_value(control, static_cast<int>(normalized_value(p) * 1000), LV_ANIM_OFF);
        lv_obj_set_style_bg_color(control, color(border), LV_PART_MAIN); lv_obj_set_style_bg_opa(control, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(control, color(ink), LV_PART_INDICATOR); lv_obj_set_style_bg_opa(control, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(control, color(text), LV_PART_KNOB); lv_obj_set_style_pad_all(control, 10, LV_PART_KNOB);
        lv_obj_set_style_bg_opa(control, LV_OPA_COVER, LV_PART_KNOB); lv_obj_set_style_radius(control, LV_RADIUS_CIRCLE, LV_PART_KNOB);
        lv_obj_set_ext_click_area(control, 20);
    }
    if (auto* group = lv_group_get_default()) lv_group_add_obj(group, control);
    bind(control, Action::Parameter, m.id, p.symbol, 0, value);
}
void Application::clouds_parameters(lv_obj_t* parent, const Module& m) {
    constexpr std::uint32_t ink = 0xbfa1ed;
    parameter_page_ = std::min<std::size_t>(parameter_page_, 1);
    button(parent, m.bypassed ? "Enable" : "Bypass", 24, 12, 104, Action::Bypass, m.id);
    button(parent, "Delete", 140, 12, 104, Action::Remove, m.id);
    auto* texture = button(parent, "Texture", 264, 12, 128, Action::ParameterPage, 0, {}, 0);
    auto* performance = button(parent, "Mix / hold", 408, 12, 136, Action::ParameterPage, 0, {}, 1);
    lv_obj_set_style_border_color(parameter_page_ ? performance : texture, color(ink), 0);
    lv_obj_set_style_border_width(parameter_page_ ? performance : texture, 2, 0);
    button(parent, "All", 552, 12, 72, Action::ParameterLayout);
    button(parent, "Actions", 640, 12, 136, Action::OpenActions, m.id);
    if (!parameter_page_) {
        const char* symbols[] = {"position", "size", "density", "pitch"};
        for (int i = 0; i < 4; ++i)
            parameter_tile(parent, m, parameter(m, symbols[i]), 24 + (i % 2) * 384, 76 + (i / 2) * 136, 120, ink);
    } else {
        label(parent, "Blend the texture. Hold it with Freeze.", 24, 80, 752, ink, &lv_font_montserrat_20);
        parameter_tile(parent, m, parameter(m, "mix"), 24, 120, 120, ink);
        parameter_tile(parent, m, parameter(m, "freeze"), 408, 120, 120, ink);
        label(parent, "Freeze is a toggle; Mix keeps its own value.\nThese are simulator controls, with no audio processing yet.", 24, 268, 752, muted);
    }
}
void Application::delay_parameters(lv_obj_t* parent, const Module& m) {
    constexpr std::uint32_t ink = 0x7bbaed;
    button(parent, m.bypassed ? "Enable" : "Bypass", 24, 12, 104, Action::Bypass, m.id);
    button(parent, "Delete", 140, 12, 104, Action::Remove, m.id);
    label(parent, "Echo timing and character", 264, 27, 276, muted, &lv_font_montserrat_14);
    button(parent, "All", 552, 12, 72, Action::ParameterLayout);
    button(parent, "Actions", 640, 12, 136, Action::OpenActions, m.id);
    const auto& mode = parameter(m, "mode");
    label(parent, "CHARACTER", 24, 76, 300, ink, &lv_font_montserrat_14);
    for (std::size_t i = 0; i < mode.choices.size(); ++i) {
        auto* choice = button(parent, mode.choices[i], 24 + static_cast<int>(i) * 256, 100, 240, Action::ParameterChoice, m.id, mode.symbol, static_cast<int>(i));
        if (mode.value == static_cast<double>(i)) {
            lv_obj_set_style_border_color(choice, color(ink), 0); lv_obj_set_style_border_width(choice, 2, 0);
            lv_obj_set_style_text_color(lv_obj_get_child(choice, 0), color(ink), 0);
        }
    }
    parameter_tile(parent, m, parameter(m, "time"), 24, 176, 156, ink);
    label(parent, "Short echoes to long repeats", 44, 288, 328, muted, &lv_font_montserrat_14);
    parameter_tile(parent, m, parameter(m, "feedback"), 408, 176, 72, ink);
    parameter_tile(parent, m, parameter(m, "mix"), 408, 260, 72, ink);
}
void Application::patches(lv_obj_t* parent) {
    label(parent, "Keep a snapshot while exploring", 24, 24, 0, text, &lv_font_montserrat_24);
    label(parent, "Session storage is in memory. Closing the app clears it.\nDisk persistence and plugin state arrive in Phase 5.", 24, 68, 710, muted);
    auto* row = panel(parent, 24, 140, 752, 100);
    auto* name = label(row, has_saved_ ? saved_.name : "No session snapshot yet", 20, 20, 560, text, &lv_font_montserrat_24);
    lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
    lv_obj_set_height(name, 30);
    label(row, has_saved_ ? std::to_string(saved_.modules.size()) + " modules / session only" : "Save the current graph to try restoring it", 20, 60, 440, muted, &lv_font_montserrat_14);
    button(row, "Load", 616, 26, 112, Action::LoadSession);
    button(parent, "Save snapshot", 24, 268, 200, Action::SaveSession);
    button(parent, "New patch", 240, 268, 168, Action::NameEntry, 0, {}, 0);
    button(parent, "Rename", 424, 268, 168, Action::NameEntry, 0, {}, 1);
    button(parent, "Parallel demo", 608, 268, 168, Action::ParallelDemo);
}
void Application::name_patch(lv_obj_t* parent) {
    auto* input = lv_textarea_create(parent); lv_obj_set_pos(input, 24, 20); lv_obj_set_size(input, 752, 56);
    lv_textarea_set_one_line(input, true);
    // Preserve a backend name longer than the limit for newly typed text.
    lv_textarea_set_text(input, draft_name_.c_str()); lv_textarea_set_max_length(input, 48);
    lv_textarea_set_placeholder_text(input, "Patch name");
    lv_obj_set_style_text_font(input, &lv_font_montserrat_24, 0); lv_obj_set_style_text_color(input, color(text), 0);
    lv_obj_set_style_bg_color(input, color(surface), 0); lv_obj_set_style_bg_opa(input, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(input, color(border), 0); lv_obj_set_style_border_width(input, 1, 0);
    lv_obj_set_style_pad_all(input, 12, 0);
    bind(input, Action::NameInput);
    if (auto* group = lv_group_get_default()) lv_group_add_obj(group, input);
    label(parent, renaming_ && lv_textarea_get_cursor_pos(input) > 48 ?
          "Existing long name preserved. Shorten to 48 characters to add text." :
          "Up to 48 characters. Cancel leaves your current patch intact.", 24, 88, 752, muted, &lv_font_montserrat_14);
    auto* keyboard = lv_keyboard_create(parent); lv_obj_set_align(keyboard, LV_ALIGN_TOP_LEFT);
    lv_obj_set_pos(keyboard, 0, 112); lv_obj_set_size(keyboard, 800, 248);
    lv_keyboard_set_textarea(keyboard, input); lv_keyboard_set_popovers(keyboard, false);
    lv_obj_set_style_bg_color(keyboard, color(background), 0); lv_obj_set_style_bg_opa(keyboard, LV_OPA_COVER, 0);
    lv_obj_set_style_text_font(keyboard, &lv_font_montserrat_18, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(keyboard, color(surface), LV_PART_ITEMS); lv_obj_set_style_bg_opa(keyboard, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_text_color(keyboard, color(text), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(keyboard, color(border), static_cast<lv_style_selector_t>(LV_PART_ITEMS) | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(keyboard, color(text), static_cast<lv_style_selector_t>(LV_PART_ITEMS) | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(keyboard, color(border), static_cast<lv_style_selector_t>(LV_PART_ITEMS) | LV_STATE_PRESSED);
    bind(keyboard, Action::Keyboard);
    lv_obj_add_state(input, LV_STATE_FOCUSED);
    if (auto* group = lv_obj_get_group(input)) { lv_group_focus_obj(input); lv_group_set_editing(group, true); }
}
void Application::confirm_replace(lv_obj_t* parent) {
    label(parent, "Current edits have not been saved", 24, 44, 752, text, &lv_font_montserrat_24);
    auto* name = label(parent, graph_.name, 24, 104, 752, accent, &lv_font_montserrat_24); lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
    lv_obj_set_height(name, 32);
    label(parent, "Replace this patch and discard its current unsaved edits?\nCancel to return and save a session snapshot first.", 24, 168, 752, muted);
    button(parent, "Cancel", 24, 268, 200, Action::CancelReplace);
    button(parent, "Replace patch", 552, 268, 224, Action::Confirm);
}
void Application::module_actions(lv_obj_t* parent) {
    const auto* m = find_module(graph_, selected_);
    if (!m || m->fixed) { label(parent, "Module actions unavailable", 24, 24); return; }
    label(parent, "Choose an action for this module", 24, 24, 0, text, &lv_font_montserrat_24);
    button(parent, "Parameters", 24, 84, 352, Action::Edit, m->id);
    auto* bypass = button(parent, m->bypassed ? "Enable module" : "Software bypass", 408, 84, 368, Action::Bypass, m->id);
    auto* duplicate = button(parent, "Duplicate", 24, 156, 352, Action::Duplicate, m->id);
    if (!module_available(*m)) { lv_obj_add_state(bypass, LV_STATE_DISABLED); lv_obj_add_state(duplicate, LV_STATE_DISABLED); }
    button(parent, "Delete module", 408, 156, 368, Action::Remove, m->id);
    label(parent, "Duplicate keeps parameter values and software bypass.\nCables and control assignments stay with the original.\nPhysical switch assignment will arrive with control mapping.", 24, 240, 752, muted);
}
void Application::settings(lv_obj_t* parent) {
    label(parent, "Windows simulator / 800 x 480", 24, 20, 0, text, &lv_font_montserrat_24);
    label(parent, "No audio processing. No hardware connection required.\nUse these states to exercise the interface.", 24, 65, 710, muted);
    button(parent, "Ready", 24, 142, 160, Action::State, 0, {}, static_cast<int>(EngineState::Ready));
    button(parent, "Disconnect", 200, 142, 160, Action::State, 0, {}, static_cast<int>(EngineState::Disconnected));
    button(parent, "Engine error", 376, 142, 180, Action::State, 0, {}, static_cast<int>(EngineState::Error));
    button(parent, "Reject next edit", 24, 216, 220, Action::Reject);
    button(parent, "Missing module", 264, 216, 220, Action::MissingFixture);
    label(parent, "DSP, xruns, memory and temperature: not measured", 24, 300, 730, muted, &lv_font_montserrat_14);
}
}
