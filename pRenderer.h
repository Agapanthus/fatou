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

#define QUEUE_LENGTH_R 40
#define QUEUE_LENGTH (QUEUE_LENGTH_R*QUEUE_LENGTH_R)

// Render parts of a frame, almost arbitrarily small parts, an then, compose them to a whole frame
class pBuffer : public syncBuffer {
public:
	// Accepts effort between 0 and 1
	pBuffer(AiSize size);
	~pBuffer();
	void scale(AiSize size); 


	void draw();
	void compose();
	void render(float effort, function<void(void)> renderF);

private:
	//vector<pointer<syncBuffer>> buffers;
	//GLuint queries[QUEUE_LENGTH];
	pointer<syncBuffer3d> buffer;

	struct {
		GLuint texture, winSizeX, winSizeY, queue_l , queue_r, maxZ, scale;
	} uniform;

	sQuad quad;
	AiSize size;

	uint32 posx;
	uint32 currentBuffer;

	pointer<shader> composer;
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
	void draw();

private:
	pointer<pBuffer> buffer;
	float maxEffort;
	AiSize size;
	float effort;
};