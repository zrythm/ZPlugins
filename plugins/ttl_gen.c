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

#include <stdio.h>

#include PLUGIN_COMMON
#include PLUGIN_TTL_H

int main (
  int argc, const char* argv[])
{
  if (argc != 2)
    {
      fprintf (
        stderr,
        "Need 1 argument, received %d\n", argc - 1);
      return -1;
    }

  FILE * f = fopen (argv[1], "w");
  if (!f)
    {
      fprintf (
        stderr, "Failed to open file %s\n", argv[1]);
      return -1;
    }

  print_ttl (f);

  fclose (f);

  return 0;
}
