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

typedef struct Limiter
{
  /** Plugin ports. */
  const LV2_Atom_Sequence* control;
  LV2_Atom_Sequence* notify;
  const float * stereo_in_l;
  const float * stereo_in_r;
  const float * attack;
  const float * release;
  const float * threshold;

  /* outputs */
  float *       stereo_out_l;
  float *       stereo_out_r;

  LimiterCommon common;

  sp_data *     sp;
  sp_peaklim *  limiter;

} Limiter;

static LV2_Handle
instantiate (
  const LV2_Descriptor*     descriptor,
  double                    rate,
  const char*               bundle_path,
  const LV2_Feature* const* features)
{
  Limiter * self = calloc (1, sizeof (Limiter));

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
  Limiter * self = (Limiter *) instance;

  switch ((PortIndex) port)
    {
    case LIMITER_CONTROL:
      self->control =
        (const LV2_Atom_Sequence *) data;
      break;
    case LIMITER_NOTIFY:
      self->notify =
        (LV2_Atom_Sequence *) data;
      break;
    case LIMITER_STEREO_IN_L:
      self->stereo_in_l = (const float *) data;
      break;
    case LIMITER_STEREO_IN_R:
      self->stereo_in_r = (const float *) data;
      break;
    case LIMITER_ATTACK:
      self->attack = (const float *) data;
      break;
    case LIMITER_RELEASE:
      self->release = (const float *) data;
      break;
    case LIMITER_THRESHOLD:
      self->threshold = (const float *) data;
      break;
    case LIMITER_STEREO_OUT_L:
      self->stereo_out_l = (float *) data;
      break;
    case LIMITER_STEREO_OUT_R:
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
  Limiter * self = (Limiter*) instance;

  sp_create (&self->sp);
  sp_peaklim_create (&self->limiter);
  sp_peaklim_init (self->sp, self->limiter);
}

static void
run (
  LV2_Handle instance,
  uint32_t n_samples)
{
  Limiter * self = (Limiter *) instance;

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
  self->limiter->thresh = *self->threshold;
  self->limiter->atk = *self->attack;
  self->limiter->rel = *self->release;
  for (uint32_t i = 0; i < n_samples; i++)
    {
      float current_in = self->stereo_in_l[i];
      sp_peaklim_compute (
        self->sp, self->limiter,
        &current_in, &self->stereo_out_l[i]);
      current_in = self->stereo_in_r[i];
      sp_peaklim_compute (
        self->sp, self->limiter,
        &current_in, &self->stereo_out_r[i]);
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
  Limiter * self = (Limiter *) instance;

  sp_destroy (&self->sp);
  sp_peaklim_destroy (&self->limiter);
}

static void
cleanup (
  LV2_Handle instance)
{
  Limiter * self = (Limiter *) instance;
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
