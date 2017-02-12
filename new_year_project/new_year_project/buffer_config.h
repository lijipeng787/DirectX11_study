#pragma once

#ifndef GRAPHICS_HEADER_BUFFER_CONFIG_H

#define GRAPHICS_HEADER_BUFFER_CONFIG_H

#include "common.h"

enum class BufferUseage {
	kUsageDefault = 0,
	kUsageImmutable,
	kUsageDynamic,
	kUsageStaging
};

class BufferConfig {
public:
	BufferConfig();

	explicit BufferConfig(const BufferConfig& copy);

	BufferConfig& operator=(const BufferConfig& copy);

	virtual ~BufferConfig();

public:
	void SetByteWidth(UINT byte_width);
	
	void SetBufferUsage(BufferUseage usage);

	virtual void SetBindType(UINT bind_type);
	
	void SetCpuAccessType(UINT cpu_access_type);
	
	void SetMiscFlags(UINT misc_flags);

	void SetStructreByteStride(UINT structre_byte_stride);

	const D3D11_BUFFER_DESC* get_buffer_desc()const;

protected:
	D3D11_BUFFER_DESC buffer_desc;
};

class VertexBufferConfig :public BufferConfig {
public:
	VertexBufferConfig();

	VertexBufferConfig(const VertexBufferConfig& copy);

	VertexBufferConfig& operator=(const VertexBufferConfig& copy);

	virtual ~VertexBufferConfig();

public:
	virtual void SetBindType(UINT bind_type)override;
};

class IndexBufferConfig :public BufferConfig {
public: 
	IndexBufferConfig();

	IndexBufferConfig(const IndexBufferConfig& copy);

	IndexBufferConfig& operator=(const IndexBufferConfig& copy);

	virtual ~IndexBufferConfig();

public:
	virtual void SetBindType(UINT bind_type)override;
};

class ConstantBufferConfig :public BufferConfig {
public:
	ConstantBufferConfig();

	ConstantBufferConfig(const ConstantBufferConfig& copy);

	ConstantBufferConfig& operator=(const ConstantBufferConfig& copy);

	virtual ~ConstantBufferConfig();

public:
	virtual void SetBindType(UINT bind_type)override;
};

#endif // !GRAPHICS_HEADER_BUFFER_CONFIG_H
