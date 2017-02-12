#include "d3d11device.h"

#include <fstream>
#include <d3dcompiler.h>

D3d11Device* D3d11Device::d3d_object_ = nullptr;

D3d11Device::D3d11Device() {}

D3d11Device::D3d11Device(const D3d11Device& copy) {}

D3d11Device::~D3d11Device() {
	if (nullptr != d3d_object_) {
		delete d3d_object_;
		d3d_object_ = nullptr;
	}
}

D3d11Device * D3d11Device::get_d3d_object(){
	if (nullptr == d3d_object_) {
		d3d_object_ = new D3d11Device;
	}
	return d3d_object_;
}

bool D3d11Device::Initialize(int screen_width, int screen_height,
	bool vsync, HWND hwnd, bool full_screen,
	float screen_depth, float screen_near) {

	vsync_enable_ = vsync;

	IDXGIFactory1 *factory;
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory)))) {
		return false;
	}

	std::vector<IDXGIAdapter1*> adapter_vector;
	IDXGIAdapter1 *tempory_adapter = nullptr;
	int counter = 0;
	std::vector<DXGI_ADAPTER_DESC1> adapter_desc_vector;
	DXGI_ADAPTER_DESC1 tempory_adapter_desc;
	ZeroMemory(&tempory_adapter_desc,sizeof(tempory_adapter_desc));

	while ((DXGI_ERROR_NOT_FOUND!=(factory->EnumAdapters1(counter++, &tempory_adapter)))) {
		tempory_adapter->GetDesc1(&tempory_adapter_desc);
		if(tempory_adapter_desc.Flags&DXGI_ADAPTER_FLAG_SOFTWARE){
			continue;
		}
		adapter_vector.push_back(tempory_adapter);
		adapter_desc_vector.push_back(tempory_adapter_desc);
	}
	if (0 == adapter_vector.size()) {
		return false;
	}

	// todo 1. manul select the apporiate adapter
	//      2. select the primary adapter on the multiple adapter platform

	// now we only select the primary adapter with hard code in may laptop

	auto adapter_desc_index = adapter_vector.size();
	IDXGIOutput *dxgi_output = nullptr;
	for (auto i = adapter_vector.size()-1; i >= 0; --i) {
		if (FAILED(adapter_vector.at(i)->EnumOutputs(0, &dxgi_output))) {
			continue;
		}
		else {
			adapter_desc_index = i;
			break;
		}
	}

	UINT num_modes = 0;
	if (FAILED(dxgi_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, nullptr))) {
		return false;
	}

	DXGI_MODE_DESC *display_mode_list = new DXGI_MODE_DESC[num_modes];
	if (!display_mode_list) {
		return false;
	}

	if (FAILED(dxgi_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, display_mode_list))) {
		return false;
	}

	UINT numerator = 0;
	UINT denominator = 0;
	auto i = num_modes;
	for (; i < num_modes; ++i) {
		if (static_cast<UINT>(screen_width) == display_mode_list[i].Width) {

			if (static_cast<UINT>(screen_height) == display_mode_list[i].Height) {

				numerator = display_mode_list[i].RefreshRate.Numerator;
				denominator = display_mode_list[i].RefreshRate.Denominator;
			}
		}
	}

	videocard_memory_capacity_ = static_cast<int>(adapter_desc_vector.at(adapter_desc_index).DedicatedVideoMemory/1024/1024);
	size_t string_length = 0;
	auto return_status = wcstombs_s(&string_length, videocard_description_, 128, adapter_desc_vector.at(adapter_desc_index).Description, 128);
	if (0 != return_status) {
		return false;
	}
	
	delete[] display_mode_list;
	display_mode_list = nullptr;

	dxgi_output->Release();
	dxgi_output = nullptr;

	for (auto it = adapter_vector.begin(); it != adapter_vector.end(); ++it) {
		(*it)->Release();
	}
	adapter_vector.clear();

	factory->Release();
	factory = nullptr;

	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));

	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.BufferDesc.Width = screen_width;
	swap_chain_desc.BufferDesc.Height = screen_height;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (vsync_enable_) {
		swap_chain_desc.BufferDesc.RefreshRate.Numerator = numerator;
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else {
		swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	swap_chain_desc.OutputWindow = hwnd;

	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;

	if (full_screen) {
		swap_chain_desc.Windowed = false;
	}
	else {
		swap_chain_desc.Windowed = true;
	}

	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	swap_chain_desc.Flags = 0;

	auto feature_level = D3D_FEATURE_LEVEL_11_0;
	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_DEBUG, &feature_level, 1, D3D11_SDK_VERSION,
		&swap_chain_desc, &swap_chain, &device,
		nullptr, &device_context))) {
		return false;
	}

	ID3D11Texture2D *back_buffer = nullptr;
	if (FAILED(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer))) {
		return false;
	}

	if (FAILED(device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view))) {
		return false;
	}
	back_buffer->Release();
	back_buffer = nullptr;

	D3D11_TEXTURE2D_DESC depth_stencil_buffer_desc;
	ZeroMemory(&depth_stencil_buffer_desc, sizeof(depth_stencil_buffer_desc));

	depth_stencil_buffer_desc.Width = screen_width;
	depth_stencil_buffer_desc.Height = screen_height;
	depth_stencil_buffer_desc.MipLevels = 1;
	depth_stencil_buffer_desc.ArraySize = 1;
	depth_stencil_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_buffer_desc.SampleDesc.Count = 1;
	depth_stencil_buffer_desc.SampleDesc.Quality = 0;
	depth_stencil_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_buffer_desc.CPUAccessFlags = 0;
	depth_stencil_buffer_desc.MiscFlags = 0;

	if (FAILED(device->CreateTexture2D(&depth_stencil_buffer_desc, nullptr, &deptn_stencil_buffer))) {
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));

	depth_stencil_desc.DepthEnable = true;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

	depth_stencil_desc.StencilEnable = true;
	depth_stencil_desc.StencilReadMask = 0xff;
	depth_stencil_desc.StencilWriteMask = 0xff;

	depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	if (FAILED(device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state))) {
		return false;
	}
	device_context->OMSetDepthStencilState(depth_stencil_state, 1);

	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
	ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));

	depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;

	if (FAILED(device->CreateDepthStencilView(deptn_stencil_buffer, &depth_stencil_view_desc, &depth_stencil_view))) {
		return false;
	}
	device_context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);

	D3D11_RASTERIZER_DESC rasterizer_desc;
	ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));

	rasterizer_desc.AntialiasedLineEnable = false;
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0.0f;
	rasterizer_desc.DepthClipEnable = true;
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.FrontCounterClockwise = false;
	rasterizer_desc.MultisampleEnable = false;
	rasterizer_desc.ScissorEnable = false;
	rasterizer_desc.SlopeScaledDepthBias = 0.0f;

	if (FAILED(device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state))) {
		return false;
	}
	device_context->RSSetState(rasterizer_state);

	rasterizer_desc.CullMode = D3D11_CULL_NONE;

	if (FAILED(device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_no_culling))) {
		return false;
	}

	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;

	if (FAILED(device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_wireframe))) {
		return false;
	}

	D3D11_VIEWPORT view_port;
	ZeroMemory(&view_port, sizeof(view_port));

	view_port.Width = static_cast<float>(screen_width);
	view_port.Height = static_cast<float>(screen_height);
	view_port.MinDepth = 0.0f;
	view_port.MaxDepth = 1.0f;
	view_port.TopLeftX = 0.0f;
	view_port.TopLeftY = 0.0f;

	device_context->RSSetViewports(1, &view_port);

	auto field_of_View = 3.141592654f / 4.0f;
	auto screen_aspect = static_cast<float>(screen_width) / static_cast<float>(screen_height);

	projection_matrix = DirectX::XMMatrixPerspectiveFovLH(field_of_View, screen_aspect, screen_near, screen_depth);

	world_matrix = DirectX::XMMatrixIdentity();

	orthoganlity_matrix = DirectX::XMMatrixOrthographicLH(static_cast<float>(screen_width), static_cast<float>(screen_height), screen_near, screen_depth);

	D3D11_DEPTH_STENCIL_DESC stencil_no_depth_desc;
	ZeroMemory(&stencil_no_depth_desc, sizeof(stencil_no_depth_desc));

	stencil_no_depth_desc.DepthEnable = false;
	stencil_no_depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	stencil_no_depth_desc.DepthFunc = D3D11_COMPARISON_LESS;
	stencil_no_depth_desc.StencilEnable = true;
	stencil_no_depth_desc.StencilReadMask = 0xFF;
	stencil_no_depth_desc.StencilWriteMask = 0xFF;
	stencil_no_depth_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	stencil_no_depth_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	stencil_no_depth_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	stencil_no_depth_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	stencil_no_depth_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	stencil_no_depth_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	stencil_no_depth_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	stencil_no_depth_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	if (FAILED(device->CreateDepthStencilState(&stencil_no_depth_desc, &stencil_no_depth_state))) {
		return false;
	}

	D3D11_BLEND_DESC alpha_blend_desc;
	ZeroMemory(&alpha_blend_desc, sizeof(alpha_blend_desc));

	alpha_blend_desc.AlphaToCoverageEnable = false;
	alpha_blend_desc.IndependentBlendEnable = false;
	alpha_blend_desc.RenderTarget[0].BlendEnable = true;
	alpha_blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	alpha_blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alpha_blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	alpha_blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	alpha_blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	alpha_blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	alpha_blend_desc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	if (FAILED(device->CreateBlendState(&alpha_blend_desc, &alpha_blending_state))) {
		return false;
	}

	alpha_blend_desc.RenderTarget[0].BlendEnable = false;

	if (FAILED(device->CreateBlendState(&alpha_blend_desc, &blending_no_alpha_state))) {
		return false;
	}

	alpha_blend_desc.AlphaToCoverageEnable = true;
	alpha_blend_desc.RenderTarget[0].BlendEnable = true;
	alpha_blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC1_ALPHA;

	if (FAILED(device->CreateBlendState(&alpha_blend_desc, &alpha_to_coverage_blending_state))) {
		return false;
	}

	return true;
}

