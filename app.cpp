/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU          ]*******************************
*
* File: app.h
* Purpose: Main Application.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "app.h"
#include "data.inl"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           app           ]*******************************/

unsigned int MaxFRate = 67;

app::app(GLFWwindow *window, nk_context* ctx) :
	cx(0.00000001f), cy(3.0f),
	iter(100),
	zoomx(3.0f), zoomy(2.0f), zoom(2.0f),
	posx(0.0f), posy(0.0f),
	animSpeed(1000.0f),
	lastTime(UINT32_MAX),
	window(window), ctx(ctx),
	navigationMode(navigation_lrclick_combi),
	supers(1.0f), tiles(3,3),
	targetFRate(30)
{

	glErrors("app::before");
	for (size_t p = 0; p < MAX_POLY; p++) coe[p] = coet[p] = 0.0f;
	coet[0] = -1.0f;
	coet[1] = -1.0f;
	coet[4] = 20.0f;
	coet[7] = 100.0f;
	coet[44] = 20.0f;
		
	app::program.reset(new shader(mainVertexShader, mainFragmentShader));
	app::uniform.screenTexture = app::program->getUniform("screenTexture");
	app::uniform.c = app::program->getUniform("c");
	app::uniform.iter = app::program->getUniform("iter");
	app::uniform.zoom = app::program->getUniform("zoom");
	app::uniform.pos = app::program->getUniform("pos");
	app::uniform.coe = app::program->getUniform("coe");
	app::uniform.coec = app::program->getUniform("coec");
	app::program->use();
	glUniform1i(app::uniform.screenTexture, 0);

	app::texprogram.reset(new shader(mainVertexShader, viewtexture));
	app::texprogram->use();
	glUniform1i(app::texprogram->getUniform("screenTexture"), 0);
	
	
	///////////////////////////////////////////////////

	//glEnable(GL_TEXTURE_2D); // Not necessary in OpenGL 3
	glErrors("app::glEnable");

	///////////////////////////

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	glErrors("main::getMaxTextureSize");

	////////////////////////////

	glfwGetWindowSize(app::window, &(app::width), &(app::height));

	app::optim.reset(new fOptimizer(float(app::targetFRate), 2.0f));
	app::colorMap.reset(new texture("../hue.png"));
	app::quad.reset(new vao(quadVertices, sizeof(quadVertices), 6));
	//app::buf1.reset(new syncBuffer(int(app::width*supers), int(app::height*supers), false, GL_LINEAR));
	app::renderer.reset(new tRenderer(AiSize(app::width, app::height), tiles));
}

app::~app() {

};

void app::keypressed(int key) {
	/*switch (key) {
	case GLFW_KEY_LEFT:
		supers *= 0.5f;
		cout << supers << endl;
		break;
	case GLFW_KEY_RIGHT:
		supers *= 2.0f;
		cout << supers << endl;
		break;
	}
	app::buf1->scale(int(app::width*supers), int(app::height*supers), false);
	*/
}


void app::reshape(int w, int h) {

	app::optim->hint( float(w*h) / (app::width*app::height));

	app::width = w;
	app::height = h;
	glViewport(0, 0, app::width, app::height);

	app::renderer->setSize(AiSize(app::width, app::height), tiles);


//	app::buf1->scale(int(app::width*supers), int(app::height*supers), false);

	glErrors("app::reshape");
}

