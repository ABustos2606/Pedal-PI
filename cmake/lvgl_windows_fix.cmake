# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
# LVGL v9.5.0's Win32 thread_delete uses HANDLE* rather than HANDLE for
# TerminateThread/CloseHandle. Build a corrected copy without editing the
# pinned checkout. The copied C implementation retains LVGL's MIT license.
set(_pedal_osal "${lvgl_SOURCE_DIR}/src/osal/lv_windows.c")
file(READ "${_pedal_osal}" _pedal_osal_source)
foreach(_pedal_expected "TerminateThread(thread, 0)" "CloseHandle(thread);")
  string(FIND "${_pedal_osal_source}" "${_pedal_expected}" _pedal_match)
  if(_pedal_match EQUAL -1)
    message(FATAL_ERROR "LVGL Win32 cleanup patch no longer matches; review the dependency revision")
  endif()
endforeach()
string(REPLACE "TerminateThread(thread, 0)" "TerminateThread(*thread, 0)" _pedal_osal_source "${_pedal_osal_source}")
string(REPLACE "CloseHandle(thread);" "CloseHandle(*thread);" _pedal_osal_source "${_pedal_osal_source}")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/patched-lvgl")
set(_pedal_fixed_osal "${CMAKE_BINARY_DIR}/patched-lvgl/lv_windows.c")
file(WRITE "${_pedal_fixed_osal}" "/* MIT licensed LVGL implementation. Pedal PI modification: dereference\n * Windows thread handles during cleanup. See licenses/LVGL-MIT.txt. */\n${_pedal_osal_source}")
get_target_property(_pedal_lvgl_sources lvgl SOURCES)
list(REMOVE_ITEM _pedal_lvgl_sources "${_pedal_osal}")
list(APPEND _pedal_lvgl_sources "${_pedal_fixed_osal}")
set_property(TARGET lvgl PROPERTY SOURCES "${_pedal_lvgl_sources}")
target_include_directories(lvgl PRIVATE "${lvgl_SOURCE_DIR}/src/osal")
