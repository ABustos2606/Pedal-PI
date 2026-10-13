// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#include "ui/application.hpp"
#include "engine/simulated_engine.hpp"
// Test-only geometry access for real pointer presses on the pinned keyboard.
#include "src/widgets/buttonmatrix/lv_buttonmatrix_private.h"
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
std::uint32_t clock_ms = 0;
lv_point_t pointer{};
bool pressed = false;
std::uint32_t keypad_key = 0;
bool keypad_pressed = false;
void check(bool value, const char* reason) { if (!value) throw std::runtime_error(reason); }
void read_pointer(lv_indev_t*, lv_indev_data_t* data) {
    data->point = pointer; data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
void read_keypad(lv_indev_t*, lv_indev_data_t* data) {
    data->key = keypad_key; data->state = keypad_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
void step(pedal::Application& app, unsigned duration = 192) {
    for (unsigned i = 0; i < duration; i += 16) {
        clock_ms += 16; lv_timer_handler(); app.tick(16);
    }
}
void click(pedal::Application& app, int x, int y) {
    pointer = {x, y}; pressed = true; step(app, 64); pressed = false; step(app);
}
void keypad_press(pedal::Application& app, std::uint32_t key) {
    keypad_key = key; keypad_pressed = true; step(app, 64); keypad_pressed = false; step(app);
}
void drag(pedal::Application& app, int ax, int ay, int bx, int by) {
    pointer = {ax, ay}; pressed = true; step(app, 64);
    for (int i = 1; i <= 8; ++i) { pointer = {ax + (bx - ax) * i / 8, ay + (by - ay) * i / 8}; step(app, 32); }
    pressed = false; step(app);
}
void hold(pedal::Application& app, int x, int y) {
    pointer = {x, y}; pressed = true; step(app, 640); pressed = false; step(app);
}
lv_obj_t* widget(lv_obj_t* root, const lv_obj_class_t* type) {
    if (lv_obj_check_type(root, type)) return root;
    for (unsigned i = 0; i < lv_obj_get_child_count(root); ++i)
        if (auto* found = widget(lv_obj_get_child(root, i), type)) return found;
    return nullptr;
}
void keyboard_key(pedal::Application& app, const std::string& key) {
    auto* kb = widget(app.screen(), &lv_keyboard_class); check(kb != nullptr, "keyboard missing");
    auto* matrix = reinterpret_cast<lv_buttonmatrix_t*>(kb);
    lv_area_t bounds; lv_obj_get_coords(kb, &bounds);
    for (unsigned i = 0; i < matrix->btn_cnt; ++i) if (key == lv_buttonmatrix_get_button_text(kb, i)) {
        const auto area = matrix->button_areas[i];
        check(area.y2 - area.y1 + 1 >= 44, "keyboard key shorter than touch target");
        check(bounds.y1 + area.y1 >= 72 && bounds.y1 + area.y2 < 432, "keyboard key clipped by viewport");
        click(app, bounds.x1 + (area.x1 + area.x2) / 2, bounds.y1 + (area.y1 + area.y2) / 2); return;
    }
    throw std::runtime_error("keyboard key unavailable: " + key);
}
void word(std::ofstream& f, std::uint32_t n, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) f.put(static_cast<char>((n >> (i * 8)) & 255));
}
void capture(pedal::Application& app, const std::filesystem::path& path) {
    lv_obj_update_layout(app.screen());
    auto* buf = lv_snapshot_take(app.screen(), LV_COLOR_FORMAT_RGB888);
    check(buf != nullptr, "snapshot failed");
    check(buf->header.w == 800 && buf->header.h == 480, "logical display dimensions changed");
    std::ofstream f(path, std::ios::binary); check(f.good(), "screenshot file unavailable");
    const unsigned stride = (800 * 3 + 3) & ~3U;
    f.put('B'); f.put('M'); word(f, 54 + stride * 480, 4); word(f, 0, 4); word(f, 54, 4);
    word(f, 40, 4); word(f, 800, 4); word(f, 480, 4); word(f, 1, 2); word(f, 24, 2);
    word(f, 0, 4); word(f, stride * 480, 4); word(f, 2835, 4); word(f, 2835, 4); word(f, 0, 4); word(f, 0, 4);
    for (int y = 479; y >= 0; --y) f.write(reinterpret_cast<const char*>(buf->data + y * buf->header.stride), stride);
    lv_draw_buf_destroy(buf);
}
}
int main(int argc, char** argv) {
    try {
        const auto output = std::filesystem::path(argc > 1 ? argv[1] : "screenshots");
        std::filesystem::create_directories(output);
        lv_init(); lv_tick_set_cb([] { return clock_ms; });
        auto* display = lv_display_create(800, 480);
        std::vector<std::uint32_t> framebuffer(800 * 480);
        lv_display_set_color_format(display, LV_COLOR_FORMAT_XRGB8888);
        lv_display_set_buffers(display, framebuffer.data(), nullptr, static_cast<std::uint32_t>(framebuffer.size() * 4), LV_DISPLAY_RENDER_MODE_FULL);
        lv_display_set_flush_cb(display, [](lv_display_t* d, const lv_area_t*, std::uint8_t*) { lv_display_flush_ready(d); });
        auto* indev = lv_indev_create(); lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev, read_pointer); lv_indev_set_display(indev, display);
        auto* group = lv_group_create(); lv_group_set_default(group);
        auto* keypad = lv_indev_create(); lv_indev_set_type(keypad, LV_INDEV_TYPE_KEYPAD);
        lv_indev_set_read_cb(keypad, read_keypad); lv_indev_set_display(keypad, display); lv_indev_set_group(keypad, group);
        pedal::SimulatedEngine engine;
        {
            pedal::Application app(engine); step(app);
            capture(app, output / "01-patch.bmp");
            click(app, 225, 252); check(app.page() == pedal::Page::Parameters, "tap did not open parameters");
            capture(app, output / "02-parameters.bmp");
            drag(app, 150, 218, 325, 218);
            check(pedal::find_module(app.graph(), 3)->parameters[0].value > 70, "slider did not update authoritative value");
            click(app, 68, 36); check(app.page() == pedal::Page::Patch, "back failed");
            click(app, 155, 228);
            check(app.graph().connections.size() == 8 && !app.pending(), "cable selection changed graph");
            capture(app, output / "10-selected-cable.bmp");
            click(app, 344, 456);
            check(app.graph().connections.size() == 8, "deselect deleted cable");
            click(app, 155, 228); click(app, 500, 456);
            check(app.graph().connections.size() == 7, "explicit cable delete failed");
            check(app.graph().connections.front().from == 3, "cable hit test removed the wrong cable");
            pointer = {128, 228}; pressed = true; step(app, 64);
            capture(app, output / "14-cable-targets.bmp");
            pressed = false; step(app);
            drag(app, 128, 228, 184, 228);
            check(app.graph().connections.size() == 8, "reconnect cable failed");
            click(app, 398, 244);
            click(app, 588, 108); // All: retain coverage of the generic parameter editor.
            click(app, 468, 359);
            check(pedal::find_module(app.graph(), 4)->parameters[3].value == 1, "freeze switch failed");
            capture(app, output / "07-clouds.bmp");
            click(app, 496, 108);
            capture(app, output / "19-parameter-page-two.bmp");
            drag(app, 150, 218, 325, 218);
            check(pedal::find_module(app.graph(), 4)->parameters[4].value > 70, "second parameter page changed wrong value");
            drag(app, 575, 218, 740, 218);
            const auto pitch = pedal::find_module(app.graph(), 4)->parameters[5].value;
            check(pitch > 12 && pitch == std::round(pitch), "integer slider did not quantize pitch");
            click(app, 296, 108);
            check(pedal::find_module(app.graph(), 4)->parameters[3].value == 1, "page switch lost freeze value");
            click(app, 68, 36);
            click(app, 562, 244);
            click(app, 588, 108);
            click(app, 570, 361);
            capture(app, output / "09-mode-menu.bmp");
            click(app, 534, 420);
            check(pedal::find_module(app.graph(), 5)->parameters[3].value == 1, "mode selector failed");
            capture(app, output / "08-delay.bmp");
            click(app, 68, 36);
            drag(app, 225, 244, 225, 124);
            check(pedal::find_module(app.graph(), 3)->y == 24, "module drag failed to snap");
            check(app.page() == pedal::Page::Patch, "drag accidentally opened parameters");
            click(app, 490, 36); check(app.page() == pedal::Page::Browser, "browser navigation failed");
            capture(app, output / "03-browser.bmp");
            click(app, 705, 369); check(app.graph().modules.size() == 6, "add module failed");
            check(app.page() == pedal::Page::Patch, "add did not return to patch");
            // Drop a cable from IN onto the added Reverb's input hit area.
            const auto* reverb = pedal::find_module(app.graph(), 6); check(reverb != nullptr, "new instance missing");
            drag(app, 128, 228, reverb->x, reverb->y + 96);
            check(app.graph().connections.size() == 9, "output to input drag failed");
            click(app, 707, 36); check(app.page() == pedal::Page::Settings, "settings navigation failed");
            capture(app, output / "04-settings.bmp");
            click(app, 273, 238); check(engine.state() == pedal::EngineState::Disconnected, "disconnect control failed");
            click(app, 70, 36); click(app, 396, 244);
            check(app.page() == pedal::Page::Parameters, "module navigation while disconnected failed");
            const auto old_mix = pedal::find_module(app.graph(), 4)->parameters[2].value;
            drag(app, 150, 354, 325, 354);
            check(pedal::find_module(app.graph(), 4)->parameters[2].value == old_mix, "disconnected slider changed engine");
            click(app, 70, 36); click(app, 716, 36); click(app, 104, 238);
            check(engine.state() == pedal::EngineState::Ready, "reconnect control failed");
            click(app, 135, 312); click(app, 70, 36); click(app, 396, 244);
            click(app, 75, 108); check(!pedal::find_module(app.graph(), 4)->bypassed, "rejected bypass accepted");
            check(app.message().find("rejection") != std::string::npos, "rejection feedback missing");
            click(app, 75, 108); check(pedal::find_module(app.graph(), 4)->bypassed, "accepted bypass missing");
            click(app, 70, 36); click(app, 600, 36); click(app, 130, 364);
            capture(app, output / "05-session.bmp");
            click(app, 70, 36); click(app, 396, 244); click(app, 182, 108);
            check(app.graph().modules.size() == 5, "delete module failed");
            click(app, 600, 36); click(app, 700, 264);
            check(app.page() == pedal::Page::ConfirmReplace, "unsaved patch load was not guarded");
            click(app, 650, 364);
            check(app.graph().modules.size() == 6, "session restore failed");
            capture(app, output / "06-edited-patch.bmp");
            // Pan the view without changing any graph coordinates or dirty state.
            const auto before_pan = app.graph();
            drag(app, 420, 384, 260, 384);
            for (std::size_t i = 0; i < app.graph().modules.size(); ++i)
                check(app.graph().modules[i].x == before_pan.modules[i].x && app.graph().modules[i].y == before_pan.modules[i].y,
                    "panning changed module coordinates");
            check(app.graph().dirty == before_pan.dirty && !app.pending(), "panning submitted an engine edit");
            capture(app, output / "11-panned-canvas.bmp");
            // Cable hit testing and module gestures use the same viewport transform.
            click(app, 484, 228); check(app.graph().connections.size() == 9, "panned cable tap deleted cable");
            click(app, 500, 456); check(app.graph().connections.size() == 8, "panned cable delete missed target");
            drag(app, 456, 228, 504, 228); check(app.graph().connections.size() == 9, "panned cable reconnection failed");
            drag(app, 408, 244, 456, 244);
            check(pedal::find_module(app.graph(), 5)->x == 576, "panned module movement lost world coordinates");
            hold(app, 236, 244); check(app.page() == pedal::Page::ModuleActions, "long press did not open actions");
            capture(app, output / "12-module-actions.bmp");
            const auto source = *pedal::find_module(app.graph(), 4);
            click(app, 170, 252); check(app.graph().modules.size() == 7, "context duplicate failed");
            const auto copy = app.graph().modules.back();
            check(copy.id != source.id && copy.uri == source.uri && copy.bypassed == source.bypassed &&
                copy.parameters[3].value == source.parameters[3].value, "UI duplicate did not preserve state");
            check(app.graph().connections.size() == 9, "UI duplicate copied cables");
            click(app, 276, 36); // Home resets the viewport, not the graph.
            click(app, source.x + 48, source.y + 120);
            check(app.page() == pedal::Page::Parameters, "Home did not restore original module location");
            click(app, 705, 108); check(app.page() == pedal::Page::ModuleActions, "parameter Actions button failed");
            click(app, 170, 180); check(app.page() == pedal::Page::Parameters, "context Parameters action failed");
            click(app, 70, 36);
            engine.reject_next();
            hold(app, source.x + 48, source.y + 120); click(app, 170, 252);
            check(app.graph().modules.size() == 7 && app.message().find("rejection") != std::string::npos,
                "rejected context duplicate changed graph or lost feedback");
            hold(app, copy.x + 48, copy.y + 120);
            check(app.page() == pedal::Page::ModuleActions, "duplicate module did not open context actions");
            click(app, 600, 180);
            check(pedal::find_module(app.graph(), copy.id)->bypassed != copy.bypassed, "context software bypass failed");
            click(app, 600, 252);
            check(!pedal::find_module(app.graph(), copy.id) && app.graph().connections.size() == 9, "context delete changed unrelated cables");
            // Run enough redraw cycles to expose stale bindings and queued callbacks.
            for (int i = 0; i < 10; ++i) { click(app, 490, 36); click(app, 70, 36); }
            check(!app.pending(), "pending requests never resolved");
            // Replace the graph through the backend and fill the visible grid.
            auto dense = pedal::demo_graph(); dense.name = "Dense patch";
            dense.modules.resize(2); dense.connections.clear();
            dense.modules[0].x = 24; dense.modules[0].y = 24;
            dense.modules[1].x = 600; dense.modules[1].y = 264;
            const auto* drive = pedal::definition("urn:pedalpi:demo:drive");
            pedal::InstanceId id = 10;
            for (int y : {24, 144, 264}) for (int x : {24, 168, 312, 456, 600}) {
                if ((x == 24 && y == 24) || (x == 600 && y == 264)) continue;
                dense.modules.push_back({id++, drive->uri, drive->name, x, y, false, false, drive->parameters});
            }
            engine.submit(pedal::LoadGraph{dense}); step(app, 256);
            check(app.graph().name == "Dense patch" && app.graph().modules.size() == 15, "backend graph replacement was not displayed");
            capture(app, output / "15-dense-canvas.bmp");
            click(app, 490, 36); click(app, 705, 141);
            check(app.graph().modules.size() == 16 && app.graph().modules.back().x > 672, "full viewport prevented adding beyond it");
            capture(app, output / "13-larger-canvas.bmp");
            click(app, 72, 144);
            check(app.page() == pedal::Page::Parameters, "new offscreen module was not revealed after add");
            click(app, 68, 36); click(app, 276, 36);
            engine.submit(pedal::LoadGraph{pedal::demo_graph()}); step(app, 256);
            click(app, 707, 36); click(app, 374, 312);
            check(app.page() == pedal::Page::Patch && app.graph().connections.size() == 8 &&
                !pedal::module_available(*pedal::find_module(app.graph(), 4)), "missing module fixture lost graph");
            capture(app, output / "16-missing-module.bmp");
            click(app, 396, 244); check(app.page() == pedal::Page::Parameters, "missing placeholder could not be inspected");
            capture(app, output / "17-missing-parameters.bmp");
            check(widget(app.screen(), &lv_slider_class) == nullptr, "missing module exposed editable sliders");
            click(app, 68, 36); hold(app, 396, 244); click(app, 170, 252);
            check(app.graph().modules.size() == 5 && !app.pending(), "disabled missing-module duplication submitted an edit");
            click(app, 600, 252); check(app.graph().modules.size() == 4 && app.graph().connections.size() == 4, "missing placeholder deletion failed");
            click(app, 490, 36); click(app, 656, 96);
            capture(app, output / "18-catalogue-status.bmp");
            click(app, 100, 220); check(app.message().find("compatibility") != std::string::npos, "untested entry reason missing");
            click(app, 705, 217); check(app.graph().modules.size() == 4 && !app.pending(), "untested catalogue entry could be added");
            click(app, 100, 298); check(app.message().find("asset") != std::string::npos, "unavailable entry reason missing");
            click(app, 705, 293); check(app.graph().modules.size() == 4, "unavailable catalogue entry could be added");
            click(app, 705, 141); check(app.graph().modules.back().uri == "urn:pedalpi:demo:mono", "available mono fixture could not be added");

            // Create, cancel and rename through actual on-screen keyboard presses.
            click(app, 600, 36); click(app, 320, 364);
            check(app.page() == pedal::Page::NamePatch, "new patch naming did not open");
            click(app, 710, 36); check(app.graph().modules.size() == 5 && app.message().find("Enter a patch name") != std::string::npos, "blank name was accepted");
            for (const auto* key : {"t", "e", "s", "t"}) keyboard_key(app, key);
            capture(app, output / "20-name-keyboard.bmp");
            click(app, 710, 36); check(app.page() == pedal::Page::ConfirmReplace, "new patch did not guard unsaved edits");
            capture(app, output / "21-replace-confirmation.bmp");
            click(app, 130, 364); check(app.page() == pedal::Page::NamePatch && app.graph().modules.size() == 5, "cancel replaced graph or lost draft");
            check(std::string(lv_textarea_get_text(widget(app.screen(), &lv_textarea_class))) == "test", "cancel lost typed name");
            keyboard_key(app, LV_SYMBOL_BACKSPACE); keyboard_key(app, "e");
            keyboard_key(app, LV_SYMBOL_OK);
            check(app.page() == pedal::Page::ConfirmReplace, "keyboard OK did not request create");
            engine.reject_next(); click(app, 650, 364);
            check(app.graph().modules.size() == 5 && app.message().find("rejection") != std::string::npos, "rejected new patch replaced graph");
            click(app, 650, 364); check(app.page() == pedal::Page::Patch && app.graph().name == "tese" && app.graph().modules.size() == 2, "new patch accepted wrong name or endpoints");
            capture(app, output / "22-new-patch.bmp");
            click(app, 600, 36); click(app, 505, 364);
            keyboard_key(app, LV_SYMBOL_BACKSPACE); keyboard_key(app, "t");
            keyboard_key(app, LV_SYMBOL_OK);
            check(app.page() == pedal::Page::Patches && app.graph().name == "test", "rename via keyboard failed");
            click(app, 130, 364); check(!app.graph().dirty, "accepted session save did not clear dirty marker");
            click(app, 320, 364); keyboard_key(app, "a"); keyboard_key(app, LV_SYMBOL_KEYBOARD);
            check(app.page() == pedal::Page::Patches && app.graph().name == "test", "keyboard cancel changed patch");
            click(app, 320, 364); keyboard_key(app, "a"); keyboard_key(app, LV_SYMBOL_OK);
            check(app.page() == pedal::Page::Patch && app.graph().name == "a", "clean patch creation required an unnecessary replacement guard");
            click(app, 600, 36); click(app, 700, 264); click(app, 650, 364);
            check(app.graph().name == "test" && !app.graph().dirty, "saved session did not survive new patch");
            check(!app.pending(), "new editor requests did not resolve");

            // Long names, clamped edge movement and keypad traversal.
            auto names = pedal::demo_graph(); names.name = "A very long patch name that must leave navigation visible";
            names.modules[2].name = "An exceptionally long effect module display name";
            engine.submit(pedal::LoadGraph{names}); step(app, 256); click(app, 276, 36);
            capture(app, output / "23-long-names.bmp");
            drag(app, 232, 244, 30, 90);
            check(pedal::find_module(app.graph(), 3)->x == pedal::module_min_x && pedal::find_module(app.graph(), 3)->y == pedal::module_min_y, "edge drag did not clamp and snap");
            check(app.page() == pedal::Page::Patch && !app.pending(), "edge drag opened parameters or left pending work");
            click(app, 276, 36);
            auto* focus = lv_group_get_focused(group);
            keypad_press(app, LV_KEY_NEXT); check(lv_group_get_focused(group) != focus, "Tab did not traverse controls");
            keypad_press(app, LV_KEY_ENTER); check(app.page() == pedal::Page::Browser, "keyboard Enter did not activate Add");
            click(app, 68, 36); click(app, 600, 36); click(app, 320, 364);
            keypad_press(app, 'p'); keypad_press(app, 'c');
            check(std::string(lv_textarea_get_text(widget(app.screen(), &lv_textarea_class))) == "pc", "physical keyboard did not enter name");
            keypad_press(app, LV_KEY_ENTER);
            check(app.page() == pedal::Page::ConfirmReplace, "physical Enter did not request new patch");
            click(app, 130, 364); keyboard_key(app, LV_SYMBOL_KEYBOARD);
            check(app.graph().name == names.name, "keyboard cancellation changed current graph");
            click(app, 505, 364);
            check(std::string(lv_textarea_get_text(widget(app.screen(), &lv_textarea_class))) == names.name,
                "opening rename silently truncated a backend name");
            keyboard_key(app, LV_SYMBOL_KEYBOARD);
            // Module-specific pages share authoritative values with the generic fallback.
            click(app, 68, 36);
            engine.submit(pedal::LoadGraph{pedal::demo_graph()}); step(app, 256);
            click(app, 396, 244);
            capture(app, output / "24-clouds-texture.bmp");
            drag(app, 150, 354, 325, 354);
            check(pedal::find_module(app.graph(), 4)->parameters[4].value > 70, "Clouds Texture density did not update");
            drag(app, 575, 354, 740, 354);
            check(pedal::find_module(app.graph(), 4)->parameters[5].value > 12, "Clouds Texture pitch did not update");
            click(app, 476, 108); click(app, 468, 268);
            check(pedal::find_module(app.graph(), 4)->parameters[3].value == 1, "Clouds Mix / hold freeze failed");
            capture(app, output / "25-clouds-mix-hold.bmp");
            drag(app, 150, 262, 325, 262);
            check(pedal::find_module(app.graph(), 4)->parameters[2].value > 70, "Clouds Mix / hold mix failed");
            click(app, 588, 108);
            check(lv_obj_has_state(widget(app.screen(), &lv_switch_class), LV_STATE_CHECKED), "generic fallback lost custom Freeze value");
            click(app, 584, 108); click(app, 476, 108);
            check(lv_obj_has_state(widget(app.screen(), &lv_switch_class), LV_STATE_CHECKED), "custom layout lost generic Freeze value");
            click(app, 68, 36); click(app, 564, 244);
            check(widget(app.screen(), &lv_dropdown_class) == nullptr, "Delay did not select custom layout");
            click(app, 400, 196);
            check(pedal::find_module(app.graph(), 5)->parameters[3].value == 1, "Delay Tape button failed");
            engine.reject_next(); click(app, 656, 196);
            check(pedal::find_module(app.graph(), 5)->parameters[3].value == 1 && !app.pending(), "rejected custom mode left optimistic state");
            click(app, 656, 196);
            check(pedal::find_module(app.graph(), 5)->parameters[3].value == 2, "Delay Ping pong button failed");
            drag(app, 150, 318, 325, 318);
            check(pedal::find_module(app.graph(), 5)->parameters[0].value > 800, "custom logarithmic delay time failed");
            drag(app, 575, 296, 740, 296);
            check(pedal::find_module(app.graph(), 5)->parameters[1].value > 70, "compact Delay feedback failed");
            drag(app, 575, 380, 740, 380);
            check(pedal::find_module(app.graph(), 5)->parameters[2].value > 70, "compact Delay mix failed");
            capture(app, output / "26-delay-character.bmp");
            click(app, 588, 108);
            check(lv_dropdown_get_selected(widget(app.screen(), &lv_dropdown_class)) == 2, "generic Delay mode differs from custom mode");
            click(app, 68, 36);

            auto reordered = pedal::demo_graph();
            reordered.modules[3].name = "Texture voice";
            std::reverse(reordered.modules[3].parameters.begin(), reordered.modules[3].parameters.end());
            engine.submit(pedal::LoadGraph{reordered}); step(app, 256); click(app, 396, 244);
            check(widget(app.screen(), &lv_switch_class) == nullptr, "display name or parameter order changed custom layout selection");
            click(app, 476, 108); click(app, 468, 268);
            const auto* reordered_clouds = pedal::find_module(app.graph(), 4);
            const auto freeze = std::find_if(reordered_clouds->parameters.begin(), reordered_clouds->parameters.end(), [](const pedal::Parameter& p) { return p.symbol == "freeze"; });
            check(freeze != reordered_clouds->parameters.end() && freeze->value == 1, "custom controls depended on parameter index");
            click(app, 68, 36);

            // Changed metadata uses the generic view, even for a recognized URI.
            auto extended = pedal::demo_graph();
            extended.dirty = true;
            extended.modules[3].parameters.push_back({"extra", "Extra", "%", pedal::ParameterKind::Continuous, 0, 100, 50});
            engine.submit(pedal::LoadGraph{extended}); step(app, 256); click(app, 396, 244);
            check(widget(app.screen(), &lv_switch_class) != nullptr, "extended schema hid parameters behind custom layout");
            click(app, 496, 108);
            drag(app, 150, 354, 325, 354);
            check(pedal::find_module(app.graph(), 4)->parameters.back().value > 70, "extended schema extra parameter inaccessible");
            click(app, 68, 36);

            // The preset uses the same replacement guard and graph commands as session restore.
            click(app, 600, 36); click(app, 690, 364);
            check(app.page() == pedal::Page::ConfirmReplace, "parallel demo did not guard unsaved edits");
            click(app, 130, 364); check(app.graph().modules[3].parameters.size() == 7, "cancelled demo lost current graph");
            click(app, 690, 364); engine.reject_next(); click(app, 650, 364);
            check(app.graph().modules[3].parameters.size() == 7 && !app.pending(), "rejected demo replaced current graph");
            click(app, 650, 364);
            check(app.page() == pedal::Page::Patch && app.graph().name == "Parallel texture" && app.graph().connections.size() == 10, "parallel starter did not load");
            capture(app, output / "27-parallel-patch.bmp");
            // Delete one side of the split; the other side and both merge cables survive.
            click(app, 338, 198); click(app, 500, 456);
            check(app.graph().connections.size() == 9, "parallel branch cable could not be removed");
            check(std::find(app.graph().connections.begin(), app.graph().connections.end(), pedal::Connection{3, 5}) != app.graph().connections.end(), "removing one branch removed the other");
            drag(app, 288, 228, 408, 120);
            check(app.graph().connections.size() == 10 && pedal::validate_graph(app.graph()).empty(), "parallel branch could not be reconnected");

            // Reference metadata: a square with two input jacks and two output jacks.
            const auto* fold = pedal::definition("http://polyeffects.com/lv2/polywarps#fold");
            check(fold != nullptr, "reference Warps entry missing");
            auto multi = pedal::demo_graph(); multi.name = "Multi-port";
            multi.modules.push_back({6, fold->uri, fold->name, 352, 264, false, false, fold->parameters, fold->ports});
            engine.submit(pedal::LoadGraph{multi}); step(app, 256);
            const auto has = [&](const pedal::Connection& c) {
                return std::find(app.graph().connections.begin(), app.graph().connections.end(), c) != app.graph().connections.end();
            };
            drag(app, 280, 228, 352, 408);
            check(has({3, 6, "out", "modulator"}), "cable dropped on the second input jack reached another port");
            drag(app, 448, 408, 664, 228);
            check(has({6, 2, "aux", "in"}), "cable dragged from the second output jack left another port");
            drag(app, 280, 228, 352, 360);
            check(has({3, 6, "out", "carrier"}) && app.graph().connections.size() == 11, "first input jack could not be patched separately");
            // Left output through one effect, right output through another.
            click(app, 294, 276); click(app, 500, 456);
            check(!has({3, 4, "out_r", "in_r"}), "right-lane cable could not be removed on its own");
            drag(app, 280, 276, 352, 408);
            check(has({3, 4}) && has({3, 6, "out_r", "modulator"}), "left and right outputs could not feed different effects");
            capture(app, output / "28-multi-port.bmp");

            // Continuous controls reach the engine while the finger is still down.
            click(app, 232, 252); check(app.page() == pedal::Page::Parameters, "Drive parameters did not open");
            pointer = {150, 218}; pressed = true; step(app, 64);
            pointer = {325, 218}; step(app, 320);
            check(pedal::find_module(engine.snapshot(), 3)->parameters[0].value > 70, "slider waited for release before reaching the engine");
            pointer = {200, 218}; step(app, 320);
            check(pedal::find_module(engine.snapshot(), 3)->parameters[0].value < 50, "newest gesture value did not replace the older one");
            pressed = false; step(app);
            check(!app.pending() && pedal::find_module(app.graph(), 3)->parameters[0].value < 50, "released slider left pending or stale state");
            click(app, 68, 36);

            // Add the real granular module from the browser and open its generic pages.
            click(app, 490, 36); click(app, 656, 96); // The browser was left on its second page above.
            capture(app, output / "29-reference-catalogue.bmp");
            click(app, 705, 141);
            const auto mist = app.graph().modules.back();
            check(mist.uri == "http://polyeffects.com/lv2/polyclouds#granular" && app.page() == pedal::Page::Patch, "reference module could not be added from the browser");
            click(app, mist.x + 48, mist.y + 120);
            check(app.page() == pedal::Page::Parameters && widget(app.screen(), &lv_slider_class) != nullptr, "reference module has no usable generic editor");
            capture(app, output / "30-reference-parameters.bmp");
            click(app, 68, 36);
        }
        // Release the headless devices before the global runtime.
        lv_indev_delete(indev);
        lv_indev_delete(keypad);
        lv_display_delete(display);
        lv_deinit();
        std::cout << "Real LVGL pointer workflows and 800x480 captures passed.\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