void app::logic() {
	bool Change = false;

	unsigned int now = gtimeGet();
	unsigned int timeElapsed = now - app::lastTime;
	if (app::lastTime == UINT32_MAX) timeElapsed = 1;
	app::lastTime = now;

	///////////////////////////// GUI

	static struct nk_color background = nk_rgb(28, 48, 62);

	nk_glfw3_new_frame();


	if (nk_begin(ctx, "Settings", nk_rect(50, 50, 230, 250),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
		NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{
		/*
		enum { EASY, HARD };
		static int op = EASY;
		nk_layout_row_static(ctx, 30, 80, 1);
		if (nk_button_label(ctx, "button"))
			fprintf(stdout, "button pressed\n");

		nk_layout_row_dynamic(ctx, 30, 2);
		if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
		if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
		*/

		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_int(ctx, "Target Framerate:", 5, &targetFRate, int(MaxFRate * 0.9f), 10, 1);
		app::optim->setTargetFramerate(float(targetFRate));

		nk_layout_row_dynamic(ctx, 20, 2);
		nk_label(ctx, "Density:", NK_TEXT_RIGHT);
		nk_label(ctx, toString(app::optim->getPixelDensity(), 2).c_str(), NK_TEXT_LEFT);

		nk_layout_row_dynamic(ctx, 20, 2);
		nk_label(ctx, "Framerate:", NK_TEXT_RIGHT);
		nk_label(ctx, toString(app::optim->getFramerate(), 2).c_str(), NK_TEXT_LEFT);

		/*
		nk_layout_row_dynamic(ctx, 20, 1);
		nk_label(ctx, "background:", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_combo_begin_color(ctx, background, nk_vec2(nk_widget_width(ctx), 400))) {
			nk_layout_row_dynamic(ctx, 120, 1);
			background = nk_color_picker(ctx, background, NK_RGBA);
			nk_layout_row_dynamic(ctx, 25, 1);
			background.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, background.r, 255, 1, 1);
			background.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, background.g, 255, 1, 1);
			background.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, background.b, 255, 1, 1);
			background.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, background.a, 255, 1, 1);
			nk_combo_end(ctx);
		}*/
	}
	nk_end(ctx);

	if (0 == nk_item_is_any_active(ctx)) {

		///////////////////// Navigate

		switch (navigationMode) {
		case navigation_drag_and_wheel:
			// TODO
			break;
		case navigation_none:
			break;
		case navigation_lrclick_combi:
		default:
		
			bool left = glfwGetMouseButton(app::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
			bool right = glfwGetMouseButton(app::window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
			
			static const float zoomSpeed = 0.0005f; // 2000.0f;
			static const float moveSpeed = 0.03f; // 30.0f;

			if (left) {
				app::zoom /= timeElapsed * zoomSpeed + 1.0f;
				//app::zoomy /= timeElapsed * zoomSpeed + 1.0f;
			}
			else if (right) {
				app::zoom *= timeElapsed * zoomSpeed + 1.0f;
				//app::zoomy *= timeElapsed * zoomSpeed + 1.0f;
			}
			float aspect = float(app::width) / float(app::height);
			app::zoomx = app::zoom * (app::width/1000.0f);
			app::zoomy = app::zoom * (app::height/1000.0f);

			if (left || right) {
				double xpos, ypos;
				glfwGetCursorPos(app::window, &xpos, &ypos);
				app::posx += (float(xpos) / float(app::width) - 0.5f) * app::zoomx * moveSpeed;
				app::posy -= (float(ypos) / float(app::height) - 0.5f) * app::zoomy * moveSpeed;

			}

			break;
		}

	}
	

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


}
void app::render() {


}
void app::display() {
		

	///////////////////////////////////

	glErrors("app::display");


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, app::width, app::height);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);



	app::optim->optimize((sRenderer*)app::renderer.data());

	while (app::renderer->renderTile([this](ARect tile) -> void {
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

		app::program->use();

		glUniform2f(app::uniform.c, cx, cy);
		glUniform2f(app::uniform.zoom, zoomx, zoomy);
		glUniform2f(app::uniform.pos, posx, posy);
		glUniform1i(app::uniform.iter, iter);
		glUniform1fv(app::uniform.coe, MAX_POLY, coe);
		glUniform1i(app::uniform.coec, coec);
		glErrors("app::uniform");

		app::colorMap->use(GL_TEXTURE0);
	})) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, app::width, app::height);
		app::texprogram->use();
		app::renderer->drawTile();
	}
	
	////////////////////////////////////

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, app::width, app::height);

	//app::texprogram->use();
	//buf1->readFrom();
	//app::quad->draw();

	nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
	glErrors("app::nuklear");

	glfwSwapBuffers(app::window);
	glErrors("app::swap");
}