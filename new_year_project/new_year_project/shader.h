#pragma once

#ifndef GRAPHICS_HEADER_SHADER_H

#define GRAPHICS_HEADER_SHADER_H

#include <memory>
#include <d3d11.h>

typedef std::shared_ptr<ID3D10Blob*> D3D10BlobPtr;

typedef std::shared_ptr<ID3D11VertexShader*> VertexShaderPtr;

typedef std::shared_ptr<ID3D11PixelShader*> PixelShaderPtr;

enum class ShaderType {
	kVertexShader = 0,
	kPixelShader,
	kGeometryShader
};

class Shader {
public:
	Shader() :compiled_shader_(std::make_shared<ID3D10Blob*>()) {}

	explicit Shader(const Shader& copy);

	Shader& operator=(const Shader& copy);

	virtual ~Shader();

public:
	virtual const ShaderType GetType()const = 0;

	void set_valid();

	bool isValid();

	D3D10BlobPtr get_compiled_shader();

	const D3D10BlobPtr get_compiled_shader()const;

protected:
	D3D10BlobPtr compiled_shader_;

	bool valid_ = false;
};

class VertexShader :public Shader {
public:
	VertexShader() :vertex_shader_(std::make_shared<ID3D11VertexShader*>()) {}

	explicit VertexShader(const VertexShader& copy);

	VertexShader& operator=(const VertexShader& copy);

	virtual ~VertexShader();

	VertexShaderPtr get_vertex_shader();

	const VertexShaderPtr get_vertex_shader()const;

public:
	virtual const ShaderType GetType()const override;

private:
	VertexShaderPtr vertex_shader_;
};

class PixelShader :public Shader {
public:
	PixelShader() :pixel_shader_(std::make_shared<ID3D11PixelShader*>()) {}

	explicit PixelShader(const PixelShader& copy);

	PixelShader& operator=(const PixelShader& copy);

	virtual ~PixelShader();

public:
	virtual const ShaderType GetType()const override;

	PixelShaderPtr get_pixel_shader();

	const PixelShaderPtr get_pixel_shader()const;

private:
	PixelShaderPtr pixel_shader_;
};

#endif // !GRAPHICS_HEADER_SHADER_H
