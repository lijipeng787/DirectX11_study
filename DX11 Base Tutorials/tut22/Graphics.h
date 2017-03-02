#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

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
	DirectX11Device *m_D3D = nullptr;

	Camera *m_Camera = nullptr;

	ModelClass *m_Model = nullptr;

	LightShaderClass* m_LightShader = nullptr;

	LightClass* m_Light = nullptr;

	RenderTextureClass* m_RenderTexture = nullptr;

	DebugWindowClass* m_DebugWindow = nullptr;

	TextureShaderClass* m_TextureShader = nullptr;
};

#endif