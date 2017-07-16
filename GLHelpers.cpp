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

void glErrors(const char* wheres) {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		fatalNote(string("OpenGL-Error@") + string(wheres) + ": " + std::to_string(err) + " " + string((const char*)gluErrorString(err)));
	}
}
#ifdef OS_WIN
unsigned int gtimeGet() {
	return clock();
}
#else
#error IMPL
#endif

int getGPURAM() {
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
#define VBO_FREE_MEMORY_ATI 0x87FB
#define TEXTURE_FREE_MEMORY_ATI 0x87FC
#define RENDERBUFFER_FREE_MEMORY_ATI 0x87FD
	glGetError();
	int values[4] = { 0,0,0,0 };
	glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, values);
	if (glGetError() == GL_NO_ERROR) return values[0];
	glGetIntegerv(TEXTURE_FREE_MEMORY_ATI, values);
	if (glGetError() == GL_NO_ERROR) return values[0];
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        Shader           ]*******************************/

void debugShader(GLuint res) {
	GLint log_length = 0;
	if (glIsShader(res))
		glGetShaderiv(res, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(res))
		glGetProgramiv(res, GL_INFO_LOG_LENGTH, &log_length);
	else {
		fatalNote("printlog: Not a shader or a program");
		return;
	}
	char* logs = (char*)malloc(log_length);
	if (glIsShader(res))
		glGetShaderInfoLog(res, log_length, NULL, logs);
	else if (glIsProgram(res))
		glGetProgramInfoLog(res, log_length, NULL, logs);

	fatalNote(logs);

	free(logs);

	throw exception("Failed to compile shader");
}

shader::shader(const string &vertexShader, const string &fragmentShader) {
	glErrors("shader::before");
	shader::program = glCreateProgram();
	
	GLuint vshader = shader::upload(vertexShader, GL_VERTEX_SHADER);
	if (!vshader)
		throw exception("Failed to load vertex shader");
	glAttachShader(shader::program, vshader);
	
	GLuint fshader = shader::upload(fragmentShader, GL_FRAGMENT_SHADER);
	if (!fshader)
		throw exception("Failed to load fragment shader");
	glAttachShader(shader::program, fshader);

	glLinkProgram(shader::program);
	GLint link_ok = GL_FALSE;
	glGetProgramiv(shader::program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		debugShader(program);
		glDeleteProgram(program);
	}
	glErrors("shader::construct");
}

GLuint shader::upload(const string &shader, GLenum shaderType) {
	GLuint res = glCreateShader(shaderType);
	const GLchar* sources[] = {
		// Define GLSL version
		"#version 330 core\n" // OpenGL 3.3
		,shader.c_str() };
	glShaderSource(res, 2, sources, NULL);

	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		debugShader(res);
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
	if (uniform == -1)
		fatalNote("Failed binding uniform " + name);
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
	if (!file.good()) fatalNote("Texture not found!");

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

	if (error != 0)
		fatalNote("error: " + std::to_string(error));

	texture::init(image, w, h);
}
texture::~texture() {
	glDeleteTextures(1, &(texture::tex));
}
void texture::use(GLuint textureID) {
	if(textureID != UINT32_MAX)
		glActiveTexture(textureID);
	glBindTexture(GL_TEXTURE_2D, texture::tex);
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          VAO            ]*******************************/

#include "data.inl"
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

