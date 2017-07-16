/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: tiledRenderer.cpp
* Purpose: Divides Frame into set of tiles.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "renderer.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         rTile           ]*******************************/

rTile::rTile(AiSize size, float effort) :
	// Mipmaps are deactivated (caused artifacts). Instead, use a serious of linear copies! 
	syncBuffer(AiSize( int(ceil(float(size.w)*effort)), int(ceil(float(size.h)*effort))), /*effort > 2.f*/ false, GL_LINEAR, GL_NEAREST),
	size(size), effortQ(effort), effort(effort), position(0.0f,0.0f,0.0f,0.0f) {

	if (rTile::effort > 2.0f) {
		int iterations = rTile::getIter();
		swapBuffer.reset(new syncBuffer(rTile::size * (1 << iterations), false, GL_LINEAR, GL_NEAREST));
	}
}

rTile::~rTile() {

}

void rTile::render(std::function<void(ARect tile)> content, AiSize sizeI, ARect position) {
	// TODO: assert size and position and effortQ > 0
	int vw = (rTile::effortQ < rTile::effort) ? int(ceil(rTile::effortQ*float(sizeI.w))) : int(ceil(float(sizeI.w)*rTile::effort));
	int vh = (rTile::effortQ < rTile::effort) ? int(ceil(rTile::effortQ*float(sizeI.h))) : int(ceil(float(sizeI.h)*rTile::effort));

	rTile::position = position;
	glBindFramebuffer(GL_FRAMEBUFFER, syncBuffer::framebuffer);
	glErrors("tRenderer::bind");
	//cout << rTile::effortQ  << " " << size.w << " " << size.h << " | " << vw << " " << vh << endl;
	glViewport(0, 0, vw, vh);
	if(content) content(position);
	rTile::quad.draw(ARect(.0f,.0f,1.f,1.f), position, true);

	rTile::useSwap = false;

	if (syncBuffer::useMipmap) {
		glBindTexture(GL_TEXTURE_2D, syncBuffer::tex);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else if (rTile::effortQ > 2.f) {
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
		//	cout << vw << " " << vh << " | " << toString(smaller) << endl;
			glBlitFramebuffer(0, 0, last.w, last.h, 0, 0, smaller.w, smaller.h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			last = smaller;
			glErrors("rTile::blit");
		}
	}
}

void rTile::setEffortQ(float effortQ) {
	if (effortQ <= rTile::effort) {
		rTile::effortQ = effortQ;
	}		
	else {
		rTile::effortQ = rTile::effort;
		rTile::scale(rTile::size, effort);
	}
}

int rTile::getIter() {
	int iterations = int(floor(log2(rTile::effortQ)));
	// We don't need the additional iteration!
	if (float(iterations) == log2(rTile::effortQ))
		iterations--;
	return iterations;
}

void rTile::scale(AiSize size, float effort) {
	rTile::size = size;
	syncBuffer::scale(AiSize(int(ceil(size.w*effort)), int(ceil(size.h*effort))), false /* effort > 2.f*/);
	rTile::effort = effort;
	rTile::effortQ = rTile::effort;

	if (rTile::effort > 2.0f) {
		int iterations = rTile::getIter();
		if(swapBuffer.data())
			swapBuffer->scale(rTile::size * (1 << iterations), false);
		else 
			swapBuffer.reset(new syncBuffer(rTile::size * (1 << iterations), false, GL_LINEAR, GL_NEAREST));
	}
	else {
		swapBuffer.reset(0, false);
	}
}

void rTile::draw(GLuint textureID) {
	// TODO: When density < 1.0f there are sometimes still blue / magenta vertical / horizontal lines in some Frames! Obviously, sometimes there are still too many pixels drawn / used for interpolation! (Possibly fixed. Don't know.)


	// TODO: Rand wird nicht korrekt beruecksichtigt! Die Letzten Tiles mit angepasster Größe werden falsch angezeigt!

	// if effort>2.0f data is already scaled to 2.0f using Blit
	float mul = (minimum(rTile::effortQ, 2.0f) / rTile::effort);
	if (rTile::useSwap) {
		rTile::swapBuffer->readFrom(textureID);
		rTile::quad.draw(rTile::position, (ARect(0.0f, 0.0f, 
			syncBuffer::iSize.w / float(rTile::swapBuffer->getSize().w), 
			syncBuffer::iSize.h / float(rTile::swapBuffer->getSize().h))) * mul, true);
	}
	else {
		syncBuffer::readFrom(textureID);
		// if effort>2.0f data is already scaled to 2.0f using Blit
		rTile::quad.draw(rTile::position, (ARect(0.0f, 0.0f, 1.0f, 1.0f)) * mul, true);
	}

	//cout << position.left << " " << position.top << " " << position.right << " " << position.bottom << endl;
	//rTile::quad.draw(rTile::position, (ARect(0.0f,0.0f,1.0f,1.0f)) * ((rTile::effortQ < rTile::effort) ? (effortQ / effort) : 1.0f), true);

}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       fOptimizer        ]*******************************/

