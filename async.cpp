/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: async.cpp
* Purpose: Asynchronous Rendering.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once

#include "stdafx.h"
#include "async.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[      asyncBuffer        ]*******************************/

asyncBuffer::asyncBuffer(AiSize size, GLuint query) : 
	syncBuffer(size, false, GL_LINEAR, GL_NEAREST), query(query), working(false) {

}
asyncBuffer::~asyncBuffer() {

}
void asyncBuffer::startJob(function<void(void)> job) {
	fassert(!asyncBuffer::isWorking());

	asyncBuffer::working = true;
	glBeginQuery(GL_TIME_ELAPSED, asyncBuffer::query);
	syncBuffer::writeTo(job);
	glEndQuery(GL_TIME_ELAPSED);
}
bool asyncBuffer::isWorking() {
	if (!asyncBuffer::working) return false;
	int done;
	glGetQueryObjectiv(asyncBuffer::query, GL_QUERY_RESULT_AVAILABLE, &done);
	glErrors("asyncBuffer::isWorking");
	if (done) {
		asyncBuffer::working = false;
	}
	return asyncBuffer::working;
}
void asyncBuffer::readFrom(GLuint textureID) {
	fassert(!asyncBuffer::isWorking());
	syncBuffer::readFrom(textureID);
}
void asyncBuffer::scale(AiSize size) {
	syncBuffer::scale(size, false);
}
void asyncBuffer::framebufferRead() {
	fassert(!asyncBuffer::isWorking());
	syncBuffer::framebufferRead();
}

AiSize asyncBuffer::getSize() {
	return syncBuffer::getSize();
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        glqueue          ]*******************************/

glqueue::glqueue(AiSize size, size_t steps, float millisecondsPerFrame) :
	mspf(millisecondsPerFrame) {

	glGenQueries(1, &(glqueue::query));
	glErrors("glqueue::genQuery");

	glqueue::buffers.resize(steps);
	for (size_t i = 0; i < steps; i++) {
		glqueue::buffers[i].reset(new asyncBuffer(size, query));
	}
}
glqueue::~glqueue() {

}
void glqueue::appendJob(function<void(void)> job) {
	// TODO
	glqueue::buffers[0]->startJob(job);
}

void glqueue::clearQueue() {
	// TODO
}


bool glqueue::bufferReady(size_t index) {
	return !glqueue::buffers[0]->isWorking();
}
void glqueue::framebufferRead(size_t index) {
	glqueue::buffers[0]->framebufferRead();
}


void glqueue::scale(AiSize size, float millisecondsPerFrame) {
	glqueue::mspf = millisecondsPerFrame;
	glqueue::buffers[0]->scale(size);
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        pRenderer        ]*******************************/
/*
#define QUEUE_LENGTH 16
pRenderer::pRenderer(AiSize size, float millisecondsPerFrame, float maxEffort) :
	queue(size, QUEUE_LENGTH, millisecondsPerFrame),
	tile(size, maxEffort, GL_LINEAR)
{


	// Check if there are enough texture units for interpolation
	GLint texture_units = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
	glErrors("pRenderer::getMaxTextureImageUnits");
	if (texture_units < QUEUE_LENGTH) {
		fatalNote("GPU doesn't support " << QUEUE_LENGTH << " textures per fragment shader");
	}
}

void pRenderer::setSize(AiSize size, float millisecondsPerFrame, float maxEffort) {
	pRenderer::tile.scale(size, maxEffort);
	pRenderer::queue.scale(size, millisecondsPerFrame);
}

pRenderer::~pRenderer() {

}




void pRenderer::render(function<void(void)> func, bool parametersChanged) {
	glErrors("pRenderer::pre");
	static bool firstOne = true;
	fassert(!firstOne || parametersChanged);
	firstOne = false;

	static bool copied = false;

	if (parametersChanged || copied) {
		copied = false;

		queue.appendJob([func, this](void)->void {
			func();
			//for (size_t i = 0; i<100; i++)
				pRenderer::quad.draw(ARect(0.0f, 0.0f, 1.0f, 1.0f), ARect(0.0f, 0.0f, 1.0f, 1.0f), true);
		});
		cout << "add" << endl;
		glErrors("pRenderer::render");
	}

	

	if (queue.bufferReady(0) && !copied) {
		copied = true;
		queue.framebufferRead(0);
		tile.framebufferWrite();
		glBlitFramebuffer(0, 0, pRenderer::tile.getSize().w, pRenderer::tile.getSize().h, 0, 0, pRenderer::tile.getSize().w, pRenderer::tile.getSize().h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		cout << "copy" << endl;
		glErrors("pRenderer::copy");
	}
	else if(!copied){
		cout << "wait " << endl;
	}
}

void pRenderer::draw() {
	tile.bind();
	//queue.buffers[0]->readFrom();
	quad.draw(ARect(0.0f,0.0f,1.0f,1.0f), ARect(0.0f, 0.0f, 1.0f, 1.0f), true);

	glErrors("pRenderer::draw");
}
*/