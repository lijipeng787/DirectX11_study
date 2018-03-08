#pragma once

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class ColorShaderClass;

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
	DirectX11Device *directx_device_;

	Camera *camera_;

	ModelClass *model_;

	ColorShaderClass *m_ColorShader;
};
