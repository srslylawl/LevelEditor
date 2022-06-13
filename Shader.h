#pragma once

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>


namespace Rendering {
	class Shader {
		void CompileShader(const char* vertexCode, const char* fragmentCode) {
			unsigned int vertex, fragment;
			int success;
			char infoLog[512];

			// vertex Shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vertexCode, NULL);
			glCompileShader(vertex);
			glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				std::cout << "ERROR: SHADER '" << name << " 'Vertex COMPILATION_FAILED\n" << infoLog << std::endl;
			}

			// similiar for Fragment Shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fragmentCode, NULL);
			glCompileShader(fragment);

			// print compile errors if any
			glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				std::cout << "ERROR: SHADER '" << name << " 'Fragment COMPILATION_FAILED\n" << infoLog << std::endl;
			}

			// shader Program
			ID = glCreateProgram();
			glAttachShader(ID, vertex);
			glAttachShader(ID, fragment);
			glLinkProgram(ID);
			// print linking errors, if any
			glGetProgramiv(ID, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(ID, 512, NULL, infoLog);
				std::cout << "ERROR: SHADER '" << name << " 'PROGRAM_LINKING_FAILED\n" << infoLog << std::endl;
			}

			// delete the shaders as they're linked into our program now and no longer necessary
			glDeleteShader(vertex);
			glDeleteShader(fragment);
		}

		std::string GetShaderCodeFromPath(const char* path) {
			std::string result;
			std::ifstream stream;
			//ensure stream can throw
			stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try {
				stream.open(path);
				std::stringstream stringStream;
				stringStream << stream.rdbuf();
				stream.close();
				result = stringStream.str();
			}
			catch (std::ifstream::failure e) {
				std::cout << "ERROR: SHADER '" << name << " 'FILE_NOT_SUCCESSFULLY_READ" << std::endl;
			}
			return result;
		}

	public:
		// the program ID
		unsigned int ID;
		std::string name;

		~Shader() {
			Delete();
			std::cout << "Shader " << name << " destructor called and deleted" << std::endl;
		}
		Shader(const char* name) {
			this->name = name;
			auto shaderPath = std::filesystem::current_path().append("Shaders/").append(name);
			auto vertPath = shaderPath.string().append(".vert");
			auto fragPath = shaderPath.string().append(".frag");
			CompileShader(GetShaderCodeFromPath(vertPath.c_str()).c_str(), GetShaderCodeFromPath(fragPath.c_str()).c_str());
		}
		Shader(const char* vertexPath, const char* fragmentPath, const char* name) {
			this->name = name;
			CompileShader(GetShaderCodeFromPath(vertexPath).c_str(), GetShaderCodeFromPath(fragmentPath).c_str());
		}
		// use/activate the shader
		void Use() {
			glUseProgram(ID);
		}
		// utility uniform functions
		void setBool(const std::string& name, bool value) const {
			glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
		}
		void setInt(const std::string& name, int value) const {
			glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
		}
		void setFloat(const std::string& name, float value) const {
			glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
		}
		void setMat4(const std::string& name, glm::mat4 value) const {
			glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value_ptr(value));
		}
		void setVec(const std::string& name, glm::vec3 value) {
			glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
		}
		void setVec(const std::string& name, glm::vec2 value) {
			glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
		}

		void Delete() {
			glDeleteProgram(ID);
		}
	};
}
