/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: GLHelpers.cpp
* Purpose: auxiliary classes.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "GLHelpers.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         Util            ]*******************************/

AErrorCode opengl_fatalError("opengl fatal"), opengl_minorError("opengl minor"), opengl_internalError("opengl internal"), opengl_compileError("opengl compile");


void glErrors(const char* wheres) {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		//fatalNote(string("OpenGL-Error@") + string(wheres) + ": " + std::to_string((int32)err)); //TODO:  +" " + string((const char*)gluErrorString(err)));
		throw OpenGLException(opengl_internalError, "@" + string(wheres) + "\ncode: " + toString(int32(err)));
	}
}

uint32 gtimeGet() {
	GLint64 timer;
	glGetInteger64v(GL_TIMESTAMP, &timer);
	return uint32( timer / 1000000 );
}

/*
int getGPURAM() {
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
#define VBO_FREE_MEMORY_ATI 0x87FB
#define TEXTURE_FREE_MEMORY_ATI 0x87FC
#define RENDERBUFFER_FREE_MEMORY_ATI 0x87FD
	glGetError();
	GLint values[4] = { 0,0,0,0 };
	glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, values);
	if (glGetError() == GL_NO_ERROR) return values[0];
	glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, values);
	if (glGetError() == GL_NO_ERROR) return values[0];
	return -1;
}*/

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        Shader           ]*******************************/

void debugShader(GLuint res, string type) {
	GLint log_length = 0;
	if (glIsShader(res))
		glGetShaderiv(res, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(res))
		glGetProgramiv(res, GL_INFO_LOG_LENGTH, &log_length);
	else {
		throw OpenGLException(opengl_fatalError, "shader::debugShader: Not a shader or a program");
		return;
	}
	char* logs = (char*)malloc(log_length);
	if (glIsShader(res))
		glGetShaderInfoLog(res, log_length, NULL, logs);
	else if (glIsProgram(res))
		glGetProgramInfoLog(res, log_length, NULL, logs);

	string Logs(logs);
	free(logs);

	throw OpenGLException(opengl_compileError, "Failed to compile " + type + " shader. Error log:\n" + Logs);
}

shader::shader(const string &vertexShader, const string &fragmentShader, const string &geometryShader) {
	glErrors("shader::before");
	shader::program = glCreateProgram();
	
	GLuint vshader = shader::upload(vertexShader, GL_VERTEX_SHADER);
	if (!vshader)
		throw OpenGLException(opengl_fatalError, "Failed to load vertex shader");
	glAttachShader(shader::program, vshader);

	if (geometryShader.length() > 0) {
		GLuint gshader = shader::upload(geometryShader, GL_GEOMETRY_SHADER);
		if (!gshader)
			throw OpenGLException(opengl_fatalError, "Failed to load geometry shader");
		glAttachShader(shader::program, gshader);
	}
	
	GLuint fshader = shader::upload(fragmentShader, GL_FRAGMENT_SHADER);
	if (!fshader)
		throw OpenGLException(opengl_fatalError, "Failed to load fragment shader");
	glAttachShader(shader::program, fshader);

	glLinkProgram(shader::program);
	GLboolean link_ok = GL_FALSE;
	glGetProgramiv(shader::program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		debugShader(program, "linking");
		glDeleteProgram(program);
	}
	glErrors("shader::construct");
}

GLuint shader::upload(const string &shader, GLenum shaderType) {
	GLuint res = glCreateShader(shaderType);
	const GLchar* sources[] = {
		// Define GLSL version
		//"#version 330 core\n" // OpenGL 3.3
		"#version 400 core\n" // OpenGL 4.0
		,shader.c_str() };
	glShaderSource(res, 2, sources, NULL);

	glCompileShader(res);
	GLboolean compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		string shader_name = "";
		switch (shaderType) {
		case GL_GEOMETRY_SHADER: shader_name = "geometry"; break;
		case GL_VERTEX_SHADER: shader_name = "vertex"; break;
		case GL_FRAGMENT_SHADER: shader_name = "fragment"; break;
		default: shader_name = "unkown"; break;
		}
		debugShader(res, shader_name);
		glDeleteShader(res);
		return 0;
	}

	return res;
}

