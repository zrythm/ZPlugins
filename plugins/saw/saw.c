/*
 * Copyright (C) 2019-2020 Alexandros Theodotou <alex at zrythm dot org>
 *
 * This file is part of ZSaw
 *
 * ZSaw is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ZSaw is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU General Affero Public License
 * along with ZSaw.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

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

  /** Standard frequency for this key. */
  float         base_freq;

  /** Velocity. */
  int           vel;

  sp_adsr *     adsr;
  sp_blsaw *    blsaws[7];

  /** Last signal known. */
  float         last_adsr;
} MidiKey;

/**
 * Worker data.
 */
typedef struct SawValues
{
  float      attack;
  float      decay;
  float      sustain;
  float      release;

  float      saturator_drive;
  float      saturator_dcoffset;
  float      distortion_shape1;
  float      distortion_shape2;
  float      reverb_mix;
  float      keyfreqs[128][7];
} SawValues;

/**
 * Atom message.
 */
typedef struct SawValuesMessage
{
  LV2_Atom    atom;
  SawValues * values;
} SawValuesMessage;

typedef struct Saw
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

  /** Current values based on \ref Saw.amount. */
  /*float         attack;*/
  /*float         decay;*/
  /*float         sustain;*/
  /*float         release;*/
  /*int           num_voices;*/

  sp_saturator * saturator;
  sp_dist *     distortion;
  sp_zitarev *  reverb;
  sp_compressor * compressor;
  sp_data *     sp;

  SawCommon common;

  SawValues *   values;

  /* cache */
  float         last_amount;
} Saw;

/**
 * To be called by the worker function.
 */
static SawValues *
calc_values (
  Saw *  self)
{
  lv2_log_note (
    &self->common.logger, "calculating values\n");
  SawValues * values = calloc (1, sizeof (SawValues));

  values->attack = 0.02f;
  values->decay = 0.04f + *self->amount * 0.4f;
  values->sustain = 0.5f;
  values->release = 0.04f + *self->amount * 0.4f;
  values->saturator_drive = 0.01f + *self->amount * 0.3f;
  values->saturator_dcoffset = 0.01f + *self->amount * 0.3f;
  values->distortion_shape1 = 0.01f + *self->amount * 0.2f;
  values->distortion_shape2 = 0.01f + *self->amount * 0.2f;
  values->reverb_mix = 0.01f + *self->amount * 0.5f;

  float freq_delta =
    (*self->amount + 0.4f * (1.f - *self->amount)) *
    2.4f;

  for (int i = 0; i < 128; i++)
    {
      for (int j = 0; j < 7; j++)
        {
          /* voice spread */
          int is_even = (j % 2) == 0;
          float freq_apart;
          if (is_even)
            {
              freq_apart = (float) (j / 2) * freq_delta;
            }
          else
            {
              freq_apart = (float) (j / 2 + 1) * - freq_delta;
            }
          values->keyfreqs[i][j] =
            self->keys[i].base_freq +
            math_round_float_to_int (freq_apart);
        }
    }

  return values;
}

/**
 * Cleanup function after work is finished.
 */
static void
free_values (
  Saw *  self,
  SawValues * values)
{
  lv2_log_note (
    &self->common.logger, "freeing values\n");
  free (values);
}

/**
 * Sets the values from the SawValues struct on the
 * actual plugin.
 */
static void
set_values (
  Saw *       self,
  SawValues * values)
{
  lv2_log_note (
    &self->common.logger, "setting values\n");
  self->saturator->drive = values->saturator_drive;
  self->saturator->dcoffset = values->saturator_dcoffset;
  self->distortion->shape1 = values->distortion_shape1;
  self->distortion->shape2 = values->distortion_shape2;
  *self->reverb->mix = values->reverb_mix;

  for (int i = 0; i < 128; i++)
    {
      MidiKey * key = &self->keys[i];

      /* adsr */
      key->adsr->atk = values->attack;
      key->adsr->dec = values->decay;
      key->adsr->sus = values->sustain;
      key->adsr->rel = values->release;

      for (int j = 0; j < 7; j++)
        {
          /* spread voices */
          *key->blsaws[j]->freq = values->keyfreqs[i][j];
        }
    }

}

