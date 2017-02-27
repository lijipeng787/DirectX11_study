#ifndef _GRAPHICSCLASS_BASE_H_
#define _GRAPHICSCLASS_BASE_H_

#include <windows.h>

class GraphicsBase {
public:
	GraphicsBase();

	GraphicsBase(const GraphicsBase& rhs) = delete;

	GraphicsBase& operator=(const GraphicsBase& rhs) = delete;

	virtual ~GraphicsBase();
public:
	virtual bool Initialize(int, int, HWND) = 0;

	virtual void Shutdown() = 0;

	virtual bool Frame() = 0;

	virtual bool Render() = 0;
};

#endif