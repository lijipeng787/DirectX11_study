#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class TextClass;
class LightShaderClass;
class LightClass;
class ModelListClass;
class FrustumClass;

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
public:
	void SetRotation(float rotation);
private:
	DirectX11Device *m_D3D = nullptr;

	Camera *m_Camera = nullptr;

	ModelClass *m_Model = nullptr;

	TextClass* m_Text = nullptr;

	LightShaderClass* m_LightShader = nullptr;

	LightClass* m_Light = nullptr;

	ModelListClass* m_ModelList = nullptr;

	FrustumClass* m_Frustum = nullptr;

	float rotationY = 0.0f;
};

#endif
