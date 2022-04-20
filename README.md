<!--
Copyright (C) 2020-2022 Alexandros Theodotou

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.
-->
ZPlugins
========

[![builds.sr.ht status](https://builds.sr.ht/~alextee/zplugins.svg)](https://builds.sr.ht/~alextee/zplugins?)
[![travis build status](https://img.shields.io/travis/zrythm/ZPlugins?label=travis%20build)](https://travis-ci.org/zrythm/ZPlugins)
[![code coverage](https://img.shields.io/coveralls/github/zrythm/ZPlugins)](https://coveralls.io/github/zrythm/ZPlugins)

Collection of audio DSP plugins inteded to be bundled
with the Zrythm DAW.

Plugins
-------

- **ZChordz** - plays chords using white keys
- **ZLFO** - full-featured LFO for CV-based automation
- **ZSaw** - supersaw synth with 1 parameter

The following plugins are deprecated and will be removed from future releases:

- **ZCompressorSP** - compressor
- **ZLimiterSP** - peak limiter
- **ZPhaserSP** - stereo phaser
- **ZPitchSP** - pitch shifter
- **ZVerbSP** - reverb based on zita-rev

Dependencies
------------

guile, [ztoolkit](https://git.zrythm.org/cgit/ztoolkit/) (with rsvg), libsndfile

Building
--------

Build and install all plugins

    meson build
    ninja -C build -Dlv2dir=lib/lv2
    ninja -C build install

To build specific plugins only, pass their names as below

    meson build -Dplugins=Saw,Verb

License
-------

ZPlugins is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

ZPlugins is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

The full text of the license can be found in the
[COPYING](COPYING) file.

For the copyright years, ZPlugins uses a range (“2008-2010”) instead of
listing individual years (“2008, 2009, 2010”) if and only if every year
in the range, inclusive, is a “copyrightable” year that would be listed
individually.

Some files, where specified, are licensed under
different licenses.

Zrythm and the Zrythm logo are trademarks of Alexandros Theodotou.
See <https://git.sr.ht/~alextee/zrythm/tree/master/item/TRADEMARKS.md>
for our trademark policy.
