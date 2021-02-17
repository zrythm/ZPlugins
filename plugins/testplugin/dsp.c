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

#include PLUGIN_CONFIG

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "../math.h"
#include PLUGIN_COMMON

/**
 * Struct for a 3 byte MIDI event, used for writing notes.
 */
typedef struct MidiNoteEvent
{
  LV2_Atom_Event event;
  uint8_t        msg[3];
} MidiNoteEvent;

typedef struct TestPlugin
{
  /** Plugin ports. */
  const LV2_Atom_Sequence* control;
  LV2_Atom_Sequence* notify;
  const float * major;

  /* outputs */
  LV2_Atom_Sequence * midi_out;

  TestPluginCommon common;

} TestPlugin;

static LV2_Handle
instantiate (
  const LV2_Descriptor*     descriptor,
  double                    rate,
  const char*               bundle_path,
  const LV2_Feature* const* features)
{
  TestPlugin * self = calloc (1, sizeof (TestPlugin));

  SET_SAMPLERATE (self, rate);

  PluginCommon * pl_common = &self->common.pl_common;
  int ret =
    plugin_common_instantiate (
      pl_common, features, 0);
  if (ret)
    goto fail;

  /* map uris */
  map_uris (pl_common->map, &self->common);

  /* init atom forge */
  lv2_atom_forge_init (
    &pl_common->forge, pl_common->map);

  /* init logger */
  lv2_log_logger_init (
    &pl_common->logger, pl_common->map, pl_common->log);

  /* print info */
  if (pl_common->options)
    {
      int j = 0;
      while (true)
        {
          LV2_Options_Option opt;
          memcpy (
            &opt, &pl_common->options[j++],
            sizeof (LV2_Options_Option));
          if (opt.key == 0 && opt.value == 0)
            {
              break;
            }

          if (opt.key ==
                pl_common->map->map (
                  pl_common->map->handle,
                  "https://lv2.zrythm.org/ns/ext/host-info#name"))
            {
              lv2_log_note (
                &pl_common->logger, "Host: %s\n",
                *((const char **) opt.value));
            }
          if (opt.key ==
                pl_common->map->map (
                  pl_common->map->handle,
                  "https://lv2.zrythm.org/ns/ext/host-info#version"))
            {
              lv2_log_note (
                &pl_common->logger, "Host Version: %s\n",
                *((const char **) opt.value));
            }
        }
    }

  return (LV2_Handle) self;

fail:
  free (self);
  return NULL;
}

static void
connect_port (
  LV2_Handle instance,
  uint32_t   port,
  void *     data)
{
  TestPlugin * self = (TestPlugin *) instance;

  switch ((PortIndex) port)
    {
    case TEST_PLUGIN_CONTROL:
      self->control =
        (const LV2_Atom_Sequence *) data;
      break;
    case TEST_PLUGIN_NOTIFY:
      self->notify =
        (LV2_Atom_Sequence *) data;
      break;
    case TEST_PLUGIN_MAJOR:
      self->major = (const float *) data;
      break;
    case TEST_PLUGIN_MIDI_OUT:
      self->midi_out =
        (LV2_Atom_Sequence *) data;
      break;
    default:
      break;
    }
}

static void
activate (
  LV2_Handle instance)
{
  /*TestPlugin * self = (TestPlugin*) instance;*/
}

static void
run (
  LV2_Handle instance,
  uint32_t n_samples)
{
  TestPlugin * self = (TestPlugin *) instance;

#ifdef TRIAL_VER
  if (get_time_since_instantiation (
        &self->common.pl_common) > SECONDS_TO_SILENCE)
    return;
#endif

  const uint32_t out_capacity =
    self->midi_out->atom.size;

  /* write an empty Sequence header to the output */
  lv2_atom_sequence_clear (self->midi_out);
  self->midi_out->atom.type = self->control->atom.type;

  /* read incoming events from host and UI */
  LV2_ATOM_SEQUENCE_FOREACH (
    self->control, ev)
    {
      if (ev->body.type == PL_URIS(self)->midi_MidiEvent)
        {
          const uint8_t * const msg =
            (const uint8_t *) (ev + 1);
          switch (lv2_midi_message_type(msg))
            {
            case LV2_MIDI_MSG_NOTE_ON:
              break;
            case LV2_MIDI_MSG_NOTE_OFF:
              break;
            case LV2_MIDI_MSG_CONTROLLER:
              /* all notes off */
              if (msg[1] == 0x7b)
                {
                }
              break;
            default:
              /* forward all other MIDI events directly */
              lv2_atom_sequence_append_event(
                self->midi_out, out_capacity, ev);
              break;
            }
        }
    }
}

static void
deactivate (
  LV2_Handle instance)
{
  /*Saw * self = (Saw *) instance;*/
}

static void
cleanup (
  LV2_Handle instance)
{
  TestPlugin * self = (TestPlugin *) instance;
  free (self);
}

static const void*
extension_data (
  const char * uri)
{
  return NULL;
}

static const LV2_Descriptor descriptor = {
  PLUGIN_URI,
  instantiate,
  connect_port,
  activate,
  run,
  deactivate,
  cleanup,
  extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor (
  uint32_t index)
{
  switch (index)
    {
    case 0:
      return &descriptor;
    default:
      return NULL;
    }
}
