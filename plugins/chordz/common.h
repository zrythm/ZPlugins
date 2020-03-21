/*
 * Copyright (C) 2018, 2020 Alexandros Theodotou <alex at zrythm dot org>
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

#ifndef __Z_CHORDZ_COMMON_H__
#define __Z_CHORDZ_COMMON_H__

#include PLUGIN_CONFIG

#include <string.h>

#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include "lv2/core/lv2.h"
#include "lv2/log/log.h"
#include "lv2/log/logger.h"
#include "lv2/midi/midi.h"
#include "lv2/urid/urid.h"
#include "lv2/time/time.h"
#include "lv2/worker/worker.h"

typedef struct ChordzUris
{
  LV2_URID atom_eventTransfer;
  LV2_URID atom_Blank;
  LV2_URID atom_Object;
  LV2_URID atom_Float;
  LV2_URID atom_Double;
  LV2_URID atom_Int;
  LV2_URID atom_Long;
  LV2_URID log_Entry;
  LV2_URID log_Error;
  LV2_URID log_Note;
  LV2_URID log_Trace;
  LV2_URID log_Warning;
  LV2_URID midi_MidiEvent;
  LV2_URID time_Position;
  LV2_URID time_bar;
  LV2_URID time_barBeat;
  LV2_URID time_beatsPerMinute;
  LV2_URID time_beatUnit;
  LV2_URID time_frame;
  LV2_URID time_speed;

} ChordzUris;

typedef enum PortIndex
{
  /** GUI to plugin communication. */
  CHORDZ_CONTROL,
  /** Plugin to UI communication. */
  CHORDZ_NOTIFY,

  CHORDZ_SCALE,
  CHORDZ_MAJOR,
  CHORDZ_BASS,
  CHORDZ_FIRST,
  CHORDZ_THIRD,
  CHORDZ_FIFTH,
  CHORDZ_SEVENTH,
  CHORDZ_OCTAVE,
  CHORDZ_NINTH,
  CHORDZ_ELEVENTH,
  CHORDZ_THIRTEENTH,

  /** Outputs. */
  CHORDZ_MIDI_OUT,

  NUM_PORTS,
} PortIndex;

typedef enum ChordzScales
{
  SCALE_C,
  SCALE_CS,
  SCALE_D,
  SCALE_DS,
  SCALE_E,
  SCALE_F,
  SCALE_FS,
  SCALE_G,
  SCALE_GS,
  SCALE_A,
  SCALE_AS,
  SCALE_B
} ChordzScales;

/**
 * Group of variables needed by both the DSP and
 * the UI.
 */
typedef struct ChordzCommon
{
  /** Log feature. */
  LV2_Log_Log *   log;

  /** Map feature. */
  LV2_URID_Map *  map;

  /** Logger convenience API. */
  LV2_Log_Logger  logger;

  /** Atom forge. */
  LV2_Atom_Forge  forge;

  /** URIs. */
  ChordzUris         uris;

  /** Plugin samplerate. */
  double          samplerate;

} ChordzCommon;

static inline void
map_uris (
  LV2_URID_Map* map,
  ChordzUris* uris)
{
#define MAP(x,uri) \
  uris->x = map->map (map->handle, uri)

  /* official URIs */
  MAP (atom_Blank, LV2_ATOM__Blank);
  MAP (atom_Object, LV2_ATOM__Object);
  MAP (atom_Float, LV2_ATOM__Float);
  MAP (atom_Double, LV2_ATOM__Double);
  MAP (atom_Int, LV2_ATOM__Int);
  MAP (atom_Long, LV2_ATOM__Long);
  MAP (atom_eventTransfer, LV2_ATOM__eventTransfer);
  MAP (log_Entry, LV2_LOG__Entry);
  MAP (log_Error, LV2_LOG__Error);
  MAP (log_Note, LV2_LOG__Note);
  MAP (log_Trace, LV2_LOG__Trace);
  MAP (log_Warning, LV2_LOG__Warning);
  MAP (midi_MidiEvent, LV2_MIDI__MidiEvent);
  MAP (time_Position, LV2_TIME__Position);
  MAP (time_bar, LV2_TIME__bar);
  MAP (time_barBeat, LV2_TIME__barBeat);
  MAP (
    time_beatsPerMinute, LV2_TIME__beatsPerMinute);
  MAP (time_beatUnit, LV2_TIME__beatUnit);
  MAP (time_frame, LV2_TIME__frame);
  MAP (time_speed, LV2_TIME__speed);

  /* custom URIs */
  //MAP (saw_freeValues, PLUGIN_URI "#freeValues");
  //MAP (saw_calcValues, PLUGIN_URI "#calcValues");
}

#ifndef MAX
# define MAX(x,y) (x > y ? x : y)
#endif

#ifndef MIN
# define MIN(x,y) (x < y ? x : y)
#endif

#ifndef CLAMP
# define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif

/**
 * Rounds a float to a given type.
 */
#define math_round_float_to_type(x,type) \
  ((type) (x + 0.5f - (x < 0.f)))

/**
 * Rounds a float to an int.
 */
#define math_round_float_to_int(x) \
  math_round_float_to_type (x,int)

/**
 * Checks if 2 floats are equal.
 */
#define math_floats_equal(a,b) \
  ((a) > (b) ? \
   (a) - (b) < FLT_EPSILON : \
   (b) - (a) < FLT_EPSILON)

#endif
