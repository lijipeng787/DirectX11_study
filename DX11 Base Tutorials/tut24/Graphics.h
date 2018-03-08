


#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class ClipPlaneShaderClass;

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

	ModelClass *model_ = nullptr;

	ClipPlaneShaderClass* m_ClipPlaneShader = nullptr;
};

#endif