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

#include "soundpipe.h"

static const int major_scale[12] = {
  1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1 };
static const int minor_scale[12] = {
  1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0 };

/**
 * Struct for a 3 byte MIDI event, used for writing notes.
 */
typedef struct MidiNoteEvent
{
  LV2_Atom_Event event;
  uint8_t        msg[3];
} MidiNoteEvent;

typedef struct Chordz
{
  /** Plugin ports. */
  const LV2_Atom_Sequence* control;
  LV2_Atom_Sequence* notify;
  const float * amount;
  const float * scale;
  const float * major;
  const float * bass;
  const float * first;
  const float * third;
  const float * fifth;
  const float * seventh;
  const float * octave;
  const float * ninth;
  const float * eleventh;
  const float * thirteenth;

  /* outputs */
  LV2_Atom_Sequence * midi_out;

  ChordzCommon common;

} Chordz;

static LV2_Handle
instantiate (
  const LV2_Descriptor*     descriptor,
  double                    rate,
  const char*               bundle_path,
  const LV2_Feature* const* features)
{
  Chordz * self = calloc (1, sizeof (Chordz));

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
  Chordz * self = (Chordz *) instance;

  switch ((PortIndex) port)
    {
    case CHORDZ_CONTROL:
      self->control =
        (const LV2_Atom_Sequence *) data;
      break;
    case CHORDZ_NOTIFY:
      self->notify =
        (LV2_Atom_Sequence *) data;
      break;
    case CHORDZ_SCALE:
      self->scale = (const float *) data;
      break;
    case CHORDZ_MAJOR:
      self->major = (const float *) data;
      break;
    case CHORDZ_BASS:
      self->bass = (const float *) data;
      break;
    case CHORDZ_FIRST:
      self->first = (const float *) data;
      break;
    case CHORDZ_THIRD:
      self->third = (const float *) data;
      break;
    case CHORDZ_FIFTH:
      self->fifth = (const float *) data;
      break;
    case CHORDZ_SEVENTH:
      self->seventh = (const float *) data;
      break;
    case CHORDZ_OCTAVE:
      self->octave = (const float *) data;
      break;
    case CHORDZ_NINTH:
      self->ninth = (const float *) data;
      break;
    case CHORDZ_ELEVENTH:
      self->eleventh = (const float *) data;
      break;
    case CHORDZ_THIRTEENTH:
      self->thirteenth = (const float *) data;
      break;
    case CHORDZ_MIDI_OUT:
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
  /*Chordz * self = (Chordz*) instance;*/
}

static int
get_semitone_diff_from_scale_root (
  int nth_white_key, int major)
{
  int count = -1;
  for (int i = 0; i < 12; i++)
    {
      if (major)
        {
          if (major_scale[i] == 1)
            count++;
        }
      else
        {
          if (minor_scale[i] == 1)
            count++;
        }
      if (count == nth_white_key)
        return i;
    }

  return 0;
}

/**
 */
static int
get_base_note (
  int octave,
  int nth_white_key,
  int scale,
  int major)
{
  return
    octave * 12 + scale +
    get_semitone_diff_from_scale_root (
      nth_white_key, major);
}

static int
get_bass_note (
  int octave,
  int nth_white_key,
  int scale,
  int major)
{
  return
    get_base_note (
      octave, nth_white_key, scale, major) - 12;
}

static int
get_third_note (
  int octave,
  int nth_white_key,
  int scale,
  int major)
{
  int keys_to_add = 0;

  if (major)
    {
      switch (nth_white_key)
        {
          /* minor / diminished */
        case 1:
        case 2:
        case 5:
        case 6:
          keys_to_add = 3;
          break;
          /* major */
        case 0:
        case 3:
        case 4:
          keys_to_add = 4;
          break;
        }
    }
  else
    {
      switch (nth_white_key)
        {
          /* minor / diminished */
        case 0:
        case 1:
        case 3:
        case 4:
          keys_to_add = 3;
          break;
          /* major */
        case 2:
        case 5:
        case 6:
          keys_to_add = 4;
          break;
        }
    }

  return
    get_base_note (
      octave, nth_white_key, scale, major) + keys_to_add;
}

static int
get_fifth_note (
  int octave,
  int nth_white_key,
  int scale,
  int major)
{
  int keys_to_add = 7;

  if (major)
    {
      if (nth_white_key == 6)
        keys_to_add = 6; /* diminished */
    }
  else
    {
      if (nth_white_key == 1)
        keys_to_add = 6; /* diminished */
    }

  return
    get_base_note (
      octave, nth_white_key, scale, major) + keys_to_add;
}

static int
get_seventh_note (
  int octave,
  int nth_white_key,
  int scale,
  int major)
{
  int keys_to_add = 0;

  if (major)
    {
      switch (nth_white_key)
        {
          /* diminished */
        case 6:
          keys_to_add = 9;
          break;
          /* minor */
        case 1:
        case 2:
        case 5:
          keys_to_add = 10;
          break;
          /* major */
        case 0:
        case 3:
        case 4:
          keys_to_add = 11;
          break;
        }
    }
  else
    {
      switch (nth_white_key)
        {
          /* diminished */
        case 1:
          keys_to_add = 9;
          break;
          /* minor */
        case 0:
        case 3:
        case 4:
          keys_to_add = 10;
          break;
          /* major */
        case 2:
        case 5:
        case 6:
          keys_to_add = 11;
          break;
        }
    }

  return
    get_base_note (
      octave, nth_white_key, scale, major) + keys_to_add;
}

static int
get_octave_note (
  int octave,
  int nth_white_key,
  int scale,
  int major)
{
  return get_base_note (
    octave, nth_white_key, scale, major) + 12;
}

static int
get_ninth_note (
  int octave,
  int nth_white_key,
  int scale,
  int major)
{
  int keys_to_add = 1;
  int semitone_diff =
    get_semitone_diff_from_scale_root (
      nth_white_key, major);
  int base_note =
    get_base_note (
      octave, nth_white_key, scale, major);
  if (major)
    {
      do
        {
          if (major_scale[
                (semitone_diff + keys_to_add) % 12] == 1)
            break;
          else
            keys_to_add++;
        } while (1);
    }
  else
    {
      do
        {
          if (minor_scale[
                (semitone_diff + keys_to_add) % 12] == 1)
            break;
          else
            keys_to_add++;

        } while (1);
    }

  return base_note + 12 + keys_to_add;
}

static int
get_eleventh_note (
  int octave,
  int nth_white_key,
  int scale,
  int major)
{
  int keys_to_add = 5;
  int semitone_diff =
    get_semitone_diff_from_scale_root (
      nth_white_key, major);
  int base_note =
    get_base_note (
      octave, nth_white_key, scale, major);
  if (major)
    {
      do
        {
          if (major_scale[
                (semitone_diff + keys_to_add) % 12] == 1)
            break;
          else
            keys_to_add++;
        } while (1);
    }
  else
    {
      do
        {
          if (minor_scale[
                (semitone_diff + keys_to_add) % 12] == 1)
            break;
          else
            keys_to_add++;

        } while (1);
    }

  return base_note + 12 + keys_to_add;
}

static int
get_thirteenth_note (
  int octave,
  int nth_white_key,
  int scale,
  int major)
{
  int keys_to_add = 8;
  int semitone_diff =
    get_semitone_diff_from_scale_root (
      nth_white_key, major);
  int base_note =
    get_base_note (
      octave, nth_white_key, scale, major);
  if (major)
    {
      do
        {
          if (major_scale[
                (semitone_diff + keys_to_add) % 12] == 1)
            break;
          else
            keys_to_add++;
        } while (1);
    }
  else
    {
      do
        {
          if (minor_scale[
                (semitone_diff + keys_to_add) % 12] == 1)
            break;
          else
            keys_to_add++;

        } while (1);
    }

  return base_note + 12 + keys_to_add;
}

/**
 * Returns multiple midi events for each midi event,
 * for each note in the chord.
 *
 * @param src The source event, assuming 3 bytes.
 * @param dest Place to store the resulting events,
 *   3 bytes each. This must be large enough to hold
 *   9 events (3 bytes each).
 * @param dest_size The size in number of MIDI events
 *   (not bytes).
 */
static void
handle_midi_event (
  Chordz *        self,
  const uint8_t * src,
  uint8_t *       dest,
  int *           dest_size)
{
  * dest_size = 0;
  switch (src[0] & 0xf0)
    {
    case 0x80:
    case 0x90:
      break;
    default:
      * dest_size = 1;
      dest[0] = src[0];
      dest[1] = src[1];
      dest[2] = src[2];
      return;
    }

  unsigned char note = src[1];
  unsigned char vel = src[2];
  int nth_white_key = -1;

  int _octave = (int) note / 12;
  switch (note % 12)
    {
    case 0: /* C pressed */
      nth_white_key = 0;
      break;
    case 1: /* C# pressed TODO use black keys for modifiers (adding extra notes?)*/
      break;
    case 2: /* D pressed */
      nth_white_key = 1;
      break;
    case 3: /* D# pressed */
      break;
    case 4: /* E pressed */
      nth_white_key = 2;
      break;
    case 5: /* F pressed */
      nth_white_key = 3;
      break;
    case 6: /* F# pressed */
      break;
    case 7: /* G pressed */
      nth_white_key = 4;
      break;
    case 8: /* G# pressed */
      break;
    case 9: /* A pressed */
      nth_white_key = 5;
      break;
    case 10: /* A# pressed */
      break;
    case 11: /* B pressed */
      nth_white_key = 6;
      break;
    }

  int scale =
    math_round_float_to_int (*self->scale);
  int is_major =
    *self->major > 0.5f;

  if (nth_white_key > -1)
    {
      float min_val = 0.01f;
      if (*self->first > min_val)
        {
          dest[*dest_size * 3 + 0] = src[0];
          dest[*dest_size * 3 + 1] =
            get_base_note (
              _octave, nth_white_key, scale, is_major);
          dest[*dest_size * 3 + 2] =
            math_round_float_to_type (
              (float) vel * *self->first, uint8_t);
          (*dest_size)++;
        }

#define ADD_EVENT(x) \
  if (*self->x > min_val) \
    { \
      dest[*dest_size * 3 + 0] = src[0]; \
      dest[*dest_size * 3 + 1] = \
        get_##x##_note ( \
          _octave, nth_white_key, scale, is_major); \
      dest[*dest_size * 3 + 2] = \
        math_round_float_to_type ( \
          (float) vel * *self->x, uint8_t); \
      (*dest_size)++; \
    }

      ADD_EVENT (bass);
      ADD_EVENT (third);
      ADD_EVENT (fifth);
      ADD_EVENT (seventh);
      ADD_EVENT (octave);
      ADD_EVENT (ninth);
      ADD_EVENT (eleventh);
      ADD_EVENT (thirteenth);
    }
}

