/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU          ]*******************************
*
* File: fractal.h
* Purpose: Creating shaders, optimizing them and managing parameters for per-fragment 
* rendered iterated functions.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"
#include "GLHelpers.h"
#include "nkHelper.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         fractal         ]*******************************/

class fractal {
public:
	fractal();

	bool nk(nk_context *ctx, tooltip &ttip);
	bool logic(uint32 timeElapsed, float animSpeed);
	void render(int32 layer);
	void view(APoint p, ASize z);

private:

	struct {
		GLint iter, screenTexture, c, zoom, pos, coec, coe;
		GLint layer;
	} uniform;

	float cx, cy;
	int32 iter;

	size_t coec;
	float coe[MAX_POLY], coet[MAX_POLY];
	int biasPower;

	pointer<shader> program;
	pointer<texture> colorMap;

	


};
