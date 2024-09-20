#pragma once

#include <memory>

#include <Windows.h>

#include "RenderPipeline.h"
#include "Scene.h"

class RenderSystem {
public:
    bool Initialize(HWND hwnd);

    void Shutdown();
    
    void Render(const Scene& scene);

private:
    std::unique_ptr<RenderPipeline> pipeline_;
};