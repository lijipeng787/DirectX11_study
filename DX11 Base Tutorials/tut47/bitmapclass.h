

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;

#include "textureclass.h"

class SimpleMoveableSurface {
private:
  struct VertexType {
    XMFLOAT3 position;
    XMFLOAT2 texture;
  };

public:
  SimpleMoveableSurface();
  SimpleMoveableSurface(const SimpleMoveableSurface &);
  ~SimpleMoveableSurface();

  bool Initialize(int, int, WCHAR *, int, int);
  void Shutdown();
  bool Render(int, int);

  int GetIndexCount();
  ID3D11ShaderResourceView *GetTexture();

private:
  bool InitializeBuffers(ID3D11Device *);
  void ShutdownBuffers();
  bool UpdateBuffers(int, int);
  void RenderBuffers(ID3D11DeviceContext *);

  bool LoadTexture(WCHAR *);
  void ReleaseTexture();

private:
  ID3D11Buffer *vertex_buffer_, *index_buffer_;
  int vertex_count_, index_count_;
  TextureClass *texture_;
  int screen_width_, screen_height_;
  int bitmap_width_, bitmap_height_;
  int previous_pos_x_, previous_pos_y_;
};

#endif