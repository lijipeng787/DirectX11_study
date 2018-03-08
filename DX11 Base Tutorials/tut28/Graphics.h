


#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class TextureShaderClass;
class RenderTextureClass;
class SimpleMoveableSurface;
class FadeShaderClass;

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
	bool RenderToTexture(float rotation);

	bool RenderScene();

	bool RenderNormalScene(float rotation);

	bool RenderFadingScene();
private:
	float frame_time_ = 0.0f;

	float m_fadeInTime = 0.0f, m_accumulatedTime = 0.0f, m_fadePercentage = 0.0f;

	bool m_fadeDone = false;

	

	Camera *camera_ = nullptr;

	ModelClass *model_ = nullptr;

	TextureShaderClass* m_TextureShader = nullptr;

	RenderTextureClass* m_RenderTexture = nullptr;

	SimpleMoveableSurface* m_Bitmap = nullptr;

	FadeShaderClass* m_FadeShader = nullptr;
};

#endif