void D3d11Device::Shutdown() {

	if (swap_chain) {
		swap_chain->SetFullscreenState(false, nullptr);
	}

	if (alpha_blending_state) {
		alpha_blending_state->Release();
		alpha_blending_state = nullptr;
	}

	if (alpha_to_coverage_blending_state) {
		alpha_to_coverage_blending_state->Release();
		alpha_to_coverage_blending_state = nullptr;
	}

	if (blending_no_alpha_state) {
		blending_no_alpha_state->Release();
		blending_no_alpha_state = nullptr;
	}

	if (depth_stencil_state) {
		depth_stencil_state->Release();
		depth_stencil_state = nullptr;
	}

	if (stencil_no_depth_state) {
		stencil_no_depth_state->Release();
		stencil_no_depth_state = nullptr;
	}

	if (rasterizer_state) {
		rasterizer_state->Release();
		rasterizer_state = nullptr;
	}

	if (rasterizer_state_no_culling) {
		rasterizer_state_no_culling->Release();
		rasterizer_state_no_culling = nullptr;
	}
	
	if (rasterizer_state_wireframe) {
		rasterizer_state_wireframe->Release();
		rasterizer_state_wireframe = nullptr;
	}

	if (depth_stencil_state) {
		depth_stencil_state->Release();
		depth_stencil_state = nullptr;
	}

	if (stencil_no_depth_state) {
		stencil_no_depth_state->Release();
		stencil_no_depth_state = nullptr;
	}

	if (depth_stencil_view) {
		depth_stencil_view->Release();
		depth_stencil_view = nullptr;
	}

	if (deptn_stencil_buffer) {
		deptn_stencil_buffer->Release();
		deptn_stencil_buffer = nullptr;
	}

	if (render_target_view) {
		render_target_view->Release();
		render_target_view = nullptr;
	}

	if (device_context) {
		device_context->Release();
		device_context = nullptr;
	}

	if (device) {
		device->Release();
		device = nullptr;
	}

	if (swap_chain) {
		swap_chain->Release();
		swap_chain = nullptr;
	}
}

