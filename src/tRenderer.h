/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: tiledRenderer.h
* Purpose: Class for rendering.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once

#include "stdafx.h"
#include "GLHelpers.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         rTile           ]*******************************/

class rTile : protected syncBuffer, public ANoncopyable {
public:
	rTile(AiSize size, float maxDensity1D, GLenum magQuality = GL_NEAREST);
	~rTile();

	uint64 render(function<void(ARect tile)> content, AiSize tileSize, ARect position);
	void draw(GLenum textureID = GL_TEXTURE0);

	void scale(AiSize size, float maxDensity1D);

	void setSampleCount(uint64 samples);

	int getIter();
	AiSize getSize();

	void bind(GLenum textureID = GL_TEXTURE0);

	void framebufferWrite();

protected:
	float maxDensity1D;
	uint64 samples;
	AiSize size;
//	bool useMargin;
	ARect position;
	sQuad quad;
	bool useSwap;
	AiSize tileSize;
	GLenum magQuality;

	pointer<syncBuffer> swapBuffer;

//	int vw, vh;
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        sRenderer        ]*******************************/

class sRenderer {
public:
	virtual void setSampleCount(uint64 samples) = 0;
	virtual uint64 getSampleCount() const = 0;
	virtual AiSize getSize() const = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       fOptimizer        ]*******************************/

class fOptimizer : public QPC, public ANoncopyable {
public:
	fOptimizer(double targetFrameRate, float maxDensity1D = 2.0f);
	void hint(double factor);  // factor should be an estimate for the change in rendering time due to parameter change (eg, factor=2.0f if rendering is expected to take twice as long)
	void optimize(sRenderer *renderer, uint64 samplesRendered);
	void setTargetFramerate(float targetFrameRate); // Dont call hint!
	void setMaxDensity1D(float maxDensity1D);
	uint64 getSamples() const;
	float getFramerate() const;
	void reset();
private:
	bool inited;
	double targetFrameRate;
	float maxDensity1D;
	double floatingTime, floatingTimePublic;
	int notAgain;
	double changing;
	uint64 aSamples;
	uint64 lastSleep;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        tRenderer        ]*******************************/

// Divides Frame into set of tiles. This is necessary for very high resolutions
// in order to reduce memory consumption.
class tRenderer : public sRenderer, public ANoncopyable {
public:
	tRenderer(AiSize size, AiSize tiles, float maxDensity1D = 2.0f);
	~tRenderer();

	void setSize(AiSize size, AiSize tiles, float maxDensity1D = 2.0f);
	
	// Returns true, if there are tiles left
	bool renderTile(function<void(ARect tile)> content, uint64 &samplesRendered);
	void drawTile(GLenum textureID = GL_TEXTURE0);

	void setSampleCount(uint64 samples);
	uint64 getSampleCount() const;
	AiSize getSize() const;

protected:
	uint64 samples;
	float maxDensity1D;
	pointer<rTile> tile;
	AiSize tiles, size;
	ARect tileArea;
	int tilec;
};

