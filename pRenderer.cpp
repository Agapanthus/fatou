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

#define USE_INTERPOLATION
#define INTERP_POINTS 4

//#define MEASURE_PERFORMANCE

// TODO: Check GL / GLSL-Version and support of used functions in the beginning of the program!

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Buffer    ]*******************************/



const string whiteShader("void main() {"
	"gl_FragColor = vec4(1.0f,1.0f,1.0f,1.0f);"
	"}");


#ifdef USE_INTERPOLATION

const string composerShader(
	"uniform sampler3D tex;"
	// x are iterations and y*INTERP_POINTS are layers. Those INTERP_POINTS Pixels are each one point to use for interpolation:
	// (coordinate.x, coordinate.y, layer, weight)
	// If pixel[0].weight == 0.0f, then the value of the first Position should be copied without interpolation.
	"uniform sampler2D interPM;"
	"in vec2 TexCoords;"
	"uniform int iteration;"
	"uniform int queue_r;"
	"layout(pixel_center_integer) in vec4 gl_FragCoord;"
	

	"void main() {"
	"	int x = int(gl_FragCoord.x) % queue_r;"
	"	int y = int(gl_FragCoord.y) % queue_r;"
	"	int layerAddress = (x + (queue_r*y)) * " + toString(INTERP_POINTS) + ";"
	"	vec4 p1 = texelFetch(interPM, ivec2(iteration, layerAddress), 0);"
	// We don't need that. Due to lower resolution, sampler3D with GL_NEAREST does this automatically!
	//"	vec2 offset = vec2(TexCoords.x - (float(x) * iWinSize.x), TexCoords.y - (float(y) * iWinSize.y) );"
	// Just load the Pixel
	"	if(p1[3] == 0.0f) {"
	"		gl_FragColor = texture(tex, vec3(TexCoords.x + p1[0], TexCoords.y + p1[1], p1[2]));"
	// Interpolate Pixels
	"	} else {"
	"		vec4 p2 = texelFetch(interPM, ivec2(iteration, layerAddress+1), 0);"
	"		vec4 p3 = texelFetch(interPM, ivec2(iteration, layerAddress+2), 0);"
	"		vec4 p4 = texelFetch(interPM, ivec2(iteration, layerAddress+3), 0);"
	"		gl_FragColor =vec4( (p1[3] * texture(tex, vec3(TexCoords.x + p1[0], TexCoords.y + p1[1], p1[2])).xyz)"
	"					+ (p2[3] * texture(tex, vec3(TexCoords.x + p2[0], TexCoords.y + p2[1], p2[2])).xyz)"
	"					+ (p3[3] * texture(tex, vec3(TexCoords.x + p3[0], TexCoords.y + p3[1], p3[2])).xyz)"
	"					+ (p4[3] * texture(tex, vec3(TexCoords.x + p4[0], TexCoords.y + p4[1], p4[2])).xyz), 1.0f);"
	"	}"
	"}");

#else

// Old Version

/*
const string composerShader(
	"uniform sampler3D tex;"
	"in vec2 TexCoords;"
	"uniform float queue_l;"
	"uniform float maxZ;"
	"uniform ivec2 winSize;"
	"uniform int queue_r;"
	"uniform vec2 scale;"

	"void main() {"
	// we use ceil(x)-1 instead of floor(x), because we want [n,n+1[ to become n, not ]n,n+1]
	// The first column / row becomes therefor negative, but by using GL_CLAMP_TO_EDGE this isn't a problem!
	// TODO: But... wait! Isn't this a bug? Doesn't this implicate that the first rects have one extra pixel? And which pixel is missing? In reality, there doesn't seem to be any duplicate pixels!! So, why is this working?!
	"	int x = (int(ceil(winSize.x * TexCoords.x)) - 1) % queue_r;"
	"	int y = (int(ceil(winSize.y * TexCoords.y)) - 1) % queue_r;"
	"	float layer = (x + (queue_r*y)) / (queue_l-1);"
	"	if(layer > maxZ) layer = maxZ;"
	//	"	gl_FragColor = texture(tex, vec3(TexCoords, layer))*0.0 + vec4(layer, layer,0.0f, 1.0f);\n"
	"	gl_FragColor = texture(tex, vec3(TexCoords.x * scale.x , TexCoords.y * scale.y, layer));\n"
	"}");
	*/

