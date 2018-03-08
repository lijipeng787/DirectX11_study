


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
class TransparentDepthShaderClass;
class TreeClass;

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

	void SetDeltaTime(float delta_time) {
		delta_time_ = delta_time;
	}
private:
	bool RenderSceneToTexture();

	void UpdateLighting();
private:
	float pos_x_ = 0.0f, pos_y_ = 0.0f, pos_z_ = 0.0f;

	float rot_x_ = 0.0f, rot_y_ = 0.0f, rot_z_ = 0.0f;

	float delta_time_ = 0.0f;

	

	Camera *camera_ = nullptr;

	LightClass* light_;

	ModelClass* m_GroundModel;

	TreeClass* m_Tree;

	RenderTextureClass* m_RenderTexture;

	DepthShaderClass* m_DepthShader;

	TransparentDepthShaderClass* m_TransparentDepthShader;

	ShadowShaderClass* m_ShadowShader;
};

#endif