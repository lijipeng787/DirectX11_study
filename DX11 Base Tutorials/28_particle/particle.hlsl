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
	float4 color : COLOR;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

PixelInputType ParticleVertexShader(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    	
	output.tex = input.tex;
    
	// Store the particle color for the pixel shader. 
    output.color = input.color;

    return output;
}

Texture2D shaderTexture;
SamplerState SampleType;

float4 ParticlePixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float4 finalColor;

    textureColor = shaderTexture.Sample(SampleType, input.tex);

	// Combine the texture color and the particle color to get the final color result.
    finalColor = textureColor * input.color;

    return finalColor;
}