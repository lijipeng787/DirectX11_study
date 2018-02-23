#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include "../CommonFramework/GraphicsBase.h"
#include <DirectXMath.h>

class DirectX11Device;
class Camera;
class ModelClass;
class TextClass;
class LightShaderClass;
class TextureShaderClass;
class LightClass;
class SimpleMoveableBitmap;

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
	void TestIntersection(int, int);

	void SetMousePosition(int x, int y) {
		mouse_x_ = x;
		mouse_y_ = y;
	}
private:
	bool RaySphereIntersect(const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&, float);
private:
	DirectX11Device *m_D3D = nullptr;

	Camera *m_Camera = nullptr;

	ModelClass* m_Model;

	TextureShaderClass* m_TextureShader;

	LightShaderClass* m_LightShader;

	LightClass* m_Light;

	TextClass* m_Text;

	SimpleMoveableBitmap* m_Bitmap;

	int m_screenWidth = 0, m_screenHeight = 0;

	int mouse_x_, mouse_y_;
};

#endif