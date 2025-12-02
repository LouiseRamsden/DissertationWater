#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <assert.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "3DMaths.h"
#include "ObjLoading.h"
#include "RendererDevice.h"
#include "RendererTarget.h"
#include "RendererShader.h"

static bool global_windowDidResize = false;

// Input
enum GameAction {
    GameActionMoveCamFwd,
    GameActionMoveCamBack,
    GameActionMoveCamLeft,
    GameActionMoveCamRight,
    GameActionTurnCamLeft,
    GameActionTurnCamRight,
    GameActionLookUp,
    GameActionLookDown,
    GameActionRaiseCam,
    GameActionLowerCam,
    GameActionCount
};
static bool global_keyIsDown[GameActionCount] = {};


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;
    switch(msg)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            bool isDown = (msg == WM_KEYDOWN);
            if(wparam == VK_ESCAPE)
                DestroyWindow(hwnd);
            else if(wparam == 'W')
                global_keyIsDown[GameActionMoveCamFwd] = isDown;
            else if(wparam == 'A')
                global_keyIsDown[GameActionMoveCamLeft] = isDown;
            else if(wparam == 'S')
                global_keyIsDown[GameActionMoveCamBack] = isDown;
            else if(wparam == 'D')
                global_keyIsDown[GameActionMoveCamRight] = isDown;
            else if(wparam == 'E')
                global_keyIsDown[GameActionRaiseCam] = isDown;
            else if(wparam == 'Q')
                global_keyIsDown[GameActionLowerCam] = isDown;
            else if(wparam == VK_UP)
                global_keyIsDown[GameActionLookUp] = isDown;
            else if(wparam == VK_LEFT)
                global_keyIsDown[GameActionTurnCamLeft] = isDown;
            else if(wparam == VK_DOWN)
                global_keyIsDown[GameActionLookDown] = isDown;
            else if(wparam == VK_RIGHT)
                global_keyIsDown[GameActionTurnCamRight] = isDown;
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_SIZE:
        {
            global_windowDidResize = true;
            break;
        }
        default:
            result = DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    // Open a window
    HWND hwnd;
    {
        WNDCLASSEXW winClass = {};
        winClass.cbSize = sizeof(WNDCLASSEXW);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
        winClass.lpfnWndProc = &WndProc;
        winClass.hInstance = hInstance;
        winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
        winClass.hCursor = LoadCursorW(0, IDC_ARROW);
        winClass.lpszClassName = L"MyWindowClass";
        winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

        if(!RegisterClassExW(&winClass)) {
            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }

        RECT initialRect = { 0, 0, 1024, 768 };
        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
        LONG initialWidth = initialRect.right - initialRect.left;
        LONG initialHeight = initialRect.bottom - initialRect.top;

        hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                                winClass.lpszClassName,
                                L"10. Blinn-Phong Lighting",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                initialWidth, 
                                initialHeight,
                                0, 0, hInstance, 0);

        if(!hwnd) {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
    }

    RendererDevice renderDevice = RendererDevice(hwnd); //Renderer Device (device, device context, swapchain)

    RendererTarget renderTarget = RendererTarget(renderDevice); //Renderer Target (Framebuffer and depth stencil)


    D3D11_INPUT_ELEMENT_DESC lightinputdesc[] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    const wchar_t* lightpaths[] =
    {
        L"Lights.hlsl",
        nullptr,
        nullptr,
        nullptr,
        L"Lights.hlsl"
    };
    RendererShader lightShader = RendererShader(
        lightpaths,
        &renderDevice,
        lightinputdesc,
        ARRAYSIZE(lightinputdesc)
    );

    D3D11_INPUT_ELEMENT_DESC bfinputdesc[] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORM", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    const wchar_t* bfpaths[] =
    {
        L"BlinnPhong.hlsl",
        nullptr,
        nullptr,
        nullptr,
        L"BlinnPhong.hlsl"
    };
    RendererShader blinnPhongShader = RendererShader(
        bfpaths,
        &renderDevice,
        bfinputdesc,
        ARRAYSIZE(bfinputdesc)
    );



    // Create Vertex and Index Buffer
    ID3D11Buffer* cubeVertexBuffer;
    ID3D11Buffer* cubeIndexBuffer;
    UINT cubeNumIndices;
    UINT cubeStride;
    UINT cubeOffset;
    {
        LoadedObj obj = loadObj("cube.obj");
        cubeStride = sizeof(VertexData);
        cubeOffset = 0;
        cubeNumIndices = obj.numIndices;

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = obj.numVertices * sizeof(VertexData);
        vertexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { obj.vertexBuffer };

        HRESULT hResult = renderDevice.GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &cubeVertexBuffer);
        assert(SUCCEEDED(hResult));

        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = obj.numIndices * sizeof(uint16_t);
        indexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { obj.indexBuffer };

        hResult = renderDevice.GetDevice()->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &cubeIndexBuffer);
        assert(SUCCEEDED(hResult));
        freeLoadedObj(obj);
    }

    // Create Sampler State
    ID3D11SamplerState* samplerState;
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

        renderDevice.GetDevice()->CreateSamplerState(&samplerDesc, &samplerState);
    }
    
    // Load Image
    int texWidth, texHeight, texNumChannels;
    int texForceNumChannels = 4;
    unsigned char* testTextureBytes = stbi_load("test.png", &texWidth, &texHeight,
                                                &texNumChannels, texForceNumChannels);
    assert(testTextureBytes);
    int texBytesPerRow = 4 * texWidth;

    // Create Texture
    ID3D11ShaderResourceView* textureView;
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width              = texWidth;
        textureDesc.Height             = texHeight;
        textureDesc.MipLevels          = 1;
        textureDesc.ArraySize          = 1;
        textureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        textureDesc.SampleDesc.Count   = 1;
        textureDesc.Usage              = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
        textureSubresourceData.pSysMem = testTextureBytes;
        textureSubresourceData.SysMemPitch = texBytesPerRow;

        ID3D11Texture2D* texture;
        renderDevice.GetDevice()->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);

        renderDevice.GetDevice()->CreateShaderResourceView(texture, nullptr, &textureView);
        texture->Release();
    }

    free(testTextureBytes);

    // Create Constant Buffer for our light vertex shader
    struct LightVSConstants
    {
        float4x4 modelViewProj;
        float4 color;
    };

    ID3D11Buffer* lightVSConstantBuffer;
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth      = sizeof(LightVSConstants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = renderDevice.GetDevice()->CreateBuffer(&constantBufferDesc, nullptr, &lightVSConstantBuffer);
        assert(SUCCEEDED(hResult));
    }

    // Create Constant Buffer for our Blinn-Phong vertex shader
    struct BlinnPhongVSConstants
    {
        float4x4 modelViewProj;
        float4x4 modelView;
        float3x3 normalMatrix;
    };

    ID3D11Buffer* blinnPhongVSConstantBuffer;
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth      = sizeof(BlinnPhongVSConstants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = renderDevice.GetDevice()->CreateBuffer(&constantBufferDesc, nullptr, &blinnPhongVSConstantBuffer);
        assert(SUCCEEDED(hResult));
    }

    struct DirectionalLight
    {
        float4 dirEye; //NOTE: Direction towards the light
        float4 color;
    };

    struct PointLight
    {
        float4 posEye;
        float4 color;
    };

    // Create Constant Buffer for our Blinn-Phong pixel shader
    struct BlinnPhongPSConstants
    {
        DirectionalLight dirLight;
        PointLight pointLights[2];
    };

    ID3D11Buffer* blinnPhongPSConstantBuffer;
    {
        D3D11_BUFFER_DESC constantBufferDesc = {};
        // ByteWidth must be a multiple of 16, per the docs
        constantBufferDesc.ByteWidth      = sizeof(BlinnPhongPSConstants) + 0xf & 0xfffffff0;
        constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
        constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hResult = renderDevice.GetDevice()->CreateBuffer(&constantBufferDesc, nullptr, &blinnPhongPSConstantBuffer);
        assert(SUCCEEDED(hResult));
    }

    ID3D11RasterizerState* rasterizerState;
    {
        D3D11_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = TRUE;

        renderDevice.GetDevice()->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    }

    ID3D11DepthStencilState* depthStencilState;
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable    = TRUE;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;

        renderDevice.GetDevice()->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    }

    // Camera
    float3 cameraPos = {0, 0, 2};
    float3 cameraFwd = {0, 0, -1};
    float cameraPitch = 0.f;
    float cameraYaw = 0.f;

    float4x4 perspectiveMat = {};
    global_windowDidResize = true; // To force initial perspectiveMat calculation

    // Timing
    LONGLONG startPerfCount = 0;
    LONGLONG perfCounterFrequency = 0;
    {
        LARGE_INTEGER perfCount;
        QueryPerformanceCounter(&perfCount);
        startPerfCount = perfCount.QuadPart;
        LARGE_INTEGER perfFreq;
        QueryPerformanceFrequency(&perfFreq);
        perfCounterFrequency = perfFreq.QuadPart;
    }
    double currentTimeInSeconds = 0.0;






    // Main Loop
    bool isRunning = true;
    while(isRunning)
    {
        float dt;
        {
            double previousTimeInSeconds = currentTimeInSeconds;
            LARGE_INTEGER perfCount;
            QueryPerformanceCounter(&perfCount);

            currentTimeInSeconds = (double)(perfCount.QuadPart - startPerfCount) / (double)perfCounterFrequency;
            dt = (float)(currentTimeInSeconds - previousTimeInSeconds);
            if(dt > (1.f / 60.f))
                dt = (1.f / 60.f);
        }

        MSG msg = {};
        while(PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                isRunning = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        // Get window dimensions
        int windowWidth, windowHeight;
        float windowAspectRatio;
        {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            windowWidth = clientRect.right - clientRect.left;
            windowHeight = clientRect.bottom - clientRect.top;
            windowAspectRatio = (float)windowWidth / (float)windowHeight;
        }

        if(global_windowDidResize)
        {
            renderDevice.GetDeviceContext()->OMSetRenderTargets(0, 0, 0);
            renderTarget.GetRenderTarget()->Release();
            renderTarget.GetDepthBuffer()->Release();

            HRESULT res = renderDevice.GetSwapChain()->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
            assert(SUCCEEDED(res));
            renderTarget = RendererTarget(renderDevice);
            perspectiveMat = makePerspectiveMat(windowAspectRatio, degreesToRadians(84), 0.1f, 1000.f);

            global_windowDidResize = false;
        }

        // Update camera
        {
            float3 camFwdXZ = normalise(float3{cameraFwd.x, 0, cameraFwd.z});
            float3 cameraRightXZ = cross(camFwdXZ, {0, 1, 0});

            const float CAM_MOVE_SPEED = 5.f; // in metres per second
            const float CAM_MOVE_AMOUNT = CAM_MOVE_SPEED * dt;
            if(global_keyIsDown[GameActionMoveCamFwd])
                cameraPos += camFwdXZ * CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionMoveCamBack])
                cameraPos -= camFwdXZ * CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionMoveCamLeft])
                cameraPos -= cameraRightXZ * CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionMoveCamRight])
                cameraPos += cameraRightXZ * CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionRaiseCam])
                cameraPos.y += CAM_MOVE_AMOUNT;
            if(global_keyIsDown[GameActionLowerCam])
                cameraPos.y -= CAM_MOVE_AMOUNT;
            
            const float CAM_TURN_SPEED = M_PI; // in radians per second
            const float CAM_TURN_AMOUNT = CAM_TURN_SPEED * dt;
            if(global_keyIsDown[GameActionTurnCamLeft])
                cameraYaw += CAM_TURN_AMOUNT;
            if(global_keyIsDown[GameActionTurnCamRight])
                cameraYaw -= CAM_TURN_AMOUNT;
            if(global_keyIsDown[GameActionLookUp])
                cameraPitch += CAM_TURN_AMOUNT;
            if(global_keyIsDown[GameActionLookDown])
                cameraPitch -= CAM_TURN_AMOUNT;

            // Wrap yaw to avoid floating-point errors if we turn too far
            while(cameraYaw >= 2*M_PI) 
                cameraYaw -= 2*M_PI;
            while(cameraYaw <= -2*M_PI) 
                cameraYaw += 2*M_PI;

            // Clamp pitch to stop camera flipping upside down
            if(cameraPitch > degreesToRadians(85)) 
                cameraPitch = degreesToRadians(85);
            if(cameraPitch < -degreesToRadians(85)) 
                cameraPitch = -degreesToRadians(85);
        }

        // Calculate view matrix from camera data
        // 
        // float4x4 viewMat = inverse(rotateXMat(cameraPitch) * rotateYMat(cameraYaw) * translationMat(cameraPos));
        // NOTE: We can simplify this calculation to avoid inverse()!
        // Applying the rule inverse(A*B) = inverse(B) * inverse(A) gives:
        // float4x4 viewMat = inverse(translationMat(cameraPos)) * inverse(rotateYMat(cameraYaw)) * inverse(rotateXMat(cameraPitch));
        // The inverse of a rotation/translation is a negated rotation/translation:
        float4x4 viewMat = translationMat(-cameraPos) * rotateYMat(-cameraYaw) * rotateXMat(-cameraPitch);
        float4x4 inverseViewMat = rotateXMat(cameraPitch) * rotateYMat(cameraYaw) * translationMat(cameraPos);
        // Update the forward vector we use for camera movement:
        cameraFwd = {-viewMat.m[2][0], -viewMat.m[2][1], -viewMat.m[2][2]};

        // Calculate matrices for cubes
        const int NUM_CUBES = 3;
        float4x4 cubeModelViewMats[NUM_CUBES];
        float3x3 cubeNormalMats[NUM_CUBES];
        {
            float3 cubePositions[NUM_CUBES] = {
                {0.f, 0.f, 0.f},
                {-3.f, 0.f, -1.5f},
                {4.5f, 0.2f, -3.f}
            };

            float modelXRotation = 0.2f * (float)(M_PI * currentTimeInSeconds);
            float modelYRotation = 0.1f * (float)(M_PI * currentTimeInSeconds);
            for(int i=0; i<NUM_CUBES; ++i)
            {
                modelXRotation += 0.6f*i; // Add an offset so cubes have different phases
                modelYRotation += 0.6f*i;
                float4x4 modelMat = rotateXMat(modelXRotation) * rotateYMat(modelYRotation) * translationMat(cubePositions[i]);
                float4x4 inverseModelMat = translationMat(-cubePositions[i]) * rotateYMat(-modelYRotation) * rotateXMat(-modelXRotation);
                cubeModelViewMats[i] = modelMat * viewMat;
                float4x4 inverseModelViewMat = inverseViewMat * inverseModelMat;
                cubeNormalMats[i] = float4x4ToFloat3x3(transpose(inverseModelViewMat));
            }
        }

        // Move the point lights
        const int NUM_LIGHTS = 2;
        float4 lightColor[NUM_LIGHTS] = {
            {0.1f, 0.4f, 0.9f, 1.f},
            {0.9f, 0.1f, 0.6f, 1.f}
        };
        float4x4 lightModelViewMats[NUM_LIGHTS];
        float4 pointLightPosEye[NUM_LIGHTS];
        {
            float4 initialPointLightPositions[NUM_LIGHTS] = {
                {1, 0.5f, 0, 1},
                {-1, 0.7f, -1.2f, 1}
            };

            float lightRotation = -0.3f * (float)(M_PI * currentTimeInSeconds);
            for(int i=0; i<NUM_LIGHTS; ++i)
            {
                lightRotation += 0.5f*i; // Add an offset so lights have different phases
                                        
                lightModelViewMats[i] = scaleMat(0.2f) * translationMat(initialPointLightPositions[i].xyz) * rotateYMat(lightRotation) * viewMat;
                pointLightPosEye[i] = lightModelViewMats[i].cols[3];
            }
        }

        FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
        renderDevice.GetDeviceContext()->ClearRenderTargetView(renderTarget.GetRenderTarget(), backgroundColor);
        
        renderDevice.GetDeviceContext()->ClearDepthStencilView(renderTarget.GetDepthBuffer(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)windowWidth, (FLOAT)windowHeight, 0.0f, 1.0f };
        renderDevice.GetDeviceContext()->RSSetViewports(1, &viewport);

        renderDevice.GetDeviceContext()->RSSetState(rasterizerState);
        renderDevice.GetDeviceContext()->OMSetDepthStencilState(depthStencilState, 0);
        
        ID3D11RenderTargetView* temp = renderTarget.GetRenderTarget();
        renderDevice.GetDeviceContext()->OMSetRenderTargets(1, &temp, renderTarget.GetDepthBuffer());

        renderDevice.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        renderDevice.GetDeviceContext()->IASetVertexBuffers(0, 1, &cubeVertexBuffer, &cubeStride, &cubeOffset);
        renderDevice.GetDeviceContext()->IASetIndexBuffer(cubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

        // Draw lights
        {
            renderDevice.GetDeviceContext()->IASetInputLayout(lightShader.GetInputLayout());
            renderDevice.GetDeviceContext()->VSSetShader(lightShader.GetVertexShader(), nullptr, 0);
            renderDevice.GetDeviceContext()->PSSetShader(lightShader.GetPixelShader(), nullptr, 0);
            renderDevice.GetDeviceContext()->VSSetConstantBuffers(0, 1, &lightVSConstantBuffer);

            for(int i=0; i<NUM_LIGHTS; ++i){
                // Update vertex shader constant buffer
                D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                renderDevice.GetDeviceContext()->Map(lightVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
                LightVSConstants* constants = (LightVSConstants*)(mappedSubresource.pData);
                constants->modelViewProj = lightModelViewMats[i] * perspectiveMat;
                constants->color = lightColor[i];
                renderDevice.GetDeviceContext()->Unmap(lightVSConstantBuffer, 0);

                renderDevice.GetDeviceContext()->DrawIndexed(cubeNumIndices, 0, 0);
            }
        }
        // Draw cubes
        {
            renderDevice.GetDeviceContext()->IASetInputLayout(blinnPhongShader.GetInputLayout());
            renderDevice.GetDeviceContext()->VSSetShader(blinnPhongShader.GetVertexShader(), nullptr, 0);
            renderDevice.GetDeviceContext()->PSSetShader(blinnPhongShader.GetPixelShader(), nullptr, 0);

            renderDevice.GetDeviceContext()->PSSetShaderResources(0, 1, &textureView);
            renderDevice.GetDeviceContext()->PSSetSamplers(0, 1, &samplerState);

            renderDevice.GetDeviceContext()->VSSetConstantBuffers(0, 1, &blinnPhongVSConstantBuffer);
            renderDevice.GetDeviceContext()->PSSetConstantBuffers(0, 1, &blinnPhongPSConstantBuffer);

            // Update pixel shader constant buffer
            {
                D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                renderDevice.GetDeviceContext()->Map(blinnPhongPSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
                BlinnPhongPSConstants* constants = (BlinnPhongPSConstants*)(mappedSubresource.pData);
                constants->dirLight.dirEye = normalise(float4{1.f, 1.f, 1.f, 0.f});
                constants->dirLight.color = {0.7f, 0.8f, 0.2f, 1.f};
                for(int i=0; i<NUM_LIGHTS; ++i){
                    constants->pointLights[i].posEye = pointLightPosEye[i];
                    constants->pointLights[i].color = lightColor[i];
                }
                renderDevice.GetDeviceContext()->Unmap(blinnPhongPSConstantBuffer, 0);
            }

            for(int i=0; i<NUM_CUBES; ++i)
            {
                // Update vertex shader constant buffer
                D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                renderDevice.GetDeviceContext()->Map(blinnPhongVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
                BlinnPhongVSConstants* constants = (BlinnPhongVSConstants*)(mappedSubresource.pData);
                constants->modelViewProj = cubeModelViewMats[i] * perspectiveMat;
                constants->modelView = cubeModelViewMats[i];
                constants->normalMatrix = cubeNormalMats[i];
                renderDevice.GetDeviceContext()->Unmap(blinnPhongVSConstantBuffer, 0);

                renderDevice.GetDeviceContext()->DrawIndexed(cubeNumIndices, 0, 0);
            }
        }
    
        renderDevice.GetSwapChain()->Present(1, 0);
    }

    return 0;
}
