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

PixelInputType MultiTextureVertexShader(VertexInputType input)
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

float4 MultiTexturePixelShader(PixelInputType input) : SV_TARGET
{
	float4 color1;
    float4 color2;
    float4 blendColor;

    // Get the pixel color from the first texture.
    color1 = shaderTextures[0].Sample(SampleType, input.tex);

    // Get the pixel color from the second texture.
    color2 = shaderTextures[1].Sample(SampleType, input.tex);

    // Blend the two pixels together and multiply by the gamma value.
    blendColor = color1 * color2 * 2.0;
    
    // Saturate the final color.
    blendColor = saturate(blendColor);

    return blendColor;
}
