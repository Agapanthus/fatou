/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU         ]********************************
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
#include "aRenderer.h"
#include "data.h"
#include "fractal.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           app           ]*******************************/

extern unsigned int MaxFRate;

enum mousebutton {
	mousebutton_left,
	mousebutton_middle,
	mousebutton_right,
	mousebutton_none
};

class app {
public:
	app(GLFWwindow *window, nk_context *ctx);
	~app();

	void reshape(int w, int h);
	void keypressed(int key);

	void logic();
	void display();
	void setFullscreen(bool fullscreen);

private:

	///////////////////// Ressources

	AiSize tiles;
	pointer<aRenderer> renderer;
	pointer<fractal> fra;

	///////////////////// Parameters

	int targetFRate;

	double zoomx, zoomy, zoom, posx, posy;

	bool isNewton;
	int doublePrec;

	int preset;

//	size_t coec;
//	float coe[MAX_POLY], coet[MAX_POLY];
//	int biasPower;
	int maxDensity;
	float maxDensityI;

	///////////////////// General 

	float animSpeed;
	unsigned int lastTime;
	int width, height;
	GLint maxTextureSize;
	bool isFullscreen;
	function<void(int)> renderF;
	bool Change; // Parameters changed
	bool pChange; // Position / Zoom changed

	///////////////////// Interaction

	enum {
		navigation_lrclick_combi,
		navigation_none,
		navigation_drag_and_wheel,
	} navigationMode;

	///////////////////// GUI

	GLFWwindow* window;
	struct nk_context *ctx;

	int show_about, show_polynomial, show_roots;
	int trace_length;

	///////////////////// Temp / Debug

	float supers;


};
