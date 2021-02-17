/*
 * Copyright (C) 2021 Alexandros Theodotou <alex at zrythm dot org>
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

static void
print_ttl (FILE * f)
{
  fprintf (f,
"@prefix atom: <http://lv2plug.in/ns/ext/atom#> .\n\
@prefix doap: <http://usefulinc.com/ns/doap#> .\n\
@prefix log:  <http://lv2plug.in/ns/ext/log#> .\n\
@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .\n\
@prefix midi: <http://lv2plug.in/ns/ext/midi#> .\n\
@prefix pprop: <http://lv2plug.in/ns/ext/port-props#> .\n\
@prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n\
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n\
@prefix rsz:  <http://lv2plug.in/ns/ext/resize-port#> .\n\
@prefix time:  <http://lv2plug.in/ns/ext/time#> .\n\
@prefix urid: <http://lv2plug.in/ns/ext/urid#> .\n\
@prefix ui:   <http://lv2plug.in/ns/extensions/ui#> .\n\
@prefix work:  <http://lv2plug.in/ns/ext/worker#> .\n\n");

  fprintf (f,
"<" PLUGIN_URI ">\n\
  a lv2:Plugin,\n\
    lv2:MIDIPlugin ;\n\
  doap:name \"" PLUGIN_NAME "\" ;\n\
  doap:license <https://www.gnu.org/licenses/agpl-3.0.html> ;\n\
  lv2:project <" PROJECT_URI "> ;\n\
  lv2:requiredFeature urid:map ;\n\
  lv2:optionalFeature log:log ,\n\
                      lv2:hardRTCapable ;\n\
  lv2:port [\n\
    a lv2:InputPort ,\n\
      atom:AtomPort ;\n\
    atom:bufferType atom:Sequence ;\n\
    atom:supports <http://lv2plug.in/ns/ext/midi#MidiEvent> ;\n\
    rsz:minimumSize 2048 ;\n\
    lv2:index 0 ;\n\
    lv2:designation lv2:control ;\n\
    lv2:symbol \"control\" ;\n\
    lv2:name \"Control\" ;\n\
    rdfs:comment \"GUI/host to plugin communication\" ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      atom:AtomPort ;\n\
    atom:bufferType atom:Sequence ;\n\
    lv2:index 1 ;\n\
    lv2:designation lv2:control ;\n\
    lv2:symbol \"notify\" ;\n\
    lv2:name \"Notify\" ;\n\
    rdfs:comment \"Plugin to GUI communication\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 2 ;\n\
    lv2:symbol \"major\" ;\n\
    lv2:name \"Major\" ;\n\
    lv2:portProperty lv2:toggled ;\n\
    lv2:default 1 ;\n\
    lv2:minimum 0 ;\n\
    lv2:maximum 1 ;\n\
  ] , [\n");

  fprintf (f, "\
    a lv2:OutputPort ,\n\
      atom:AtomPort ;\n\
    atom:bufferType atom:Sequence ;\n\
    atom:supports <http://lv2plug.in/ns/ext/midi#MidiEvent> ;\n\
    rsz:minimumSize 2048 ;\n\
    lv2:index 3 ;\n\
    lv2:designation lv2:control ;\n\
    lv2:symbol \"midi_out\" ;\n\
    lv2:name \"MIDI out\" ;\n\
    rdfs:comment \"MIDI output\" ;\n\
  ] .\n\n");
}
