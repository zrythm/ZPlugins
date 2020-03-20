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

# name, version
plugins = [
  ['Saw', '0.0.1'],
  ]

foreach pl : plugins
  pl_str = 'Z' + pl[0]
  pl_caps = pl[0].to_upper ()
  pl_lowercase = pl[0].to_lower ()
  pl_dsp_lib_name_noext = pl_str + '_dsp'

  pl_dir = join_paths(lv2dir, pl_str + '.lv2')
  pl_cdata = configuration_data ()
  pl_cdata.set ('PLUGIN_NAME', pl_str)
  pl_cdata.set (
    'PROJECT_URI',
    'https://www.zrythm.org/plugins')
  pl_cdata.set (
    'PLUGIN_URI',
    pl_cdata.get ('PROJECT_URI') + '/' + pl_str)
  if os_windows
    pl_cdata.set (
      'PLUGIN_DSP_BINARY', pl_dsp_lib_name_noext + '.dll')
  else
    pl_cdata.set (
      'PLUGIN_DSP_BINARY', pl_dsp_lib_name_noext + '.so')
  endif
  pl_cdata.set ('PLUGIN_TTL', pl_str + '.ttl')

  config_h_data = configuration_data ()
  config_h_data.set_quoted (
    'PROJECT_URI',
    pl_cdata.get ('PROJECT_URI'))
  config_h_data.set_quoted (
    'PLUGIN_NAME',
    pl_cdata.get ('PLUGIN_NAME'))
  config_h_data.set_quoted (
    'PLUGIN_URI',
    pl_cdata.get ('PLUGIN_URI'))
  config_h_data.set_quoted (
    'INSTALL_PATH', pl_dir)
  if get_option('buildtype') == 'release'
    config_h_data.set ('RELEASE', 1)
  endif
  config_h_data.set_quoted (
    pl_caps + '_VERSION', pl[1])

  # create config.h
  pl_config_h = configure_file (
    output: pl_lowercase + '_config.h',
    configuration: config_h_data,
    )
  pl_config_h_dep = declare_dependency (
    sources: pl_config_h,
    )

  lv2_dep = dependency (
    'lv2', version: '>=1.16.0', required: false)
  if not lv2_dep.found()
    lilv_proj = subproject('lilv')
    lv2_dep = lilv_proj.get_variable('lv2_dep')
  endif

  pl_deps = [
    pl_config_h_dep,
    cc.find_library ('m'),
    lv2_dep,
    #dependency ('sndfile', static: true),
    #dependency ('flac', static: true),
    #dependency ('ogg', static: true),
    #dependency ('vorbis', static: true),
    ]

  # create shared library
  pl_inc_dirs = include_directories([
    '.', pl_lowercase,
    join_paths ('..', 'ext', 'Soundpipe', 'h'),
    ])
  pl_dsp_lib = shared_library (
    pl_dsp_lib_name_noext,
    name_prefix: '',
    sources: [
      join_paths (
        pl_lowercase, pl_lowercase + '.c'),
      ],
    dependencies: pl_deps,
    include_directories: pl_inc_dirs,
    link_with: soundpipe_lib,
    install: true,
    install_dir: pl_dir,
    c_args: [
      common_cflags,
      '-DPLUGIN_CONFIG="../' + pl_lowercase + '_config.h"',
      ],
    )

  # create and install manifest ttl
  manifest_ttl = configure_file (
    input: join_paths (pl_lowercase, 'manifest.ttl.in'),
    output: pl_str + '_manifest.ttl',
    configuration: pl_cdata,
    )
  install_data (
    manifest_ttl,
    install_dir: pl_dir,
    rename: 'manifest.ttl',
    )

  # create and install ttl
  lv2_ttl_gen = executable (
    pl_lowercase + 'ttl_gen',
    sources: [
      join_paths (
        pl_lowercase, 'ttl_gen.c'),
      ],
    include_directories: pl_inc_dirs,
    dependencies: lv2_dep,
    c_args: [
      common_cflags,
      '-DPLUGIN_CONFIG="../' + pl_lowercase + '_config.h"',
      ],
    install: false,
    )
  pl_ttl = custom_target (
    pl_str + '.ttl',
    output: pl_str + '.ttl',
    input: [ lv2_ttl_gen, pl_config_h ],
    command: [
      lv2_ttl_gen, '@OUTPUT@' ],
    install: true,
    install_dir: pl_dir,
    )

  # test
  pl_build_dir = meson.current_build_dir ()
  lv2lint = find_program ('lv2lint', required: false)
  lv2_validate = find_program (
    'lv2_validate', required: false)
  sord_validate = find_program (
    'sord_validate', required: false)
  if lv2lint.found() and (os_linux or os_freebsd)
    lv2lint_wrap = find_program (
      join_paths ('..', 'scripts', 'lv2lint_wrap.sh'))
    test (
      'LV2 lint', lv2lint_wrap,
      args: [
        lv2lint.path(),
        pl_build_dir,
        pl_str,
        pl_cdata.get('PLUGIN_URI'),
        ])
  endif
  if lv2_validate.found() and sord_validate.found()
    test (
      'LV2 validate', lv2_validate,
      args: pl_ttl)
  endif
endforeach
