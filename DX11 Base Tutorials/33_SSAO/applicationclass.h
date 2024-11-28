////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = false;
const float SCREEN_NEAR = 0.3f;
const float SCREEN_DEPTH = 1000.0f;

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "cameraclass.h"
#include "d3dclass.h"
#include "deferredbuffersclass.h"
#include "gbuffershaderclass.h"
#include "inputclass.h"
#include "lightclass.h"
#include "lightshaderclass.h"
#include "modelclass.h"
#include "orthowindowclass.h"
#include "rendertextureclass.h"
#include "ssaoblurshaderclass.h"
#include "ssaoshaderclass.h"

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
  bool RenderGBuffer();
  bool RenderSsao();
  bool BlurSsaoTexture();
  bool Render();

private:
  D3DClass *m_Direct3D;
  CameraClass *m_Camera;
  LightClass *m_Light;
  ModelClass *m_SphereModel, *m_GroundModel;
  DeferredBuffersClass *m_DeferredBuffers;
  GBufferShaderClass *m_GBufferShader;
  RenderTextureClass *m_SsaoRenderTexture;
  OrthoWindowClass *m_FullScreenWindow;
  SsaoShaderClass *m_SsaoShader;
  TextureClass *m_RandomTexture;
  RenderTextureClass *m_BlurSsaoRenderTexture;
  SsaoBlurShaderClass *m_SsaoBlurShader;
  LightShaderClass *m_LightShader;
  int m_screenWidth, m_screenHeight;
};

#endif