// New, faster Version
const string composerShader(
	"uniform sampler3D tex;"
	"in vec2 TexCoords;"
	"uniform float queue_l;"
	"uniform float maxZ;"
	"uniform int queue_r;"
	"layout(pixel_center_integer) in vec4 gl_FragCoord;"

	"void main() {"
	"	int x = int(gl_FragCoord.x) % queue_r;"
	"	int y = int(gl_FragCoord.y) % queue_r;"
	"	float layer = (x + (queue_r*y)) / (queue_l-1);"
	"	if(layer > maxZ) layer = maxZ;"
	"	gl_FragColor = texture(tex, vec3(TexCoords.x , TexCoords.y, layer));\n"
	"}");

#endif

int roundUpToNextMultipleOfN(int numToRound, int N) {
	if (N == 0)
		return numToRound;
	int remainder = numToRound % N;
	if (remainder == 0)
		return numToRound;
	return numToRound + N - remainder;
}

pBuffer::pBuffer(AiSize size) :
	syncBuffer(size, false, GL_LINEAR, GL_NEAREST), size(0, 0) {

	// Check if there are enough texture units for interpolation
	/*	GLint texture_units = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
	glErrors("pRenderer::getMaxTextureImageUnits");
	if (texture_units < QUEUE_LENGTH) {
	fatalNote("GPU doesn't support " << QUEUE_LENGTH << " textures per fragment shader");
	}*/

	white.reset(new shader(mainVertexShader, whiteShader));

	pBuffer::composer.reset(new shader(mainVertexShader, composerShader));

	pBuffer::composer->use();
	uniform.texture = pBuffer::composer->getUniform("tex");
	uniform.queue_r = pBuffer::composer->getUniform("queue_r");
#ifdef USE_INTERPOLATION
	uniform.interPM = pBuffer::composer->getUniform("interPM");
	uniform.iteration = pBuffer::composer->getUniform("iteration");
//	uniform.iWinSize = pBuffer::composer->getUniform("iWinSize");
	glUniform1i(uniform.interPM, 1);
#else
	//uniform.scale = pBuffer::composer->getUniform("scale");
	//uniform.winSize = pBuffer::composer->getUniform("winSize");
	uniform.maxZ = pBuffer::composer->getUniform("maxZ");
	uniform.queue_l = pBuffer::composer->getUniform("queue_l");
#endif
	glUniform1i(uniform.texture, 0);

	pBuffer::scale(size);
	//glGenQueries(QUEUE_LENGTH, pBuffer::queries);
}
pBuffer::~pBuffer() {

}

void pBuffer::scale(AiSize size) {
	fassert(QUEUE_LENGTH > 0);

	if (size != pBuffer::size) {
		if (size != syncBuffer::iSize)
			syncBuffer::scale(size, false);

		if (buffer.empty())
			buffer.reset(new syncBuffer3d(((size + QUEUE_LENGTH_R - 1) / QUEUE_LENGTH_R), QUEUE_LENGTH, GL_NEAREST, GL_NEAREST));
		else
			buffer->scale(((size + QUEUE_LENGTH_R - 1) / QUEUE_LENGTH_R));

		if (coeffBuffer.empty()) {
			coeffBuffer.reset(new floatStorage(AiSize(QUEUE_LENGTH, QUEUE_LENGTH*INTERP_POINTS)));
			recalculateCoeff(); // TODO: When changing QUEUE_LENGTH this needs to be rerun!
		}
		else {
			coeffBuffer->scale(AiSize(QUEUE_LENGTH, QUEUE_LENGTH*INTERP_POINTS));
			recalculateCoeff(); // TODO: When changing QUEUE_LENGTH this needs to be rerun!
		}


	}
	pBuffer::size = size;
	pBuffer::posx = 0;
	pBuffer::currentBuffer = 0;
}

inline float normLayer(float l) {
	float layer = l / float(QUEUE_LENGTH - 1);
	// Very strange thing, but sampler3d returns all zero for z==1.0f...
	//if (layer == 1.0f) layer = 0.999999f; 
	// (Instead, I'm using GL_CLAMP_TO_EDGE for now.)
	return layer;
}

