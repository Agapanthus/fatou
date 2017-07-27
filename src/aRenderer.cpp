/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: aRenderer.cpp
* Purpose: Class choosing the right renderer and controlling the framerate.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "aRenderer.h"
#include "data.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[      autoRenderer       ]*******************************/

aRenderer::aRenderer(AiSize size, AiSize tiles, float maxDensity1D, float targetFramerate, function<void(int)> renderF) :
	tiles(tiles), maxDensity1D(maxDensity1D), renderF(renderF), windowSize(size), useProgressive(true),
	realZ(0.0f,0.0f), realP(0.0f,0.0f), samplesRendered(0) {
	tR.reset(new tRenderer(size, tiles, maxDensity1D));

	texprogram.reset(new shader(mainVertexShader, viewtexture));
	texprogram->use();
	glUniform1i(texprogram->getUniform("screenTexture"), 0);

	optim.reset(new fOptimizer(targetFramerate, maxDensity1D));

	pR.reset(new pRenderer(size, maxDensity1D));

	//octx.reset(new offscreenctx<worker, workerMsg>(new worker()));
	//octx->start();

	// TODO: in animation, automatically turn tiles to 1,1 if the density is less than 1 
}


aRenderer::~aRenderer() {
	// Destroy the context first! this important, because it shares resources with this class which become unavailable during destruction...
	//octx.reset(0, false);
}


void aRenderer::render(int x, int y, bool changed) {
	fassert(aRenderer::realZ.w != 0.0f); // you must call view before rendering!
	fassert(aRenderer::realZ.h != 0.0f);

	bool didntUsedProgressive = !useProgressive;
	useProgressive = !changed;

	// Using the aRender instead of pRender after Changes which will not repeat in the following frames is good, because the aRenderer will present a preview Frame which can be shown while pRender still works on it's first Frame!
	
	if (aRenderer::useProgressive) {
		pR->view(aRenderer::realP, aRenderer::realZ, aRenderer::viewF);
		optim->optimize((sRenderer*)pR.data(), aRenderer::samplesRendered);
		aRenderer::samplesRendered = pR->render(renderF, didntUsedProgressive);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, aRenderer::windowSize.w, aRenderer::windowSize.h);
		texprogram->use();
		pR->draw(x, y);
	}
	else {
		aRenderer::viewF(aRenderer::realP, aRenderer::realZ);

		optim->optimize((sRenderer*)tR.data(), aRenderer::samplesRendered);
		aRenderer::samplesRendered = 0;
		while (tR->renderTile([this](ARect tile) -> void { renderF(0); }, aRenderer::samplesRendered)) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, aRenderer::windowSize.w, aRenderer::windowSize.h);
			texprogram->use();
			tR->drawTile();
		}
	}
}
void aRenderer::setSize(AiSize size) {
	if (aRenderer::useProgressive) {
		//octx->push(sizeChangeMessage(size));
		aRenderer::pR->setSize(size, aRenderer::maxDensity1D);
	}
	else {
		aRenderer::tR->setSize(size, tiles, aRenderer::maxDensity1D);
	}
	windowSize = size;	
	
	// Obsolete
	//aRenderer::optim->hint(float(windowSize.w*windowSize.h) / (size.w*size.h)); // TODO: This won't work properly when maximizing a window with degree 200 polynomials inside!
}

void aRenderer::setMaxEffort(float maxDensity1D) {
	aRenderer::optim->setMaxDensity1D(maxDensity1D);
	aRenderer::maxDensity1D = maxDensity1D;
	aRenderer::pR->setSize(windowSize, maxDensity1D);
}

void aRenderer::setTargetFramerate(float framerate) {
	aRenderer::optim->setTargetFramerate(framerate);
}

float aRenderer::getDensity1D() {
	if (aRenderer::useProgressive) {
		return sqrt(aRenderer::pR->getProgress()) * aRenderer::maxDensity1D;
	}
	else {
		return float(sqrt(double(aRenderer::optim->getSamples()) / double(aRenderer::windowSize.area())));
	}
}
float aRenderer::getFramerate() {
	return aRenderer::optim->getFramerate();
}

void aRenderer::view(APoint pos, ASize zoom, function<void(APoint, ASize)> viewF) {
	fassert(zoom.w != 0.0f);
	fassert(zoom.h != 0.0f);

	aRenderer::viewF = viewF;

	if (aRenderer::realZ == ASize(0.0f,0.0f)) {
		aRenderer::viewF(pos, zoom);
	}

	aRenderer::realP = pos;
	aRenderer::realZ = zoom;
}
uint64 aRenderer::getSamplesPerFrame() {
	return aRenderer::samplesRendered; // optim->getSamples();
}