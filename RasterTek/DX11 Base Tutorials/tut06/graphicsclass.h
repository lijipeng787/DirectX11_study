#ifndef HEADER_GRAPHICSCLASS_H
#define HEADER_GRAPHICSCLASS_H

#include <memory>
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "inputclass.h"
#include "lightshaderclass.h"
#include "lightclass.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class Graphics{
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

	std::shared_ptr<Light> light_ = nullptr;
	
	std::shared_ptr<Camera> camera_ = nullptr;

	std::shared_ptr<Input> input_ = nullptr;

	std::shared_ptr<LightShader> light_shader_ = nullptr;

	std::shared_ptr<Model> model_ = nullptr;
};

#endif //!HEADER_GRAPHICSCLASS_H