static LV2_Worker_Status
work (
  LV2_Handle                  instance,
  LV2_Worker_Respond_Function respond,
  LV2_Worker_Respond_Handle   handle,
  uint32_t                    size,
  const void *                data)
{
  Saw * self = (Saw *) instance;

  lv2_log_note (
    &self->common.logger, "working\n");
  const LV2_Atom * atom =
    (const LV2_Atom *) data;
  if (atom->type == self->common.uris.saw_freeValues)
    {
      /* free old values */
      const SawValuesMessage * msg =
        (const SawValuesMessage *) data;
      free_values (self, msg->values);
    }
  else if (atom->type == self->common.uris.saw_calcValues)
    {
      /* recalc values */
      SawValues * values = calc_values (self);
      respond (handle, sizeof (values), &values);
    }

  return LV2_WORKER_SUCCESS;
}

static LV2_Worker_Status
work_response (
  LV2_Handle  instance,
  uint32_t    size,
  const void* data)
{
  Saw * self = (Saw *) instance;

  /* install the new values */
  SawValues * values = * (SawValues * const *) data;
  set_values (self, values);

  /* send a message to the worker to free the
   * values */
  SawValuesMessage msg = {
    { sizeof (SawValues *),
      self->common.uris.saw_freeValues },
    values };

  self->common.schedule->schedule_work (
    self->common.schedule->handle,
    sizeof (msg), &msg);
  lv2_log_note (
    &self->common.logger, "inside work response\n");

  return LV2_WORKER_SUCCESS;
}

static LV2_Handle
instantiate (
  const LV2_Descriptor*     descriptor,
  double                    rate,
  const char*               bundle_path,
  const LV2_Feature* const* features)
{
  Saw * self = calloc (1, sizeof (Saw));

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
      else if (HAVE_FEATURE (LV2_WORKER__schedule))
        {
          self->common.schedule =
            (LV2_Worker_Schedule *) features[i]->data;
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
      lv2_log_error (
        &self->common.logger, "Missing feature urid:map\n");
      goto fail;
    }
  else if (!self->common.schedule)
    {
      lv2_log_error (
        &self->common.logger,
        "Missing feature work:schedule\n");
      goto fail;
    }

  /* map uris */
  map_uris (self->common.map, &self->common.uris);

  /* init atom forge */
  lv2_atom_forge_init (
    &self->common.forge, self->common.map);

  /* init logger */
  lv2_log_logger_init (
    &self->common.logger, self->common.map, self->common.log);

  /* create synth */
  srand (time (NULL));
  for (int i = 0; i < 128; i++)
    {
      MidiKey * key = &self->keys[i];

      sp_create (&self->sp);
      self->sp->len = 1;

      key->base_freq =
        440.f * powf (2.f, ((float) i - 69.f) / 12.f);

      /* create 7 saws */
      for (int j = 0; j < 7; j++)
        {
          sp_blsaw_create (&key->blsaws[j]);
          sp_blsaw_init (self->sp, key->blsaws[j]);
          *key->blsaws[j]->amp = 1.0f;

#define COMPUTE(times) \
  for (int k = 0; k < (times); k++) { \
    sp_blsaw_compute ( \
      self->sp, key->blsaws[j], NULL, &self->sp->out[0]); \
  }

          /* randomize voices a bit */
          int distance = 6000;
          int is_even = (j % 2) == 0;
          int computed = distance * 5;
          if (is_even)
            {
              computed += (j / 2) * distance;
            }
          else
            {
              computed += (j / 2 + 1) * - distance;
            }
          COMPUTE (computed);
        }

      /* create adsr */
      sp_adsr_create (&key->adsr);
      sp_adsr_init (self->sp, key->adsr);
    }

  /* create compressor */
  sp_compressor_create (&self->compressor);
  sp_compressor_init (self->sp, self->compressor);
  *self->compressor->ratio = 2;
  *self->compressor->thresh = -8;
  *self->compressor->atk = 0.1f;
  *self->compressor->rel = 0.2f;

  /* create saturator */
  sp_saturator_create (&self->saturator);
  sp_saturator_init (self->sp, self->saturator);

  /* create distortion */
  sp_dist_create (&self->distortion);
  sp_dist_init (self->sp, self->distortion);

  /* create reverb */
  sp_zitarev_create (&self->reverb);
  sp_zitarev_init (self->sp, self->reverb);
  *self->reverb->level = 0.f;

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
  Saw * self = (Saw*) instance;

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
  Saw * self = (Saw*) instance;

  /* load the default values */
  SawValues * values = calc_values (self);
  set_values (self, values);
  free_values (self, values);
}

/**
 * Processes 1 sample.
 */
