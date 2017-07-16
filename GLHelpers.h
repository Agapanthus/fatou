/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU         ]********************************
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

// Available GPURAM (for textures) in kilobytes. -1 if unknown.
int getGPURAM();

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         shader          ]*******************************/

class shader {
public:
	shader(const string &vertexShader, const string &fragmentShader);
	~shader();
	void use();
	GLint getUniform(const string &name);
protected:
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
protected:
	void init(const std::vector<unsigned char> &image, unsigned int width, unsigned int height);
	GLuint tex;
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          VAO            ]*******************************/

class vao {
public:
	vao(const float* data, GLsizeiptr dataSize, GLsizei faces, bool dynamic = false);
	void update(const float* data, GLsizeiptr dataSize, GLsizei faces);
	void draw();
	~vao();
protected:
	GLuint vaodata, vbodata;
	GLsizei faces;
	bool dynamic;
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          sQuad          ]*******************************/

class sQuad : protected vao {
public:
	sQuad();
	void draw(ARect pos, ARect post, bool transTexCoord = false);
	~sQuad();
protected:
	float quadVertices[24];
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Sync buffer       ]*******************************/

class syncBuffer {
public:
	syncBuffer(AiSize iSize, bool useMipmap, GLuint quality = GL_LINEAR, GLuint qualityM = GL_NEAREST);
	~syncBuffer();
	void writeTo(std::function<void(void)> content);
	void scale(AiSize iSize, bool useMipmap);
	void readFrom(GLuint textureID = UINT32_MAX);
	void framebufferRead();
	void framebufferWrite();
	AiSize getSize();
protected:
	GLuint framebuffer, rbo, tex;
	AiSize iSize;
	bool useMipmap;
	GLuint quality;
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Async buffer      ]*******************************/

class asyncBuffer {
public:
	asyncBuffer();
	~asyncBuffer();
};