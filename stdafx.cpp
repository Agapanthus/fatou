/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU         ]********************************
*
* File: stdafx.cpp
* Purpose: Precompiled Header.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"

// Libraries
#ifdef OS_WIN
	#pragma comment(lib , "../../third_party/glew-2.0.0/lib/Release/Win32/glew32s.lib")

	#pragma comment (lib, "glu32.lib")   
	#pragma comment (lib, "opengl32.lib") 

	
	#ifdef _DEBUG
		#pragma comment(lib, "../../third_party/glfw-3.2.1/src/Debug/glfw3.lib")
	#else
		#pragma comment(lib, "../../third_party/glfw-3.2.1/src/Release/glfw3.lib")
	#endif

#endif


#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Entry Point         ]*****************************/

#ifdef OS_WIN

int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow) {

#ifdef USE_CONSOLE
	AConsole cons(2000);
#endif
	atexit(_exit);
	return _main(0, 0);
}

#elif defined(OS_LIN)
int main(int argc, char **argv) {
	return _main(argc, argv);
	// TODO: _exit();
}


#else
#error Unknown OS
#endif
