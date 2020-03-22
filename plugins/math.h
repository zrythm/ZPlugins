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

#endif