shader::~shader() {
	glDeleteProgram(shader::program);
}

void shader::use() {
	glUseProgram(shader::program);
	glErrors("shader::use");
}

GLint shader::getUniform(const string &name) {
	GLint uniform = glGetUniformLocation(program, name.c_str());
	if (uniform == -1) throw OpenGLException(opengl_minorError, "Failed binding uniform " + name);
	glErrors("shader::getuniform");
	return uniform;
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        Texture          ]*******************************/

texture::texture(const std::vector<unsigned char> &image, unsigned int width, unsigned int height) {
	texture::init(image, width, height);	
}

void texture::init(const std::vector<unsigned char> &image, unsigned int width, unsigned int height) {
	glErrors("texture::before");
	glGenTextures(1, &(texture::tex));
	glBindTexture(GL_TEXTURE_2D, texture::tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
	//glGenerateMipmap(GL_TEXTURE_2D); 
	glErrors("texture::construct");
}

texture::texture(const std::string &filen) {
	std::vector<unsigned char> buffer, image;

	std::ifstream file(filen, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.good()) throw OpenGLException(opengl_fatalError, "Texture >>" + filen + "<< not found!");

	//get filesize
	std::streamsize size = 0;
	if (file.seekg(0, std::ios::end).good()) size = file.tellg();
	if (file.seekg(0, std::ios::beg).good()) size -= file.tellg();

	//read contents of the file into the vector
	if (size > 0) {
		buffer.resize((size_t)size);
		file.read((char*)(&buffer[0]), size);
	}
	else buffer.clear();
	file.close();

	unsigned long w, h;
	int error = decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());

	if (error != 0) throw OpenGLException(opengl_fatalError, "Error reading png-file >>" + filen + "<<!");
	
	texture::init(image, w, h);
}

texture::~texture() {
	glDeleteTextures(1, &(texture::tex));
}

void texture::use(GLenum textureID) {
	glActiveTexture(textureID);
	glBindTexture(GL_TEXTURE_2D, texture::tex);
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          VAO            ]*******************************/

vao::vao(const float* data, GLsizeiptr dataSize, GLsizei faces, bool dynamic) : faces(faces), dynamic(dynamic) {
	glErrors("vao::before");

	glGenVertexArrays(1, &(vao::vaodata));
	glGenBuffers(1, &(vao::vbodata));
	glBindVertexArray((vao::vaodata));
	glBindBuffer(GL_ARRAY_BUFFER, (vao::vbodata));
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glErrors("vao::construct");
}

void vao::update(const float* data, GLsizeiptr dataSize, GLsizei faces) {
	glBindBuffer(GL_ARRAY_BUFFER, (vao::vbodata));
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	vao::faces = faces;
	glErrors("vao::update");
}

vao::~vao() {
	glDeleteVertexArrays(1, &(vao::vaodata));
	glDeleteBuffers(1, &(vao::vbodata));
}

void vao::draw() {
	glBindVertexArray(vao::vaodata);
	glDrawArrays(GL_TRIANGLES, 0, faces);
	glBindVertexArray(0);
	glErrors("vao::draw");
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          sQuad          ]*******************************/

static float quadVertices_t[24];
sQuad::sQuad() : vao(quadVertices_t, sizeof(quadVertices_t), 6, true) {

}

void sQuad::draw(ARect pos, ARect post) {
	pos = pos * 2 - 1;
	float quadVertices[24];
	quadVertices[0] = pos.left;
	quadVertices[1] = pos.bottom;
	quadVertices[4] = pos.left;
	quadVertices[5] = pos.top;
	quadVertices[8] = pos.right;
	quadVertices[9] = pos.top;

	quadVertices[12] = pos.left;
	quadVertices[13] = pos.bottom;
	quadVertices[16] = pos.right;
	quadVertices[17] = pos.top;
	quadVertices[20] = pos.right;
	quadVertices[21] = pos.bottom;

	quadVertices[2] = post.left;
	quadVertices[3] = post.bottom;
	quadVertices[6] = post.left;
	quadVertices[7] = post.top;
	quadVertices[10] = post.right;
	quadVertices[11] = post.top;

	quadVertices[14] = post.left;
	quadVertices[15] = post.bottom;
	quadVertices[18] = post.right;
	quadVertices[19] = post.top;
	quadVertices[22] = post.right;
	quadVertices[23] = post.bottom;
	
	vao::update(quadVertices, sizeof(quadVertices), 6);
	vao::draw();
}

