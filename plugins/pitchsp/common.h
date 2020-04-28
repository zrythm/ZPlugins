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

#ifndef __Z_PITCH_COMMON_H__
#define __Z_PITCH_COMMON_H__

#include PLUGIN_CONFIG

#include "../common.h"

typedef struct PitchUris
{
} PitchUris;

typedef enum PortIndex
{
  /** GUI to plugin communication. */
  PITCH_CONTROL,
  /** Plugin to UI communication. */
  PITCH_NOTIFY,

  PITCH_STEREO_IN_L,
  PITCH_STEREO_IN_R,

  PITCH_SHIFT,
  PITCH_WINDOW,
  PITCH_XFADE,

  /** Outputs. */
  PITCH_STEREO_OUT_L,
  PITCH_STEREO_OUT_R,

  NUM_PORTS,
} PortIndex;

/**
 * Group of variables needed by both the DSP and
 * the UI.
 */
typedef struct PitchCommon
{
  /** URIs. */
  PitchUris  uris;

  PluginCommon    pl_common;

} PitchCommon;

static inline void
map_uris (
  LV2_URID_Map*      urid_map,
  PitchCommon * pitch_common)
{
  map_common_uris (
    urid_map, &pitch_common->pl_common.uris);

#define MAP(x,uri) \
  pitch_common->uris.x = \
    urid_map->map (urid_map->handle, uri)

  /* custom URIs */

#undef MAP
}

#endif
