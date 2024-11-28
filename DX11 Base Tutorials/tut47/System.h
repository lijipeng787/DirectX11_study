

#include "../CommonFramework/SystemBase.h"

class GraphicsClass;

class System : public SystemBase {
public:
  System();

  System(const System &rhs) = delete;

  System &operator=(const System &rhs) = delete;

  virtual ~System();

public:
  virtual bool Initialize() override;

  virtual void Shutdown() override;

  virtual bool Frame() override;

private:
  bool HandleInput();

private:
  bool m_beginCheck = false;

  int screen_width_ = 0, screen_height_ = 0;

  GraphicsClass *graphics_;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System *ApplicationInstance = nullptr;

#endif