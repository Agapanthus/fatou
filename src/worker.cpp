/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: worker.cpp
* Purpose: Thread for progressive rendering.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "worker.h"
#include "data.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          worker         ]*******************************/


worker::worker(int dummy) :
	cx(0.00000001f), cy(3.0f),
	iter(100),
	zoomx(1.0f), zoomy(1.0f), 
	posx(0.0f), posy(0.0f)
{
	pR.reset(new pRenderer(AiSize(800, 600), 2.0f));

	worker::zoomx = 2.0f * (800.0f / 1000.0f);
	worker::zoomy = 2.0f * (600.0f / 1000.0f);
	
#ifdef USE_TEXTURE_ARRAY
	worker::program.reset(new shader(mainVertexShaderRenamed, mainFragmentShader, triangleLayerSelector));
	worker::uniform.layer = worker::program->getUniform("lr");
#else
	worker::program.reset(new shader(mainVertexShader, mainFragmentShader));
#endif
	worker::uniform.screenTexture = worker::program->getUniform("screenTexture");
	worker::uniform.c = worker::program->getUniform("c");
	worker::uniform.iter = worker::program->getUniform("iter");
	worker::uniform.zoom = worker::program->getUniform("zoom");
	worker::uniform.pos = worker::program->getUniform("pos");
	worker::uniform.coe = worker::program->getUniform("coe");
	worker::uniform.coec = worker::program->getUniform("coec");
	worker::program->use();
	glUniform1i(worker::uniform.screenTexture, 0);
	glUniform2f(worker::uniform.zoom, zoomx, zoomy);
	glUniform2f(worker::uniform.pos, posx, posy);

	worker::colorMap.reset(new texture(basePath + "res/hue.png"));

	for (size_t p = 0; p < MAX_POLY; p++) coe[p] = coet[p] = 0.0f;
#if 0
	coet[0] = -1.0f;
	coet[1] = -1.0f;
	coet[4] = 20.0f;
	coet[7] = 100.0f;
	coet[44] = 20.0f;
#else // Max: 115k
	coet[0] = -1.0f;
	coet[7] = 100.0f;
	coet[977] = 100.0f;
	coet[1] = -1.0f;
#endif
	for (size_t p = 0; p < MAX_POLY; p++) coe[p] = coet[p];


	texprogram.reset(new shader(mainVertexShader, viewtexture));
	texprogram->use();
	glUniform1i(texprogram->getUniform("screenTexture"), 0);



	worker::buf.reset(new syncBuffer(AiSize(800, 600), false));
}

worker::~worker() {

}

void worker::render() {
	// Get messages
	static workerMsg msg(true);
	while(AQueue::pop(msg, false)) {
		switch (msg.type) {
		case workerMsg::sizeChange:
			break;
		case workerMsg::cancel:
			break;
		}
	}

	worker::pR->view(APoint(posx, posy), ASize(zoomx, zoomy), [this](APoint pos, ASize zoom)->void {
		worker::program->use();
		glUniform2f(worker::uniform.zoom, zoom.w, zoom.h);
		glUniform2f(worker::uniform.pos, pos.x, pos.y);
		glErrors("worker::viewFunction");
	});


	///////////////////// Animate Polynomials
	for (size_t p = 0; p < MAX_POLY; p++) {
		if (coet[p] != 0.0f || coe[p] != 0.0f) {
			coec = p;
			if (abs(coe[p] - coet[p]) < 0.000001f) coe[p] = coet[p];
			else {
				//float blendTim = tanh(0.01f * timeElapsed / (animSpeed / 1000.0f));
				float blendTim = 0.0f;
				coe[p] = coe[p] * (1.0f - blendTim) + coet[p] * blendTim;
				//Change = true;
			}
		}
	}

	static uint64 totalR = 0;
	static QPC q;
	if(totalR == 0) q.get();

	// Render a frame
	uint64 rendered = 0;
	{
		criticalSection cs;

		pR->setSampleCount(10 * 1000);
		rendered = pR->render([this](size_t layer)->void {
			
			worker::program->use();

			cx *= 1.0001f;

			glUniform2f(worker::uniform.c, cx, cy);
			glUniform1i(worker::uniform.iter, iter);
			glUniform1fv(worker::uniform.coe, MAX_POLY, coe);
			glUniform1i(worker::uniform.coec, coec);
#ifdef USE_TEXTURE_ARRAY
			glUniform1i(worker::uniform.layer, layer);
#endif
			glErrors("worker::uniform");

			worker::colorMap->use(GL_TEXTURE0);
						
		}, false);
		


	//glFinish(); // blocking
		glFlush(); // nonblocking

	}

	totalR += rendered;
	if (totalR >= 800 * 600 * 4) {
		double time = q.get();
		cout << "TOTAL: " << toString(time/1000.0f, 2) << " " << int(totalR / time) << "k" << endl;
		totalR = 0;
	}

}

void worker::draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	ARect t = worker::pR->drawExternal();
	//ARect t = ARect(0.0f, 0.0f, 1.0f, 1.0f);
	//buf->readFrom();
	sQuad quad;
	quad.draw(ARect(0.0f, 0.0f, 1.0f, 1.0f), t);
}