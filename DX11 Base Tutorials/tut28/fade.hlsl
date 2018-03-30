cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
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
};

PixelInputType FadeVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
	
	output.tex = input.tex;
    
    return output;
}

Texture2D shaderTexture;
SamplerState SampleType;

cbuffer FadeBuffer
{
    float fadeAmount;
    float3 padding;
};

float4 FadePixelShader(PixelInputType input) : SV_TARGET
{
	float4 color;

    // Sample the texture pixel at this location.
    color = shaderTexture.Sample(SampleType, input.tex);

	// Reduce the color brightness to the current fade percentage.
    color = color * fadeAmount;

	return color;
}