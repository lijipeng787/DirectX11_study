#pragma once

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class LightShaderClass;
class LightClass;
class RenderTextureClass;
class DebugWindowClass;
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
private:
	bool RenderToTexture();

	bool RenderScene();
private:
	Camera *camera_ = nullptr;

	ModelClass *model_ = nullptr;

	LightShaderClass* light_shader_ = nullptr;

	LightClass* light_ = nullptr;

	RenderTextureClass* render_texture_ = nullptr;

	DebugWindowClass* debug_window_ = nullptr;

	TextureShaderClass* texture_shader_ = nullptr;
};
