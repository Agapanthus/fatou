/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: async.h
* Purpose: Asynchronous Rendering.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once

#include "stdafx.h"
#include "renderer.h"
#include "parallelctx.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[      asyncBuffer        ]*******************************/

class asyncBuffer : public syncBuffer, public ANoncopyable {
public:
	asyncBuffer(AiSize size, GLuint query);
	~asyncBuffer();
	void startJob(function<void(void)> job);
	bool isWorking();
	void readFrom(GLuint textureID = GL_TEXTURE0);
	void scale(AiSize size);
	void framebufferRead();
	AiSize getSize();
private:
	GLuint query;
	bool working;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        glqueue          ]*******************************/

class glqueue : public ANoncopyable {
public:
	// Asynchronously processes size_t steps jobs. 
	// The special feature is, that you can clear the queue and therefore don't need
	// to wait for a large queue of jobs to finish if you want to cancel it. The number 
	// of uploaded (not-cancelable) jobs is estimated using the target Framerate to
	// make them finish them in less than 2 Frames. Therefore, all jobs should
	// take about the same time to finish.
	glqueue(AiSize size, size_t steps, float millisecondsPerFrame);
	~glqueue();
	void appendJob(function<void(void)> job);
	void scale(AiSize size, float millisecondsPerFrame);
	void clearQueue();
	bool bufferReady(size_t index);
	void framebufferRead(size_t index);


	vector<pointer<asyncBuffer>> buffers;
private:
	GLuint query;
	float mspf;

	vector<function<void(void)>> jobs;

};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        pRenderer        ]*******************************/
/*
class pRenderer : public ANoncopyable {
public:
	pRenderer(AiSize size, float millisecondsPerFrame, float maxEffort);
	~pRenderer();

	void setSize(AiSize size, float millisecondsPerFrame, float maxEffort);

	void render(function<void(void)> func, bool parametersChanged);
	void draw();

	sQuad quad;

	void startMovement();
	void endMovement();
	void move(double dx, double dy, double dz);
	

private:
	rTile tile;
	glqueue queue;
};*/