void D3d11Device::BeginScene(float red, float green, float blue, float alpha) {
	
	float color[4];
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	device_context->ClearRenderTargetView(render_target_view, color);
	device_context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void D3d11Device::EndScene() {
	if (vsync_enable_) {
		swap_chain->Present(1, 0);
	}
	else {
		swap_chain->Present(0, 0);
	}
}

ID3D11Device* D3d11Device::get_device() {
	return device;
}

ID3D11DeviceContext* D3d11Device::get_device_context() {
	return device_context;
}

Result D3d11Device::CreateVertexShader(
	LPCWSTR vertex_shader_filename, 
	LPCSTR entry_point, 
	LPCSTR target,
	LPCWSTR vertex_shader_name
	){

	Status s;
	Result result(std::make_pair(-1, s));

	auto count = vertex_shader_name_table_.count(vertex_shader_name);
	if (1 == count) {
		result.first = vertex_shader_name_table_[vertex_shader_name];
		return (result);
	}

	ID3D10Blob *error_message = nullptr;
	VertexShader tem_vertex_shader;
	auto compiled_shader_buffer = tem_vertex_shader.get_compiled_shader().get();

	if (FAILED(D3DCompileFromFile(
		vertex_shader_filename, 
		nullptr, nullptr, 
		entry_point, target,
		D3D10_SHADER_ENABLE_STRICTNESS, 
		0, (compiled_shader_buffer), 
		&error_message))) {

		if (error_message) {
			auto error_slice = Slice(static_cast<char*>(error_message->GetBufferPointer()));
			result.second = s.ShaderCompiledFailed(error_slice);
			return (result);
		}
		else {
			auto error_slice = Slice("vertex shader compiled failed");
			result.second = s.ShaderCompiledFailed(error_slice);
			return (result);
		}
	}

	auto tem_shader_buffer = tem_vertex_shader.get_vertex_shader().get();
	if (FAILED(device->CreateVertexShader(
		(*compiled_shader_buffer)->GetBufferPointer(),
		(*compiled_shader_buffer)->GetBufferSize(),
		nullptr,tem_shader_buffer))){

		result.second = s.CreatedFailed("vertex shader created failed");
		return (result);
	}

	tem_vertex_shader.set_valid();
	vertex_shader_container_.push_back(tem_vertex_shader);

	vertex_shader_name_table_[vertex_shader_name] = vertex_shader_container_.size() - 1;

	result.first = vertex_shader_container_.size() - 1;
	return (result);
}

Result D3d11Device::CreatePixelShader(
	LPCWSTR pixel_shader_filename, 
	LPCSTR entry_point, LPCSTR target,
	LPCWSTR pixel_shader_name
	){

	Status s;
	Result result(std::make_pair(-1, s));
	
	auto count = pixel_shader_name_table_.count(pixel_shader_name);
	if (1 == count) {
		result.first = pixel_shader_name_table_[pixel_shader_name];
		return (result);
	}

	ID3D10Blob *error_message = nullptr;
	PixelShader tem_pixel_shader;
	auto compiled_shader_buffer = tem_pixel_shader.get_compiled_shader().get();

	if (FAILED(D3DCompileFromFile(
		pixel_shader_filename,
		nullptr, nullptr,
		entry_point, target,
		D3D10_SHADER_ENABLE_STRICTNESS,
		0,
		compiled_shader_buffer,
		&error_message))) {

		if (error_message) {
			auto error_slice = Slice(static_cast<char*>(error_message->GetBufferPointer()));
			result.second = s.ShaderCompiledFailed(error_slice);
			return (result);
		}
		else {
			result.second = s.ShaderCompiledFailed("pixel shader complied failed");
			return (result);
		}
	}

	auto tem_shader_buffer = tem_pixel_shader.get_pixel_shader().get();

	if (FAILED(device->CreatePixelShader(
		(*compiled_shader_buffer)->GetBufferPointer(),
		(*compiled_shader_buffer)->GetBufferSize(),
		nullptr,
		tem_shader_buffer))) {
	
		result.second = s.CreatedFailed("pixel shader create failed");
		return (result);
	}

	tem_pixel_shader.set_valid();
	pixel_shader_container_.push_back(tem_pixel_shader);
	pixel_shader_name_table_[pixel_shader_name] = pixel_shader_container_.size() - 1;

	result.first = pixel_shader_container_.size() - 1;
	return (result);
}

Result D3d11Device::CreateInputLayout(
	LPCWSTR input_layout_name, 
	InputLayoutDesc * input_layout_desces, 
	UINT num_input_layout_desc, 
	ID3D10Blob * compiled_shader_program,
	LPCWSTR associate_shader_name
	){

	Status s;
	Result result(std::make_pair(-1, s));

	auto count = input_layout_name_table_.count(input_layout_name);
	if (1 == count) {
		result.first = input_layout_name_table_[input_layout_name];
		return (result);
	}

	ID3D11InputLayout *tem_input_layout;
	if (SUCCEEDED(d3d_object_->get_device()->CreateInputLayout(
		input_layout_desces->get_input_layout_desc(),
		num_input_layout_desc,
		compiled_shader_program->GetBufferPointer(),
		compiled_shader_program->GetBufferSize(),
		&tem_input_layout))) {

		input_layout_container_.push_back(std::move(tem_input_layout));
		input_layout_name_table_[input_layout_name] = input_layout_container_.size() - 1;

		result.first = input_layout_container_.size() - 1;
		return (result);
	}
	else {
		result.second=s.CreatedFailed("input layout created failed");
		return (result);
	}
}

Result D3d11Device::CreateConstantBuffer(
	const ConstantBufferConfig& constant_buffer_config,
	const D3D11_SUBRESOURCE_DATA * constant_init_data,
	const LPCWSTR constant_buffer_name
	) {

	Status s;
	Result result(std::make_pair(-1, s));

	auto cout = constant_buffer_name_table_.count(constant_buffer_name);
	if (1 == cout) {
		result.first = constant_buffer_name_table_[constant_buffer_name];
		return (result);
	}

	ID3D11Buffer *tem_buffer = nullptr;
	if (SUCCEEDED(this->device->CreateBuffer(constant_buffer_config.get_buffer_desc(), constant_init_data, &tem_buffer))) {
		constant_buffer_container_.push_back(tem_buffer);
		constant_buffer_name_table_[constant_buffer_name] = constant_buffer_container_.size() - 1;

		result.first = constant_buffer_container_.size() - 1;
		return (result);
	}
	else {
		result.second = s.CreatedFailed("constant buffer created failed");
		return (result);
	}
}

Result D3d11Device::CreateVertexBuffer(
	const VertexBufferConfig vertex_buffer_config,
	const D3D11_SUBRESOURCE_DATA *vertex_init_data,
	const LPCWSTR vertex_buffer_name
	) {

	Status s;
	Result result(std::make_pair(-1, s));

	auto cout = vertex_buffer_name_table_.count(vertex_buffer_name);
	if (1 == cout) {
		result.first = vertex_buffer_name_table_[vertex_buffer_name];
		return (result);
	}

	ID3D11Buffer *tem_buffer = nullptr;
	if (SUCCEEDED(this->device->CreateBuffer(vertex_buffer_config.get_buffer_desc(), vertex_init_data, &tem_buffer))) {
		vertex_buffer_container_.push_back(tem_buffer);
		vertex_buffer_name_table_[vertex_buffer_name] = vertex_buffer_container_.size() - 1;

		result.first = vertex_buffer_container_.size() - 1;
		return (result);
	}
	else {
		result.second = s.CreatedFailed("vertex buffer created failed");
		return (result);
	}
}

Result D3d11Device::CreateIndexBuffer(
	const IndexBufferConfig index_buffer_config,
	const D3D11_SUBRESOURCE_DATA *index_init_data,
	const LPCWSTR index_buffer_name
	) {

	Status s;
	Result result(std::make_pair(-1, s));

	auto cout = vertex_buffer_name_table_.count(index_buffer_name);
	if (1 == cout) {
		result.first = vertex_buffer_name_table_[index_buffer_name];
		return (result);
	}

	ID3D11Buffer *tem_buffer = nullptr;
	if (SUCCEEDED(this->device->CreateBuffer(index_buffer_config.get_buffer_desc(), index_init_data, &tem_buffer))) {
		vertex_buffer_container_.push_back(tem_buffer);
		vertex_buffer_name_table_[index_buffer_name] = vertex_buffer_container_.size() - 1;

		result.first = vertex_buffer_container_.size() - 1;
		return (result);
	}
	else {
		result.second = s.CreatedFailed("index buffer created failed");
		return (result);
	}
}

Result D3d11Device::CreateSampler(
	const LPCWSTR sampler_state_name,
	SamplerState * sampler_state_desc,
	const UINT num_sampler_state_desc
	) {

	Status s;
	Result result(std::make_pair(-1, s));

	auto current_index = sampler_state_name_table_.count(sampler_state_name);
	if (0 != current_index) {
		result.first = current_index;
		return (result);
	}

	ID3D11SamplerState *tem_sampler_state = nullptr;
	if (SUCCEEDED(get_device()->CreateSamplerState(sampler_state_desc->GetSamplerStateDesc(), &tem_sampler_state))) {
		sampler_state_container_.push_back(tem_sampler_state);
		sampler_state_name_table_[sampler_state_name] = sampler_state_container_.size() - 1;

		result.first = sampler_state_container_.size() - 1;
		return (result);
	}
	else {
		result.second = s.CreatedFailed("sampler created failed");
		return (result);
	}
}

const VertexShader& D3d11Device::GetVertexShaderByIndex(int index)const{
	return vertex_shader_container_.at(index);
}

const VertexShader& D3d11Device::GetVertexShaderByName(LPCWSTR shader_name){
	assert(vertex_buffer_name_table_[shader_name] != -1);
	assert(vertex_shader_name_table_[shader_name] < vertex_shader_container_.size());

	return vertex_shader_container_.at(vertex_shader_name_table_[shader_name]);
}

const PixelShader& D3d11Device::GetPixelShaderByIndex(int index) const{
	return pixel_shader_container_.at(index);
}

const PixelShader& D3d11Device::GetPixelShaderByName(LPCWSTR shader_name){
	assert(pixel_shader_name_table_[shader_name] != -1);
	assert(pixel_shader_name_table_[shader_name] < pixel_shader_container_.size());
	
	return pixel_shader_container_.at(pixel_shader_name_table_[shader_name]);
}

ID3D11InputLayout * D3d11Device::GetInputLayoutByIndex(int index) const{
	return input_layout_container_.at(index);
}

ID3D11InputLayout * D3d11Device::GetInputLayoutByName(LPCWSTR name){
	assert(input_layout_name_table_[name] != -1);
	assert(input_layout_name_table_[name] < input_layout_container_.size());

	return input_layout_container_.at(input_layout_name_table_[name]);
}

ID3D11Buffer * D3d11Device::GetVertexBufferByIndex(const int index) const{
	return vertex_buffer_container_.at(index);
}

ID3D11Buffer * D3d11Device::GetVertexBufferByName(const LPCWSTR buffer_name){
	assert(vertex_buffer_name_table_[buffer_name] != -1);
	assert(vertex_buffer_name_table_[buffer_name] < vertex_buffer_container_.size());

	return vertex_buffer_container_.at(vertex_buffer_name_table_[buffer_name]);
}

ID3D11Buffer * D3d11Device::GetConstantBufferByIndex(const int index) const{
	return constant_buffer_container_.at(index);
}

ID3D11Buffer * D3d11Device::GetConstantBufferByName(const LPCWSTR buffer_name){
	assert(constant_buffer_name_table_[buffer_name] != -1);
	assert(constant_buffer_name_table_[buffer_name] < constant_buffer_container_.size());
	
	return constant_buffer_container_.at(constant_buffer_name_table_[buffer_name]);
}

ID3D11SamplerState * D3d11Device::GetSamplerByIndex(const int index) const{
	return sampler_state_container_.at(index);
}

ID3D11SamplerState * D3d11Device::GetSamplerByName(const LPCWSTR name){
	assert(sampler_state_name_table_[name] != -1);
	assert(sampler_state_name_table_[name] < sampler_state_container_.size());

	return sampler_state_container_.at(sampler_state_name_table_[name]);
}

void D3d11Device::get_projection_matrix(DirectX::XMMATRIX & projection_matrix){
	projection_matrix = this->projection_matrix;
}

void D3d11Device::get_world_matrix(DirectX::XMMATRIX & world_matrix){
	world_matrix = this->world_matrix;
}

void D3d11Device::get_orthogonality_matrix(DirectX::XMMATRIX & orthogonality_matrix){
	orthogonality_matrix = this->orthoganlity_matrix;
}

void D3d11Device::get_videocard_info(char * videocard_name, int &videocard_memory_capacity){
	strcpy_s(videocard_name, 128, (this->videocard_description_));
	videocard_memory_capacity = this->videocard_memory_capacity_;
}

void D3d11Device::TurnZBufferOn(){
	device_context->OMSetDepthStencilState(depth_stencil_state, 1);
}

void D3d11Device::TurnZBufferOff(){
	device_context->OMSetDepthStencilState(stencil_no_depth_state, 1);
}

void D3d11Device::TurnCullingOn(){
	device_context->RSSetState(rasterizer_state);
}

void D3d11Device::TurnCullingOff(){
	device_context->RSSetState(rasterizer_state_no_culling);
}

void D3d11Device::TurnAlphaBlendingOn(){

	float blend_factor[4];
	
	blend_factor[0] = 0.0f;
	blend_factor[1] = 0.0f;
	blend_factor[2] = 0.0f;
	blend_factor[3] = 0.0f;
	
	device_context->OMSetBlendState(alpha_blending_state, blend_factor, 0xffffffff);
}

void D3d11Device::TurnAlphaBlendingOff(){

	float blend_factor[4];

	blend_factor[0] = 0.0f;
	blend_factor[1] = 0.0f;
	blend_factor[2] = 0.0f;
	blend_factor[3] = 0.0f;

	device_context->OMSetBlendState(blending_no_alpha_state, blend_factor, 0xffffffff);
}

void D3d11Device::TurnWireFrameOn(){
	device_context->RSSetState(rasterizer_state_wireframe);
}

void D3d11Device::TurnWireFrameOff(){
	device_context->RSSetState(rasterizer_state);
}

void D3d11Device::TurnAlphaToCoverageBlendingOn(){

	float blend_factor[4];

	blend_factor[0] = 0.0f;
	blend_factor[1] = 0.0f;
	blend_factor[2] = 0.0f;
	blend_factor[3] = 0.0f;

	device_context->OMSetBlendState(alpha_to_coverage_blending_state, blend_factor, 0xffffffff);
}
