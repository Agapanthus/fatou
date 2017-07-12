/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           *******************************
*
* File: main.cpp
* Purpose: 
*
* Copyright 2017 EricSkaliks
*
*/

#include "stdafx.h"
#include "app.h"


pointer<app> APP;

int _main(int argc, char **argv) {
	try {
		glutInit(&argc, argv);
		glutInitDisplayMode(/*GLUT_DEPTH |*/ /*GLUT_SINGLE*/  GLUT_DOUBLE | GLUT_RGBA);
		glutInitWindowPosition(100, 100);
		glutInitWindowSize(800, 600);
		GLuint window = glutCreateWindow("Fatou");

		GLenum glew_status = glewInit();
		if (glew_status != GLEW_OK) {
			fatalNote(string("Error: ") + string((const char*)glewGetErrorString(glew_status)));
		}
		if (!GLEW_VERSION_2_0) {
			fatalNote("Error: your graphic card does not support OpenGL 2.0\n");
		}

		// init App
		APP.reset(new app());

		// Setup callbacks
		glutKeyboardFunc([](unsigned char key, int x, int y) -> void { APP->keypressed(key); });
		glutReshapeFunc([](int w, int h) -> void { APP->reshape(w, h); });
		glutIdleFunc([]() -> void { APP->render(); });
		glutDisplayFunc([]() -> void { APP->display(); });
		glutMouseFunc([](int button, int state, int x, int y) -> void {
			mousebutton mb = mousebutton_none;
			switch (button) {
			case GLUT_LEFT_BUTTON: mb = mousebutton_left; break;
			case GLUT_MIDDLE_BUTTON: mb = mousebutton_middle; break;
			case GLUT_RIGHT_BUTTON: mb = mousebutton_right; break;
			}
			if (mb != mousebutton_none)
				APP->mousestatechanged(mb, state == GLUT_DOWN);
		});
		glutMotionFunc([](int x, int y) -> void { APP->mousemove(x, y); });
		glutPassiveMotionFunc([](int x, int y) -> void { APP->mousemove(x, y); });

		glutMainLoop();

		glutDestroyWindow(window);
	}
	catch (const exception &e) {
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
}