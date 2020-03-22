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

#ifndef __Z_SAW_COMMON_H__
#define __Z_SAW_COMMON_H__

#include PLUGIN_CONFIG

#include <string.h>

#include "../common.h"

typedef struct SawUris
{
  /* custom URIs for communication */
  LV2_URID saw_calcValues;
  LV2_URID saw_freeValues;
} SawUris;

typedef enum PortIndex
{
  /** GUI to plugin communication. */
  SUPERSAW_CONTROL,
  /** Plugin to UI communication. */
  SUPERSAW_NOTIFY,

  /** Amount for reverb + detune + number
   * of saws. */
  SUPERSAW_AMOUNT,

#if 0
  /** Spacing of voices. */
  SUPERSAW_WIDTH,

  /** Voice blend. */
  SUPERSAW_BLEND,

  /** ADSR. */
  SUPERSAW_ATTACK,
  SUPERSAW_DECAY,
  SUPERSAW_SUSTAIN,
  SUPERSAW_RELEASE,

  SUPERSAW_REVERB,
  SUPERSAW_DISTORTION,
#endif

  /** Outputs. */
  SUPERSAW_STEREO_OUT_L,
  SUPERSAW_STEREO_OUT_R,
  NUM_PORTS,
} PortIndex;

/**
 * Group of variables needed by both the DSP and
 * the UI.
 */
typedef struct SawCommon
{
  /** URIs. */
  SawUris         uris;

  PluginCommon    pl_common;
} SawCommon;

static inline void
map_uris (
  LV2_URID_Map*  urid_map,
  SawCommon *    saw_common)
{
  map_common_uris (
    urid_map, &saw_common->pl_common.uris);

#define MAP(x,uri) \
  saw_common->uris.x = \
    urid_map->map (urid_map->handle, uri)

  /* custom URIs */
  MAP (saw_freeValues, PLUGIN_URI "#freeValues");
  MAP (saw_calcValues, PLUGIN_URI "#calcValues");

#undef MAP
}

#endif
