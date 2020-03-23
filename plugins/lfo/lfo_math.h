/*
 * Copyright (C) 2019-2020 Alexandros Theodotou <alex at zrythm dot org>
 *
 * This file is part of ZLFO
 *
 * ZLFO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ZLFO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU General Affero Public License
 * along with ZLFO.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * \file
 *
 * Math module.
 */

#ifndef __Z_LFO_MATH_H__
#define __Z_LFO_MATH_H__

#include PLUGIN_CONFIG

#include <float.h>
#include <math.h>

#include "common.h"

static inline float
sync_rate_to_float (
  SyncRate     rate,
  SyncRateType type)
{
  float r = 0.01f;
  switch (rate)
    {
    case SYNC_1_128:
      r = 1.f / 128.f;
      break;
    case SYNC_1_64:
      r = 1.f / 64.f;
      break;
    case SYNC_1_32:
      r = 1.f / 32.f;
      break;
    case SYNC_1_16:
      r = 1.f / 16.f;
      break;
    case SYNC_1_8:
      r = 1.f / 8.f;
      break;
    case SYNC_1_4:
      r = 1.f / 4.f;
      break;
    case SYNC_1_2:
      r = 1.f / 2.f;
      break;
    case SYNC_1_1:
      r = 1.f;
      break;
    case SYNC_2_1:
      r = 2.f;
      break;
    case SYNC_4_1:
      r = 4.f;
      break;
    case SYNC_8_1:
      r = 8.f;
      break;
    case SYNC_16_1:
      r = 16.f;
      break;
    case SYNC_32_1:
      r = 32.f;
      break;
    case SYNC_64_1:
      r = 64.f;
      break;
    case SYNC_128_1:
      r = 128.f;
      break;
    default:
      break;
    }

  switch (type)
    {
    case SYNC_TYPE_NORMAL:
      break;
    case SYNC_TYPE_DOTTED:
      r *= 1.5f;
      break;
    case SYNC_TYPE_TRIPLET:
      r *= (2.f / 3.f);
      break;
    default:
      break;
    }

  return r;
}

/**
 * Returns the number to use for dividing by the
 * grid step.
 *
 * Eg., when grid step is HALF, this returns 2, the
 * bottom half of "1/2".
 */
static inline int
grid_step_to_divisor (
  GridStep step)
{
  int ret = 0;
  switch (step)
    {
    case GRID_STEP_FULL:
      ret = 1;
      break;
    case GRID_STEP_HALF:
      ret = 2;
      break;
    case GRID_STEP_FOURTH:
      ret = 4;
      break;
    case GRID_STEP_EIGHTH:
      ret = 8;
      break;
    case GRID_STEP_SIXTEENTH:
      ret = 16;
      break;
    case GRID_STEP_THIRTY_SECOND:
      ret = 32;
      break;
    default:
      break;
    }

  return ret;
}

/**
 * Gets the y value for a node at the given X coord.
 *
 * See https://stackoverflow.com/questions/17623152/how-map-tween-a-number-based-on-a-dynamic-curve
 * @param x X-coordinate.
 * @param curviness Curviness variable (1.0 is
 *   a straight line, 0.0 is full curved).
 * @param start_higher Start at higher point.
 */
static inline double
get_y_normalized (
  double x,
  double curviness,
  CurveAlgorithm algo,
  int    start_higher,
  int    curve_up)
{
  if (!start_higher)
    x = 1.0 - x;
  if (curve_up)
    x = 1.0 - x;

  double val;
  switch (algo)
    {
    case CURVE_ALGORITHM_EXPONENT:
      val =
        pow (x, curviness);
      break;
    case CURVE_ALGORITHM_SUPERELLIPSE:
      val =
        pow (
          1.0 - pow (x, curviness),
          (1.0 / curviness));
      break;
    }
  if (curve_up)
    {
      val = 1.0 - val;
    }
  return val;

  fprintf (
    stderr, "This line should not be reached");
}

static inline float
get_frames_per_beat (
  float    bpm,
  float    samplerate)
{
  return 60.0f / bpm * samplerate;
}

static inline float
get_effective_freq (
  int            freerunning,
  float          freq,
  HostPosition * host_pos,
  float          sync_rate_float)
{
  /* if beat_unit is 0 that means we don't know the
   * time info yet */
  if (freerunning || host_pos->beat_unit == 0)
    {
      if (!freerunning && host_pos->beat_unit == 0)
        {
          fprintf (
            stderr,
            "Host did not send time info. Beat "
            "unit is unknown.\n");
        }
      return freq;
    }
  else /* synced */
    {
      /* bpm / (60 * BU * sync note) */
      return
        host_pos->bpm /
          (60.f *
           host_pos->beat_unit * sync_rate_float);
    }
}

static inline uint32_t
get_period_size (
  int            freerunning,
  HostPosition * host_pos,
  float          effective_freq,
  float          sync_rate_float,
  float          frames_per_beat,
  float          samplerate)
{
  /* if beat_unit is 0 that means we don't know the
   * time info yet */
  if (freerunning || host_pos->beat_unit == 0)
    {
      if (!freerunning && host_pos->beat_unit == 0)
        {
          fprintf (
            stderr,
            "Host did not send time info. Beat "
            "unit is unknown.\n");
        }
      return
        (uint32_t) (samplerate / effective_freq);
    }
  else /* synced */
    {
      return
        (uint32_t)
        (frames_per_beat * host_pos->beat_unit *
         sync_rate_float);
    }
}

