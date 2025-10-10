////////////////////////////////////////////////////////////////////////////////
// Filename: terrainclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TERRAINCLASS_H_
#define _TERRAINCLASS_H_

//////////////
// INCLUDES //
//////////////
#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: TerrainClass
////////////////////////////////////////////////////////////////////////////////
class TerrainClass {
private:
  struct VertexType {
    XMFLOAT3 position;
    XMFLOAT4 color;
  };

public:
  TerrainClass();
  TerrainClass(const TerrainClass &);
  ~TerrainClass();

  bool Initialize(ID3D11Device *);
  void Shutdown();
  void Render(ID3D11DeviceContext *);

  int GetIndexCount();

private:
  bool InitializeBuffers(ID3D11Device *);
  void ShutdownBuffers();
  void RenderBuffers(ID3D11DeviceContext *);

private:
  int m_terrainWidth, m_terrainHeight;
  int m_vertexCount, m_indexCount;
  ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
};

#endif