#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class TextureClass {
public:
    explicit TextureClass() = default;

    TextureClass(const TextureClass&) = delete;

    TextureClass& operator=(const TextureClass&) = delete;

    TextureClass(TextureClass&&) noexcept = default;

    TextureClass& operator=(TextureClass&&) noexcept = default;

    virtual ~TextureClass() = default;

public:
    bool Initialize(const WCHAR* filename);

    void Shutdown();

    ID3D11ShaderResourceView* GetTexture() const;

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
};