#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class SpecMapShaderClass;
class LightClass;

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
	DirectX11Device *directx_device_ = nullptr;

	Camera *camera_ = nullptr;

	ModelClass *model_ = nullptr;

	SpecMapShaderClass* m_SpecMapShader = nullptr;

	LightClass* m_Light = nullptr;
};

#endif