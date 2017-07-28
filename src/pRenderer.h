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

//#define DISABLE_BRANCHING
//#define USE_DOTPRODUCT
#define USE_TEXEL_FETCH // Disabling this is not implemented!

#define USE_INTERPOLATION
#define INTERP_POINTS 4

#define MEASURE_PERFORMANCE


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Buffer    ]*******************************/

// TODO: increasing QUEUE_LENGTH has SIGNIFICANT negative effect on rendering time! Why and how to solve that?! (--> Seems to be due to frequent FBO binds...)
#define QUEUE_LENGTH_R 10
#define QUEUE_LENGTH_X QUEUE_LENGTH_R
#define QUEUE_LENGTH_Y QUEUE_LENGTH_R
// TODO: Implement X != Y
#define QUEUE_LENGTH (QUEUE_LENGTH_X*QUEUE_LENGTH_Y)

// Render parts of a frame, almost arbitrarily small parts, an then, compose them to a whole frame
class pBuffer {
public:
	// Accepts effort between 0 and 1
	pBuffer(AiSize size);
	~pBuffer();
	void scale(AiSize size); 


	float getProgress();
	void discard();
	void draw(int x, int y, APoint pos, ASize zoom);
	void compose();
	uint64 render(uint64 samples, function<void(int)> renderF);

	void quickFill();

	void setPosition(APoint pos, ASize zoom);

private:
	//vector<pointer<syncBuffer>> buffers;
	//GLuint queries[QUEUE_LENGTH];

	void swap();
	void initIdentity(int32 i, int32 lr);

	pointer<syncBuffer3d> buffer;

	vector<int32> permutationMap;

	pointer<floatStorage> coeffBuffer;
	void recalculateCoeff();

	struct {
		GLuint texture, queue_l , queue_r, maxZ, interPM, iteration, iWinSize;
		//GLuint winSize, scale;
		GLuint inverse_queue_l_minus_one;
	} uniform;

	sQuad quad;
	AiSize size;

	uint32 posx;
	uint32 currentBuffer;

	pointer<shader> composer;
	pointer<shader> white;
	glLine line;

	pointer<syncBuffer> buffers[2];
	APoint rPosition[2];
	ASize rZoom[2];
	size_t writeBuffer, readBuffer;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Renderer  ]*******************************/

// Scroll around, choose good buffer sizes...
class pRenderer : public sRenderer {
public:
	pRenderer(AiSize size, float maxDensity1D);
	~pRenderer();

	void view(APoint pos, ASize zoom, function<void(APoint, ASize)> viewF);

	uint64 render(function<void(int)> renderF, bool discard);
	void setSize(AiSize size, float maxDensity1D);

	void setSampleCount(uint64 samples);
	uint64 getSampleCount() const;
	void draw(int x, int y);
	float getProgress();

	AiSize getSize() const;

private:
	pointer<pBuffer> buffer;
	float maxDensity1D;
	AiSize size;
	uint64 samples;

	// "real" value is the native value the shader is rendering with and "target" is the value visible to the user
	ASize targetZ, realZ;
	APoint targetP, realP;

	ASize minQuality;

	bool fresh;
};