sQuad::~sQuad() {
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         glLine          ]*******************************/

glLine::glLine() : vao(quadVertices_t, sizeof(quadVertices_t), 6, true) {
}

void glLine::draw(ARect pos) {
	pos = pos * 2 - 1;
	
	float lineVertices[8];
	lineVertices[0] = pos.left;
	lineVertices[1] = pos.top;
	lineVertices[4] = pos.right;
	lineVertices[5] = pos.bottom;
	vao::update(lineVertices, sizeof(lineVertices), 1);

	glBindVertexArray(vao::vaodata);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glErrors("glline::draw");
}

glLine::~glLine() {

}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Sync buffer       ]*******************************/

syncBuffer::syncBuffer(AiSize iSize, bool useMipmap, GLenum quality, GLenum qualityM) :
	iSize(iSize), useMipmap(useMipmap), quality(quality) {
	glGenFramebuffers(1, &framebuffer);
	glErrors("syncBuffer::generate");
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &(syncBuffer::tex));
	glBindTexture(GL_TEXTURE_2D, syncBuffer::tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, syncBuffer::iSize.w, syncBuffer::iSize.h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmap ? GL_LINEAR_MIPMAP_LINEAR : quality);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, qualityM);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, syncBuffer::tex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
		throw OpenGLException(opengl_fatalError, "syncBuffer: Framebuffer is not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glErrors("syncBuffer::construct");
}

void syncBuffer::scale(AiSize iSize, bool useMipmap) {
	syncBuffer::iSize = iSize;
	
	glBindTexture(GL_TEXTURE_2D, syncBuffer::tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, syncBuffer::iSize.w, syncBuffer::iSize.h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	if (useMipmap != syncBuffer::useMipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmap ? GL_LINEAR_MIPMAP_LINEAR : syncBuffer::quality);
		syncBuffer::useMipmap = useMipmap;
	}
}

syncBuffer::~syncBuffer() {
	glDeleteTextures(1, &(syncBuffer::tex));
	glDeleteFramebuffers(1, &(syncBuffer::framebuffer));
	glErrors("syncBuffer::destruct");
}

void syncBuffer::writeTo(std::function<void(void)> content) {
	glBindFramebuffer(GL_FRAMEBUFFER, syncBuffer::framebuffer);
	glViewport(0, 0, syncBuffer::iSize.w, syncBuffer::iSize.h);
	if(content) content();
	if (syncBuffer::useMipmap) {
		glBindTexture(GL_TEXTURE_2D, syncBuffer::tex);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glErrors("syncBuffer::write");
}

void syncBuffer::readFrom(GLenum textureID) {
	glActiveTexture(textureID);
	glBindTexture(GL_TEXTURE_2D, syncBuffer::tex);
	glErrors("syncBuffer::read");
}

void syncBuffer::framebufferRead() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, syncBuffer::framebuffer);
}

void syncBuffer::framebufferWrite() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, syncBuffer::framebuffer);
}

