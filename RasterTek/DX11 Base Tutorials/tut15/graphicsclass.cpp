#include "graphicsclass.h"
#include <new>

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND hwnd){
	// Create the Direct3D object.
	d3d12_device_ = D3D12Device::GetD3d12DeviceInstance();
	if(!d3d12_device_){
		return false;
	}

	// Initialize the Direct3D object.
	if(CHECK(d3d12_device_->Initialize(
		screenWidth, 
		screenHeight, 
		VSYNC_ENABLED, 
		hwnd, 
		FULL_SCREEN, 
		SCREEN_DEPTH, 
		SCREEN_NEAR
		))){
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	camera_ = std::make_shared<Camera>();
	if (!camera_) {
		return false;
	}
	camera_->SetPosition(0.0f, 0.0f, -5.0f);

	font_shader_ = std::make_shared<FontShader>();
	if (!font_shader_) {
		return false;
	}

	if (CHECK(font_shader_->Initialize(L"../../tut12/font.vs", L"../../tut12/font.ps"))) {
		return false;
	}

	text_ = std::make_shared<Text>();
	if (!text_) {
		MessageBox(hwnd, L"Could not initialize Text object.", L"Error", MB_OK);
		return false;
	}

	if (CHECK(text_->LoadFont(L"../../tut12/data/fontdata.txt", L"../../tut12/data/font.dds"))) {
		MessageBox(hwnd, L"Could not initialize Font data.", L"Error", MB_OK);
		return false;
	}

	DirectX::XMMATRIX base_matrix = {};
	camera_->GetViewMatrix(base_matrix);
	if (CHECK(text_->Initialize(screenWidth, screenHeight, base_matrix))) {
		return false;
	}
	
	text_->SetVertexShader(CD3DX12_SHADER_BYTECODE(font_shader_->GetVertexShaderBlob().Get()));
	text_->SetPixelShader(CD3DX12_SHADER_BYTECODE(font_shader_->GetPixelSHaderBlob().Get()));

	text_->InitializeRootSignature();
	text_->InitializeGraphicsPipelineState();
	text_->InitializeConstantBuffer();

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

	camera_->Update();

	DirectX::XMMATRIX world = {};
	d3d12_device_->GetWorldMatrix(world);
	world = DirectX::XMMatrixTranspose(world);

	DirectX::XMMATRIX orthogonality = {};
	d3d12_device_->GetOrthoMatrix(orthogonality);
	orthogonality = DirectX::XMMatrixTranspose(orthogonality);
	
	DirectX::XMMATRIX view = {};
	camera_->GetViewMatrix(view);
	view = DirectX::XMMatrixTranspose(view);

	auto root_signature = text_->GetRootSignature();
	auto normal_pso = text_->GetNormalPso();
	auto blend_enabled_pso = text_->GetBlendEnabledPso();
	auto texture_des_heap = text_->GetShaderRescourceView();

	text_->UpdateMatrixConstant(world, view, orthogonality);
	DirectX::XMFLOAT4 pixel_color(1.0f, 0.0f, 0.0f, 0.0f);
	text_->UpdateLightConstant(pixel_color);

	for (auto i = 0; i < text_->GetSentenceCount(); ++i) {

		d3d12_device_->SetVertexBuffer(text_->GetVertexBufferView(i));
		d3d12_device_->SetIndexBuffer(text_->GetIndexBufferView(i), text_->GetIndexCount(i));

		ID3D12DescriptorHeap *heap[] = { text_->GetShaderRescourceView().Get() };

		if (FAILED(d3d12_device_->render(
			root_signature.Get(),
			blend_enabled_pso.Get(),
			heap, 1,
			text_->GetMatrixConstantBuffer(),
			text_->GetPixelConstantBuffer()
			))) {
			return false;
		}
	}

	return true;
}
