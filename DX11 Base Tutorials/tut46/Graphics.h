


#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class SimpleMoveableSurface;
class TextureShaderClass;
class RenderTextureClass;
class OrthoWindowClass;
class HorizontalBlurShaderClass;
class VerticalBlurShaderClass;
class GlowMapShaderClass;
class GlowShaderClass;

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
	bool RenderGlowMapToTexture();

	bool DownSampleTexture();

	bool RenderHorizontalBlurToTexture();

	bool RenderVerticalBlurToTexture();

	bool UpSampleTexture();

	bool RenderUIElementsToTexture();

	bool RenderGlowScene();
private:
	

	Camera *camera_ = nullptr;

	TextureShaderClass* m_TextureShader;

	SimpleMoveableSurface* m_Bitmap;

	RenderTextureClass
		*m_RenderTexture = nullptr,
		*m_DownSampleTexure = nullptr,
		*m_HorizontalBlurTexture = nullptr,
		*m_VerticalBlurTexture = nullptr,
		*m_UpSampleTexure = nullptr;

	OrthoWindowClass *m_SmallWindow, *m_FullScreenWindow;

	HorizontalBlurShaderClass* m_HorizontalBlurShader;

	VerticalBlurShaderClass* m_VerticalBlurShader;

	GlowMapShaderClass* m_GlowMapShader;

	GlowShaderClass* m_GlowShader;
};

#endif