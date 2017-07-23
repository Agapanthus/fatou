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

/*
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
*/


#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "../third_party/nuklear/nuklear.h"
#include "../third_party/nuklear/nuklear_glfw_gl3.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Entry Point         ]*****************************/

string basePath;
inline string stripFilename(const string &path) {
	if (path.length() == 0) return path;
	int32 i;
	for (i = path.length() - 1; path[i] != '\\' && path[i] != '/' && i >= 0; i--) {}
	return path.substr(0, minimum(path.length(), i+1U));
}

#ifdef OS_WIN

int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow) {

	{
		char buf[1024] = { 0 };
		DWORD ret = GetModuleFileNameA(NULL, buf, sizeof(buf));
		if (ret == 0 || ret == sizeof(buf)) {
			fatalNote("Getting module file name failed!");
		}
		basePath = stripFilename(string(buf));
	}

#ifdef USE_CONSOLE
	AConsole cons(2000);
#endif
	atexit(_exit);
	return _main(0, 0);
}

#elif defined(OS_LIN)
int main(int argc, char **argv) {
	basePath = stripFilename(string(argv[0]));
	return _main(argc, argv);
	// TODO: _exit();
}


#else
#error Unknown OS
#endif
