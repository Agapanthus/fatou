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

	enum filters { 
		edge_scharr,
		edge_sobel_feldman,
		box_blur,
		gaussian_blur_3,
		gaussian_blur_5,
		gaussian_blur_9,
		unsharp_masking,
		sharpen,
		edge,
		relief,
		gamma_correct
	};

	/////////////////////////////////

	fractal(const string &script, const string &scriptdeep, double cy, int iter, int biasPower);

	void setFunction(const string &script);
	void setDoublePrecision(bool doublePrecision); /* {
		if (fractal::doublePrecision != doublePrecision) {
			fractal::doublePrecision = doublePrecision;
		//	fractal::recompile();
		}
	}*/
	/*void setPreFilters(vector<fractal::filters> fil) {
		preFilters = fil; // TODO: use them!
	}
	void setPostFilters(vector<fractal::filters> fil) {
		postFilters = fil; // TODO: use them!
	}
	*/
	// TODO: optimization!
	// TODO: Colors! Smoothing! Orbits! Orbit traps!
	// TODO: music visualization!

	////////////////////////////////

	bool nk(nk_context *ctx, tooltip &ttip);
	bool logic(uint32 timeElapsed, float animSpeed);
	void render(int32 layer);
	void view(APointd p, ASized z);


	// TODO: Nur hier, weil das einfacher ist.
	double coe[MAX_POLY], coet[MAX_POLY];
	float coef[MAX_POLY];

private:

	//void recompile();

	struct {
		GLint iter, screenTexture, c, zoom, pos, coec, coe;
		GLint layer;
	} uniform;

	struct {
		GLint iter, screenTexture, c, zoom, pos, coec, coe;
		GLint layer;
	} uniformd;

	double cx, cy;
	int32 iter;

	size_t coec;
	double biasPower;

	pointer<shader> program;
	pointer<shader> programd;
	pointer<texture> colorMap;

	////////////////////////////////////////////

	bool doublePrecision;

	bool optimizeShader;
	vector<fractal::filters> preFilters;
	vector<fractal::filters> postFilters;



};
