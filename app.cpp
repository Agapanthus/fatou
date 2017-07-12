/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           *******************************
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

GLuint LoadPNG(std::string Filename)
{
	std::ifstream File;
	File.open(Filename.c_str(), std::ios::in | std::ios::binary);

	File.seekg(0, std::ios::end);//zum ende der datei springen

	int FileLength = File.tellg();
	File.seekg(0, std::ios::beg);

	std::vector<unsigned char> Buffer, Image;
	Buffer.resize(FileLength);

	File.read((char*)(&Buffer[0]), FileLength);

	File.close();

	unsigned long XSize = 0, YSize = 0;

	decodePNG(Image, XSize, YSize, &Buffer[0], (unsigned long)Buffer.size());

	//jetzt die Textur erstellen


	GLuint NewTexture;
	glGenTextures(1, &NewTexture);
	glBindTexture(GL_TEXTURE_2D, NewTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, XSize, YSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, &Image[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return NewTexture;
}
app::app() : 
	cx(0.00000001f), cy(3.0f),
	iter(100), 
	zoomx(3.0f), zoomy(2.0f), 
	posx(0.0f), posy(0.0f),
	animSpeed(1000.0f),
	lastTime(UINT32_MAX)
{
	for (size_t p = 0; p < MAX_POLY; p++) coe[p] = coet[p] = 0.0f;
	coet[0] = -1.0f;
	coet[3] = 1.0f;
		
	app::program.reset(new shader(mainVertexShader, mainFragmentShader));
	app::uniform.screenTexture = app::program->getUniform("screenTexture");
	app::uniform.c = app::program->getUniform("c");
	app::uniform.iter = app::program->getUniform("iter");
	app::uniform.zoom = app::program->getUniform("zoom");
	app::uniform.pos = app::program->getUniform("pos");
	app::uniform.coe = app::program->getUniform("coe");
	app::uniform.coec = app::program->getUniform("coec");

	// TODO: app::guiProgram.reset(NULL);

	///////////////////////////////////////////////////

	glEnable(GL_TEXTURE_2D);
	colorMap.reset(new texture("../hue.png"));
	

	text = LoadPNG("../hue.png");

	quad.reset(new vao(quadVertices, sizeof(quadVertices), 6));
}

app::~app() {

};

void app::reshape(int w, int h) {
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
void app::keypressed(GLuint key) {

}
void app::mousemove(int x, int y) {

}
void app::mousestatechanged(mousebutton button, bool pressed) {

}

void app::render() {

	bool Change = false;

	unsigned int timeElapsed = gtimeGet() - lastTime;
	if (lastTime == UINT32_MAX) timeElapsed = 1,
	lastTime = gtimeGet();

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

	glutPostRedisplay();
}
void app::display() {
	glErrors("app::display");
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	app::program->use();

	glUniform1i(app::uniform.screenTexture, 0);
	glUniform2f(app::uniform.c, cx, cy);
	glUniform2f(app::uniform.zoom, zoomx, zoomy);
	glUniform2f(app::uniform.pos, posx, posy);
	glUniform1i(app::uniform.iter, iter);
	glUniform1fv(app::uniform.coe, MAX_POLY, coe);
	glUniform1i(app::uniform.coec, coec);
	glErrors("app::uniform");

	app::colorMap->use(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, text);
	app::quad->draw();
	
	glutSwapBuffers();
}