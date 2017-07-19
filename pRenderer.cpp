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
	"uniform float queue_l;"
	"uniform float maxZ;"
	"uniform int winSizeX;"
	"uniform int winSizeY;"
	"uniform int queue_r;"
	"uniform vec2 scale;"

	"void main() {"
	"	if(maxZ > 10000.0) gl_FragColor= vec4(maxZ);"
	"	int x = int((winSizeX*TexCoords.x)) % queue_r;"
	"	int y = int((winSizeY*TexCoords.y)) % queue_r;"
	"	float layer = (x + (queue_r*y) )/ (queue_l-1);"
/*	"	float layer =  ((mod(TexCoords.x, pixelSizeX) / (pixelSizeX) / queue_r) + (mod(TexCoords.y, pixelSizeY) / (pixelSizeY) )) - 0.026f ; "
	//"	if(layer > maxZ) layer = maxZ;"
	//"	if(layer > 1.0f) layer = 1.0f;"*/
	"	if(layer < 0.0f) gl_FragColor = vec4(1.0f); else "
	"	if(layer > 1.0f) gl_FragColor = vec4(1.0f,0.0f,0.0f,1.0f); else "
//	"	gl_FragColor = texture(tex, vec3(TexCoords, layer))*0.0 + vec4(layer, layer,0.0f, 1.0f);\n"
	"	gl_FragColor = texture(tex, vec3(TexCoords.x * scale.x , TexCoords.y * scale.y, layer));\n"
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
	uniform.queue_l = pBuffer::composer->getUniform("queue_l");
	uniform.winSizeY = pBuffer::composer->getUniform("winSizeY");
	uniform.winSizeX = pBuffer::composer->getUniform("winSizeX");
	uniform.queue_r = pBuffer::composer->getUniform("queue_r");
	uniform.scale = pBuffer::composer->getUniform("scale");
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
			buffer.reset(new syncBuffer3d(((size + QUEUE_LENGTH_R - 1) / QUEUE_LENGTH_R), QUEUE_LENGTH, GL_NEAREST, GL_NEAREST));
		else
			buffer->scale(((size + QUEUE_LENGTH_R - 1) / QUEUE_LENGTH_R));

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

	// 1 Manchmal wird das Bild schwarz
	// 2 es gibt verirrte Pixel in Spalte 1
	// 3 Beim skalieren werden die Pixel falsch sortiert!

	bool changed = false;
	while (effort > 0.0f) {
		//if (pBuffer::posx == size.w) return 0.0f;
		if (pBuffer::currentBuffer == QUEUE_LENGTH) break;

		int posxnew = buffer->getSize().w - 1; // minimum(uint32(buffer->getSize().w), uint32(round(posx + (float(QUEUE_LENGTH) * effort * float(buffer->getSize().w)))));
		if (posxnew == posx) {
			break; 
		}
		ASize ratio(ASize(buffer->getSize() * QUEUE_LENGTH_R) / ASize(size));
		ARect part(0.0f, 0.0f, 1.0f, 1.0f); // float(posx) / float(buffer->getSize().w), 0.0f, float(posxnew) / float(buffer->getSize().w), 1.0f);
		ARect partT = part * ASize(ratio.w, ratio.h) ;

		pBuffer::posx = posxnew;

		ASize delta((((pBuffer::currentBuffer ) % QUEUE_LENGTH_R ) + 0.5f) / float(QUEUE_LENGTH_R) - 0.5f,
			 (((pBuffer::currentBuffer ) / QUEUE_LENGTH_R ) + 0.5f) / float(QUEUE_LENGTH_R) - 0.5f);
		delta = delta / ASize(buffer->getSize());
		
		changed = true;
		buffer->writeTo(pBuffer::currentBuffer);
		renderF();
		quad.draw(part, partT+delta);
		if (posxnew == buffer->getSize().w-1) {
			pBuffer::currentBuffer++;
			pBuffer::posx = 0;
		}
		effort -= (partT.right - partT.left) / float(QUEUE_LENGTH);
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
		glUniform1f(uniform.queue_l,  float(QUEUE_LENGTH) );
		glUniform1i(uniform.winSizeY, (size.h));
		glUniform1i(uniform.winSizeX, (size.w));
		glUniform1i(uniform.queue_r, (QUEUE_LENGTH_R));
		glUniform2f(uniform.scale, float(size.w) / float(buffer->getSize().w*QUEUE_LENGTH_R), float(size.h) / float(buffer->getSize().h*QUEUE_LENGTH_R));

		glErrors("pBuffer::composeUniform");

		ARect part(0.0f, 0.0f, 1.0f, 1.0f);
		ARect partT(part);
		ASize Relation(float(pBuffer::size.w) / float(pBuffer::buffer->getSize().w * QUEUE_LENGTH_R),
			float(pBuffer::size.h) / float(pBuffer::buffer->getSize().h * QUEUE_LENGTH_R));
		partT = partT * Relation;
		
		cout << toString(pBuffer::size) << " " << toString( pBuffer::buffer->getSize() * QUEUE_LENGTH_R) << " " << toString(partT) << endl;
		pBuffer::quad.draw(part, partT);
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
	//cout << toString(size) << maxEffort << endl;
	pRenderer::maxEffort = maxEffort;
	pRenderer::size = size;
	buffer->scale(AiSize(int(ceil(size.w * maxEffort)), int(ceil(size.h*maxEffort))));
}

void pRenderer::setEffort(float effort) {
	fassert(pRenderer::maxEffort >= effort);

	effort = 100.0f / size.h;
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