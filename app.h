/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           *******************************
*
* File: app.h
* Purpose: Main Application.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once

#include "stdafx.h"
#include "GLHelpers.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           app           ]*******************************/

enum mousebutton {
	mousebutton_left,
	mousebutton_middle,
	mousebutton_right,
	mousebutton_none
};

class app {
public:
	app();
	~app();

	void reshape(int w, int h);
	void keypressed(GLuint key);
	void mousemove(int x, int y);
	void mousestatechanged(mousebutton button, bool pressed);

	void render();
	void display();

private:
	pointer<shader> program;
	pointer<shader> guiProgram;
	pointer<texture> colorMap;
	pointer<vao> quad;

	struct {
		GLint iter, screenTexture, c, zoom, pos, coec, coe;
	} uniform;

	float cx, cy, zoomx, zoomy, posx, posy;
	int iter;
	size_t coec;
	float coe[MAX_POLY], coet[MAX_POLY];

	float animSpeed;

	unsigned int lastTime;

	GLuint text;
};

extern const string mainVertexShader;
extern const string mainFragmentShader;
extern const float quadVertices[];