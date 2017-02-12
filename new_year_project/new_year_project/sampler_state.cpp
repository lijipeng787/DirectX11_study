#include "sampler_state.h"

SamplerState::SamplerState(){
	sampler_state_desc_.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_state_desc_.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_state_desc_.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_state_desc_.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_state_desc_.MipLODBias = 0.0f;
	sampler_state_desc_.MaxAnisotropy = 1;
	sampler_state_desc_.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_state_desc_.BorderColor[0] = 0;
	sampler_state_desc_.BorderColor[1] = 0;
	sampler_state_desc_.BorderColor[2] = 0;
	sampler_state_desc_.BorderColor[3] = 0;
	sampler_state_desc_.MinLOD = 0;
	sampler_state_desc_.MaxLOD = D3D11_FLOAT32_MAX;
}

SamplerState::SamplerState(const SamplerState & copy){
	if (this != &copy) {
		sampler_state_desc_.Filter = copy.sampler_state_desc_.Filter;
		sampler_state_desc_.AddressU = copy.sampler_state_desc_.AddressU;
		sampler_state_desc_.AddressV = copy.sampler_state_desc_.AddressV;
		sampler_state_desc_.AddressW = copy.sampler_state_desc_.AddressW;
		sampler_state_desc_.MipLODBias = copy.sampler_state_desc_.MipLODBias;
		sampler_state_desc_.MaxAnisotropy = copy.sampler_state_desc_.MaxAnisotropy;
		sampler_state_desc_.ComparisonFunc = copy.sampler_state_desc_.ComparisonFunc;
		sampler_state_desc_.BorderColor[0] = copy.sampler_state_desc_.BorderColor[0];
		sampler_state_desc_.BorderColor[1] = copy.sampler_state_desc_.BorderColor[1];
		sampler_state_desc_.BorderColor[2] = copy.sampler_state_desc_.BorderColor[2];
		sampler_state_desc_.BorderColor[3] = copy.sampler_state_desc_.BorderColor[3];
		sampler_state_desc_.MinLOD = copy.sampler_state_desc_.MinLOD;
		sampler_state_desc_.MaxLOD = copy.sampler_state_desc_.MaxLOD;
	}
}

const SamplerState & SamplerState::operator=(const SamplerState & copy){
	if (this != &copy) {
		sampler_state_desc_.Filter = copy.sampler_state_desc_.Filter;
		sampler_state_desc_.AddressU = copy.sampler_state_desc_.AddressU;
		sampler_state_desc_.AddressV = copy.sampler_state_desc_.AddressV;
		sampler_state_desc_.AddressW = copy.sampler_state_desc_.AddressW;
		sampler_state_desc_.MipLODBias = copy.sampler_state_desc_.MipLODBias;
		sampler_state_desc_.MaxAnisotropy = copy.sampler_state_desc_.MaxAnisotropy;
		sampler_state_desc_.ComparisonFunc = copy.sampler_state_desc_.ComparisonFunc;
		sampler_state_desc_.BorderColor[0] = copy.sampler_state_desc_.BorderColor[0];
		sampler_state_desc_.BorderColor[1] = copy.sampler_state_desc_.BorderColor[1];
		sampler_state_desc_.BorderColor[2] = copy.sampler_state_desc_.BorderColor[2];
		sampler_state_desc_.BorderColor[3] = copy.sampler_state_desc_.BorderColor[3];
		sampler_state_desc_.MinLOD = copy.sampler_state_desc_.MinLOD;
		sampler_state_desc_.MaxLOD = copy.sampler_state_desc_.MaxLOD;
	}
	return *this;
}

SamplerState::~SamplerState(){
}

