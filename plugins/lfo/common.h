/*
 * Copyright (C) 2019-2020 Alexandros Theodotou <alex at zrythm dot org>
 *
 * This file is part of ZPlugins
 *
 * ZPlugins is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ZPlugins is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU General Affero Public License
 * along with ZPlugins.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * \file
 *
 * Common code for both the DSP and the UI.
 */

#ifndef __Z_LFO_COMMON_H__
#define __Z_LFO_COMMON_H__

#include PLUGIN_CONFIG

#include <string.h>

#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include "lv2/core/lv2.h"
#include "lv2/log/log.h"
#include "lv2/urid/urid.h"
#include "lv2/time/time.h"

#include "../common.h"

/** Min, max and default frequency. */
#define MIN_FREQ 0.01f
#define DEF_FREQ 1.f
#define MAX_FREQ 60.f

typedef struct LfoUris
{
  /* custom URIs for communication */

  /** The object URI. */
  LV2_URID ui_state;

  /* object property URIs */
  LV2_URID ui_state_current_sample;
  LV2_URID ui_state_period_size;
  LV2_URID ui_state_samplerate;
  LV2_URID ui_state_saw_multiplier;
  LV2_URID ui_state_sine_multiplier;

  /** Messages for UI on/off. */
  LV2_URID ui_on;
  LV2_URID ui_off;
} LfoUris;

typedef enum PortIndex
{
  /** GUI to plugin communication. */
  LFO_CONTROL,
  /** Plugin to UI communication. */
  LFO_NOTIFY,
  /** Plugin to UI communication of the current
   * sample. */
  LFO_SAMPLE_TO_UI,

  LFO_CV_GATE,
  LFO_CV_TRIGGER,
  LFO_GATE,
  LFO_TRIGGER,
  LFO_GATED_MODE,
  LFO_SYNC_RATE,
  LFO_SYNC_RATE_TYPE,
  LFO_FREQ,
  LFO_SHIFT,
  LFO_RANGE_MIN,
  LFO_RANGE_MAX,
  LFO_STEP_MODE,
  LFO_FREE_RUNNING,
  LFO_GRID_STEP,
  LFO_HINVERT,
  LFO_VINVERT,
  LFO_SINE_TOGGLE,
  LFO_SAW_TOGGLE,
  LFO_SQUARE_TOGGLE,
  LFO_TRIANGLE_TOGGLE,
  LFO_CUSTOM_TOGGLE,
  LFO_NODE_1_POS,
  LFO_NODE_1_VAL,
  LFO_NODE_1_CURVE,
  LFO_NODE_2_POS,
  LFO_NODE_2_VAL,
  LFO_NODE_2_CURVE,
  LFO_NODE_3_POS,
  LFO_NODE_3_VAL,
  LFO_NODE_3_CURVE,
  LFO_NODE_4_POS,
  LFO_NODE_4_VAL,
  LFO_NODE_4_CURVE,
  LFO_NODE_5_POS,
  LFO_NODE_5_VAL,
  LFO_NODE_5_CURVE,
  LFO_NODE_6_POS,
  LFO_NODE_6_VAL,
  LFO_NODE_6_CURVE,
  LFO_NODE_7_POS,
  LFO_NODE_7_VAL,
  LFO_NODE_7_CURVE,
  LFO_NODE_8_POS,
  LFO_NODE_8_VAL,
  LFO_NODE_8_CURVE,
  LFO_NODE_9_POS,
  LFO_NODE_9_VAL,
  LFO_NODE_9_CURVE,
  LFO_NODE_10_POS,
  LFO_NODE_10_VAL,
  LFO_NODE_10_CURVE,
  LFO_NODE_11_POS,
  LFO_NODE_11_VAL,
  LFO_NODE_11_CURVE,
  LFO_NODE_12_POS,
  LFO_NODE_12_VAL,
  LFO_NODE_12_CURVE,
  LFO_NODE_13_POS,
  LFO_NODE_13_VAL,
  LFO_NODE_13_CURVE,
  LFO_NODE_14_POS,
  LFO_NODE_14_VAL,
  LFO_NODE_14_CURVE,
  LFO_NODE_15_POS,
  LFO_NODE_15_VAL,
  LFO_NODE_15_CURVE,
  LFO_NODE_16_POS,
  LFO_NODE_16_VAL,
  LFO_NODE_16_CURVE,
  LFO_NUM_NODES,
  LFO_SINE_OUT,
  LFO_TRIANGLE_OUT,
  LFO_SAW_OUT,
  LFO_SQUARE_OUT,
  LFO_CUSTOM_OUT,
  NUM_LFO_PORTS,
} PortIndex;

typedef enum GridStep
{
  GRID_STEP_FULL,
  GRID_STEP_HALF,
  GRID_STEP_FOURTH,
  GRID_STEP_EIGHTH,
  GRID_STEP_SIXTEENTH,
  GRID_STEP_THIRTY_SECOND,
  NUM_GRID_STEPS,
} GridStep;

typedef enum SyncRate
{
  SYNC_1_128,
  SYNC_1_64,
  SYNC_1_32,
  SYNC_1_16,
  SYNC_1_8,
  SYNC_1_4,
  SYNC_1_2,
  SYNC_1_1,
  SYNC_2_1,
  SYNC_4_1,
  SYNC_8_1,
  SYNC_16_1,
  SYNC_32_1,
  SYNC_64_1,
  SYNC_128_1,
  NUM_SYNC_RATES,
} SyncRate;

