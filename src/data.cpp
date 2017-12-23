/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: data.cpp
* Purpose: Shared constant data structures.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "data.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          data           ]*******************************/

const string viewtexture(
	"uniform sampler2D screenTexture;"
	"in vec2 TexCoords;"
	"void main() {"
	"	gl_FragColor = texture(screenTexture, TexCoords);"
	"}");


const string defaultFragmentEnding(
	"	vec2 nz = normalize(z) * 0.4f + 0.5f;"
	"	gl_FragColor = i >= (iter - 1) ? vec4(0.0) : (1 - vali / iter)*texture(screenTexture, nz);"
	"	gl_FragColor.w = 1.0;"
	"}"
);

const string defaultDeepFragmentEnding(
	"	vec2 nz = normalize(vec2(float(z.x), float(z.y))) * 0.4f + 0.5f;"
	"	gl_FragColor = i >= (iter - 1) ? vec4(0.0) : (1 - vali / iter)*texture(screenTexture, nz);"
	"	gl_FragColor.w = 1.0;"
	"}"
);

const string defaultFragmentBeginning(
	"uniform sampler2D screenTexture;"
	"in vec2 TexCoords;"

	"uniform int iter;"
	"uniform vec2 c, zoom, pos; "

	"uniform float coe[" + std::to_string(MAX_POLY) + "];"
	"uniform int coec;"

	"\n"
	"#define product(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)\n"
	"#define divide(a, b) vec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))\n"
	"#define magnitudeA(a) sqrt(2*a.x*a.x)\n"
	"#define msizeA(a) 2*a.x*a.x\n" // Using them will result in quite interesting results!
	"#define magnitude(a) sqrt(a.x*a.x+a.y*a.y)\n"
	"#define msize(a) (a.x*a.x+a.y*a.y)\n"

	"void main() {"
	"	vec2 z;"
	"	vec2 tc = vec2(TexCoords[0], TexCoords[1]);"
	"	z.x = zoom.x * (tc.x - 0.5f) + pos.x;"
	"	z.y = zoom.y * (tc.y - 0.5f) + pos.y;"

	"	int i;"
);

const string defaultDeepFragmentBeginning(
	"uniform sampler2D screenTexture;"
	"in vec2 TexCoords;"

	"uniform int iter;"
	"uniform dvec2 c, zoom, pos; "

	"uniform double coe[" + std::to_string(MAX_POLY) + "];"
	"uniform int coec;"

	"\n"
	"#define product(a, b) dvec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)\n"
	"#define divide(a, b) dvec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))\n"
	"#define magnitudeA(a) sqrt(2*a.x*a.x)\n"
	"#define msizeA(a) 2*a.x*a.x\n" // Using them will result in quite interesting results!
	"#define magnitude(a) sqrt(a.x*a.x+a.y*a.y)\n"
	"#define msize(a) (a.x*a.x+a.y*a.y)\n"

	"void main() {"
	"	dvec2 z;"
	"	dvec2 tc = dvec2(TexCoords[0], TexCoords[1]);"
	"	z.x = zoom.x * (tc.x - 0.5) + pos.x;"
	"	z.y = zoom.y * (tc.y - 0.5) + pos.y;"

	"	int i;"
);

const string mainDeepFragmentShader(
	defaultDeepFragmentBeginning +
	"	float nu = 0.0f;"
	"	for (i = 0; i<iter; i++) {"
	"		int ii;"
	"		dvec2 f = dvec2(coe[0], 0.0);"
	"		dvec2 ff = dvec2(0.0);"

	"		dvec2 ex = dvec2(1.0, 0.0);"
	"		for (ii = 1; ii <= coec; ii++) {"
	"			if (coe[ii] != 0) ff += (coe[ii] * ii) * ex;"
	"			ex = product(ex, z);"
	"			if (coe[ii] != 0) f += coe[ii] * ex;"
	"		}"
	"		dvec2 dif = divide(f, ff);"
	//"		if (msize(dif) < c.x) {" // TODO: This should be msize(f) not msize(dif), shouldn't it?
	//"			nu = log( log(float(msize(dif))) / log(float(c.x)) ) * 3.78418963391826f; "//log(float(coec)); " // TODO: Nothing serious. Needs to be improved!
	"		if (msize(dif) < c.x) {"
	"			nu = log( log(float(msize(dif))) / log(float(c.x)) ) * log(float(coec)); "
	"			break;"
	"		}"
	"		z -= dif;"
	"	}"
	"	float vali = (float(i+1) -  nu) * float(c.y);"
	"	if (vali > float(iter)) vali = float(iter);"
	+ defaultDeepFragmentEnding);

