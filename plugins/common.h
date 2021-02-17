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

#ifdef TRIAL_VER
#include <time.h>
#endif

#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include "lv2/core/lv2.h"
#include "lv2/log/log.h"
#include "lv2/log/logger.h"
#include "lv2/midi/midi.h"
#include "lv2/options/options.h"
#include "lv2/urid/urid.h"
#include "lv2/time/time.h"
#include "lv2/worker/worker.h"

typedef struct PluginUris
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

} PluginUris;

/**
 * Group of variables needed by all plugins and their
 * UIs.
 */
typedef struct PluginCommon
{
  /** Log feature. */
  LV2_Log_Log *   log;

  /** Map feature. */
  LV2_URID_Map *  map;

  /** Logger convenience API. */
  LV2_Log_Logger  logger;

  /** Worker schedule feature. */
  LV2_Worker_Schedule* schedule;

  /** Atom forge. */
  LV2_Atom_Forge  forge;

  LV2_Options_Option * options;

  /** URIs. */
  PluginUris      uris;

  /** Plugin samplerate. */
  double          samplerate;

#ifdef TRIAL_VER
  clock_t         instantiation_time;
#endif

} PluginCommon;

#ifdef TRIAL_VER
static inline double
get_time_since_instantiation (
  PluginCommon * pl_common)
{
  clock_t end = clock ();
  return
    (double)
    (end - pl_common->instantiation_time) /
    CLOCKS_PER_SEC;
}
#endif

static inline void
map_common_uris (
  LV2_URID_Map* map,
  PluginUris* uris)
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
  MAP (time_beatsPerMinute, LV2_TIME__beatsPerMinute);
  MAP (time_beatUnit, LV2_TIME__beatUnit);
  MAP (time_frame, LV2_TIME__frame);
  MAP (time_speed, LV2_TIME__speed);

#undef MAP
}

/**
 * @param with_worker 1 to add the worker feature.
 *
 * @return Non-zero on fail.
 */
static inline int
plugin_common_instantiate (
  PluginCommon *              self,
  const LV2_Feature * const * features,
  int                         with_worker)
{
#ifdef TRIAL_VER
  self->instantiation_time = clock ();
#endif

#define HAVE_FEATURE(x) \
  (!strcmp(features[i]->URI, x))

  for (int i = 0; features[i]; ++i)
    {
      if (HAVE_FEATURE (LV2_URID__map))
        {
          self->map =
            (LV2_URID_Map*) features[i]->data;
        }
      else if (HAVE_FEATURE (LV2_LOG__log))
        {
          self->log =
            (LV2_Log_Log *) features[i]->data;
        }
      else if (HAVE_FEATURE (LV2_OPTIONS__options))
        {
          self->options =
            (LV2_Options_Option*) features[i]->data;
        }

      if (with_worker)
        {
          if (HAVE_FEATURE (LV2_WORKER__schedule))
            {
              self->schedule =
                (LV2_Worker_Schedule *) features[i]->data;
            }
        }
    }
#undef HAVE_FEATURE

  if (!self->map)
    {
      lv2_log_error (
        &self->logger, "Missing feature urid:map\n");
      return -1;
    }
  else if (with_worker && !self->schedule)
    {
      lv2_log_error (
        &self->logger,
        "Missing feature work:schedule\n");
      return -1;
    }

  return 0;
}

/** Gets the samplerate form a plugin or UI. */
#define GET_SAMPLERATE(_x) \
  ((_x)->common.pl_common.samplerate)

/** Gets the samplerate form a plugin or UI. */
#define SET_SAMPLERATE(_x,rate) \
  ((_x)->common.pl_common.samplerate = rate)

#define PL_URIS(_x) \
  (&(_x)->common.pl_common.uris)

#define URIS(_x) \
  (&(_x)->common.uris)

#define SCHEDULE(_x) \
  ((_x)->common.pl_common.schedule)

#define FORGE(_x) \
  (&(_x)->common.pl_common.forge)

#endif
