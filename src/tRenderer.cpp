/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: renderer.cpp
* Purpose: classes for Rendering.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "tRenderer.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         rTile           ]*******************************/


void rTile::framebufferWrite() {
	// TODO: Remove this function!
	syncBuffer::framebufferWrite();
}


rTile::rTile(AiSize size, float maxDensity1D, GLenum magQuality) :
	// Mipmaps are deactivated (caused artifacts). Instead, use series of linear copies! 
	syncBuffer(AiSize( int(ceil(float(size.w)*maxDensity1D)), int(ceil(float(size.h)*maxDensity1D))), false, GL_LINEAR, magQuality), 
	magQuality(magQuality),	size(size), samples(rTile::getSize().area()), maxDensity1D(maxDensity1D), position(0.0f,0.0f,0.0f,0.0f) {

	if (rTile::maxDensity1D > 2.0f) {
		int iterations = rTile::getIter();
		swapBuffer.reset(new syncBuffer(rTile::size * (1 << iterations), false, GL_LINEAR, magQuality));
	}
}

rTile::~rTile() {

}

uint64 rTile::render(std::function<void(ARect tile)> content, AiSize tileSize, ARect position) {
	fassert(rTile::samples > 0);
	fassert(tileSize.w <= rTile::size.w);
	fassert(tileSize.h <= rTile::size.h);

	float effortQ = float(pow(float(double(rTile::samples) / double(rTile::getSize().area())), 0.5f) * rTile::maxDensity1D);

	int vw = (effortQ < rTile::maxDensity1D) ? int(ceil(effortQ*float(tileSize.w))) : int(ceil(float(tileSize.w)*rTile::maxDensity1D));
	int vh = (effortQ < rTile::maxDensity1D) ? int(ceil(effortQ*float(tileSize.h))) : int(ceil(float(tileSize.h)*rTile::maxDensity1D));
	rTile::tileSize = tileSize;


	rTile::position = position;
	glBindFramebuffer(GL_FRAMEBUFFER, syncBuffer::framebuffer);
	glErrors("tRenderer::bind");
	//cout << effortQ  << " " << size.w << " " << size.h << " | " << vw << " " << vh << endl;
	glViewport(0, 0, vw, vh);
	if(content) content(position);
	rTile::quad.draw(ARect(.0f,.0f,1.f,1.f), position);

	rTile::useSwap = false;

	if (syncBuffer::useMipmap) {
		glBindTexture(GL_TEXTURE_2D, syncBuffer::tex);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else if (effortQ > 2.f) {
		int iterations = rTile::getIter();

		// Downscale
		AiSize last(vw, vh);
		for (int i = iterations; i > 0; i--) {
			
			if (rTile::useSwap) {
				syncBuffer::framebufferWrite();
				rTile::swapBuffer->framebufferRead();
			}
			else {
				syncBuffer::framebufferRead();
				rTile::swapBuffer->framebufferWrite();
			}
			rTile::useSwap = !rTile::useSwap;
			
			AiSize smaller(rTile::size * (1 << i));

			// TODO: Use Shader! Blit is quite slow!
			glBlitFramebuffer(0, 0, last.w, last.h, 0, 0, smaller.w, smaller.h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			last = smaller;
			glErrors("rTile::blit");
		}
	}

	return uint64(vw)*vh;
}

void rTile::setSampleCount(uint64 samples) {
	fassert(samples <= rTile::getSize().area());
	rTile::samples = samples;
}

int rTile::getIter() {
	float effortQ = float(pow(float(double(rTile::samples) / double(rTile::getSize().area())), 0.5f) * rTile::maxDensity1D);

	int iterations = int(floor(log2(effortQ)));
	// We don't need the additional iteration!
	if (double(iterations) == log2(effortQ))
		iterations--;
	return iterations;
}

AiSize rTile::getSize() {
	return syncBuffer::getSize();
}

void rTile::scale(AiSize size, float maxDensity1D) {
	rTile::size = size;
	syncBuffer::scale(AiSize(int(ceil(size.w*maxDensity1D)), int(ceil(size.h*maxDensity1D))), false /* effort > 2.f*/);
	rTile::maxDensity1D = maxDensity1D;
	rTile::samples = rTile::getSize().area();

	if (rTile::maxDensity1D > 2.0f) {
		int iterations = rTile::getIter();
		if(swapBuffer.data())
			swapBuffer->scale(rTile::size * (1 << iterations), false);
		else 
			swapBuffer.reset(new syncBuffer(rTile::size * (1 << iterations), false, GL_LINEAR, rTile::magQuality));
	}
	else {
		swapBuffer.reset(0, false);
	}
}

void rTile::bind(GLenum textureID) {
	if (rTile::useSwap) {
		rTile::swapBuffer->readFrom(textureID);
	}
	else {
		syncBuffer::readFrom(textureID);
	}
}
void rTile::draw(GLenum textureID) {
	fassert(position.right - position.left > 0);
	fassert(position.bottom - position.top > 0);

	// TODO: When density < 1.0f there are sometimes still blue / magenta vertical / horizontal lines in some Frames! Obviously, sometimes there are still too many pixels drawn / used for interpolation! (Not fixed!)

	rTile::bind(textureID);


	float effortQ = float(pow(float(double(rTile::samples) / double(rTile::getSize().area())), 0.5f) * rTile::maxDensity1D);

	// if effort>2.0f data is already scaled to 2.0f using Blit
	float mul = (minimum(effortQ, 2.0f) / rTile::maxDensity1D);
	if (rTile::useSwap) {
		rTile::quad.draw(rTile::position, ARect(0.0f, 0.0f, 
			(syncBuffer::iSize.w / float(rTile::swapBuffer->getSize().w)  ),
			(syncBuffer::iSize.h / float(rTile::swapBuffer->getSize().h))) * mul);
	}
	else {
		if (rTile::getIter() <= 0) {
			// Needs to correct Size by tileSize/size if one of the tiles is a little bit smaller
			rTile::quad.draw(rTile::position, ARect(0.0f, 0.0f,
				rTile::tileSize.w / float(rTile::size.w),
				rTile::tileSize.h / float(rTile::size.h)) * mul);
		}
		else {
			// No tilesize-Correction necessary! Texture is already scaled to optimal size!
			rTile::quad.draw(rTile::position, ARect(0.0f, 0.0f,1.0f, 1.0f) * mul);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       fOptimizer        ]*******************************/

fOptimizer::fOptimizer(double targetFrameRate, float maxDensity1D) 
	: inited(false), targetFrameRate(targetFrameRate), maxDensity1D(maxDensity1D), notAgain(0), changing(1.0),
	lastSleep(0) {

}
void fOptimizer::hint(double factor) {
	fOptimizer::changing *= factor;
}
void fOptimizer::optimize(sRenderer *renderer, uint64 samplesRendered) {
	// TODO: Implement parameterChange
	// TODO: For very intense jobs, switch to low-framerate mode to archieve higher samples-per-second scores!
	// TODO: Automatically choose good depth for the pRenderer!

	if (fOptimizer::inited == false) {
		fOptimizer::aSamples = 1000;
		if (samplesRendered > 0) {
			fOptimizer::inited = true;
			fOptimizer::floatingTimePublic = fOptimizer::floatingTime = QPC::get();
		}

	}
	else {
		double lastTime = QPC::get();
		if (samplesRendered > 0) {
			fOptimizer::floatingTime = (9.0*fOptimizer::floatingTime + lastTime) / 10.0;
		}
		fOptimizer::floatingTimePublic = (9.0*fOptimizer::floatingTimePublic + lastTime) / 10.0;
		double optimTime = 1000.0 / fOptimizer::targetFrameRate;
		double ratio = sqrt( optimTime / fOptimizer::floatingTime);
		notAgain--;

		double inverseCuEf = 1.0 / fOptimizer::changing;
		if (abs(inverseCuEf - 1.0f) > 0.1f) {
			// Something like a buffer scale caused rendering time to increase dramatically
			
			//if (inverseCuEf < ratio)				
			ratio = (2.0f * inverseCuEf + ratio) / 3.0f;
			notAgain = 0;

			//cout << "Hint: (" << ratio << ") ";
		}
		else {
			// Let it decay...
			fOptimizer::changing = (fOptimizer::changing + 1.0) / 2.0;
		}


		if (samplesRendered > 0 && renderer &&
			( (abs(ratio - 1.0f) > 0.1f && notAgain <= 0) 
				|| (abs(ratio - 1.0f) > 0.01f && notAgain < int(-3.0f* fOptimizer::targetFrameRate)))) {
			
			notAgain = 10;

			fOptimizer::changing = 1.0;

			//fOptimizer::aSamples = renderer->getSampleCount();
			fOptimizer::aSamples = uint64(round(double(fOptimizer::aSamples) * ratio));
			if (fOptimizer::aSamples < 100) fOptimizer::aSamples = 100; // Minimum Samples
			else if (fOptimizer::aSamples > double(fOptimizer::maxDensity1D*fOptimizer::maxDensity1D) * double(renderer->getSize().area())) fOptimizer::aSamples = double(fOptimizer::maxDensity1D*fOptimizer::maxDensity1D) * double(renderer->getSize().area());

			renderer->setSampleCount(fOptimizer::aSamples);

			//cout << fOptimizer::aEffort << endl;

		}
		
		// TODO: uncomment the condition! Therefore, fOptimizer needs to be improved!

		// Sleep if Framerate is highter than 60FPS
		if ((optimTime - lastTime > 1.0)/* && (fOptimizer::aEffort == fOptimizer::maxEffort)*/) {
			uint64 sleepTime = int(floor(optimTime - lastTime));
			if (sleepTime > (17 - int(ceil(fOptimizer::floatingTime)) + fOptimizer::lastSleep)) 
				sleepTime = 17 - int(ceil(fOptimizer::floatingTime)) + fOptimizer::lastSleep;
			if (sleepTime < 0) sleepTime = 0;
			if(sleepTime > 0) QPC::sleep(sleepTime);
			fOptimizer::lastSleep = sleepTime;
		}
	}

	if (renderer) renderer->setSampleCount(fOptimizer::aSamples);
}
uint64 fOptimizer::getSamples() const {
	return fOptimizer::aSamples;
}
void fOptimizer::setTargetFramerate(float targetFramerate) {
	fOptimizer::targetFrameRate = targetFramerate;
}
void fOptimizer::setMaxDensity1D(float maxDensity1D) {
	fOptimizer::maxDensity1D = maxDensity1D;
}
float fOptimizer::getFramerate() const {
	return float(1000.0 / fOptimizer::floatingTimePublic);
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        tRenderer        ]*******************************/

tRenderer::tRenderer(AiSize size, AiSize tiles, float maxEffort) : 
	samples(size.area()), tilec(0), tileArea(0.0f,0.0f,0.0f,0.0f) {
	tRenderer::setSize(size, tiles, maxEffort);
}
tRenderer::~tRenderer() {

}
void tRenderer::setSize(AiSize size, AiSize tiles, float maxDensity1D) {
	tRenderer::maxDensity1D = maxDensity1D;
	tRenderer::tiles = tiles;
	tRenderer::size = size;
	tRenderer::tile.reset(new rTile(AiSize( int(ceil(size.w / float(tiles.w))), int(ceil(size.h / float(tiles.h)))), tRenderer::maxDensity1D, (tiles.w*tiles.h == 1) ? GL_LINEAR : GL_NEAREST));
	tRenderer::tile->setSampleCount(tRenderer::samples);
}
bool tRenderer::renderTile(std::function<void(ARect tile)> content, uint64 &samplesRendered) {
	
	if (tRenderer::tilec >= tRenderer::tiles.w*tRenderer::tiles.h) {
		tRenderer::tilec = 0;
		return false;
	}

	int c = tRenderer::tilec;
	int x = c % tRenderer::tiles.w;
	int y = (c - x) / tRenderer::tiles.w;
	AiRect xywh = AiRect(int(ceil(tRenderer::size.w / float(tRenderer::tiles.w))) * x,
		int(ceil(tRenderer::size.h / float(tRenderer::tiles.h))) * y,
		int(ceil(tRenderer::size.w / float(tRenderer::tiles.w))),
		int(ceil(tRenderer::size.h / float(tRenderer::tiles.h))));
	if (x == tRenderer::tiles.w - 1) xywh.right = tRenderer::size.w - xywh.left;
	if (y == tRenderer::tiles.h - 1) xywh.bottom = tRenderer::size.h - xywh.top;
	
//	cout << c << " " << x << " " << y << " " << xywh.left << " " << xywh.top << " " << xywh.right << " " << xywh.bottom << endl;

//	tRenderer::tile->setEffortQ((c+1)*0.1f);

	tRenderer::tileArea = ARect(xywh.left / float(tRenderer::size.w), xywh.top / float(tRenderer::size.h),
		(xywh.left + xywh.right) / float(tRenderer::size.w), (xywh.top + xywh.bottom) / float(tRenderer::size.h));

	samplesRendered += tRenderer::tile->render(content, AiSize(xywh.right, xywh.bottom), tRenderer::tileArea);
	tRenderer::tilec++;
	return true;
}
void tRenderer::setSampleCount(uint64 samples) {
	tRenderer::samples = samples;
	tRenderer::tile->setSampleCount(samples);
}
uint64 tRenderer::getSampleCount() const {
	return tRenderer::samples;
}
void tRenderer::drawTile(GLenum textureID) {
	tRenderer::tile->draw(textureID);
}
AiSize tRenderer::getSize() const {
	return tRenderer::size;
}