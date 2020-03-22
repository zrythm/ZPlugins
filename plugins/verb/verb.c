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

typedef struct Verb
{
  /** Plugin ports. */
  const LV2_Atom_Sequence* control;
  LV2_Atom_Sequence* notify;
  const float * stereo_in_l;
  const float * stereo_in_r;
  const float * predelay;
  const float * low_freq_x;
  const float * decay_60_low;
  const float * decay_60_mid;
  const float * hf_damping;
  const float * eq1_freq;
  const float * eq1_level;
  const float * eq2_freq;
  const float * eq2_level;
  const float * wet;
  const float * level;

  /* outputs */
  float *       stereo_out_l;
  float *       stereo_out_r;

  VerbCommon common;

  sp_data *     sp;
  sp_zitarev *  rev;

} Verb;

static LV2_Handle
instantiate (
  const LV2_Descriptor*     descriptor,
  double                    rate,
  const char*               bundle_path,
  const LV2_Feature* const* features)
{
  Verb * self = calloc (1, sizeof (Verb));

  SET_SAMPLERATE (self, rate);

  PluginCommon * pl_common = &self->common.pl_common;
  int ret =
    plugin_common_instantiate (
      pl_common, features, 1);
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
  Verb * self = (Verb *) instance;

#define SET_FLOAT_INPUT(caps,sc) \
  case VERB_##caps: \
    self->sc = (const float *) data; \
    break

  switch ((PortIndex) port)
    {
    case VERB_CONTROL:
      self->control =
        (const LV2_Atom_Sequence *) data;
      break;
    case VERB_NOTIFY:
      self->notify =
        (LV2_Atom_Sequence *) data;
      break;
    SET_FLOAT_INPUT (STEREO_IN_L, stereo_in_l);
    SET_FLOAT_INPUT (STEREO_IN_R, stereo_in_r);
    SET_FLOAT_INPUT (PREDELAY, predelay);
    SET_FLOAT_INPUT (LOW_FREQ_CROSSOVER, low_freq_x);
    SET_FLOAT_INPUT (DECAY_60_LOW, decay_60_low);
    SET_FLOAT_INPUT (DECAY_60_MID, decay_60_mid);
    SET_FLOAT_INPUT (HF_DAMPING, hf_damping);
    SET_FLOAT_INPUT (EQ1_FREQ, eq1_freq);
    SET_FLOAT_INPUT (EQ1_LEVEL, eq1_level);
    SET_FLOAT_INPUT (EQ2_FREQ, eq2_freq);
    SET_FLOAT_INPUT (EQ2_LEVEL, eq2_level);
    SET_FLOAT_INPUT (WET, wet);
    SET_FLOAT_INPUT (LEVEL, level);
    case VERB_STEREO_OUT_L:
      self->stereo_out_l = (float *) data;
      break;
    case VERB_STEREO_OUT_R:
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
  Verb * self = (Verb*) instance;

  sp_create (&self->sp);
  sp_zitarev_create (&self->rev);
  sp_zitarev_init (self->sp, self->rev);
}

static void
run (
  LV2_Handle instance,
  uint32_t n_samples)
{
  Verb * self = (Verb *) instance;

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

  /* compress */
  *self->rev->in_delay = *self->predelay;
  *self->rev->lf_x = *self->low_freq_x;
  *self->rev->rt60_low = *self->decay_60_low;
  *self->rev->rt60_mid = *self->decay_60_mid;
  *self->rev->hf_damping = *self->hf_damping;
  *self->rev->eq1_freq = *self->eq1_freq;
  *self->rev->eq1_level = *self->eq1_level;
  *self->rev->eq2_freq = *self->eq2_freq;
  *self->rev->eq2_level = *self->eq2_level;
  *self->rev->mix = *self->wet;
  *self->rev->level = *self->level;
  for (uint32_t i = 0; i < n_samples; i++)
    {
      float current_in[2] = {
        self->stereo_in_l[i], self->stereo_in_r[i],
      };
      sp_zitarev_compute (
        self->sp, self->rev,
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
  Verb * self = (Verb *) instance;

  sp_destroy (&self->sp);
  sp_zitarev_destroy (&self->rev);
}

static void
cleanup (
  LV2_Handle instance)
{
  Verb * self = (Verb *) instance;
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
