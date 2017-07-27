/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: data.h
* Purpose: Shared constant data structures.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"
#include "GLHelpers.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          data           ]*******************************/

extern const string viewtexture;
extern const string mainVertexShader;
extern const string mainFragmentShader;
//extern const float quadVertices[];

void setStyle(nk_context *ctx, bool transparent);


#ifdef USE_TEXTURE_ARRAY
static const string mainVertexShaderRenamed(
	"layout (location = 0) in vec2 aPos;"
	"layout(location = 1) in vec2 aTexCoords;"
	"out vec2 TexCoordsVs;"
	"void main() {"
	"	TexCoordsVs = aTexCoords;"
	"	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
	"}");
static const string triangleLayerSelector(
	"uniform int lr;"
	"in vec2 TexCoordsVs[3];"
	"out vec2 TexCoords;"
	"layout(triangles) in;"
	"layout(triangle_strip, max_vertices = 3) out;"
	"void main(void) {"
	"	for (int i = 0; i<gl_in.length(); i++) {"
	"		TexCoords = TexCoordsVs[i];"
	"		gl_Layer = lr;"
	"		gl_Position = gl_in[i].gl_Position;"
	"		EmitVertex();"
	"	}"
	"	EndPrimitive();"
	"}");
#endif