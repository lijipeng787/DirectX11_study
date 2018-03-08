


#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class TextureShaderClass;

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
	void SetPosition(float x,float y,float z);
private:
	

	Camera *camera_ = nullptr;

	TextureShaderClass* m_TextureShader = nullptr;

	ModelClass *m_FloorModel = nullptr, *m_BillboardModel = nullptr;

	float x_ = 0.0f, y_ = 0.0f, z_ = 0.0f;
};

#endif