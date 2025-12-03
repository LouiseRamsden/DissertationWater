#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const char* path, RendererDevice* device)
{
    if (path == nullptr) 
    {

    }
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.BorderColor[0] = 1.0f;
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    device->GetDevice()->CreateSamplerState(&samplerDesc, &m_pSamplerState);


    // Load Image
    int texWidth, texHeight, texNumChannels;
    int texForceNumChannels = 4;
    unsigned char* texBytes = stbi_load(path, &texWidth, &texHeight,
        &texNumChannels, texForceNumChannels);
    if (texBytes == nullptr) throw std::exception("unable to load texture");
    int texBytesPerRow = 4 * texWidth;

    // Create Texture
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = texWidth;
    textureDesc.Height = texHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
    textureSubresourceData.pSysMem = texBytes;
    textureSubresourceData.SysMemPitch = texBytesPerRow;


    device->GetDevice()->CreateTexture2D(&textureDesc, &textureSubresourceData, &m_pTexture);

    device->GetDevice()->CreateShaderResourceView(m_pTexture, nullptr, &m_pTextureView);
    m_pTexture->Release();

    free(texBytes);
}