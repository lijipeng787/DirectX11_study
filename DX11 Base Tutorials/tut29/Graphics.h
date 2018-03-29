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

	float water_height_ = 0.0f, water_translation_ = 0.0f;

	Camera *camera_ = nullptr;

	ModelClass
		*ground_model_ = nullptr,
		*wall_model_ = nullptr,
		*bath_model_ = nullptr,
		*water_model_ = nullptr;

	LightClass* light_ = nullptr;

	RenderTextureClass
		*refraction_texture_ = nullptr,
		*reflection_texture_ = nullptr;

	LightShaderClass* light_shader_ = nullptr;

	RefractionShaderClass* refraction_shader_ = nullptr;

	WaterShaderClass* water_shader_ = nullptr;
};
