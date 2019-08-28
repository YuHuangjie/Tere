#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <iostream>
#include <vector>
#include "GLHeader.h"
#include "Error.h"

using namespace std;

struct Shader
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
		char const * vsPointer = vertex_code.c_str();
		glShaderSource(VertexShaderID, 1, &vsPointer, NULL);
		glCompileShader(VertexShaderID);

		// Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!Result) {
			std::vector<char> msg(InfoLogLength);

			glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &msg[0]);
			if (msg.size() != 0) {
				// When you see errors originated from here, first make sure opengl
				// is properly initialized
				THROW_ON_ERROR("VERTEX ERROR: %s, CODE: %.7s", &msg[0], vsPointer);
			}
			else {
				THROW_ON_ERROR("VERTEX ERROR: Empty, CODE: %.7s", vsPointer);
			}
		}

		// Compile Fragment Shader
		char const * fsPointer = frag_code.c_str();
		glShaderSource(FragmentShaderID, 1, &fsPointer, NULL);
		glCompileShader(FragmentShaderID);

		// Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!Result) {
			std::vector<char> msg(InfoLogLength);

			glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &msg[0]);
			if (msg.size() != 0) {
				THROW_ON_ERROR("FRAGMENT ERROR: %s, CODE: %.7s", &msg[0], fsPointer);
			}
			else {
				THROW_ON_ERROR("FRAGMENT ERROR: Empty, CODE: %.7s", fsPointer);
			}
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
			std::vector<char> msg(InfoLogLength);
			
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &msg[0]);
			if (msg.size() != 0) {
				THROW_ON_ERROR("LINK ERROR: %s, CODE: %.7s", &msg[0], fsPointer);
			}
			else {
				THROW_ON_ERROR("LINK ERROR: EMPTY, CODE: %.7s", fsPointer);
			}
		}

		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);

		ID = ProgramID;
	}
};



#endif
