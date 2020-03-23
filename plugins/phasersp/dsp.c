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

typedef struct Phaser
{
  /** Plugin ports. */
  const LV2_Atom_Sequence* control;
  LV2_Atom_Sequence* notify;
  const float * stereo_in_l;
  const float * stereo_in_r;
  const float * max_notch_freq;
  const float * min_notch_freq;
  const float * notch_width;
  const float * notch_freq;
  const float * vibrato_mode;
  const float * depth;
  const float * feedback_gain;
  const float * invert;
  const float * level;
  const float * lfo_bpm;

  /* outputs */
  float *       stereo_out_l;
  float *       stereo_out_r;

  PhaserCommon common;

  sp_data *     sp;
  sp_phaser *  phaser;

} Phaser;

static LV2_Handle
instantiate (
  const LV2_Descriptor*     descriptor,
  double                    rate,
  const char*               bundle_path,
  const LV2_Feature* const* features)
{
  Phaser * self = calloc (1, sizeof (Phaser));

  SET_SAMPLERATE (self, rate);

  PluginCommon * pl_common = &self->common.pl_common;
  int ret =
    plugin_common_instantiate (
      pl_common, features, false);
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
  Phaser * self = (Phaser *) instance;

#define SET_FLOAT_INPUT(caps,sc) \
  case PHASER_##caps: \
    self->sc = (const float *) data; \
    break

  switch ((PortIndex) port)
    {
    case PHASER_CONTROL:
      self->control =
        (const LV2_Atom_Sequence *) data;
      break;
    case PHASER_NOTIFY:
      self->notify =
        (LV2_Atom_Sequence *) data;
      break;
    SET_FLOAT_INPUT (STEREO_IN_L, stereo_in_l);
    SET_FLOAT_INPUT (STEREO_IN_R, stereo_in_r);
    SET_FLOAT_INPUT (MAX_NOTCH_FREQ, max_notch_freq);
    SET_FLOAT_INPUT (MIN_NOTCH_FREQ, min_notch_freq);
    SET_FLOAT_INPUT (NOTCH_WIDTH, notch_width);
    SET_FLOAT_INPUT (NOTCH_FREQ, notch_freq);
    SET_FLOAT_INPUT (VIBRATO_MODE, vibrato_mode);
    SET_FLOAT_INPUT (DEPTH, depth);
    SET_FLOAT_INPUT (FEEDBACK_GAIN, feedback_gain);
    SET_FLOAT_INPUT (INVERT, invert);
    SET_FLOAT_INPUT (LEVEL, level);
    SET_FLOAT_INPUT (LFO_BPM, lfo_bpm);
    case PHASER_STEREO_OUT_L:
      self->stereo_out_l = (float *) data;
      break;
    case PHASER_STEREO_OUT_R:
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
  Phaser * self = (Phaser*) instance;

  sp_create (&self->sp);
  sp_phaser_create (&self->phaser);
  sp_phaser_init (self->sp, self->phaser);
}

static void
run (
  LV2_Handle instance,
  uint32_t n_samples)
{
  Phaser * self = (Phaser *) instance;

#ifdef TRIAL_VER
  if (get_time_since_instantiation (
        &self->common.pl_common) > SECONDS_TO_SILENCE)
    return;
#endif

#if 0
  struct timeval tp;
  gettimeofday(&tp, NULL);
  long int ms = tp.tv_sec * 1000000 + tp.tv_usec;
#endif

  /* read incoming events from host and UI */
  LV2_ATOM_SEQUENCE_FOREACH (
    self->control, ev)
    {
      /* TODO */
    }

  *self->phaser->MaxNotch1Freq = *self->max_notch_freq;
  *self->phaser->MinNotch1Freq = *self->min_notch_freq;
  *self->phaser->Notch_width = *self->notch_width;
  *self->phaser->NotchFreq = *self->notch_freq;
  *self->phaser->VibratoMode = *self->vibrato_mode;
  *self->phaser->depth = *self->depth;
  *self->phaser->feedback_gain = *self->feedback_gain;
  *self->phaser->invert = *self->invert;
  *self->phaser->level = *self->level;
  *self->phaser->lfobpm = *self->lfo_bpm;
  for (uint32_t i = 0; i < n_samples; i++)
    {
      float current_in[] = {
        self->stereo_in_l[i], self->stereo_in_r[i],
      };
      sp_phaser_compute (
        self->sp, self->phaser,
        &current_in[0], &current_in[1],
        &self->stereo_out_l[i], &self->stereo_out_r[i]);
    }

#if 0
  gettimeofday(&tp, NULL);
  ms = (tp.tv_sec * 1000000 + tp.tv_usec) - ms;
  printf("us taken %ld\n", ms);
#endif
}

static void
deactivate (
  LV2_Handle instance)
{
  Phaser * self = (Phaser *) instance;

  sp_destroy (&self->sp);
  sp_phaser_destroy (&self->phaser);
}

static void
cleanup (
  LV2_Handle instance)
{
  Phaser * self = (Phaser *) instance;
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