AiSize syncBuffer::getSize() {
	return syncBuffer::iSize;
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[      float storage      ]*******************************/

floatStorage::floatStorage(AiSize iSize) : iSize(iSize) {
	glGenTextures(1, &(floatStorage::tex));
	glBindTexture(GL_TEXTURE_2D, floatStorage::tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	floatStorage::data.resize(iSize.w * iSize.h * 4);
}

floatStorage::~floatStorage() {
	glDeleteTextures(1, &(floatStorage::tex));
}

void floatStorage::upload() {
	glBindTexture(GL_TEXTURE_2D, floatStorage::tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, floatStorage::iSize.w, floatStorage::iSize.h, 0, GL_RGBA, GL_FLOAT, floatStorage::data.data());
}

void floatStorage::scale(AiSize iSize) {
	floatStorage::iSize = iSize;
	floatStorage::data.resize(iSize.w * iSize.h * 4);
}

void floatStorage::bind(GLenum textureID) {
	glActiveTexture(textureID);
	glBindTexture(GL_TEXTURE_2D, floatStorage::tex);
}

AiSize floatStorage::getSize() {
	return floatStorage::iSize;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[  Volumetric Sync buffer ]*******************************/

syncBuffer3d::syncBuffer3d(AiSize iSize, uint32 depth, GLenum quality, GLenum qualityM) :
	iSize(iSize), depth(depth), quality(quality) {
	syncBuffer3d::framebuffers = new GLuint[syncBuffer3d::depth];

#ifdef USE_TEXTURE_ARRAY
	glGenFramebuffers(1, framebuffers);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &(syncBuffer3d::tex));
	glBindTexture(GL_TEXTURE_2D_ARRAY, syncBuffer3d::tex);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, syncBuffer3d::iSize.w, syncBuffer3d::iSize.h, syncBuffer3d::depth,	0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glErrors("syncBuffer3D::Allocate");
	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, qualityM);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, quality);
	
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, syncBuffer3d::tex, 0);
	glErrors("syncBuffer3D::FramebufferTexture");
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw OpenGLException(opengl_fatalError, "syncBuffer3d: Framebuffer is not complete!");
#else
	glGenFramebuffers(depth, framebuffers);

	glGenTextures(1, &(syncBuffer3d::tex));
	glBindTexture(GL_TEXTURE_3D, syncBuffer3d::tex);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, syncBuffer3d::iSize.w, syncBuffer3d::iSize.h, syncBuffer3d::depth, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, qualityM);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, quality);

	for (size_t i = 0; i < depth; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, syncBuffer::tex, 0);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, syncBuffer3d::tex, 0, i);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			throw OpenGLException(opengl_fatalError, "syncBuffer3d: Framebuffer " + toString(i) + " is not complete!");
	}
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glErrors("syncBuffer3d::construct");
}
void syncBuffer3d::scale(AiSize iSize) {
	syncBuffer3d::iSize = iSize;
	
#ifdef USE_TEXTURE_ARRAY
	glBindTexture(GL_TEXTURE_2D_ARRAY, syncBuffer3d::tex);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, syncBuffer3d::iSize.w, syncBuffer3d::iSize.h, syncBuffer3d::depth, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

#else
	glBindTexture(GL_TEXTURE_3D, syncBuffer3d::tex);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, syncBuffer3d::iSize.w, syncBuffer3d::iSize.h, syncBuffer3d::depth, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
#endif

	glErrors("syncBuffer3d::scale");
}
syncBuffer3d::~syncBuffer3d() {
	glDeleteTextures(1, &(syncBuffer3d::tex));

#ifdef USE_TEXTURE_ARRAY
	glDeleteFramebuffers(1, syncBuffer3d::framebuffers);
#else
	glDeleteFramebuffers(syncBuffer3d::depth, syncBuffer3d::framebuffers);
#endif
	delete[] syncBuffer3d::framebuffers;
	glErrors("syncBuffer3d::destruct");
}
void syncBuffer3d::writeTo(uint32 layer) {
#ifdef USE_TEXTURE_ARRAY
	glBindFramebuffer(GL_FRAMEBUFFER, syncBuffer3d::framebuffers[0]);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, syncBuffer3d::framebuffers[layer]);
#endif
	glViewport(0, 0, syncBuffer3d::iSize.w, syncBuffer3d::iSize.h);
	glErrors("syncBuffer3d::writeTo");
}
void syncBuffer3d::bind(GLenum textureID) {
	glActiveTexture(textureID);
#ifdef USE_TEXTURE_ARRAY
	glBindTexture(GL_TEXTURE_2D_ARRAY, syncBuffer3d::tex);
#else
	glBindTexture(GL_TEXTURE_3D, syncBuffer3d::tex);
#endif
	glErrors("syncBuffer3d::bind");
}
AiSize syncBuffer3d::getSize() {
	return syncBuffer3d::iSize;
}
