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

#ifndef __Z_PHASER_COMMON_H__
#define __Z_PHASER_COMMON_H__

#include PLUGIN_CONFIG

#include "../common.h"

typedef struct PhaserUris
{
} PhaserUris;

typedef enum PortIndex
{
  /** GUI to plugin communication. */
  PHASER_CONTROL,
  /** Plugin to UI communication. */
  PHASER_NOTIFY,

  PHASER_STEREO_IN_L,
  PHASER_STEREO_IN_R,

  PHASER_MAX_NOTCH_FREQ,
  PHASER_MIN_NOTCH_FREQ,
  PHASER_NOTCH_WIDTH,
  PHASER_NOTCH_FREQ,
  PHASER_VIBRATO_MODE,
  PHASER_DEPTH,
  PHASER_FEEDBACK_GAIN,
  PHASER_INVERT,
  PHASER_LEVEL,
  PHASER_LFO_BPM,

  /** Outputs. */
  PHASER_STEREO_OUT_L,
  PHASER_STEREO_OUT_R,

  NUM_PORTS,
} PortIndex;

/**
 * Group of variables needed by both the DSP and
 * the UI.
 */
typedef struct PhaserCommon
{
  /** URIs. */
  PhaserUris  uris;

  PluginCommon    pl_common;

} PhaserCommon;

static inline void
map_uris (
  LV2_URID_Map*   urid_map,
  PhaserCommon * phaser_common)
{
  map_common_uris (
    urid_map, &phaser_common->pl_common.uris);

#define MAP(x,uri) \
  phaser_common->uris.x = \
    urid_map->map (urid_map->handle, uri)

  /* custom URIs */

#undef MAP
}

#endif
