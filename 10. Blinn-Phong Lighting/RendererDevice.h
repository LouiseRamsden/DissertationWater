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

class RendererDevice
{
	ID3D11Device1* m_pDevice = nullptr;
	ID3D11DeviceContext1* m_pDeviceContext = nullptr;
    IDXGISwapChain1* m_pSwapChain = nullptr;
public:
	RendererDevice(HWND hwnd) 
	{
        ID3D11Device* baseDevice;
        ID3D11DeviceContext* baseDeviceContext;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
            0, creationFlags,
            featureLevels, ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION, &baseDevice,
            0, &baseDeviceContext);
        if (FAILED(hResult)) {
            MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
            throw std::exception("D3D11CreateDevice() failed");
        }

        // Get 1.1 interface of D3D11 Device and Context
        hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&m_pDevice);
        if (!SUCCEEDED(hResult)) throw std::exception("QueryInterface() failed");
        baseDevice->Release();

        hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&m_pDeviceContext);
        if(!SUCCEEDED(hResult)) throw std::exception("QueryInterface() failed");
        baseDeviceContext->Release();

#ifdef _DEBUG
        // Set up debug layer to break on D3D11 errors
        ID3D11Debug* d3dDebug = nullptr;
        m_pDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
        if (d3dDebug)
        {
            ID3D11InfoQueue* d3dInfoQueue = nullptr;
            if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
            {
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
                d3dInfoQueue->Release();
            }
            d3dDebug->Release();
        }
#endif

        // Get DXGI Factory (needed to create Swap Chain)
        IDXGIFactory2* dxgiFactory;
        {
            IDXGIDevice1* dxgiDevice;
            HRESULT hResult = m_pDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
            if(!SUCCEEDED(hResult)) throw std::exception("Unable to query interface");

            IDXGIAdapter* dxgiAdapter;
            hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
            if(!SUCCEEDED(hResult)) throw std::exception("Unable to get adapter");
            dxgiDevice->Release();

            DXGI_ADAPTER_DESC adapterDesc;
            dxgiAdapter->GetDesc(&adapterDesc);

            OutputDebugStringA("Graphics Device: ");
            OutputDebugStringW(adapterDesc.Description);

            hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
            if(!SUCCEEDED(hResult)) throw std::exception("Unable to get parent");
            dxgiAdapter->Release();
        }

        DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
        d3d11SwapChainDesc.Width = 0; // use window width
        d3d11SwapChainDesc.Height = 0; // use window height
        d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        d3d11SwapChainDesc.SampleDesc.Count = 1;
        d3d11SwapChainDesc.SampleDesc.Quality = 0;
        d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        d3d11SwapChainDesc.BufferCount = 2;
        d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        d3d11SwapChainDesc.Flags = 0;

        hResult = dxgiFactory->CreateSwapChainForHwnd(m_pDevice, hwnd, &d3d11SwapChainDesc, 0, 0, &m_pSwapChain);
        if(!SUCCEEDED(hResult)) throw std::exception("unable to create swapchain from HWND");

        dxgiFactory->Release();

	}
    ID3D11Device1* GetDevice() 
    {
        return m_pDevice;
    }
    ID3D11DeviceContext1* GetContext()
    {
        return m_pDeviceContext;
    }
    IDXGISwapChain1* GetSwapChain() 
    {
        return m_pSwapChain;
    }
};

