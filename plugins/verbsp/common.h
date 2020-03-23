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

#ifndef __Z_VERB_COMMON_H__
#define __Z_VERB_COMMON_H__

#include "../common.h"

typedef struct VerbUris
{

} VerbUris;

typedef enum PortIndex
{
  /** GUI to plugin communication. */
  VERB_CONTROL,
  /** Plugin to UI communication. */
  VERB_NOTIFY,

  VERB_STEREO_IN_L,
  VERB_STEREO_IN_R,
  VERB_PREDELAY,
  VERB_LOW_FREQ_CROSSOVER,
  VERB_DECAY_60_LOW,
  VERB_DECAY_60_MID,
  VERB_HF_DAMPING,
  VERB_EQ1_FREQ,
  VERB_EQ1_LEVEL,
  VERB_EQ2_FREQ,
  VERB_EQ2_LEVEL,
  VERB_WET,
  VERB_LEVEL,

  /** Outputs. */
  VERB_STEREO_OUT_L,
  VERB_STEREO_OUT_R,

  NUM_PORTS,
} PortIndex;

/**
 * Group of variables needed by both the DSP and
 * the UI.
 */
typedef struct VerbCommon
{
  /** Custom URIs. */
  VerbUris        uris;

  PluginCommon    pl_common;
} VerbCommon;

static inline void
map_uris (
  LV2_URID_Map * urid_map,
  VerbCommon *   verb_common)
{
  map_common_uris (
    urid_map, &verb_common->pl_common.uris);

#define MAP(x,uri) \
  verb_common->uris.x = \
    urid_map->map (urid_map->handle, uri)

  /* custom URIs */

#undef MAP
}

#endif
