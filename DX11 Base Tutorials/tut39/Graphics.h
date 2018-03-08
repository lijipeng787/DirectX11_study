#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class ParticleShaderClass;
class ParticleSystemClass;

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
	void SetFrameTime(float frame_time) {
		frame_time_ = frame_time;
	}
private:
	float frame_time_ = 0.0f;

	DirectX11Device *directx_device_ = nullptr;

	Camera *camera_ = nullptr;

	ModelClass* model_ = nullptr;

	ParticleShaderClass* m_ParticleShader;

	ParticleSystemClass* m_ParticleSystem;
};

#endif