/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: pRenderer.h
* Purpose: Progressive Renderer.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "pRenderer.h"
#include "data.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Buffer    ]*******************************/

const string composerShader(
	//"uniform sampler2D tex1, tex2, tex3;"
	"uniform sampler3D tex;"
	"in vec2 TexCoords;"
	"uniform float maxZ;"
	"uniform float stepSize;"
	"uniform float pixelSizeX;"
	"uniform float pixelSizeY;"
	"uniform float queue_r;"

	"void main() {"
	"	if(maxZ > 10000.0) gl_FragColor= vec4(maxZ, stepSize, pixelSizeX, pixelSizeY*queue_r);"
	"	int x = int(800*TexCoords.x) % 20;"
	"	int y = int(600*TexCoords.y) % 20;"
	"	float layer = (x + (20*y))/20.0f/20.0f;"
/*	"	float layer =  ((mod(TexCoords.x, pixelSizeX) / (pixelSizeX) / queue_r) + (mod(TexCoords.y, pixelSizeY) / (pixelSizeY) )) - 0.026f ; "
	//"	if(layer > maxZ) layer = maxZ;"
	//"	if(layer > 1.0f) layer = 1.0f;"*/
	"	if(layer < 0.0f) gl_FragColor = vec4(1.0f); else "
	"	if(layer > 1.0f) gl_FragColor = vec4(1.0f,0.0f,0.0f,1.0f); else "
//	"	gl_FragColor = texture(tex, vec3(TexCoords, layer))*0.0 + vec4(layer, layer,0.0f, 1.0f);\n"
	"	gl_FragColor = texture(tex, vec3(TexCoords, layer));\n"
	"}");

int roundUpToNextMultipleOfN(int numToRound, int N) {
	if (N == 0)
		return numToRound;
	int remainder = numToRound % N;
	if (remainder == 0)
		return numToRound;
	return numToRound + N - remainder;
}

pBuffer::pBuffer(AiSize size) :
	syncBuffer(size, false, GL_LINEAR, GL_NEAREST), size(0,0) {

	// Check if there are enough texture units for interpolation
/*	GLint texture_units = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
	glErrors("pRenderer::getMaxTextureImageUnits");
	if (texture_units < QUEUE_LENGTH) {
		fatalNote("GPU doesn't support " << QUEUE_LENGTH << " textures per fragment shader");
	}*/

	pBuffer::composer.reset(new shader(mainVertexShader, composerShader));

	pBuffer::composer->use();
	uniform.texture = pBuffer::composer->getUniform("tex");
	uniform.maxZ = pBuffer::composer->getUniform("maxZ");
	uniform.stepSize = pBuffer::composer->getUniform("stepSize");
	uniform.pixelSizeX = pBuffer::composer->getUniform("pixelSizeX");
	uniform.pixelSizeY = pBuffer::composer->getUniform("pixelSizeY");
	uniform.queue_r = pBuffer::composer->getUniform("queue_r");
	glUniform1i(uniform.texture, 0);

	/*
	glUniform1i(pBuffer::composer->getUniform("tex1"), 0);
	glUniform1i(pBuffer::composer->getUniform("tex2"), 1);
	glUniform1i(pBuffer::composer->getUniform("tex3"), 2);
	*/

	//buffers.resize(QUEUE_LENGTH);
	pBuffer::scale(size);
	//glGenQueries(QUEUE_LENGTH, pBuffer::queries);
}
pBuffer::~pBuffer() {

}

void pBuffer::scale(AiSize size) {
	//if (size.w % collect + size.h % collect != 0) throw exception("Size must be divideable by collect!");
	fassert(QUEUE_LENGTH > 0);

	if (size != pBuffer::size) {
		if(size != syncBuffer::iSize) 
			syncBuffer::scale(size, false);

		if (buffer.empty())
			buffer.reset(new syncBuffer3d(((size + QUEUE_LENGTH_R) / QUEUE_LENGTH_R), QUEUE_LENGTH, GL_NEAREST, GL_NEAREST));
		else
			buffer->scale(((size + QUEUE_LENGTH_R) / QUEUE_LENGTH_R));

		/*if (buffers[0].empty() ? true : ((size + QUEUE_LENGTH_R)/QUEUE_LENGTH_R) != buffers[0]->getSize()) {
			for (size_t i = 0; i < QUEUE_LENGTH; i++) {
				buffers[i].reset(new syncBuffer(((size + QUEUE_LENGTH_R) / QUEUE_LENGTH_R), false, GL_NEAREST, GL_NEAREST));
			}
		}*/
	}
	pBuffer::size = size;
	pBuffer::posx = 0;
	pBuffer::currentBuffer = 0;
}

