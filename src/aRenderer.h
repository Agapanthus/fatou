/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: aRenderer.h
* Purpose: Class choosing the right renderer and controlling the framerate.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"
#include "renderer.h"
#include "pRenderer.h"
//#include "parallelctx.h"
//#include "worker.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[      autoRenderer       ]*******************************/

class aRenderer : public ANoncopyable {
public:
	aRenderer(AiSize size, AiSize tiles, float maxEffort, float targetFramerate, function<void(void)> renderF);
	~aRenderer();
	void render(int x, int y, bool changed);
	void setSize(AiSize size);
	void setMaxEffort(float effort);
	void setTargetFramerate(float framerate);
	float getPixelDensity();
	float getFramerate();


protected:
	pointer<fOptimizer> optim;
	AiSize windowSize;
	float maxEffort;
	AiSize tiles;
	pointer<shader> texprogram;
	bool useProgressive;

	pointer<tRenderer> tR;
	pointer<pRenderer> pR;
	//pointer<offscreenctx<worker, workerMsg>> octx;
	function<void(void)> renderF;
};