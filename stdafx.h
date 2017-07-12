/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           *******************************
*
* File: stdafx.h
* Purpose: Precompiled Header.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once

#include "abstraction.h"

#define MAX_POLY 1000


// OpenGL
#define GLEW_STATIC
#include "../../third_party/glew-2.0.0/include/GL/glew.h"
#define FREEGLUT_LIB_PRAGMAS 0
#define FREEGLUT_STATIC
#include "../../third_party/freeglut-3.0.0/include/GL/freeglut.h"

// Libraries
#ifdef OS_WIN
#include <time.h>
	#pragma comment(lib , "../../third_party/glew-2.0.0/lib/Release/Win32/glew32s.lib")
	#ifdef _DEBUG
		#pragma comment(lib, "../../third_party/freeglut-3.0.0/lib/Debug/freeglut_staticd.lib")
	#else
		#pragma comment(lib, "../../third_party/freeglut-3.0.0/lib/Release/freeglut_static.lib")
	#endif
	#pragma comment (lib, "glu32.lib")   
	#pragma comment (lib, "opengl32.lib") 
#endif

// Entrypoint
int _main(int argc, char **argv);
void _exit();


