#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../Common/GraphicsBase.h"

constexpr bool FULL_SCREEN = false;
constexpr bool VSYNC_ENABLED = true;
constexpr float SCREEN_DEPTH = 1000.0f;
constexpr float SCREEN_NEAR = 0.1f;

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
	Camera* m_Camera;

	ModelClass* m_Model;

	ColorShaderClass* m_ColorShader;
};

#endif