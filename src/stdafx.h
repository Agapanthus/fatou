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

#include <queue>
#include <mutex>
#include <condition_variable>

// OpenGL
//#define GLEW_STATIC
//#include <GL/glew.h>
#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
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
#include "../third_party/nuklear/nuklear.h"
#include "../third_party/nuklear/nuklear_glfw_gl3.h"

#ifdef OS_WIN
#include <shellapi.h>
#endif

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Entry Point         ]*****************************/

int _main(int argc, char **argv);
void _exit();


