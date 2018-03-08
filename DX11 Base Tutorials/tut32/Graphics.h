#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class RenderTextureClass;
class TextureShaderClass;
class GlassShaderClass;

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
private:
	bool RenderToTexture(float rotation);
private:
	float rotation_ = 0.0f;

	DirectX11Device *directx_device_ = nullptr;

	Camera *camera_ = nullptr;

	ModelClass* model_;

	ModelClass* m_WindowModel;

	RenderTextureClass* m_RenderTexture;

	TextureShaderClass* m_TextureShader;

	GlassShaderClass* m_GlassShader;
};

#endif