#ifndef HEADER_GRAPHICSCLASS_H
#define HEADER_GRAPHICSCLASS_H

#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "textureshaderclass.h"
#include "inputclass.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class Graphics
{
public:
	Graphics();

	explicit Graphics(const Graphics&);
	
	~Graphics();

public:
	bool Initialize(int, int, HWND);
	
	void Shutdown();
	
	bool Frame();

private:
	bool Render();

private:
	D3D12Device* d3d12_device_ = nullptr;
	
	std::shared_ptr<Camera> camera_ = nullptr;

	std::shared_ptr<Bitmap> bitmap_ = nullptr;

	std::shared_ptr<TextureShader> texture_shader_ = nullptr;

	std::shared_ptr<Input> input_ = nullptr;
};

#endif //!HEADER_GRAPHICSCLASS_H