inline float netDistance(const AiSize &tile, const AiPoint &a, const AiPoint &b, ASize &relative) {
	// tile/2 - a        | Define coordinate system with a centered in the tile. Now the distance from a (the origin) to every point p inside the tile is <= the distance from a to each point p' in other tiles.
	// + b				 | Place point b in this coordinate system
	AiSize dist(b - a + tile/2);
	// If dist.w<0 then b'.x from the right neighbouring tile is closer to a.x than b.x
	if (dist.w < 0) {
		dist.w += tile.w;
		relative.w += 1.0f;
	}
	// if dist.w>=tile.w then b'.x from the left neighbouring tile is closer to a.x than b.x
	else if (dist.w > tile.w) {
		dist.w -= tile.w;
		relative.w -= 1.0f;
	}
	if (dist.h < 0) {
		dist.h += tile.h;
		relative.h += 1.0f;
	} else if (dist.h > tile.h) {
		dist.h -= tile.h;
		relative.h -= 1.0f;
	}
	// Move back to the origin [ a = (0,0) ] and then calculate magnitude
	fassert((dist - (tile / 2)).magnitude() <= AiSize(QUEUE_LENGTH_X, QUEUE_LENGTH_Y).magnitude());
	return (dist - (tile / 2)).magnitude();
}


// TODO: Make it use other equivalent points from other tiles, when there are less than INTERP_POINTS samples!
// TODO: Do this asynchronously!
// TODO: Use prettier algorithm: faster and better (smoother?) interpolation!
void pBuffer::recalculateCoeff() {
	// Defines the order of layers (maps a layer to each iteration)
	permutationMap.resize(QUEUE_LENGTH);
	for (int l = 0; l < QUEUE_LENGTH; l++) { 
		permutationMap[l] = QUEUE_LENGTH-1- l;
	}
	random_shuffle(std::begin(permutationMap), std::end(permutationMap));

	for (size_t i = 0; i < QUEUE_LENGTH; i++) { // Iterations
		for (size_t l = 0; l < QUEUE_LENGTH; l++) { // Layers
			//for(size_t p = 0; p < INTERP_POINTS; p++) { // Points					
			
			if (i < l) {
				AiPoint target(permutationMap[l] % QUEUE_LENGTH_R, permutationMap[l] / QUEUE_LENGTH_R);

		//		cout << "\n\n////////////////////////////////\nIL: " << i << " " << l << " " << toString(target) << endl;
				
				// Find n closest points
				struct pointStruct {
					pointStruct() : layer(0), relativePosition(0.0f,0.0f), dist(0.0f), relevance(0.0f) {}
					int32 layer;
					APoint relativePosition;
					float dist;
					float relevance;
				} points[INTERP_POINTS];

				for (size_t p = 0; p < INTERP_POINTS; p++) { // Points		

					for (size_t w = 0; w < QUEUE_LENGTH; w++) {
						// Ignore Points which are already used
						bool found = false;
						for (size_t r = 0; r < p; r++) {
							if (points[r].layer == w) {
								found = true;
								break;
							}
						}
						if (found) continue;

						// Only look on layers already written to
						if (w <= i) {
							// Measure distance
							ASize relative(0.0f,0.0f);
							float dist = netDistance(AiSize(QUEUE_LENGTH_X, QUEUE_LENGTH_Y), target, AiPoint(permutationMap[w] % QUEUE_LENGTH_R, permutationMap[w] / QUEUE_LENGTH_R), relative);
							
							// Use this point, if it is the first one or it is closer to the target
							if (dist < points[p].dist || points[p].dist == 0.0f) {
								points[p].layer = w;
								points[p].dist = dist;
								points[p].relativePosition = relative;
							//	cout << x << " " << y << "    " << xw << " " << yw << endl;
							}
						}
					}
					// Result will depend exponentially on the distance and longer distances give smaller relevances
					points[p].relevance = pow( 3.0f - 2.0 * sqrt(i / float(QUEUE_LENGTH)), -points[p].dist);
					// zero should stay zero
					if (points[p].dist == 0.0f) points[p].relevance = 0.0f;
				}
				float sumRel = 0.0f;
				for (size_t p = 0; p < INTERP_POINTS; p++) { // Points		
					sumRel += points[p].relevance;
				}
				fassert(points[0].relevance != 0.0f); // This is a reserved value! (See fragment shader)
				fassert(sumRel > 0.0f);
				for (size_t p = 0; p < INTERP_POINTS; p++) { // Points		
					coeffBuffer->set(i, permutationMap[l]*INTERP_POINTS + p,
						points[p].relativePosition.x / float(buffer->getSize().w),
						points[p].relativePosition.y / float(buffer->getSize().h),
						normLayer(float(permutationMap[points[p].layer])),
						points[p].relevance / sumRel);
		//			cout << "Dist: " << points[p].dist << " " << points[p].relevance / sumRel << " " << points[p].layer << " " << toString(points[p].relativePosition) << endl;
				}


			} else {
				coeffBuffer->set(i, permutationMap[l]*INTERP_POINTS + 0, 0.0f, 0.0f, normLayer(float(permutationMap[l])), 0.0f);
			}
			//}

			//if (i == 5 && l == 4)Sleep(20000000);
		}
	}
	cout << "done" << endl;
	//Sleep(100000);

	coeffBuffer->upload();
}

