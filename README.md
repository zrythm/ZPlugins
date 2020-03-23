ZPlugins
========

[![builds.sr.ht status](https://builds.sr.ht/~alextee/zplugins.svg)](https://builds.sr.ht/~alextee/zplugins?)

Collection of audio DSP plugins to be bundled with the
Zrythm DAW.

Plugins
-------

- **ZChordz** - plays chords using white keys
- **ZCompressorSP** - compressor
- **ZLimiterSP** - peak limiter
- **ZLFO** - full-featured LFO for CV-based automation
- **ZPhaserSP** - stereo phaser
- **ZSaw** - supersaw synth with 1 parameter
- **ZVerbSP** - reverb based on zita-rev

Dependencies
------------

ztoolkit (with rsvg), libsndfile

Building
--------

Build and install all plugins

    cd ext/Soundpipe && CC=gcc make && cd ../..
    meson build
    ninja -C build
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

----

Copyright (C) 2020 Alexandros Theodotou

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.
