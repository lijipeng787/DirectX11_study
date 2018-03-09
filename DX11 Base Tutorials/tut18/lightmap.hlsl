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

PixelInputType LightMapVertexShader(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	output.tex = input.tex;
    
    return output;
}

Texture2D shaderTextures[2];
SamplerState SampleType;

float4 LightMapPixelShader(PixelInputType input) : SV_TARGET
{
	float4 color;
    float4 lightColor;
    float4 finalColor;

    color = shaderTextures[0].Sample(SampleType, input.tex);

    // Get the pixel color from the light map.
    lightColor = shaderTextures[1].Sample(SampleType, input.tex);

    // Blend the two pixels together.
    finalColor = color * lightColor;

	return finalColor;
}
