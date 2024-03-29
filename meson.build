# Copyright (C) 2019-2021 Alexandros Theodotou <alex at zrythm dot org>
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

project (
  'ZPlugins', ['c'],
  version: '0.2.5',
  license: 'AGPLv3+',
  meson_version: '>= 0.53.0',
  default_options: [
    'warning_level=2',
    'buildtype=debug',
    'c_std=gnu11',
    ],
  )

fs = import('fs')
guile = find_program (['guile2.2', 'guile'])
sh = find_program ('sh')
bash = find_program ('bash')
lv2lint = find_program ('lv2lint', required: false)
lv2_validate = find_program (
  'lv2_validate', required: false)
sord_validate = find_program (
  'sord_validate', required: false)
carla_single = find_program (
  'carla-single', required: false)
carla_single_wrap = find_program (
  'scripts/carla_single_wrap.sh')
lv2lint_wrap = find_program (
  'scripts/lv2lint_wrap.sh')
lv2_validate_wrap = find_program (
  'scripts/lv2_validate_wrap.sh')

# dependencies
ztoolkit_dep = dependency(
  'ztoolkit', version: '>= 0.1.1',
  fallback: ['ztoolkit', 'ztoolkit_dep'],
  default_options: ['enable_rsvg=true'])
lv2_dep = dependency (
  'lv2', version: '>=1.16.0', required: false)

cc = meson.get_compiler ('c')

# detect os
os_darwin = false
os_linux = false
os_freebsd = false
os_windows = false

if host_machine.system() == 'darwin'
  os_darwin = true
elif host_machine.system() == 'linux'
  os_linux = true
elif host_machine.system() == 'freebsd'
  os_freebsd = true
elif host_machine.system() == 'windows'
  os_windows = true
endif

prefix = get_option('prefix')
lv2dir = prefix / get_option('lv2dir')
lv2_core_path = get_option('lv2_core_path')

# cflags
common_cflags = cc.get_supported_arguments([
  '-fvisibility=hidden',
  '-Wformat=2',
  '-Wno-missing-field-initializers',
  '-Wno-unused-parameter',
  '-Wno-sequence-point',
  '-Wignored-qualifiers',
  '-Wno-cast-function-type',
  ])
if get_option ('strict_flags')
  common_cflags += cc.get_supported_arguments([
    #'-Werror=cast-qual',
    '-Werror=clobbered',
    #'-Werror=conversion',
    '-Werror=disabled-optimization',
    '-Werror=double-promotion',
    '-Werror=float-equal',
    '-Werror=logical-op',
    '-Werror=pointer-arith',
    '-Werror=sign-conversion',
    '-Werror=overlength-strings',
    '-Werror=stringop-truncation',
    '-Werror=missing-declarations',
    #'-Werror=redundant-decls',
    '-Werror=shadow',
    '-Werror=undef',
    '-Werror=unused',
    '-Werror=strict-aliasing',
    '-fstrict-aliasing',
    #'-Werror=strict-overflow',
    '-Wstrict-overflow=2',
    '-fstrict-overflow',
    '-Werror=duplicated-branches',
    '-Werror=duplicated-cond',
    '-Werror=null-dereference',
    '-Werror=init-self',
    '-Werror=jump-misses-init',
    '-Werror=missing-prototypes',
    '-Werror=nested-externs',
    '-Werror=write-strings',
    '-Werror=implicit-fallthrough',
    '-Werror=sign-compare',
    '-Werror=discarded-qualifiers',
    '-Werror=float-conversion',
    '-Werror=implicit-function-declaration',
    '-Werror=uninitialized',
    '-Werror=maybe-uninitialized',
    '-Werror=return-type',
    '-Werror=int-conversion',
    '-Werror=format-security',
    '-Werror=incompatible-pointer-types',
    '-Werror=implicit-int',
    '-Werror=multistatement-macros',
    '-Werror=switch',
    '-Werror=overflow',
    '-Werror=array-bounds',
    '-Werror=enum-compare',
    '-Werror=misleading-indentation',
    '-Werror=int-in-bool-context',
    '-Werror=type-limits',
    '-Werror=deprecated-declarations',
    '-Werror=format-extra-args',
    '-Werror=format',
    ])
  if cc.get_id() == 'gcc'
    common_cflags += cc.get_supported_arguments([
      '-Wextra',
      '-Weverything',
      ])
  endif
endif

if (get_option ('buildtype') == 'release')
  common_cflags += cc.get_supported_arguments([
    '-ffast-math',
    '-DPIC',
    '-fdata-sections',
    '-ffunction-sections',
    '-mtune=generic',
    '-msse',
    '-msse2',
    '-mfpmath=sse',
    ])
endif

if os_windows
  common_cflags += [
    '-D_WOE32=1',
    ]
endif

subdir ('ext')
subdir ('plugins')
