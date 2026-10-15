# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# LVGL v9.5.0's Win32 thread_delete uses HANDLE* rather than HANDLE for
# TerminateThread/CloseHandle. Build a corrected copy without editing the
# pinned checkout. The copied C implementation retains LVGL's MIT license.
set(_peepo_osal "${lvgl_SOURCE_DIR}/src/osal/lv_windows.c")
file(READ "${_peepo_osal}" _peepo_osal_source)
foreach(_peepo_expected "TerminateThread(thread, 0)" "CloseHandle(thread);")
  string(FIND "${_peepo_osal_source}" "${_peepo_expected}" _peepo_match)
  if(_peepo_match EQUAL -1)
    message(FATAL_ERROR "LVGL Win32 cleanup patch no longer matches; review the dependency revision")
  endif()
endforeach()
string(REPLACE "TerminateThread(thread, 0)" "TerminateThread(*thread, 0)" _peepo_osal_source "${_peepo_osal_source}")
string(REPLACE "CloseHandle(thread);" "CloseHandle(*thread);" _peepo_osal_source "${_peepo_osal_source}")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/patched-lvgl")
set(_peepo_fixed_osal "${CMAKE_BINARY_DIR}/patched-lvgl/lv_windows.c")
file(WRITE "${_peepo_fixed_osal}" "/* MIT licensed LVGL implementation. Peepo modification: dereference\n * Windows thread handles during cleanup. See licenses/LVGL-MIT.txt. */\n${_peepo_osal_source}")
get_target_property(_peepo_lvgl_sources lvgl SOURCES)
list(REMOVE_ITEM _peepo_lvgl_sources "${_peepo_osal}")
list(APPEND _peepo_lvgl_sources "${_peepo_fixed_osal}")
set_property(TARGET lvgl PROPERTY SOURCES "${_peepo_lvgl_sources}")
target_include_directories(lvgl PRIVATE "${lvgl_SOURCE_DIR}/src/osal")
