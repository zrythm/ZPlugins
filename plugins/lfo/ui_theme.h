/*
 * Copyright (C) 2020-2021 Alexandros Theodotou <alex at zrythm dot org>
 *
 * This file is part of ZLFO
 *
 * ZLFO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ZLFO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 */

#ifndef __ZLFO_UI_THEME_H__
#define __ZLFO_UI_THEME_H__

#include PLUGIN_CONFIG

#include "lv2/log/logger.h"

#include <ztoolkit/ztk.h>

#include <glib.h>

/**
 * Theme for the ZLFO UI.
 */
typedef struct LfoUiTheme
{
  /** Background color. */
  ZtkColor bg;

  /** Selected area background color. */
  ZtkColor selected_bg;

  /** Button color. */
  ZtkColor button_normal;

  /** Button hover color. */
  ZtkColor button_hover;

  /** Active grey color. */
  ZtkColor button_active;

  /** Button click color. */
  ZtkColor button_click;

  /** Left button click color. */
  ZtkColor left_button_click;

  /** Bright click color. */
  ZtkColor bright_click;

  /** Line/curve color. */
  ZtkColor line;

  /** Grid line color. */
  ZtkColor grid;

  /** Grid strong line color. */
  ZtkColor grid_strong;

  /** Line below or above the button when hovered. */
  ZtkColor button_lining_active;
  ZtkColor button_lining_hover;

  ZtkRsvgHandle * sine_svg;
  ZtkRsvgHandle * saw_svg;
  ZtkRsvgHandle * triangle_svg;
  ZtkRsvgHandle * square_svg;
  ZtkRsvgHandle * custom_svg;
  ZtkRsvgHandle * curve_svg;
  ZtkRsvgHandle * step_svg;
  ZtkRsvgHandle * curve_active_svg;
  ZtkRsvgHandle * step_active_svg;

  ZtkRsvgHandle * range_svg;

  ZtkRsvgHandle * sync_svg;
  ZtkRsvgHandle * freeb_svg;
  ZtkRsvgHandle * sync_black_svg;
  ZtkRsvgHandle * freeb_black_svg;

  ZtkRsvgHandle * zrythm_svg;
  ZtkRsvgHandle * zrythm_hover_svg;
  ZtkRsvgHandle * zrythm_orange_svg;

  ZtkRsvgHandle * grid_snap_svg;
  ZtkRsvgHandle * grid_snap_hover_svg;
  ZtkRsvgHandle * grid_snap_click_svg;
  ZtkRsvgHandle * hmirror_svg;
  ZtkRsvgHandle * hmirror_hover_svg;
  ZtkRsvgHandle * hmirror_click_svg;
  ZtkRsvgHandle * vmirror_svg;
  ZtkRsvgHandle * vmirror_hover_svg;
  ZtkRsvgHandle * vmirror_click_svg;
  ZtkRsvgHandle * invert_svg;
  ZtkRsvgHandle * shift_svg;

  ZtkRsvgHandle * down_arrow_svg;

} LfoUiTheme;

static inline int
zlfo_ui_theme_init (
  LfoUiTheme *     theme,
  LV2_Log_Logger * logger,
  const char *     bundle_path)
{

#define SET_COLOR(cname,_hex) \
  ztk_color_parse_hex ( \
    &theme->cname, _hex); \
  theme->cname.alpha = 1.0

  SET_COLOR (bg, "#323232");
  SET_COLOR (button_normal, "#4A4A4A");
  SET_COLOR (button_hover, "#5D5D5D");
  SET_COLOR (button_active, "#6D6D6D");
  SET_COLOR (button_click, "#F79616");
  SET_COLOR (left_button_click, "#D68A0C");
  SET_COLOR (bright_click, "#FF6501");
  SET_COLOR (line, "#D68A0C");
  SET_COLOR (selected_bg, "#262626");
  SET_COLOR (grid_strong, "#DDDDDD");
  SET_COLOR (grid, "#999999");
  SET_COLOR (button_lining_active, "#2EB398");
  SET_COLOR (button_lining_hover, "#19664c");

  char * abs_path;
#define LOAD_SVG(name) \
  abs_path = \
    g_build_filename ( \
      bundle_path, "resources", #name ".svg", \
      NULL); \
  theme->name##_svg = \
    ztk_rsvg_load_svg (abs_path); \
  if (!theme->name##_svg) \
    { \
      lv2_log_error ( \
        logger, \
        "Failed loading SVG: %s", abs_path); \
      return -1; \
    }

  LOAD_SVG (sine);
  LOAD_SVG (triangle);
  LOAD_SVG (saw);
  LOAD_SVG (square);
  LOAD_SVG (custom);
  LOAD_SVG (curve);
  LOAD_SVG (step);
  LOAD_SVG (curve_active);
  LOAD_SVG (step_active);
  LOAD_SVG (range);
  LOAD_SVG (sync);
  LOAD_SVG (freeb);
  LOAD_SVG (sync_black);
  LOAD_SVG (freeb_black);
  LOAD_SVG (zrythm);
  LOAD_SVG (zrythm_hover);
  LOAD_SVG (zrythm_orange);
  LOAD_SVG (grid_snap);
  LOAD_SVG (grid_snap_hover);
  LOAD_SVG (grid_snap_click);
  LOAD_SVG (hmirror);
  LOAD_SVG (hmirror_hover);
  LOAD_SVG (hmirror_click);
  LOAD_SVG (vmirror);
  LOAD_SVG (vmirror_hover);
  LOAD_SVG (vmirror_click);
  LOAD_SVG (invert);
  LOAD_SVG (shift);
  LOAD_SVG (down_arrow);

  return 0;
}

/**
 * Sets the cairo color to that in the theme.
 */
#define zlfo_ui_theme_set_cr_color(theme,cr,color_name) \
  ztk_color_set_for_cairo ( \
    &(theme)->color_name, cr)

#define zlfo_ui_theme_set_cr_color_with_alpha( \
  theme,cr,color_name,alpha) \
  cairo_set_source_rgba ( \
    cr, (theme)->color_name.red, \
    (theme)->color_name.green, \
    (theme)->color_name.blue, alpha)

#endif