void pBuffer::draw(int x, int y) {
	if (pBuffer::currentBuffer > 0) {
		syncBuffer::readFrom();
		pBuffer::quad.draw();
	}
	
#if 0
	// To visualize, of which other pixels each composed pixel consists
	if (x >= 0 && y >= 0) {
		/*
		// Smoothing need Antialiasing: glfwWindowHint(GLFW_SAMPLES, 4);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		*/

		static float lmposx = -1.0f;
		static float lmposy = -1.0f;
		
		float mposx = x / float(size.w);
		float mposy = 1.0f - y / float(size.h);

		int xz = (x % QUEUE_LENGTH_X);
		int yz = QUEUE_LENGTH_Y - 1 - (y % QUEUE_LENGTH_Y);
		int l = xz + QUEUE_LENGTH_X*yz;
		const int iter = 13;
		int value = 0;
		white->use();

		if (coeffBuffer->get(iter, l*INTERP_POINTS + 0, 3) != 0.0f) {
			for (size_t p = 0; p < INTERP_POINTS; p++) {
				value = p;

				if (coeffBuffer->get(iter, l*INTERP_POINTS + value, 3) != 0.0f) {

					int tx = int(coeffBuffer->get(iter, l*INTERP_POINTS + value, 2)*QUEUE_LENGTH) % QUEUE_LENGTH_X;
					int ty = int(coeffBuffer->get(iter, l*INTERP_POINTS + value, 2)*QUEUE_LENGTH) / QUEUE_LENGTH_X;


					if (lmposx != mposx || lmposy != mposy) {
				//		cout << "pointing: " << xz << " " << yz << "    rel: " << coeffBuffer->get(iter, l*INTERP_POINTS + value, 0) << " " << coeffBuffer->get(iter, l*INTERP_POINTS + value, 1) << "  l: " << coeffBuffer->get(iter, l*INTERP_POINTS + value, 2) << "    target: " << tx << " " << ty << endl;
					}
					APoint source(mposx + (tx - xz) / float(size.w) + coeffBuffer->get(iter, l*INTERP_POINTS + value, 0),
						mposy + (ty - yz) / float(size.h) + coeffBuffer->get(iter, l*INTERP_POINTS + value, 1));

					line.draw(ARect(source.x, source.y, mposx, mposy));
				}
			}
		}
		lmposx = mposx;
		lmposy = mposy;
	}
#endif
}
void pBuffer::render(float effort, function<void(void)> renderF) {

	bool changed = false;
#ifdef MEASURE_PERFORMANCE
	glQueryCreator gqt;
	{
		glQuery q = gqt.create();
#endif

		while (effort > 0.0f) {
			if (pBuffer::currentBuffer == QUEUE_LENGTH) break;

			int posxnew = minimum(int(buffer->getSize().w), int(round(posx + (float(QUEUE_LENGTH) * effort * float(buffer->getSize().w)))));
			if (posxnew == posx) {
				break;
			}
			ASize ratio(ASize(buffer->getSize() * QUEUE_LENGTH_R) / ASize(size));
			ARect part(float(posx) / float(buffer->getSize().w), 0.0f, float(posxnew) / float(buffer->getSize().w), 1.0f);
			ARect partT = part * ASize(ratio.w, ratio.h);

			pBuffer::posx = posxnew;

			ASize delta(((permutationMap[pBuffer::currentBuffer] % QUEUE_LENGTH_R) + 0.5f) / float(QUEUE_LENGTH_R) - 0.5f,
				((permutationMap[pBuffer::currentBuffer] / QUEUE_LENGTH_R) + 0.5f) / float(QUEUE_LENGTH_R) - 0.5f);
			delta = delta / ASize(buffer->getSize());

			buffer->writeTo(permutationMap[pBuffer::currentBuffer]);
			renderF();
			quad.draw(part, partT + delta);
			if (posxnew == buffer->getSize().w) {
				pBuffer::currentBuffer++;
				changed = true;
				pBuffer::posx = 0;
			}
			effort -= (partT.right - partT.left) / float(QUEUE_LENGTH);
		}
#ifdef MEASURE_PERFORMANCE
	}
	if(changed)	cout << "Render: " << gqt.getTime() << " ms   ";
	{
		glQuery q = gqt.create();
#endif
		// TODO: Improve performance by using Array Textures with Geometry shader (No FBO's!), not using Texel Fetch and composing less! 
		if (changed) // TODO: Composing is quite time consuming... Maybe it's a good idea to give some option like "compose at most 3 times per second" (HINT: Composing 4k takes between 8 and 60 ms on my Thinkpad T540p, depending on the number of layers and the number of interpolated texels)
			pBuffer::compose();

#ifdef MEASURE_PERFORMANCE
	}
	if (changed) cout << "Compose: " << gqt.getTime() << " ms" << endl;
#endif
}
void pBuffer::compose() {

	buffer->bind(GL_TEXTURE0);
	coeffBuffer->bind(GL_TEXTURE1);

	syncBuffer::writeTo([this](void)->void {
		composer->use();
#ifdef USE_INTERPOLATION
		glUniform1i(uniform.iteration, maximum(0, int32(pBuffer::currentBuffer) - 1)); //  minimum(24U, pBuffer::currentBuffer));
	//	cout << currentBuffer << endl;
	//	glUniform2f(uniform.iWinSize, 1.0f/size.w, 1.0f/size.h);
#else
		glUniform1f(uniform.maxZ, maximum(0, int32(pBuffer::currentBuffer) - 1) / maximum(1.0f, float(QUEUE_LENGTH)));
		glUniform1f(uniform.queue_l, float(QUEUE_LENGTH));
		//glUniform2f(uniform.scale, float(size.w) / float(buffer->getSize().w*QUEUE_LENGTH_R), float(size.h) / float(buffer->getSize().h*QUEUE_LENGTH_R));
		//glUniform2i(uniform.winSize, size.w, size.h);
#endif
		glUniform1i(uniform.queue_r, (QUEUE_LENGTH_R));
		
		glErrors("pBuffer::composeUniform");

		ARect part(0.0f, 0.0f, 1.0f, 1.0f);
		ARect partT(part);
		ASize Relation(float(pBuffer::size.w) / float(pBuffer::buffer->getSize().w * QUEUE_LENGTH_R),
			float(pBuffer::size.h) / float(pBuffer::buffer->getSize().h * QUEUE_LENGTH_R));
		partT = partT * Relation;

		pBuffer::quad.draw(part, partT);
	});

}

