


#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class TextClass;
class LightShaderClass;
class LightClass;
class ModelListClass;
class FrustumClass;

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
	void SetRotation(float rotation) {
		rotationY = rotation;
	}
private:
	

	Camera *camera_ = nullptr;

	ModelClass *model_ = nullptr;

	TextClass* m_Text = nullptr;

	LightShaderClass* light_shader_ = nullptr;

	LightClass* light_ = nullptr;

	ModelListClass* m_ModelList = nullptr;

	FrustumClass* m_Frustum = nullptr;

	float rotationY = 0.0f;
};

#endif
