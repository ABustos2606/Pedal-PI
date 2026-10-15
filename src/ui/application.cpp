// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
#include "ui/application.hpp"
#include "core/tuning.hpp"
// v9.5.0 exposes the hit-test payload through this header. Keep this usage
// isolated and recheck it when upgrading the pinned LVGL dependency.
#include "src/core/lv_obj_event_private.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace peepo {
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
enum class ModuleLayout { Generic, Clouds, Delay, Tuner };
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
    if (m.uri == "urn:peepo:demo:clouds") return ModuleLayout::Clouds;
    if (m.uri == "urn:peepo:demo:delay") return ModuleLayout::Delay;
    if (tuner_uri(m.uri) && std::any_of(m.parameters.begin(), m.parameters.end(), [](const Parameter& p) { return p.symbol == "tuning"; })) return ModuleLayout::Tuner;
    return ModuleLayout::Generic;
}
// Vertical placement of one connector on its edge of the square. Every audio
// channel has its own connector, so a stereo port shows two (left above
// right). A single connector stays centred; several share the edge in equal
// touch bands.
constexpr std::uint32_t control_ink = 0xe8c56a;
std::vector<const Port*> cv_ports(const Module& m, Direction direction) {
    std::vector<const Port*> found;
    for (const auto& p : module_ports(m)) if (p.type == PortType::Cv && p.direction == direction) found.push_back(&p);
    return found;
}
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
Application::Application(Engine& engine, PatchStore* store) : engine_(engine), graph_(engine.snapshot()), store_(store && store->usable() ? store : nullptr) {
    last_state_ = engine_.state();
    ever_ready_ = last_state_ == EngineState::Ready;
    refresh_stored();
    startup_pending_ = store_ && !store_->startup().empty();
    rebuild();
}
void Application::refresh_stored() {
    if (store_) stored_ = store_->list();
}
std::string Application::status_text() const {
    switch (engine_.state()) {
    case EngineState::Ready: return engine_.capabilities().simulation ? "SIMULATED" : "READY";
    case EngineState::Connecting: return ever_ready_ ? "RECONNECTING" : "STARTING";
    case EngineState::Disconnected: return "OFFLINE";
    default: return "ENGINE ERROR";
    }
}
std::string Application::engine_banner() const {
    const auto reason = engine_note_.empty() ? std::string() : " (" + engine_note_ + ")";
    switch (engine_.state()) {
    case EngineState::Ready: return {};
    case EngineState::Connecting:
        return ever_ready_ ? "Engine connection lost" + reason + ". Retrying; edits are paused and the last graph is shown."
                           : "Connecting to the engine" + reason + "...";
    case EngineState::Disconnected: return "Engine disconnected. Settings > Connect reconnects; the last graph is shown.";
    default: return "Engine error" + reason + ". Edits are paused.";
    }
}
Application::~Application() { engine_.watch_tuner(std::nullopt); if (screen_) lv_obj_delete(screen_); }
void Application::message(std::string value) {
    message_ = std::move(value);
    if (footer_) lv_label_set_text(footer_, message_.c_str());
}
void Application::navigate(Page page) {
    // Entering the browser always starts from the closed list of categories.
    if (page == Page::Browser && page_ != Page::Browser) { browser_group_.clear(); browser_scroll_ = 0; browser_reveal_ = false; }
    // Another program, or a damaged disk, may have changed what is stored.
    if (page == Page::Patches && page_ != Page::Patches) refresh_stored();
    page_ = page; rebuild_ = true;
}
RequestId Application::submit(Command command) {
    if (engine_.state() != EngineState::Ready) {
        message(engine_.state() == EngineState::Connecting && ever_ready_ ? "Engine is reconnecting; edits are paused." : "Engine unavailable. Reconnect in Settings.");
        rebuild_ = true; return 0;
    }
    auto kind = std::holds_alternative<NewPatch>(command) || std::holds_alternative<LoadSession>(command) || std::holds_alternative<LoadGraph>(command) ||
        std::holds_alternative<LoadPatch>(command) ? PendingKind::Replace :
        std::holds_alternative<RenamePatch>(command) ? PendingKind::Rename : PendingKind::Edit;
    const auto id = engine_.submit(std::move(command));
    pending_[id] = kind;
    message(engine_.capabilities().simulation ? "Waiting for simulated engine..." : "Waiting for engine...");
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
        if (!e.request) {
            // The connection's own news: keep the reason for the banner.
            if (e.state == EngineState::Ready) {
                engine_note_.clear();
                if (last_state_ != EngineState::Ready) message(ever_ready_ ? "Engine is back; showing its graph." : "Engine connected.");
            }
            else if (!e.message.empty()) engine_note_ = e.message;
            if (e.state == EngineState::Ready) ever_ready_ = true;
            // A fresh engine after a restart: offer the startup patch again.
            if (e.state == EngineState::Ready && last_state_ != EngineState::Ready && store_ && !store_->startup().empty() &&
                graph_.patch_id.empty() && !graph_.dirty && std::all_of(graph_.modules.begin(), graph_.modules.end(), [](const Module& m) { return m.fixed; }))
                startup_pending_ = true;
            last_state_ = e.state;
        }
        if (e.accepted && e.request && kind == PendingKind::Replace) {
            pan_x_ = pan_y_ = 0; selected_cable_.reset(); parameter_page_ = 0; navigate(Page::Patch);
        } else if (e.accepted && e.request && kind == PendingKind::Rename) navigate(Page::Patches);
        if (selected_cable_ && std::find(graph_.connections.begin(), graph_.connections.end(), *selected_cable_) == graph_.connections.end())
            selected_cable_.reset();
        if (!e.accepted) message(e.message);
        else if (!e.message.empty()) message(e.message);
        else if (e.request) message("Edit accepted by engine.");
        if (kind == PendingKind::Add) {
            if (!e.accepted) message("Could not add " + adding_[e.request] + ": " + e.message);
            adding_.erase(e.request);
        } else if (kind == PendingKind::Save && staging_) {
            // The engine has written the bundle; publish it, or leave the previous revision alone.
            if (e.accepted) {
                const auto issue = store_->publish(*staging_);
                if (issue.empty()) message("Saved " + graph_.name + " (revision " + std::to_string(staging_->revision) + ").");
                else { store_->discard(*staging_); message("Not saved: " + issue); }
            } else store_->discard(*staging_);
            staging_.reset();
            refresh_stored();
        } else if (kind == PendingKind::SaveAs) {
            if (e.accepted) start_save(save_as_id_);
            save_as_id_.clear();
        } else if (kind == PendingKind::Replace && e.accepted && !load_note_.empty()) message(e.message.empty() ? load_note_ : e.message + " " + load_note_);
        if (kind == PendingKind::Replace) load_note_.clear();
        rebuild_ = true;
    }
    for (const auto& g : recognizer_.advance(ms)) gesture(g);
    if (startup_pending_ && engine_.state() == EngineState::Ready && pending_.empty()) {
        startup_pending_ = false;
        // Only when the engine plays nothing stored and nothing unsaved: a
        // restarted interface must never replace the patch being played.
        const auto id = store_->startup();
        if (!id.empty() && graph_.patch_id.empty() && !graph_.dirty) load_stored(id, false, false);
    }
    // Rebuilding during a held key can deliver its release to a new button.
    // Keep both pointer and keypad targets alive until their gesture ends.
    const bool pressed = (pointer_input_ && lv_indev_get_state(pointer_input_) == LV_INDEV_STATE_PRESSED) ||
        (keypad_input_ && lv_indev_get_state(keypad_input_) == LV_INDEV_STATE_PRESSED);
    if (stream_ && !stream_->in_flight && !pressed) stream_.reset();
    if (rebuild_ && !dragging_ && !panning_ && !cable_source_ && !pressed) rebuild();
    update_metrics();
    const auto* shown = page_ == Page::Parameters && !generic_parameters_ ? find_module(graph_, selected_) : nullptr;
    engine_.watch_tuner(shown && module_layout(*shown) == ModuleLayout::Tuner ? std::optional<InstanceId>{shown->id} : std::nullopt);
    update_tuner();
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
    // Unavailable actions stay visible but read as such.
    lv_obj_set_style_text_opa(b, LV_OPA_40, LV_STATE_DISABLED);
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
                cable_source_ = b.id; cable_port_ = b.symbol; cable_cv_ = (b.number & 4) != 0;
                const auto from = jack(*m, b.symbol, Direction::Output, static_cast<unsigned>(b.number >> 3));
                message(cable_cv_ ? "Control cable from " + m->name + ". Drop it on the square connector under another module."
                                  : "Cable from " + m->name + " / " + b.symbol + ". Drop it on an input jack.");
                preview_ = lv_line_create(canvas_); lv_obj_remove_flag(preview_, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_set_style_line_color(preview_, color(cable_cv_ ? control_ink : accent), 0); lv_obj_set_style_line_width(preview_, 3, 0);
                if (cable_cv_) preview_points_[0] = {static_cast<lv_value_precise_t>(m->x + 48 - pan_x_), static_cast<lv_value_precise_t>(m->y - pan_y_)};
                else preview_points_[0] = {static_cast<lv_value_precise_t>(m->x + 96 - pan_x_), static_cast<lv_value_precise_t>(m->y + (from ? from->centre : 48) - pan_y_)};
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
            if (cable_source_ && cable_cv_) {
                // A module can have a dozen control inputs, far too many for its edge.
                // Drop on its control connector, then choose the ports from a list.
                InstanceId destination = 0;
                if (point.x >= 0 && point.x < 800 && point.y >= 72 && point.y < 432)
                    for (const auto& item : graph_.modules)
                        if (item.id != cable_source_ && !cv_ports(item, Direction::Input).empty() &&
                            std::abs(point.x + pan_x_ - item.x - 48) <= 24 && std::abs(point.y - 72 + pan_y_ - item.y - 96) <= 24) destination = item.id;
                if (destination && code == LV_EVENT_RELEASED) {
                    const auto outputs = cv_ports(*m, Direction::Output);
                    cv_from_ = cable_source_; cv_to_ = destination; cv_page_ = 0;
                    cv_output_ = outputs.size() == 1 ? outputs.front()->symbol : std::string();
                    navigate(Page::CvConnect);
                } else message("Drop the control cable on the square connector under a module.");
                if (preview_) lv_obj_delete(preview_);
                preview_ = nullptr; cable_source_ = 0; indicate_targets(false);
            } else if (cable_source_) {
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
    if (b.action == Action::RangeEnd) {
        // The From (0) and To (1) ends of an assignment's range, sent on release.
        const auto* a = pinned(editing_control_);
        const auto* p = a ? assignment_parameter(graph_, *a) : nullptr;
        if (!p) return;
        auto end = *p;
        end.value = value_from_normalized(*p, lv_slider_get_value(target) / 1000.0);
        if (p->kind != ParameterKind::Continuous) end.value = std::round(end.value);
        if (b.value_label) lv_label_set_text(b.value_label, format_value(end).c_str());
        if (code == LV_EVENT_RELEASED) {
            auto changed = *a;
            (b.number ? changed.maximum : changed.minimum) = end.value;
            if (changed.minimum > changed.maximum) { message("From must not pass To; use Invert to reverse the direction."); rebuild_ = true; return; }
            submit(Assign{changed});
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
            if (b.action == Action::Add) {
                if (const auto request = submit(AddModule{b.symbol, x, y})) {
                    const auto* d = definition(b.symbol);
                    pending_[request] = PendingKind::Add; adding_[request] = d ? d->name : b.symbol;
                }
            } else submit(DuplicateModule{b.id, x, y});
            reveal(x, y); navigate(Page::Patch);
        } else message("No free module slot remains on this canvas.");
        break;
    }
    case Action::SelectCable:
        if (b.number >= 0 && static_cast<std::size_t>(b.number) < cables_.size()) {
            selected_cable_ = cables_[b.number].connection; rebuild_ = true;
            const auto* from = find_module(graph_, selected_cable_->from); const auto* to = find_module(graph_, selected_cable_->to);
            message(from && to ? from->name + " / " + selected_cable_->output + " > " + to->name + " / " + selected_cable_->input : "Cable selected.");
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
        // 0: new patch, 1: rename, 2: save under a new name as another stored patch.
        renaming_ = b.number == 1; saving_as_ = b.number == 2;
        draft_name_ = b.number ? graph_.name : "";
        message(saving_as_ ? "Saves a separate copy; the original stays as it was saved." :
                renaming_ ? "Rename keeps modules, values and cables." : "New patch starts with IN connected to OUT.");
        navigate(Page::NamePatch); break;
    case Action::SavePatch: save_patch(b.number == 1); break;
    case Action::LoadStored: load_stored(b.symbol, false, true); break;
    case Action::DeleteStored: {
        const auto patch = store_ ? store_->find(b.symbol) : std::nullopt;
        if (!patch) { refresh_stored(); rebuild_ = true; break; }
        const auto id = patch->id;
        ask("Delete saved patch?", patch->name, "Every saved revision of this patch is removed from disk.\nThe patch playing now is not changed.", "Delete", [this, id] {
            const auto issue = store_->remove(id);
            message(issue.empty() ? "Saved patch deleted." : issue);
            refresh_stored(); navigate(Page::Patches);
        });
        break;
    }
    case Action::StartupStored: {
        if (!store_) break;
        const bool already = store_->startup() == b.symbol;
        const auto issue = store_->set_startup(already ? std::string() : b.symbol);
        message(!issue.empty() ? issue : already ? "No startup patch: the engine keeps whatever it plays." : "This patch loads when the instrument starts.");
        refresh_stored(); rebuild_ = true; break;
    }
    case Action::PatchPage: patch_page_ = static_cast<std::size_t>(b.number); rebuild_ = true; break;
    case Action::AssignControl:
        editing_control_ = b.symbol; picking_module_ = 0; picker_page_ = 0;
        message("Choose what " + control_label(b.symbol) + " controls in this patch.");
        navigate(Page::AssignPicker); break;
    case Action::EditControl: editing_control_ = b.symbol; navigate(Page::AssignmentDetail); break;
    case Action::ClearControl:
        if (pinned(b.symbol)) submit(Unassign{b.symbol});
        else if (store_ && store_->global_controls().contains(b.symbol)) {
            const auto issue = store_->set_global_control(b.symbol, {});
            message(issue.empty() ? control_label(b.symbol) + " cleared." : issue);
        }
        navigate(Page::Controls); break;
    case Action::PickModule: picking_module_ = b.id; picker_page_ = 0; rebuild_ = true; break;
    case Action::PickerPage: picker_page_ = static_cast<std::size_t>(b.number); rebuild_ = true; break;
    case Action::PickTarget: {
        if (b.symbol == "@next-patch" || b.symbol == "@previous-patch") {
            const auto issue = store_ ? store_->set_global_control(editing_control_, b.symbol.substr(1)) : std::string("Patch storage is not available");
            message(issue.empty() ? control_label(editing_control_) + " changes patch in every patch that does not use it otherwise." : issue);
            navigate(Page::Controls); break;
        }
        const auto* m = find_module(graph_, picking_module_);
        if (!m) { picking_module_ = 0; rebuild_ = true; break; }
        Assignment a;
        a.control = editing_control_; a.module = m->id;
        if (b.symbol == "@bypass") a.action = ControlAction::Bypass;
        else {
            const auto it = std::find_if(m->parameters.begin(), m->parameters.end(), [&](const Parameter& p) { return p.symbol == b.symbol; });
            if (it == m->parameters.end()) break;
            a.parameter = it->symbol; a.minimum = it->minimum; a.maximum = it->maximum;
        }
        submit(Assign{a});
        navigate(Page::Controls); break;
    }
    case Action::InvertControl:
        if (const auto* a = pinned(editing_control_)) { auto changed = *a; changed.inverted = !changed.inverted; submit(Assign{changed}); }
        break;
    case Action::SwitchModeChoice:
        if (const auto* a = pinned(editing_control_)) { auto changed = *a; changed.mode = b.number ? SwitchMode::Momentary : SwitchMode::Toggle; submit(Assign{changed}); }
        break;
    case Action::ConfirmAction: if (confirmation_.act) { auto act = std::move(confirmation_.act); confirmation_.act = nullptr; act(); } break;
    case Action::CancelAction: confirmation_.act = nullptr; navigate(confirmation_.back); break;
    case Action::PowerOff:
        if (!power_off_) break;
        if (pending()) { message("Wait for pending edits before shutting down."); break; }
        ask("Shut down", "Shut down Peepo?", std::string(graph_.dirty ? "This patch has unsaved edits; they will be lost.\n" : "") +
            "Saved patches are safe. Unplug when the screen goes dark.", "Shut down", [this] {
            if (const auto issue = power_off_(); !issue.empty()) message(issue);
            else message("Shutting down. Unplug when the screen goes dark.");
            navigate(Page::Settings);
        });
        confirmation_.back = Page::Settings;
        break;
    case Action::CommitName: commit_name(); break;
    case Action::Confirm:
        if (replacement_ && !pending()) submit(*replacement_);
        else message("Wait for pending edits before replacing the patch.");
        break;
    case Action::CancelReplace: replacement_.reset(); navigate(confirm_origin_); break;
    case Action::ParameterPage: parameter_page_ = static_cast<std::size_t>(b.number); rebuild_ = true; break;
    case Action::BrowserGroup:
        // One category open at a time; tapping the open one closes it.
        browser_group_ = browser_group_ == b.symbol ? std::string() : b.symbol;
        browser_reveal_ = !browser_group_.empty(); rebuild_ = true; break;
    case Action::CvPage: cv_page_ = static_cast<std::size_t>(b.number); rebuild_ = true; break;
    case Action::CvChoice:
        if (cv_output_.empty()) { cv_output_ = b.symbol; cv_page_ = 0; rebuild_ = true; }
        else { submit(Connect{{cv_from_, cv_to_, cv_output_, b.symbol}}); navigate(Page::Patch); }
        break;
    default: break;
    }
}
void Application::commit_name() {
    if (pending()) { message("Wait for pending edits before naming a patch."); return; }
    if (auto issue = validate_patch_name(draft_name_); !issue.empty()) { message(issue); return; }
    if (saving_as_) {
        // A new identity; the name changes first, so the copy is saved under it.
        const auto id = store_ ? store_->new_id() : std::string();
        if (id.empty()) { message("Patch storage is not available."); return; }
        navigate(Page::Patches);
        if (draft_name_ == graph_.name) { start_save(id); return; }
        if (const auto request = submit(RenamePatch{draft_name_})) { pending_[request] = PendingKind::SaveAs; save_as_id_ = id; }
    } else if (renaming_) submit(RenamePatch{draft_name_});
    else replace_patch(NewPatch{draft_name_}, Page::NamePatch);
}
void Application::save_patch(bool as_new) {
    if (!store_) return;
    if (pending()) { message("Wait for pending edits before saving."); return; }
    if (as_new) {
        renaming_ = false; saving_as_ = true; draft_name_ = graph_.name;
        message("Saves a separate copy; the original stays as it was saved.");
        navigate(Page::NamePatch);
        return;
    }
    // Save writes a new revision of the stored patch this graph came from.
    start_save(graph_.patch_id.empty() ? store_->new_id() : graph_.patch_id);
}
void Application::start_save(const std::string& id) {
    std::string issue;
    auto staging = store_->stage(id, issue);
    if (!staging) { message(issue); return; }
    const auto request = submit(SaveGraph{SaveScope::Persistent, engine_.capabilities().plugin_state, staging->location, id});
    if (!request) { store_->discard(*staging); return; }
    pending_[request] = PendingKind::Save;
    staging_ = std::move(*staging);
    message("Saving " + graph_.name + "...");
}
void Application::load_stored(const std::string& id, bool allow_missing, bool guarded) {
    const auto patch = store_ ? store_->find(id) : std::nullopt;
    if (!patch) { message("That patch is no longer stored."); refresh_stored(); rebuild_ = true; return; }
    if (!patch->revision) { message(patch->name + ": " + patch->issue); return; }
    const auto blocks = saved_blocks(patch->bundle);
    std::vector<std::string> absent;
    for (const auto& block : blocks)
        if (const auto* known = definition(block.plugin); !known || known->availability != Availability::Available) absent.push_back(block.name);
    std::string names;
    for (const auto& name : absent) names += (names.empty() ? "" : ", ") + name;
    if (!absent.empty() && !allow_missing) {
        ask("Modules unavailable", patch->name, "Not available here: " + names + ".\nLoad the rest without plugin state? Saving afterwards leaves these modules out.", "Load the rest",
            [this, id, guarded] { navigate(Page::Patches); load_stored(id, true, guarded); });
        return;
    }
    Command command = LoadPatch{patch->location, blocks, patch->id};
    load_note_ = patch->issue;
    if (!patch->missing_files.empty()) {
        std::string files;
        for (const auto& file : patch->missing_files) files += (files.empty() ? "" : ", ") + file;
        load_note_ += (load_note_.empty() ? "" : " ") + std::string("Missing files: ") + files + ".";
    }
    if (!absent.empty()) {
        // The engine loads none of a bundle with a missing plugin. Rebuild the
        // rest from its description instead: routing, values, names, layout.
        auto rest = bundle_graph(patch->bundle);
        std::erase_if(rest.modules, [](const Module& m) { const auto* d = definition(m.uri); return !m.fixed && (!d || d->availability != Availability::Available); });
        std::erase_if(rest.connections, [&](const Connection& c) { return !find_module(rest, c.from) || !find_module(rest, c.to); });
        rest.patch_id = patch->id;
        rest.dirty = true;
        command = LoadGraph{std::move(rest)};
        load_note_ = "Left out: " + names + ". Plugin state was not restored; saving keeps the patch without them.";
    }
    if (guarded) replace_patch(std::move(command), Page::Patches);
    else submit(std::move(command));
}
void Application::ask(std::string title, std::string heading, std::string body, std::string button, std::function<void()> act) {
    confirmation_ = {std::move(title), std::move(heading), std::move(body), std::move(button), std::move(act)};
    navigate(Page::ConfirmAction);
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
    } else {
        if (browser_list_) browser_scroll_ = lv_obj_get_scroll_y(browser_list_);
        lv_obj_clean(screen_);
    }
    browser_list_ = nullptr;
    visible_.clear();
    bindings_.clear(); module_widgets_.clear(); input_jacks_.clear(); cv_inputs_.clear(); dots_.clear(); cables_.clear(); canvas_ = nullptr; footer_ = nullptr;
    tempo_value_ = cpu_value_ = nullptr;
    tuner_note_ = tuner_detail_ = tuner_status_ = tuner_needle_ = nullptr;
    auto* header = panel(screen_, 0, 0, 800, 72, background);
    lv_obj_set_style_radius(header, 0, 0);
    const bool patch = page_ == Page::Patch;
    if (patch) {
        label(header, "Peepo", 24, 10, 0, accent, &lv_font_montserrat_14);
        auto* title = label(header, graph_.name + (graph_.dirty ? " *" : ""), 24, 32, 200, text, &lv_font_montserrat_24);
        lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
        lv_obj_set_height(title, 28);
        button(header, "Home", 244, 12, 64, Action::Home);
        label(header, status_text(), 318, 28, 0, engine_.state() == EngineState::Ready ? accent : 0xf0aa72, &lv_font_montserrat_14);
        auto* add = button(header, "+ Add", 448, 12, 96, Action::Navigate, 0, {}, static_cast<int>(Page::Browser));
        // Nothing can be added until the engine answers again.
        if (engine_.state() != EngineState::Ready) lv_obj_add_state(add, LV_STATE_DISABLED);
        button(header, "Patches", 556, 12, 104, Action::Navigate, 0, {}, static_cast<int>(Page::Patches));
        button(header, "Settings", 672, 12, 104, Action::Navigate, 0, {}, static_cast<int>(Page::Settings));
    } else {
        // Back returns to where the page was opened from.
        const auto back = page_ == Page::NamePatch ? Page::Patches : page_ == Page::Controls ? Page::Settings :
            page_ == Page::AssignPicker || page_ == Page::AssignmentDetail ? Page::Controls : Page::Patch;
        button(header, "< Back", 16, 12, 104, Action::Navigate, 0, {}, static_cast<int>(back));
        std::string title = page_ == Page::Browser ? "Add a module" : page_ == Page::Patches ? (store_ ? "Patches" : "Session patches") :
            page_ == Page::Settings ? (engine_.capabilities().simulation ? "Simulator settings" : "Engine settings") : "Parameters";
        if (page_ == Page::NamePatch) title = saving_as_ ? "Save as a new patch" : renaming_ ? "Rename patch" : "New patch";
        if (page_ == Page::ConfirmAction) title = confirmation_.title;
        if (page_ == Page::Controls) title = "Controls";
        if (page_ == Page::AssignPicker && picking_module_) if (const auto* m = find_module(graph_, picking_module_)) title = control_label(editing_control_) + ": " + m->name;
        if (page_ == Page::AssignPicker && !picking_module_) title = "Assign " + control_label(editing_control_);
        if (page_ == Page::AssignmentDetail) title = control_label(editing_control_);
        if (page_ == Page::ConfirmReplace) title = "Replace current patch?";
        if (page_ == Page::CvConnect) title = "Control cable";
        if (page_ == Page::Parameters || page_ == Page::ModuleActions)
            if (const auto* m = find_module(graph_, selected_)) title = m->name + (page_ == Page::ModuleActions ? " actions" : "");
        auto* heading = label(header, title, 144, 23, 480, text, &lv_font_montserrat_24);
        lv_label_set_long_mode(heading, LV_LABEL_LONG_DOT);
        lv_obj_set_height(heading, 30);
        if (page_ == Page::NamePatch) button(header, saving_as_ ? "Save" : renaming_ ? "Rename" : "Create", 640, 12, 136, Action::CommitName);
        else label(header, engine_.capabilities().simulation ? "SIMULATED" : "ENGINE", 660, 28, 0, accent, &lv_font_montserrat_14);
    }
    // The browser is a list to scroll; it has no use for the status bar.
    const bool status_bar = page_ != Page::Browser;
    auto* content = panel(screen_, 0, 72, 800, status_bar ? 360 : 408, background);
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
    case Page::CvConnect: cv_connect(content); break;
    case Page::ConfirmAction: confirm_action(content); break;
    case Page::Controls: controls(content); break;
    case Page::AssignPicker: assign_picker(content); break;
    case Page::AssignmentDetail: assignment_detail(content); break;
    }
    if (patch && engine_.state() != EngineState::Ready) {
        // Over the canvas, but never in the way: touches pass through to the patch below.
        auto* banner = panel(content, 24, 8, 752, 40, 0x2e241c);
        lv_obj_set_style_border_color(banner, color(0xf0aa72), 0);
        lv_obj_remove_flag(banner, LV_OBJ_FLAG_CLICKABLE);
        auto* note = label(banner, engine_banner(), 14, 10, 724, 0xf0aa72, &lv_font_montserrat_14);
        lv_label_set_long_mode(note, LV_LABEL_LONG_DOT); lv_obj_set_height(note, 18);
    }
    if (!status_bar) { lv_obj_update_layout(screen_); return; }
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
        unsigned out_channels = 1, in_channels = 1; bool cv = false;
        if (const auto* source = find_module(graph_, edge.from)) for (const auto& p : module_ports(*source))
            if (p.symbol == edge.output && p.direction == Direction::Output) { out_channels = p.channels; cv = p.type == PortType::Cv; }
        if (const auto* sink = find_module(graph_, edge.to)) for (const auto& p : module_ports(*sink))
            if (p.symbol == edge.input && p.direction == Direction::Input) in_channels = p.channels;
        for (unsigned channel = 0; channel < std::max(out_channels, in_channels); ++channel) {
        auto* line = lv_line_create(parent);
        lv_obj_set_style_line_color(line, color(selected || !routing.error.empty() ? 0xf0aa72 : cv ? control_ink : accent), 0);
        lv_obj_set_style_line_width(line, cv ? (selected ? 5 : 3) : (selected ? 6 : 4), 0);
        lv_obj_set_style_line_rounded(line, true, 0);
        lv_obj_add_flag(line, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(line, LV_OBJ_FLAG_ADV_HITTEST);
        lv_obj_set_ext_click_area(line, 12);
        bind(line, Action::SelectCable, 0, {}, static_cast<int>(cables_.size()));
        cables_.push_back({line, edge, {}, routing, out_channels == 1 ? 0 : channel, in_channels == 1 ? 0 : channel, cv});
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
            bind(hit, Action::Port, m.id, port.symbol, (output ? 1 : 0) | static_cast<int>(channel << 3));
            if (!output) input_jacks_.emplace(std::pair{m.id, port.symbol}, dot);
            }
        }
        // Control connectors: one square socket for all control outputs on top,
        // one for all control inputs underneath.
        for (const bool output : {false, true}) {
            if (cv_ports(m, output ? Direction::Output : Direction::Input).empty()) continue;
            auto* hit = lv_obj_create(card); lv_obj_remove_style_all(hit);
            lv_obj_set_size(hit, 44, 44); lv_obj_set_pos(hit, 26, output ? -22 : 74);
            lv_obj_remove_flag(hit, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(hit, LV_OBJ_FLAG_PRESS_LOCK);
            auto* dot = panel(hit, 15, 15, 14, 14, background);
            lv_obj_set_style_radius(dot, 3, 0);
            lv_obj_set_style_border_width(dot, 3, 0); lv_obj_set_style_border_color(dot, color(control_ink), 0);
            lv_obj_remove_flag(dot, LV_OBJ_FLAG_CLICKABLE);
            bind(hit, Action::Port, m.id, {}, (output ? 1 : 0) | 4);
            if (!output) cv_inputs_[m.id] = dot;
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
    for (const auto& [id, dot] : cv_inputs_) {
        lv_obj_set_style_outline_color(dot, color(control_ink), 0);
        lv_obj_set_style_outline_width(dot, active && cable_cv_ && id != cable_source_ ? 4 : 0, 0);
        lv_obj_set_style_outline_opa(dot, LV_OPA_COVER, 0);
    }
    for (const auto& [key, jack] : input_jacks_) {
        const auto& [id, symbol] = key;
        const auto* m = find_module(graph_, id); if (!m) continue;
        const auto* d = definition(m->uri);
        const bool valid = active && !cable_cv_ && validate_connection(graph_, {cable_source_, id, cable_port_, symbol}).empty();
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
        if (c.cv) {
            // Top of the source to the underside of the destination.
            const auto sx = static_cast<lv_value_precise_t>(lv_obj_get_x(a->second) + 48), sy = static_cast<lv_value_precise_t>(lv_obj_get_y(a->second));
            const auto tx = static_cast<lv_value_precise_t>(lv_obj_get_x(b->second) + 48), ty = static_cast<lv_value_precise_t>(lv_obj_get_y(b->second) + 96);
            c.points[0] = {sx, sy}; c.points[1] = {sx, sy - 14}; c.points[2] = {tx, ty + 14}; c.points[3] = {tx, ty};
            lv_line_set_points(c.line, c.points, 4);
            continue;
        }
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
    // Categories in signal-chain order; any the catalogue invents follow alphabetically.
    static const std::vector<std::string> order = {"Drive", "Amp", "Cabinet", "Dynamics", "Filter", "Modulation", "Pitch",
        "Delay", "Reverb", "Texture", "Resonator", "Sound source", "Control", "Utility"};
    const auto rank = [](const std::string& category) {
        return static_cast<std::size_t>(std::find(order.begin(), order.end(), category) - order.begin());
    };
    std::map<std::string, std::vector<const ModuleDefinition*>> groups;
    for (const auto& d : catalogue()) if (d.category != "Endpoint" && !d.hidden) groups[d.category].push_back(&d);
    std::vector<std::string> categories;
    for (const auto& [category, _] : groups) categories.push_back(category);
    std::stable_sort(categories.begin(), categories.end(), [&](const auto& a, const auto& b) { return rank(a) < rank(b); });

    browser_list_ = parent;
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(parent, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_style_pad_bottom(parent, 16, 0);
    auto* group = lv_group_get_default();
    lv_obj_t* open = nullptr;
    int y = 12;
    for (const auto& category : categories) {
        auto& entries = groups[category];
        // Usable modules first, each part alphabetical.
        std::stable_sort(entries.begin(), entries.end(), [](const ModuleDefinition* a, const ModuleDefinition* b) {
            const bool ua = a->availability == Availability::Available, ub = b->availability == Availability::Available;
            return ua != ub ? ua : a->name < b->name;
        });
        const bool expanded = category == browser_group_;
        auto* header = lv_button_create(parent); lv_obj_remove_style_all(header);
        lv_obj_set_pos(header, 16, y); lv_obj_set_size(header, 768, 56);
        lv_obj_set_style_bg_color(header, color(expanded ? border : surface), 0); lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(header, color(border), LV_STATE_PRESSED);
        lv_obj_set_style_border_width(header, 1, 0); lv_obj_set_style_border_color(header, color(border), 0);
        lv_obj_set_style_radius(header, 10, 0);
        const auto ink = entries.front()->color;
        auto* chevron = label(header, expanded ? LV_SYMBOL_DOWN : LV_SYMBOL_RIGHT, 18, 0, 0, muted, &lv_font_montserrat_18);
        lv_obj_align(chevron, LV_ALIGN_LEFT_MID, 18, 0);
        auto* name = label(header, category, 0, 0, 560, ink, &lv_font_montserrat_20);
        lv_obj_align(name, LV_ALIGN_LEFT_MID, 52, 0);
        const auto available = std::count_if(entries.begin(), entries.end(), [](const auto* d) { return d->availability == Availability::Available; });
        auto* count = label(header, std::to_string(available) + " ready / " + std::to_string(entries.size()), 0, 0, 160, muted, &lv_font_montserrat_14);
        lv_obj_set_style_text_align(count, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_align(count, LV_ALIGN_RIGHT_MID, -20, 0);
        bind(header, Action::BrowserGroup, 0, category);
        if (group) lv_group_add_obj(group, header);
        if (expanded) open = header;
        y += 64;
        if (!expanded) continue;
        y -= 4;
        // A row is just the name and what it does; tapping it adds the module.
        for (const auto* d : entries) {
            const bool usable = d->availability == Availability::Available;
            auto* row = lv_button_create(parent); lv_obj_remove_style_all(row);
            lv_obj_set_pos(row, 16, y); lv_obj_set_size(row, 768, 60);
            lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_color(row, color(surface), LV_STATE_PRESSED); lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
            lv_obj_set_style_radius(row, 8, 0);
            lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
            lv_obj_set_style_border_width(row, 1, 0); lv_obj_set_style_border_color(row, color(border), 0);
            auto* title = label(row, d->name, 0, 0, 220, d->color, &lv_font_montserrat_18);
            lv_label_set_long_mode(title, LV_LABEL_LONG_DOT); lv_obj_set_height(title, 22);
            lv_obj_align(title, LV_ALIGN_LEFT_MID, 36, 0);
            // An entry that cannot be added says why instead of what it does.
            const auto about = usable ? d->description : "Needs setup: " + (d->availability_reason.empty() ? d->description : d->availability_reason);
            auto* description = label(row, about, 0, 0, 488, muted, &lv_font_montserrat_14);
            // At most two lines, centred against the name.
            lv_label_set_long_mode(description, LV_LABEL_LONG_DOT);
            lv_obj_set_height(description, LV_SIZE_CONTENT); lv_obj_set_style_max_height(description, 36, 0);
            lv_obj_align(description, LV_ALIGN_LEFT_MID, 268, 0);
            if (usable) {
                bind(row, Action::Add, 0, d->uri);
                if (group) lv_group_add_obj(group, row);
            } else {
                // Not clickable, so a drag starting here still scrolls the list.
                lv_obj_remove_flag(row, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_set_style_opa(row, LV_OPA_40, 0);
            }
            y += 60;
        }
        y += 12;
    }
    lv_obj_update_layout(parent);
    if (browser_reveal_ && open) {
        // Bring the opened category to the top so its modules are in view.
        lv_obj_scroll_to_y(parent, lv_obj_get_y(open) - 12, LV_ANIM_OFF);
        browser_reveal_ = false;
    } else lv_obj_scroll_to_y(parent, browser_scroll_, LV_ANIM_OFF);
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
    if (!generic_parameters_ && layout == ModuleLayout::Tuner) { tuner_parameters(parent, *m); return; }
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
    } else label(parent, engine_.capabilities().simulation ? "Simulated controls" : "Live controls", 272, 27, 0, muted, &lv_font_montserrat_14);
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
void Application::tuner_parameters(lv_obj_t* parent, const Module& m) {
    auto* meter = panel(parent, 24, 12, 752, 242);
    tuner_note_ = label(meter, "--", 0, 8, 752, text, &lv_font_montserrat_48);
    lv_obj_set_style_text_align(tuner_note_, LV_TEXT_ALIGN_CENTER, 0);
    tuner_detail_ = label(meter, "-- Hz", 0, 64, 752, muted);
    lv_obj_set_style_text_align(tuner_detail_, LV_TEXT_ALIGN_CENTER, 0);
    tuner_status_ = label(meter, "Play one string", 0, 98, 752, muted);
    lv_obj_set_style_text_align(tuner_status_, LV_TEXT_ALIGN_CENTER, 0);
    // +/- 5 cents is the green target; the scale spans a whole semitone.
    auto* target = panel(meter, 346, 142, 60, 58, 0x264b43);
    lv_obj_set_style_border_width(target, 0, 0);
    for (int i = 0; i <= 10; ++i) {
        auto* mark = panel(meter, 76 + i * 60 - 1, i == 5 ? 146 : 158, 2, i == 5 ? 48 : 24, i == 5 ? accent : muted);
        lv_obj_set_style_border_width(mark, 0, 0);
    }
    label(meter, "-50  FLAT", 42, 208, 150, muted, &lv_font_montserrat_14);
    auto* centre = label(meter, "0", 346, 208, 60, accent, &lv_font_montserrat_14);
    lv_obj_set_style_text_align(centre, LV_TEXT_ALIGN_CENTER, 0);
    auto* sharp = label(meter, "SHARP  +50", 560, 208, 150, muted, &lv_font_montserrat_14);
    lv_obj_set_style_text_align(sharp, LV_TEXT_ALIGN_RIGHT, 0);
    tuner_needle_ = panel(meter, 372, 142, 8, 58, accent);
    lv_obj_set_style_border_width(tuner_needle_, 0, 0);
    lv_obj_remove_flag(tuner_needle_, LV_OBJ_FLAG_CLICKABLE);
    parameter_tile(parent, m, parameter(m, "tuning"), 24, 268, 72);
    button(parent, "All", 408, 280, 80, Action::ParameterLayout);
    button(parent, m.bypassed ? "Enable" : "Bypass", 500, 280, 112, Action::Bypass, m.id);
    button(parent, "Actions", 640, 280, 136, Action::OpenActions, m.id);
    update_tuner();
}
void Application::update_tuner() {
    if (!tuner_note_) return;
    const auto* m = find_module(graph_, selected_);
    if (!m) return;
    const bool ready = engine_.state() == EngineState::Ready && !m->bypassed && !engine_.capabilities().simulation;
    const auto reading = tuning_from_frequency(ready ? engine_.tuner_frequency(m->id) : std::nullopt, parameter(*m, "tuning").value);
    std::string detail = "Connect IN to this tuner; play one string.";
    std::string status = engine_.capabilities().simulation ? "No audio in simulator" : !ready ? (m->bypassed ? "Tuner bypassed" : "Engine offline") : "Waiting for a note";
    if (reading) {
        std::ostringstream value;
        value << std::fixed << std::setprecision(1) << reading->frequency << " Hz   |   " << std::showpos << reading->cents << " cents";
        detail = value.str();
        status = std::abs(reading->cents) <= 5 ? "In tune" : reading->cents < 0 ? "Flat - tighten string" : "Sharp - loosen string";
        lv_obj_set_x(tuner_needle_, 372 + static_cast<int>(std::round(std::clamp(reading->cents, -50.0, 50.0) * 6)));
        lv_obj_remove_flag(tuner_needle_, LV_OBJ_FLAG_HIDDEN);
    } else lv_obj_add_flag(tuner_needle_, LV_OBJ_FLAG_HIDDEN);
    const auto ink = reading ? (std::abs(reading->cents) <= 5 ? accent : 0xf0aa72) : muted;
    lv_obj_set_style_text_color(tuner_note_, color(ink), 0);
    lv_obj_set_style_text_color(tuner_status_, color(ink), 0);
    lv_obj_set_style_bg_color(tuner_needle_, color(ink), 0);
    const std::string note = reading ? reading->note : "--";
    if (note != lv_label_get_text(tuner_note_)) lv_label_set_text(tuner_note_, note.c_str());
    if (detail != lv_label_get_text(tuner_detail_)) lv_label_set_text(tuner_detail_, detail.c_str());
    if (status != lv_label_get_text(tuner_status_)) lv_label_set_text(tuner_status_, status.c_str());
}
void Application::parameter_tile(lv_obj_t* parent, const Module& m, const Parameter& p, int x, int y, int height, std::uint32_t ink) {
    auto* tile = panel(parent, x, y, 368, height);
    label(tile, p.name, 20, 12, 150);
    // Unpinned encoders follow the tiles in the order they are drawn; pinned
    // controls are named on their target wherever it appears.
    const auto follower = "enc" + std::to_string(visible_.size() + 1);
    visible_.push_back({m.id, p.symbol});
    std::string badges;
    for (const auto& a : graph_.assignments)
        if (a.module == m.id && a.action == ControlAction::Parameter && a.parameter == p.symbol && assignment_target(graph_, a))
            badges += (badges.empty() ? "" : " ") + std::string(is_encoder(a.control) ? "E" : "S") + a.control.substr(is_encoder(a.control) ? 3 : 5);
    if (!badges.empty()) badges += " pinned";
    else if (visible_.size() <= layout_.encoders && !pinned(follower)) badges = "E" + follower.substr(3);
    // Compact tiles have no room beside their slider.
    if (!badges.empty() && height >= 100) {
        auto* badge = label(tile, badges, 236, height - 24, 116, badges.ends_with("pinned") ? accent : muted, &lv_font_montserrat_14);
        lv_obj_set_style_text_align(badge, LV_TEXT_ALIGN_RIGHT, 0);
    }
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
void Application::confirm_action(lv_obj_t* parent) {
    auto* heading = label(parent, confirmation_.heading, 24, 44, 752, accent, &lv_font_montserrat_24);
    lv_label_set_long_mode(heading, LV_LABEL_LONG_DOT); lv_obj_set_height(heading, 32);
    label(parent, confirmation_.body, 24, 104, 752, muted);
    button(parent, "Cancel", 24, 268, 200, Action::CancelAction);
    button(parent, confirmation_.button, 552, 268, 224, Action::ConfirmAction);
}
void Application::stored_patches(lv_obj_t* parent) {
    constexpr std::size_t rows = 3;
    label(parent, "Saved patches", 24, 16, 0, text, &lv_font_montserrat_24);
    const auto pages = std::max<std::size_t>(1, (stored_.size() + rows - 1) / rows);
    patch_page_ = std::min(patch_page_, pages - 1);
    if (pages > 1) {
        auto* previous = button(parent, "<", 600, 4, 48, Action::PatchPage, 0, {}, static_cast<int>(patch_page_ ? patch_page_ - 1 : 0));
        if (!patch_page_) lv_obj_add_state(previous, LV_STATE_DISABLED);
        label(parent, std::to_string(patch_page_ + 1) + " / " + std::to_string(pages), 664, 18, 56, muted, &lv_font_montserrat_14);
        auto* next = button(parent, ">", 728, 4, 48, Action::PatchPage, 0, {}, static_cast<int>(std::min(patch_page_ + 1, pages - 1)));
        if (patch_page_ + 1 >= pages) lv_obj_add_state(next, LV_STATE_DISABLED);
    }
    if (stored_.empty()) label(parent, "Nothing saved yet. Save keeps this patch on disk, with its\nplugin state, and survives restarting the app and the engine.", 24, 72, 752, muted);
    for (std::size_t i = patch_page_ * rows; i < std::min(stored_.size(), patch_page_ * rows + rows); ++i) {
        const auto& patch = stored_[i];
        auto* row = panel(parent, 24, 60 + static_cast<int>(i - patch_page_ * rows) * 64, 752, 56);
        const bool playing = patch.id == graph_.patch_id;
        if (playing) lv_obj_set_style_border_color(row, color(accent), 0);
        auto* name = label(row, patch.name, 16, 6, 424, text);
        lv_label_set_long_mode(name, LV_LABEL_LONG_DOT); lv_obj_set_height(name, 22);
        std::string detail = patch.revision ? "Revision " + std::to_string(patch.revision) + " / " + std::to_string(patch.bundle.blocks.size()) + " modules" : "Unreadable";
        if (patch.startup) detail += " / loads at startup";
        if (playing) detail += graph_.dirty ? " / playing, unsaved edits" : " / playing";
        if (!patch.missing_files.empty()) detail += " / " + std::to_string(patch.missing_files.size()) + " missing files";
        if (!patch.issue.empty()) detail += " / " + patch.issue;
        auto* info = label(row, detail, 16, 30, 424, patch.issue.empty() && patch.missing_files.empty() ? muted : 0xf0aa72, &lv_font_montserrat_14);
        lv_label_set_long_mode(info, LV_LABEL_LONG_DOT); lv_obj_set_height(info, 18);
        auto* load = button(row, "Load", 448, 3, 96, Action::LoadStored, 0, patch.id);
        if (!patch.revision) lv_obj_add_state(load, LV_STATE_DISABLED);
        auto* startup = button(row, "Startup", 552, 3, 104, Action::StartupStored, 0, patch.id);
        if (patch.startup) { lv_obj_set_style_border_color(startup, color(accent), 0); lv_obj_set_style_text_color(lv_obj_get_child(startup, 0), color(accent), 0); }
        button(row, "Delete", 664, 3, 80, Action::DeleteStored, 0, patch.id);
    }
    const std::string playing = graph_.patch_id.empty() ? "never saved" : graph_.dirty ? "unsaved edits since its last save" : "saved";
    auto* now = label(parent, "Playing: " + graph_.name + " / " + playing, 24, 252, 752, muted, &lv_font_montserrat_14);
    lv_label_set_long_mode(now, LV_LABEL_LONG_DOT); lv_obj_set_height(now, 18);
    button(parent, "Save", 24, 280, 140, Action::SavePatch, 0, {}, 0);
    button(parent, "Save as", 176, 280, 140, Action::SavePatch, 0, {}, 1);
    button(parent, "New patch", 328, 280, 140, Action::NameEntry, 0, {}, 0);
    button(parent, "Rename", 480, 280, 140, Action::NameEntry, 0, {}, 1);
    if (engine_.capabilities().simulation) button(parent, "Parallel demo", 632, 280, 144, Action::ParallelDemo);
}
void Application::patches(lv_obj_t* parent) {
    if (store_) { stored_patches(parent); return; }
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
void Application::cv_connect(lv_obj_t* parent) {
    const auto* from = find_module(graph_, cv_from_); const auto* to = find_module(graph_, cv_to_);
    if (!from || !to) { label(parent, "Module no longer exists", 24, 24); return; }
    const bool choosing_output = cv_output_.empty();
    const auto ports = cv_ports(choosing_output ? *from : *to, choosing_output ? Direction::Output : Direction::Input);
    auto* prompt = label(parent, choosing_output ? "Which output of " + from->name + "?"
                                                 : from->name + " / " + cv_output_ + "  >  which input of " + to->name + "?", 24, 14, 560, text, &lv_font_montserrat_20);
    lv_label_set_long_mode(prompt, LV_LABEL_LONG_DOT); lv_obj_set_height(prompt, 26);
    const auto count = std::max<std::size_t>(1, (ports.size() + 7) / 8);
    cv_page_ = std::min(cv_page_, count - 1);
    if (count > 1) {
        auto* previous = button(parent, "<", 600, 4, 48, Action::CvPage, 0, {}, static_cast<int>(cv_page_ ? cv_page_ - 1 : 0));
        if (!cv_page_) lv_obj_add_state(previous, LV_STATE_DISABLED);
        label(parent, std::to_string(cv_page_ + 1) + " / " + std::to_string(count), 664, 18, 56, muted, &lv_font_montserrat_14);
        auto* next = button(parent, ">", 728, 4, 48, Action::CvPage, 0, {}, static_cast<int>(std::min(cv_page_ + 1, count - 1)));
        if (cv_page_ + 1 >= count) lv_obj_add_state(next, LV_STATE_DISABLED);
    }
    for (std::size_t i = cv_page_ * 8; i < std::min(ports.size(), cv_page_ * 8 + 8); ++i) {
        const auto slot = i - cv_page_ * 8;
        auto* choice = button(parent, ports[i]->symbol, 24 + static_cast<int>(slot % 2) * 384, 64 + static_cast<int>(slot / 2) * 68, 368, Action::CvChoice, 0, ports[i]->symbol);
        lv_obj_set_style_border_color(choice, color(control_ink), 0);
    }
}
void Application::confirm_replace(lv_obj_t* parent) {
    label(parent, "Current edits have not been saved", 24, 44, 752, text, &lv_font_montserrat_24);
    auto* name = label(parent, graph_.name, 24, 104, 752, accent, &lv_font_montserrat_24); lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
    lv_obj_set_height(name, 32);
    label(parent, std::string("Replace this patch and discard its current unsaved edits?\nCancel to return and save ") + (store_ ? "it first." : "a session snapshot first."), 24, 168, 752, muted);
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
    if (!engine_.capabilities().simulation) {
        label(parent, "Engine connection", 24, 20, 0, text, &lv_font_montserrat_24);
        auto* state = label(parent, status_text() + (engine_note_.empty() ? std::string() : ": " + engine_note_), 24, 65, 752, engine_.state() == EngineState::Ready ? accent : 0xf0aa72);
        lv_label_set_long_mode(state, LV_LABEL_LONG_DOT); lv_obj_set_height(state, 24);
        label(parent, "Disconnecting stops only this interface; the engine keeps playing.\nA lost engine is retried every second.", 24, 96, 752, muted, &lv_font_montserrat_14);
        button(parent, "Connect", 24, 142, 160, Action::State, 0, {}, static_cast<int>(EngineState::Ready));
        button(parent, "Disconnect", 200, 142, 160, Action::State, 0, {}, static_cast<int>(EngineState::Disconnected));
        button(parent, "Controls", 376, 142, 160, Action::Navigate, 0, {}, static_cast<int>(Page::Controls));
        if (power_off_) button(parent, "Shut down", 552, 142, 224, Action::PowerOff);
        label(parent, store_ ? "Patches are stored in " + store_->root().generic_string() : "Patch storage is not available: session snapshots only.", 24, 216, 752, muted, &lv_font_montserrat_14);
        label(parent, "CPU is the engine's mean run load. Xruns, memory and temperature: not measured", 24, 300, 752, muted, &lv_font_montserrat_14);
        return;
    }
    label(parent, "Windows simulator / 800 x 480", 24, 20, 0, text, &lv_font_montserrat_24);
    label(parent, "No audio processing. No hardware connection required.\nUse these states to exercise the interface.", 24, 65, 710, muted);
    button(parent, "Ready", 24, 142, 160, Action::State, 0, {}, static_cast<int>(EngineState::Ready));
    button(parent, "Disconnect", 200, 142, 160, Action::State, 0, {}, static_cast<int>(EngineState::Disconnected));
    button(parent, "Engine error", 376, 142, 180, Action::State, 0, {}, static_cast<int>(EngineState::Error));
    button(parent, "Reject next edit", 24, 216, 220, Action::Reject);
    button(parent, "Missing module", 264, 216, 220, Action::MissingFixture);
    button(parent, "Controls", 504, 216, 220, Action::Navigate, 0, {}, static_cast<int>(Page::Controls));
    label(parent, "Keys: Q/A W/S E/D R/F turn encoders 1-4, keys 1-4 push them, Z X C are the stomps.", 24, 276, 752, muted, &lv_font_montserrat_14);
    label(parent, "DSP, xruns, memory and temperature: not measured", 24, 300, 730, muted, &lv_font_montserrat_14);
}

// Physical controls. Events arrive from whatever stands in for the hardware;
// the recognizer turns them into gestures, applied here on the LVGL thread.
void Application::control(const ControlEvent& event) { recognizer_.input(event); }
const Assignment* Application::pinned(const std::string& control) const {
    const auto it = std::find_if(graph_.assignments.begin(), graph_.assignments.end(), [&](const Assignment& a) { return a.control == control; });
    return it == graph_.assignments.end() ? nullptr : &*it;
}
std::size_t Application::parameter_pages(const Module& m) const {
    const auto layout = module_layout(m);
    if (!generic_parameters_ && layout == ModuleLayout::Clouds) return 2;
    if (!generic_parameters_ && layout == ModuleLayout::Delay) return 1;
    if (!generic_parameters_ && layout == ModuleLayout::Tuner) return 1;
    return std::max<std::size_t>(1, (m.parameters.size() + 3) / 4);
}
void Application::gesture(const Gesture& g) {
    // Typing a name: the keys standing in for controls are text there.
    if (page_ == Page::NamePatch) return;
    active_control_ = g.control;
    if (page_ == Page::Controls) rebuild_ = true;
    if (is_encoder(g.control)) encoder(g);
    else if (is_stomp(g.control)) stomp(g);
}
void Application::encoder(const Gesture& g) {
    const auto name = control_label(g.control);
    // A pinned encoder controls its target on every page; an unpinned one
    // follows the parameter page in view, tile by tile.
    const auto* pin = pinned(g.control);
    InstanceId id = 0;
    std::string symbol;
    if (pin && assignment_parameter(graph_, *pin)) { id = pin->module; symbol = pin->parameter; }
    else if (!pin && page_ == Page::Parameters) {
        const auto index = static_cast<std::size_t>(std::stoul(g.control.substr(3))) - 1;
        if (index < visible_.size()) { id = visible_[index].id; symbol = visible_[index].symbol; }
    }
    const auto* m = id ? find_module(graph_, id) : nullptr;
    const Parameter* p = nullptr;
    if (m) for (const auto& candidate : m->parameters) if (candidate.symbol == symbol) p = &candidate;
    switch (g.kind) {
    case Gesture::Kind::Turn: {
        if (!p) {
            message(pin ? name + " is pinned to a module that is gone. Settings > Controls repairs it." :
                    page_ == Page::Parameters ? name + " has no control on this page." : name + " is not pinned. Open a module's parameters, or Settings > Controls.");
            return;
        }
        // Continue from a value still on its way to the engine, not the last acknowledged one.
        const bool streaming = stream_ && stream_->id == id && stream_->symbol == symbol;
        const auto current = streaming ? stream_->latest : p->value;
        const auto next = turned_value(*p, current, g.steps, g.fine, g.interval_ms, pin);
        auto shown = *p; shown.value = next;
        message(name + (g.fine ? " (fine): " : ": ") + m->name + " / " + p->name + " " + format_value(shown) + (next == current ? " (limit)" : ""));
        if (next == current) return;
        if (p->kind == ParameterKind::Continuous || p->kind == ParameterKind::Integer) stream_parameter(id, symbol, next);
        else submit(SetParameter{id, symbol, next});
        return;
    }
    case Gesture::Kind::Tap:
        if (pin && m) {
            // Show what the pinned encoder controls.
            selected_ = m->id; generic_parameters_ = false;
            const auto index = static_cast<std::size_t>(std::find_if(m->parameters.begin(), m->parameters.end(), [&](const Parameter& q) { return q.symbol == symbol; }) - m->parameters.begin());
            parameter_page_ = module_layout(*m) == ModuleLayout::Generic ? index / 4 : 0;
            navigate(Page::Parameters);
        } else if (page_ == Page::Parameters) {
            if (const auto* shown = find_module(graph_, selected_)) { parameter_page_ = (parameter_page_ + 1) % parameter_pages(*shown); rebuild_ = true; }
        } else message(name + " push: opens what it is pinned to, or the next page of parameters.");
        return;
    case Gesture::Kind::Hold:
        if (pin) {
            submit(Unassign{g.control});
            message(name + " unpinned: it follows the parameter page again.");
        } else if (p) {
            Assignment a;
            a.control = g.control; a.module = id; a.parameter = symbol; a.minimum = p->minimum; a.maximum = p->maximum;
            submit(Assign{a});
            message(name + " pinned to " + m->name + " / " + p->name + " on every page. Hold again to unpin.");
        } else message("Open a module's parameters, then hold an encoder to pin the control it turns.");
        return;
    default: return;
    }
}
void Application::stomp(const Gesture& g) {
    const auto name = control_label(g.control);
    const bool pressed = g.kind == Gesture::Kind::Press;
    if (const auto* pin = pinned(g.control)) {
        const auto* m = assignment_target(graph_, *pin);
        if (!m) { if (pressed) message(name + " is pinned to a module that is gone. Settings > Controls repairs it."); return; }
        // Switch actions go straight to the engine: never coalesced with turns.
        if (pin->action == ControlAction::Bypass) {
            if (pressed) { submit(SetBypass{m->id, !m->bypassed}); message(name + ": " + m->name + (m->bypassed ? " on" : " bypassed")); }
            return;
        }
        const auto* p = assignment_parameter(graph_, *pin);
        const auto value = switched_value(*p, p->value, *pin, pressed);
        if (!value || *value == p->value) return;
        auto shown = *p; shown.value = *value;
        submit(SetParameter{m->id, p->symbol, *value});
        message(name + ": " + m->name + " / " + p->name + " " + format_value(shown));
        return;
    }
    if (!pressed) return;
    const auto global = store_ ? store_->global_controls() : std::map<std::string, std::string>{};
    if (const auto it = global.find(g.control); it != global.end()) { step_patch(it->second == "next-patch" ? 1 : -1); return; }
    message(name + " is not assigned. Settings > Controls.");
}
void Application::step_patch(int direction) {
    refresh_stored();
    std::vector<const StoredPatch*> loadable;
    for (const auto& patch : stored_) if (patch.revision) loadable.push_back(&patch);
    if (loadable.empty()) { message("No saved patches to step through."); return; }
    const auto here = std::find_if(loadable.begin(), loadable.end(), [&](const StoredPatch* p) { return p->id == graph_.patch_id; });
    std::size_t next = 0;
    if (here == loadable.end()) next = direction > 0 ? 0 : loadable.size() - 1;
    else next = (static_cast<std::size_t>(here - loadable.begin()) + loadable.size() + direction) % loadable.size();
    // Unsaved edits are guarded as on screen: the footswitch never discards them silently.
    load_stored(loadable[next]->id, false, true);
}
std::string Application::assignment_text(const std::string& control) const {
    if (const auto* a = pinned(control)) {
        const auto* m = assignment_target(graph_, *a);
        if (!m) return "Orphaned: its module is gone or changed. Edit to clear it.";
        if (a->action == ControlAction::Bypass) return m->name + " / bypass";
        const auto* p = assignment_parameter(graph_, *a);
        auto low = *p, high = *p; low.value = a->minimum; high.value = a->maximum;
        std::string summary = m->name + " / " + p->name;
        if (a->minimum != p->minimum || a->maximum != p->maximum) summary += ", " + format_value(low) + " to " + format_value(high);
        if (a->inverted) summary += ", inverted";
        if (is_stomp(control)) summary += a->mode == SwitchMode::Momentary ? ", momentary" : ", toggle";
        return summary;
    }
    if (store_) if (const auto global = store_->global_controls(); global.contains(control))
        return global.at(control) == "next-patch" ? "Next patch (every patch)" : "Previous patch (every patch)";
    return is_encoder(control) ? "Follows the parameter page" : "Not assigned";
}
void Application::controls(lv_obj_t* parent) {
    const auto ids = layout_.ids();
    for (std::size_t i = 0; i < ids.size(); ++i) {
        const auto& id = ids[i];
        auto* row = panel(parent, 24, 8 + static_cast<int>(i) * 50, 752, 48);
        // Turning or pressing a control shows which row it is.
        if (id == active_control_) lv_obj_set_style_border_color(row, color(accent), 0);
        label(row, control_label(id), 14, 13, 120, id == active_control_ ? accent : text);
        const auto* pin = pinned(id);
        const bool orphan = pin && !assignment_target(graph_, *pin);
        auto* what = label(row, assignment_text(id), 140, 14, 396, orphan ? 0xf0aa72 : pin ? text : muted, &lv_font_montserrat_14);
        lv_label_set_long_mode(what, LV_LABEL_LONG_DOT); lv_obj_set_height(what, 18);
        button(row, "Assign", 544, 0, 100, Action::AssignControl, 0, id);
        auto* edit = button(row, "Edit", 652, 0, 100, Action::EditControl, 0, id);
        if (!pin && !(store_ && store_->global_controls().contains(id))) lv_obj_add_state(edit, LV_STATE_DISABLED);
    }
}
void Application::assign_picker(lv_obj_t* parent) {
    struct Choice { std::string title; Action action; InstanceId id; std::string symbol; };
    std::vector<Choice> choices;
    const bool stomp = is_stomp(editing_control_);
    if (!picking_module_) {
        if (stomp && store_) {
            choices.push_back({"Next patch", Action::PickTarget, 0, "@next-patch"});
            choices.push_back({"Previous patch", Action::PickTarget, 0, "@previous-patch"});
        }
        for (const auto& m : graph_.modules)
            if (!m.fixed && module_available(m) && (stomp || !m.parameters.empty())) choices.push_back({m.name, Action::PickModule, m.id, {}});
    } else if (const auto* m = find_module(graph_, picking_module_)) {
        if (stomp) choices.push_back({"Bypass", Action::PickTarget, 0, "@bypass"});
        for (const auto& p : m->parameters) choices.push_back({p.name, Action::PickTarget, 0, p.symbol});
    }
    auto* prompt = label(parent, picking_module_ ? "Which control?" : stomp ? "A module, or a patch change for every patch" : "Which module?", 24, 18, 560, text, &lv_font_montserrat_20);
    lv_label_set_long_mode(prompt, LV_LABEL_LONG_DOT); lv_obj_set_height(prompt, 26);
    if (choices.empty()) { label(parent, "Nothing in this patch can be controlled yet.", 24, 72, 752, muted); return; }
    const auto pages = std::max<std::size_t>(1, (choices.size() + 7) / 8);
    picker_page_ = std::min(picker_page_, pages - 1);
    if (pages > 1) {
        auto* previous = button(parent, "<", 600, 4, 48, Action::PickerPage, 0, {}, static_cast<int>(picker_page_ ? picker_page_ - 1 : 0));
        if (!picker_page_) lv_obj_add_state(previous, LV_STATE_DISABLED);
        label(parent, std::to_string(picker_page_ + 1) + " / " + std::to_string(pages), 664, 18, 56, muted, &lv_font_montserrat_14);
        auto* next = button(parent, ">", 728, 4, 48, Action::PickerPage, 0, {}, static_cast<int>(std::min(picker_page_ + 1, pages - 1)));
        if (picker_page_ + 1 >= pages) lv_obj_add_state(next, LV_STATE_DISABLED);
    }
    for (std::size_t i = picker_page_ * 8; i < std::min(choices.size(), picker_page_ * 8 + 8); ++i) {
        const auto slot = i - picker_page_ * 8;
        auto* choice = button(parent, choices[i].title, 24 + static_cast<int>(slot % 2) * 384, 64 + static_cast<int>(slot / 2) * 68, 368,
                              choices[i].action, choices[i].id, choices[i].symbol);
        if (auto* title = lv_obj_get_child(choice, 0)) { lv_label_set_long_mode(title, LV_LABEL_LONG_DOT); lv_obj_set_width(title, 340); lv_obj_center(title); }
    }
}
void Application::assignment_detail(lv_obj_t* parent) {
    const auto& id = editing_control_;
    auto* heading = label(parent, assignment_text(id), 24, 16, 752, accent, &lv_font_montserrat_20);
    lv_label_set_long_mode(heading, LV_LABEL_LONG_DOT); lv_obj_set_height(heading, 26);
    button(parent, "Clear", 24, 280, 160, Action::ClearControl, 0, id);
    button(parent, "Done", 616, 280, 160, Action::Navigate, 0, {}, static_cast<int>(Page::Controls));
    const auto* a = pinned(id);
    if (!a) { label(parent, "This control acts the same in every patch.", 24, 64, 752, muted); return; }
    const auto* p = assignment_parameter(graph_, *a);
    if (!assignment_target(graph_, *a)) {
        label(parent, "Its module was removed, or now holds a different plugin, so it is never applied.\nClear it, then assign the control again.", 24, 64, 752, muted);
        return;
    }
    if (!p) { label(parent, "Each press switches the module's software bypass.", 24, 64, 752, muted); return; }
    auto* invert = button(parent, a->inverted ? "Inverted" : "Invert", 24, 60, 160, Action::InvertControl);
    if (a->inverted) lv_obj_set_style_border_color(invert, color(accent), 0);
    if (is_stomp(id)) {
        auto* toggle = button(parent, "Toggle", 200, 60, 160, Action::SwitchModeChoice, 0, {}, 0);
        auto* momentary = button(parent, "Momentary", 376, 60, 160, Action::SwitchModeChoice, 0, {}, 1);
        lv_obj_set_style_border_color(a->mode == SwitchMode::Momentary ? momentary : toggle, color(accent), 0);
    }
    label(parent, is_stomp(id) ? (a->mode == SwitchMode::Momentary ? "Held: To. Released: From." : "Each press: From, then To.")
                               : "Clockwise moves from From towards To; push and turn for fine steps.", 24, 120, 752, muted, &lv_font_montserrat_14);
    if (p->kind == ParameterKind::Toggle || p->kind == ParameterKind::Enumeration) return;
    // The two ends of the range, in the parameter's own (linear or logarithmic) scale.
    for (int end = 0; end < 2; ++end) {
        const int y = 148 + end * 64;
        auto shown = *p; shown.value = end ? a->maximum : a->minimum;
        label(parent, end ? "To" : "From", 24, y + 8, 80, text);
        auto* value = label(parent, format_value(shown), 600, y + 8, 176, accent);
        lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
        auto* slider = lv_slider_create(parent); lv_obj_set_pos(slider, 120, y + 14); lv_obj_set_size(slider, 460, 12);
        lv_slider_set_range(slider, 0, 1000); lv_slider_set_value(slider, static_cast<int>(normalized_value(shown) * 1000), LV_ANIM_OFF);
        lv_obj_set_style_bg_color(slider, color(border), LV_PART_MAIN); lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(slider, color(accent), LV_PART_INDICATOR); lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(slider, color(text), LV_PART_KNOB); lv_obj_set_style_pad_all(slider, 10, LV_PART_KNOB);
        lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_KNOB); lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);
        lv_obj_set_ext_click_area(slider, 20);
        bind(slider, Action::RangeEnd, 0, {}, end, value);
    }
}
}
