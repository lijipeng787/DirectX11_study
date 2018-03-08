


#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class FireShaderClass;

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
	

	Camera *camera_ = nullptr;

	ModelClass* model_;

	FireShaderClass* m_FireShader;
};

#endif