#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class LightClass;
class RenderTextureClass;
class RefractionShaderClass;
class LightShaderClass;
class WaterShaderClass;

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

	void SetFameTime(float frame_time);
private:
	bool RenderRefractionToTexture();

	bool RenderReflectionToTexture();

	bool RenderScene();
private:
	float frame_time_ = 0.0f;

	float m_fadeInTime = 0.0f, m_accumulatedTime = 0.0f, m_fadePercentage = 0.0f;

	bool m_fadeDone = false;

	DirectX11Device *m_D3D = nullptr;

	Camera *m_Camera = nullptr;

	ModelClass *m_GroundModel, *m_WallModel, *m_BathModel, *m_WaterModel;

	LightClass* m_Light;

	RenderTextureClass *m_RefractionTexture, *m_ReflectionTexture;

	LightShaderClass* m_LightShader;

	RefractionShaderClass* m_RefractionShader;

	WaterShaderClass* m_WaterShader;

	float m_waterHeight, m_waterTranslation;
};

#endif