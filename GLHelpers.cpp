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
	//if(textureID != UINT32_MAX)
	//	glActiveTexture(textureID);
	glBindTexture(GL_TEXTURE_2D, texture::tex);
}


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          VAO            ]*******************************/

#include "data.inl"
vao::vao(const float* data, GLsizeiptr dataSize, GLsizei faces) : faces(faces) {
	glErrors("vao::before");

	glGenVertexArrays(1, &(vao::vaodata));
	glGenBuffers(1, &(vao::vbodata));
	glBindVertexArray((vao::vaodata));
	glBindBuffer(GL_ARRAY_BUFFER, (vao::vbodata));
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glErrors("vao::construct");
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