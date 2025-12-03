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



#include "RendererDevice.h"


class Texture
{
    ID3D11SamplerState* m_pSamplerState = {nullptr};
    ID3D11ShaderResourceView* m_pTextureView = {nullptr};
    ID3D11Texture2D* m_pTexture = {nullptr};
public:
    Texture(const char* path, RendererDevice* device);

    ID3D11SamplerState* GetSamplerState() 
    {
        return m_pSamplerState;
    }
    ID3D11SamplerState** GetSamplerStateAddr()
    {
        return &m_pSamplerState;
    }
    ID3D11ShaderResourceView* GetTextureView()
    {
        return m_pTextureView;
    }
    ID3D11ShaderResourceView** GetTextureViewAddr()
    {
        return &m_pTextureView;
    }
    ID3D11Texture2D* GetTexture() 
    {
        return m_pTexture;
    }
    ID3D11Texture2D** GetTextureAddr()
    {
        return &m_pTexture;
    }
};

