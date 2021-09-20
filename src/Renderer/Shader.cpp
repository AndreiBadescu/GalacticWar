#include "Renderer/Shader.hpp"
#include "Utility/Allocation.hpp"
#include <fstream>
#include <iostream>
#include <string.h>

namespace gal {
	Shader::Shader()
		:m_hShader(nullhandle)
	{
	}

	Shader::Shader(const char* pVertexFilePath, const char* pFragmentFilePath) {

		std::ifstream fVertexFile(pVertexFilePath, std::ios::ate | std::ios::in | std::ios::binary);
		std::ifstream fFragmentFile(pFragmentFilePath, std::ios::ate | std::ios::in | std::ios::binary);
		
		u64 uVertexFileSize = fVertexFile.tellg();
		u64 uFragmentFileSize = fFragmentFile.tellg();

		ArenaAllocator arena(1 + (uVertexFileSize + uFragmentFileSize) / ArenaAllocator::GetPageSize());

		fVertexFile.seekg(std::ios::beg);
		fFragmentFile.seekg(std::ios::beg);

		char* pVertexSource = arena.alloc<char>(uVertexFileSize);
		char* pFragmentSource = arena.alloc<char>(uFragmentFileSize);

		fVertexFile.read(pVertexSource, uVertexFileSize);
		fFragmentFile.read(pFragmentSource, uFragmentFileSize);

		handle_t hVertexShader = glCreateShader(GL_VERTEX_SHADER);
		handle_t hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(hVertexShader, 1, &pVertexSource, (GLint*)&uVertexFileSize);
		glCompileShader(hVertexShader);

		i32 compileStatus;

		glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &compileStatus);

		if (!compileStatus) {
			constexpr char prefix[] = "Vertex Compile Error : ";
			constexpr u32 size = sizeof(prefix);
			char infoLog[512];
			strcpy(infoLog, prefix);
			glGetShaderInfoLog(hVertexShader, 512 - (size - 1), nullptr, infoLog + (size - 1));
			throw std::runtime_error(infoLog);
		}

		glShaderSource(hFragmentShader, 1, &pFragmentSource, (GLint*)&uFragmentFileSize);
		glCompileShader(hFragmentShader);

		glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &compileStatus);

		if (!compileStatus) {
			constexpr char prefix[] = "Framgent Compile Error : ";
			constexpr u32 size = sizeof(prefix);
			char infoLog[512];
			strcpy(infoLog, prefix);
			glGetShaderInfoLog(hFragmentShader, 512 - (size - 1), nullptr, infoLog + (size - 1));
			throw std::runtime_error(infoLog);
		}

		m_hShader = glCreateProgram();

		glAttachShader(m_hShader, hVertexShader);
		glAttachShader(m_hShader, hFragmentShader);

		glLinkProgram(m_hShader);

		glGetProgramiv(m_hShader, GL_LINK_STATUS, &compileStatus);

		if (!compileStatus) {
			constexpr char prefix[] = "Shader Link Error : ";
			constexpr u32 size = sizeof(prefix);
			char infoLog[512];
			strcpy(infoLog, prefix);
			glGetProgramInfoLog(hFragmentShader, 512 - (size - 1), nullptr, infoLog + (size - 1));
			throw std::runtime_error(infoLog);
		}

		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);

	}
	Shader::Shader(Shader&& other) noexcept
		:m_hShader(other.m_hShader)
	{
		other.m_hShader = nullhandle;
	}
	Shader& Shader::operator=(Shader&& other) noexcept
	{
		m_hShader = other.m_hShader;
		other.m_hShader = nullhandle;
		return *this;
	}
	void Shader::Bind() const {
		glUseProgram(m_hShader);
	}
	void Shader::SetUniformBindigPoint(const char* pUniformName, u32 uBindingPoint)
	{
		u32 uBlockIndex = glGetUniformBlockIndex(m_hShader, pUniformName);
		glUniformBlockBinding(m_hShader, uBlockIndex, uBindingPoint);
	}
}