#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include <vector>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3dcommon.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#define CHECK(return_value) ((return_value==false)?true:false)

typedef std::vector<D3D12_VIEWPORT> ViewPortVector;

typedef std::vector<D3D12_RECT> ScissorRectVector;

typedef Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChainPtr;

typedef Microsoft::WRL::ComPtr<ID3D12DebugDevice> D2d12DebugDevicePtr;

typedef Microsoft::WRL::ComPtr<ID3D12DebugCommandQueue> D3d12DebugCommandQueuePtr;

typedef Microsoft::WRL::ComPtr<ID3D12DebugCommandList> D3d12DebugCommandListPtr;

typedef Microsoft::WRL::ComPtr<ID3D12Device> D3d12DevicePtr;
		
typedef Microsoft::WRL::ComPtr<ID3D12Resource> D3d12ResourcePtr;
		
typedef Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocatorPtr;

typedef Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueuePtr;
		
typedef Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignaturePtr;
		
typedef Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapPtr;
		
typedef Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineStateObjectPtr;
		
typedef Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GraphicsCommandListPtr;
		
typedef Microsoft::WRL::ComPtr<ID3D12Fence> FencePtr;

typedef Microsoft::WRL::ComPtr<ID3DBlob> BlobPtr;

class D3D12Device
{
public:
	D3D12Device();

	D3D12Device(const D3D12Device&) = delete;

	~D3D12Device();

public:
	static D3D12Device* GetD3d12DeviceInstance();

	D3d12DevicePtr GetD3d12Device() { return d3d12device_; }

public:
	bool Initialize(
		int screen_width, int screen_height,
		bool vsync, HWND hwnd, bool fullscreen,
		float screen_depth, float screen_near
		);

	void UpdateConstantParameters();

	bool ResetCommandList();

	bool SetGraphicsRootSignature(
		RootSignaturePtr graphics_rootsignature, 
		const UINT commandlist_index = 0
		);

	bool SetPipelineStateObject(PipelineStateObjectPtr pso,
		const UINT commandlist_index = 0
		);

	bool SetPipelineParameters(
		ID3D12DescriptorHeap * const descriptor_arr[], 
		const D3d12ResourcePtr constant_buffer,
		const UINT num_descriptors, 
		const UINT commandlist_index = 0
		);

	void SetVertexBuffer(
		const D3D12_VERTEX_BUFFER_VIEW& vertex_buffer_view,
		const UINT commandlist_index = 0
		);

	void SetIndexBuffer(
		const D3D12_INDEX_BUFFER_VIEW& index_buffer_view,
		const UINT num_index,
		const UINT commandlist_index = 0
		);

	bool PopulateGraphicsCommandList();

	bool render(
		RootSignaturePtr root_signature,
		PipelineStateObjectPtr pso,
		ID3D12DescriptorHeap * const descriptor_arr[],
		const UINT num_descriptors,
		const D3d12ResourcePtr constant_buffer
		);

	bool WaitForPreviousFrame();

	void GetProjectionMatrix(DirectX::XMMATRIX& projection);
	
	void GetWorldMatrix(DirectX::XMMATRIX& world);
	
	void GetOrthoMatrix(DirectX::XMMATRIX& ortho);

	void GetVideoCardInfo(char* card_name, int& memory);

private:
	static const UINT frame_cout_ = 2;

	bool is_vsync_enabled_ = false;
	
	int video_card_memory_ = -1;
	
	char video_card_description_[128] = { 0 };

	bool is_command_list_reset_ = false;

	bool is_use_vertex_buffer_ = true;

	bool is_use_index_buffer_ = true;

	UINT index_count_per_instance_ = 0;

private:
	ViewPortVector viewport_cache_ = {};

	ScissorRectVector scissor_rect_cache_ = {};

	SwapChainPtr swap_chain_ = nullptr;

	D2d12DebugDevicePtr d3d12_debug_device_ = nullptr;

	D3d12DebugCommandQueuePtr debug_command_queue_ = nullptr;

	D3d12DebugCommandListPtr debug_command_list_ = nullptr;

	D3d12DevicePtr d3d12device_ = nullptr;

	CommandAllocatorPtr command_allocator_ = nullptr;

	GraphicsCommandListPtr default_graphics_command_list_ = nullptr;

	CommandQueuePtr default_command_queue_ = nullptr;

	FencePtr fence_ = nullptr;

private:
	D3d12ResourcePtr render_targets_[frame_cout_] = { nullptr };

	DescriptorHeapPtr render_target_view_heap_ = nullptr;

	UINT render_target_descriptor_size_ = 0;

	D3d12ResourcePtr depth_stencil_resource_ = nullptr;

	DescriptorHeapPtr depth_stencil_view_heap_ = nullptr;

	UINT depth_stencil_descriptor_size_ = 0;

	RootSignaturePtr root_signature_ = nullptr;

	PipelineStateObjectPtr pso_cache_ = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;

	D3D12_INDEX_BUFFER_VIEW index_buffer_view_;

	UINT frame_index_ = 0;

	HANDLE fence_handle_ = 0;

	UINT64 fence_value_ = -1;

private:
	DirectX::XMMATRIX projection_matrix_;
	
	DirectX::XMMATRIX world_matrix_;
	
	DirectX::XMMATRIX ortho_matrix_;

private:
	static D3D12Device* d3d12device_instance_;
};

#endif