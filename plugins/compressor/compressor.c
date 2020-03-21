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

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "common.h"

#include "soundpipe.h"

typedef struct Compressor
{
  /** Plugin ports. */
  const LV2_Atom_Sequence* control;
  LV2_Atom_Sequence* notify;
  const float * stereo_in_l;
  const float * stereo_in_r;
  const float * attack;
  const float * release;
  const float * ratio;
  const float * threshold;

  /* outputs */
  float *       stereo_out_l;
  float *       stereo_out_r;

  CompressorCommon common;

  sp_data *     sp;
  sp_compressor * compressor;

} Compressor;

static LV2_Handle
instantiate (
  const LV2_Descriptor*     descriptor,
  double                    rate,
  const char*               bundle_path,
  const LV2_Feature* const* features)
{
  Compressor * self = calloc (1, sizeof (Compressor));

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
      lv2_log_error (
        &self->common.logger, "Missing feature urid:map\n");
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
  Compressor * self = (Compressor *) instance;

  switch ((PortIndex) port)
    {
    case COMPRESSOR_CONTROL:
      self->control =
        (const LV2_Atom_Sequence *) data;
      break;
    case COMPRESSOR_NOTIFY:
      self->notify =
        (LV2_Atom_Sequence *) data;
      break;
    case COMPRESSOR_STEREO_IN_L:
      self->stereo_in_l = (const float *) data;
      break;
    case COMPRESSOR_STEREO_IN_R:
      self->stereo_in_r = (const float *) data;
      break;
    case COMPRESSOR_ATTACK:
      self->attack = (const float *) data;
      break;
    case COMPRESSOR_RELEASE:
      self->release = (const float *) data;
      break;
    case COMPRESSOR_RATIO:
      self->ratio = (const float *) data;
      break;
    case COMPRESSOR_THRESHOLD:
      self->threshold = (const float *) data;
      break;
    case COMPRESSOR_STEREO_OUT_L:
      self->stereo_out_l = (float *) data;
      break;
    case COMPRESSOR_STEREO_OUT_R:
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
  Compressor * self = (Compressor*) instance;

  sp_create (&self->sp);
  sp_compressor_create (&self->compressor);
  sp_compressor_init (self->sp, self->compressor);
}

static void
run (
  LV2_Handle instance,
  uint32_t n_samples)
{
  Compressor * self = (Compressor *) instance;

#ifndef RELEASE
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
  *self->compressor->ratio = *self->ratio;
  *self->compressor->thresh = *self->threshold;
  *self->compressor->atk = *self->attack;
  *self->compressor->rel = *self->release;
  for (uint32_t i = 0; i < n_samples; i++)
    {
      float current_in = self->stereo_in_l[i];
      sp_compressor_compute (
        self->sp, self->compressor,
        &current_in, &self->stereo_out_l[i]);
      current_in = self->stereo_in_r[i];
      sp_compressor_compute (
        self->sp, self->compressor,
        &current_in, &self->stereo_out_r[i]);
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
  Compressor * self = (Compressor *) instance;

  sp_destroy (&self->sp);
  sp_compressor_destroy (&self->compressor);
}

static void
cleanup (
  LV2_Handle instance)
{
  Compressor * self = (Compressor *) instance;
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
