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
	DirectX11Device *m_D3D;

	Camera *m_Camera;

	ModelClass *m_Model;

	ColorShaderClass *m_ColorShader;
};
