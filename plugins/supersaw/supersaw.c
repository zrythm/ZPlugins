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

typedef struct MidiKey
{
  /** Pitch 0-127. */
  int           pitch;

  /** Whether currently pressed. */
  int           pressed;

  /** How many samples we are into the phase. */
  size_t        offset;

  sp_adsr *     adsr;
  sp_saturator * saturator;
  sp_blsaw *    blsaws[7];
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
  MidiKey       keys[128];

  /** Current values based on \ref SuperSaw.amount. */
  float         attack;
  float         decay;
  float         sustain;
  float         release;
  int           num_voices;

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
      key->sp->len = 4800 * 6;

      /* create 7 saws */
      for (int j = 0; j < 7; j++)
        {
          sp_create (&key->sp);
          key->sp->len = 4800 * 60;
          sp_blsaw_create (&key->blsaws[j]);
          sp_blsaw_init (key->sp, key->blsaws[j]);
          float freq =
            440.f * powf (2.f, ((float) i - 69.f) / 12.f);
          *key->blsaws[j]->freq = freq;

#define COMPUTE(times) \
  for (int k = 0; k < (times); k++) { \
    sp_blsaw_compute ( \
      key->sp, key->blsaws[j], NULL, &key->sp->out[0]); }

          /* randomize voices a bit */
          int distance = 6000;
          int is_even = (j % 2) == 0;
          int computed = distance * 5;
          if (is_even)
            computed += (j / 2) * distance;
          else
            computed += (j / 2 + 1) * - distance;
          COMPUTE (computed);

          *key->blsaws[j]->freq = freq + rand () % 4;

          *key->blsaws[j]->amp = 1.0f;
        }

      /* create adsr */
      sp_adsr_create (&key->adsr);
      sp_adsr_init (key->sp, key->adsr);

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

      /* compute adsr */
      key->adsr->atk = self->attack;
      key->adsr->dec = self->decay;
      key->adsr->sus = self->sustain;
      key->adsr->rel = self->release;
      SPFLOAT adsr = 0, gate = key->pressed;
      sp_adsr_compute (
        key->sp, key->adsr, &gate, &adsr);
      adsr = adsr < 1.01f ? adsr : 0.f;

      if (adsr > 0.f)
        {
          /* compute as many supersaws as needed */
          for (int j = 0; j < 7; j++)
            {
              sp_blsaw_compute (
                key->sp, key->blsaws[j], NULL,
                &key->sp->out[0]);

              float proximity_to_voice1 =
                ((float) (7 - j) / 7.f);
              proximity_to_voice1 +=
                *self->amount * (1.f - proximity_to_voice1);
              float val =
                key->sp->out[0] * adsr * proximity_to_voice1;

              if (j % 2 == 0)
                {
                  self->stereo_out_l[*offset] += val * 0.8f;
                  self->stereo_out_r[*offset] += val * 0.2f;
                }
              else
                {
                  self->stereo_out_l[*offset] += val * 0.2f;
                  self->stereo_out_r[*offset] += val * 0.8f;
                }
            }
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

  /* adjust amounts */
  self->attack = 0.02f;
  self->decay = 0.04f;
  self->sustain = 0.5f;
  self->release = 0.04f;
  self->num_voices =
    1 + math_round_float_to_int (*self->amount * 6.f);
  printf ("num voices %d\n", self->num_voices);

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
      for (int j = 0; j < 7; j++)
        {
          sp_blsaw_destroy (&key.blsaws[j]);
          sp_destroy (&key.sp);
        }
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
