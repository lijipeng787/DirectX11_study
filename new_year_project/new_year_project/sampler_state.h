#pragma once

#ifndef GRAPHICS_HEADER_SAMPLER_STATE_H

#define GRAPHICS_HEADER_SAMPLER_STATE_H

#include <d3d11.h>

enum class FilterType {
	kPointSamplingForMinMagMip = 0,
	
	kPointSamplingForMinMagLinearInterPolationForMip,
	
	kPoingSamplingForMinMipLinearInterpolationForMag,
	
	kLinearInterPolationForMagMipPointSamplingForMin,
	
	kPointSamplingForMagMipLinearInterpolationForMin,
	
	kLinearInterpolationForMinMipPointSamplingForMag,
	
	kLinearInterpolationForMinMagPointSamplingForMip,
	
	kLinearInterpolationForMinMagMip,
	
	kAnisotropicInterplationForMinMagMip,
	
	kPointSamplingForMinMagMipCompare,
	
	kPointSamplingForMinMapLinearInterpolationForMagCompare,
	
	kPointSamplingForMinMipLinearInterpolationForMagCompare,
	
	kLinearInterpolationForMagMipPointSamplingForMinCompare,
	
	kPointSamplingForMagMipLinearInterpolationForMinCompare,
	
	kLinearInterpolationForMinMipPointSamplingForMagCompare,
	
	kLinearInterpolationForMinMagPointSamplingFormipCompare,

	kLinearInterpolationForMinMagMipCompare,
	
	kAnisotropicInterplationForMinMagMipCompare,
	
	kMinPointSamplingForMinMagMip,
	
	kMinPointSamplingForMinMagLinearInterPolationForMip,
	
	kMinPoingSamplingForMinMipLinearInterpolationForMag,
	
	kMinLinearInterPolationForMagMipPointSamplingForMin,
	
	kMinPointSamplingForMagMipLinearInterpolationForMin,
	
	kMinLinearInterpolationForMinMipPointSamplingForMag,
	
	kMinLinearInterpolationForMinMagPointSamplingForMip,
	
	kMinLinearInterpolationForMinMagMip,
	
	kMinAnisotropicInterplationForMinMagMip,
	
	kMaxPointSamplingForMinMagMip,
	
	kMaxPointSamplingForMinMagLinearInterPolationForMip,
	
	kMaxPoingSamplingForMinMipLinearInterpolationForMag,
		
	kMaxLinearInterPolationForMagMipPointSamplingForMin,
	
	kMaxPointSamplingForMagMipLinearInterpolationForMin,
	
	kMaxLinearInterpolationForMinMipPointSamplingForMag,
	
	kMaxLinearInterpolationForMinMagPointSamplingForMip,	
	
	kMaxLinearInterpolationForMinMagMip,

	kMaxAnisotropicInterplationForMinMagMip,
};

enum class TextureAddressType {
	kAddressWrap = 0,
	kAddressMirror,
	kAddressClamp,
	kAddressBorder,
	kAddressMirrorOnce
};

enum class ComparsionType {
	ComparsionNever = 0,
	ComparsionLess,
	ComparsionEqual,
	ComparsionLessEqual,
	ComparsionGreater,
	ComparsionNotEqual,
	ComparsionGreaterEqual,
	ComparsionAlways
};

class SamplerState {
public:
	SamplerState();

	explicit SamplerState(const SamplerState& copy);

	const SamplerState& operator=(const SamplerState& copy);

	~SamplerState();

public:
	void SetFilter(const FilterType filter_type);
	
	void SetAddressUVW(
		const TextureAddressType u_texture_address_type,
		const TextureAddressType v_texture_address_type,
		const TextureAddressType w_texture_address_type
		);
	
	void SetMipLODBiasAndMaxAnisotropy(const float mip_lod_bias, const UINT max_anisotropy);
	
	void SetComparisonFunc(const ComparsionType comparsion_type);
	
	void SetBorderColorArry(float x, float y, float z, float w);
	
	void SetMinAndMaxLod(float min_lod, float max_lod);

	void SetSamplerStateDesc(D3D11_SAMPLER_DESC *sampler_state_desc);

	void SetNumDesc(int num);

	const D3D11_SAMPLER_DESC* GetSamplerStateDesc();
	
private:
	int num_desc_ = 0;

	D3D11_SAMPLER_DESC sampler_state_desc_;
};

#endif // !GRAPHICS_HEADER_SAMPLER_STATE_H
