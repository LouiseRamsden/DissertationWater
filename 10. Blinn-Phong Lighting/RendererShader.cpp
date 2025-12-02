#include "RendererShader.h"


void RendererShader::InitVertexShader(const wchar_t* path)
{
	{
		ID3DBlob* compileErrors;
		HRESULT hResult = D3DCompileFromFile(path, nullptr, nullptr, "vs_main", "vs_5_0", m_shaderCompileFlags, 0, &m_vsBlob, &compileErrors);
		if (FAILED(hResult))
		{
			const char* errorString = NULL;
			if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
				errorString = "Could not compile shader; file not found";
			else if (compileErrors) {
				errorString = (const char*)compileErrors->GetBufferPointer();
			}
			MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
			throw std::exception("Shader Compile Error");
		}

		hResult = m_pDevice->GetDevice()->CreateVertexShader(m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), nullptr, &m_pVertex);
		if (!SUCCEEDED(hResult)) throw std::exception("Unable to create vertex shader");
	}
}
void RendererShader::InitHullShader(const wchar_t* path)
{
	throw std::exception("NOT IMPLEMENTED");
}
void RendererShader::InitDomainShader(const wchar_t* path)
{
	throw std::exception("NOT IMPLEMENTED");
}
void RendererShader::InitGeometryShader(const wchar_t* path)
{
	throw std::exception("NOT IMPLEMENTED");
}
void RendererShader::InitPixelShader(const wchar_t* path)
{
	ID3DBlob* blob;
	{
		ID3DBlob* compileErrors;
		HRESULT hResult = D3DCompileFromFile(path, nullptr, nullptr, "ps_main", "ps_5_0", m_shaderCompileFlags, 0, &blob, &compileErrors);
		if (FAILED(hResult))
		{
			const char* errorString = NULL;
			if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
				errorString = "Could not compile shader; file not found";
			else if (compileErrors) {
				errorString = (const char*)compileErrors->GetBufferPointer();
			}
			MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
			throw std::exception("Shader Compile Error");
		}

		hResult = m_pDevice->GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_pPixel);
		if (!SUCCEEDED(hResult)) throw std::exception("Unable to create pixel shader");
	}
	blob->Release();
}