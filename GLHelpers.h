/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           *******************************
*
* File: GLHelpers.h
* Purpose: auxiliary classes.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         Util            ]*******************************/

void glErrors(const char* wheres);
unsigned int gtimeGet();

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         shader          ]*******************************/

class shader {
public:
	shader(const string &vertexShader, const string &fragmentShader);
	~shader();
	void use();
	GLint getUniform(const string &name);

private:
	GLuint program;
	GLuint upload(const string &shader, GLenum shaderType);
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        Texture          ]*******************************/

class texture {
public:
	texture(const std::vector<unsigned char> &image, unsigned int width, unsigned int height);
	texture(const std::string &filename);
	~texture();
	void use(GLuint textureID = UINT32_MAX);
private:
	void init(const std::vector<unsigned char> &image, unsigned int width, unsigned int height);
	GLuint tex;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          VAO            ]*******************************/

class vao {
public:
	vao(const float* data, GLsizeiptr dataSize, GLsizei faces);
	void draw();
	~vao();
private:
	GLuint vaodata, vbodata;
	GLsizei faces;
};