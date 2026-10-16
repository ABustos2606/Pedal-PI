// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
//
// A footswitch as a control source. The interface sets Pressed when the
// stomp chosen by Stomp goes down and up (or flips it on each press when
// Latching is on); this module turns that into a control signal that can be
// cabled to any control input: On value while pressed, Off value otherwise.
#include <lv2/core/lv2.h>
#include <stdint.h>
#include <stdlib.h>

#define FOOTSWITCH_URI "urn:peepo:lv2:footswitch"

enum { PRESSED, STOMP, LATCHING, ON_VALUE, OFF_VALUE, OUT, PORT_COUNT };

typedef struct {
    const float* pressed;
    const float* on_value;
    const float* off_value;
    float* out;
} Footswitch;

static LV2_Handle instantiate(const LV2_Descriptor* descriptor, double rate, const char* path, const LV2_Feature* const* features) {
    (void)descriptor; (void)rate; (void)path; (void)features;
    return (LV2_Handle)calloc(1, sizeof(Footswitch));
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
    Footswitch* self = (Footswitch*)instance;
    switch (port) {
    case PRESSED: self->pressed = (const float*)data; break;
    case ON_VALUE: self->on_value = (const float*)data; break;
    case OFF_VALUE: self->off_value = (const float*)data; break;
    case OUT: self->out = (float*)data; break;
    default: break; // Stomp and Latching are read by the interface.
    }
}

static void run(LV2_Handle instance, uint32_t samples) {
    Footswitch* self = (Footswitch*)instance;
    if (!self->out) return;
    float value = 0.0f;
    if (self->pressed && self->on_value && self->off_value) value = *self->pressed >= 0.5f ? *self->on_value : *self->off_value;
    for (uint32_t i = 0; i < samples; ++i) self->out[i] = value;
}

static void cleanup(LV2_Handle instance) { free(instance); }

static const LV2_Descriptor descriptor = {FOOTSWITCH_URI, instantiate, connect_port, NULL, run, NULL, cleanup, NULL};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) { return index == 0 ? &descriptor : NULL; }
