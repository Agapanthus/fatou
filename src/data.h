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
extern const string mainDeepFragmentShader;
//extern const float quadVertices[];

void setStyle(nk_context *ctx, bool transparent);




extern const string defaultFragmentEnding;
extern const string defaultFragmentBeginning;
extern const string defaultDeepFragmentEnding;
extern const string defaultDeepFragmentBeginning;