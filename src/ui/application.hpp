// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#pragma once
#include "controls/controls.hpp"
#include "engine/engine.hpp"
#include "storage/patch_store.hpp"
#include "lvgl.h"
#include <deque>
#include <functional>
#include <map>

namespace pedal {
enum class Page { Patch, Browser, Parameters, Patches, Settings, ModuleActions, NamePatch, ConfirmReplace, CvConnect, ConfirmAction,
                  Controls, AssignPicker, AssignmentDetail };
class Application {
public:
    // Without a store the Patches page offers the in-memory session snapshot.
    explicit Application(Engine&, PatchStore* = nullptr);
    ~Application();
    void tick(unsigned milliseconds);
    const Graph& graph() const { return graph_; }
    Page page() const { return page_; }
    bool pending() const { return !pending_.empty(); }
    const std::string& message() const { return message_; }
    lv_obj_t* screen() const { return screen_; }
    // A raw event from a physical control, or from keys standing in for one.
    void control(const ControlEvent&);
    const ControlLayout& control_layout() const { return layout_; }
private:
    enum class Action { Navigate, Add, Module, Port, SelectCable, DeleteCable, ClearCable, Pan, Home,
                        OpenActions, Edit, Duplicate, Parameter, Bypass, Remove, SaveSession, LoadSession, State, Reject,
                        NameEntry, NameInput, Keyboard, CommitName, Confirm, CancelReplace, ParameterPage, BrowserGroup, MissingFixture,
                        ParameterLayout, ParameterChoice, ParallelDemo, CvChoice, CvPage,
                        SavePatch, LoadStored, DeleteStored, StartupStored, PatchPage, ConfirmAction, CancelAction,
                        AssignControl, EditControl, ClearControl, PickModule, PickTarget, PickerPage, InvertControl, SwitchModeChoice, RangeEnd };
    struct Binding {
        Application* app;
        Action action;
        InstanceId id = 0;
        std::string symbol;
        int number = 0;
        lv_obj_t* value_label = nullptr;
    };
    static void event(lv_event_t*);
    void handle(Binding&, lv_event_t*);
    RequestId submit(Command);
    void stream_parameter(InstanceId, const std::string& symbol, double value);
    void send_stream();
    void navigate(Page);
    void rebuild();
    void patch_view(lv_obj_t*);
    void browser(lv_obj_t*);
    void parameters(lv_obj_t*);
    void parameter_tile(lv_obj_t*, const Module&, const Parameter&, int x, int y, int height = 120, std::uint32_t ink = 0x8edbc2);
    void clouds_parameters(lv_obj_t*, const Module&);
    void delay_parameters(lv_obj_t*, const Module&);
    void patches(lv_obj_t*);
    void settings(lv_obj_t*);
    void module_actions(lv_obj_t*);
    void name_patch(lv_obj_t*);
    void confirm_replace(lv_obj_t*);
    void cv_connect(lv_obj_t*);
    void commit_name();
    void replace_patch(Command, Page origin);
    void confirm_action(lv_obj_t*);
    void stored_patches(lv_obj_t*);
    void refresh_stored();
    void save_patch(bool as_new);
    void start_save(const std::string& id);
    void load_stored(const std::string& id, bool allow_missing, bool guarded);
    void ask(std::string title, std::string heading, std::string body, std::string button, std::function<void()> act);
    std::string status_text() const;
    std::string engine_banner() const;
    void gesture(const Gesture&);
    void encoder(const Gesture&);
    void stomp(const Gesture&);
    void step_patch(int direction);
    const Assignment* pinned(const std::string& control) const;
    std::size_t parameter_pages(const Module&) const;
    void controls(lv_obj_t*);
    void assign_picker(lv_obj_t*);
    void assignment_detail(lv_obj_t*);
    std::string assignment_text(const std::string& control) const;
    void update_viewport();
    void update_cables();
    void update_metrics();
    void indicate_targets(bool);
    std::optional<std::pair<int, int>> vacant_slot() const;
    void reveal(int x, int y);
    void bind(lv_obj_t*, Action, InstanceId = 0, std::string = {}, int = 0, lv_obj_t* = nullptr);
    lv_obj_t* button(lv_obj_t*, const std::string&, int, int, int, Action, InstanceId = 0, std::string = {}, int = 0);
    void message(std::string);
    Engine& engine_;
    Graph graph_;
    Graph saved_;
    bool has_saved_ = false;
    Page page_ = Page::Patch;
    InstanceId selected_ = 0;
    bool rebuild_ = true;
    std::string message_ = "Drag space to pan. Hold a module for actions. Drag output to input.";
    enum class PendingKind { Edit, Rename, Replace, Add, Save, SaveAs };
    std::map<RequestId, PendingKind> pending_;
    // Module names of adds in flight, to say which plugin failed.
    std::map<RequestId, std::string> adding_;
    // Saved patches on disk; null when the application has no store.
    PatchStore* store_ = nullptr;
    std::vector<StoredPatch> stored_;
    std::size_t patch_page_ = 0;
    // A save the engine is writing, published once it answers.
    std::optional<PatchStore::Staging> staging_;
    std::string save_as_id_, load_note_;
    bool saving_as_ = false;
    // Load the startup patch once the engine is ready, if it plays no stored patch.
    bool startup_pending_ = false;
    // The connection: whether it was ever ready, and the last reason given for its state.
    bool ever_ready_ = false;
    EngineState last_state_ = EngineState::Ready;
    std::string engine_note_;
    struct Confirmation { std::string title, heading, body, button; std::function<void()> act; };
    Confirmation confirmation_;
    // Physical controls: gestures, and what the encoders follow on a parameter page.
    ControlLayout layout_;
    ControlRecognizer recognizer_;
    struct Visible { InstanceId id; std::string symbol; };
    std::vector<Visible> visible_;
    // The control last used, highlighted on the Controls page; the one being assigned or edited.
    std::string active_control_, editing_control_;
    InstanceId picking_module_ = 0;
    std::size_t picker_page_ = 0;
    std::size_t parameter_page_ = 0;
    // The module browser: the one open category, and where the list was scrolled.
    std::string browser_group_;
    int browser_scroll_ = 0;
    bool browser_reveal_ = false;
    lv_obj_t* browser_list_ = nullptr;
    bool generic_parameters_ = false;
    bool renaming_ = false;
    std::string draft_name_;
    std::optional<Command> replacement_;
    Page confirm_origin_ = Page::Patches;
    lv_obj_t *screen_ = nullptr, *canvas_ = nullptr, *footer_ = nullptr;
    lv_obj_t *tempo_value_ = nullptr, *cpu_value_ = nullptr;
    lv_indev_t* pointer_input_ = nullptr;
    lv_indev_t* keypad_input_ = nullptr;
    std::deque<Binding> bindings_;
    std::map<InstanceId, lv_obj_t*> module_widgets_;
    std::multimap<std::pair<InstanceId, std::string>, lv_obj_t*> input_jacks_;
    std::vector<lv_obj_t*> dots_;
    std::optional<Connection> selected_cable_;
    // One drawn line per channel of a logical cable.
    struct Cable { lv_obj_t* line; Connection connection; lv_point_precise_t points[4]; RoutingPlan routing; unsigned out_channel = 0, in_channel = 0; bool cv = false; };
    std::deque<Cable> cables_;
    lv_obj_t* preview_ = nullptr;
    lv_point_precise_t preview_points_[2]{};
    InstanceId cable_source_ = 0;
    std::string cable_port_;
    // A continuous gesture in progress: one request in flight, newest value wins.
    struct Stream { InstanceId id; std::string symbol; double latest = 0; std::optional<double> sent; RequestId in_flight = 0; };
    std::optional<Stream> stream_;
    lv_point_t press_{};
    int start_x_ = 0, start_y_ = 0;
    int pan_x_ = 0, pan_y_ = 0;
    bool dragging_ = false, panning_ = false, long_pressed_ = false;
    // Control (CV) cables: one connector per module side, the port is chosen from a list.
    bool cable_cv_ = false;
    InstanceId cv_from_ = 0, cv_to_ = 0;
    std::string cv_output_;
    std::size_t cv_page_ = 0;
    std::map<InstanceId, lv_obj_t*> cv_inputs_;
};
}
