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

static void
print_ttl (FILE * f)
{
  fprintf (f,
"@prefix atom: <http://lv2plug.in/ns/ext/atom#> .\n\
@prefix doap:  <http://usefulinc.com/ns/doap#> .\n\
@prefix log:   <http://lv2plug.in/ns/ext/log#> .\n\
@prefix lv2:   <http://lv2plug.in/ns/lv2core#> .\n\
@prefix midi:  <http://lv2plug.in/ns/ext/midi#> .\n\
@prefix pg:    <http://lv2plug.in/ns/ext/port-groups#> .\n\
@prefix pprop: <http://lv2plug.in/ns/ext/port-props#> .\n\
@prefix rdf:   <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n\
@prefix rdfs:  <http://www.w3.org/2000/01/rdf-schema#> .\n\
@prefix rsz:   <http://lv2plug.in/ns/ext/resize-port#> .\n\
@prefix time:  <http://lv2plug.in/ns/ext/time#> .\n\
@prefix urid:  <http://lv2plug.in/ns/ext/urid#> .\n\
@prefix ui:    <http://lv2plug.in/ns/extensions/ui#> .\n\
@prefix units: <http://lv2plug.in/ns/extensions/units#> .\n\
@prefix work:  <http://lv2plug.in/ns/ext/worker#> .\n\n");

  fprintf (f,
"<" PLUGIN_URI ">\n\
  a lv2:Plugin,\n\
    lv2:" PLUGIN_TYPE " ;\n\
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
      lv2:AudioPort ;\n\
    lv2:index 2 ;\n\
    lv2:symbol \"stereo_in_l\" ;\n\
    lv2:name \"Stereo In L\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:AudioPort ;\n\
    lv2:index 3 ;\n\
    lv2:symbol \"stereo_in_r\" ;\n\
    lv2:name \"Stereo In R\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 4 ;\n\
    lv2:symbol \"shift\" ;\n\
    lv2:name \"Shift\" ;\n\
    lv2:default 0 ;\n\
    lv2:minimum -24 ;\n\
    lv2:maximum 24 ;\n\
    lv2:portProperty lv2:integer ;\n\
    units:unit units:semitone12TET; \n\
    rdfs:comment \"Pitch shift\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 5 ;\n\
    lv2:symbol \"window\" ;\n\
    lv2:name \"Window\" ;\n\
    lv2:default 1000 ;\n\
    lv2:minimum 1 ;\n\
    lv2:maximum 10000 ;\n\
    lv2:portProperty lv2:integer ;\n\
    units:unit units:frame; \n\
    rdfs:comment \"Window size\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 6 ;\n\
    lv2:symbol \"xfade\" ;\n\
    lv2:name \"Cross-fade\" ;\n\
    lv2:default 10 ;\n\
    lv2:minimum 0 ;\n\
    lv2:maximum 10000 ;\n\
    lv2:portProperty lv2:integer ;\n\
    units:unit units:frame; \n\
    rdfs:comment \"Crossfade\" ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:AudioPort ;\n\
    lv2:index 7 ;\n\
    lv2:symbol \"stereo_out_l\" ;\n\
    lv2:name \"Stereo Out L\" ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:AudioPort ;\n\
    lv2:index 8 ;\n\
    lv2:symbol \"stereo_out_r\" ;\n\
    lv2:name \"Stereo Out R\" ;\n\
  ] .\n");
}
