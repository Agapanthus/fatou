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
#include "tRenderer.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Buffer    ]*******************************/

// TODO: increasing QUEUE_LENGTH has SIGNIFICANT negative effect on rendering time! Why and how to solve that?! (--> Seems to be due to frequent FBO binds...)
#define QUEUE_LENGTH_R 5
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


	void discard();
	void draw(int x, int y, ARect tC);
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

	void view(APoint pos, ASize zoom, function<void(APoint, ASize)> viewF);

	void render(function<void(void)> renderF, bool discard);
	void setSize(AiSize size, float maxEffort);

	void setEffort(float effort);
	float getEffort() const;
	void draw(int x, int y);

private:
	pointer<pBuffer> buffer;
	float maxEffort;
	AiSize size;
	float effort;

	ASize targetZ, realZ;
	APoint targetP, realP;

	ASize minQuality;

	bool fresh;
};