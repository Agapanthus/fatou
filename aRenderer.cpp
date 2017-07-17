/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: aRenderer.cpp
* Purpose: Class choosing the right renderer and controlling the framerate.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"
#include "aRenderer.h"
#include "data.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[      autoRenderer       ]*******************************/

aRenderer::aRenderer(AiSize size, AiSize tiles, float maxEffort, float targetFramerate, function<void(void)> renderF) :
	tiles(tiles), maxEffort(maxEffort), renderF(renderF), windowSize(size) {
	tR.reset(new tRenderer(size, tiles, maxEffort));

	texprogram.reset(new shader(mainVertexShader, viewtexture));
	texprogram->use();
	glUniform1i(texprogram->getUniform("screenTexture"), 0);

	optim.reset(new fOptimizer(targetFramerate, maxEffort));
	octx.reset(new offscreenctx<worker, workerMsg>(new worker()));
	octx->start();
}


aRenderer::~aRenderer() {
	// Destroy the context first! this important, because it shares resources with this class which become unavailable during destruction...
	octx.reset(0, false);
}


void aRenderer::render() {

	optim->optimize((sRenderer*)tR.data());
	while (tR->renderTile([this](ARect tile) -> void { renderF(); })) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, aRenderer::windowSize.w, aRenderer::windowSize.h);
		texprogram->use();
		tR->drawTile();
	}

}
void aRenderer::setSize(AiSize size) {
	aRenderer::tR->setSize(size, tiles, maxEffort);
	aRenderer::optim->hint(float(windowSize.w*windowSize.h) / (size.w*size.h));
	octx->push(sizeChangeMessage(size));
	windowSize = size;
}

void aRenderer::setMaxEffort(float effort) {
	aRenderer::optim->setMaxEffort(effort);
}

void aRenderer::setTargetFramerate(float framerate) {
	aRenderer::optim->setTargetFramerate(framerate);
}

float aRenderer::getPixelDensity() {
	return aRenderer::optim->getPixelDensity();
}
float aRenderer::getFramerate() {
	return aRenderer::optim->getFramerate();
}