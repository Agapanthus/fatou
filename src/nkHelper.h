/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU          ]*******************************
*
* File: nkHelper.h
* Purpose: Helpers for the GUI.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"

extern int show_tooltips;


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         tooltip         ]*******************************/

class tooltip {
public:
	tooltip(nk_context *ctx) : nkTooltip_in(&ctx->input) {

	}

	void create(nk_context *ctx, const string &text) {
		nkTooltip_bounds = nk_widget_bounds(ctx);
		if (nk_input_is_mouse_hovering_rect(nkTooltip_in, nkTooltip_bounds) 
				&& nk_window_is_hovered(ctx) /*otherwise tooltips will even appear when the hovered element is invisible. !!! TODO: But they still appear, when hovering another window which is ontop of them!*/) 
				nk_tooltip(ctx, text.c_str()); 
	}

private:
	const struct nk_input *nkTooltip_in;
	struct nk_rect nkTooltip_bounds;
};

