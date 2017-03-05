#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

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

	void SetRotation(float rotation) {
		rotation_ = rotation;
	}
private:
	bool RenderSceneToTexture(float rotation);

	bool DownSampleTexture();

	bool RenderHorizontalBlurToTexture();

	bool RenderVerticalBlurToTexture();

	bool UpSampleTexture();

	bool Render2DTextureScene();
private:
	float rotation_ = 0.0f;

	DirectX11Device *m_D3D = nullptr;

	Camera *m_Camera = nullptr;

	ModelClass *m_Model = nullptr;

	TextureShaderClass* m_TextureShader = nullptr;

	HorizontalBlurShaderClass* m_HorizontalBlurShader = nullptr;

	VerticalBlurShaderClass* m_VerticalBlurShader = nullptr;

	RenderTextureClass 
		*m_RenderTexture = nullptr, 
		*m_DownSampleTexure = nullptr,
		*m_HorizontalBlurTexture = nullptr, 
		*m_VerticalBlurTexture = nullptr,
		*m_UpSampleTexure = nullptr;

	OrthoWindowClass *m_SmallWindow = nullptr, *m_FullScreenWindow = nullptr;
};

#endif