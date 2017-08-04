/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU          ]*******************************
*
* File: fractal.cpp
* Purpose: Creating shaders, optimizing them and managing parameters for per-fragment
* rendered iterated functions.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "fractal.h"
#include "data.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         fractal         ]*******************************/

#ifdef USE_TEXTURE_ARRAY
const string mainVertexShaderRenamed(
	"layout (location = 0) in vec2 aPos;"
	"layout(location = 1) in vec2 aTexCoords;"
	"out vec2 TexCoordsVs;"
	"void main() {"
	"	TexCoordsVs = aTexCoords;"
	"	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
	"}");
const string triangleLayerSelector(
	"uniform int lr;"
	"in vec2 TexCoordsVs[3];"
	"out vec2 TexCoords;"
	"layout(triangles) in;"
	"layout(triangle_strip, max_vertices = 3) out;"
	"void main(void) {"
	"	for (int i = 0; i<gl_in.length(); i++) {"
	"		TexCoords = TexCoordsVs[i];"
	"		gl_Layer = lr;"
	"		gl_Position = gl_in[i].gl_Position;"
	"		EmitVertex();"
	"	}"
	"	EndPrimitive();"
	"}");
#endif

fractal::fractal(const string &script) : cx(0.00000001f), cy(3.0f), iter(100), biasPower(-40), AST(MAX_HASKELL_NODES) {


	fractal::setFunction(""); // script);

	//////////////////////////////////////////////



	for (size_t p = 0; p < MAX_POLY; p++) coe[p] = coet[p] = 0.0f;

#if 1
	coet[0] = -1.0f;
	coet[1] = -1.0f;
	coet[4] = 20.0f;
	coet[7] = 100.0f;
	coet[44] = 20.0f;
#else
	coet[0] = -1.0f;
	coet[7] = 100.0f;
	coet[977] = 100.0f;
	coet[1] = -1.0f;
#endif

	for (size_t p = 0; p < MAX_POLY; p++) coe[p] = coet[p];


	/////////////////////


	colorMap.reset(new texture(basePath + "res/hue.png"));


#ifdef USE_TEXTURE_ARRAY
	program.reset(new shader(mainVertexShaderRenamed, mainFragmentShader, triangleLayerSelector));
	uniform.layer = program->getUniform("lr");
#else
	program.reset(new shader(mainVertexShader, mainFragmentShader));
#endif
	uniform.screenTexture = program->getUniform("screenTexture");
	uniform.c = program->getUniform("c");
	uniform.iter = program->getUniform("iter");
	uniform.zoom = program->getUniform("zoom");
	uniform.pos = program->getUniform("pos");
	uniform.coe = program->getUniform("coe");
	uniform.coec = program->getUniform("coec");
	program->use();
	glUniform1i(uniform.screenTexture, 0);
}



void fractal::setFunction(const string &script) {
	
	// Testing

	any main = AST.getNew();
	main->cdr.set(42, ASExpr_symbol);
	main->car.set(AST.getNew(5));

	main->car.next()->car.next()->cdr.set(main);
	main->car.next()->car.next()->car.next()->cdr.set(AST.getNew(2));

	any tail = main->car.next()->car.next()->car.next();
	AST.free(tail->car.next());
	tail->car.set(AST.getNew(5));

	cout << AST.print(main) << endl;
	
	cout << "done!" << endl;

	AST.free(main);
}

void fractal::recompile() {
	// TODO
}


void fractal::render(int32 layer) {
	//glClear(GL_COLOR_BUFFER_BIT);
	//glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	program->use();

	glUniform2f(uniform.c, cx, cy);
	glUniform1i(uniform.iter, iter);
	glUniform1fv(uniform.coe, MAX_POLY, coe);
	glUniform1i(uniform.coec, coec);
#ifdef USE_TEXTURE_ARRAY
	glUniform1i(uniform.layer, layer);
#endif
	glErrors("uniform");

	colorMap->use(GL_TEXTURE0);

}

void fractal::view(APoint p, ASize z) {
	program->use();
	glUniform2f(uniform.zoom, z.w, z.h);
	glUniform2f(uniform.pos, p.x, p.y);
}


bool fractal::nk(nk_context *ctx, tooltip &ttip) {
	bool Change = false;


	static const int32 max_box_len = 1024;
	static char box_buffer[max_box_len];
	string init = "Test";
	static int32 box_len = init.length();
	if(box_len == init.length()) 
		strcpy_s(box_buffer, max_box_len, init.c_str());

	nk_layout_row_dynamic(ctx, 120, 1);
	nk_edit_string(ctx, NK_EDIT_BOX, box_buffer, &box_len, max_box_len, nk_filter_default);



	////////////////


	nk_layout_row_dynamic(ctx, 25, 1);
	int iter = fractal::iter;
	nk_property_int(ctx, "Iterations:", 1, &(iter), 1000, 10, iter * 0.001f + 0.03f);
	if (iter != fractal::iter) {
		Change = true;
		fractal::iter = iter;
	}

	nk_layout_row_dynamic(ctx, 25, 1);
	ttip.create(ctx, "Magnitudes below 2^bias are assumed to have converged. Too small values cause floating point errors! Use double precision for better quality!");
	nk_property_int(ctx, "Bias:", -50, &biasPower, 10, 1, 0.1f);
	if (cx != pow(2.0f, float(biasPower))) {
		Change = true;
		cx = pow(2.0f, float(biasPower));
	}

	nk_layout_row_dynamic(ctx, 25, 1);
	int dummy = 0;
	nk_property_int(ctx, "Anim. speed:", -50, &dummy, 10, 1, 0.1f);
	// TODO: Implement!

	return Change;
}

bool fractal::logic(uint32 timeElapsed, float animSpeed) {
	bool Change = false;

	///////////////////// Animate Polynomials
	for (size_t p = 0; p < MAX_POLY; p++) {
		if (coet[p] != 0.0f || coe[p] != 0.0f) {
			coec = p;
			if (abs(coe[p] - coet[p]) < 0.000001f) coe[p] = coet[p];
			else {
				float blendTim = tanh(0.01f * timeElapsed / (animSpeed / 1000.0f));
				coe[p] = coe[p] * (1.0f - blendTim) + coet[p] * blendTim;
				Change = true;
			}
		}
	}
	//	coe[1] = (sin(clock() / 10000.0f + 3.16*1) ) * 1.5f + 0.5f;

	return Change;
}
