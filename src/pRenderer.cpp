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


// TODO: Check GL / GLSL-Version and support of used functions in the beginning of the program!

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Buffer    ]*******************************/

const string whiteShader("void main() {"
	"gl_FragColor = vec4(1.0f,1.0f,1.0f,1.0f);"
	"}");

#ifdef USE_INTERPOLATION

// TODO: This is by far to slow! -> TexelFetch is much slower than texture!

const string composerShader(
#ifdef USE_TEXTURE_ARRAY
	"uniform sampler2DArray tex;"
#else
	"uniform sampler3D tex;"
#endif
	// x are iterations and y*INTERP_POINTS are layers. Those INTERP_POINTS Pixels are each one point to use for interpolation:
	// (coordinate.x, coordinate.y, layer, weight)
	// If pixel[0].weight == 0.0f, then the value of the first Position should be copied without interpolation.
	"uniform sampler2D interPM;"
	"in vec2 TexCoords;"
	"uniform int queue_r;"
	"layout(pixel_center_integer) in vec4 gl_FragCoord;"

#ifdef USE_TEXEL_FETCH
	"uniform int iteration;"
#else
	"uniform float inverse_queue_l_minus_one;"
	"uniform float iteration;"
#endif

	"void main() {"
	"	int x = int(gl_FragCoord.x) % queue_r;"
	"	int y = int(gl_FragCoord.y) % queue_r;"
#ifdef USE_TEXEL_FETCH
	"	int layerAddress = (x + (queue_r*y)) * " + toString(INTERP_POINTS) + ";"
	"	vec4 p1 = texelFetch(interPM, ivec2(iteration, layerAddress), 0);"
#else
	//"	float layerAddress = (x + (queue_r*y)) * inverse_queue_l_minus_one;"
	"	float layerAddress = (x + (queue_r*y)) / (24);"
	"	vec4 p1 = texture(interPM, vec2(iteration, layerAddress), 0);"
#endif
	// We don't need that. Due to lower resolution, sampler3D with GL_NEAREST does this automatically!
	//"	vec2 offset = vec2(TexCoords.x - (float(x) * iWinSize.x), TexCoords.y - (float(y) * iWinSize.y) );"
	// Just load the Pixel
#ifndef DISABLE_BRANCHING
	"	if(p1[3] == 0.0f) {"
	"		gl_FragColor = texture(tex, vec3(TexCoords.x + p1[0], TexCoords.y + p1[1], p1[2]));"
	// Interpolate Pixels
	"	} else {"
#endif
#ifdef USE_TEXEL_FETCH
	"		vec4 p2 = texelFetch(interPM, ivec2(iteration, layerAddress+1), 0);"
	"		vec4 p3 = texelFetch(interPM, ivec2(iteration, layerAddress+2), 0);"
	"		vec4 p4 = texelFetch(interPM, ivec2(iteration, layerAddress+3), 0);"
#else
	"		float layerStep = inverse_queue_l_minus_one / " + toString(INTERP_POINTS) + ";"
	"		vec4 p2 = texture(interPM, vec2(iteration, layerStep   + layerAddress), 0);"
	"		vec4 p3 = texture(interPM, vec2(iteration, layerStep*2 + layerAddress), 0);"
	"		vec4 p4 = texture(interPM, vec2(iteration, layerStep*3 + layerAddress), 0);"
#endif
#ifdef USE_DOTPRODUCT
	"		vec4 weights = vec4(p1[3], p2[3], p3[3], p4[3]);"
	"		vec4 t1 = texture(tex, vec3(TexCoords.x + p1[0], TexCoords.y + p1[1], p1[2]));"
	"		vec4 t2 = texture(tex, vec3(TexCoords.x + p2[0], TexCoords.y + p2[1], p2[2]));"
	"		vec4 t3 = texture(tex, vec3(TexCoords.x + p3[0], TexCoords.y + p3[1], p3[2]));"
	"		vec4 t4 = texture(tex, vec3(TexCoords.x + p4[0], TexCoords.y + p4[1], p4[2]));"
	"		vec4 valuesr = vec4(t1.r, t2.r, t3.r, t4.r);"
	"		vec4 valuesg = vec4(t1.g, t2.g, t3.g, t4.g);"
	"		vec4 valuesb = vec4(t1.b, t2.b, t3.b, t4.b);"
	"		gl_FragColor = vec4( dot(weights, valuesr), dot(weights, valuesg), dot(weights, valuesb), 1.0f);"
#else
	"		gl_FragColor =vec4( (p1[3] * texture(tex, vec3(TexCoords.x + p1[0], TexCoords.y + p1[1], p1[2])).xyz)"
	"					+ (p2[3] * texture(tex, vec3(TexCoords.x + p2[0], TexCoords.y + p2[1], p2[2])).xyz)"
	"					+ (p3[3] * texture(tex, vec3(TexCoords.x + p3[0], TexCoords.y + p3[1], p3[2])).xyz)"
	"					+ (p4[3] * texture(tex, vec3(TexCoords.x + p4[0], TexCoords.y + p4[1], p4[2])).xyz), 1.0f);"
#endif
#ifndef DISABLE_BRANCHING
	"	}"
#endif
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
#ifdef USE_TEXTURE_ARRAY
	"uniform sampler2DArray tex;"
#else
	"uniform sampler3D tex;"
#endif
	"in vec2 TexCoords;"
	"uniform float queue_l;"
	"uniform float maxZ;"
	"uniform int queue_r;"
	"layout(pixel_center_integer) in vec4 gl_FragCoord;"

	"void main() {"
	"	int x = int(gl_FragCoord.x) % queue_r;"
	"	int y = int(gl_FragCoord.y) % queue_r;"
#ifdef USE_TEXTURE_ARRAY
	"	float layer = queue_l*maxZ;"
	"	layer = (x + (queue_r*y));"
#else
	"	float layer = (x + (queue_r*y)) / (queue_l-1);"
	"	if(layer > maxZ) layer = maxZ;"
#endif
	"	gl_FragColor = texture(tex, vec3(TexCoords.x , TexCoords.y, layer));\n"
	"}");