typedef enum SyncRateType
{
  SYNC_TYPE_NORMAL,
  SYNC_TYPE_DOTTED,
  SYNC_TYPE_TRIPLET,
  NUM_SYNC_RATE_TYPES,
} SyncRateType;

typedef enum CurveAlgorithm
{
  CURVE_ALGORITHM_EXPONENT,
  CURVE_ALGORITHM_SUPERELLIPSE,
} CurveAlgorithm;

typedef struct HostPosition
{
  float     bpm;

  /** Current global frame. */
  long      frame;

  /** Transport speed (0.0 is stopped, 1.0 is
   * normal playback, -1.0 is reverse playback,
   * etc.). */
  float     speed;

  int       beat_unit;
} HostPosition;

/**
 * Group of variables needed by both the DSP and
 * the UI.
 */
typedef struct LfoCommon
{
  HostPosition   host_pos;

  /** URIs. */
  LfoUris         uris;

  PluginCommon    pl_common;

  /** Size of 1 LFO period in samples. */
  long          period_size;

  /**
   * Current sample index in the period.
   *
   * This should be sent to the UI.
   */
  long          current_sample;

  /**
   * Sine multiplier.
   *
   * This is a pre-calculated variable that is used
   * when calculating the sine value.
   */
  float         sine_multiplier;

  float         saw_multiplier;
} LfoCommon;

typedef struct NodeIndexElement
{
  int   index;
  float pos;
} NodeIndexElement;

static inline void
map_uris (
  LV2_URID_Map*  urid_map,
  LfoCommon *    lfo_common)
{
  map_common_uris (
    urid_map, &lfo_common->pl_common.uris);

#define MAP(x,uri) \
  lfo_common->uris.x = \
    urid_map->map (urid_map->handle, uri)

  /* custom URIs */
  MAP (ui_on, PLUGIN_URI "#ui_on");
  MAP (ui_off, PLUGIN_URI "#ui_off");
  MAP (ui_state, PLUGIN_URI "#ui_state");
  MAP (
    ui_state_current_sample,
    PLUGIN_URI "#ui_state_current_sample");
  MAP (
    ui_state_sine_multiplier,
    PLUGIN_URI "#ui_state_sine_multiplier");
  MAP (
    ui_state_saw_multiplier,
    PLUGIN_URI "#ui_state_saw_multiplier");
  MAP (
    ui_state_period_size,
    PLUGIN_URI "#ui_state_period_size");
  MAP (
    ui_state_samplerate,
    PLUGIN_URI "#ui_state_samplerate");
}

/**
 * Updates the position inside HostPosition with
 * the given time_Position atom object.
 */
static inline void
update_position_from_atom_obj (
  LfoCommon *             lfo_common,
  const LV2_Atom_Object * obj)
{
  PluginUris * uris = &lfo_common->pl_common.uris;
  HostPosition * host_pos = &lfo_common->host_pos;

  /* Received new transport position/speed */
  LV2_Atom *beat = NULL,
           *bpm = NULL,
           *beat_unit = NULL,
           *speed = NULL,
           *frame = NULL;
  lv2_atom_object_get (
    obj, uris->time_barBeat, &beat,
    uris->time_beatUnit, &beat_unit,
    uris->time_beatsPerMinute, &bpm,
    uris->time_frame, &frame,
    uris->time_speed, &speed, NULL);
  if (bpm && bpm->type == uris->atom_Float)
    {
      /* Tempo changed, update BPM */
      host_pos->bpm = ((LV2_Atom_Float*)bpm)->body;
     }
  if (speed && speed->type == uris->atom_Float)
    {
      /* Speed changed, e.g. 0 (stop) to 1 (play) */
      host_pos->speed =
        ((LV2_Atom_Float *) speed)->body;
    }
  if (beat_unit && beat_unit->type == uris->atom_Int)
    {
      host_pos->beat_unit =
        ((LV2_Atom_Int *) beat_unit)->body;
    }
  if (frame && frame->type == uris->atom_Long)
    {
      host_pos->frame =
        ((LV2_Atom_Int *) frame)->body;
    }
  if (beat && beat->type == uris->atom_Float)
    {
      /*const float bar_beats =*/
        /*((LV2_Atom_Float *) beat)->body;*/
      /*self->beat_offset = fmodf (bar_beats, 1.f);*/
    }
}

/**
 * Gets the val of the custom graph at x, with
 * x_size corresponding to the period size.
 */
static inline float
get_custom_val_at_x (
  const float        prev_node_pos,
  const float        prev_node_val,
  const float        prev_node_curve,
  const float        next_node_pos,
  const float        next_node_val,
  const float        next_node_curve,
  float              x,
  float              x_size)
{
  if (next_node_pos - prev_node_pos < 0.00000001f)
    return prev_node_val;

  float xratio = x / x_size;

  float range = next_node_pos - prev_node_pos;

  /* x relative to the start of the previous node */
  float rel_x = xratio - prev_node_pos;

  /* get slope */
  float m =
    (next_node_val - prev_node_val) / range;

  return m * (rel_x) + prev_node_val;
}

#endif
