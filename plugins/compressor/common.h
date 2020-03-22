/*
 * Copyright (C) 2020 Alexandros Theodotou <alex at zrythm dot org>
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

#ifndef __Z_COMMON_H__
#define __Z_COMMON_H__

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

typedef struct CompressorUris
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

} CompressorUris;

typedef enum PortIndex
{
  /** GUI to plugin communication. */
  COMPRESSOR_CONTROL,
  /** Plugin to UI communication. */
  COMPRESSOR_NOTIFY,

  COMPRESSOR_STEREO_IN_L,
  COMPRESSOR_STEREO_IN_R,
  COMPRESSOR_ATTACK,
  COMPRESSOR_RELEASE,
  COMPRESSOR_RATIO,
  COMPRESSOR_THRESHOLD,

  /** Outputs. */
  COMPRESSOR_STEREO_OUT_L,
  COMPRESSOR_STEREO_OUT_R,

  NUM_PORTS,
} PortIndex;

/**
 * Group of variables needed by both the DSP and
 * the UI.
 */
typedef struct CompressorCommon
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
  CompressorUris  uris;

  /** Plugin samplerate. */
  double          samplerate;

} CompressorCommon;

static inline void
map_uris (
  LV2_URID_Map* map,
  CompressorUris* uris)
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

  /* custom URIs */
  //MAP (saw_freeValues, PLUGIN_URI "#freeValues");
  //MAP (saw_calcValues, PLUGIN_URI "#calcValues");
}

#endif
