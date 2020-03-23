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

/**
 * \file
 *
 * Math utilities.
 */

#ifndef __ZPLUGINS_MATH_UTILS_H__
#define __ZPLUGINS_MATH_UTILS_H__

#include <float.h>
#include <math.h>

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif
#ifndef powf
float powf (float dummy0, float dummy1);
#endif
#ifndef pow
double pow (double dummy0, double dummy1);
#endif
#ifndef sinf
float sinf (float dummy0);
#endif

static const float PI = (float) M_PI;

#ifndef MAX
# define MAX(x,y) (x > y ? x : y)
#endif

#ifndef MIN
# define MIN(x,y) (x < y ? x : y)
#endif

#ifndef CLAMP
# define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif

/**
 * Rounds a float to a given type.
 */
#define math_round_float_to_type(x,type) \
  ((type) (x + 0.5f - (x < 0.f)))

/**
 * Rounds a float to an int.
 */
#define math_round_float_to_int(x) \
  math_round_float_to_type (x,int)

/**
 * Checks if 2 floats are equal.
 */
#define math_floats_equal(a,b) \
  ((a) > (b) ? \
   (a) - (b) < FLT_EPSILON : \
   (b) - (a) < FLT_EPSILON)

/**
 * Checks if 2 floats are equal.
 */
#define math_floats_equal_w_epsilon(a,b,epsilon) \
  ((a) > (b) ? \
   (a) - (b) < epsilon : \
   (b) - (a) < epsilon)

#define math_doubles_equal(a,b) \
  (a > b ? a - b < DBL_EPSILON : b - a < DBL_EPSILON)

#endif
