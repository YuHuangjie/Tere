#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <iostream>
#include <vector>
#include "Common.hpp"
#if GL_WIN || GL_OSX
#include <GL/glew.h>
#elif GL_ES3_IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif GL_ES3_ANDROID
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif
using namespace std;

class Shader
{
public:
	unsigned int ID;

	Shader() : ID(0) 
	{}
	
	Shader(const string &vertex_code, const string &frag_code)
	{
		GLint Result = GL_FALSE;
		int InfoLogLength;
		// Create the shaders
		GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		// Compile Vertex Shader
		//printf("Compiling shader : %s\n", vertex_file_path);
		char const * VertexSourcePointer = vertex_code.c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
		glCompileShader(VertexShaderID);

		// Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!Result) {
			std::vector<char> VertexShaderErrorMessage(InfoLogLength);
			glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
			if (VertexShaderErrorMessage.size() != 0)
				throw runtime_error(string("VERTEX ERROR: ") + string(VertexShaderErrorMessage.data()));
			else
				throw runtime_error(string("VERTEX ERROR: "));
		}

		// Compile Fragment Shader
		//printf("Compiling shader : %s\n", fragment_file_path);
		char const * FragmentSourcePointer = frag_code.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
		glCompileShader(FragmentShaderID);

		// Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!Result) {
			std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
			glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
			if (FragmentShaderErrorMessage.size() != 0)
				throw runtime_error(string("FRAGMENT ERROR: ") + string(FragmentShaderErrorMessage.data()));
			else
				throw runtime_error(string("FRAGMENT ERROR: "));
		}

		// Link the program
		//fprintf(stdout, "Linking programn\n");
		GLuint ProgramID = glCreateProgram();
		glAttachShader(ProgramID, VertexShaderID);
		glAttachShader(ProgramID, FragmentShaderID);
		glLinkProgram(ProgramID);

		// Check the program
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!Result) {
			std::vector<char> ProgramErrorMessage(InfoLogLength);
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			if (ProgramErrorMessage.size() != 0)
				throw runtime_error(string("LINK ERROR: ") + string(ProgramErrorMessage.data()));
			else
				throw runtime_error(string("LINK ERROR: "));
		}

		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);

		ID = ProgramID;
	}
};



#endif
