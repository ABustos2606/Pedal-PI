// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#include "ui/application.hpp"
#include "catalogue/catalogue.hpp"
#include "engine/ingen/ingen_engine.hpp"
#include "engine/simulated_engine.hpp"
#include <windows.h>
#include <shellapi.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR command_line, int) {
    bool smoke = command_line && wcscmp(command_line, L"--smoke") == 0;
    // --engine tcp://host:port drives a running Ingen instead of the simulation,
    // for example one started under WSL, which Windows reaches on 127.0.0.1.
    // --catalogue file.json names the modules on offer; by default the
    // exported catalogue.json beside this program, with its overlay. The
    // simulation offers them too, beside its own demo modules.
    std::string address, catalogue_path, overlay_path;
    auto utf8 = [](const wchar_t* wide) {
        std::string text(static_cast<std::size_t>(WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr)), '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide, -1, text.data(), static_cast<int>(text.size()), nullptr, nullptr);
        text.resize(text.empty() ? 0 : text.size() - 1);
        return text;
    };
    int count = 0;
    if (auto* arguments = CommandLineToArgvW(GetCommandLineW(), &count)) {
        for (int i = 1; i < count; ++i) smoke = smoke || wcscmp(arguments[i], L"--smoke") == 0;
        for (int i = 1; i + 1 < count; ++i) {
            if (wcscmp(arguments[i], L"--engine") == 0) address = utf8(arguments[i + 1]);
            if (wcscmp(arguments[i], L"--catalogue") == 0) catalogue_path = utf8(arguments[i + 1]);
        }
        LocalFree(arguments);
    }
    const bool named = !catalogue_path.empty();
    if (!named) {
        wchar_t program[MAX_PATH]{};
        GetModuleFileNameW(nullptr, program, MAX_PATH);
        auto folder = utf8(program);
        folder.resize(folder.find_last_of("\\/") + 1);
        catalogue_path = folder + "catalogue.json";
        overlay_path = folder + "catalogue-overlay.json";
    }
    std::vector<pedal::ModuleDefinition> offered;
    const auto catalogue_issue = pedal::load_catalogue(catalogue_path, overlay_path, offered);
    if (!address.empty()) {
        // Without a readable catalogue the engine's graph is still shown; its
        // modules are placeholders and nothing can be added.
        pedal::replace_catalogue(std::move(offered));
    } else if (catalogue_issue.empty()) {
        // The simulation keeps its demo modules: its sample patches and custom
        // pages are built around them. The exported entries take the place of
        // the built-in reference ones, which describe the same plugins.
        std::vector<pedal::ModuleDefinition> both;
        for (const auto& d : pedal::demo_catalogue()) if (d.category != "Endpoint") both.push_back(d);
        both.insert(both.end(), offered.begin(), offered.end());
        pedal::replace_catalogue(std::move(both));
    }
    // A simulation without the default file simply keeps its built-in list.
    const bool expected = !address.empty() || named || std::filesystem::exists(std::filesystem::path(std::u8string(catalogue_path.begin(), catalogue_path.end())));
    if (!catalogue_issue.empty() && expected && !smoke)
        MessageBoxW(nullptr, std::wstring(catalogue_issue.begin(), catalogue_issue.end()).c_str(), L"Pedal PI - catalogue", MB_OK | MB_ICONWARNING);
    lv_init();
    auto* display = lv_windows_create_display(address.empty() ? L"Pedal PI - UI simulator" : L"Pedal PI - Ingen engine", 800, 480, 100, true, true);
    if (!display) return 1;
    auto* group = lv_group_create(); lv_group_set_default(group);
    lv_windows_acquire_pointer_indev(display);
    lv_indev_set_group(lv_windows_acquire_keypad_indev(display), group);
    std::unique_ptr<pedal::Engine> owned;
    if (address.empty()) owned = std::make_unique<pedal::SimulatedEngine>();
    else {
        owned = std::make_unique<pedal::IngenEngine>(pedal::ingen::Options{address});
        owned->connect();
    }
    auto& engine = *owned;
    lv_lock();
    bool display_alive = true;
    unsigned completed_flushes = 0;
    lv_display_add_event_cb(display, [](lv_event_t* e) {
        *static_cast<bool*>(lv_event_get_user_data(e)) = false;
    }, LV_EVENT_DELETE, &display_alive);
    lv_display_add_event_cb(display, [](lv_event_t* e) {
        ++*static_cast<unsigned*>(lv_event_get_user_data(e));
    }, LV_EVENT_FLUSH_FINISH, &completed_flushes);
    pedal::Application app(engine);
    lv_unlock();
    const auto window = lv_windows_get_display_window_handle(display);
    if (smoke) ShowWindow(window, SW_HIDE);
    const auto started = GetTickCount64();
    auto last = GetTickCount64();
    bool framebuffer_ready = false;
    while (true) {
        const auto now = GetTickCount64();
        lv_lock();
        if (!display_alive || !IsWindow(window)) { lv_unlock(); break; }
        if (!framebuffer_ready) {
            // The Win32 driver provisions its buffer on a deferred timer.
            // An earlier refresh can pause without drawing; the queued full
            // invalidation then prevents another refresh request. Resume it
            // explicitly once the buffer exists, without requiring a click.
            auto* buffer = lv_display_get_buf_active(display);
            if (buffer && buffer->data && buffer->data_size) {
                lv_obj_invalidate(app.screen());
                auto* refresh = lv_display_get_refr_timer(display);
                lv_timer_resume(refresh);
                lv_timer_ready(refresh);
                framebuffer_ready = true;
            }
        }
        app.tick(static_cast<unsigned>(now - last));
        lv_unlock();
        last = now;
        const auto wait = lv_timer_handler();
        Sleep(std::clamp<unsigned>(wait, 1, 10));
        if (smoke && now - started > 1000) {
            lv_lock();
            if (!display_alive) { lv_unlock(); break; }
            auto* buffer = lv_display_get_buf_active(display);
            unsigned dark = 0, cards = 0;
            if (buffer && buffer->data) {
                for (unsigned y = 0; y < buffer->header.h; ++y) {
                    const auto* row = reinterpret_cast<const std::uint32_t*>(buffer->data + y * buffer->header.stride);
                    for (unsigned x = 0; x < buffer->header.w; ++x) {
                        const auto pixel = row[x] & 0xffffff;
                        if (pixel == 0x10171f) ++dark;
                        if (pixel == 0x1a2530) ++cards;
                    }
                }
            }
            const bool rendered = buffer && buffer->header.w == 800 && buffer->header.h == 480
                && completed_flushes > 0 && dark > 100000 && cards > 10000;
            std::ofstream log("windows-render-check.txt");
            log << "resolution=" << lv_display_get_horizontal_resolution(display) << "x" << lv_display_get_vertical_resolution(display)
                << " buffer=" << (buffer != nullptr) << " flushes=" << completed_flushes
                << " background_pixels=" << dark << " card_pixels=" << cards << "\n";
            log.close();
            if (rendered) {
                // Capture the real Win32 display buffer, not an LVGL snapshot.
                std::ofstream bitmap("windows-render-check.bmp", std::ios::binary);
                auto word = [&](std::uint32_t value, unsigned bytes) {
                    for (unsigned i = 0; i < bytes; ++i) bitmap.put(static_cast<char>((value >> (i * 8)) & 255));
                };
                bitmap.put('B'); bitmap.put('M'); word(54 + 800 * 480 * 4, 4); word(0, 4); word(54, 4);
                word(40, 4); word(800, 4); word(480, 4); word(1, 2); word(32, 2);
                word(0, 4); word(800 * 480 * 4, 4); word(2835, 4); word(2835, 4); word(0, 4); word(0, 4);
                for (int y = 479; y >= 0; --y)
                    bitmap.write(reinterpret_cast<const char*>(buffer->data + y * buffer->header.stride), 800 * 4);
            }
            lv_unlock();
            if (!rendered) ExitProcess(2);
            PostMessageW(window, WM_CLOSE, 0, 0);
        }
    }
    // The Windows driver deletes its display (and screen) on window close.
    // Exit the process without touching objects owned by that deleted display.
    ExitProcess(0);
}
