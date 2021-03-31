/*
 * Copyright (C) 2019-2020 Alexandros Theodotou <alex at zrythm dot org>
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
  Copyright 2012-2019 David Robillard <http://drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include PLUGIN_CONFIG

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include PLUGIN_COMMON
#include "lfo_math.h"
#include "ui_theme.h"

#include <cairo.h>

#include <lv2/atom/util.h>
#include <lv2/ui/ui.h>

#include <ztoolkit/ztk.h>

/** Width and height of the window. */
#define WIDTH 480
#define HEIGHT 261

#define LEFT_BTN_WIDTH 40
#define TOP_BTN_HEIGHT 38
#define MID_REGION_WIDTH 394
#define MID_BTN_WIDTH 193
#define MID_REGION_HEIGHT 180
#define SYNC_RATE_BOX_WIDTH 46
#define SYNC_RATE_BOX_HEIGHT 16
#define FREQ_BOX_WIDTH 48
#define ARROW_BTN_WIDTH 15
#define RANGE_POINT_WIDTH 10
#define RANGE_HEIGHT 150
#define NODE_WIDTH 12
#define GRID_HPADDING 26
#define GRID_SPACE 42
#define GRID_WIDTH (8 * GRID_SPACE)
#define GRID_XSTART_GLOBAL \
  (LEFT_BTN_WIDTH + 4 + GRID_HPADDING)
#define GRID_XEND_GLOBAL \
  (LEFT_BTN_WIDTH + 4 + GRID_HPADDING + \
   8 * GRID_SPACE)
#define GRID_YSTART_OFFSET 46
#define GRID_YEND_OFFSET 164
#define GRID_YSTART_GLOBAL \
    (TOP_BTN_HEIGHT + 2 + GRID_YSTART_OFFSET)
#define GRID_YEND_GLOBAL \
    (TOP_BTN_HEIGHT + 2 + GRID_YEND_OFFSET)
#define GRID_HEIGHT \
  (GRID_YEND_OFFSET - GRID_YSTART_OFFSET)
#define RANGE_STARTX 461

#define GRAPH_OVERLAY_ALPHA 0.6

/** Double click interval, in seconds. */
#define DOUBLE_CLICK_INTERVAL 0.24

#define GET_HANDLE \
  LfoUi * self = (LfoUi *) puglGetHandle (view);

typedef struct ZtkApp ZtkApp;

typedef enum LeftButton
{
  LEFT_BTN_SINE,
  LEFT_BTN_TRIANGLE,
  LEFT_BTN_SAW,
  LEFT_BTN_SQUARE,
  LEFT_BTN_CUSTOM,
  NUM_LEFT_BUTTONS,
} LeftButton;

typedef enum TopButton
{
  TOP_BTN_CURVE,
  TOP_BTN_STEP,
  NUM_TOP_BUTTONS,
} TopButton;

typedef enum BotButton
{
  BOT_BTN_SYNC,
  BOT_BTN_FREE,
  NUM_BOT_BUTTONS,
} BotButton;

typedef enum GridButton
{
  GRID_BTN_SNAP,
  GRID_BTN_HMIRROR,
  GRID_BTN_VMIRROR,
  NUM_GRID_BUTTONS,
} GridButton;

typedef enum LabelType
{
  LBL_TYPE_INVERT,
  LBL_TYPE_SHIFT,
  NUM_LBL_TYPES,
} LabelType;

typedef enum DrawDataType
{
  DATA_TYPE_BTN_TOP,
  DATA_TYPE_BTN_LEFT,
  DATA_TYPE_BTN_BOT,
  DATA_TYPE_BTN_GRID,
  DATA_TYPE_LBL,
} DrawDataType;

typedef struct LfoUi
{
  /** Port values. */
  float            gate;
  int              trigger;
  float            cv_gate;
  float            cv_trigger;
  float            freq;
  float            shift;
  float            range_min;
  float            range_max;
  int              step_mode;
  int              freerun;
  int              hinvert;
  int              vinvert;
  int              sine_on;
  int              saw_on;
  int              square_on;
  int              triangle_on;
  int              custom_on;
  float            sync_rate;
  float            sync_rate_type;
  float            grid_step;
  float            nodes[16][3];
  int              num_nodes;

  /* Non-port values */

  LfoCommon       common;

  LV2UI_Write_Function write;
  LV2UI_Controller controller;

  /**
   * This is the window passed in the features from
   * the host.
   *
   * The pugl window will be wrapped in here.
   */
  void *           parent_window;

  /**
   * Resize handle for the parent window.
   */
  LV2UI_Resize*    resize;

  /** Pointer to the mid region widget, to use
   * for redisplaying only its rect. */
  ZtkWidget *      mid_region;

  /** Widgets for the current nodes. */
  ZtkWidget *      node_widgets[16];

  /** Cache to remember when last double click
   * occured, so that it can be ignored. */
  double           last_double_click;

  /** Timestamp of last delete click, so we
   * don't delete more than once. */
  double           last_delete_click;

  /** Index of the current node being dragged,
   * or -1. */
  int              dragging_node;

  /** This is double here so we can be more
   * precise with calculations. */
  double           current_sample;

  /** Last time the current sample was set at. */
  gint64           last_current_sample_set;

  /** Whether we need to recalculate the caches. */
  int              has_change;

  ZtkRect          last_rect;

  /** Caches. */
  float            sine_cache[GRID_WIDTH];
  float            saw_cache[GRID_WIDTH];

  char             bundle_path[2000];

  LfoUiTheme      ui_theme;

  cairo_t *        cached_cr;
  cairo_surface_t * cached_surface;

  ZtkApp *         app;
} LfoUi;

/**
 * Data to be passed around in the callbacks.
 */
typedef struct DrawData
{
  /** Enum value corresponding to the type. */
  int            val;
  DrawDataType   type;
  LfoUi *       zlfo_ui;
} DrawData;

#define SEND_PORT_EVENT(_self,idx,val) \
  { \
    float fval = (float) val; \
    _self->write ( \
       _self->controller, (uint32_t) idx, \
       sizeof (float), 0, &fval); \
  }

#define GENERIC_GETTER(sc) \
static float \
sc##_getter ( \
  ZtkControl * control, \
  LfoUi *     self) \
{ \
  return self->sc; \
}

