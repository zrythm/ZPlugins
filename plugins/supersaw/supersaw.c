/*
 * Copyright (C) 2019-2020 Alexandros Theodotou <alex at zrythm dot org>
 *
 * This file is part of ZSuperSaw
 *
 * ZSuperSaw is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ZSuperSaw is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU General Affero Public License
 * along with ZSuperSaw.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "common.h"

#include "soundpipe.h"

typedef enum MidiKeyPhase
{
  PHASE_OFF,
  PHASE_ATTACK,
  PHASE_DECAY,
  PHASE_SUSTAIN,
  PHASE_RELEASE,
} MidiKeyPhase;

typedef struct MidiKey
{
  /** Pitch 0-127. */
  int           pitch;

  /** Whether currently pressed. */
  int           pressed;

  /** Current phase. */
  MidiKeyPhase  phase;

  /** How many samples we are into the phase. */
  size_t        offset;

  sp_adsr *     adsr;
  sp_blsaw *    blsaw;
  sp_data *     sp;
} MidiKey;

/**
 * One voice per active key.
 */
/*typedef struct Voice*/
/*{*/
/*} Voice;*/

typedef struct SuperSaw
{
  /** Plugin ports. */
  const LV2_Atom_Sequence* control;
  LV2_Atom_Sequence* notify;
  const float * amount;

  /* outputs */
  float *       stereo_out_l;
  float *       stereo_out_r;

  /** Events in the queue. */
  MidiKey       keys[127];

  /** Length of attack in samples. */
  size_t        attack_length;

  /** Length of decay in samples. */
  size_t        decay_length;

  /** Sustain value. */
  float         sustain_val;

  /** Voices. */
  /*Voice         voices[14];*/
  /*int           num_voices;*/

  SuperSawCommon common;
} SuperSaw;

static LV2_Handle
instantiate (
  const LV2_Descriptor*     descriptor,
  double                    rate,
  const char*               bundle_path,
  const LV2_Feature* const* features)
{
  SuperSaw * self = calloc (1, sizeof (SuperSaw));

  self->common.samplerate = rate;

#define HAVE_FEATURE(x) \
  (!strcmp(features[i]->URI, x))

  for (int i = 0; features[i]; ++i)
    {
      if (HAVE_FEATURE (LV2_URID__map))
        {
          self->common.map =
            (LV2_URID_Map*) features[i]->data;
        }
      else if (HAVE_FEATURE (LV2_LOG__log))
        {
          self->common.log =
            (LV2_Log_Log *) features[i]->data;
        }
    }
#undef HAVE_FEATURE

  if (!self->common.map)
    {
      fprintf (stderr, "Missing feature urid:map\n");
      return NULL;
    }

  /* map uris */
  map_uris (self->common.map, &self->common.uris);

  /* init atom forge */
  lv2_atom_forge_init (
    &self->common.forge, self->common.map);

  return (LV2_Handle) self;
}

static void
connect_port (
  LV2_Handle instance,
  uint32_t   port,
  void *     data)
{
  SuperSaw * self = (SuperSaw*) instance;

  switch ((PortIndex) port)
    {
    case SUPERSAW_CONTROL:
      self->control =
        (const LV2_Atom_Sequence *) data;
      break;
    case SUPERSAW_NOTIFY:
      self->notify =
        (LV2_Atom_Sequence *) data;
      break;
    case SUPERSAW_AMOUNT:
      self->amount = (const float *) data;
      break;
    case SUPERSAW_STEREO_OUT_L:
      self->stereo_out_l = (float *) data;
      break;
    case SUPERSAW_STEREO_OUT_R:
      self->stereo_out_r = (float *) data;
      break;
    default:
      break;
    }
}

static void
activate (
  LV2_Handle instance)
{
  SuperSaw * self = (SuperSaw*) instance;

  srand (time (NULL));
  for (int i = 0; i < 128; i++)
    {
      MidiKey * key = &self->keys[i];
      sp_create (&key->sp);
      sp_blsaw_create (&key->blsaw);
      sp_blsaw_init (key->sp, key->blsaw);
      sp_adsr_create (&key->adsr);
      sp_adsr_init (key->sp, key->adsr);
      *key->blsaw->freq =
        440.f * powf (2.f, ((float) i - 49.f) / 12.f);
      *key->blsaw->amp = 0.3f;
    }
}

/**
 * Processes 1 sample.
 */
static inline void
process (
  SuperSaw * self,
  uint32_t * offset)
{
  self->stereo_out_l[*offset] = 0.f;
  self->stereo_out_r[*offset] = 0.f;

  for (int i = 0; i < 128; i++)
    {
      MidiKey * key = &self->keys[i];
      /*float val = 0.f;*/
#if 0
      if (key->phase == PHASE_OFF)
        continue;

      switch (key->phase)
        {
        case PHASE_ATTACK:
        case PHASE_DECAY:
        case PHASE_SUSTAIN:
        case PHASE_RELEASE:
          break;
        default:
          break;
        }
#endif

      if (key->pressed)
        {
          key->sp->len = 4800 * 4;
          sp_blsaw_compute (
            key->sp, key->blsaw, NULL, &key->sp->out[0]);
          key->offset =
            (key->offset + 1) % (size_t) self->common.samplerate;

          self->stereo_out_l[*offset] += key->sp->out[0];
          self->stereo_out_r[*offset] += key->sp->out[0];
        }
    }
  (*offset)++;
}

static void
run (
  LV2_Handle instance,
  uint32_t n_samples)
{
  SuperSaw * self = (SuperSaw *) instance;

  uint32_t processed = 0;

  /* read incoming events from host and UI */
  LV2_ATOM_SEQUENCE_FOREACH (
    self->control, ev)
    {
      if (ev->body.type ==
            self->common.uris.midi_MidiEvent)
        {
          const uint8_t * const msg =
            (const uint8_t *) (ev + 1);
          switch (lv2_midi_message_type(msg))
            {
            case LV2_MIDI_MSG_NOTE_ON:
              printf ("note on at %ld: %u (vel %u)\n",
                ev->time.frames, msg[1], msg[2]);
              self->keys[msg[1]].pressed = 1;
              break;
            case LV2_MIDI_MSG_NOTE_OFF:
              printf ("note off at %ld: %u\n",
                ev->time.frames, msg[1]);
              self->keys[msg[1]].pressed = 0;
              break;
            default:
              printf ("unknown MIDI message\n");
              break;
            }
          while (processed < ev->time.frames)
            {
              process (self, &processed);
            }
        }
      if (lv2_atom_forge_is_object_type (
            &self->common.forge, ev->body.type))
        {
          const LV2_Atom_Object * obj =
            (const LV2_Atom_Object*)&ev->body;

          /* TODO */
          (void) obj;
        }
    }

  for (uint32_t i = processed; i < n_samples; i++)
    {
      process (self, &processed);
    }
}

static void
deactivate (
  LV2_Handle instance)
{
  SuperSaw * self = (SuperSaw *) instance;

  for (int i = 0; i < 128; i++)
    {
      MidiKey key = self->keys[i];
      sp_blsaw_destroy (&key.blsaw);
      sp_destroy (&key.sp);
    }
}

static void
cleanup (
  LV2_Handle instance)
{
  free (instance);
}

static const void*
extension_data (
  const char* uri)
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
