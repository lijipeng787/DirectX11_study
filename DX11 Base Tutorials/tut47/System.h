


#include "../CommonFramework/SystemBase.h"

class GraphicsClass;

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
	bool HandleInput();
private:
	bool m_beginCheck = false;

	int m_screenWidth = 0, m_screenHeight = 0;

	GraphicsClass *m_Graphics;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System* ApplicationInstance = nullptr;


#endif