#define DEFINE_GET_SET(caps,sc) \
GENERIC_GETTER (sc); \
static void \
sc##_setter ( \
  ZtkControl * control, \
  LfoUi *     self, \
  float        val) \
{ \
  self->sc = val; \
  ztk_debug ( \
    "setting " #sc " to %f", (double) val); \
  SEND_PORT_EVENT (self, LFO_##caps, self->sc); \
  self->has_change = 1; \
}

DEFINE_GET_SET (SHIFT, shift);
DEFINE_GET_SET (SYNC_RATE, sync_rate);
DEFINE_GET_SET (FREQ, freq);
DEFINE_GET_SET (RANGE_MIN, range_min);
DEFINE_GET_SET (RANGE_MAX, range_max);

#undef GENERIC_GETTER
#undef DEFINE_GET_SET

static void
node_pos_setter (
  LfoUi *     self,
  unsigned int idx,
  float        val)
{
  self->nodes[idx][0] = val;
  SEND_PORT_EVENT (
    self, LFO_NODE_1_POS + idx * 3, val);
  self->has_change = 1;
}

static void
node_val_setter (
  LfoUi *     self,
  unsigned int idx,
  float        val)
{
  self->nodes[idx][1] = val;
  SEND_PORT_EVENT (
    self, LFO_NODE_1_VAL + idx * 3, val);
  self->has_change = 1;
}

/**
 * Resets a surface and cairo_t with a new surface
 * and cairo_t based on the given rectangle and
 * cairo_t.
 *
 * To be used inside draw calls of widgets that
 * use caching.
 */
static void
z_cairo_reset_caches (
  cairo_t **         cr_cache,
  cairo_surface_t ** surface_cache,
  int                width,
  int                height,
  cairo_t *          new_cr)
{
  if (*surface_cache)
    cairo_surface_destroy (
      *surface_cache);
  if (*cr_cache)
    cairo_destroy (*cr_cache);

  *surface_cache =
    cairo_surface_create_similar (
      cairo_get_target (new_cr),
      CAIRO_CONTENT_COLOR_ALPHA,
      width, height);
  *cr_cache =
    cairo_create (*surface_cache);
}

static void
num_nodes_setter (
  LfoUi *     self,
  int          val)
{
  self->num_nodes = val;
  SEND_PORT_EVENT (
    self, LFO_NUM_NODES, val);
  self->has_change = 1;
}

static void
bg_draw_cb (
  ZtkWidget * widget,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  LfoUi *    self)
{
  /* clear background to black first */
  cairo_set_source_rgba (cr, 0, 0, 0, 1);
  cairo_rectangle (
    cr, widget->rect.x, widget->rect.y,
    widget->rect.width, widget->rect.height);
  cairo_fill (cr);

  /* set theme background */
  zlfo_ui_theme_set_cr_color (
    &self->ui_theme, cr, bg);
  cairo_rectangle (
    cr, widget->rect.x, widget->rect.y,
    widget->rect.width, widget->rect.height);
  cairo_fill (cr);
}

static void
add_bg_widget (
  LfoUi * self)
{
  ZtkRect rect = {
    0, 0, self->app->width, self->app->height };
  ZtkDrawingArea * da =
    ztk_drawing_area_new (
      &rect, NULL,
      (ZtkWidgetDrawCallback) bg_draw_cb,
      NULL, self);
  ztk_app_add_widget (
    self->app, (ZtkWidget *) da, 0);
}

typedef struct ComboBoxElement
{
  int           id;
  char          label[600];
  ZtkComboBox * combo;
  LfoUi *      zlfo_ui;
} ComboBoxElement;

static void
sync_rate_type_activate_cb (
  ZtkWidget *       widget,
  ComboBoxElement * el)
{
  ztk_debug (
    "activate %p %d %s", el->combo,
    el->id, el->label);
  LfoUi * self = el->zlfo_ui;
  self->sync_rate_type = (float) el->id;
  SEND_PORT_EVENT (
    self, LFO_SYNC_RATE_TYPE,
    self->sync_rate_type);
  self->has_change = 1;
}

static void
grid_step_activate_cb (
  ZtkWidget *       widget,
  ComboBoxElement * el)
{
  /*ztk_debug (*/
    /*"activate %p %d %s", el->combo,*/
    /*el->id, el->label);*/
  LfoUi * self = el->zlfo_ui;
  self->grid_step = (float) el->id;
  SEND_PORT_EVENT (
    self, LFO_GRID_STEP, self->grid_step);
  self->has_change = 1;
}

/**
 * Called when one of the buttons was clicked.
 */
static void
on_btn_clicked (
  ZtkWidget * widget,
  DrawData *  data)
{
  /*ztk_debug ("Button clicked!");*/

  LfoUi * self = data->zlfo_ui;
  switch (data->type)
    {
    case DATA_TYPE_BTN_TOP:
      switch (data->val)
        {
        case TOP_BTN_CURVE:
          self->step_mode = 0;
          SEND_PORT_EVENT (
            self, LFO_STEP_MODE, self->step_mode);
          self->has_change = 1;
          break;
        case TOP_BTN_STEP:
          self->step_mode = 1;
          SEND_PORT_EVENT (
            self, LFO_STEP_MODE, self->step_mode);
          self->has_change = 1;
          break;
        }
      break;
    case DATA_TYPE_BTN_LEFT:
      switch (data->val)
        {
#define HANDLE_BTN(caps,lowercase) \
  case LEFT_BTN_##caps: \
    if (self->lowercase##_on) \
      { \
        self->lowercase##_on = 0; \
        SEND_PORT_EVENT ( \
          self, LFO_##caps##_TOGGLE, 0.f); \
        self->has_change = 1; \
      } \
    else \
      { \
        self->lowercase##_on = 1; \
        SEND_PORT_EVENT ( \
          self, LFO_##caps##_TOGGLE, 1.f); \
        self->has_change = 1; \
      } \
    break
          HANDLE_BTN (SINE, sine);
          HANDLE_BTN (SAW, saw);
          HANDLE_BTN (TRIANGLE, triangle);
          HANDLE_BTN (SQUARE, square);
          HANDLE_BTN (CUSTOM, custom);
          self->has_change = 1;
        default:
          break;
        }
      break;
    case DATA_TYPE_BTN_BOT:
      switch (data->val)
        {
        case BOT_BTN_SYNC:
          self->freerun = 0;
          SEND_PORT_EVENT (
            self, LFO_FREE_RUNNING, self->freerun);
          self->has_change = 1;
          break;
        case BOT_BTN_FREE:
          self->freerun = 1;
          SEND_PORT_EVENT (
            self, LFO_FREE_RUNNING, self->freerun);
          self->has_change = 1;
          break;
        }
      break;
    case DATA_TYPE_BTN_GRID:
      switch (data->val)
        {
        case GRID_BTN_SNAP:
          {
            ZtkComboBox * combo =
              ztk_combo_box_new (
                widget, 0, 0);
            ztk_app_add_widget (
              widget->app, (ZtkWidget *) combo,
              100);
            for (int i = 0; i < NUM_GRID_STEPS; i++)
              {
                ComboBoxElement * el =
                  calloc (
                    1, sizeof (ComboBoxElement));
                el->id = i;
                el->combo = combo;
                el->zlfo_ui = self;
                switch (i)
                  {
                  case GRID_STEP_FULL:
                    strcpy (el->label, "full");
                    break;
                  case GRID_STEP_HALF:
                    strcpy (el->label, "1/2");
                    break;
                  case GRID_STEP_FOURTH:
                    strcpy (el->label, "1/4");
                    break;
                  case GRID_STEP_EIGHTH:
                    strcpy (el->label, "1/8");
                    break;
                  case GRID_STEP_SIXTEENTH:
                    strcpy (el->label, "1/16");
                    break;
                  case GRID_STEP_THIRTY_SECOND:
                    strcpy (el->label, "1/32");
                    break;
                  default:
                    break;
                  }
                ztk_combo_box_add_text_element (
                  combo, el->label,
                  (ZtkWidgetActivateCallback)
                  grid_step_activate_cb, el);
              }
          }
          break;
        case GRID_BTN_HMIRROR:
          self->hinvert = !self->hinvert;
          SEND_PORT_EVENT (
            self, LFO_HINVERT, self->hinvert);
          self->has_change = 1;
          break;
        case GRID_BTN_VMIRROR:
          self->vinvert = !self->vinvert;
          SEND_PORT_EVENT (
            self, LFO_VINVERT, self->vinvert);
          self->has_change = 1;
          break;
        }
      break;
    default:
      break;
    }
}

static void
on_sync_rate_type_clicked (
  ZtkWidget * widget,
  LfoUi *    self)
{
  ZtkComboBox * combo =
    ztk_combo_box_new (
      widget, 1, 0);
  ztk_app_add_widget (
    widget->app, (ZtkWidget *) combo, 100);

  for (int i = 0; i < NUM_SYNC_RATE_TYPES; i++)
    {
      ComboBoxElement * el =
        calloc (1, sizeof (ComboBoxElement));
      el->id = i;
      el->combo = combo;
      el->zlfo_ui = self;
      switch (i)
        {
        case SYNC_TYPE_NORMAL:
          strcpy (el->label, "normal");
          break;
        case SYNC_TYPE_DOTTED:
          strcpy (el->label, "dotted");
          break;
        case SYNC_TYPE_TRIPLET:
          strcpy (el->label, "triplet");
          break;
        default:
          break;
        }
      ztk_combo_box_add_text_element (
        combo, el->label,
        (ZtkWidgetActivateCallback)
        sync_rate_type_activate_cb, el);
    }
}

static int
get_button_active (
  ZtkButton * btn,
  DrawData *  data)
{
  LfoUi * self = data->zlfo_ui;

  switch (data->type)
    {
    case DATA_TYPE_BTN_TOP:
      switch (data->val)
        {
        case TOP_BTN_CURVE:
          return !self->step_mode;
          break;
        case TOP_BTN_STEP:
          return self->step_mode;
          break;
        }
      break;
    case DATA_TYPE_BTN_LEFT:
      switch (data->val)
        {
        case LEFT_BTN_SINE:
          return self->sine_on;
          break;
        case LEFT_BTN_TRIANGLE:
          return self->triangle_on;
          break;
        case LEFT_BTN_SAW:
          return self->saw_on;
          break;
        case LEFT_BTN_SQUARE:
          return self->square_on;
          break;
        case LEFT_BTN_CUSTOM:
          return self->custom_on;
          break;
        default:
          break;
        }
      break;
    case DATA_TYPE_BTN_BOT:
      switch (data->val)
        {
        case BOT_BTN_SYNC:
          return !self->freerun;
          break;
        case BOT_BTN_FREE:
          return self->freerun;
          break;
        }
      break;
    case DATA_TYPE_BTN_GRID:
      switch (data->val)
        {
        case GRID_BTN_HMIRROR:
          return self->hinvert;
          break;
        case GRID_BTN_VMIRROR:
          return self->vinvert;
          break;
        }
      break;
    case DATA_TYPE_LBL:
      break;
    }

  return 0;
}

static void
add_left_buttons (
  LfoUi * self)
{
  const int padding = 2;
  const int width = LEFT_BTN_WIDTH;
  const int height = 50;
  for (int i = 0; i < NUM_LEFT_BUTTONS; i++)
    {
      ZtkRect rect = {
        padding, padding + i * (height + padding),
        width, height };
      DrawData * data =
        calloc (1, sizeof (DrawData));
      data->val = i;
      data->type = DATA_TYPE_BTN_LEFT;
      data->zlfo_ui = self;
      ZtkButton * btn =
        ztk_button_new (
          &rect,
          (ZtkWidgetActivateCallback)
          on_btn_clicked, data);
      ztk_button_make_toggled (
        btn,
        (ZtkButtonToggledGetter)
        get_button_active);
      ztk_button_set_background_colors (
        btn,
        &self->ui_theme.button_normal,
        &self->ui_theme.button_hover,
        &self->ui_theme.left_button_click);

#define MAKE_BUTTON_SVGED(caps,lowercase) \
  case LEFT_BTN_##caps: \
    { \
      ztk_button_make_svged (\
        btn, hpadding, vpadding, \
        self->ui_theme.lowercase##_svg, \
        self->ui_theme.lowercase##_svg, \
        self->ui_theme.lowercase##_svg); \
    } \
    break

      int hpadding = 8;
      int vpadding = 4;
      switch (data->val)
        {
          MAKE_BUTTON_SVGED (SINE, sine);
          MAKE_BUTTON_SVGED (TRIANGLE, triangle);
          MAKE_BUTTON_SVGED (SAW, saw);
          MAKE_BUTTON_SVGED (SQUARE, square);
          MAKE_BUTTON_SVGED (CUSTOM, custom);
        }

#undef MAKE_BUTTON_SVGED

      ztk_app_add_widget (
        self->app, (ZtkWidget *) btn, 1);
    }
}

static void
top_and_bot_btn_bg_cb (
  ZtkWidget * w,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  DrawData *  data)
{
  LfoUi * self = data->zlfo_ui;

  /* set background */
  ZtkWidgetState state = w->state;
  int is_normal = 0;
  int is_pressed = 0;
  int is_hovered = 0;
  if (state & ZTK_WIDGET_STATE_PRESSED ||
      get_button_active ((ZtkButton *) w, data))
    {
      zlfo_ui_theme_set_cr_color (
        &self->ui_theme, cr, selected_bg);
      is_pressed = 1;
    }
  else if (state & ZTK_WIDGET_STATE_HOVERED)
    {
      zlfo_ui_theme_set_cr_color (
        &self->ui_theme, cr, button_hover);
      is_hovered = 1;
    }
  else
    {
      zlfo_ui_theme_set_cr_color (
        &self->ui_theme, cr, button_normal);
      is_normal = 1;
    }

  if (data->type == DATA_TYPE_BTN_TOP)
    {
      double height_with_border =
        is_normal ?
          /* show border if normal */
          w->rect.height - 2 :
          w->rect.height + 1;

      cairo_rectangle (
        cr, w->rect.x, w->rect.y, w->rect.width,
        height_with_border);
      cairo_fill (cr);

      /* draw line on bot */
      if (is_pressed)
        {
          zlfo_ui_theme_set_cr_color (
            &self->ui_theme, cr, button_lining_active);
        }
      else if (is_hovered)
        {
          zlfo_ui_theme_set_cr_color (
            &self->ui_theme, cr, button_lining_hover);
        }
      cairo_rectangle (
        cr, w->rect.x,
        w->rect.y + height_with_border - 4, w->rect.width,
        4);
      cairo_fill (cr);
    }
  else if (data->type == DATA_TYPE_BTN_BOT)
    {
      double y_with_border =
        is_normal ?
          /* show border if normal */
          w->rect.y :
          w->rect.y - 3;
      double height_with_border =
        is_normal ?
          /* show border if normal */
          w->rect.height :
          w->rect.height + 3;

      cairo_rectangle (
        cr, w->rect.x,
        y_with_border,
        w->rect.width,
        height_with_border);
      cairo_fill (cr);

      /* draw line on top */
      if (is_pressed)
        {
          zlfo_ui_theme_set_cr_color (
            &self->ui_theme, cr, button_lining_active);
        }
      else if (is_hovered)
        {
          zlfo_ui_theme_set_cr_color (
            &self->ui_theme, cr, button_lining_hover);
        }
      cairo_rectangle (
        cr, w->rect.x,
        y_with_border, w->rect.width,
        4);
      cairo_fill (cr);
    }

  if (data->type == DATA_TYPE_BTN_BOT)
    {

#define DRAW_SVG(caps,svg) \
  case BOT_BTN_##caps: \
  { \
    int hpadding = 0; \
    int vpadding = 0; \
    ZtkRect rect = { \
      (w->rect.x + hpadding), \
      w->rect.y + vpadding, \
      w->rect.width - hpadding * 2, \
      w->rect.height - vpadding * 2 }; \
    if (data->val == BOT_BTN_SYNC) \
      { \
        rect.x -=  \
          (SYNC_RATE_BOX_WIDTH + \
             ARROW_BTN_WIDTH) / 2.0; \
      } \
    else if (data->val == BOT_BTN_FREE) \
      { \
        rect.x -= FREQ_BOX_WIDTH / 2.0; \
      } \
    ztk_rsvg_draw ( \
      self->ui_theme.svg##_svg, cr, &rect); \
  } \
      break

      switch (data->val)
        {
          DRAW_SVG (SYNC, sync);
          DRAW_SVG (FREE, freeb);
        }

#undef DRAW_SVG
    }
}

static void
add_top_buttons (
  LfoUi * self)
{
  const int padding = 2;
  const int width = MID_BTN_WIDTH;
  const int height = TOP_BTN_HEIGHT;
  const int start = LEFT_BTN_WIDTH + padding;
  for (int i = 0; i < NUM_TOP_BUTTONS; i++)
    {
      ZtkRect rect = {
        start + padding + i * (width + padding),
        padding, width, height };
      DrawData * data =
        calloc (1, sizeof (DrawData));
      data->val = i;
      data->type = DATA_TYPE_BTN_TOP;
      data->zlfo_ui = self;
      ZtkButton * btn =
        ztk_button_new (
          &rect,
          (ZtkWidgetActivateCallback)
          on_btn_clicked, data);
      ztk_button_add_background_callback (
        btn,
        (ZtkWidgetDrawCallback)
        top_and_bot_btn_bg_cb);
      ztk_button_make_toggled (
        btn,
        (ZtkButtonToggledGetter)
        get_button_active);

#define MAKE_BUTTON_SVGED(caps,lowercase) \
  case TOP_BTN_##caps: \
    { \
      ztk_button_make_svged (\
        btn, hpadding, vpadding, \
        self->ui_theme.lowercase##_svg, \
        self->ui_theme.lowercase##_svg, \
        self->ui_theme.lowercase##_svg); \
    } \
    break

      int hpadding = 6;
      int vpadding = 6;
      switch (data->val)
        {
          MAKE_BUTTON_SVGED (CURVE, curve);
          MAKE_BUTTON_SVGED (STEP, step);
        }

#undef MAKE_BUTTON_SVGED

      ztk_app_add_widget (
        self->app, (ZtkWidget *) btn, 1);
    }
}

static void
sync_rate_control_draw_cb (
  ZtkWidget * widget,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  LfoUi *    self)
{
  /* draw black bg */
  cairo_set_source_rgba (
    cr, 0, 0, 0, 1);
  cairo_rectangle (
    cr, widget->rect.x, widget->rect.y,
    widget->rect.width, widget->rect.height);
  cairo_fill (cr);

  /* draw label */
  int val = (int) self->sync_rate;
  char lbl[12] = "\0";
  switch (val)
    {
    case SYNC_1_128:
      strcpy (lbl, "1 / 128");
      break;
    case SYNC_1_64:
      strcpy (lbl, "1 / 64");
      break;
    case SYNC_1_32:
      strcpy (lbl, "1 / 32");
      break;
    case SYNC_1_16:
      strcpy (lbl, "1 / 16");
      break;
    case SYNC_1_8:
      strcpy (lbl, "1 / 8");
      break;
    case SYNC_1_4:
      strcpy (lbl, "1 / 4");
      break;
    case SYNC_1_2:
      strcpy (lbl, "1 / 2");
      break;
    case SYNC_1_1:
      strcpy (lbl, "1 / 1");
      break;
    case SYNC_2_1:
      strcpy (lbl, "2 / 1");
      break;
    case SYNC_4_1:
      strcpy (lbl, "4 / 1");
      break;
    case SYNC_8_1:
      strcpy (lbl, "8 / 1");
      break;
    case SYNC_16_1:
      strcpy (lbl, "16 / 1");
      break;
    case SYNC_32_1:
      strcpy (lbl, "32 / 1");
      break;
    case SYNC_64_1:
      strcpy (lbl, "64 / 1");
      break;
    case SYNC_128_1:
      strcpy (lbl, "128 / 1");
      break;
      break;
    }
  switch ((int) self->sync_rate_type)
    {
    case SYNC_TYPE_DOTTED:
      strcat (lbl, ".");
      break;
    case SYNC_TYPE_TRIPLET:
      strcat (lbl, "t");
      break;
    default:
      break;
    }

  /* Draw label */
  cairo_text_extents_t extents;
  cairo_set_font_size (cr, 10);
  cairo_text_extents (cr, lbl, &extents);
  cairo_move_to (
    cr,
    (widget->rect.x + widget->rect.width / 2.0) -
      (extents.width / 2.0 + 1.0),
    (widget->rect.y + widget->rect.height) -
      widget->rect.height / 2.0 +
        extents.height / 2.0);
  cairo_set_source_rgba (cr, 1, 1, 1, 1);
  cairo_show_text (cr, lbl);
}

static void
freq_control_draw_cb (
  ZtkWidget * widget,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  LfoUi *    self)
{
  /* draw black bg */
  cairo_set_source_rgba (
    cr, 0, 0, 0, 1);
  cairo_rectangle (
    cr, widget->rect.x, widget->rect.y,
    widget->rect.width, widget->rect.height);
  cairo_fill (cr);

  /* draw label */
  char lbl[12];
  if (self->freq < 1.f)
    sprintf (lbl, "%.2f Hz", (double) self->freq);
  else
    sprintf (lbl, "%.1f Hz", (double) self->freq);

  /* Draw label */
  cairo_text_extents_t extents;
  cairo_set_font_size (cr, 10);
  cairo_text_extents (cr, lbl, &extents);
  cairo_move_to (
    cr,
    (widget->rect.x + widget->rect.width / 2.0) -
      (extents.width / 2.0 + 1.0),
    (widget->rect.y + widget->rect.height) -
      widget->rect.height / 2.0 +
        extents.height / 2.0);
  cairo_set_source_rgba (cr, 1, 1, 1, 1);
  cairo_show_text (cr, lbl);
}

static int
sync_rate_control_btn_event_cb (
  ZtkWidget *             w,
  const PuglEventButton * btn,
  LfoUi *                self)
{
  ZtkWidgetState state = w->state;
  if (state & ZTK_WIDGET_STATE_PRESSED)
    {
      if (ztk_widget_is_hit (w, btn->x, btn->y))
        {
          self->freerun = 0;
          SEND_PORT_EVENT (
            self, LFO_FREE_RUNNING, self->freerun);
          self->has_change = 1;
        }
    }
  return 1;
}

static int
freq_control_btn_event_cb (
  ZtkWidget *             w,
  const PuglEventButton * btn,
  LfoUi *                self)
{
  ZtkWidgetState state = w->state;
  if (state & ZTK_WIDGET_STATE_PRESSED)
    {
      if (ztk_widget_is_hit (w, btn->x, btn->y))
        {
          self->freerun = 1;
          SEND_PORT_EVENT (
            self, LFO_FREE_RUNNING, self->freerun);
          self->has_change = 1;
        }
    }
  return 1;
}

static void
add_bot_buttons (
  LfoUi * self)
{
  const int padding = 2;
  const int width = MID_BTN_WIDTH;
  const int height = TOP_BTN_HEIGHT;
  const int start = LEFT_BTN_WIDTH + padding;
  for (int i = 0; i < NUM_BOT_BUTTONS; i++)
    {
      ZtkRect rect = {
        start + padding + i * (width + padding),
        TOP_BTN_HEIGHT + 4 + MID_REGION_HEIGHT,
        width, height };
      DrawData * data =
        calloc (1, sizeof (DrawData));
      data->val = i;
      data->type = DATA_TYPE_BTN_BOT;
      data->zlfo_ui = self;
      ZtkButton * btn =
        ztk_button_new (
          &rect,
          (ZtkWidgetActivateCallback)
          on_btn_clicked, data);
      ztk_button_add_background_callback (
        btn,
        (ZtkWidgetDrawCallback)
        top_and_bot_btn_bg_cb);
      ztk_button_make_toggled (
        btn,
        (ZtkButtonToggledGetter)
        get_button_active);

      ztk_app_add_widget (
        self->app, (ZtkWidget *) btn, 1);
    }

  /* add sync rate control */
  ZtkRect rect = {
    (start + padding + width / 2) -
      ARROW_BTN_WIDTH / 2,
    MID_REGION_HEIGHT + TOP_BTN_HEIGHT + 14,
    SYNC_RATE_BOX_WIDTH, SYNC_RATE_BOX_HEIGHT };
  ZtkControl * control =
    ztk_control_new (
      &rect,
      (ZtkControlGetter) sync_rate_getter,
      (ZtkControlSetter) sync_rate_setter,
      (ZtkWidgetDrawCallback)
      sync_rate_control_draw_cb,
      ZTK_CTRL_DRAG_VERTICAL,
      self, 0.f, (float) (NUM_SYNC_RATES - 1),
      0.0f);
  ((ZtkWidget *) control)->user_data = self;
  control->sensitivity = 0.008f;
  ((ZtkWidget *) control)->button_event_cb =
    (ZtkWidgetButtonEventCallback)
    sync_rate_control_btn_event_cb;
  ztk_app_add_widget (
    self->app, (ZtkWidget *) control, 2);

  /* add sync rate type dropdown */
  rect.x =
    (start + padding + width / 2 +
       SYNC_RATE_BOX_WIDTH) -
    (ARROW_BTN_WIDTH / 2 + 1);
  rect.y =
    MID_REGION_HEIGHT + TOP_BTN_HEIGHT + 14;
  rect.width = ARROW_BTN_WIDTH;
  rect.height = SYNC_RATE_BOX_HEIGHT;
  ZtkButton * btn =
    ztk_button_new (
      &rect,
      (ZtkWidgetActivateCallback)
      on_sync_rate_type_clicked, self);
  ZtkColor bg = { 0, 0, 0, 1 };
  ztk_button_set_background_colors (
    btn,
    &bg,
    &self->ui_theme.button_hover,
    &self->ui_theme.bright_click);
  ztk_button_make_svged (
    btn, 3, 0,
    self->ui_theme.down_arrow_svg,
    self->ui_theme.down_arrow_svg,
    self->ui_theme.down_arrow_svg);
  ztk_app_add_widget (
    self->app, (ZtkWidget *) btn, 4);

  /* add frequency control */
  rect.x =
    start + padding + width + padding + width / 2;
  rect.y =
    MID_REGION_HEIGHT + TOP_BTN_HEIGHT + 14;
  rect.width = FREQ_BOX_WIDTH;
  rect.height = SYNC_RATE_BOX_HEIGHT;
  control =
    ztk_control_new (
      &rect,
      (ZtkControlGetter) freq_getter,
      (ZtkControlSetter) freq_setter,
      (ZtkWidgetDrawCallback)
      freq_control_draw_cb,
      ZTK_CTRL_DRAG_VERTICAL,
      self, MIN_FREQ, MAX_FREQ, MIN_FREQ);
  ((ZtkWidget *) control)->user_data = self;
  control->sensitivity = 0.00096f;
  ((ZtkWidget *) control)->button_event_cb =
    (ZtkWidgetButtonEventCallback)
    freq_control_btn_event_cb;
  ztk_app_add_widget (
    self->app, (ZtkWidget *) control, 2);
}

/**
 * Draws the graphs in curve mode.
 */
static void
draw_graph (
  LfoUi *  self,
  cairo_t * cr)
{
  double max_range =
    MAX (self->range_max, self->range_min);
  double min_range =
    MIN (self->range_max, self->range_min);
  double range = max_range - min_range;

  double grid_step_divisor =
    (double)
    grid_step_to_divisor (
      (GridStep) self->grid_step);
  double step_px = GRID_WIDTH / grid_step_divisor;

  /* sort node curves by position */
  NodeIndexElement node_indices[self->num_nodes];
  sort_node_indices_by_pos (
    self->nodes, node_indices,
    self->num_nodes);

  if (self->has_change)
    {
      for (int i = 0; i < GRID_WIDTH; i++)
        {
          self->sine_cache[i] =
            sinf (
              ((float) i * 2.f * PI) /
              (float) GRID_WIDTH);
          self->saw_cache[i] =
            (1.f - (float) i / (float) GRID_WIDTH) *
            2.f - 1.f;
        }
      /*g_message ("has change");*/
    }

  cairo_set_source_rgba (
    cr, self->ui_theme.left_button_click.red,
    self->ui_theme.left_button_click.green,
    self->ui_theme.left_button_click.blue,
    GRAPH_OVERLAY_ALPHA);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
  if (self->step_mode)
    cairo_set_line_width (cr, step_px);
  else
    cairo_set_line_width (cr, 6);
  double prev_draw_sine,
         prev_draw_triangle,
         prev_draw_saw, prev_draw_square,
         prev_draw_custom;
  int i = 0;
  double idouble = 0;
  if (self->step_mode)
    {
      idouble = step_px / 2.0;
      i = (int) idouble;
    }
   /* we are approximating so be sure it
    * doesn't go beyond the width by small
    * decimals */
  while (idouble < GRID_WIDTH - 0.01)
    {
      /* from 0 to GRID_WIDTH */
      long xvall = (long) i;
      double xvald = (double) i;

      xvall =
        invert_and_shift_xval (
          i, GRID_WIDTH, self->hinvert,
          self->shift);
      xvald = (double) xvall;

#define DRAW_VAL(val) \
  /* invert vertically */ \
  if (self->vinvert) \
    { \
      val = - val; \
    } \
 \
  /* adjust range */ \
  val = \
    min_range + \
  ((val + 1.0) / 2.0) * range; \
 \
  double draw_val = \
    ((val + 1.0) * GRID_HEIGHT) / 2.0; \
 \
  /* invert because higher Y means lower \
   * in cairo */ \
  draw_val = GRID_HEIGHT - draw_val; \
 \
  if (self->step_mode) \
    { \
      /* draw line */ \
      cairo_move_to ( \
        cr, \
        GRID_XSTART_GLOBAL + idouble, \
        GRID_YSTART_GLOBAL + GRID_HEIGHT); \
      cairo_line_to ( \
        cr, \
        GRID_XSTART_GLOBAL + idouble, \
        GRID_YSTART_GLOBAL + draw_val); \
      cairo_stroke (cr); \
    } \
  else if (i != 0) \
    { \
      /* draw line */ \
      cairo_move_to ( \
        cr, \
        GRID_XSTART_GLOBAL + i - 1, \
        GRID_YSTART_GLOBAL + \
          prev_draw_##val); \
      cairo_line_to ( \
        cr, \
        GRID_XSTART_GLOBAL + i, \
        GRID_YSTART_GLOBAL + \
          draw_val); \
      cairo_stroke (cr); \
    } \
  prev_draw_##val = draw_val

      double ratio = xvald / GRID_WIDTH;

      if (self->sine_on)
        {
          /* calculate sine */
          double sine =
            (double) self->sine_cache[xvall];

          DRAW_VAL (sine);
        }
      if (self->saw_on)
        {
          /* calculate saw */
          double saw =
            (double) self->saw_cache[xvall];

          DRAW_VAL (saw);
        }
      if (self->triangle_on)
        {
          double triangle;
          if (ratio > 0.4999)
            {
              triangle =
                (1.0 - ratio) * 4.0 - 1.0;
            }
          else
            {
              triangle =
                ratio * 4.0 - 1.0;
            }

          DRAW_VAL (triangle);
        }
      if (self->square_on)
        {
          double square;
          if (ratio > 0.4999)
            {
              square = - 1.0;
            }
          else
            {
              square = 1.0;
            }

          DRAW_VAL (square);
        }
      if (self->custom_on)
        {
          int prev_idx =
            get_prev_idx (
              node_indices, self->num_nodes,
              (float) ratio);
          int next_idx =
            get_next_idx (
              node_indices, self->num_nodes,
              (float) ratio);

          /* calculate custom */
          double custom =
            (double)
            get_custom_val_at_x (
              self->nodes[prev_idx][0],
              self->nodes[prev_idx][1],
              self->nodes[prev_idx][2],
              next_idx < 0 ? 1.f :
                self->nodes[next_idx][0],
              next_idx < 0 ?
                self->nodes[0][1] :
                self->nodes[next_idx][1],
              next_idx < 0 ?
                self->nodes[0][2] :
                self->nodes[next_idx][2],
              (float) xvald, GRID_WIDTH);

          /* adjust for -1 to 1 */
          custom = custom * 2 - 1;

          DRAW_VAL (custom);
        }

      if (self->step_mode)
        {
          idouble += step_px;
          i = (int) idouble;
        }
      else
        {
          i++;
          idouble = (double) i;
        }
    }
#undef DRAW_VAL

  if (self->custom_on)
    {
      /* draw node curves */
      zlfo_ui_theme_set_cr_color (&self->ui_theme, cr, line);
      cairo_set_line_width (cr, 6);
      for (i = 0; i < self->num_nodes - 1; i++)
        {
          int index = node_indices[i].index;
          int next_index =
            node_indices[i + 1].index;
          ZtkWidget * nodew =
            self->node_widgets[index];
          ZtkWidget * next_nodew =
            self->node_widgets[next_index];

          cairo_move_to (
            cr,
            nodew->rect.x + nodew->rect.width / 2,
            nodew->rect.y + nodew->rect.height / 2);
          cairo_line_to (
            cr,
            next_nodew->rect.x +
              next_nodew->rect.width / 2,
            next_nodew->rect.y +
              next_nodew->rect.height / 2);
          cairo_stroke (cr);
        }

      /* draw line from last node to first node
       * reappearing at the end */
      ZtkWidget * first_nodew =
        self->node_widgets[0];
      ZtkWidget * last_nodew =
        self->node_widgets[
          node_indices[self->num_nodes - 1].index];
      ZtkRect rect = first_nodew->rect;
      rect.x = GRID_XEND_GLOBAL - rect.width / 2;

      /* draw line */
      cairo_move_to (
        cr,
        last_nodew->rect.x +
          last_nodew->rect.width / 2,
        last_nodew->rect.y +
          last_nodew->rect.height / 2);
      cairo_line_to (
        cr,
        rect.x + rect.width / 2,
        rect.y + rect.height / 2);
      cairo_stroke (cr);

      /* draw faded end node */
      zlfo_ui_theme_set_cr_color (&self->ui_theme, cr, selected_bg);
      cairo_arc (
        cr,
        rect.x + rect.width / 2,
        rect.y + rect.width / 2,
        rect.width / 2,
        0, 2 * G_PI);
      cairo_fill (cr);
      zlfo_ui_theme_set_cr_color (&self->ui_theme, cr, line);
      cairo_set_line_width (cr, 4);
      cairo_arc (
        cr,
        rect.x + rect.width / 2,
        rect.y + rect.width / 2,
        rect.width / 2,
        0, 2 * G_PI);
      cairo_stroke (cr);
    }
}

static void
redraw_mid_region (
  LfoUi * self)
{
#if 0
  PuglRect rect;
  rect.x = GRID_XSTART_GLOBAL;
  rect.y = GRID_YSTART_GLOBAL;
  rect.width = GRID_WIDTH;
  rect.height = GRID_HEIGHT;
  puglPostRedisplayRect (
    self->app->view, rect);
#endif
  puglPostRedisplay (
    self->app->view);
}

static void
mid_region_bg_draw_cb (
  ZtkWidget * widget,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  LfoUi *    self)
{
  if (self->has_change ||
      !ztk_rect_is_equal (
        draw_rect, &self->last_rect))
    {
      /*ztk_message ("change");*/
      ztk_rect_copy (
        &self->last_rect, draw_rect);
      z_cairo_reset_caches (
        &self->cached_cr,
        &self->cached_surface,
        (int) WIDTH,
        (int) HEIGHT, cr);

      /* set background */
      zlfo_ui_theme_set_cr_color (
        &self->ui_theme, self->cached_cr, selected_bg);
      cairo_rectangle (
        self->cached_cr, widget->rect.x, widget->rect.y,
        widget->rect.width, widget->rect.height);
      cairo_fill (self->cached_cr);

      float sync_rate_float =
        sync_rate_to_float (
          self->sync_rate,
          self->sync_rate_type);

      /**
       * Effective frequency.
       *
       * This is either the free-running frequency,
       * or the frequency corresponding to the current
       * sync rate.
       */
      float effective_freq =
        get_effective_freq (
          self->freerun, self->freq,
          &self->common.host_pos, sync_rate_float);

      /* calculate current sample */
      gint64 cur_time = g_get_monotonic_time ();
      if (self->last_current_sample_set == 0 ||
          (!self->freerun &&
              self->common.host_pos.speed < 0.001f))
        {
          self->last_current_sample_set = cur_time;
        }
      else
        {
          double samples_diff =
            ((double) GET_SAMPLERATE (self) *
             ((double)
               (cur_time -
                  self->last_current_sample_set) /
               1000000.0));
          self->current_sample += samples_diff;
          while (self->current_sample >=
                   (double) self->common.period_size)
            {
              self->current_sample -=
                (double) self->common.period_size;
            }
          self->last_current_sample_set = cur_time;
        }

      /* draw grid */
      for (int i = 0; i < 9; i++)
        {
          if ((i % 4) == 0)
            {
              zlfo_ui_theme_set_cr_color (
                &self->ui_theme, self->cached_cr, grid_strong);
            }
          else
            {
              zlfo_ui_theme_set_cr_color (
                &self->ui_theme, self->cached_cr, grid);
            }
          cairo_move_to (
            self->cached_cr,
            widget->rect.x + GRID_HPADDING +
              i * GRID_SPACE,
            widget->rect.y + GRID_YSTART_OFFSET);
          cairo_line_to (
            self->cached_cr,
            widget->rect.x + GRID_HPADDING +
              i * GRID_SPACE,
            widget->rect.y + GRID_YEND_OFFSET);
          cairo_stroke (self->cached_cr);
        }
      zlfo_ui_theme_set_cr_color (
        &self->ui_theme, self->cached_cr, grid_strong);
      cairo_move_to (
        self->cached_cr,
        GRID_XSTART_GLOBAL,
        widget->rect.y + 105);
      cairo_line_to (
        self->cached_cr,
        GRID_XEND_GLOBAL,
        widget->rect.y + 105);
      cairo_stroke (self->cached_cr);

      recalc_vars (
        self->freerun,
        &self->common.sine_multiplier,
        &self->common.saw_multiplier,
        &self->common.period_size,
        NULL,
        &self->common.host_pos, effective_freq,
        sync_rate_float,
        (float) GET_SAMPLERATE (self));

      /* draw other visible waves in the back */
      draw_graph (self, self->cached_cr);
    }

  cairo_set_source_surface (
    cr, self->cached_surface, 0, 0);
  cairo_paint (cr);

  /* draw current position */
  double current_offset =
    self->current_sample /
    (double) self->common.period_size;

  cairo_set_source_rgba (cr, 1, 1, 1, 1);
  cairo_move_to (
    cr,
    widget->rect.x + GRID_HPADDING +
      current_offset * GRID_WIDTH,
    widget->rect.y + GRID_YSTART_OFFSET);
  cairo_line_to (
    cr,
    widget->rect.x + GRID_HPADDING +
      current_offset * GRID_WIDTH,
    widget->rect.y + GRID_YEND_OFFSET);
  cairo_stroke (cr);

  self->has_change = 0;
}

static void
mid_region_bg_update_cb (
  ZtkWidget * w,
  LfoUi *    self)
{
  int double_click = 0;
  if (w->last_btn_press >
        w->before_last_btn_press &&
      !(math_doubles_equal (
          w->last_btn_press,
          self->last_double_click)))
    {
      double diff_sec =
        w->last_btn_press -
        w->before_last_btn_press;
#if 0
      g_message (
        "last btn press %f last btn release %f "
        "(diff %f)", w->last_btn_press,
        w->last_btn_release, diff_sec);
#endif
      double_click =
        diff_sec < DOUBLE_CLICK_INTERVAL &&
        diff_sec > 0.001;
      if (double_click)
        {
          self->last_double_click =
            w->last_btn_press;
        }
    }

  double dx = w->app->offset_press_x;
  double dy = w->app->offset_press_y;
  dx -= GRID_XSTART_GLOBAL;
  dy -= GRID_YSTART_GLOBAL;

  /* create new node */
  if (double_click && self->num_nodes < 16)
    {
      /* set next available node */
      node_pos_setter (
        self,
        (unsigned int) self->num_nodes,
        (float)
        CLAMP (dx / GRID_WIDTH, 0.0, 1.0));
      node_val_setter (
        self,
        (unsigned int) self->num_nodes,
        1.f -
          (float)
          CLAMP (dy / GRID_HEIGHT, 0.0, 1.0));
      self->dragging_node = self->num_nodes;
      num_nodes_setter (
        self, self->num_nodes + 1);
    }
  else if (w->state &
             ZTK_WIDGET_STATE_RIGHT_PRESSED)
    {
      self->dragging_node = -1;
    }
  else if (w->state & ZTK_WIDGET_STATE_PRESSED)
    {
      /* move currently dragged node */
      if (self->dragging_node >= 0)
        {
          node_pos_setter (
            self,
            (unsigned int) self->dragging_node,
            (float)
            CLAMP (dx / GRID_WIDTH, 0.0, 1.0));
          node_val_setter (
            self,
            (unsigned int) self->dragging_node,
            1.f -
              (float)
              CLAMP (dy / GRID_HEIGHT, 0.0, 1.0));
        }
    }
  else
    {
      self->dragging_node = -1;
    }
}

static void
add_mid_region_bg (
  LfoUi * self)
{
  ZtkRect rect = {
    LEFT_BTN_WIDTH + 4,
    TOP_BTN_HEIGHT + 2,
    MID_REGION_WIDTH - 6, MID_REGION_HEIGHT };
  ZtkDrawingArea * da =
    ztk_drawing_area_new (
      &rect,
      (ZtkWidgetGenericCallback)
      mid_region_bg_update_cb,
      (ZtkWidgetDrawCallback) mid_region_bg_draw_cb,
      NULL, self);
  self->mid_region = (ZtkWidget *) da;
  ztk_app_add_widget (
    self->app, (ZtkWidget *) da, 0);
}

typedef struct NodeData
{
  /** Node index. */
  int idx;

  LfoUi *     zlfo_ui;
} NodeData;

static void
node_update_cb (
  ZtkWidget * w,
  NodeData *  data)
{
  LfoUi * self = data->zlfo_ui;

  /* set visibility */
  if (data->idx < self->num_nodes)
    {
      ztk_widget_set_visible (w, 1);
    }
  else
    {
      ztk_widget_set_visible (w, 0);
      return;
    }

  /* delete if right clicked */
  if (w->state & ZTK_WIDGET_STATE_RIGHT_PRESSED)
    {
      if (data->idx != 0 &&
          !math_doubles_equal (
            self->last_delete_click,
            w->last_btn_press))
        {
          for (int i = data->idx; i < 15; i++)
            {
              self->nodes[i][0] =
                self->nodes[i + 1][0];
              self->nodes[i][1] =
                self->nodes[i + 1][1];
              node_pos_setter (
                self, (unsigned int) i,
                self->nodes[i + 1][0]);
              node_val_setter (
                self, (unsigned int) i,
                self->nodes[i + 1][1]);
            }
          self->num_nodes--;
          num_nodes_setter (
            self, self->num_nodes);
          self->last_delete_click =
            w->last_btn_press;
        }
    }
  /* move if dragged */
  else if (w->state & ZTK_WIDGET_STATE_PRESSED)
    {
      double dx = w->app->offset_press_x;
      double dy = w->app->offset_press_y;
      dx -= GRID_XSTART_GLOBAL;
      dy -= GRID_YSTART_GLOBAL;

      /* first and node should not be movable */
      if (data->idx != 0)
        {
          node_pos_setter (
            self, (unsigned int) data->idx,
            (float)
            CLAMP (dx / GRID_WIDTH, 0.0, 1.0));
        }
      node_val_setter (
        self, (unsigned int) data->idx,
        1.f -
          (float)
          CLAMP (dy / GRID_HEIGHT, 0.0, 1.0));
    }

  double width = NODE_WIDTH;
  double x_offset =
    (double) self->nodes[data->idx][0];
  double y_offset =
    1.0 - (double) self->nodes[data->idx][1];
  double total_height = GRID_HEIGHT;

  /* set rectangle */
  ZtkRect rect = {
    (GRID_XSTART_GLOBAL +
      x_offset * GRID_WIDTH) - width / 2,
    (GRID_YSTART_GLOBAL +
      y_offset * total_height) - width / 2,
    width, width };
  w->rect = rect;
}

static void
node_draw_cb (
  ZtkWidget * w,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  NodeData *  data)
{
  LfoUi * self = data->zlfo_ui;

  if (!self->custom_on)
    return;

  double width = NODE_WIDTH;

  zlfo_ui_theme_set_cr_color (&self->ui_theme, cr, grid_strong);
  cairo_arc (
    cr,
    w->rect.x + width / 2,
    w->rect.y + width / 2,
    width / 2,
    0, 2 * G_PI);
  cairo_fill (cr);

  zlfo_ui_theme_set_cr_color (&self->ui_theme, cr, line);
  cairo_set_line_width (cr, 4);
  cairo_arc (
    cr,
    w->rect.x + width / 2,
    w->rect.y + width / 2,
    width / 2,
    0, 2 * G_PI);
  cairo_stroke (cr);
}

static void
add_nodes (
  LfoUi * self)
{
  for (int i = 0; i < 16; i++)
    {
      ZtkRect rect = {
        0, 0, 0, 0 };
      NodeData * data =
        calloc (1, sizeof (NodeData));
      data->idx = i;
      data->zlfo_ui = self;
      ZtkDrawingArea * da =
        ztk_drawing_area_new (
          &rect,
          (ZtkWidgetGenericCallback)
          node_update_cb,
          (ZtkWidgetDrawCallback)
          node_draw_cb,
          NULL, data);
      ZtkWidget * w = (ZtkWidget *) da;
      ztk_widget_set_visible (w, 0);
      self->node_widgets[i] = w;
      ztk_app_add_widget (
        self->app, w,
        /* nodes on the left should be on top */
        2 + (15 - i));
    }
}

static void
range_draw_cb (
  ZtkWidget * widget,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  LfoUi *    self)
{
  /* draw bg svg */
  ZtkRect rect = {
    widget->rect.x,
    widget->rect.y,
    widget->rect.width,
    widget->rect.height };
  ztk_rsvg_draw (
    self->ui_theme.range_svg, cr, &rect);

  /* draw range */
  double width = RANGE_POINT_WIDTH;
  double start_x = RANGE_STARTX - width / 2;
  double start_y = 83 - width / 2;
  double range_height = RANGE_HEIGHT;
  zlfo_ui_theme_set_cr_color (
    &self->ui_theme, cr, button_click);

  /* range */
  double range_min_y_normalized =
    1.0 - ((double) self->range_min + 1.0) / 2.0;
  double range_max_y_normalized =
    1.0 - ((double) self->range_max + 1.0) / 2.0;
  cairo_set_line_width (cr, 4.0);
  cairo_move_to (
    cr, start_x + width / 2,
    start_y + range_max_y_normalized *
      range_height + width / 2);
  cairo_line_to (
    cr, start_x + width / 2,
    start_y + range_min_y_normalized *
      range_height + width / 2);
  cairo_stroke (cr);
}

typedef struct RangePointData
{
  /** 1 if min, 0 if max. */
  int is_min;

  ZtkDrawingArea * da;

  LfoUi * zlfo_ui;
} RangePointData;

static void
range_point_update_cb (
  ZtkWidget * w,
  RangePointData * data)
{
  LfoUi * self = data->zlfo_ui;

  /* get min/max coordinates */
  double min_y =
    (1.0 - ((-1.0 +
      1.0) / 2.0)) * RANGE_HEIGHT +
     83 - RANGE_POINT_WIDTH / 2;
  double max_y =
    (1.0 - ((1.0 +
      1.0) / 2.0)) * RANGE_HEIGHT +
     83 - RANGE_POINT_WIDTH / 2;
  (void) min_y;

  if (w->state & ZTK_WIDGET_STATE_PRESSED)
    {
      double dy = w->app->offset_press_y;
      dy -= (max_y + w->rect.height / 2.0);
      dy = dy / RANGE_HEIGHT;
      dy = CLAMP (dy, 0.0, 1.0);
      dy = 1.0 - dy;

      /* at this point, dy is 1 at the top and 0
       * at the bot */

      dy = dy * 2.0 - 1.0;

      if (data->is_min)
        {
          range_min_setter (
            NULL, self, (float) dy);
        }
      else
        {
          range_max_setter (
            NULL, self, (float) dy);
        }
    }

  (void) range_min_getter;
  (void) range_max_getter;

  /* update position */
  w->rect.y =
    (1.0 - (((double)
      (data->is_min ?
         self->range_min : self->range_max) +
      1.0) / 2.0)) * RANGE_HEIGHT +
     83 - RANGE_POINT_WIDTH / 2;
}

static void
range_point_draw_cb (
  ZtkWidget * widget,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  RangePointData * data)
{
  LfoUi * self = data->zlfo_ui;

  double width = RANGE_POINT_WIDTH;
  double start_x = widget->rect.x;
  double start_y = widget->rect.y;

  zlfo_ui_theme_set_cr_color (
    &self->ui_theme, cr, button_click);
  cairo_arc (
    cr, start_x + width / 2, start_y + width / 2,
    width / 2,
    0, 2 * G_PI);
  cairo_fill (cr);
}

static void
add_range (
  LfoUi * self)
{
  /* add min point */
  double point_start_x =
    RANGE_STARTX - RANGE_POINT_WIDTH / 2;
  double point_start_y = 83 - RANGE_POINT_WIDTH / 2;
  ZtkRect rect = {
    point_start_x, point_start_y,
    RANGE_POINT_WIDTH, RANGE_POINT_WIDTH };
  RangePointData * rp =
    calloc (1, sizeof (RangePointData));
  rp->is_min = 1;
  rp->zlfo_ui = self;
  ZtkDrawingArea * da =
    ztk_drawing_area_new (
      &rect,
      (ZtkWidgetGenericCallback)
      range_point_update_cb,
      (ZtkWidgetDrawCallback) range_point_draw_cb,
      NULL, rp);
  rp->da = da;
  ztk_app_add_widget (
    self->app, (ZtkWidget *) da, 2);

  /* add max point */
  rp = calloc (1, sizeof (RangePointData));
  rp->is_min = 0;
  rp->zlfo_ui = self;
  da =
    ztk_drawing_area_new (
      &rect,
      (ZtkWidgetGenericCallback)
      range_point_update_cb,
      (ZtkWidgetDrawCallback) range_point_draw_cb,
      NULL, rp);
  rp->da = da;
  ztk_app_add_widget (
    self->app, (ZtkWidget *) da, 2);

  /* add line */
  rect.x =
    (LEFT_BTN_WIDTH + MID_REGION_WIDTH) - 10;
  rect.y = 58;
  rect.width = 64;
  rect.height = 180;
  da =
    ztk_drawing_area_new (
      &rect, NULL,
      (ZtkWidgetDrawCallback) range_draw_cb,
      NULL, self);
  ztk_app_add_widget (
    self->app, (ZtkWidget *) da, 0);
}

#if 0
static void
zrythm_icon_draw_cb (
  ZtkWidget * widget,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  LfoUi *    self)
{
  ZtkRect rect = {
    widget->rect.x,
    widget->rect.y,
    widget->rect.width,
    widget->rect.height };
  ZtkWidgetState state = widget->state;
  if (state & ZTK_WIDGET_STATE_PRESSED)
    {
      zlfo_ui_theme_set_cr_color (cr, button_hover);
      ztk_rsvg_draw (
        zlfo_ui_theme.zrythm_orange_svg, cr, &rect);
    }
  else if (state & ZTK_WIDGET_STATE_HOVERED)
    {
      zlfo_ui_theme_set_cr_color (cr, button_hover);
      ztk_rsvg_draw (
        zlfo_ui_theme.zrythm_hover_svg, cr, &rect);
    }
  else
    {
      zlfo_ui_theme_set_cr_color (
        cr, button_normal);
      ztk_rsvg_draw (
        zlfo_ui_theme.zrythm_svg, cr, &rect);
    }
}
#endif

static void
on_zrythm_btn_clicked (
  ZtkWidget * widget,
  LfoUi *    self)
{
  const double width = 378;
  const double height = 180;
  ZtkRect full_rect = {
    0, 0, WIDTH, HEIGHT };
  ZtkRect rect = {
    WIDTH / 2 - width / 2,
    HEIGHT / 2 - height / 2,
    width, height };
  ZtkDialog * dialog =
    ztk_dialog_new (
      widget->app, &full_rect, &rect, self);
  ztk_dialog_make_about (
    dialog, "LFO",
    "v" LFO_VERSION,
    "Copyright (C) 2020 Alexandros Theodotou",
    ZTK_DIALOG_ABOUT_LICENSE_AGPL_3_PLUS,
    "Design by Mire");
  ztk_app_add_widget (
    self->app, (ZtkWidget *) dialog, 400);
}

static void
add_zrythm_icon (
  LfoUi * self)
{
  ZtkRect rect = {
    LEFT_BTN_WIDTH + MID_REGION_WIDTH + 8,
    6, 30, 30 };
  ZtkButton * btn =
    ztk_button_new (
      &rect,
      (ZtkWidgetActivateCallback)
      on_zrythm_btn_clicked, self);
  ztk_button_make_svged (
    btn, 0, 0,
    self->ui_theme.zrythm_svg,
    self->ui_theme.zrythm_hover_svg,
    self->ui_theme.zrythm_orange_svg);
  ztk_app_add_widget (
    self->app, (ZtkWidget *) btn, 0);
}

/**
 * Macro to get real value.
 */
#define GET_REAL_VAL \
  ((*ctrl->getter) (ctrl, ctrl->object))

/**
 * MAcro to get real value from knob value.
 */
#define REAL_VAL_FROM_KNOB(knob) \
  (ctrl->min + (float) knob * \
   (ctrl->max - ctrl->min))

/**
 * Converts from real value to knob value
 */
#define KNOB_VAL_FROM_REAL(real) \
  (((float) real - ctrl->min) / \
   (ctrl->max - ctrl->min))

/**
 * Sets real val
 */
#define SET_REAL_VAL(real) \
   ((*ctrl->setter)(ctrl->object, (float) real))

static void
shift_control_draw_cb (
  ZtkWidget * widget,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  LfoUi *    self)
{
  ZtkControl * ctrl = (ZtkControl *) widget;

  /* draw bg */
  zlfo_ui_theme_set_cr_color (
    &self->ui_theme, cr, button_normal);
  cairo_rectangle (
    cr, widget->rect.x, widget->rect.y,
    widget->rect.width, widget->rect.height);
  cairo_fill (cr);

  /* draw black bg */
  const int bg_padding = 2;
  zlfo_ui_theme_set_cr_color (&self->ui_theme, cr, bg);
  cairo_rectangle (
    cr, widget->rect.x + bg_padding,
    widget->rect.y + bg_padding,
    widget->rect.width - bg_padding * 2,
    widget->rect.height - bg_padding * 2);
  cairo_fill (cr);

  /* set color */
  if (widget->state & ZTK_WIDGET_STATE_PRESSED)
    {
      cairo_set_source_rgba (cr, 0.9, 0.9, 0.9, 1);
    }
  else if (widget->state & ZTK_WIDGET_STATE_HOVERED)
    {
      cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1);
    }
  else
    {
      cairo_set_source_rgba (cr, 0.7, 0.7, 0.7, 1);
    }

  /* the half width of the available bar area */
  double half_width =
    (widget->rect.width  - bg_padding * 2.0) / 2.0;

  double handle_size = 12.0;

  /* draw bar */
  double real_val = (double) GET_REAL_VAL;
  if (real_val < 0.5)
    {
      double work_val = real_val / 0.5;
      double start_x =
        work_val * half_width - handle_size / 2.0;
      cairo_rectangle (
        cr,
        widget->rect.x + bg_padding +
          (start_x < 0.0 ? 0.0 : start_x),
        widget->rect.y + bg_padding,
        start_x < 0.0 ?
          handle_size + start_x : handle_size,
        widget->rect.height - bg_padding * 2);
    }
  else
    {
      double work_val = (real_val - 0.5) / 0.5;
      double start_x =
        widget->rect.x + bg_padding + half_width +
        (work_val * half_width - handle_size / 2.0);
      double extrusion =
        (start_x + handle_size) -
          ((widget->rect.x + widget->rect.width) - bg_padding);
      cairo_rectangle (
        cr, start_x,
        widget->rect.y + bg_padding,
        extrusion > 0.0 ?
          handle_size - extrusion : handle_size,
        widget->rect.height - bg_padding * 2);
    }
  cairo_fill (cr);
}

static void
grid_lbl_draw_cb (
  ZtkWidget * widget,
  cairo_t *   cr,
  ZtkRect *   draw_rect,
  DrawData *  data)
{
  LfoUi * self = data->zlfo_ui;

  /* draw svgs */
#define DRAW_SVG(caps,lowercase) \
  case LBL_TYPE_##caps: \
    { \
      ZtkRect rect = { \
        widget->rect.x, \
        widget->rect.y, \
        widget->rect.width, \
        widget->rect.height }; \
      ztk_rsvg_draw ( \
        self->ui_theme.lowercase##_svg, \
        cr, &rect); \
    } \
    break

  switch (data->val)
    {
      DRAW_SVG (INVERT, invert);
      DRAW_SVG (SHIFT, shift);
    default:
      break;
    }

#undef DRAW_SVG
}

static void
add_grid_controls (
  LfoUi * self)
{
  int padding = 2;
  int width = 76;
  int height = 22;
  int start = LEFT_BTN_WIDTH + padding + 12;
  for (int i = 0; i < NUM_GRID_BUTTONS; i++)
    {
      ZtkRect rect;
      switch (i)
        {
        case GRID_BTN_SNAP:
          rect.x =
            start + padding;
          rect.width = 76;
          break;
        case GRID_BTN_HMIRROR:
          rect.x =
            start + padding + width + padding +
            69;
          rect.width = 40;
          break;
        case GRID_BTN_VMIRROR:
          rect.x =
            start + padding + width + padding +
            111;
          rect.width = 40;
          break;
        default:
          break;
        }
      rect.height = 22;
      rect.y = TOP_BTN_HEIGHT + 12;
      DrawData * data =
        calloc (1, sizeof (DrawData));
      data->val = i;
      data->type = DATA_TYPE_BTN_GRID;
      data->zlfo_ui = self;
      ZtkButton * btn =
        ztk_button_new (
          &rect,
          (ZtkWidgetActivateCallback)
          on_btn_clicked, data);
      ztk_button_set_background_colors (
        btn,
        &self->ui_theme.bg,
        &self->ui_theme.button_hover,
        &self->ui_theme.left_button_click);
      switch (i)
        {
        case GRID_BTN_HMIRROR:
          ztk_button_make_svged (
            btn, 0, 0,
            self->ui_theme.hmirror_svg,
            self->ui_theme.hmirror_hover_svg,
            self->ui_theme.hmirror_click_svg);
          break;
        case GRID_BTN_VMIRROR:
          ztk_button_make_svged (
            btn, 0, 0,
            self->ui_theme.vmirror_svg,
            self->ui_theme.vmirror_hover_svg,
            self->ui_theme.vmirror_click_svg);
          break;
        case GRID_BTN_SNAP:
          ztk_button_make_svged (
            btn, 0, 0,
            self->ui_theme.grid_snap_svg,
            self->ui_theme.grid_snap_hover_svg,
            self->ui_theme.grid_snap_click_svg);
          break;
        }
      ztk_button_make_toggled (
        btn,
        (ZtkButtonToggledGetter)
        get_button_active);
      ztk_app_add_widget (
        self->app, (ZtkWidget *) btn, 4);
    }

  /* add shift control */
  ZtkRect rect = {
    start + padding + width + padding + 210,
    TOP_BTN_HEIGHT + 12, 76, 22 };
  ZtkControl * control =
    ztk_control_new (
      &rect,
      (ZtkControlGetter) shift_getter,
      (ZtkControlSetter) shift_setter,
      (ZtkWidgetDrawCallback) shift_control_draw_cb,
      ZTK_CTRL_DRAG_HORIZONTAL,
      self, 0.f, 1.f, 0.5f);
  control->sensitivity = 0.02f;
  ZtkWidget * control_widget = (ZtkWidget *) control;
  control_widget->user_data = self;
  ztk_control_set_relative_mode (control, 0);
  ztk_app_add_widget (
    self->app, (ZtkWidget *) control, 4);

  /* add labels */
  padding = 2;
  width = 76;
  height = 22;
  start = LEFT_BTN_WIDTH + padding;
  for (int i = 0; i < NUM_LBL_TYPES; i++)
    {
      if (i == LBL_TYPE_INVERT)
        {
          rect.x = 138;
        }
      else if (i == LBL_TYPE_SHIFT)
        {
          rect.x = 282;
        }
      rect.y = TOP_BTN_HEIGHT + 12;
      rect.width = width;
      rect.height = height;

      DrawData * data =
        calloc (1, sizeof (DrawData));
      data->val = i;
      data->type = DATA_TYPE_LBL;
      data->zlfo_ui = self;
      ZtkDrawingArea * da =
        ztk_drawing_area_new (
          &rect, NULL,
          (ZtkWidgetDrawCallback) grid_lbl_draw_cb,
          NULL, data);
      ztk_app_add_widget (
        self->app, (ZtkWidget *) da, 1);
    }
}

static void
create_ui (
  LfoUi * self)
{
  /* resize the host's window. */
  self->resize->ui_resize (
    self->resize->handle, WIDTH, HEIGHT);

  self->app = ztk_app_new (
    PLUGIN_NAME, self->parent_window,
    WIDTH, HEIGHT);

  /* init the theme */
  int ret =
    zlfo_ui_theme_init (
      &self->ui_theme,
      &self->common.pl_common.logger,
      self->bundle_path);
  if (ret)
    {
      return;
    }

  /** add each control */
  add_bg_widget (self);
  add_left_buttons (self);
  add_top_buttons (self);
  add_bot_buttons (self);
  add_mid_region_bg (self);
  add_nodes (self);
  add_grid_controls (self);
  add_range (self);
  add_zrythm_icon (self);
}

static LV2UI_Handle
instantiate (
  const LV2UI_Descriptor*   descriptor,
  const char*               plugin_uri,
  const char*               bundle_path,
  LV2UI_Write_Function      write_function,
  LV2UI_Controller          controller,
  LV2UI_Widget*             widget,
  const LV2_Feature* const* features)
{
  LfoUi * self = calloc (1, sizeof (LfoUi));
  self->write = write_function;
  self->controller = controller;
  self->dragging_node = -1;
  self->has_change = 1;
  strcpy (self->bundle_path, bundle_path);

  PluginCommon * pl_common = &self->common.pl_common;

#ifndef RELEASE
  ztk_log_set_level (ZTK_LOG_LEVEL_DEBUG);
#endif

#define HAVE_FEATURE(x) \
  (!strcmp(features[i]->URI, x))

  for (int i = 0; features[i]; ++i)
    {
      if (HAVE_FEATURE (LV2_UI__parent))
        {
          self->parent_window = features[i]->data;
        }
      else if (HAVE_FEATURE (LV2_UI__resize))
        {
          self->resize =
            (LV2UI_Resize*)features[i]->data;
        }
      else if (HAVE_FEATURE (LV2_URID__map))
        {
          pl_common->map =
            (LV2_URID_Map *) features[i]->data;
        }
      else if (HAVE_FEATURE (LV2_LOG__log))
        {
          pl_common->log =
            (LV2_Log_Log *) features[i]->data;
        }
    }

#undef HAVE_FEATURE

  if (!pl_common->map)
    {
      lv2_log_error (
        &pl_common->logger, "Missing feature urid:map\n");
    }

  /* map uris */
  map_uris (pl_common->map, &self->common);

  lv2_atom_forge_init (
    &pl_common->forge, pl_common->map);

  /* create UI and set the native window to the
   * widget */
  create_ui (self);
  *widget =
    (LV2UI_Widget)
    puglGetNativeWindow (self->app->view);

  /* let the plugin know that the UI is active */
  uint8_t obj_buf[64];
  lv2_atom_forge_set_buffer (
    &pl_common->forge, obj_buf, 64);
  LV2_Atom_Forge_Frame frame;
  lv2_atom_forge_frame_time (
    &pl_common->forge, 0);
  LV2_Atom* msg =
    (LV2_Atom *)
    lv2_atom_forge_object (
      &pl_common->forge, &frame, 1,
      self->common.uris.ui_on);
  lv2_atom_forge_pop (&pl_common->forge, &frame);
  self->write (
    self->controller, 0,
    lv2_atom_total_size (msg),
    pl_common->uris.atom_eventTransfer, msg);

  return self;
}

static void
cleanup (LV2UI_Handle handle)
{
  LfoUi * self = (LfoUi *) handle;
  PluginCommon * pl_common = &self->common.pl_common;

  /* let the plugin know that the UI is off */
  uint8_t obj_buf[64];
  lv2_atom_forge_set_buffer (
    &pl_common->forge, obj_buf, 64);
  LV2_Atom_Forge_Frame frame;
  lv2_atom_forge_frame_time (
    &pl_common->forge, 0);
  LV2_Atom* msg =
    (LV2_Atom *)
    lv2_atom_forge_object (
      &pl_common->forge, &frame, 1,
      self->common.uris.ui_off);
  lv2_atom_forge_pop (&pl_common->forge, &frame);
  self->write (
    self->controller, 0,
    lv2_atom_total_size (msg),
    pl_common->uris.atom_eventTransfer, msg);

  ztk_app_free (self->app);

  free (self);
}

/**
 * Port event from the plugin.
 */
static void
port_event (
  LV2UI_Handle handle,
  uint32_t     port_index,
  uint32_t     buffer_size,
  uint32_t     format,
  const void*  buffer)
{
  LfoUi * self = (LfoUi *) handle;
  PluginCommon * pl_common = &self->common.pl_common;

  /* check type of data received
   *  format == 0: [float] control-port event
   *  format > 0: message
   *  Every event message is sent as separate
   *  port-event
   */
  if (format == 0)
    {
      switch (port_index)
        {
        case LFO_FREQ:
          self->freq = * (const float *) buffer;
          break;
        case LFO_CV_GATE:
          self->cv_gate =
            * (const float *) buffer;
          break;
        case LFO_CV_TRIGGER:
          self->cv_trigger =
            * (const float *) buffer;
          break;
        case LFO_GATE:
          self->gate =
            * (const float *) buffer;
          break;
        case LFO_TRIGGER:
          self->trigger =
            (int) * (const float *) buffer;
          break;
        case LFO_SHIFT:
          self->shift = * (const float *) buffer;
          break;
        case LFO_RANGE_MIN:
          self->range_min =
            * (const float *) buffer;
          break;
        case LFO_RANGE_MAX:
          self->range_max =
            * (const float *) buffer;
          break;
        case LFO_STEP_MODE:
          self->step_mode =
            (int) * (const float *) buffer;
          break;
        case LFO_FREE_RUNNING:
          self->freerun =
            (int) * (const float *) buffer;
          break;
        case LFO_GRID_STEP:
          self->grid_step =
            (int) * (const float *) buffer;
          break;
        case LFO_SYNC_RATE:
          self->sync_rate =
            * (const float *) buffer;
          break;
        case LFO_SYNC_RATE_TYPE:
          self->sync_rate_type =
            * (const float *) buffer;
          break;
        case LFO_HINVERT:
          self->hinvert =
            (int) * (const float *) buffer;
          break;
        case LFO_VINVERT:
          self->vinvert =
            (int) * (const float *) buffer;
          break;
        case LFO_SINE_TOGGLE:
          self->sine_on =
            (int) * (const float *) buffer;
          break;
        case LFO_SAW_TOGGLE:
          self->saw_on =
            (int) * (const float *) buffer;
          break;
        case LFO_SQUARE_TOGGLE:
          self->square_on =
            (int) * (const float *) buffer;
          break;
        case LFO_TRIANGLE_TOGGLE:
          self->triangle_on =
            (int) * (const float *) buffer;
          break;
        case LFO_CUSTOM_TOGGLE:
          self->custom_on =
            (int) * (const float *) buffer;
          break;
        case LFO_NUM_NODES:
          self->num_nodes =
            (int) * (const float *) buffer;
          break;
        case LFO_SAMPLE_TO_UI:
          self->current_sample =
            (double) * (const float *) buffer;
          break;
        default:
          break;
        }

      if (port_index >= LFO_NODE_1_POS &&
          port_index <= LFO_NODE_16_CURVE)
        {
          unsigned int prop =
            (port_index - LFO_NODE_1_POS) % 3;
          unsigned int node_id =
            (port_index - LFO_NODE_1_POS) / 3;
          self->nodes[node_id][prop] =
            * (const float *) buffer;
        }
      /*puglPostRedisplay (self->app->view);*/

      if (port_index != LFO_SAMPLE_TO_UI)
        {
          self->has_change = 1;
        }
    }
  else if (format ==
             pl_common->uris.atom_eventTransfer)
    {
      const LV2_Atom* atom =
        (const LV2_Atom*) buffer;
      if (lv2_atom_forge_is_object_type (
            &pl_common->forge, atom->type))
        {
          const LV2_Atom_Object* obj =
            (const LV2_Atom_Object*) atom;
          if (obj->body.otype ==
                self->common.uris.ui_state)
            {
              const LV2_Atom
                * current_sample = NULL,
                * samplerate = NULL,
                * period_size = NULL,
                * sine_multiplier = NULL,
                * saw_multiplier = NULL;
              lv2_atom_object_get (
                obj,
                self->common.uris.
                  ui_state_current_sample,
                &current_sample,
                self->common.uris.
                  ui_state_period_size,
                &period_size,
                self->common.uris.
                  ui_state_samplerate,
                &samplerate,
                self->common.uris.
                  ui_state_sine_multiplier,
                &sine_multiplier,
                self->common.uris.
                  ui_state_saw_multiplier,
                &saw_multiplier,
                NULL);
              if (current_sample &&
                  current_sample->type ==
                    pl_common->uris.atom_Long &&
                  samplerate &&
                  samplerate->type ==
                    pl_common->uris.atom_Double &&
                  period_size &&
                  period_size->type ==
                    pl_common->uris.atom_Long &&
                  sine_multiplier &&
                  sine_multiplier->type ==
                    pl_common->uris.atom_Float &&
                  saw_multiplier &&
                  saw_multiplier->type ==
                    pl_common->uris.atom_Float)
                {
                  self->common.current_sample =
                    ((LV2_Atom_Long*)
                     current_sample)->body;
                  self->current_sample =
                    (double)
                    self->common.current_sample;
                  SET_SAMPLERATE (
                    self,
                    ((LV2_Atom_Double*)
                     samplerate)->body);
                  self->common.period_size =
                    ((LV2_Atom_Long*)
                     period_size)->body;
                  self->common.sine_multiplier =
                    ((LV2_Atom_Float*)
                     sine_multiplier)->body;
                  self->common.saw_multiplier =
                    ((LV2_Atom_Float*)
                     saw_multiplier)->body;
                }
              else
                {
                  ztk_warning (
                    "failed to read UI state "
                    "atom");
                }
            }
          if (obj->body.otype ==
                pl_common->uris.time_Position)
            {
              update_position_from_atom_obj (
                &self->common, obj);
            }

          self->has_change = 1;

/*#if 0*/
          /*redraw_mid_region (self);*/
/*#endif*/
        }
      else
        {
#if 0
          log_error (
            self->log, &self->uris,
            "Unknown message type");
#endif
        }
    }
  else
    {
#if 0
      log_error (
        self->log, &self->uris,
        "Unknown format");
#endif
    }
}

/* Optional non-embedded UI show interface. */
static int
ui_show (LV2UI_Handle handle)
{
  printf ("show called\n");
  LfoUi * self = (LfoUi *) handle;
  ztk_app_show_window (self->app);
  return 0;
}

/* Optional non-embedded UI hide interface. */
static int
ui_hide (LV2UI_Handle handle)
{
  printf ("hide called\n");
  LfoUi * self = (LfoUi *) handle;
  ztk_app_hide_window (self->app);

  return 0;
}

/**
 * LV2 idle interface for optional non-embedded
 * UI.
 */
static int
ui_idle (LV2UI_Handle handle)
{
  LfoUi * self = (LfoUi *) handle;

  ztk_app_idle (self->app);
  redraw_mid_region (self);

  return 0;
}

/**
 * LV2 resize interface for the host.
 */
static int
ui_resize (
  LV2UI_Feature_Handle handle, int w, int h)
{
  LfoUi * self = (LfoUi *) handle;
  self->resize->ui_resize (
    self->resize->handle, WIDTH, HEIGHT);
  return 0;
}

/**
 * Called by the host to get the idle and resize
 * functions.
 */
static const void*
extension_data (const char* uri)
{
  static const LV2UI_Idle_Interface idle = {
    ui_idle };
  static const LV2UI_Resize resize = {
    0 ,ui_resize };
  static const LV2UI_Show_Interface show = {
    ui_show, ui_hide };
  if (!strcmp(uri, LV2_UI__idleInterface))
    {
      return &idle;
    }
  if (!strcmp(uri, LV2_UI__resize))
    {
      return &resize;
    }
  if (!strcmp(uri, LV2_UI__showInterface))
    {
      return &show;
    }
  return NULL;
}

static const LV2UI_Descriptor descriptor = {
  PLUGIN_UI_URI,
  instantiate,
  cleanup,
  port_event,
  extension_data,
};

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor*
lv2ui_descriptor (uint32_t index)
{
  switch (index)
    {
    case 0:
      return &descriptor;
    default:
      return NULL;
    }
}
