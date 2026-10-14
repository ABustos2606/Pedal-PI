// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#ifndef LV_CONF_H
#define LV_CONF_H
#define LV_COLOR_DEPTH 32
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB
#ifdef _WIN32
#define LV_USE_OS LV_OS_WINDOWS
#define LV_USE_WINDOWS 1
#else
#define LV_USE_OS LV_OS_NONE
/* Linux: the framebuffer and evdev touch need only the kernel's headers.
 * DRM/KMS (the Pi's display) and SDL (a desktop window for development)
 * are enabled by CMake when their libraries are found. */
#define LV_USE_LINUX_FBDEV 1
#define LV_USE_EVDEV 1
#ifdef PEDAL_LVGL_DRM
#define LV_USE_LINUX_DRM 1
#endif
#ifdef PEDAL_LVGL_SDL
#define LV_USE_SDL 1
#define LV_SDL_INCLUDE_PATH <SDL2/SDL.h>
#define LV_SDL_RENDER_MODE LV_DISPLAY_RENDER_MODE_DIRECT
#define LV_SDL_BUF_COUNT 1
#define LV_SDL_ACCELERATED 0
#define LV_SDL_FULLSCREEN 0
#define LV_SDL_DIRECT_EXIT 1
#endif
#endif
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_DEFAULT &lv_font_montserrat_18
#define LV_USE_SNAPSHOT 1
#define LV_USE_DEMO_WIDGETS 0
#define LV_BUILD_EXAMPLES 0
#define LV_USE_LOG 0
#define LV_USE_THEME_DEFAULT 0
#define LV_USE_THORVG_INTERNAL 0
#endif