void SamplerState::SetFilter(const FilterType filter_type){
	
	switch (filter_type) {
	case FilterType::kPointSamplingForMinMagMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	}
	case FilterType::kPointSamplingForMinMagLinearInterPolationForMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		break;
	}
	case FilterType::kPoingSamplingForMinMipLinearInterpolationForMag: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	}
	case FilterType::kLinearInterPolationForMagMipPointSamplingForMin: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		break;
	}
	case FilterType::kPointSamplingForMagMipLinearInterpolationForMin: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		break;
	}
	case FilterType::kLinearInterpolationForMinMipPointSamplingForMag: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	}
	case FilterType::kLinearInterpolationForMinMagPointSamplingForMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	}
	case FilterType::kLinearInterpolationForMinMagMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	}
	case FilterType::kAnisotropicInterplationForMinMagMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_ANISOTROPIC;
		break;
	}
	case FilterType::kPointSamplingForMinMagMipCompare: {
		sampler_state_desc_.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		break;
	}
	case FilterType::kPointSamplingForMinMipLinearInterpolationForMagCompare: {
		sampler_state_desc_.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		break;
	}
	case FilterType::kPointSamplingForMinMapLinearInterpolationForMagCompare: {
		sampler_state_desc_.Filter = D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	}
	case FilterType::kLinearInterpolationForMagMipPointSamplingForMinCompare: {
		sampler_state_desc_.Filter = D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		break;
	}
	case FilterType::kPointSamplingForMagMipLinearInterpolationForMinCompare: {
		sampler_state_desc_.Filter = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		break;
	}
	case FilterType::kLinearInterpolationForMinMipPointSamplingForMagCompare: {
		sampler_state_desc_.Filter = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	}
	case FilterType::kLinearInterpolationForMinMagPointSamplingFormipCompare: {
		sampler_state_desc_.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		break;
	}
	case FilterType::kLinearInterpolationForMinMagMipCompare: {
		sampler_state_desc_.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		break;
	}
	case FilterType::kAnisotropicInterplationForMinMagMipCompare: {
		sampler_state_desc_.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;
		break;
	}
	case FilterType::kMinPointSamplingForMinMagMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	}
	case FilterType::kMinPointSamplingForMinMagLinearInterPolationForMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		break;
	}
	case FilterType::kMinPoingSamplingForMinMipLinearInterpolationForMag: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	}
	case FilterType::kMinLinearInterPolationForMagMipPointSamplingForMin: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		break;
	}
	case FilterType::kMinPointSamplingForMagMipLinearInterpolationForMin: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		break;
	}
	case FilterType::kMinLinearInterpolationForMinMipPointSamplingForMag: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	}
	case FilterType::kMinLinearInterpolationForMinMagPointSamplingForMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	}
	case FilterType::kMinLinearInterpolationForMinMagMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	}
	case FilterType::kMinAnisotropicInterplationForMinMagMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MINIMUM_ANISOTROPIC;
		break;
	}
	case FilterType::kMaxPointSamplingForMinMagMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
		break;
	}
	case FilterType::kMaxPointSamplingForMinMagLinearInterPolationForMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
		break;
	}
	case FilterType::kMaxPoingSamplingForMinMipLinearInterpolationForMag: {
		sampler_state_desc_.Filter = D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	}
	case FilterType::kMaxLinearInterPolationForMagMipPointSamplingForMin: {
		sampler_state_desc_.Filter = D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
		break;
	}
	case FilterType::kMaxPointSamplingForMagMipLinearInterpolationForMin: {
		sampler_state_desc_.Filter = D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
		break;
	}
	case FilterType::kMaxLinearInterpolationForMinMipPointSamplingForMag: {
		sampler_state_desc_.Filter = D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	}
	case FilterType::kMaxLinearInterpolationForMinMagPointSamplingForMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
		break;
	}
	case FilterType::kMaxLinearInterpolationForMinMagMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
		break;
	}
	case FilterType::kMaxAnisotropicInterplationForMinMagMip: {
		sampler_state_desc_.Filter = D3D11_FILTER_MAXIMUM_ANISOTROPIC;
		break;
	}
	}
}

