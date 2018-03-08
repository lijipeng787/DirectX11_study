


#include "../CommonFramework/GraphicsBase.h"

class DirectX11Device;
class Camera;
class ModelClass;
class ShaderManagerClass;
class LightClass;
class BumpModelClass;

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
	float rotation_ = 0.0f;

	

	Camera *camera_ = nullptr;

	ShaderManagerClass* m_ShaderManager;
	
	LightClass* light_;
	
	ModelClass* m_Model1;
	
	ModelClass* m_Model2;
	
	BumpModelClass* m_Model3;
};

#endif