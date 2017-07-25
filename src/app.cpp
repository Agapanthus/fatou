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


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           app           ]*******************************/

unsigned int MaxFRate = 67;

static const char *densitySelect[] = { "0.25", "1", "4", "9", "16", "36", "100" };
static const float densityTable[] = { 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 6.0f, 10.0f };


app::app(GLFWwindow *window, nk_context* ctx) :
	cx(0.00000001f), cy(3.0f),
	iter(100),
	zoomx(1.0f), zoomy(1.0f), zoom(2.0f),
	posx(0.0f), posy(0.0f),
	animSpeed(1000.0f),
	lastTime(UINT32_MAX),
	window(window), ctx(ctx),
	navigationMode(navigation_lrclick_combi),
	supers(1.0f), tiles(1,1),
	targetFRate(25),
	biasPower(-40), show_about(false), isFullscreen(false),
	show_polynomial(true),
	trace_length(0), show_roots(false), show_tooltips(true),
	maxDensity(2), maxDensityI(densityTable[maxDensity])
{

	glErrors("app::before");
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

	
	
	///////////////////////////////////////////////////

	//glEnable(GL_TEXTURE_2D); // Not necessary in OpenGL 3
	glErrors("app::glEnable");

	///////////////////////////

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize); // TODO: Use this information!
	glErrors("main::getMaxTextureSize");

	////////////////////////////

	glfwGetWindowSize(app::window, &(app::width), &(app::height));

	app::colorMap.reset(new texture(basePath + "res/hue.png"));
	
	app::renderF = [this](void) -> void {
		//glClear(GL_COLOR_BUFFER_BIT);
		//glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

		app::program->use();

		glUniform2f(app::uniform.c, cx, cy);
		glUniform1i(app::uniform.iter, iter);
		glUniform1fv(app::uniform.coe, MAX_POLY, coe);
		glUniform1i(app::uniform.coec, coec);
		glErrors("app::uniform");

		app::colorMap->use(GL_TEXTURE0);
	};


	app::renderer.reset(new aRenderer(AiSize(app::width, app::height), tiles, app::maxDensityI, float(app::targetFRate), renderF));
	
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
	// TODO: When the application window is scaled while rendering with low framerate (<=5) under windows, the application sometimes stops presenting frames anymore!
	
	app::width = w;
	app::height = h;
	glViewport(0, 0, app::width, app::height);

	app::renderer->setSize(AiSize(app::width, app::height));
	app::Change = true;

	glErrors("app::reshape");
}

void app::setFullscreen(bool fullscreen) {
	if (app::isFullscreen == fullscreen) return;
	if (fullscreen) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(app::window, 0, 0, 0, mode->width, mode->height, mode->refreshRate);
		glfwSetWindowSize(app::window, mode->width / 2, mode->height / 2);
		glfwSetWindowPos(app::window, mode->width / 4, mode->height / 4);
	}
	app::isFullscreen = fullscreen;
}

