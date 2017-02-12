#include "d3dclass.h"

D3D12Device* D3D12Device::d3d12device_instance_ = nullptr;

D3D12Device::D3D12Device(){
}

D3D12Device::~D3D12Device(){
	WaitForPreviousFrame();
}

D3D12Device * D3D12Device::GetD3d12DeviceInstance(){
	if (nullptr == d3d12device_instance_) {
		d3d12device_instance_ = new D3D12Device;
	}
	return d3d12device_instance_;
}

bool D3D12Device::Initialize(int screen_width, int screen_height,
	bool vsync, HWND hwnd, bool fullscreen,
	float screen_depth, float screen_near
	) {

#if defined (_DEBUG)

	Microsoft::WRL::ComPtr<ID3D12Debug> debug_controller = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)))) {
		debug_controller->EnableDebugLayer();
	}
	else {
		assert(0);
	}

#endif //!(_DEBUG)

	IDXGIFactory4 *factory = nullptr;
	{
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
			return false;
		}
	}

	IDXGIAdapter1 *adapter = nullptr;
	DXGI_ADAPTER_DESC1 adapter_desc = { 0 };

	for (UINT adapter_index = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapter_index, &adapter); ++adapter_index) {
		adapter->GetDesc1(&adapter_desc);
		if (DXGI_ADAPTER_FLAG_SOFTWARE&adapter_desc.Flags) {
			continue;
		}
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12device_)))) {

			d3d12device_->SetName(L"d3d12 device");
			video_card_memory_ = static_cast<int>(adapter_desc.DedicatedVideoMemory / 1024 / 1024);
			size_t string_length = 0;
			auto error = wcstombs_s(&string_length, video_card_description_, 128, adapter_desc.Description, 128);
			if (error != 0) {
				return false;
			}

			break;
		}
		adapter_desc = { 0 };
	}
	adapter->Release();

	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	if (FAILED(d3d12device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&default_command_queue_)))) {
		return false;
	}
	default_command_queue_->SetName(L"default command queue");

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.BufferCount = frame_cout_;
	swap_chain_desc.Width = screen_width;
	swap_chain_desc.Height = screen_height;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.SampleDesc.Count = 1;


	Microsoft::WRL::ComPtr<IDXGISwapChain1> tem_swap_chain = nullptr;

	if (FAILED(factory->CreateSwapChainForHwnd(
		default_command_queue_.Get(),
		hwnd,
		&swap_chain_desc,
		nullptr,
		nullptr,
		&tem_swap_chain
		))) {
		return false;
	}

	if (FAILED(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER))) {
		return false;
	}

	if (FAILED(tem_swap_chain.As(&swap_chain_))) {
		return false;
	}

	frame_index_ = swap_chain_->GetCurrentBackBufferIndex();

	factory->Release();

	D3D12_DESCRIPTOR_HEAP_DESC render_target_heap_desc = {};
	render_target_heap_desc.NumDescriptors = frame_cout_;
	render_target_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	render_target_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(d3d12device_->CreateDescriptorHeap(&render_target_heap_desc, IID_PPV_ARGS(&render_target_view_heap_)))) {
		return false;
	}
	render_target_view_heap_->SetName(L"render targets descriptor heap");

	render_target_descriptor_size_ = d3d12device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE render_target_handle(render_target_view_heap_->GetCPUDescriptorHandleForHeapStart());

	for (UINT index = 0; index < frame_cout_; ++index) {
		if (FAILED(swap_chain_->GetBuffer(index, IID_PPV_ARGS(&render_targets_[index])))) {
			return false;
		}
		d3d12device_->CreateRenderTargetView(render_targets_[index].Get(), nullptr, render_target_handle);
		render_target_handle.Offset(1, render_target_descriptor_size_);
	}

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(d3d12device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&depth_stencil_view_heap_)))) {
		return false;
	}
	depth_stencil_view_heap_->SetName(L"detpth stencil descriptor heap");

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	if (FAILED(d3d12device_->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D32_FLOAT, 
			screen_width, 
			screen_height, 
			1,
			0,
			1,
			0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
			),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depth_stencil_resource_)
		))) {
		return false;
	}
	depth_stencil_resource_->SetName(L"depth stencil resource");

	d3d12device_->CreateDepthStencilView(
		depth_stencil_resource_.Get(), 
		&depthStencilDesc, 
		depth_stencil_view_heap_->GetCPUDescriptorHandleForHeapStart()
		);

	depth_stencil_descriptor_size_ = d3d12device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	if (FAILED(d3d12device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator_)))) {
		return false;
	}
	command_allocator_->SetName(L"default command allocator");

	if (FAILED(d3d12device_->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		command_allocator_.Get(),
		nullptr,
		IID_PPV_ARGS(&default_graphics_command_list_)))) {
		return false;
	}
	default_graphics_command_list_->SetName(L"default graphics command list");

	if (FAILED(default_graphics_command_list_->Close())) {
		return false;
	}

	if (FAILED(d3d12device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)))) {
		return false;
	}
	fence_->SetName(L"default fence");

	fence_handle_ = CreateEvent(nullptr, false, false, nullptr);

	if (nullptr == fence_handle_) {
		if (FAILED((HRESULT_FROM_WIN32(GetLastError())))) {
			return false;
		}
	}

	D3D12_VIEWPORT view_port = {};
	view_port.Width = static_cast<float>(screen_width);
	view_port.Height = static_cast<float>(screen_height);
	view_port.MinDepth = 0.0f;
	view_port.MaxDepth = 1.0f;
	view_port.TopLeftX = 0.0f;
	view_port.TopLeftY = 0.0f;
	viewport_cache_.push_back(view_port);

	D3D12_RECT rect = {};
	rect.right = static_cast<LONG>(screen_width);
	rect.bottom = static_cast<LONG>(screen_height);
	scissor_rect_cache_.push_back(rect);

	float field_of_View, screen_aspect;

	field_of_View = (DirectX::XM_PI / 4.0f);
	screen_aspect = static_cast<float>(screen_width) / static_cast<float>(screen_height);

	projection_matrix_ = DirectX::XMMatrixIdentity();

	projection_matrix_ = DirectX::XMMatrixPerspectiveFovLH(field_of_View, screen_aspect, screen_near, screen_depth);

	world_matrix_ = DirectX::XMMatrixIdentity();

	ortho_matrix_ = DirectX::XMMatrixOrthographicLH(static_cast<float>(screen_width), static_cast<float>(screen_height), screen_near, screen_depth);

	return true;
}

