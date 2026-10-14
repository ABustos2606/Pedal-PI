// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#pragma once
#include "engine/engine.hpp"
#include "lvgl.h"
#include <deque>
#include <map>

namespace pedal {
enum class Page { Patch, Browser, Parameters, Patches, Settings, ModuleActions, NamePatch, ConfirmReplace, CvConnect };
class Application {
public:
    explicit Application(Engine&);
    ~Application();
    void tick(unsigned milliseconds);
    const Graph& graph() const { return graph_; }
    Page page() const { return page_; }
    bool pending() const { return !pending_.empty(); }
    const std::string& message() const { return message_; }
    lv_obj_t* screen() const { return screen_; }
private:
    enum class Action { Navigate, Add, Module, Port, SelectCable, DeleteCable, ClearCable, Pan, Home,
                        OpenActions, Edit, Duplicate, Parameter, Bypass, Remove, SaveSession, LoadSession, State, Reject,
                        NameEntry, NameInput, Keyboard, CommitName, Confirm, CancelReplace, ParameterPage, BrowserPage, AvailabilityInfo, MissingFixture,
                        ParameterLayout, ParameterChoice, ParallelDemo, CvChoice, CvPage };
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
    enum class PendingKind { Edit, Rename, Replace };
    std::map<RequestId, PendingKind> pending_;
    std::size_t parameter_page_ = 0, browser_page_ = 0;
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
