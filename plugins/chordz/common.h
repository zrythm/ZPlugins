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

#include "../common.h"

typedef struct ChordzUris
{
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
  /** Custom URIs. */
  ChordzUris      uris;

  PluginCommon    pl_common;
} ChordzCommon;

static inline void
map_uris (
  LV2_URID_Map*  urid_map,
  ChordzCommon * chordz_common)
{
  map_common_uris (
    urid_map, &chordz_common->pl_common.uris);

#define MAP(x,uri) \
  chordz_common->uris.x = \
    urid_map->map (urid_map->handle, uri)

  /* custom URIs */

#undef MAP
}

#endif
