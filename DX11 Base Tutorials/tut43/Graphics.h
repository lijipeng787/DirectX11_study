#pragma once

#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class ViewPointClass;
class TextureClass;
class ProjectionShaderClass;
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
	bool RenderToTexture();

	bool RenderScene();
private:
	Camera *camera_ = nullptr;

	ModelClass *m_GroundModel, *m_CubeModel;

	LightClass* light_;

	ProjectionShaderClass* m_ProjectionShader;

	TextureClass* m_ProjectionTexture;

	ViewPointClass* m_ViewPoint;
};