sQuad::sQuad() : vao(sQuad::quadVertices, sizeof(sQuad::quadVertices), 6, true) {
	sQuad::quadVertices[2] = 0.0f;
	sQuad::quadVertices[3] = 1.0f;
	sQuad::quadVertices[6] = 0.0f;
	sQuad::quadVertices[7] = 0.0f;
	sQuad::quadVertices[10] = 1.0f;
	sQuad::quadVertices[11] = 0.0f;

	sQuad::quadVertices[14] = 0.0f;
	sQuad::quadVertices[15] = 1.0f;
	sQuad::quadVertices[18] = 1.0f;
	sQuad::quadVertices[19] = 0.0f;
	sQuad::quadVertices[22] = 1.0f;
	sQuad::quadVertices[23] = 1.0f;
}
void sQuad::draw(ARect pos, ARect post, bool transTexCoord) {
	//ARect pos(posi.left * 2 - 1, posi.top * 2 - 1, posi.right * 2 - 1, posi.bottom * 2 - 1);
	pos = pos * 2 - 1;
	sQuad::quadVertices[0] = pos.left;
	sQuad::quadVertices[1] = pos.bottom;
	sQuad::quadVertices[4] = pos.left;
	sQuad::quadVertices[5] = pos.top;
	sQuad::quadVertices[8] = pos.right;
	sQuad::quadVertices[9] = pos.top;

	sQuad::quadVertices[12] = pos.left;
	sQuad::quadVertices[13] = pos.bottom;
	sQuad::quadVertices[16] = pos.right;
	sQuad::quadVertices[17] = pos.top;
	sQuad::quadVertices[20] = pos.right;
	sQuad::quadVertices[21] = pos.bottom;

	if (transTexCoord) {
		sQuad::quadVertices[2] = post.left;
		sQuad::quadVertices[3] = post.bottom;
		sQuad::quadVertices[6] = post.left;
		sQuad::quadVertices[7] = post.top;
		sQuad::quadVertices[10] = post.right;
		sQuad::quadVertices[11] = post.top;

		sQuad::quadVertices[14] = post.left;
		sQuad::quadVertices[15] = post.bottom;
		sQuad::quadVertices[18] = post.right;
		sQuad::quadVertices[19] = post.top;
		sQuad::quadVertices[22] = post.right;
		sQuad::quadVertices[23] = post.bottom;
	}
	vao::update(sQuad::quadVertices, sizeof(sQuad::quadVertices), 6);
	vao::draw();
}
sQuad::~sQuad() {
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Sync buffer       ]*******************************/

syncBuffer::syncBuffer(AiSize iSize, bool useMipmap, GLuint quality, GLuint qualityM) :
	iSize(iSize), useMipmap(useMipmap), quality(quality) {
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &(syncBuffer::tex));
	glBindTexture(GL_TEXTURE_2D, syncBuffer::tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, syncBuffer::iSize.w, syncBuffer::iSize.h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmap ? GL_LINEAR_MIPMAP_LINEAR : quality);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, qualityM);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, syncBuffer::tex, 0);

	glGenRenderbuffers(1, &(syncBuffer::rbo));
	glBindRenderbuffer(GL_RENDERBUFFER, syncBuffer::rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, syncBuffer::iSize.w, syncBuffer::iSize.h); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, syncBuffer::rbo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		fatalNote("Framebuffer is not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glErrors("syncBuffer::construct");
}
void syncBuffer::scale(AiSize iSize, bool useMipmap) {
	syncBuffer::iSize = iSize;

	glBindFramebuffer(GL_FRAMEBUFFER, syncBuffer::framebuffer);

	glBindTexture(GL_TEXTURE_2D, syncBuffer::tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, syncBuffer::iSize.w, syncBuffer::iSize.h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	if (useMipmap != syncBuffer::useMipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useMipmap ? GL_LINEAR_MIPMAP_LINEAR : syncBuffer::quality);
		syncBuffer::useMipmap = useMipmap;
	}

	glBindRenderbuffer(GL_RENDERBUFFER, syncBuffer::rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, syncBuffer::iSize.w, syncBuffer::iSize.h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, syncBuffer::rbo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		fatalNote("Framebuffer is not complete!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
syncBuffer::~syncBuffer() {
	glDeleteTextures(1, &(syncBuffer::tex));
	glDeleteRenderbuffers(1, &(syncBuffer::rbo));
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
}
void syncBuffer::readFrom(GLuint textureID) {
	if (textureID != UINT32_MAX)
		glActiveTexture(textureID);
	glBindTexture(GL_TEXTURE_2D, syncBuffer::tex);
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
/*****************************[       Async buffer      ]*******************************/

asyncBuffer::asyncBuffer() {

}
asyncBuffer::~asyncBuffer() {

}