void app::logic() {
	app::Change = false;
	app::pChange = false;

	unsigned int now = gtimeGet();
	unsigned int timeElapsed = now - app::lastTime;
	if (app::lastTime == UINT32_MAX) timeElapsed = 1;
	app::lastTime = now;

	///////////////////////////// GUI

	//static struct nk_color background = nk_rgb(28, 48, 62);

	nk_glfw3_new_frame();

	
	const struct nk_input *in = &ctx->input;
	struct nk_rect bounds;
#define TOOLTIP(TEXT) \
	if(app::show_tooltips) {\
		bounds = nk_widget_bounds(ctx); \
		if (nk_input_is_mouse_hovering_rect(in, bounds) \
			&& nk_window_is_hovered(ctx) /*otherwise tooltips will even appear when the hovered element is invisible. !!! TODO: But they still appear, when hovering another window which is ontop of them!*/) \
			nk_tooltip(ctx, TEXT); \
	}

	if (show_polynomial) {
		if (nk_begin(ctx, "Fractal", nk_rect(10, 330, 220, 200),
			NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
			NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE)) {


			if (nk_tree_push(ctx, NK_TREE_TAB, "Modify", NK_MINIMIZED)) {
				nk_layout_row_dynamic(ctx, 25, 1);
				nk_property_int(ctx, "Degree:", 0, &(app::targetFRate), int(MaxFRate * 0.9f), 1000, 0.1f);
				nk_property_float(ctx, "Value:", 5, &(app::animSpeed), (MaxFRate * 0.9f), 10, 0.1f);
				nk_tree_pop(ctx);
			}
		}
		nk_end(ctx);
	}

	if (nk_begin(ctx, "Fatou", nk_rect(10, 10, 220, 320),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
		NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{

		nk_layout_row_dynamic(ctx, 25, 2);
		if (nk_button_label(ctx, "About")) {
			show_about = true;
		}
		if (nk_button_label(ctx, "Help")) {
			app::setFullscreen(false);
#ifdef OS_WIN
			ShellExecute(NULL, "open", "help\\index.html", NULL, NULL, SW_SHOW);
#elif defined(OS_LIN)
			// TODO: Where will this work properly? Where won't?
			system("see ./help/index.html");
			// system("helpfile.html"); firefox, xdg-open, /usr/bin/x-www-browser
#else
			#error IMPL
#endif
		}

		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_button_label(ctx, "Exit")) {
			glfwSetWindowShouldClose(app::window, true);
		}


		nk_layout_row_dynamic(ctx, 25, 1);
		if (!isFullscreen) {
			if (nk_button_label(ctx, "Enter Fullscreen")) {
				app::setFullscreen(true);
			}
		}
		else {
			if (nk_button_label(ctx, "Exit Fullscreen")) {
				app::setFullscreen(false);
			}
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Renderer", NK_MAXIMIZED)) {

			nk_layout_row_dynamic(ctx, 25, 1);
			int iter = app::iter;
			nk_property_int(ctx, "Iterations:", 1, &(iter), 1000, 10, iter * 0.001f + 0.03f);
			if (iter != app::iter) {
				Change = true;
				app::iter = iter;
			}

			nk_layout_row_dynamic(ctx, 25, 1);
			TOOLTIP("Magnitudes below 2^bias are assumed to have converged. Too small values cause floating point errors! Use double precision for better quality!");
			nk_property_int(ctx, "Bias:", -50, &biasPower, 10, 1, 0.1f);
			if (app::cx != pow(2.0f, float(biasPower))) {
				Change = true;
				app::cx = pow(2.0f, float(biasPower));
			}

			nk_layout_row_dynamic(ctx, 25, 1);
			int dummy = 0;
			nk_property_int(ctx, "Anim. speed:", -50, &dummy, 10, 1, 0.1f);
			// TODO: 

			nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 2);
			nk_layout_row_push(ctx, 0.6f);
			TOOLTIP("High densities need VERY much RAM! To compensate that, increase the number of tiles.");
			nk_label(ctx, "Max. Density:", NK_TEXT_RIGHT);
			nk_layout_row_push(ctx, 0.4f);
			app::maxDensity = nk_combo(ctx, densitySelect, sizeof(densitySelect)/sizeof(densitySelect[0]), app::maxDensity, 25, nk_vec2(80, 110));
			nk_layout_row_end(ctx);
			if (app::maxDensityI != densityTable[app::maxDensity]) {
				Change = true;
				app::maxDensityI = densityTable[app::maxDensity];
				app::renderer->setMaxEffort(app::maxDensityI);
			}

			nk_layout_row_dynamic(ctx, 25, 1);
			nk_property_int(ctx, "Min. framerate:", 5, &(app::targetFRate), int(MaxFRate * 0.9f), 10, 0.1f);
			app::renderer->setTargetFramerate(float(app::targetFRate));

			nk_layout_row_dynamic(ctx, 20, 2);
			TOOLTIP("samples per pixel (density > 1.0 leads to antialiasing)");
			nk_label(ctx, "Density:", NK_TEXT_RIGHT);
			float dens = app::renderer->getPixelDensity();
			nk_label(ctx, toString(dens*dens, 2).c_str(), NK_TEXT_LEFT);


			nk_layout_row_dynamic(ctx, 20, 2);
			nk_label(ctx, "Framerate:", NK_TEXT_RIGHT);
			nk_label(ctx, toString(app::renderer->getFramerate(), 2).c_str(), NK_TEXT_LEFT);

			nk_layout_row_dynamic(ctx, 20, 1);
			TOOLTIP("Progressively increases density. This option is ignored while running animations!");
			static int titlebar = true;
			nk_checkbox_label(ctx, "Progressive renderer", &titlebar);
			// TODO

			nk_layout_row_dynamic(ctx, 20, 1);
			TOOLTIP("Instead of discarding outdated image data, it is progressively interpolated with new data.");
			nk_checkbox_label(ctx, "Progressive update", &titlebar);
			// TODO: Dazu auch eine Prozentanzeige "Veraltete Texel: 23.5%", die dann schnell absinkt

			nk_layout_row_dynamic(ctx, 20, 1);
			TOOLTIP("Enables \"deep zoom\" and improves image quality but is much slower!");
			static int titlebar2 = true;
			nk_checkbox_label(ctx, "Double precision", &titlebar2);
			// TODO

			nk_layout_row_dynamic(ctx, 25, 1);
			TOOLTIP("High numbers of tiles reduce RAM consumption but are computationally more intense. This option is ignored while progressive rendering is used.");
			nk_property_int(ctx, "Tiles:", 1, &(app::iter), 1000, 10, app::iter * 0.001f + 0.03f);
			// TODO

			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "View", NK_MINIMIZED)) {
			
			nk_layout_row_dynamic(ctx, 20, 1);
			nk_checkbox_label(ctx, "Show About", &show_about);
			
			nk_layout_row_dynamic(ctx, 20, 1);
			nk_checkbox_label(ctx, "Show Tooltips", &show_tooltips); // TODO: Add more Tooltips!

			nk_layout_row_dynamic(ctx, 20, 1);
			nk_checkbox_label(ctx, "Fractal Settings", &show_polynomial);

			nk_layout_row_dynamic(ctx, 20, 1);
			nk_checkbox_label(ctx, "Show Roots", &show_roots);
			// TODO

			nk_layout_row_dynamic(ctx, 25, 1);
			nk_property_int(ctx, "Trace Iterations:", 0, &(app::trace_length), 1000, 10, 0.1f + 0.03f);
			// TODO

			// ToDO: Value at

			nk_tree_pop(ctx);

		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Navigation", NK_MINIMIZED)) {


			nk_layout_row_dynamic(ctx, 20, 1);
			TOOLTIP("Instead of discarding all data when the camera is moving, some Pixels are calcluted by interpolation of old ones. Might lead to heterogenous pixel density!");
			static int titlebar = true;
			nk_checkbox_label(ctx, "Recycle Texels", &titlebar);
			// TODO

			nk_layout_row_dynamic(ctx, 25, 1);
			if (nk_button_label(ctx, "Navigate there")) {
				glfwSetWindowShouldClose(app::window, true);
			}		
			// TODO

			nk_tree_pop(ctx);

		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Colors", NK_MINIMIZED)) {

			// TODO!
			// Color Modes: Colormap circular, Colormap rectangular, Colormap Linear
			// Color map!
			// Increase White with length of radius
			// Modify Curve for Blackness!

			// Orbit Traps! + Images!

			nk_layout_row_dynamic(ctx, 25, 1);
			nk_property_float(ctx, "Gradient:", 0.0f, &(app::cy), 10.0f, 1, 0.02f);

			nk_tree_pop(ctx);

		}


		

		if (nk_tree_push(ctx, NK_TREE_TAB, "Import / Export", NK_MINIMIZED)) {

			nk_layout_row_dynamic(ctx, 25, 2);
			nk_property_int(ctx, "Width:", 0, &(app::targetFRate), int(MaxFRate * 0.9f), 1000, 0.1f);
			nk_property_int(ctx, "Height:", 0, &(app::targetFRate), int(MaxFRate * 0.9f), 1000, 0.1f);


			nk_layout_row_dynamic(ctx, 25, 1);
			if (nk_button_label(ctx, "Save PNG")) {
				glfwSetWindowShouldClose(app::window, true);
			}
			// TODO

			nk_layout_row_dynamic(ctx, 25, 1);
			if (nk_button_label(ctx, "Save Video")) {
				glfwSetWindowShouldClose(app::window, true);
			}
			// TODO

			nk_layout_row_dynamic(ctx, 25, 1);
			if (nk_button_label(ctx, "Save Configuration")) {
				glfwSetWindowShouldClose(app::window, true);
			}
			// TODO

			nk_layout_row_dynamic(ctx, 25, 1);
			if (nk_button_label(ctx, "Load Configuration")) {
				glfwSetWindowShouldClose(app::window, true);
			}
			// TODO


			nk_tree_pop(ctx);

		}


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

	if (show_about) {
		struct nk_rect s = { app::width / 2.0f - 150, app::height / 2.0f - 100, 300.0f, 175.0f };
		if (nk_begin(ctx, "About", s,
			NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {
			nk_layout_row_dynamic(ctx, 20, 1);
			nk_label(ctx, "Fatou 0.1.1", NK_TEXT_CENTERED);
			nk_label(ctx, "by Eric Skaliks", NK_TEXT_CENTERED);
			nk_label(ctx, "Fatou is licensed under the MIT License.", NK_TEXT_CENTERED);
			nk_label(ctx, "See https://github.com/Agapanthus/fatou.", NK_TEXT_CENTERED);
			nk_layout_row_dynamic(ctx, 25, 1);
			if (nk_button_label(ctx, "Close")) {
				show_about = false;
			}	
			nk_end(ctx);
		}
		else show_about = false;
	}

	bool nuklearHandledEvents = true;
	if (0 == nk_item_is_any_active(ctx) && 0 == nk_window_is_any_hovered(ctx)) {
		nuklearHandledEvents = false;
		setStyle(app::ctx, true);

	}
	else {
		setStyle(app::ctx, false);
	}


		///////////////////// Navigate

		switch (navigationMode) {
		case navigation_drag_and_wheel:
			// TODO
			break;
		case navigation_none:
			break;
		case navigation_lrclick_combi:
		default:
		
			// TODO: Only move, if this button-hold was induced by a button click outside any nuklear-widgets!

			bool left = nuklearHandledEvents ? false : glfwGetMouseButton(app::window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
			bool right = nuklearHandledEvents ? false : glfwGetMouseButton(app::window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
			
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
			//float aspect = float(app::width) / float(app::height);
			app::zoomx = app::zoom * (app::width/1000.0f);
			app::zoomy = app::zoom * (app::height/1000.0f);

			if (left || right) {
				double xpos, ypos;
				glfwGetCursorPos(app::window, &xpos, &ypos);
				app::posx += (float(xpos) / float(app::width) - 0.5f) * app::zoomx * moveSpeed;
				app::posy -= (float(ypos) / float(app::height) - 0.5f) * app::zoomy * moveSpeed;
				app::pChange = true;
			}

			break;
		}


	///////////////////// Animate Polynomials
	for (size_t p = 0; p < MAX_POLY; p++) {
		if (coet[p] != 0.0f || coe[p] != 0.0f) {
			coec = p;
			if (abs(coe[p] - coet[p]) < 0.000001f) coe[p] = coet[p];
			else {
				float blendTim = tanh(0.01f * timeElapsed / (animSpeed / 1000.0f));
				coe[p] = coe[p] * (1.0f - blendTim) + coet[p] * blendTim;
				app::Change = true;
			}
		}
	}

//	coe[1] = (sin(clock() / 10000.0f + 3.16*1) ) * 1.5f + 0.5f;
	app::renderer->view(APoint(app::posx, app::posy), ASize(app::zoomx, app::zoomy), [this](APoint p, ASize z)->void {
		app::program->use();
		glUniform2f(app::uniform.zoom, z.w, z.h);
		glUniform2f(app::uniform.pos, p.x, p.y);
	});

}
void app::render() {
	/*static bool changed = true;
	app::progrenderer->render(app::renderF, changed);
	changed = false;
	*/
}
void app::display() {
		

	///////////////////////////////////

	glErrors("app::display");

	/*
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, app::width, app::height);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	*/

	/*
	app::optim->optimize((sRenderer*)app::renderer.data());
	while (app::renderer->renderTile([this](ARect tile) -> void { app::renderF(); })) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, app::width, app::height);
		app::texprogram->use();
		app::renderer->drawTile();
	}
	*/

	////////////////////////////////////

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, app::width, app::height);

	{
		double xpos, ypos;
		glfwGetCursorPos(app::window, &xpos, &ypos);
		app::renderer->render(int(xpos), int(ypos), app::Change /*|| app::pChange*/);
	}
	/*
	app::texprogram->use();
	app::progrenderer->draw();
	*/
	//app::texprogram->use();
	//buf1->readFrom();
	//app::quad->draw();

	nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
	glErrors("app::nuklear");

}
