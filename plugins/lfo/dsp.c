/*
 * Copyright (C) 2019-2020 Alexandros Theodotou <alex at zrythm dot org>
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

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "../math.h"
#include PLUGIN_COMMON
#include "lfo_math.h"

#define math_floats_almost_equal(a,b) \
  (a > b ? \
   (a - b) < 0.0001f : \
   (b - a) < 0.0001f)

#define IS_FREERUN(x) (*x->freerun > 0.001f)
#define IS_STEP_MODE(x) (*x->step_mode > 0.001f)
#define IS_TRIGGERED(x) (*x->trigger > 0.001f)
#define IS_GATED_MODE(x) (*x->gated_mode > 0.001f)
#define IS_GATED(x) (*x->gate > 0.001f)
#define SINE_ON(x) (*x->sine_on > 0.001f)
#define SQUARE_ON(x) (*x->square_on > 0.001f)
#define TRIANGLE_ON(x) (*x->triangle_on > 0.001f)
#define SAW_ON(x) (*x->saw_on > 0.001f)
#define CUSTOM_ON(x) (*x->custom_on > 0.001f)

typedef struct LFO
{
  /** Plugin ports. */
  const LV2_Atom_Sequence* control;
  LV2_Atom_Sequence* notify;
  const float * gate;
  const float * trigger;
  const float * cv_gate;
  const float * cv_trigger;
  const float * gated_mode;
  const float * freq;
  const float * shift;
  const float * range_min;
  const float * range_max;
  const float * step_mode;
  const float * freerun;
  const float * grid_step;
  const float * sync_rate;
  const float * sync_rate_type;
  const float * hinvert;
  const float * vinvert;
  const float * sine_on;
  const float * saw_on;
  const float * square_on;
  const float * triangle_on;
  const float * custom_on;
  const float * nodes[16][3];
  const float * num_nodes;

  /* outputs */
  float *       cv_out;
  float *       sine_out;
  float *       saw_out;
  float *       triangle_out;
  float *       square_out;
  float *       custom_out;
  float *       sample_to_ui;

  /** This is how far we are inside a beat, from 0.0
   * to 1.0. */
  /*float         beat_offset;*/

  LfoCommon    common;

  /* FIXME this can be a local variable */
  LV2_Atom_Forge_Frame notify_frame;

  /** Whether the UI is active or not. */
  int           ui_active;

  /** Temporary variables. */

  /* whether the plugin was freerunning in the
   * last cycle. this is used to detect changes
   * in freerunning/sync. */
  int           was_freerunning;

  /** Frequency during the last run. */
  float         last_freq;
  float         last_sync_rate;
  float         last_sync_rate_type;

  /* These are used to detect changes so we
   * can notify the UI. */
  long          last_period_size;
  double        last_samplerate;

  /** Flag to be used to send messages to the
   * UI. */
  int           first_run_with_ui;

} LFO;

