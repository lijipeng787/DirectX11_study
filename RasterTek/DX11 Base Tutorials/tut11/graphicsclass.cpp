#include "graphicsclass.h"
#include <new>

Graphics::Graphics(){
}

Graphics::Graphics(const Graphics& other){
}

Graphics::~Graphics(){
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	// Create the Direct3D object.
	d3d12_device_ = D3D12Device::GetD3d12DeviceInstance();
	if(!d3d12_device_)
	{
		return false;
	}
	// Initialize the Direct3D object.
	if(CHECK(d3d12_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))){
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	camera_ = std::make_shared<Camera>();
	if (!camera_) {
		return false;
	}
	camera_->SetPosition(0.0f, 0.0f, -5.0f);

	texture_shader_ = std::make_shared<TextureShader>();
	if (!texture_shader_) {
		return false;
	}
	if (CHECK(texture_shader_->Initialize(L"../../tut11/texture.vs", L"../../tut11/texture.ps"))) {
		MessageBox(hwnd, L"Could not initialize Texture Shader.", L"Error", MB_OK);
		return false;
	}

	bitmap_ = std::make_shared<Bitmap>();
	if (!bitmap_) {
		return false;
	}

	bitmap_->SetVertexShader(CD3DX12_SHADER_BYTECODE(texture_shader_->GetVertexShaderBlob().Get()));
	bitmap_->SetPixelShader(CD3DX12_SHADER_BYTECODE(texture_shader_->GetPixelSHaderBlob().Get()));
	if (CHECK(bitmap_->Initialize(L"../../tut11/data/seafloor.dds",800,600,255,255))) {
		MessageBox(hwnd, L"Could not initialize Model.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void Graphics::Shutdown(){
	// Release the D3D object.
	if(d3d12_device_){
		delete d3d12_device_;
		d3d12_device_ = nullptr;
	}
}

bool Graphics::Frame(){
	// Render the graphics scene.
	if (FAILED(Render())) {
		return false;
	}

	return true;
}


bool Graphics::Render(){

	camera_->Render();

	DirectX::XMMATRIX world = {};
	d3d12_device_->GetWorldMatrix(world);
	world = DirectX::XMMatrixTranspose(world);

	DirectX::XMMATRIX orthogonality = {};
	d3d12_device_->GetOrthoMatrix(orthogonality);
	orthogonality = DirectX::XMMatrixTranspose(orthogonality);
	
	DirectX::XMMATRIX view = {};
	camera_->GetViewMatrix(view);
	view = DirectX::XMMatrixTranspose(view);

	d3d12_device_->SetVertexBuffer(bitmap_->GetVertexBufferView());
	d3d12_device_->SetIndexBuffer(bitmap_->GetIndexBufferView(),bitmap_->GetIndexCount());

	auto root_signature = bitmap_->GetRootSignature();
	auto pso = bitmap_->GetPipelineStateObject();
	auto texture_des_heap = bitmap_->GetShaderRescourceView();

	ID3D12DescriptorHeap *heap[] = { texture_des_heap.Get() };

	bitmap_->UpdateConstantBuffer(world, view, orthogonality);
	bitmap_->UpdateBitmapPos(100, 100);

	if (FAILED(d3d12_device_->render(root_signature.Get(),pso.Get(),heap,1,bitmap_->GetConstantBuffer()))) {
		return false;
	}

	return true;
}
