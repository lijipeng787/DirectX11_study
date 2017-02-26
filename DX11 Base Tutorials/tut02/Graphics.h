#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include <windows.h>

constexpr bool FULL_SCREEN = false;

constexpr bool VSYNC_ENABLED = true;

constexpr float SCREEN_DEPTH = 1000.0f;

constexpr float SCREEN_NEAR = 0.1f;

class Graphics {
public:
	Graphics();

	Graphics(const Graphics& rhs) = delete;

	Graphics& operator=(const Graphics& rhs) = delete;

	~Graphics();
public:
	bool Initialize(int, int, HWND);

	void Shutdown();

	bool Frame();
private:
	bool Render();
private:
};

#endif