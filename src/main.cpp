/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU         ]********************************
*
* File: main.cpp
* Purpose: 
*
* Copyright 2017 Eric Skaliks
*
*/


#include "stdafx.h"
#include "app.h"
#include "fractal.h"

#include "fHaskell.h"

pointer<app> APP;
nk_context* ctx;
static const bool doubleBuffered = false; // Double Buffered GUI at low framerates is a bad idea... // TODO: Can I make it dynamic? Swap buffer twice if frame rate is low?

int _main(int argc, char **argv) {

	fHaskell h;
	Sleep(100000);
	return 0;

	/*{
		fractal f("");

	}
	return 0;
	*/

	try {
		glfwSetErrorCallback([](int e, const char *d) -> void { fatalNote(std::to_string(e) + " " + string(d)); });
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_DOUBLEBUFFER, doubleBuffered ? true : false);

	
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

		GLFWwindow* window = glfwCreateWindow(INITIAL_W, INITIAL_H, "Fatou", NULL, NULL);
		if (window == NULL) {
			fatalNote("Failed creating Window.");
			exit(-1);
		}
		glfwMakeContextCurrent(window);
		glfwSetKeyCallback(window, [](GLFWwindow*, int key, int, int action, int)->void { 
			if(action == GLFW_PRESS || action == GLFW_REPEAT)
				APP->keypressed(key);  
		});

		glbinding::Binding::initialize(false); // only resolve functions that are actually used (lazy)
		// TODO: Error Checking! Version Checking!

		
		///////////////////////////////////

		ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
		{
			struct nk_font_atlas *atlas;
			nk_glfw3_font_stash_begin(&atlas);
			nk_glfw3_font_stash_end();

		}

		//////////////////////////////////////////

		APP.reset(new app(window, ctx));

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) -> void { APP->reshape(w, h); });
		glErrors("glfw::callbacks");

		
		///////////////////////////////////

		while (!glfwWindowShouldClose(window)) {

			glfwPollEvents();

			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(window, true);
			
			APP->logic();
			APP->display();

			if (doubleBuffered) {
				glfwSwapBuffers(window);
				glErrors("main::swap");
			}
			else {
				glFinish();
				glErrors("main::finish");
			}
		}
	}
	catch (const AException &e) {
		fatalNote(string("AException: ") + e.details + string("\ncode: ") + toString(e.code.code) + " (" + e.code.name +  ")");
	}
	catch (const std::exception &e) {
		fatalNote(string("Exception: ") + e.what());
	}
	catch (...) {
		fatalNote("Sorry. Program crashed for unknown reasons.");
	}
	return 0;
}

void _exit() {
	// force deallocation
	APP.reset(0, false);
	nk_glfw3_shutdown();
	glfwTerminate();
}
