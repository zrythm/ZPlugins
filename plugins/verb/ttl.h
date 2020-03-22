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
  ] , [\n");

  fprintf (f, "\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 4 ;\n\
    lv2:symbol \"in_delay\" ;\n\
    lv2:name \"Pre-delay\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    units:unit units:ms ;\n\
    rdfs:comment \"Delay in ms before reverberation begins\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 5 ;\n\
    lv2:symbol \"lf_x\" ;\n\
    lv2:name \"Low freq crossover\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    lv2:portProperty pprop:logarithmic; \n\
    units:unit units:hz ;\n\
    rdfs:comment \"Crossover frequency separating low and middle frequencies\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 6 ;\n\
    lv2:symbol \"rt60_low\" ;\n\
    lv2:name \"Low freq decay\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    units:unit units:s ;\n\
    rdfs:comment \"Time to decay 60db in low-frequency band\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 7 ;\n\
    lv2:symbol \"rt60_mid\" ;\n\
    lv2:name \"Mid freq decay\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    units:unit units:s ;\n\
    rdfs:comment \"Time to decay 60db in mid-frequency band\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 8 ;\n\
    lv2:symbol \"hf_damping\" ;\n\
    lv2:name \"High freq damping\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    lv2:portProperty pprop:logarithmic; \n\
    units:unit units:hz ;\n\
    rdfs:comment \"Frequency at which the high-frequency T60 is half the middle-band's T60\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 9 ;\n\
    lv2:symbol \"eq1_freq\" ;\n\
    lv2:name \"EQ1 freq\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    lv2:portProperty pprop:logarithmic; \n\
    units:unit units:hz ;\n\
    rdfs:comment \"Center frequency of second-order Regalia Mitra peaking equalizer section 1\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 10 ;\n\
    lv2:symbol \"eq1_level\" ;\n\
    lv2:name \"EQ1 level\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    units:unit units:db ;\n\
    rdfs:comment \"Peak level of second-order Regalia-Mitra peaking equalizer section 1\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 11 ;\n\
    lv2:symbol \"eq2_freq\" ;\n\
    lv2:name \"EQ2 freq\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    lv2:portProperty pprop:logarithmic; \n\
    units:unit units:hz ;\n\
    rdfs:comment \"Center frequency of second-order Regalia Mitra peaking equalizer section 2\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 12 ;\n\
    lv2:symbol \"eq2_level\" ;\n\
    lv2:name \"EQ2 level\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    units:unit units:db ;\n\
    rdfs:comment \"Peak level of second-order Regalia-Mitra peaking equalizer section 2\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 13 ;\n\
    lv2:symbol \"wet\" ;\n\
    lv2:name \"Wet\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    rdfs:comment \"0 = all dry, 1 = all wet\" ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 14 ;\n\
    lv2:symbol \"level\" ;\n\
    lv2:name \"Level\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    units:unit units:db ;\n\
    rdfs:comment \"Output scale factor\" ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:AudioPort ;\n\
    lv2:index 15 ;\n\
    lv2:symbol \"stereo_out_l\" ;\n\
    lv2:name \"Stereo Out L\" ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:AudioPort ;\n\
    lv2:index 16 ;\n\
    lv2:symbol \"stereo_out_r\" ;\n\
    lv2:name \"Stereo Out R\" ;\n\
  ] .\n",
    /* predelay */
    40.0, 10.0, 100.0,
    /* low freq crossover */
    200.0, 50.0, 1000.0,
    /* decay low */
    3.0, 1.0, 8.0,
    /* decay mid */
    2.0, 1.0, 8.0,
    /* hf damping */
    6000.0, 1500.0, 47040.0,
    /* eq1 freq */
    315.0, 40.0, 2500.0,
    /* eq1 level */
    0.0, -15.0, 15.0,
    /* eq2 freq */
    1500.0, 160.0, 12000.0,
    /* eq2 level */
    0.0, -15.0, 15.0,
    /* wet */
    1.0, 0.0, 1.0,
    /* level */
    -20.0, -70.0, 20.0
    );
}
