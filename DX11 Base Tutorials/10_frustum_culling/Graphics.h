#pragma once

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
	GraphicsClass() {}

	GraphicsClass(const GraphicsClass& rhs) = delete;

	GraphicsClass& operator=(const GraphicsClass& rhs) = delete;

	virtual ~GraphicsClass() {}
public:
	virtual bool Initialize(int, int, HWND)override;

	virtual void Shutdown()override;

	virtual bool Frame()override;

	virtual bool Render()override;
public:
	inline void SetRotation(float rotation_) {
		rotation_y_ = rotation_;
	}
private:
	Camera *camera_ = nullptr;

	ModelClass *model_ = nullptr;

	TextClass* text_ = nullptr;

	LightShaderClass* light_shader_ = nullptr;

	LightClass* light_ = nullptr;

	ModelListClass* model_list_ = nullptr;

	FrustumClass* frustum_ = nullptr;

	float rotation_y_ = 0.0f;
};