fOptimizer::fOptimizer(float targetFrameRate, float maxEffort) 
	: inited(false), targetFrameRate(targetFrameRate), maxEffort(maxEffort), notAgain(0), cuEffort(1.0f),
	lastSleep(0) {

}
void fOptimizer::hint(float factor) {
	fOptimizer::cuEffort *= factor;
}
void fOptimizer::optimize(sRenderer *renderer) {
	// TODO: Implement parameterChange
	if (fOptimizer::inited == false) {
		fOptimizer::inited = true;
		renderer->setEffort(0.1f);
		fOptimizer::floatingTime = QPC::get();
	}
	else {
		double lastTime = QPC::get();
		fOptimizer::floatingTime = (9.0*fOptimizer::floatingTime + lastTime) / 10.0;
		float optimTime = 1000.0f / fOptimizer::targetFrameRate;
		float ratio = sqrt( optimTime / float(fOptimizer::floatingTime) );
		notAgain--;

		float inverseCuEf = 1.0f / fOptimizer::cuEffort;
		if (abs(inverseCuEf - 1.0f) > 0.1f) {
			// Something like a buffer scale caused rendering time to increase dramatically
			
			//if (inverseCuEf < ratio)				
			ratio = (2.0f * inverseCuEf + ratio) / 3.0f;
			notAgain = 0;

			//cout << "Hint: (" << ratio << ") ";
		}
		else {
			// Let it decay...
			fOptimizer::cuEffort = (fOptimizer::cuEffort + 1.0f) / 2.0f;
		}


		if ( (abs(ratio - 1.0f) > 0.1f && notAgain <= 0) || (abs(ratio - 1.0f) > 0.01f && notAgain < int(-3.0f* fOptimizer::targetFrameRate))) {
			notAgain = 10;

			fOptimizer::cuEffort = 1.0f;

			fOptimizer::aEffort = renderer->getEffort();
			fOptimizer::aEffort *= ratio;
			if (fOptimizer::aEffort < 0.01f) fOptimizer::aEffort = 0.01f;
			else if (fOptimizer::aEffort > fOptimizer::maxEffort) fOptimizer::aEffort = fOptimizer::maxEffort;

			renderer->setEffort(fOptimizer::aEffort);

			//cout << fOptimizer::aEffort << endl;

		}

		// Sleep if Framerate is highter than 60FPS
		if ((optimTime - lastTime > 1.0) && (fOptimizer::aEffort == fOptimizer::maxEffort)) {
			int sleepTime = int(floor(optimTime - lastTime));
			if (sleepTime > (17 - int(ceil(fOptimizer::floatingTime)) + fOptimizer::lastSleep)) 
				sleepTime = 17 - int(ceil(fOptimizer::floatingTime)) + fOptimizer::lastSleep;
			if (sleepTime < 0) sleepTime = 0;
			if(sleepTime > 0) QPC::sleep(sleepTime);
			fOptimizer::lastSleep = sleepTime;
		}
	}
}
float fOptimizer::getPixelDensity() const {
	return fOptimizer::aEffort;
}
void fOptimizer::setTargetFramerate(float targetFramerate) {
	fOptimizer::targetFrameRate = targetFramerate;
}
void fOptimizer::setMaxEffort(float maxEffort) {
	fOptimizer::maxEffort = maxEffort;
}
float fOptimizer::getFramerate() const {
	return float(1000.0 / fOptimizer::floatingTime);
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        tRenderer        ]*******************************/

tRenderer::tRenderer(AiSize size, AiSize tiles, float maxEffort) : 
	effort(0.1f), tilec(0), tileArea(0.0f,0.0f,0.0f,0.0f) {
	tRenderer::setSize(size, tiles, maxEffort);
}

tRenderer::~tRenderer() {

}

void tRenderer::setSize(AiSize size, AiSize tiles, float maxEffort) {
	tRenderer::maxEffort = maxEffort;
	tRenderer::tiles = tiles;
	tRenderer::size = size;
	tRenderer::tile.reset(new rTile(AiSize( int(ceil(size.w / float(tiles.w))), int(ceil(size.h / float(tiles.h)))), tRenderer::maxEffort));
	tRenderer::tile->setEffortQ(tRenderer::effort);
}

bool tRenderer::renderTile(std::function<void(ARect tile)> content) {
	
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

	tRenderer::tile->render(content, AiSize(xywh.right, xywh.bottom), tRenderer::tileArea);
	tRenderer::tilec++;
	return true;
}


void tRenderer::setEffort(float effort) {
	tRenderer::effort = effort;
	tRenderer::tile->setEffortQ(effort);
}
float tRenderer::getEffort() const {
	return tRenderer::effort;
}

void tRenderer::drawTile(GLuint textureID) {
	tRenderer::tile->draw(textureID);
}