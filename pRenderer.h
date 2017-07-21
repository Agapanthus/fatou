/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: pRenderer.h
* Purpose: Progressive Renderer.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"
#include "renderer.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Buffer    ]*******************************/

#define QUEUE_LENGTH_R 10
#define QUEUE_LENGTH_X QUEUE_LENGTH_R
#define QUEUE_LENGTH_Y QUEUE_LENGTH_R
// TODO: Implement X != Y
#define QUEUE_LENGTH (QUEUE_LENGTH_X*QUEUE_LENGTH_Y)

// Render parts of a frame, almost arbitrarily small parts, an then, compose them to a whole frame
class pBuffer : public syncBuffer {
public:
	// Accepts effort between 0 and 1
	pBuffer(AiSize size);
	~pBuffer();
	void scale(AiSize size); 


	void draw(int x, int y);
	void compose();
	void render(float effort, function<void(void)> renderF);

private:
	//vector<pointer<syncBuffer>> buffers;
	//GLuint queries[QUEUE_LENGTH];
	pointer<syncBuffer3d> buffer;

	vector<int32> permutationMap;

	pointer<floatStorage> coeffBuffer;
	void recalculateCoeff();

	struct {
		GLuint texture, queue_l , queue_r, maxZ, interPM, iteration, iWinSize;
		//GLuint winSize, scale;
	} uniform;

	sQuad quad;
	AiSize size;

	uint32 posx;
	uint32 currentBuffer;

	pointer<shader> composer;
	pointer<shader> white;
	glLine line;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Renderer  ]*******************************/

// Scroll around, choose good buffer sizes...
class pRenderer : public sRenderer {
public:
	pRenderer(AiSize size, float maxEffort);
	~pRenderer();

	void render(function<void(void)> renderF);
	void setSize(AiSize size, float maxEffort);

	void setEffort(float effort);
	float getEffort() const;
	void draw(int x, int y);

private:
	pointer<pBuffer> buffer;
	float maxEffort;
	AiSize size;
	float effort;
};