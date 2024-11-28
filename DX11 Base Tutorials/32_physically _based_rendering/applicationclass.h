////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_NEAR = 0.3f;
const float SCREEN_DEPTH = 1000.0f;

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "cameraclass.h"
#include "d3dclass.h"
#include "inputclass.h"
#include "lightclass.h"
#include "modelclass.h"
#include "pbrshaderclass.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ApplicationClass
////////////////////////////////////////////////////////////////////////////////
class ApplicationClass {
public:
  ApplicationClass();
  ApplicationClass(const ApplicationClass &);
  ~ApplicationClass();

  bool Initialize(int, int, HWND);
  void Shutdown();
  bool Frame(InputClass *);

private:
  bool Render(float);

private:
  D3DClass *m_Direct3D;
  CameraClass *m_Camera;
  ModelClass *m_Model;
  LightClass *m_Light;
  PbrShaderClass *m_PbrShader;
};

#endif