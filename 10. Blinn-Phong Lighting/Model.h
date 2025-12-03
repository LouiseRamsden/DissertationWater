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
#include "ObjLoading.h"
#include "RendererDevice.h"
class Model
{
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	UINT m_numIndices;
	UINT m_stride;
	UINT m_offset;
public:
	Model(const char* path, RendererDevice* device)
	{
        LoadedObj obj = loadObj(path);
        m_stride = sizeof(VertexData);
        m_offset = 0;
        m_numIndices = obj.numIndices;

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.ByteWidth = obj.numVertices * sizeof(VertexData);
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexSubresourceData = { obj.vertexBuffer };

        HRESULT hResult = device->GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &m_pVertexBuffer);
        if(!SUCCEEDED(hResult)) throw std::exception("Unable to Create Vertex Buffer");

        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.ByteWidth = obj.numIndices * sizeof(uint16_t);
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubresourceData = { obj.indexBuffer };

        hResult = device->GetDevice()->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &m_pIndexBuffer);
        if (!SUCCEEDED(hResult)) throw std::exception("Unable to Create Index Buffer");
        freeLoadedObj(obj);
	}
    ID3D11Buffer* GetVertexBuffer() 
    {
        return m_pVertexBuffer;
    }
    ID3D11Buffer** GetVertexBufferAddr() 
    {
        return &m_pVertexBuffer;
    }
    ID3D11Buffer* GetIndexBuffer() 
    {
        return m_pIndexBuffer;
    }
    ID3D11Buffer** GetIndexBufferAddr()
    {
        return &m_pIndexBuffer;
    }
    UINT GetNumIndices() 
    {
        return m_numIndices;
    }
    UINT* const GetNumIndicesAddr() 
    {
        return &m_numIndices;
    }
    UINT GetStride() 
    {
        return m_stride;
    }
    UINT* const GetStrideAddr()
    {
        return &m_stride;
    }
    UINT GetOffset() 
    {
        return m_offset;
    }
    UINT* const GetOffsetAddr()
    {
        return &m_offset;
    }
};