void D3D12Device::UpdateConstantParameters(){
}

bool D3D12Device::ResetCommandList(){
	if (FAILED(command_allocator_->Reset())) {
		return false;
	}

	if (FAILED(default_graphics_command_list_->Reset(command_allocator_.Get(), nullptr))) {
		return false;
	}
	is_command_list_reset_ = true;
	
	return true;
}

bool D3D12Device::SetGraphicsRootSignature(
	RootSignaturePtr graphics_rootsignature,
	const UINT commandlist_index){
	if (false == is_command_list_reset_) {
		return false;
	}
	default_graphics_command_list_->SetGraphicsRootSignature(graphics_rootsignature.Get());
	return true;
}

bool D3D12Device::SetPipelineStateObject(
	PipelineStateObjectPtr pso, 
	const UINT commandlist_index){
	if (false == is_command_list_reset_) {
		return false;
	}
	default_graphics_command_list_->SetPipelineState(pso.Get());
	return true;
}

bool D3D12Device::SetPipelineParameters(
	ID3D12DescriptorHeap * const descriptor_arr[], 
	const D3d12ResourcePtr constant_buffer,
	const UINT num_descriptors, 
	const UINT commandlist_index){
	if (false == is_command_list_reset_) {
		return false;
	}
	default_graphics_command_list_->SetDescriptorHeaps(num_descriptors, descriptor_arr);
	
	default_graphics_command_list_->SetGraphicsRootDescriptorTable(0, descriptor_arr[0]->GetGPUDescriptorHandleForHeapStart());

	default_graphics_command_list_->SetGraphicsRootConstantBufferView(1, constant_buffer->GetGPUVirtualAddress());

	return true;
}

