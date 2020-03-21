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

#include "common.h"

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

  fprintf (f,
"@prefix atom: <http://lv2plug.in/ns/ext/atom#> .\n\
@prefix doap: <http://usefulinc.com/ns/doap#> .\n\
@prefix foaf: <http://xmlns.com/foaf/0.1/> .\n\
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
"<" PROJECT_URI ">\n\
  a doap:Project ;\n\
  doap:name \"Zrythm plugins\" .\n\
\n");

  fprintf (f,
"<" PLUGIN_URI ">\n\
  a lv2:Plugin,\n\
    lv2:MIDIPlugin ;\n\
  doap:name \"" PLUGIN_NAME "\" ;\n\
  doap:maintainer [\n\
    foaf:name \"\"\"Alexandros Theodotou\"\"\" ;\n\
    foaf:homepage <https://www.zrythm.org> ;\n\
  ] ;\n\
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
    lv2:symbol \"scale\" ;\n\
    lv2:name \"Scale\" ;\n\
    lv2:portProperty lv2:integer ;\n\
    lv2:portProperty lv2:enumeration ;\n\
    lv2:scalePoint [ rdfs:label \"C\"; rdf:value 0 ] ;\n\
    lv2:scalePoint [ rdfs:label \"Db\"; rdf:value 1 ] ;\n\
    lv2:scalePoint [ rdfs:label \"D\"; rdf:value 2 ] ;\n\
    lv2:scalePoint [ rdfs:label \"Eb\"; rdf:value 3 ] ;\n\
    lv2:scalePoint [ rdfs:label \"E\"; rdf:value 4 ] ;\n\
    lv2:scalePoint [ rdfs:label \"F\"; rdf:value 5 ] ;\n\
    lv2:scalePoint [ rdfs:label \"F#\"; rdf:value 6 ] ;\n\
    lv2:scalePoint [ rdfs:label \"G\"; rdf:value 7 ] ;\n\
    lv2:scalePoint [ rdfs:label \"Ab\"; rdf:value 8 ] ;\n\
    lv2:scalePoint [ rdfs:label \"A\"; rdf:value 9 ] ;\n\
    lv2:scalePoint [ rdfs:label \"Bb\"; rdf:value 10 ] ;\n\
    lv2:scalePoint [ rdfs:label \"B\"; rdf:value 11 ] ;\n\
    lv2:default 0 ;\n\
    lv2:minimum 0 ;\n\
    lv2:maximum 11 ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 3 ;\n\
    lv2:symbol \"major\" ;\n\
    lv2:name \"Major\" ;\n\
    lv2:portProperty lv2:toggled ;\n\
    lv2:default 1 ;\n\
    lv2:minimum 0 ;\n\
    lv2:maximum 1 ;\n\
  ] , [\n");

  for (int i = CHORDZ_BASS; i <= CHORDZ_THIRTEENTH; i++)
    {
      fprintf (f, "\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index %d ;\n", i);
      char symbol[600];
      char name[600];
      double def = 0.f;
      switch (i)
        {
        case CHORDZ_BASS:
          strcpy (symbol, "bass");
          strcpy (name, "Bass");
          def = 1.f;
          break;
        case CHORDZ_FIRST:
          strcpy (symbol, "first");
          strcpy (name, "1st");
          def = 1.f;
          break;
        case CHORDZ_THIRD:
          strcpy (symbol, "third");
          strcpy (name, "3rd");
          def = 1.f;
          break;
        case CHORDZ_FIFTH:
          strcpy (symbol, "fifth");
          strcpy (name, "5th");
          def = 1.f;
          break;
        case CHORDZ_SEVENTH:
          strcpy (symbol, "seventh");
          strcpy (name, "7th");
          def = 1.f;
          break;
        case CHORDZ_OCTAVE:
          strcpy (symbol, "octave");
          strcpy (name, "Octave");
          break;
        case CHORDZ_NINTH:
          strcpy (symbol, "ninth");
          strcpy (name, "9th");
          break;
        case CHORDZ_ELEVENTH:
          strcpy (symbol, "eleventh");
          strcpy (name, "11th");
          break;
        case CHORDZ_THIRTEENTH:
          strcpy (symbol, "thirteenth");
          strcpy (name, "13th");
          break;
        default:
          break;
        }
      fprintf (f, "\
    lv2:symbol \"%s\" ;\n\
    lv2:name \"%s\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n", symbol, name, def, 0.0, 1.0);
    }

  fprintf (f, "\
    a lv2:OutputPort ,\n\
      atom:AtomPort ;\n\
    atom:bufferType atom:Sequence ;\n\
    atom:supports <http://lv2plug.in/ns/ext/midi#MidiEvent> ;\n\
    rsz:minimumSize 2048 ;\n\
    lv2:index %d ;\n\
    lv2:designation lv2:control ;\n\
    lv2:symbol \"midi_out\" ;\n\
    lv2:name \"MIDI out\" ;\n\
    rdfs:comment \"MIDI output\" ;\n\
  ] .\n\n", CHORDZ_MIDI_OUT);

  fclose (f);

  return 0;
}
