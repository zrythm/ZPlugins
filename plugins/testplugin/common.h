/*
 * Copyright (C) 2021 Alexandros Theodotou <alex at zrythm dot org>
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

#ifndef __Z_TEST_PLUGIN_COMMON_H__
#define __Z_TEST_PLUGIN_COMMON_H__

#include PLUGIN_CONFIG

#include "../common.h"

typedef struct TestPluginUris
{
} TestPluginUris;

typedef enum PortIndex
{
  /** GUI to plugin communication. */
  TEST_PLUGIN_CONTROL,
  /** Plugin to UI communication. */
  TEST_PLUGIN_NOTIFY,

  TEST_PLUGIN_MAJOR,

  /** Outputs. */
  TEST_PLUGIN_MIDI_OUT,

  NUM_PORTS,
} PortIndex;

/**
 * Group of variables needed by both the DSP and
 * the UI.
 */
typedef struct TestPluginCommon
{
  /** Custom URIs. */
  TestPluginUris      uris;

  PluginCommon    pl_common;
} TestPluginCommon;

static inline void
map_uris (
  LV2_URID_Map*  urid_map,
  TestPluginCommon * chordz_common)
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
