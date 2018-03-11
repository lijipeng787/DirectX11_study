


#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class RenderTextureClass;
class TextureShaderClass;
class GlassShaderClass;

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
	bool RenderToTexture(float rotation_);
private:
	float rotation_ = 0.0f;

	

	Camera *camera_ = nullptr;

	ModelClass* model_;

	ModelClass* m_WindowModel;

	RenderTextureClass* render_texture_;

	TextureShaderClass* texture_shader_;

	GlassShaderClass* m_GlassShader;
};

#endif