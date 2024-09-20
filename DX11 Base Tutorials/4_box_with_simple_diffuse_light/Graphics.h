#pragma once

#include "../CommonFramework/GraphicsBase.h"
#include "Scene.h"
#include "RenderSystem.h"

#include <memory>

class GraphicsClass : public GraphicsBase {
public:
	GraphicsClass() = default;

	GraphicsClass(const GraphicsClass& rhs) = delete;
	
	GraphicsClass& operator=(const GraphicsClass& rhs) = delete;
	
	virtual ~GraphicsClass() = default;

public:
	virtual bool Initialize(int screenWidth, int screenHeight, HWND hwnd) override;
	
	virtual void Shutdown() override;
	
	virtual void Frame(float deltatime) override;
	
	virtual void Render() override;

private:
	std::unique_ptr<RenderSystem> render_system_;
	
	std::unique_ptr<Scene> scene_;
};
