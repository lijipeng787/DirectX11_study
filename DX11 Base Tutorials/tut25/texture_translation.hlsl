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

PixelInputType TranslateVertexShader(VertexInputType input)
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

cbuffer TranslationBuffer
{
	float textureTranslation;
};

float4 TranslatePixelShader(PixelInputType input) : SV_TARGET
{
	// Translate the position where we sample the pixel from.
	input.tex.x += textureTranslation;

	return shaderTexture.Sample(SampleType, input.tex);
}