#endif

#define SWITCH10(A) A = ((A) == 0) ? 1 : 0

pBuffer::pBuffer(AiSize size) : size(0, 0) {

	

	pBuffer::buffers[0].reset(new syncBuffer(size, false, GL_LINEAR, GL_LINEAR));
	pBuffer::buffers[1].reset(new syncBuffer(size, false, GL_LINEAR, GL_LINEAR));
	pBuffer::readBuffer = 0;
	pBuffer::writeBuffer = 0;
	pBuffer::rPosition[0] = APoint(0.0f, 0.0f);
	pBuffer::rPosition[1] = APoint(0.0f, 0.0f);
	pBuffer::rZoom[0] = ASize(1.0f, 1.0f);
	pBuffer::rZoom[1] = ASize(1.0f, 1.0f);

	pBuffer::white.reset(new shader(mainVertexShader, whiteShader));
	
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
#ifndef USE_TEXEL_FETCH
	uniform.inverse_queue_l_minus_one = pBuffer::composer->getUniform("inverse_queue_l_minus_one");
#endif
	glUniform1i(uniform.texture, 0);

	pBuffer::scale(size);
}
pBuffer::~pBuffer() {}

void pBuffer::scale(AiSize size) {
	fassert(QUEUE_LENGTH > 0);

	if (size != pBuffer::size) {
		if (size != pBuffer::buffers[0]->getSize()) {
			pBuffer::buffers[0]->scale(size, false);
			pBuffer::buffers[1]->scale(size, false);
		}

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

inline float normLayer(float lr) {
#ifdef USE_TEXTURE_ARRAY
	return lr;
#else
	float layer = lr / float(QUEUE_LENGTH - 1);
	// Very strange thing, but sampler3d returns all zero for z==1.0f...
	//if (layer == 1.0f) layer = 0.999999f; 
	// (Instead, I'm using GL_CLAMP_TO_EDGE for now.)
	return layer;
#endif
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

inline float netDistanceExplicit(const AiSize &tile, const AiPoint &a, const AiPoint &b, AiSize relative) {
	return (a - (relative * tile + b)).magnitude();
}

inline void pBuffer::initIdentity(int32 i, int32 lr) {
#ifdef DISABLE_BRANCHING
	coeffBuffer->set(i, permutationMap[l] * INTERP_POINTS + 0, 0.0f, 0.0f, normLayer(float(permutationMap[lr])), 1.0f);
	coeffBuffer->set(i, permutationMap[l] * INTERP_POINTS + 1, 0.0f, 0.0f, normLayer(float(permutationMap[lr])), 0.0f);
	coeffBuffer->set(i, permutationMap[l] * INTERP_POINTS + 2, 0.0f, 0.0f, normLayer(float(permutationMap[lr])), 0.0f);
	coeffBuffer->set(i, permutationMap[l] * INTERP_POINTS + 3, 0.0f, 0.0f, normLayer(float(permutationMap[lr])), 0.0f);
#else
	coeffBuffer->set(i, permutationMap[lr] * INTERP_POINTS + 0, 0.0f, 0.0f, normLayer(float(permutationMap[lr])), 0.0f);
#endif
}

struct pointStruct {
	pointStruct() :
		layer(0), relativePosition(0.0f, 0.0f), interest(0.0f), relevance(0.0f), normVec(0.0f, 0.0f), dist(0.0f) {}
	pointStruct(int layer, APoint relativePosition, APoint normVec, float dist, float relevance) :
		layer(layer), relativePosition(relativePosition), normVec(normVec), dist(dist), relevance(relevance), interest(0.0f) {}
	int32 layer;
	APoint relativePosition;
	float interest;
	APoint normVec;
	float relevance;
	float dist;
};

inline void insertPoint(pointStruct *const ps, pointStruct point) {
	int32 p;
	for (p = INTERP_POINTS; p > 0 ? ps[p - 1].dist > point.dist || ps[p - 1].dist == 0.0f : false; p--) {
		if (p < INTERP_POINTS) ps[p] = ps[p - 1];
	}
	if (p < INTERP_POINTS) ps[p] = point;
}

// TODO: Do this asynchronously!
void pBuffer::recalculateCoeff() {
	// Defines the order of layers (maps a layer to each iteration) 
	// We use random_shuffle, because we want a uniform distribution over time. 
	permutationMap.resize(QUEUE_LENGTH);
	for (int lr = 0; lr < QUEUE_LENGTH; lr++) {
		permutationMap[lr] = QUEUE_LENGTH - 1 - lr;
	}
	std::random_shuffle(std::begin(permutationMap), std::end(permutationMap));


	//////////////////////
	// Every Pixel is calculated as the weighted sum of the INTERP_POINTS closest points in it's surrounding.
	// Optionally, the algorithm will favour points which come from most different directions (to ignore points, which are "hidden" behind another).

	// The last one (points[n][INTERP_POINTS-1]) is always the worst (the one with the largest dist)
	pointStruct points[QUEUE_LENGTH][INTERP_POINTS];

	// Do for every iteration...
	for (int32 i = 0; i < QUEUE_LENGTH; i++) {
		AiPoint newPoint(permutationMap[i] % QUEUE_LENGTH_R, permutationMap[i] / QUEUE_LENGTH_R);

		// Identity-Points
		for (int32 lr = 0; lr <= i; lr++) {
			initIdentity(i, lr);
		}

		// Add this point to those point-structs, for which this point is better than the existing one 
		for (int32 lr = i+1; lr < QUEUE_LENGTH; lr++) { 
			AiPoint interPoint(permutationMap[lr] % QUEUE_LENGTH_R, permutationMap[lr] / QUEUE_LENGTH_R);
			
			if (true) {
				for (int32 p = 0; p < maximum(1, INTERP_POINTS - i); p++) {
					ASize relative(0.0f, 0.0f);
					float dist;

					if (i < INTERP_POINTS) {
						// Search for other instances of the same point in other layers, if there aren't enough samples in a single tile...
						dist = 0.0f;
						fassert(INTERP_POINTS <= 9); // cause we are only looking on the 8 neighbouring tiles + 1 center tile
						for (int32 xdist = -1; xdist <= 1; xdist += 1) {
							for (int32 ydist = -1; ydist <= 1; ydist += 1) {
								ASize adist = ASize(float(xdist), float(ydist));
								// never double-use the same instance of a point!
								bool found = false;
								for (int32 r = 0; r < INTERP_POINTS; r++) {
									if (points[lr][r].relativePosition == adist && points[lr][r].layer == permutationMap[i]) {
										found = true;
										break;
									}
								}
								if (found) continue;

								float td = netDistanceExplicit(AiSize(QUEUE_LENGTH_X, QUEUE_LENGTH_Y), interPoint, newPoint, AiSize(xdist, ydist));
								if (dist == 0.0f || td < dist) {
									relative = adist;
									dist = td;
								}
							}
						}
					}
					else {
						dist = netDistance(AiSize(QUEUE_LENGTH_X, QUEUE_LENGTH_Y), interPoint, newPoint, relative);
					}

					// This one's better! Update data...
					if (points[lr][INTERP_POINTS - 1].dist > dist || points[lr][INTERP_POINTS - 1].dist == 0.0f) {
						// Result will depend exponentially on the distance and longer distances give smaller relevances
						float relevance = pow(1.0f + (3.2f / (float(QUEUE_LENGTH_X + QUEUE_LENGTH_Y) / 2.0f)) * (1.0f + 3.0f * sqrt(i / float(QUEUE_LENGTH))), -dist); // -(dist / 42); // Just a rule of thumb... TODO: Correct this line! It is actually wrong... Use QUEUE_LENGTH == 45 and watch the first ~50 iterations to see the problem...

						insertPoint(points[lr], pointStruct(permutationMap[i], relative, APoint(newPoint) / newPoint.magnitude(), dist, relevance));
					}

					// Calculate Coefficents...
					float sumRel = 0.0f;
					for (size_t p = 0; p < INTERP_POINTS; p++) sumRel += points[lr][p].relevance;
					fassert(points[lr][0].relevance != 0.0f); // This is a reserved value! (See fragment shader)
					fassert(sumRel > 0.0f);
					for (size_t p = 0; p < INTERP_POINTS; p++) { // Points		
						coeffBuffer->set(i, permutationMap[lr] * INTERP_POINTS + p,
							points[lr][p].relativePosition.x / float(buffer->getSize().w),
							points[lr][p].relativePosition.y / float(buffer->getSize().h),
							normLayer(float(points[lr][p].layer)),
							points[lr][p].relevance / sumRel);
					}
				}
			}
		}

	}

	coeffBuffer->upload();
}

void pBuffer::draw(int x, int y, APoint pos, ASize zoom) {
	// TODO: There's maybe a bug! The image sometimes jumps when beginning a new rendering cyclus...
	// TODO: The zoom is juddery if the internal cashe size exceeds Full HD. (For example fullscreen + antialias). Two things to do: First: Reduce composer-target-size density. This makes composing faster and displaying more fluent! Second: Find out, how to make it fluent without first! Maybe a glFinish at the right place...?
	// TODO: Diagramme mit puffer, pufferfüllung, pufferdichte, Framezeit, statischer Framezeit, dynamischer Framezeit, Ereigniszeit 
	// TODO: Calibrate buffers, when algorithm is changing! Choose better rendering times!

	if (pBuffer::currentBuffer > 0) {
		pBuffer::readBuffer = pBuffer::writeBuffer;
	}


	// Solving z2 * (t1 - 0.5) + p2 = z1 * (t2 - 0.5) + p1 for t2 with t1 = (0,0) or t2 = (1,1)
	ASize t2_00((zoom * (-0.5f) + pos - pBuffer::rPosition[readBuffer]) / pBuffer::rZoom[readBuffer] + 0.5f);
	ASize t2_11((zoom * (0.5f) + pos - pBuffer::rPosition[readBuffer]) / pBuffer::rZoom[readBuffer] + 0.5f);
	ARect tC(t2_00.w, t2_00.h, t2_11.w, t2_11.h);

	pBuffer::buffers[readBuffer]->readFrom();
	ARect e(0.0f, 0.0f, 1.0f, 1.0f);
	pBuffer::quad.draw(ARect(0.0f, 0.0f, 1.0f, 1.0f), tC );
	
	
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
		int lr = xz + QUEUE_LENGTH_X*yz;
		const int iter = 13;
		int value = 0;
		white->use();

		if (coeffBuffer->get(iter, lr*INTERP_POINTS + 0, 3) != 0.0f) {
			for (size_t p = 0; p < INTERP_POINTS; p++) {
				value = p;

				if (coeffBuffer->get(iter, lr*INTERP_POINTS + value, 3) != 0.0f) {

					int tx = int(coeffBuffer->get(iter, lr*INTERP_POINTS + value, 2)*QUEUE_LENGTH) % QUEUE_LENGTH_X;
					int ty = int(coeffBuffer->get(iter, lr*INTERP_POINTS + value, 2)*QUEUE_LENGTH) / QUEUE_LENGTH_X;


					if (lmposx != mposx || lmposy != mposy) {
				//		cout << "pointing: " << xz << " " << yz << "    rel: " << coeffBuffer->get(iter, l*INTERP_POINTS + value, 0) << " " << coeffBuffer->get(iter, l*INTERP_POINTS + value, 1) << "  l: " << coeffBuffer->get(iter, l*INTERP_POINTS + value, 2) << "    target: " << tx << " " << ty << endl;
					}
					APoint source(mposx + (tx - xz) / float(size.w) + coeffBuffer->get(iter, lr*INTERP_POINTS + value, 0),
						mposy + (ty - yz) / float(size.h) + coeffBuffer->get(iter, lr*INTERP_POINTS + value, 1));

					line.draw(ARect(source.x, source.y, mposx, mposy));
				}
			}
		}
		lmposx = mposx;
		lmposy = mposy;
	}
#endif
}

uint64 pBuffer::render(uint64 inSamples, function<void(int)> renderF) {
	int64 samplesLeft = inSamples;
	bool changed = false;
#ifdef MEASURE_PERFORMANCE
	glQueryCreator gqt;
	{
		glQuery q = gqt.create();
#endif

#ifdef USE_TEXTURE_ARRAY
		buffer->writeTo(0);
#endif
		
		bool first = true;

		while (samplesLeft > 0) {
			if (pBuffer::currentBuffer == QUEUE_LENGTH) break;
			
			uint32 posxnew = minimum(uint32(buffer->getSize().w), posx + uint32(round(double(samplesLeft) / double(buffer->getSize().h))));

			if (posxnew == posx) {
				if (first) {
					first = false;
					posxnew++;
				} else break;
			}
			ASize ratio(ASize(buffer->getSize() * QUEUE_LENGTH_R) / ASize(size));
			ARect part(float(posx) / float(buffer->getSize().w), 0.0f, float(posxnew) / float(buffer->getSize().w), 1.0f);
			ARect partT = part * ASize(ratio.w, ratio.h);

			pBuffer::posx = posxnew;

			ASize delta(((permutationMap[pBuffer::currentBuffer] % QUEUE_LENGTH_R) + 0.5f) / float(QUEUE_LENGTH_R) - 0.5f,
				((permutationMap[pBuffer::currentBuffer] / QUEUE_LENGTH_R) + 0.5f) / float(QUEUE_LENGTH_R) - 0.5f);
			delta = delta / ASize(buffer->getSize());

#ifndef USE_TEXTURE_ARRAY
			buffer->writeTo(permutationMap[pBuffer::currentBuffer]);
#endif
			renderF(permutationMap[pBuffer::currentBuffer]);
			quad.draw(part, partT + delta);

			if (posxnew == buffer->getSize().w) {
				pBuffer::currentBuffer++;
				changed = true;
				pBuffer::posx = 0;
			}
			samplesLeft -= uint64(round((partT.right - partT.left) * buffer->getSize().w)) * buffer->getSize().h;
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
	
	return inSamples - samplesLeft;
}

void pBuffer::compose() {

	buffer->bind(GL_TEXTURE0);
	coeffBuffer->bind(GL_TEXTURE1);

	pBuffer::buffers[writeBuffer]->writeTo([this](void)->void {
		composer->use();
#ifdef USE_INTERPOLATION
	//	glUniform2f(uniform.iWinSize, 1.0f/size.w, 1.0f/size.h);
#ifdef USE_TEXEL_FETCH
		glUniform1i(uniform.iteration,  maximum(0, int32(pBuffer::currentBuffer) - 1));
		//Sleep(50);
#else
		glUniform1f(uniform.iteration, maximum(0.0f, float(pBuffer::currentBuffer) - 1) / float(QUEUE_LENGTH));
	//	cout << maximum(0.0f, float(pBuffer::currentBuffer) - 1) / float(QUEUE_LENGTH) << endl;
		glUniform1f(uniform.inverse_queue_l_minus_one, 1.0f / float(QUEUE_LENGTH - 1));
#endif
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

	// Only change the write buffer! The read buffer flips as soon as the quality of the other buffer is better or quickFill() is called!
	SWITCH10(pBuffer::writeBuffer);
	pBuffer::rPosition[pBuffer::writeBuffer] = pBuffer::rPosition[pBuffer::readBuffer];
	pBuffer::rZoom[pBuffer::writeBuffer] = pBuffer::rZoom[pBuffer::readBuffer];
}

float pBuffer::getProgress() {
	return float(pBuffer::currentBuffer) / float(QUEUE_LENGTH);
}

/*
void pBuffer::swap() {
	std::swap(pBuffer::writeBuffer, pBuffer::readBuffer);
	pBuffer::rPosition[pBuffer::writeBuffer] = pBuffer::rPosition[pBuffer::readBuffer];
	pBuffer::rZoom[pBuffer::writeBuffer] = pBuffer::rZoom[pBuffer::readBuffer];
}
*/

void pBuffer::quickFill() {
	// TODO
}

void pBuffer::setPosition(APoint pos, ASize zoom) {
	pBuffer::rPosition[pBuffer::writeBuffer] = pos;
	pBuffer::rZoom[pBuffer::writeBuffer] = zoom;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[   Progressive Renderer  ]*******************************/

pRenderer::pRenderer(AiSize size, float maxDensity1D) : size(size), maxDensity1D(maxDensity1D), samples(100), minQuality(1.0f,1.0f), fresh(false) {
	pRenderer::buffer.reset(new pBuffer(AiSize(int(ceil(size.w * maxDensity1D)), int(ceil(size.h*maxDensity1D)))));
}

pRenderer::~pRenderer() {

}

uint64 pRenderer::render(function<void(int)> renderF, bool discard) {
	if(discard)	buffer->discard();
	return buffer->render(pRenderer::samples, renderF);
}

void pRenderer::setSize(AiSize size, float maxDensity1D) {
	pRenderer::maxDensity1D = maxDensity1D;
	pRenderer::size = size;
	buffer->scale(AiSize(int(ceil(size.w * maxDensity1D)), int(ceil(size.h * maxDensity1D))));
}

void pRenderer::setSampleCount(uint64 samples) {
	pRenderer::samples = samples;
}

uint64 pRenderer::getSampleCount() const {
	return pRenderer::samples;
}

void pRenderer::draw(int x, int y) {
	fassert(realZ != ASize(0.0f, 0.0f));
	fassert(targetZ != ASize(0.0f, 0.0f));
	
	buffer->draw(x, y, targetP, targetZ);
	// TODO: Improve drawing! We need the algorithm to pre-downscale the buffer when density > 4.0
}

void pRenderer::view(APoint pos, ASize zoom, function<void(APoint, ASize)> viewF) {
	// TODO: Zwei Puffer! Den zweiten erst zeigen, sobald der neue besser als der alte ist! Dadurch auch das Ruckeln entfernen!
	// TODO: Wenn am Rand was gebraucht wird, nur diesen Randbereich neurendern!

	if (pRenderer::targetP == pos && pRenderer::targetZ == zoom && 
		(pRenderer::realZ != pRenderer::targetZ || pRenderer::realP != pRenderer::targetP)) {
		pRenderer::fresh = true;
		pRenderer::realZ = pRenderer::targetZ;
		pRenderer::realP = pRenderer::targetP;
		viewF(pRenderer::realP, pRenderer::realZ);
		pRenderer::buffer->discard();
		cout << "stabil" << endl;
	}
	else {
		pRenderer::targetP = pos; 
		pRenderer::targetZ = zoom;

		ASize qualy(pRenderer::targetZ.w / pRenderer::realZ.w * pRenderer::maxDensity1D, pRenderer::targetZ.h / pRenderer::realZ.h * pRenderer::maxDensity1D);
		if (qualy.w < minQuality.w || qualy.h < minQuality.h) {
			pRenderer::realZ = pRenderer::targetZ;
			pRenderer::realP = pRenderer::targetP;
			pRenderer::fresh = false;
		}
		else if (qualy.w > maxDensity1D || qualy.h > maxDensity1D) {
			// TODO: Provisorisch! Smart update !
			pRenderer::realZ = pRenderer::targetZ*2.0f;
			pRenderer::realP = pRenderer::targetP;
			viewF(pRenderer::realP, pRenderer::realZ);
			pRenderer::buffer->discard();
			pRenderer::fresh = false;
		}

		if (pRenderer::realZ == ASize(0.0f, 0.0f)) {
			pRenderer::realZ = pRenderer::targetZ;
			pRenderer::realP = pRenderer::targetP;
			viewF(pRenderer::realP, pRenderer::realZ);
			pRenderer::buffer->discard();
			pRenderer::fresh = true;
		}
		else if (pRenderer::realZ == pRenderer::targetZ && pRenderer::realP == pRenderer::targetP) {
			if (!pRenderer::fresh) {
				pRenderer::fresh = true;
				viewF(pRenderer::realP, pRenderer::realZ);
				pRenderer::buffer->discard();
			}
		}
	}

	buffer->setPosition(pRenderer::realP, pRenderer::realZ);
}

AiSize pRenderer::getSize() const {
	return pRenderer::size;
}

float pRenderer::getProgress() {
	return buffer->getProgress();
}
