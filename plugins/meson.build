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

# name, type, version
plugins = [
  ['Chordz', 'MIDIPlugin', '1.0.0'],
  ['CompressorSP', 'CompressorPlugin', '1.0.0'],
  ['LimiterSP', 'LimiterPlugin', '0.1.0'],
  ['LFO', 'OscillatorPlugin', '1.0.1'],
  ['PhaserSP', 'PhaserPlugin', '0.1.0'],
  #['Saturator', 'DistortionPlugin', '0.1.0'],
  ['Saw', 'InstrumentPlugin', '1.0.0'],
  ['VerbSP', 'ReverbPlugin', '0.1.0'],
  ]

fs = import('fs')
guile = find_program ('guile')
ztoolkit_dep = dependency(
  'ztoolkit', version: '== 0.1.1',
  fallback: ['ztoolkit', 'ztoolkit_dep'],
  default_options: ['enable_rsvg=true'])

if os_darwin
  libext = '.dylib'
elif os_windows
  libext = '.dll'
else
  libext = '.so'
endif

foreach pl : plugins
if get_option ('plugins').contains (pl[0])

  plugin_name = pl[0]
  plugin_name_for_export = plugin_name
  if get_option ('trial_ver')
    plugin_name_for_export = plugin_name + '-trial'
  endif

  pl_str = 'Z' + plugin_name_for_export
  pl_caps = plugin_name.to_upper ()
  pl_lowercase = plugin_name.to_lower ()
  pl_dsp_lib_name_noext = pl_str + '_dsp'
  pl_ui_lib_name_noext = pl_str + '_ui'
  pl_type = pl[1]
  pl_version = pl[2]
  pl_minor_version = pl_version.split('.')[1]
  pl_micro_version = pl_version.split('.')[1]

  pl_dir = join_paths(lv2dir, pl_str + '.lv2')
  project_uri = 'https://www.zrythm.org/plugins'
  pl_uri = project_uri + '/' + pl_str
  pl_ui_uri = pl_uri + '#UI'
  pl_dsp_binary = pl_dsp_lib_name_noext + libext
  pl_ui_binary = pl_ui_lib_name_noext + libext
  pl_ttl = pl_str + '.ttl'

  resources_exist = fs.is_dir (
    join_paths (pl_lowercase, 'resources'))

  # get UI data
  ui_type = 'none'
  ui_uri = 'none'
  ui_binary = 'none'
  ui_exists = fs.is_file (
    join_paths (pl_lowercase, 'ui.c'))
  if ui_exists
    if os_windows
      ui_type = 'WindowsUI'
    elif os_linux or os_freebsd
      ui_type = 'X11UI'
    elif os_darwin
      ui_type = 'CocoaUI'
    endif
    ui_uri = pl_ui_uri
    ui_binary = pl_ui_binary
  endif

  # set config.h data
  config_h_data = configuration_data ()
  config_h_data.set_quoted (
    'PROJECT_URI', project_uri)
  config_h_data.set_quoted (
    'PLUGIN_NAME', pl_str)
  config_h_data.set_quoted (
    'PLUGIN_URI', pl_uri)
  config_h_data.set_quoted (
    'PLUGIN_UI_URI', pl_ui_uri)
  config_h_data.set_quoted (
    'PLUGIN_UI_TYPE', ui_type)
  config_h_data.set_quoted (
    'PLUGIN_TYPE', pl_type)
  config_h_data.set_quoted (
    'PLUGIN_COMMON',
    join_paths (pl_lowercase, 'common.h'))
  config_h_data.set_quoted (
    'PLUGIN_TTL_H', join_paths (pl_lowercase, 'ttl.h'))
  config_h_data.set_quoted (
    'INSTALL_PATH', pl_dir)
  if get_option('buildtype') == 'release'
    config_h_data.set ('RELEASE', 1)
  endif
  if get_option ('trial_ver')
    config_h_data.set ('TRIAL_VER', 1)
    config_h_data.set ('SECONDS_TO_SILENCE', 900)
  endif
  config_h_data.set_quoted (
    pl_caps + '_VERSION', pl[2])

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
    ]

  # create dsp shared library
  pl_inc_dirs = include_directories([
    '.', pl_lowercase,
    join_paths ('..', 'ext', 'Soundpipe', 'h'),
    ])
  pl_dsp_lib = shared_library (
    pl_dsp_lib_name_noext,
    name_prefix: '',
    sources: [
      join_paths (
        pl_lowercase, 'dsp.c'),
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

  # create UI shared library if UI exists
  if ui_exists
    ui_inc_dirs = include_directories([
      '.', pl_lowercase,
      ])
    pl_ui_lib = shared_library (
      pl_ui_lib_name_noext,
      name_prefix: '',
      sources: [
        join_paths (
          pl_lowercase, 'ui.c'),
        ],
      dependencies: [
        pl_deps,
        ztoolkit_dep,
        dependency ('librsvg-2.0'),
        ],
      include_directories: ui_inc_dirs,
      install: true,
      install_dir: pl_dir,
      c_args: [
        common_cflags,
        '-DPLUGIN_CONFIG="../' + pl_lowercase + '_config.h"',
        ],
      )
  endif

  # installs resources if any
  if resources_exist
    install_subdir (
      join_paths (pl_lowercase, 'resources'),
      install_dir: pl_dir)
  endif

  presets_file = 'none'
  # TODO add presets

  # create and install manifest ttl
  manifest_ttl = configure_file (
    input: 'manifest_gen.scm',
    output: pl_str + '_manifest.ttl',
    command: [
      guile, '-s', '@INPUT@', '@OUTPUT@',
      project_uri, pl_type, pl_uri,
      pl_dsp_binary, pl_minor_version,
      pl_micro_version, pl_ttl,
      ui_type, ui_uri, ui_binary, presets_file,
      ],
    )
  install_data (
    manifest_ttl,
    install_dir: pl_dir,
    rename: 'manifest.ttl',
    )

  # create and install ttl
  lv2_ttl_gen = executable (
    pl_lowercase + '_ttl_gen',
    sources: [
      'ttl_gen.c'
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
        lv2lint.path(), pl_build_dir, pl_str,
        pl_uri,
        ],
      suite: pl[0])
  endif
  if lv2_validate.found() and sord_validate.found()
    test (
      'LV2 validate', lv2_validate,
      args: pl_ttl,
      suite: pl[0])
  endif
endif
endforeach