void SamplerState::SetAddressUVW(
	const TextureAddressType u_texture_address_type,
	const TextureAddressType v_texture_address_type,
	const TextureAddressType w_texture_address_type
	)
{
	switch (u_texture_address_type){
	case TextureAddressType::kAddressWrap: {
		sampler_state_desc_.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	}
	case TextureAddressType::kAddressMirror: {
		sampler_state_desc_.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	}
	case TextureAddressType::kAddressClamp: {
		sampler_state_desc_.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	}
	case TextureAddressType::kAddressBorder: {
		sampler_state_desc_.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		break;
	}
	case TextureAddressType::kAddressMirrorOnce: {
		sampler_state_desc_.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		break;
	}
	default:
		break;
	}

	switch (v_texture_address_type){
	case TextureAddressType::kAddressWrap: {
		sampler_state_desc_.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	}
	case TextureAddressType::kAddressMirror: {
		sampler_state_desc_.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	}
	case TextureAddressType::kAddressClamp: {
		sampler_state_desc_.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	}
	case TextureAddressType::kAddressBorder: {
		sampler_state_desc_.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		break;
	}
	case TextureAddressType::kAddressMirrorOnce: {
		sampler_state_desc_.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		break;
	}
	default:
		break;
	}

	switch (w_texture_address_type){
	case TextureAddressType::kAddressWrap: {
		sampler_state_desc_.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	}
	case TextureAddressType::kAddressMirror: {
		sampler_state_desc_.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	}
	case TextureAddressType::kAddressClamp: {
		sampler_state_desc_.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	}
	case TextureAddressType::kAddressBorder: {
		sampler_state_desc_.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		break;
	}
	case TextureAddressType::kAddressMirrorOnce: {
		sampler_state_desc_.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		break;
	}
	default:
		break;
	}
}

void SamplerState::SetMipLODBiasAndMaxAnisotropy(
	const float mip_lod_bias, 
	const UINT max_anisotropy
	){
	sampler_state_desc_.MipLODBias = mip_lod_bias;
	sampler_state_desc_.MaxAnisotropy = max_anisotropy;
}

void SamplerState::SetComparisonFunc(const ComparsionType comparsion_type){
	switch (comparsion_type){
	case ComparsionType::ComparsionNever: {
		sampler_state_desc_.ComparisonFunc = D3D11_COMPARISON_NEVER;
		break;
	}
	case ComparsionType::ComparsionLess: {
		sampler_state_desc_.ComparisonFunc = D3D11_COMPARISON_LESS;
		break; 
	}
	case ComparsionType::ComparsionEqual: {
		sampler_state_desc_.ComparisonFunc = D3D11_COMPARISON_EQUAL;
		break; }
	case ComparsionType::ComparsionLessEqual: {
		sampler_state_desc_.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		break; }
	case ComparsionType::ComparsionGreater: {
		sampler_state_desc_.ComparisonFunc = D3D11_COMPARISON_GREATER;
		break; }
	case ComparsionType::ComparsionNotEqual: {
		sampler_state_desc_.ComparisonFunc = D3D11_COMPARISON_NOT_EQUAL;
		break; }
	case ComparsionType::ComparsionGreaterEqual: {
		sampler_state_desc_.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
		break; }
	case ComparsionType::ComparsionAlways: {
		sampler_state_desc_.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		break; }
	default:
		break;
	}
}

void SamplerState::SetBorderColorArry(float x, float y, float z, float w){
	sampler_state_desc_.BorderColor[0] = x;
	sampler_state_desc_.BorderColor[1] = y;
	sampler_state_desc_.BorderColor[2] = z;
	sampler_state_desc_.BorderColor[3] = w;

}

void SamplerState::SetMinAndMaxLod(float min_lod, float max_lod){
	sampler_state_desc_.MinLOD = min_lod;
	sampler_state_desc_.MaxLOD = max_lod;
}

void SamplerState::SetSamplerStateDesc(D3D11_SAMPLER_DESC * sampler_state_desc){
	sampler_state_desc_ = *sampler_state_desc;
}

void SamplerState::SetNumDesc(int num){
	num_desc_ = num;
}

const D3D11_SAMPLER_DESC * SamplerState::GetSamplerStateDesc(){
	return &sampler_state_desc_;
}
