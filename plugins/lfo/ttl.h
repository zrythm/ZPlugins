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

typedef enum PortType
{
  PORT_TYPE_FLOAT,
  PORT_TYPE_INT,
  PORT_TYPE_ENUM,
  PORT_TYPE_TOGGLE,
} PortType;

typedef enum NodeProperty
{
  NODE_PROP_POS,
  NODE_PROP_VAL,
  NODE_PROP_CURVE,
} NodeProperty;

static void
print_ttl (FILE * f)
{
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
@prefix time:  <http://lv2plug.in/ns/ext/time#> .\n\
@prefix urid: <http://lv2plug.in/ns/ext/urid#> .\n\
@prefix units:   <http://lv2plug.in/ns/extensions/units#> .\n\
@prefix ui:   <http://lv2plug.in/ns/extensions/ui#> .\n\n");

  fprintf (f,
"<" PLUGIN_URI ">\n\
  a lv2:Plugin,\n\
    lv2:" PLUGIN_TYPE " ;\n\
  doap:name \"" PLUGIN_NAME "\" ;\n\
  doap:license <https://www.gnu.org/licenses/agpl-3.0.html> ;\n\
  lv2:project <" PROJECT_URI "> ;\n\
  lv2:requiredFeature urid:map ;\n\
  lv2:optionalFeature lv2:hardRTCapable ;\n\
  lv2:optionalFeature log:log ;\n\
  lv2:port [\n\
    a lv2:InputPort ,\n\
      atom:AtomPort ;\n\
    atom:bufferType atom:Sequence ;\n\
    atom:supports time:Position ;\n\
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
    a lv2:OutputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index 2 ;\n\
    lv2:symbol \"sample_to_ui\" ;\n\
    lv2:name \"Sample to UI\" ;\n\
    rdfs:comment \"Plugin to GUI communication\" ;\n\
    lv2:default %d ;\n\
    lv2:minimum %d ;\n\
    lv2:maximum %d ;\n\
    lv2:portProperty lv2:integer ;\n\
    lv2:portProperty pprop:notOnGUI ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:CVPort ;\n\
    lv2:index 3 ;\n\
    lv2:symbol \"cv_gate\" ;\n\
    lv2:name \"Gate\" ;\n\
    rdfs:comment \"CV gate\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n\
    a lv2:InputPort ,\n\
      lv2:CVPort ;\n\
    lv2:index 4 ;\n\
    lv2:symbol \"cv_trigger\" ;\n\
    lv2:name \"Trigger\" ;\n\
    rdfs:comment \"CV trigger\" ;\n\
    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
    lv2:portProperty pprop:trigger, pprop:discreteCV ;\n\
  ] , [\n",
  0, 0, 30720000, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0);

  /* write input controls */
  int index = LFO_GATE;
  for (int i = index; i <= LFO_NUM_NODES; i++)
    {
      float def = 0.f;
      float min = 0.f;
      float max = 1.f;
      int defi = 0;
      int mini = 0;
      int maxi = 1;
      int is_trigger = 0;
      PortType type = PORT_TYPE_FLOAT;
      char symbol[256] = "\0";
      char name[256] = "\0";
      char comment[800] = "\0";
      char unit[800] = "\0";
      char scale_points[20000] = "\0";
      switch (i)
        {
        case LFO_SINE_TOGGLE:
          strcpy (symbol, "sine_toggle");
          strcpy (name, "Sine toggle");
          type = PORT_TYPE_TOGGLE;
          break;
        case LFO_SAW_TOGGLE:
          strcpy (symbol, "saw_toggle");
          strcpy (name, "Saw toggle");
          type = PORT_TYPE_TOGGLE;
          break;
        case LFO_SQUARE_TOGGLE:
          strcpy (symbol, "square_toggle");
          strcpy (name, "Square toggle");
          type = PORT_TYPE_TOGGLE;
          break;
        case LFO_TRIANGLE_TOGGLE:
          strcpy (symbol, "triangle_toggle");
          strcpy (name, "Triangle toggle");
          type = PORT_TYPE_TOGGLE;
          break;
        case LFO_CUSTOM_TOGGLE:
          strcpy (symbol, "custom_toggle");
          strcpy (name, "Custom toggle");
          type = PORT_TYPE_TOGGLE;
          def = 1.f;
          break;
        case LFO_GATED_MODE:
          strcpy (symbol, "gated_mode");
          strcpy (name, "Gated mode");
          strcpy (
            comment,
            "If this is on, output will only be "
            "produced if gate or CV gate have a "
            "non-zero signal");
          type = PORT_TYPE_TOGGLE;
          break;
        case LFO_GATE:
          strcpy (symbol, "gate");
          strcpy (name, "Gate");
          strcpy (
            comment, "Not used at the moment");
          break;
        case LFO_TRIGGER:
          strcpy (symbol, "trigger");
          strcpy (name, "Trigger");
          is_trigger = 1;
          break;
        case LFO_SYNC_RATE:
          strcpy (symbol, "sync_rate");
          strcpy (name, "Sync rate");
          type = PORT_TYPE_ENUM;
          sprintf (
            scale_points, "\
    lv2:scalePoint [ rdfs:label \"1/128\"; rdf:value 0 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/64\"; rdf:value 1 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/32\"; rdf:value 2 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/16\"; rdf:value 3 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/8\"; rdf:value 4 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/4\"; rdf:value 5 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/2\"; rdf:value 6 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/1\"; rdf:value 7 ] ;\n\
    lv2:scalePoint [ rdfs:label \"2/1\"; rdf:value 8 ] ;\n\
    lv2:scalePoint [ rdfs:label \"4/1\"; rdf:value 9 ] ;\n\
    lv2:scalePoint [ rdfs:label \"8/1\"; rdf:value 10 ] ;\n\
    lv2:scalePoint [ rdfs:label \"16/1\"; rdf:value 11 ] ;\n\
    lv2:scalePoint [ rdfs:label \"32/1\"; rdf:value 11 ] ;\n\
    lv2:scalePoint [ rdfs:label \"64/1\"; rdf:value 11 ] ;\n\
    lv2:scalePoint [ rdfs:label \"128/1\"; rdf:value 11 ] ;\n");
          defi = SYNC_1_4;
          mini = 0;
          maxi = NUM_SYNC_RATES - 1;
          break;
        case LFO_GRID_STEP:
          strcpy (symbol, "grid_step");
          strcpy (name, "Grid step");
          type = PORT_TYPE_ENUM;
          sprintf (
            scale_points, "\
    lv2:scalePoint [ rdfs:label \"1/1\"; rdf:value 0 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/2\"; rdf:value 1 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/4\"; rdf:value 2 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/8\"; rdf:value 3 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/16\"; rdf:value 4 ] ;\n\
    lv2:scalePoint [ rdfs:label \"1/32\"; rdf:value 5 ] ;\n");
          defi = GRID_STEP_EIGHTH;
          mini = 0;
          maxi = GRID_STEP_THIRTY_SECOND;
          break;
        case LFO_SYNC_RATE_TYPE:
          strcpy (symbol, "sync_rate_type");
          strcpy (name, "Sync rate type");
          type = PORT_TYPE_ENUM;
          sprintf (
            scale_points, "\
    lv2:scalePoint [ rdfs:label \"Normal\"; rdf:value 0 ] ;\n\
    lv2:scalePoint [ rdfs:label \"Dotted (.)\"; rdf:value 1 ] ;\n\
    lv2:scalePoint [ rdfs:label \"Triplet (t)\"; rdf:value 2 ] ;\n");
          defi = SYNC_TYPE_NORMAL;
          mini = 0;
          maxi = SYNC_TYPE_TRIPLET;
          break;
        case LFO_FREQ:
          strcpy (symbol, "freq");
          strcpy (name, "Frequency");
          strcpy (
            comment, "Frequency if free running");
          strcpy (unit, "hz");
          min = MIN_FREQ;
          def = DEF_FREQ;
          max = MAX_FREQ;
          break;
        case LFO_SHIFT:
          strcpy (symbol, "shift");
          strcpy (name, "Shift");
          strcpy (comment, "Shift (phase)");
          def = 0.5f;
          break;
        case LFO_RANGE_MIN:
          strcpy (symbol, "range_min");
          strcpy (name, "Range min");
          min = -1.f;
          def = -1.f;
          break;
        case LFO_RANGE_MAX:
          strcpy (symbol, "range_max");
          strcpy (name, "Range max");
          min = -1.f;
          def = 1.f;
          break;
        case LFO_STEP_MODE:
          strcpy (symbol, "step_mode");
          strcpy (name, "Step mode");
          strcpy (comment, "Step mode enabled");
          type = PORT_TYPE_TOGGLE;
          break;
        case LFO_FREE_RUNNING:
          strcpy (symbol, "free_running");
          strcpy (name, "Free running");
          strcpy (comment, "Free run toggle");
          type = PORT_TYPE_TOGGLE;
          def = 1.f;
          break;
        case LFO_HINVERT:
          strcpy (symbol, "hinvert");
          strcpy (name, "H invert");
          strcpy (comment, "Horizontal invert");
          type = PORT_TYPE_TOGGLE;
          break;
        case LFO_VINVERT:
          strcpy (symbol, "vinvert");
          strcpy (name, "V invert");
          strcpy (comment, "Vertical invert");
          type = PORT_TYPE_TOGGLE;
          break;
        case LFO_NUM_NODES:
          strcpy (symbol, "num_nodes");
          strcpy (name, "Node count");
          type = PORT_TYPE_INT;
          defi = 2;
          mini = 2;
          maxi = 16;
          break;
        default:
          break;
        }
      if (i >= LFO_NODE_1_POS &&
          i <= LFO_NODE_16_CURVE)
        {
          NodeProperty prop =
            (i - LFO_NODE_1_POS) % 3;
          int node_id = (i - LFO_NODE_1_POS) / 3 + 1;

          switch (prop)
            {
            case NODE_PROP_POS:
              sprintf (
                symbol, "node_%d_pos", node_id);
              sprintf (
                name, "Node %d position", node_id);
              if (node_id == 2)
                def = 1.f;
              break;
            case NODE_PROP_VAL:
              sprintf (
                symbol, "node_%d_val", node_id);
              sprintf (
                name, "Node %d value", node_id);
              if (node_id == 1)
                def = 1.f;
              break;
            case NODE_PROP_CURVE:
              sprintf (
                symbol, "node_%d_curve", node_id);
              sprintf (
                name, "Node %d curve", node_id);
              break;
            default:
              break;
            }
        }

      /* write port */
      fprintf (f,
"    a lv2:InputPort ,\n\
      lv2:ControlPort ;\n\
    lv2:index %d ;\n\
    lv2:symbol \"%s\" ;\n\
    lv2:name \"%s\" ;\n",
        i, symbol, name);

      if (unit[0] != '\0')
        {
          fprintf (f, "\
    units:unit units:%s ;\n", unit);
        }
      if (comment[0] != '\0')
        {
          fprintf (f,
"    rdfs:comment \"%s\" ;\n",
            comment);
        }

      if (type == PORT_TYPE_FLOAT ||
          type == PORT_TYPE_TOGGLE)
        {
          fprintf (f,
"    lv2:default %f ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n",
            (double) def, (double) min, (double) max);
        }
      else if (type == PORT_TYPE_INT ||
               type == PORT_TYPE_ENUM)
        {
          fprintf (f,
"    lv2:default %d ;\n\
    lv2:minimum %d ;\n\
    lv2:maximum %d ;\n",
            defi, mini, maxi);
        }

      if (is_trigger)
        {
          fprintf (f,
"    lv2:portProperty pprop:trigger ;\n");
        }
      else if (type == PORT_TYPE_INT)
        {
          fprintf (f,
"    lv2:portProperty lv2:integer ;\n");
        }
      else if (type == PORT_TYPE_TOGGLE)
        {
          fprintf (f,
"    lv2:portProperty lv2:toggled ;\n");
        }
      else if (type == PORT_TYPE_ENUM)
        {
          fprintf (f,
"    lv2:portProperty lv2:integer ;\n");
          fprintf (f,
"    lv2:portProperty lv2:enumeration ;\n");
          fprintf (f, "%s", scale_points);
        }

      fprintf (f,
"  ] , [\n");
    }

  /* write cv outs */
  fprintf (f,
"    a lv2:OutputPort ,\n\
      lv2:CVPort ;\n\
    lv2:index %d ;\n\
    lv2:symbol \"sine_out\" ;\n\
    lv2:name \"Sine\" ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:CVPort ;\n\
    lv2:index %d ;\n\
    lv2:symbol \"triangle_out\" ;\n\
    lv2:name \"Triangle\" ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:CVPort ;\n\
    lv2:index %d ;\n\
    lv2:symbol \"saw_out\" ;\n\
    lv2:name \"Saw\" ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:CVPort ;\n\
    lv2:index %d ;\n\
    lv2:symbol \"square_out\" ;\n\
    lv2:name \"Square\" ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] , [\n\
    a lv2:OutputPort ,\n\
      lv2:CVPort ;\n\
    lv2:index %d ;\n\
    lv2:symbol \"custom_out\" ;\n\
    lv2:name \"Custom\" ;\n\
    lv2:minimum %f ;\n\
    lv2:maximum %f ;\n\
  ] .\n\n",
    LFO_SINE_OUT, -1.0, 1.0,
    LFO_TRIANGLE_OUT, -1.0, 1.0,
    LFO_SAW_OUT, -1.0, 1.0,
    LFO_SQUARE_OUT,  -1.0, 1.0,
    LFO_CUSTOM_OUT, -1.0, 1.0);

  /* write UI */
  fprintf (f,
"<" PLUGIN_UI_URI ">\n\
  a ui:" PLUGIN_UI_TYPE " ;\n\
  lv2:requiredFeature urid:map ,\n\
                      ui:idleInterface ;\n\
  lv2:optionalFeature log:log ,\n\
                      ui:noUserResize ;\n\
  lv2:extensionData ui:idleInterface ,\n\
                    ui:showInterface ;\n\
  ui:portNotification [\n\
    ui:plugin <" PLUGIN_URI "> ;\n\
    lv2:symbol \"notify\" ;\n\
    ui:notifyType atom:Blank ;\n\
  ] .");
}
