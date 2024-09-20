cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer ClipPlaneBuffer
{
	float4 clipPlane;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float clip : SV_ClipDistance0;
};

PixelInputType ClipPlaneVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;
	
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	output.tex = input.tex;

    // Set the clipping plane.
    output.clip = dot(mul(input.position, worldMatrix), clipPlane);

    return output;
}

Texture2D shaderTexture;
SamplerState SampleType;

float4 ClipPlanePixelShader(PixelInputType input) : SV_TARGET
{
	return shaderTexture.Sample(SampleType, input.tex);
}