void pBuffer::draw() {
	syncBuffer::readFrom();
	pBuffer::quad.draw();
}
void pBuffer::render(float effort, function<void(void)> renderF) {
	bool changed = false;
	while (effort > 0.0f) {
		//if (pBuffer::posx == size.w) return 0.0f;
		if (pBuffer::currentBuffer == QUEUE_LENGTH) break;

		int posxnew = minimum(uint32(size.w), uint32(round(posx + (float(QUEUE_LENGTH) * effort * float(size.w)))));
		if (posxnew == posx) break; // posxnew = posx + 1;

		ARect part(float(posx) / float(size.w), 0.0f, float(posxnew) / float(size.w), 1.0f);
		pBuffer::posx = posxnew;

		float stepWidth = 1.0f / float(QUEUE_LENGTH_R);
		ASize delta(0.0f, // float( (pBuffer::currentBuffer % QUEUE_LENGTH_R) + 0.5f) * stepWidth - 0.5f,
			0.0f); // float(int(pBuffer::currentBuffer / QUEUE_LENGTH_R) + 0.5f) * stepWidth - 0.5f);
		delta = delta / ASize(buffer->getSize());
		
		changed = true;
		buffer->writeTo(pBuffer::currentBuffer);
		renderF();
		quad.draw(part, part+delta);
		if (posxnew == size.w) {
			pBuffer::currentBuffer++;
			pBuffer::posx = 0;
		}
		effort -= (1.0f / float(QUEUE_LENGTH) * ((part.right - part.left)));
	}
	if (changed) pBuffer::compose();
}
void pBuffer::compose() {
	//buffers[0]->framebufferRead();
	//syncBuffer::framebufferWrite();
	//glBlitFramebuffer(0, 0, size.w, size.h, 0, 0, size.w, size.h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/*
	for (size_t i = 0; i < QUEUE_LENGTH; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		buffers[i]->readFrom(GL_TEXTURE0 + i);
	}
	*/



	buffer->bind(GL_TEXTURE0);
	
	syncBuffer::writeTo([this](void)->void {
		composer->use();
		glUniform1f(uniform.maxZ, maximum(0, int32(pBuffer::currentBuffer) - 1) / maximum(1.0f, float(QUEUE_LENGTH - 1)));
		glUniform1f(uniform.stepSize,  1.0f / maximum(1.0f, float(QUEUE_LENGTH - 1)) );
		glUniform1f(uniform.pixelSizeX, float(QUEUE_LENGTH_R) / float(size.w));
		glUniform1f(uniform.pixelSizeY, float(QUEUE_LENGTH_R) / float(size.h));
		glUniform1f(uniform.queue_r, float(QUEUE_LENGTH_R));
		glErrors("pBuffer::composeUniform");
		pBuffer::quad.draw();
	});
	
	//buffers[0]
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Renderer  ]*******************************/

pRenderer::pRenderer(AiSize size, float maxEffort) : size(size), maxEffort(maxEffort), effort(0.1f) {
	pRenderer::buffer.reset(new pBuffer(AiSize(int(ceil(size.w * maxEffort)), int(ceil(size.h*maxEffort)))));
}

pRenderer::~pRenderer() {

}

void pRenderer::render(function<void(void)> renderF) {
	buffer->render(pRenderer::effort / pRenderer::maxEffort, renderF);
}

void pRenderer::setSize(AiSize size, float maxEffort) {
	cout << toString(size) << maxEffort << endl;
	pRenderer::maxEffort = maxEffort;
	pRenderer::size = size;
	buffer->scale(AiSize(int(ceil(size.w * maxEffort)), int(ceil(size.h*maxEffort))));
}

void pRenderer::setEffort(float effort) {
	fassert(pRenderer::maxEffort >= effort);

	effort = 1.0f / size.h;
/*
	if (pRenderer::effort != effort) {
		pRenderer::buffer->setEffort(effort / pRenderer::maxEffort);
	}*/
	pRenderer::effort = effort;	
}
float pRenderer::getEffort() const {
	return pRenderer::effort;
}
void pRenderer::draw() {
	buffer->draw();
}