static LV2_Handle
instantiate (
  const LV2_Descriptor*     descriptor,
  double                    rate,
  const char*               bundle_path,
  const LV2_Feature* const* features)
{
  LFO * self = calloc (1, sizeof (LFO));

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
recalc_multipliers (
  LFO * self)
{
  /* no ports connected yet */
  if (!self->freerun)
    return;

  float sync_rate_float =
    sync_rate_to_float (
      *self->sync_rate,
      *self->sync_rate_type);

  /**
   * Effective frequency.
   *
   * This is either the free-running frequency,
   * or the frequency corresponding to the current
   * sync rate.
   */
  float effective_freq =
    get_effective_freq (
      IS_FREERUN (self), *self->freq,
      &self->common.host_pos, sync_rate_float);

  recalc_vars (
    IS_FREERUN (self),
    &self->common.sine_multiplier,
    &self->common.saw_multiplier,
    &self->common.period_size,
    &self->common.current_sample,
    &self->common.host_pos, effective_freq,
    sync_rate_float,
    (float) GET_SAMPLERATE (self));
}

static void
connect_port (
  LV2_Handle instance,
  uint32_t   port,
  void *     data)
{
  LFO * self = (LFO*) instance;

  switch ((PortIndex) port)
    {
    case LFO_CONTROL:
      self->control =
        (const LV2_Atom_Sequence *) data;
      break;
    case LFO_NOTIFY:
      self->notify =
        (LV2_Atom_Sequence *) data;
      break;
    case LFO_CV_GATE:
      self->cv_gate = (const float *) data;
      break;
    case LFO_CV_TRIGGER:
      self->cv_trigger = (const float *) data;
      break;
    case LFO_GATE:
      self->gate = (const float *) data;
      break;
    case LFO_TRIGGER:
      self->trigger = (const float *) data;
      break;
    case LFO_GATED_MODE:
      self->gated_mode = (const float *) data;
      break;
    case LFO_FREQ:
      self->freq = (const float *) data;
      break;
    case LFO_SHIFT:
      self->shift = (const float *) data;
      break;
    case LFO_RANGE_MIN:
      self->range_min = (const float *) data;
      break;
    case LFO_RANGE_MAX:
      self->range_max = (const float *) data;
      break;
    case LFO_STEP_MODE:
      self->step_mode = (const float *) data;
      break;
    case LFO_FREE_RUNNING:
      self->freerun = (const float *) data;
      break;
    case LFO_GRID_STEP:
      self->grid_step = (const float *) data;
      break;
    case LFO_SYNC_RATE:
      self->sync_rate = (const float *) data;
      break;
    case LFO_SYNC_RATE_TYPE:
      self->sync_rate_type = (const float *) data;
      break;
    case LFO_HINVERT:
      self->hinvert = (const float *) data;
      break;
    case LFO_VINVERT:
      self->vinvert = (const float *) data;
      break;
    case LFO_SINE_TOGGLE:
      self->sine_on = (const float *) data;
      break;
    case LFO_SAW_TOGGLE:
      self->saw_on = (const float *) data;
      break;
    case LFO_SQUARE_TOGGLE:
      self->square_on = (const float *) data;
      break;
    case LFO_TRIANGLE_TOGGLE:
      self->triangle_on = (const float *) data;
      break;
    case LFO_CUSTOM_TOGGLE:
      self->custom_on = (const float *) data;
      break;
    case LFO_SINE_OUT:
      self->sine_out = (float *) data;
      break;
    case LFO_SAW_OUT:
      self->saw_out = (float *) data;
      break;
    case LFO_TRIANGLE_OUT:
      self->triangle_out = (float *) data;
      break;
    case LFO_SQUARE_OUT:
      self->square_out = (float *) data;
      break;
    case LFO_CUSTOM_OUT:
      self->custom_out = (float *) data;
      break;
    case LFO_SAMPLE_TO_UI:
      self->sample_to_ui = (float *) data;
      break;
    case LFO_NUM_NODES:
      self->num_nodes = (float *) data;
      break;
    default:
      break;
    }

  if (port >= LFO_NODE_1_POS &&
      port <= LFO_NODE_16_CURVE)
    {
      unsigned int prop =
        (port - LFO_NODE_1_POS) % 3;
      unsigned int node_id =
        (port - LFO_NODE_1_POS) / 3;
      self->nodes[node_id][prop] =
        (const float *) data;
    }
}

static void
send_position_to_ui (
  LFO *  self)
{
  PluginCommon * pl_common = &self->common.pl_common;

  /* forge container object of type time_Position */
  lv2_atom_forge_frame_time (&pl_common->forge, 0);
  LV2_Atom_Forge_Frame frame;
  lv2_atom_forge_object (
    &pl_common->forge, &frame, 0,
    pl_common->uris.time_Position);

  /* append property for bpm */
  lv2_atom_forge_key (
    &pl_common->forge,
    pl_common->uris.time_beatsPerMinute);
  lv2_atom_forge_float (
    &pl_common->forge, self->common.host_pos.bpm);

  /* append property for current sample */
  lv2_atom_forge_key (
    &pl_common->forge,
    pl_common->uris.time_frame);
  lv2_atom_forge_long (
    &pl_common->forge,
    self->common.host_pos.frame);

  /* append speed */
  lv2_atom_forge_key (
    &pl_common->forge,
    pl_common->uris.time_speed);
  lv2_atom_forge_float (
    &pl_common->forge,
    self->common.host_pos.speed);

  /* append beat unit */
  lv2_atom_forge_key (
    &pl_common->forge,
    pl_common->uris.time_beatUnit);
  lv2_atom_forge_int (
    &pl_common->forge,
    self->common.host_pos.beat_unit);

  /* finish object */
  lv2_atom_forge_pop (&pl_common->forge, &frame);
}

static void
send_messages_to_ui (
  LFO * self,
  int    send_position)
{
  PluginCommon * pl_common = &self->common.pl_common;

  /* set up forge to write directly to notify
   * output port */
  const uint32_t notify_capacity =
    self->notify->atom.size;
  lv2_atom_forge_set_buffer (
    &pl_common->forge, (uint8_t*) self->notify,
    notify_capacity);

  /* start a sequence in the notify output port */
  lv2_atom_forge_sequence_head (
    &pl_common->forge, &self->notify_frame, 0);

  /* forge container object of type "ui_state" */
  lv2_atom_forge_frame_time (&pl_common->forge, 0);
  LV2_Atom_Forge_Frame frame;
  lv2_atom_forge_object (
    &pl_common->forge, &frame, 0,
    self->common.uris.ui_state);

  /* append property for current sample */
  lv2_atom_forge_key (
    &pl_common->forge,
    self->common.uris.ui_state_current_sample);
  lv2_atom_forge_long (
    &pl_common->forge,
    self->common.current_sample);

  /* append property for period size */
  lv2_atom_forge_key (
    &pl_common->forge,
    self->common.uris.ui_state_period_size);
  lv2_atom_forge_long (
    &pl_common->forge, self->common.period_size);

  /* append samplerate */
  lv2_atom_forge_key (
    &pl_common->forge,
    self->common.uris.ui_state_samplerate);
  lv2_atom_forge_double (
    &pl_common->forge, GET_SAMPLERATE (self));

  /* append sine multiplier */
  lv2_atom_forge_key (
    &pl_common->forge,
    self->common.uris.ui_state_sine_multiplier);
  lv2_atom_forge_float (
    &pl_common->forge,
    self->common.sine_multiplier);

  /* append saw multiplier */
  lv2_atom_forge_key (
    &pl_common->forge,
    self->common.uris.ui_state_saw_multiplier);
  lv2_atom_forge_float (
    &pl_common->forge,
    self->common.saw_multiplier);

  /* finish object */
  lv2_atom_forge_pop (&pl_common->forge, &frame);

  if (send_position)
    {
      send_position_to_ui (self);
    }
}

static void
activate (
  LV2_Handle instance)
{
  LFO * self = (LFO*) instance;

  self->first_run_with_ui = 1;

  recalc_multipliers (self);
}

static void
run (
  LV2_Handle instance,
  uint32_t n_samples)
{
  LFO * self = (LFO *) instance;
  PluginCommon * pl_common = &self->common.pl_common;

  int xport_changed = 0;

  /* read incoming events from host and UI */
  LV2_ATOM_SEQUENCE_FOREACH (
    self->control, ev)
    {
      if (lv2_atom_forge_is_object_type (
            &pl_common->forge, ev->body.type))
        {
          const LV2_Atom_Object * obj =
            (const LV2_Atom_Object*)&ev->body;
          if (obj->body.otype ==
                pl_common->uris.time_Position)
            {
              update_position_from_atom_obj (
                &self->common, obj);
              xport_changed = 1;
            }
          else if (obj->body.otype ==
                     self->common.uris.ui_on)
            {
              self->ui_active = 1;
              self->first_run_with_ui = 1;
            }
          else if (obj->body.otype ==
                     self->common.uris.ui_off)
            {
              self->ui_active = 0;
            }
        }
    }

  int freq_changed =
    !math_floats_almost_equal(
      self->last_freq, *self->freq);
  int is_freerunning = *self->freerun > 0.0001f;
  int sync_or_freerun_mode_changed =
    self->was_freerunning != is_freerunning;
  int sync_rate_changed =
    !(math_floats_almost_equal (
      self->last_sync_rate, *self->sync_rate) &&
    math_floats_almost_equal (
      self->last_sync_rate_type,
      *self->sync_rate_type));

  /* if freq or transport changed, reset the
   * multipliers */
  if (xport_changed || freq_changed ||
      sync_rate_changed ||
      sync_or_freerun_mode_changed)
    {
#if 0
      fprintf (
        stderr, "xport %d freq %d sync %d\n",
        xport_changed, freq_changed,
        sync_or_freerun_mode_changed);
#endif
      recalc_multipliers (self);
    }

  float max_range =
    MAX (*self->range_max, *self->range_min);
  float min_range =
    MIN (*self->range_max, *self->range_min);
  float range = max_range - min_range;

  float grid_step_divisor =
    (float)
    grid_step_to_divisor (
      (GridStep) *self->grid_step);
  long step_frames =
    (long)
    ((float) self->common.period_size /
     grid_step_divisor);

  /* sort node curves by position */
  NodeIndexElement node_indices[
    (int) *self->num_nodes];
  float nodes[16][3];
  for (int i = 0; i < 16; i++)
    {
      for (int j = 0; j < 3; j++)
        {
          nodes[i][j] = *(self->nodes[i][j]);
        }
    }
  sort_node_indices_by_pos (
    nodes, node_indices,
    (int) *self->num_nodes);

  /* handle control trigger */
  if (IS_TRIGGERED (self))
    {
      self->common.current_sample = 0;
    }

  for (uint32_t i = 0; i < n_samples; i++)
    {
      /* handle cv trigger */
      if (self->cv_trigger[i] > 0.00001f)
        self->common.current_sample = 0;

      /* invert horizontally */
      long shifted_current_sample =
        invert_and_shift_xval (
          self->common.current_sample,
          self->common.period_size,
          *self->hinvert >= 0.01f,
          *self->shift);

      if (IS_STEP_MODE (self))
        {
          /* find closest step and set the current
           * sample to the middle of it */
          shifted_current_sample =
            (shifted_current_sample / step_frames) *
              step_frames +
            step_frames / 2;
        }

      float ratio =
         (float) shifted_current_sample /
         (float) self->common.period_size;

      if (SINE_ON (self))
        {
          /* calculate sine */
          self->sine_out[i] =
            sinf (
              ((float) shifted_current_sample *
                  self->common.sine_multiplier));
        }
      if (SAW_ON (self))
        {
          /* calculate saw */
          self->saw_out[i] =
            (1.f - ratio) * 2.f - 1.f;
        }
      if (TRIANGLE_ON (self))
        {
          if (ratio > 0.4999f)
            {
              self->triangle_out[i] =
                (1.f - ratio) * 4.f - 1.f;
            }
          else
            {
              self->triangle_out[i] =
                ratio * 4.f - 1.f;
            }
        }
      if (SQUARE_ON (self))
        {
          if (ratio > 0.4999f)
            {
              self->square_out[i] = - 1.f;
            }
          else
            {
              self->square_out[i] = 1.f;
            }
        }
      if (CUSTOM_ON (self))
        {
          int prev_idx =
            get_prev_idx (
              node_indices, (int) * self->num_nodes,
              (float) ratio);
          int next_idx =
            get_next_idx (
              node_indices, (int) * self->num_nodes,
              (float) ratio);

          /* calculate custom */
          self->custom_out[i] =
            get_custom_val_at_x (
              *self->nodes[prev_idx][0],
              *self->nodes[prev_idx][1],
              *self->nodes[prev_idx][2],
              next_idx < 0 ? 1.f :
                *self->nodes[next_idx][0],
              next_idx < 0 ?
                *self->nodes[0][1] :
                *self->nodes[next_idx][1],
              next_idx < 0 ?
                *self->nodes[0][2] :
                *self->nodes[next_idx][2],
              shifted_current_sample,
              self->common.period_size);

          /* adjust for -1 to 1 */
          self->custom_out[i] =
            self->custom_out[i] * 2 - 1;
        }

      /* invert vertically */
      if (*self->vinvert >= 0.01f)
        {
#define INVERT(x) \
  self->x##_out[i] = - self->x##_out[i]

          INVERT (sine);
          INVERT (saw);
          INVERT (triangle);
          INVERT (square);
          INVERT (custom);

#undef INVERT
        }

      /* if in gating mode and gate is not active,
       * set all output to zero */
      if (IS_GATED_MODE (self) &&
          !(IS_GATED (self) || self->cv_gate[i] > 0.001f))
        {
          self->sine_out[i] = 0.f;
          self->saw_out[i] = 0.f;
          self->triangle_out[i] = 0.f;
          self->square_out[i] = 0.f;
          self->custom_out[i] = 0.f;
        }

      /* adjust range */
#define ADJUST_RANGE(x) \
  self->x##_out[i] = \
    min_range + \
    ((self->x##_out[i] + 1.f) / 2.f) * range

      ADJUST_RANGE (sine);
      ADJUST_RANGE (saw);
      ADJUST_RANGE (triangle);
      ADJUST_RANGE (square);
      ADJUST_RANGE (custom);

#undef ADJUST_RANGE

      if (is_freerunning ||
          (!is_freerunning &&
           self->common.host_pos.speed >
             0.00001f))
        {
          self->common.current_sample++;
        }
      if (self->common.current_sample ==
            self->common.period_size)
        self->common.current_sample = 0;
    }
#if 0
  fprintf (
    stderr, "current sample %ld, "
    "period size%ld\n",
    self->current_sample, self->period_size);
#endif

  if (self->ui_active &&
      (self->common.period_size !=
         self->last_period_size ||
       !math_doubles_equal (
         GET_SAMPLERATE (self),
         self->last_samplerate) ||
       xport_changed ||
       self->first_run_with_ui))
    {
      /*fprintf (stderr, "sending messages\n");*/
      send_messages_to_ui (self, xport_changed);
      self->first_run_with_ui = 0;
    }

  /* set current sample for UI to pick up */
  *self->sample_to_ui =
    (float) self->common.current_sample;

  /* remember values */
  self->last_freq = *self->freq;
  self->last_sync_rate = *self->sync_rate;
  self->last_sync_rate_type = *self->sync_rate_type;
  self->was_freerunning = is_freerunning;
  self->last_period_size =
    self->common.period_size;
  self->last_samplerate = GET_SAMPLERATE (self);
}

static void
deactivate (
  LV2_Handle instance)
{
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
