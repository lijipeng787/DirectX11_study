#pragma once

#include <Windows.h>
#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class OrthoWindowClass;
class VerticalBlurShaderClass;
class RenderTextureClass;
class HorizontalBlurShaderClass;
class TextureShaderClass;

class GraphicsClass :public GraphicsBase {
public:
	GraphicsClass();

	GraphicsClass(const GraphicsClass& rhs) = delete;

	GraphicsClass& operator=(const GraphicsClass& rhs) = delete;

	virtual ~GraphicsClass();
public:
	virtual bool Initialize(int, int, HWND)override;

	virtual void Shutdown()override;

	virtual bool Frame()override;

	virtual bool Render()override;

	inline void SetRotation(float rotation_) {
		rotation_ = rotation_;
	}
private:
	bool RenderSceneToTexture(float rotation_);

	bool DownSampleTexture();

	bool RenderHorizontalBlurToTexture();

	bool RenderVerticalBlurToTexture();

	bool UpSampleTexture();

	bool Render2DTextureScene();
private:
	float rotation_ = 0.0f;

	Camera *camera_ = nullptr;

	ModelClass *model_ = nullptr;

	TextureShaderClass* texture_shader_ = nullptr;

	HorizontalBlurShaderClass* m_HorizontalBlurShader = nullptr;

	VerticalBlurShaderClass* m_VerticalBlurShader = nullptr;

	RenderTextureClass 
		*render_texture_ = nullptr, 
		*m_DownSampleTexure = nullptr,
		*m_HorizontalBlurTexture = nullptr, 
		*m_VerticalBlurTexture = nullptr,
		*m_UpSampleTexure = nullptr;

	OrthoWindowClass *m_SmallWindow = nullptr, *m_FullScreenWindow = nullptr;
};