static inline uint32_t
get_current_sample (
  int            freerunning,
  HostPosition * host_pos,
  uint32_t       period_size)
{
  if (freerunning)
    {
      return 0;
    }
  else if (host_pos->beat_unit == 0)
    {
      /* if beat_unit is 0 that means we don't
       * know the time info yet */
      fprintf (
        stderr,
        "Host did not send time info. Beat "
        "unit is unknown.\n");
      return 0;
    }
  else /* synced */
    {
      return host_pos->frame % period_size;
    }
}

static inline int
float_array_contains_nonzero (
  const float * arr,
  size_t        size)
{
  for (size_t i = 0; i < size; i++)
    {
      if (arr[i] > 0.0001f ||
          arr[i] < - 0.0001f)
        return 1;
    }
  return 0;
}

/**
 * @param sine_multipler Position to save the sine
 *   multiplier in.
 * @param saw_multipler Position to save the saw
 *   multiplier in.
 */
static void
recalc_vars (
  int     freerunning,
  float * sine_multiplier,
  float * saw_multiplier,
  long *  period_size,
  long *  current_sample,
  HostPosition * host_pos,
  float   effective_freq,
  float   sync_rate_float,
  float   samplerate)
{
  float frames_per_beat =
    get_frames_per_beat (host_pos->bpm, samplerate);

  /*
   * F = frequency
   * X = samples processed
   * SR = sample rate
   *
   * First, get the radians.
   * ? radians =
   *   (2 * PI) radians per LFO cycle *
   *   F cycles per second *
   *   (1 / SR samples per second) *
   *   X samples
   *
   * Then the LFO value is the sine of
   * (radians % (2 * PI)).
   *
   * This multiplier handles the part known by now
   * and the first part of the calculation should
   * become:
   * ? radians = X samples * sine_multiplier
   */
  *sine_multiplier =
    (effective_freq / samplerate) * 2.f * PI;

  /*
   * F = frequency
   * X = samples processed
   * SR = sample rate
   *
   * First, get the value.
   * ? value =
   *   (1 value per LFO cycle *
   *    F cycles per second *
   *    1 / SR samples per second *
   *    X samples) % 1
   *
   * Then the LFO value is value * 2 - 1 (to make
   * it start from -1 and end at 1).
   *
   * This multiplier handles the part known by now and the
   * first part of the calculation should become:
   * ? value = ((X samples * saw_multiplier) % 1) * 2 - 1
   */
  *saw_multiplier = effective_freq / samplerate;

  *period_size =
    get_period_size (
      freerunning, host_pos, effective_freq,
      sync_rate_float, frames_per_beat, samplerate);
  if (current_sample)
    {
      *current_sample =
        get_current_sample (
          freerunning, host_pos, *period_size);
    }
}

static inline long
invert_and_shift_xval (
  long    base,
  long    max_val,
  int     hinvert,
  float   shift)
{
  long ret = base;

  /* invert horizontally */
  if (hinvert)
    {
      ret = max_val - ret;
      while (ret >= max_val)
        ret -= max_val;
    }

  /* shift */
  if (shift >= 0.5f)
    {
      /* add the samples to shift from 0 to
       * (half the period width) */
      ret +=
        (long)
        /* shift ratio */
        (((shift - 0.5f) * 2.f) *
        /* half a period */
        (max_val / 2.f));

      /* readjust */
      ret = ret % max_val;
    }
  else
    {
      /* subtract the samples to shift between
       * 0 and (half the period width) */
      ret -=
        (long)
        /* shift ratio */
        (((0.5f - shift) * 2.f) *
        /* half a period */
        (max_val / 2.f));

      /* readjust */
      while (ret < 0)
        {
          ret += max_val;
        }
    }

  return ret;
}

/**
 * This will return -1 if the next index is
 * the copy of the first one at the end.
 */
static inline int
get_next_idx (
  NodeIndexElement * elements,
  int                num_nodes,
  float              ratio)
{
  float max_pos = 2.f;
  int max_idx = 0;
  for (int i = 0; i < num_nodes; i++)
    {
      if (elements[i].pos < max_pos &&
          elements[i].pos >= ratio)
        {
          max_pos = elements[i].pos;
          max_idx = elements[i].index;
        }
    }

  /* if no match, the next index is the copy of
   * the first one */
  if (max_pos > 1.9f)
    {
      return -1;
    }

  return max_idx;
}

static inline int
get_prev_idx (
  NodeIndexElement * elements,
  int                num_nodes,
  float              ratio)
{
  float min_pos = -1.f;
  int min_idx = 0;
  for (int i = 0; i < num_nodes; i++)
    {
      if (elements[i].pos > min_pos &&
          elements[i].pos <
            (ratio + 0.0001f))
        {
          min_pos = elements[i].pos;
          min_idx = elements[i].index;
        }
    }
  return min_idx;
}

static int
pos_cmp (const void * a, const void * b)
{
  float pos_a = (*(NodeIndexElement*)a).pos;
  float pos_b = (*(NodeIndexElement*)b).pos;
  return pos_a > pos_b;
}

static inline void
sort_node_indices_by_pos (
  float              nodes[16][3],
  NodeIndexElement * elements,
  int                num_nodes)
{
  for (int i = 0; i < num_nodes; i++)
    {
      /* set index and position */
      elements[i].index = i;
      elements[i].pos = nodes[i][0];
    }

  qsort (
    elements, (size_t) num_nodes,
    sizeof (NodeIndexElement), pos_cmp);
}

#endif
