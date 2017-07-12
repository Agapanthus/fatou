/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           *******************************
*
* File: data.inl
* Purpose: Shaders and Data.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"

const string mainFragmentShader(
	"uniform sampler2D screenTexture;"
	"in vec2 TexCoords;"

	"uniform int iter;"
	"uniform vec2 c, zoom, pos; "

	"uniform float coe[" + std::to_string(MAX_POLY) + "];"
	"uniform int coec;"

	"\n"
	"#define product(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)\n"
	"#define divide(a, b) vec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))\n"
	"#define magnitude(a) a.x*a.x+a.x*a.x\n"

	"void main() {"
	"	vec2 z;"
	"	vec2 tc = vec2(TexCoords[0], TexCoords[1]);"
	"	z.x = zoom.x * (tc.x - 0.5) + pos.x;"
	"	z.y = zoom.y * (tc.y - 0.5) + pos.y;"

	"	int i;"
	"	for (i = 0; i<iter; i++) {"
	"		int ii;"
	"		vec2 f = vec2(coe[0], 0.0);"
	"		vec2 ff = vec2(0.0);"

	"		vec2 ex = vec2(1.0, 0.0);"
	"		for (ii = 1; ii <= coec; ii++) {"
	"			if (coe[ii] != 0) ff += (coe[ii] * ii) * ex;"
	"			ex = product(ex, z);"
	"			if (coe[ii] != 0) f += coe[ii] * ex;"
	"		}"
	"		vec2 dif = divide(f, ff);"
	"		if (magnitude(dif) < c.x) break;"
	"		z -= dif;"
	"	}"
	"	float vali = float(i)*c.y;"
	"	if (vali > float(iter)) vali = float(iter);"
	"	vec2 nz = normalize(z) * 0.4 + 0.5;"
	"	gl_FragColor = i >= (iter - 1) ? vec4(0.0) : (1 - vali / iter)*texture(screenTexture, nz);"
	"	gl_FragColor.w = 1.0;"
	"}");

const string mainVertexShader(
	"layout (location = 0) in vec2 aPos;"
	"layout(location = 1) in vec2 aTexCoords;"
	"out vec2 TexCoords;"
	"void main() {"
	"	TexCoords = aTexCoords;"
	"	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
	"}");

// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
const float quadVertices[] = {
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	1.0f, -1.0f,  1.0f, 0.0f,
	1.0f,  1.0f,  1.0f, 1.0f
};