void D3D12Device::SetVertexBuffer(
	const D3D12_VERTEX_BUFFER_VIEW & vertex_buffer_view, 
	const UINT commandlist_index
	){
	vertex_buffer_view_ = vertex_buffer_view;
}

void D3D12Device::SetIndexBuffer(
	const D3D12_INDEX_BUFFER_VIEW & index_buffer_view,
	const UINT num_index,
	const UINT commandlist_index
	){
	index_count_per_instance_ = num_index;
	index_buffer_view_ = index_buffer_view;
}

bool D3D12Device::PopulateGraphicsCommandList() {

	if (false == is_command_list_reset_) {
		return false;
	}

	default_graphics_command_list_->RSSetViewports(1, &viewport_cache_.at(0));
	default_graphics_command_list_->RSSetScissorRects(1, &scissor_rect_cache_.at(0));

	default_graphics_command_list_->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			render_targets_[frame_index_].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET)
		);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(
		render_target_view_heap_->GetCPUDescriptorHandleForHeapStart(),
		frame_index_,
		render_target_descriptor_size_);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle(
		depth_stencil_view_heap_->GetCPUDescriptorHandleForHeapStart()
		);

	default_graphics_command_list_->OMSetRenderTargets(1, &rtv_handle, FALSE, &dsv_handle);

	const float clear_color[] = { 0.0f,0.2f,0.4f,1.0f };
	default_graphics_command_list_->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
	default_graphics_command_list_->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	default_graphics_command_list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if(is_use_vertex_buffer_){
		default_graphics_command_list_->IASetVertexBuffers(0, 1, &vertex_buffer_view_);
	}
	if(is_use_index_buffer_){
		default_graphics_command_list_->IASetIndexBuffer(&index_buffer_view_);
	}

	default_graphics_command_list_->DrawIndexedInstanced(index_count_per_instance_, 1, 0, 0, 0);

	default_graphics_command_list_->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		render_targets_[frame_index_].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT)
		);

	if (FAILED(default_graphics_command_list_->Close())) {
		return false;
	}

	return true;
}

bool D3D12Device::render(
	RootSignaturePtr root_signature,
	PipelineStateObjectPtr pso,
	ID3D12DescriptorHeap * const descriptor_arr[],
	const UINT num_descriptors,
	const D3d12ResourcePtr constant_buffer
	){

	if (FAILED(ResetCommandList())) {
		return false;
	}

	SetGraphicsRootSignature(root_signature);
	SetPipelineStateObject(pso);
	SetPipelineParameters(descriptor_arr, constant_buffer,num_descriptors);

	PopulateGraphicsCommandList();

	ID3D12CommandList *command_list[] = { default_graphics_command_list_.Get() };
	default_command_queue_->ExecuteCommandLists(1, command_list);

	if (FAILED(swap_chain_->Present(1, 0))) {
		return false;
	}

	WaitForPreviousFrame();

	return true;
}

bool D3D12Device::WaitForPreviousFrame() {

	const UINT64 fence = fence_value_;
	if (FAILED(default_command_queue_->Signal(fence_.Get(), fence_value_))) {
		return false;
	}
	++fence_value_;

	if (fence_->GetCompletedValue() < fence) {
		if (FAILED(fence_->SetEventOnCompletion(fence, fence_handle_))) {
			return false;
		}
		WaitForSingleObject(fence_handle_, INFINITE);
	}

	frame_index_ = swap_chain_->GetCurrentBackBufferIndex();

	return true;
}

void D3D12Device::GetProjectionMatrix(DirectX::XMMATRIX& projection_matrix ){
	projection_matrix = projection_matrix_;
}


void D3D12Device::GetWorldMatrix(DirectX::XMMATRIX& world_matrix ){
	world_matrix = world_matrix_;
}


void D3D12Device::GetOrthoMatrix(DirectX::XMMATRIX& ortho_matrix ){
	ortho_matrix = ortho_matrix_;
}


void D3D12Device::GetVideoCardInfo(char* card_name, int& memory){
	strcpy_s(card_name, 128, video_card_description_);
	memory = video_card_memory_;
}