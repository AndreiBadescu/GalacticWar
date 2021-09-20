#pragma once
#include "RendererCommon.hpp"


namespace gal {

	class Shader {
	public:
		Shader();
		Shader(const char* pVertexFilePath, const char* pFragmentFilePath);
		Shader(Shader&& other) noexcept;
		Shader& operator=(Shader&& other) noexcept;
		

		void Bind() const;
		void SetUniformBindigPoint(const char* pUniformName, u32 uBindingPoint);
		inline handle_t GetProgramHandle() const { return m_hShader; }
	private:
		handle_t m_hShader;
	};

}