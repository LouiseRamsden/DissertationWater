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

class RendererTarget
{
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
public:
	RendererTarget(RendererDevice& device) 
	{
        ID3D11Texture2D* d3d11FrameBuffer;
        HRESULT hResult = device.GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
        if(!SUCCEEDED(hResult)) throw std::exception("Unable to Retrieve buffer");

        hResult = device.GetDevice()->CreateRenderTargetView(d3d11FrameBuffer, 0, &m_pRenderTargetView);
        if(!SUCCEEDED(hResult)) throw std::exception("Unable to create Render Target View");

        D3D11_TEXTURE2D_DESC depthBufferDesc;
        d3d11FrameBuffer->GetDesc(&depthBufferDesc);

        d3d11FrameBuffer->Release();

        depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        ID3D11Texture2D* depthBuffer;
        device.GetDevice()->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

        device.GetDevice()->CreateDepthStencilView(depthBuffer, nullptr, &m_pDepthStencilView);

        depthBuffer->Release();

	}
    ID3D11RenderTargetView* GetRenderTarget() 
    {
        return m_pRenderTargetView;
    }
    ID3D11RenderTargetView** GetRenderTargetAddr() 
    {
        return &m_pRenderTargetView;
    }
    ID3D11DepthStencilView* GetDepthBuffer() 
    {
        return m_pDepthStencilView;
    }
};

