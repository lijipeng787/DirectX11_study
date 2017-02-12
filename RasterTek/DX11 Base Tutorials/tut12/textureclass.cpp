#include "textureclass.h"

bool Texture::set_texture(WCHAR * texture_filename) {
	auto device = D3D12Device::GetD3d12DeviceInstance()->GetD3d12Device();

	D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
	srv_heap_desc.NumDescriptors = 1;
	srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	if (FAILED(device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&shader_resource_view_heap_)))) {
		return false;
	}

	if (FAILED(CreateDDSTextureFromFile(
		device.Get(),
		texture_filename,
		0,
		false,
		&texture_,
		shader_resource_view_heap_.Get()->GetCPUDescriptorHandleForHeapStart()
		))) {
		return false;
	}

	return true;
}

DescriptorHeapPtr Texture::get_texture() {
	return shader_resource_view_heap_;
}