static void
send_midi_events (
  Chordz *              self,
  LV2_Midi_Message_Type type,
  const uint32_t        out_capacity,
  uint8_t *             events,
  int                   num_events,
  int                   frames)
{
  for (int i = 0; i < num_events; i++)
    {
      MidiNoteEvent ev;

      ev.event.time.frames = frames;
      ev.event.body.type = type;
      ev.event.body.size = 3;

      ev.msg[0] = events[i * 3 + 0];
      ev.msg[1] = events[i * 3 + 1];
      ev.msg[2] = events[i * 3 + 2];

      /* write event */
      lv2_atom_sequence_append_event (
        self->midi_out, out_capacity, &ev.event);
    }
}

static void
run (
  LV2_Handle instance,
  uint32_t n_samples)
{
  Chordz * self = (Chordz *) instance;

#ifdef TRIAL_VER
  if (get_time_since_instantiation (
        &self->common.pl_common) > SECONDS_TO_SILENCE)
    return;
#endif

#ifndef RELEASE
  struct timeval tp;
  gettimeofday(&tp, NULL);
  long int ms = tp.tv_sec * 1000000 + tp.tv_usec;
#endif

  uint8_t dest[9 * 3];
  int dest_size;
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
              /* TODO note with velocity 0 can be note off */
              if (msg[2] == 0)
                {
                }
              else
                {
                  handle_midi_event (
                    self, msg, dest, &dest_size);
                }
              send_midi_events (
                self, ev->body.type, out_capacity,
                dest, dest_size, ev->time.frames);
              break;
            case LV2_MIDI_MSG_NOTE_OFF:
              handle_midi_event (
                self, msg, dest, &dest_size);
              send_midi_events (
                self, ev->body.type, out_capacity,
                dest, dest_size, ev->time.frames);
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

#ifndef RELEASE
  gettimeofday(&tp, NULL);
  ms = (tp.tv_sec * 1000000 + tp.tv_usec) - ms;
  printf("us taken %ld\n", ms);
#endif
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
  Chordz * self = (Chordz *) instance;
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
