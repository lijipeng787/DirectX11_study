#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../CommonFramework/GraphicsBase.h"

const int SHADOWMAP_WIDTH = 1024;
const int SHADOWMAP_HEIGHT = 1024;
const float SHADOWMAP_DEPTH = 50.0f;
const float SHADOWMAP_NEAR = 1.0f;

class DirectX11Device;
class Camera;
class ModelClass;
class DepthShaderClass;
class ShadowShaderClass;
class LightClass;
class RenderTextureClass;

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
	void SetPosition(float x, float y, float z) {
		pos_x_ = x;
		pos_y_ = y;
		pos_z_ = z;
	}

	void SetRotation(float x, float y, float z) {
		rot_x_ = x;
		rot_y_ = y;
		rot_z_ = z;
	}

	void SetFrameTime(float frame_time) {
		frame_time_ = frame_time;
	}
private:
	bool RenderSceneToTexture();

	bool RenderBlackAndWhiteShadows();

	bool DownSampleTexture();

	bool RenderHorizontalBlurToTexture();

	bool RenderVerticalBlurToTexture();

	bool UpSampleTexture();

	bool RenderSceneToTexture2();
private:
	float pos_x_ = 0.0f, pos_y_ = 0.0f, pos_z_ = 0.0f;

	float rot_x_ = 0.0f, rot_y_ = 0.0f, rot_z_ = 0.0f;

	float frame_time_ = 0.0f;

	DirectX11Device *m_D3D = nullptr;

	Camera *m_Camera = nullptr;

	ModelClass *m_CubeModel, *m_GroundModel, *m_SphereModel;

	LightClass* m_Light;

	RenderTextureClass* m_RenderTexture;

	DepthShaderClass* m_DepthShader;

	ShadowShaderClass* m_ShadowShader;
};

#endif