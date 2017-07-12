/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           *******************************
*
* File: stdafx.cpp
* Purpose: Precompiled Header.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Entry Point         ******************************/

#ifdef OS_WIN

int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow) {
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