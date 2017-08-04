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


#define USE_TEXTURE_ARRAY


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         Util            ]*******************************/

extern AErrorCode opengl_fatalError, opengl_minorError, opengl_internalError, opengl_compileError;

class OpenGLException : public AException { public: OpenGLException(const AErrorCode &code, const string &details = "") : AException(code, details) {} };

void glErrors(const char* wheres);
unsigned int gtimeGet();

// Available GPURAM (for textures) in kilobytes. -1 if unknown.
//int getGPURAM();

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         shader          ]*******************************/

class shader {
public:
	shader(const string &vertexShader, const string &fragmentShader, const string &geometryShader = "");
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
	void use(GLenum textureID = GL_TEXTURE0);
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
	void draw(ARect pos = ARect(0.0f, 0.0f, 1.0f, 1.0f), ARect post = ARect(0.0f, 0.0f, 1.0f, 1.0f));
	~sQuad();
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         glLine          ]*******************************/

class glLine : protected vao {
public:
	glLine();
	void draw(ARect pos);
	~glLine();
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Sync buffer       ]*******************************/

class syncBuffer {
public:
	syncBuffer(AiSize iSize, bool useMipmap, GLenum quality = GL_LINEAR, GLenum qualityM = GL_NEAREST);
	~syncBuffer();
	void writeTo(std::function<void(void)> content);
//	void upload(AiSize size, const float *data);
	void scale(AiSize iSize, bool useMipmap);
	void readFrom(GLenum textureID = GL_TEXTURE0);
	void framebufferRead();
	void framebufferWrite();
	AiSize getSize();
protected:
	GLuint framebuffer, tex;
	AiSize iSize;
	bool useMipmap;
	GLenum quality;
};

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[      float storage      ]*******************************/

class floatStorage {
public:
	floatStorage(AiSize iSize);
	~floatStorage();
	inline void set(size_t x, size_t y, float a, float b, float c, float d) {
		fassert(int32(y) < iSize.h);
		fassert(int32(x) < iSize.w);

		size_t addr = (x+ y*iSize.w) * 4;
		data[addr + 0] = a;
		data[addr + 1] = b;
		data[addr + 2] = c;
		data[addr + 3] = d;
	}
	inline float get(size_t x, size_t y, size_t ele) {
		size_t addr = (x + y*iSize.w) * 4;
		return data[addr + ele];
	}
	void upload();
	void scale(AiSize iSize);
	void bind(GLenum textureID = GL_TEXTURE0);
	AiSize getSize();
protected:
	GLuint tex;
	AiSize iSize;
	vector<float> data;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[  Volumetric Sync buffer ]*******************************/

class syncBuffer3d {
public:
	syncBuffer3d(AiSize iSize, uint32 depth, GLenum quality = GL_NEAREST, GLenum qualityM = GL_NEAREST);
	~syncBuffer3d();
	void writeTo(uint32 layer);
	void scale(AiSize iSize);
	void bind(GLenum textureID = GL_TEXTURE0);
	AiSize getSize();
protected:
	GLuint *framebuffers, tex;
	AiSize iSize;
	uint32 depth;
	GLenum quality;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[      measure Time       ]*******************************/

class glQuery : ANoncopyable {
public:
	// Move constructor, but noncopyable!
	glQuery(glQuery &&move) : alive(true) {
		glQuery::query = query;
		move.alive = false;
	}
	glQuery & operator=(glQuery &&move) {
		if (this != &move) {
			alive = true;
			query = move.query;
			move.alive = false;
		}
		return *this;
	}
	glQuery(GLuint query) : query(query), alive(true) {
		glBeginQuery(GL_TIME_ELAPSED, query);
	}
	~glQuery() {
		if (alive) {
			glEndQuery(GL_TIME_ELAPSED);
			glErrors("glQuery::end");
		}
	}
protected:
	GLuint query;
	bool alive;
};
class glQueryCreator {
public:
	glQueryCreator() {
		glGenQueries(1, &query);
	}
	glQuery create() {
		return glQuery(query);
	}
	float getTime() {
		GLuint64 elapsed_time;
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
		return float(double(elapsed_time) / 1000000.0);
	}
	
protected:
	GLuint query;
};
