#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <exception>
#include <cstdint>

#include "RendererDevice.h"

typedef unsigned char shadermask_t;

#define SHADER_VERTEX 0
#define SHADER_HULL 1
#define SHADER_DOMAIN 2
#define SHADER_GEOMETRY 3
#define SHADER_PIXEL 4

class RendererShader
{
	ID3D11VertexShader* m_pVertex = nullptr;
	ID3D11HullShader* m_pHull = nullptr;
	ID3D11DomainShader* m_pDomain = nullptr;
	ID3D11GeometryShader* m_pGeometry = nullptr;
	ID3D11PixelShader* m_pPixel = nullptr;
	UINT m_shaderCompileFlags = 0;
	RendererDevice* m_pDevice = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;

	ID3DBlob* m_vsBlob = nullptr; // for input layout this has to be available to everything



	void InitVertexShader(const wchar_t* path);

	void InitHullShader(const wchar_t* path);

	void InitDomainShader(const wchar_t* path);

	void InitGeometryShader(const wchar_t* path);

	void InitPixelShader(const wchar_t* path);
	
public:
	RendererShader(const wchar_t** paths, RendererDevice* device, const D3D11_INPUT_ELEMENT_DESC* inputElementDesc, int inputElementLength) 
	{
		m_pDevice = device;
#if defined(_DEBUG)
		m_shaderCompileFlags |= D3DCOMPILE_DEBUG;
#endif
		for (int i = 0; i < 5; i++) 
		{
			if (paths[i] == nullptr) continue;
			switch (i) 
			{
			case SHADER_VERTEX:
				InitVertexShader(paths[i]);
				break;
			case SHADER_HULL:
				InitHullShader(paths[i]);
				break;
			case SHADER_DOMAIN:
				InitDomainShader(paths[i]);
				break;
			case SHADER_GEOMETRY:
				InitGeometryShader(paths[i]);
				break;
			case SHADER_PIXEL:
				InitPixelShader(paths[i]);
				break;
			}
			
		}
		HRESULT hResult = device->GetDevice()->CreateInputLayout(inputElementDesc, inputElementLength, m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), &m_pInputLayout);
		if(!SUCCEEDED(hResult)) throw std::exception("Unable to create input layout");
		m_vsBlob->Release();
	}
	ID3D11VertexShader* GetVertexShader() { return m_pVertex; }
	ID3D11HullShader* GetHullShader() { return m_pHull; }
	ID3D11DomainShader* GetDomainShader() { return m_pDomain; }
	ID3D11GeometryShader* GetGeometryShader() { return m_pGeometry; }
	ID3D11PixelShader* GetPixelShader() { return m_pPixel; }
	ID3D11InputLayout* GetInputLayout() { return m_pInputLayout; }
};

