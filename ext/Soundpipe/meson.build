# Copyright (C) 2020 Alexandros Theodotou <alex at zrythm dot org>
#
# This file is part of ZPlugins
#
# ZPlugins is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ZPlugins is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with ZPlugins.  If not, see <https://www.gnu.org/licenses/>.

# things that need to be built before
pre_soundpipe = custom_target (
  'pre-soundpipe',
  output: 'pre-soundpipe',
  command: [
    'cd', meson.current_source_dir (), '&&',
    'CC=' + cc.get_id (), 'make',
    ])

subdir('modules')
subdir('lib')

soundpipe_lib = static_library (
  'soundpipe-lib',
  soundpipe_lib_files,
  soundpipe_module_files,
  pre_soundpipe,
  include_directories: [
    'h',
    join_paths ('lib', 'dr_wav'),
    join_paths ('lib', 'faust'),
    join_paths ('lib', 'fft'),
    join_paths ('lib', 'inih'),
    join_paths ('lib', 'kissfft'),
    join_paths ('lib', 'openlpc'),
    join_paths ('lib', 'spa'),
    ],
  pic: true,
  dependencies: [
    dependency('sndfile'),
    cc.find_library('m'),
    ],
  c_args: [
    '-fvisibility=hidden',
    ],
  )
