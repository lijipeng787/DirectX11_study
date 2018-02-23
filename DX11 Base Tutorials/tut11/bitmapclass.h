#ifndef _BITMAPCLASS_H_
#define _BITMAPCLASS_H_

#include <d3d11.h>

class SimpleTexture;

class SimpleMoveableBitmap{
public:
	SimpleMoveableBitmap() {}
	
	SimpleMoveableBitmap(const SimpleMoveableBitmap& rhs) = delete;

	SimpleMoveableBitmap& operator=(const SimpleMoveableBitmap& rhs) = delete;
	
	~SimpleMoveableBitmap() {}
public:
	bool Initialize(int screenWidth, int screenHeight, int bitmapHeight, int bitmapWidth);

	bool LoadTextureFromFile(WCHAR *fileName);

	bool InitializeVertexAndIndexBuffers();

	void Release();
	
	bool Render(int, int);

	int GetIndexCount();
	
	ID3D11ShaderResourceView* GetTexture();
private:
	void ReleaseBuffers();
	
	bool UpdateBuffers(int positionX, int positionY);
	
	void SubmitBuffers();
	
	void ReleaseTexture();
private:
	ID3D11Buffer *vertex_buffer_ = nullptr, *index_buffer_ = nullptr;

	int vertex_count_ = 0, index_count_ = 0;
	
	SimpleTexture* texture_ = nullptr;
	
	int screen_width_ = 0, screen_height_ = 0;
	
	int bitmap_width_ = 0, bitmap_height_ = 0;
	
	int previous_posititon_x_ = 0, previous_posititon_y_ = 0;
};

#endif