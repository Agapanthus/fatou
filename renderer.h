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
	rTile(AiSize size, float effort, GLuint magQuality = GL_NEAREST);
	~rTile();

	void render(function<void(ARect tile)> content, AiSize tileSize, ARect position);
	void draw(GLuint textureID = GL_TEXTURE0);

	void scale(AiSize size, float effort);

	// if effortQ is <= effort, subcache is used (fast!)
	void setEffortQ(float effortQ);

	int getIter();
	AiSize getSize();

	void bind(GLuint textureID = GL_TEXTURE0);

	void framebufferWrite();

protected:
	float effort, effortQ;
	AiSize size;
//	bool useMargin;
	ARect position;
	sQuad quad;
	bool useSwap;
	AiSize tileSize;
	GLuint magQuality;

	pointer<syncBuffer> swapBuffer;

//	int vw, vh;
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        sRenderer        ]*******************************/

class sRenderer {
public:
	// effort is about proportional to the square of the time necessary to render
	// a tile and controls the resolution of the tile.
	virtual void setEffort(float effort) = 0;
	virtual float getEffort() const = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       fOptimizer        ]*******************************/

class fOptimizer : public QPC, public ANoncopyable {
public:
	fOptimizer(float targetFrameRate, float maxEffort = 2.0f);
	void hint(float factor);  // factor should be an estimate for the change in rendering time due to parameter change (eg, factor=2.0f if rendering is expected to take twice as long)
	void optimize(sRenderer *renderer);
	void setTargetFramerate(float targetFrameRate); // Dont call hint!
	void setMaxEffort(float maxEffort);
	float getPixelDensity() const;
	float getFramerate() const;
private:
	bool inited;
	float targetFrameRate;
	float maxEffort;
	double floatingTime;
	int notAgain;
	float cuEffort;
	float aEffort;
	int lastSleep;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        tRenderer        ]*******************************/

// Divides Frame into set of tiles. This is necessary for very high resolutions
// in order to reduce memory consumption.
class tRenderer : public sRenderer, public ANoncopyable {
public:
	tRenderer(AiSize size, AiSize tiles, float maxEffort = 2.0f);
	~tRenderer();

	void setSize(AiSize size, AiSize tiles, float maxEffort = 2.0f);
	
	// Returns true, if there are tiles left
	bool renderTile(function<void(ARect tile)> content);
	void drawTile(GLuint textureID = GL_TEXTURE0);

	void setEffort(float effort);
	float getEffort() const;

protected:
	float effort, maxEffort;
	pointer<rTile> tile;
	AiSize tiles, size;
	ARect tileArea;
	int tilec;
};

