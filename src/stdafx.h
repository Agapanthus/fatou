/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU         ]********************************
*
* File: stdafx.h
* Purpose: Precompiled Header.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        Constants         ]******************************/

#define MAX_POLY 1000
#define INITIAL_W 800
#define INITIAL_H 600

#define USE_CONSOLE
#define USE_ASSERTATIONS

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         Include           ]*****************************/

#include "abstraction.h"
#include <time.h>
#include <functional>
using std::function;
#include <thread>
using std::thread;

// To get shuffle
#include <algorithm>

// To get memset
#include <cstring>

#include <queue>
#include <mutex>
#include <condition_variable>
#include <xutility>
using std::move;

// getch() (for debugging)
#include <stdio.h>
#include <conio.h>

// OpenGL
#include <glbinding/gl32core/gl.h>
#include <glbinding/Binding.h>
//#include <glbinding/gl/extension.h> 
using namespace gl;

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_GLFW_GL3_MOUSE_GRABBING 1
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "../third_party/util/nuklear.h"
#include "../third_party/util/nuklear_glfw_gl3.h"


#ifdef OS_WIN
#include <shellapi.h>
#endif
#ifdef OS_LIN
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif

// Path with trailing (back-)slash relative to which one can finde resources etc.! Is not defined before int main(2) is called!
extern string basePath;

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Entry Point         ]*****************************/

int _main(int argc, char **argv);
void _exit();


