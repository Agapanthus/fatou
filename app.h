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
#include "renderer.h"


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
	void render();
	void display();
	void setFullscreen(bool fullscreen);

private:

	///////////////////// Ressources

	pointer<shader> program;
	pointer<shader> texprogram;
	pointer<texture> colorMap;
	pointer<vao> quad;
	//pointer<syncBuffer> buf1;
	pointer<tRenderer> renderer;
	AiSize tiles;
	pointer<fOptimizer> optim;

	///////////////////// Parameters

	int targetFRate;
	struct {
		GLint iter, screenTexture, c, zoom, pos, coec, coe;
	} uniform;

	float cx, cy, zoomx, zoomy, zoom, posx, posy;
	int iter;
	size_t coec;
	float coe[MAX_POLY], coet[MAX_POLY];
	int biasPower;
	int maxDensity;
	float maxDensityI;

	///////////////////// General 

	float animSpeed;
	unsigned int lastTime;
	int width, height;
	GLint maxTextureSize;
	bool isFullscreen;


	///////////////////// Interaction

	enum {
		navigation_lrclick_combi,
		navigation_none,
		navigation_drag_and_wheel,
	} navigationMode;

	///////////////////// GUI

	GLFWwindow* window;
	struct nk_context *ctx;

	int show_about, show_polynomial, show_roots, show_tooltips;
	int trace_length;

	///////////////////// Temp / Debug

	float supers;


};

extern const string mainVertexShader;
extern const string mainFragmentShader;
extern const float quadVertices[];