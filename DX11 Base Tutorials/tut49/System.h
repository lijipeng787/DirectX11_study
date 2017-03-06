#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

#include "../CommonFramework/SystemBase.h"

class GraphicsClass;
class PositionClass;

class System :public SystemBase {
public:
	System();

	System(const System& rhs) = delete;

	System& operator=(const System& rhs) = delete;

	virtual ~System();
public:
	virtual bool Initialize()override;

	virtual void Shutdown()override;

	virtual bool Frame()override;
private:
	bool HandleInput(float frame_time);
private:
	GraphicsClass *m_Graphics;

	PositionClass *m_Position;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System* ApplicationInstance = nullptr;


#endif