void pBuffer::discard() {
	pBuffer::posx = 0;
	pBuffer::currentBuffer = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Renderer  ]*******************************/

pRenderer::pRenderer(AiSize size, float maxEffort) : size(size), maxEffort(maxEffort), effort(0.1f) {
	pRenderer::buffer.reset(new pBuffer(AiSize(int(ceil(size.w * maxEffort)), int(ceil(size.h*maxEffort)))));
}

pRenderer::~pRenderer() {

}

void pRenderer::render(function<void(void)> renderF, bool discard) {
	if(discard)	buffer->discard();
	buffer->render(pRenderer::effort / pRenderer::maxEffort, renderF);
}

void pRenderer::setSize(AiSize size, float maxEffort) {
	pRenderer::maxEffort = maxEffort;
	pRenderer::size = size;
	buffer->scale(AiSize(int(ceil(size.w * maxEffort)), int(ceil(size.h*maxEffort))));
}

void pRenderer::setEffort(float effort) {
	fassert(pRenderer::maxEffort >= effort);

	effort = 1.0f / QUEUE_LENGTH; 
	
	pRenderer::effort = effort;
}
float pRenderer::getEffort() const {
	return pRenderer::effort;
}
void pRenderer::draw(int x, int y) {
	buffer->draw(x, y);
}
