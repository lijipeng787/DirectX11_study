#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_

#include "fontclass.h"
#include "fontshaderclass.h"

class Text {

private:
	struct ConstantBufferType {
		DirectX::XMMATRIX world_;
		DirectX::XMMATRIX base_view_;
		DirectX::XMMATRIX orthonality_;
	};

	struct PixelBufferType {
		DirectX::XMFLOAT4 pixel_color_;
	};

	struct SentenceType {
		D3d12ResourcePtr vertex_buffer_ = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_ = {};
		D3d12ResourcePtr index_buffer_ = nullptr;
		D3D12_INDEX_BUFFER_VIEW index_buffer_view_ = {};

		int vertex_count_;
		int index_count_;
		int max_length_;
		float red_, green_, blue_;
	};

	struct VertexType {
		DirectX::XMFLOAT3 position_;
		DirectX::XMFLOAT2 texture_;
	};

public:
	Text() {}

	explicit Text(const Text& copy) {}

	~Text() {/* todo deallocate sentence vector memory */ }

public:
	bool Initialize(int screenWidth, int screenHeight, const DirectX::XMMATRIX& baseViewMatrix);

	bool LoadFont(WCHAR *font_data, WCHAR *font_texture) {

		m_font_ = std::make_shared<Font>();
		if (!m_font_) {
			return false;
		}

		if (CHECK(m_font_->Initialize(font_data, font_texture))) {
			return false;
		}
		return true;
	}

	void SetVertexShader(const D3D12_SHADER_BYTECODE& verte_shader_bitecode) {
		vertex_shader_bitecode_ = verte_shader_bitecode;
	}

	void SetPixelShader(const D3D12_SHADER_BYTECODE& pixel_shader_bitcode) {
		pixel_shader_bitecode_ = pixel_shader_bitcode;
	}

	bool InitializeRootSignature();

	bool InitializeGraphicsPipelineState();

	bool InitializeConstantBuffer();

	bool InitializeSentence(SentenceType** sentence, int max_length);

	const UINT GetIndexCount(int index)const { 
		return sentence_vector_.at(index)->index_count_; 
	}

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView(int index)const { 
		return sentence_vector_.at(index)->vertex_buffer_view_;
	}

	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView(int index)const {
		return sentence_vector_.at(index)->index_buffer_view_;
	}

	const int GetSentenceCount()const { return static_cast<int>(sentence_vector_.size()); }

	RootSignaturePtr GetRootSignature()const { return root_signature_; }

	PipelineStateObjectPtr GetNormalPso()const { return pipeline_state_object_; }

	PipelineStateObjectPtr GetBlendEnabledPso()const { return blend_enabled_pso_; }

	DescriptorHeapPtr GetShaderRescourceView()const { return m_font_->GetTexture(); }

	D3d12ResourcePtr GetMatrixConstantBuffer()const { return matrix_constant_buffer_; }

	D3d12ResourcePtr GetPixelConstantBuffer()const { return pixel_color_constant_buffer_; }

	bool UpdateSentenceVertexBuffer(
		SentenceType* sentence,
		WCHAR* text,
		int positionX, int positionY,
		float red, float green, float blue
		);

	bool UpdateMatrixConstant(
		DirectX::XMMATRIX& world,
		DirectX::XMMATRIX& base_view,
		DirectX::XMMATRIX& orthonality
		);

	bool UpdateLightConstant(
		DirectX::XMFLOAT4& pixel_color
		);

private:
	D3d12ResourcePtr matrix_constant_buffer_ = nullptr;

	ConstantBufferType matrix_constant_data_ = {};

	D3d12ResourcePtr pixel_color_constant_buffer_ = nullptr;

	PixelBufferType pixel_color_constant_data_ = {};

	RootSignaturePtr root_signature_ = nullptr;

	PipelineStateObjectPtr pipeline_state_object_ = nullptr;

	PipelineStateObjectPtr blend_enabled_pso_ = nullptr;

	D3D12_SHADER_BYTECODE vertex_shader_bitecode_ = {};

	D3D12_SHADER_BYTECODE pixel_shader_bitecode_ = {};

private:
	std::shared_ptr<Font> m_font_ = nullptr;

	int m_screen_width, m_screen_height;

	DirectX::XMMATRIX m_base_view_matrix;

	std::vector<SentenceType*> sentence_vector_;
};

#endif