static void
process (
  Saw * self,
  uint32_t * offset)
{
  float * current_l = &self->stereo_out_l[*offset];
  float * current_r = &self->stereo_out_r[*offset];
  *current_l = 0.f;
  *current_r = 0.f;

  for (int i = 0; i < 128; i++)
    {
      MidiKey * key = &self->keys[i];

      if (key->last_adsr < 0.0001f &&
          !key->pressed)
        continue;

      if (*offset == 0)
        printf ("computing for key %d\n", i);
      /* compute adsr */
      SPFLOAT adsr = 0, gate = key->pressed;
      sp_adsr_compute (
        self->sp, key->adsr, &gate, &adsr);
      adsr = adsr < 1.001f ? adsr : 0.0f;

      float normalized_vel = ((float) key->vel) / 127.f;

      if (adsr > 0.0f)
        {
          /* compute as many supersaws as needed */
          for (int j = 0; j < 7; j++)
            {
              sp_blsaw_compute (
                self->sp, key->blsaws[j], NULL,
                &self->sp->out[0]);

              float proximity_to_voice1 =
                ((float) (7 - j) / 7.f);
              proximity_to_voice1 +=
                *self->amount * (1.f - proximity_to_voice1);
              float val =
                self->sp->out[0] * adsr * proximity_to_voice1;

              /* multiply by velocity */
              val *= normalized_vel;

              if (j % 2 == 0)
                {
                  *current_l += val * 0.8f;
                  *current_r += val * 0.2f;
                }
              else
                {
                  *current_l += val * 0.2f;
                  *current_r += val * 0.8f;
                }
            }

          key->last_adsr = adsr;
        }
    }

#if 0
  /* compress */
  sp_compressor_compute (
    self->sp, self->compressor, current_l, current_l);
  sp_compressor_compute (
    self->sp, self->compressor, current_r, current_r);

  /* saturate - for some reason it makes noise when it's
   * silent */
  if (fabsf (*current_l) > 0.001f ||
      fabsf (*current_r) > 0.001f)
    {
      float saturated = 0;
      sp_saturator_compute (
        self->sp, self->saturator, current_l,
        &saturated);
      *current_l += saturated;
      sp_saturator_compute (
        self->sp, self->saturator, current_r,
        &saturated);
      *current_r += saturated;
    }

  /* distort */
  float distortion = 0;
  sp_dist_compute (
    self->sp, self->distortion, current_l,
    &distortion);
  *current_l += distortion;
  sp_dist_compute (
    self->sp, self->distortion, current_r,
    &distortion);
  *current_r += distortion;

  /* reverb */
  sp_zitarev_compute (
    self->sp, self->reverb,
    current_l, current_r,
    current_l, current_r);
#endif

  (*offset)++;
}

static void
run (
  LV2_Handle instance,
  uint32_t n_samples)
{
  Saw * self = (Saw *) instance;

  struct timeval tp;
  gettimeofday(&tp, NULL);
  long int ms = tp.tv_sec * 1000000 + tp.tv_usec;

  uint32_t processed = 0;

  if (!math_floats_equal (self->last_amount, *self->amount))
    {
      /* send a message to the worker to calculate new
       * values */
      SawValuesMessage msg = {
        { 0,
          self->common.uris.saw_calcValues }
      };

      self->common.schedule->schedule_work (
        self->common.schedule->handle,
        sizeof (msg), &msg);
      lv2_log_note (
        &self->common.logger, "scheduled to recalculate\n");
    }

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
              /*printf ("note on at %ld: %u (vel %u)\n",*/
                /*ev->time.frames, msg[1], msg[2]);*/
              self->keys[msg[1]].pressed = 1;
              self->keys[msg[1]].vel = msg[2];
              break;
            case LV2_MIDI_MSG_NOTE_OFF:
              /*printf ("note off at %ld: %u\n",*/
                /*ev->time.frames, msg[1]);*/
              self->keys[msg[1]].pressed = 0;
              break;
            default:
              /*printf ("unknown MIDI message\n");*/
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

  self->last_amount = *self->amount;

  gettimeofday(&tp, NULL);
  ms = (tp.tv_sec * 1000000 + tp.tv_usec) - ms;
  printf("us taken %ld\n", ms);
}

static void
deactivate (
  LV2_Handle instance)
{
  Saw * self = (Saw *) instance;

  for (int i = 0; i < 128; i++)
    {
      MidiKey * key = &self->keys[i];
      for (int j = 0; j < 7; j++)
        {
          sp_blsaw_destroy (&key->blsaws[j]);
        }
      sp_destroy (&self->sp);
    }
}

static void
cleanup (
  LV2_Handle instance)
{
  Saw * self = (Saw *) instance;
  free (self);
}

static const void*
extension_data (
  const char * uri)
{
  static const LV2_Worker_Interface worker =
    { work, work_response, NULL };
  if (!strcmp(uri, LV2_WORKER__interface))
    {
      return &worker;
    }
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
