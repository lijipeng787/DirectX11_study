#pragma once

#include "../CommonFramework/SystemBase.h"
#include "../CommonFramework/Timer.h"

class GraphicsClass;

class System :public SystemBase {
public:
	System() = default;

	System(const System& rhs) = delete;

	System& operator=(const System& rhs) = delete;

	virtual ~System() = default;

public:
	virtual bool Initialize()override;

	virtual void Shutdown()override;

	virtual bool Frame()override;

private:
	GraphicsClass *graphics_;

	Timer timer_;
	float fixed_time_step_ = 1.0f / 60.0f;  // 60 FPS
	float accumulator_ = 0.0f;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System* ApplicationInstance = nullptr;
