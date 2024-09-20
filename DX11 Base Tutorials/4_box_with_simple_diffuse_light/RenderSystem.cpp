#include "RenderSystem.h"

bool RenderSystem::Initialize(HWND hwnd) {
    pipeline_ = std::make_unique<RenderPipeline>();
    if (!pipeline_->Initialize(hwnd)) {
        MessageBox(hwnd, L"Could not initialize the render pipeline.", L"Error", MB_OK);
        return false;
    }

    return true;
}

void RenderSystem::Shutdown() {
    if (pipeline_) {
        pipeline_->Shutdown();
        pipeline_.reset();
    }
}

void RenderSystem::Render(const Scene& scene) {
    if (pipeline_) {
        pipeline_->Render(scene);
    }
}