const string mainFragmentShader(
	defaultFragmentBeginning +
#define Continuous_coloring
#ifdef Continuous_coloring
	"	float nu = 0.0f;"
#endif	
	"	for (i = 0; i<iter; i++) {"
	"		int ii;"
	"		vec2 f = vec2(coe[0], 0.0f);"
	"		vec2 ff = vec2(0.0f);"

	"		vec2 ex = vec2(1.0f, 0.0f);"
	"		for (ii = 1; ii <= coec; ii++) {"
	"			if (coe[ii] != 0) ff += (coe[ii] * ii) * ex;"
	"			ex = product(ex, z);"
	"			if (coe[ii] != 0) f += coe[ii] * ex;"
	"		}"
	"		vec2 dif = divide(f, ff);"
	"		if (msize(dif) < c.x) {"
#ifdef Continuous_coloring
	// Multiply with large Number and use msizeA to get really intresting lines!
	"			nu = log( log(msize(dif)) / log(c.x) ) * 3.78418963391826f; "//log(float(coec)); " // TODO: Nothing serious. Needs to be improved!
#endif
	"			break;"
	"		}"
	"		z -= dif;"
	"	}"
#ifdef Continuous_coloring
	"	float vali = (float(i+1) -  nu) * c.y;"
	"	if (vali > float(iter)) vali = float(iter);"
#else
	"	float vali = float(i)*c.y;"
	"	if (vali > float(iter)) vali = float(iter);"
#endif
	+ defaultFragmentEnding);



//////////////////////////////

const string mainVertexShader(
	"layout (location = 0) in vec2 aPos;"
	"layout(location = 1) in vec2 aTexCoords;"
	"out vec2 TexCoords;"
	"void main() {"
	"	TexCoords = aTexCoords;"
	"	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
	"}");


static struct nk_color table[NK_COLOR_COUNT];
void setStyle(nk_context *ctx, bool transparent) {
	AColor primary(65, 85, 85);
	AColor hover(28, 163, 80);
	AColor active(90, 210, 130);
	unsigned char eleTrans = transparent ? 180 : 255;
	table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
	table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, transparent ? 215 : 255);
	table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 255);
	table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
	table[NK_COLOR_BUTTON] = nk_rgba(primary.c[AColor::AColor_r], primary.c[AColor::AColor_g], primary.c[AColor::AColor_b], eleTrans);
	table[NK_COLOR_BUTTON_HOVER] = nk_rgba(hover.c[AColor::AColor_r], hover.c[AColor::AColor_g], hover.c[AColor::AColor_b], 255);
	table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(active.c[AColor::AColor_r], active.c[AColor::AColor_g], active.c[AColor::AColor_b], 255);
	table[NK_COLOR_TOGGLE] = nk_rgba(51, 51, 56, eleTrans);
	table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(hover.c[AColor::AColor_r], hover.c[AColor::AColor_g], hover.c[AColor::AColor_b], 255);
	table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(active.c[AColor::AColor_r], active.c[AColor::AColor_g], active.c[AColor::AColor_b], 255);

	table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, eleTrans);
	table[NK_COLOR_TRANSP] = nk_rgba(0, 0, 0, 0);

	table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 0);
	table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
	table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
	table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 255);
	table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
	table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
	table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 255);
	table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
	table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, eleTrans);
	table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
	table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
	table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);

	table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, eleTrans );

	table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(primary.c[AColor::AColor_r], primary.c[AColor::AColor_g], primary.c[AColor::AColor_b], eleTrans);
	table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(hover.c[AColor::AColor_r], hover.c[AColor::AColor_g], hover.c[AColor::AColor_b], 255);
	table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(active.c[AColor::AColor_r], active.c[AColor::AColor_g], active.c[AColor::AColor_b], 255);
	table[NK_COLOR_TAB_HEADER] = nk_rgba(57, 167, 71, 255);
	table[NK_COLOR_TREE_BASE] = table[NK_COLOR_TRANSP];
	table[NK_COLOR_TREE_HOVER] = nk_rgba(hover.c[AColor::AColor_r], hover.c[AColor::AColor_g], hover.c[AColor::AColor_b], 255);
	table[NK_COLOR_TREE_TEXT] = nk_rgba(active.c[AColor::AColor_r], active.c[AColor::AColor_g], active.c[AColor::AColor_b], 255);
	table[NK_COLOR_TREE_BORDER] = nk_rgba(46, 46, 46, 200);

	nk_style_from_table(ctx, table);
}
