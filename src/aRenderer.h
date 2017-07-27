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
#include "tRenderer.h"
#include "pRenderer.h"
//#include "parallelctx.h"
//#include "worker.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[      autoRenderer       ]*******************************/

class aRenderer : public ANoncopyable {
public:
	aRenderer(AiSize size, AiSize tiles, float maxDensity1D, float targetFramerate, function<void(int)> renderF);
	~aRenderer();
	void render(int x, int y, bool changed);
	void setSize(AiSize size);
	void setMaxEffort(float maxDensity1D);
	void setTargetFramerate(float framerate);
	float getDensity1D();
	uint64 getSamplesPerFrame();
	float getFramerate();

	void view(APoint pos, ASize zoom, function<void(APoint, ASize)> viewF);

protected:
	pointer<fOptimizer> optim;
	AiSize windowSize;
	float maxDensity1D;
	AiSize tiles;
	pointer<shader> texprogram;
	bool useProgressive;

	pointer<tRenderer> tR;
	pointer<pRenderer> pR;
	//pointer<offscreenctx<worker, workerMsg>> octx;
	function<void(int)> renderF;

	ASize realZ;
	APoint realP;
	function<void(APoint, ASize)> viewF;

	uint64 samplesRendered;
};