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
@prefix units:   <http://lv2plug.in/ns/extensions/units#> .\n\
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
    lv2:symbol \"max_notch_freq\" ;\n\
    lv2:name \"Max Notch Freq\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    lv2:portProperty pprop:logarithmic; \n\
    units:unit units:hz ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 5 ;\n\
    lv2:symbol \"min_notch_freq\" ;\n\
    lv2:name \"Min Notch Freq\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    lv2:portProperty pprop:logarithmic; \n\
    units:unit units:hz ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 6 ;\n\
    lv2:symbol \"notch_width\" ;\n\
    lv2:name \"Notch width\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    lv2:portProperty pprop:logarithmic; \n\
    units:unit units:hz ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 7 ;\n\
    lv2:symbol \"notch_freq\" ;\n\
    lv2:name \"Notch Freq\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 8 ;\n\
    lv2:symbol \"vibrato_mode\" ;\n\
    lv2:name \"Vibrato Mode\" ;\n\
    lv2:default 1 ;\n\
    lv2:minimum 0 ;\n\
    lv2:maximum 1 ;\n\
    lv2:portProperty lv2:toggled ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 9 ;\n\
    lv2:symbol \"depth\" ;\n\
    lv2:name \"Depth\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 10 ;\n\
    lv2:symbol \"feedback_gain\" ;\n\
    lv2:name \"Feedback Gain\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 11 ;\n\
    lv2:symbol \"invert\" ;\n\
    lv2:name \"Invert\" ;\n\
    lv2:default 0 ;\n\
    lv2:minimum 0 ;\n\
    lv2:maximum 1 ;\n\
    lv2:portProperty lv2:toggled ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 12 ;\n\
    lv2:symbol \"level\" ;\n\
    lv2:name \"Level\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    units:unit units:db ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 13 ;\n\
    lv2:symbol \"lfo_bpm\" ;\n\
    lv2:name \"LFO BPM\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:AudioPort ;\n\
    lv2:index 14 ;\n\
    lv2:symbol \"stereo_out_l\" ;\n\
    lv2:name \"Stereo Out L\" ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:AudioPort ;\n\
    lv2:index 15 ;\n\
    lv2:symbol \"stereo_out_r\" ;\n\
    lv2:name \"Stereo Out R\" ;\n\
  ] .\n",
    /* max notch */
    800.0, 20.0, 10000.0,
    /* min notch */
    100.0, 20.0, 5000.0,
    /* notch width */
    1000.0, 10.0, 5000.0,
    /* notch freq */
    1.5, 1.1, 4.0,
    /* depth */
    1.0, 0.0, 1.0,
    /* feedback gain */
    0.0, 0.0, 1.0,
    /* level */
    0.0, -60.0, 10.0,
    /* lfo bpm */
    30.0, 24.0, 360.0
    );
}
