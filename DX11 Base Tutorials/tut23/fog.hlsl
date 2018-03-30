cbuffer PerFrameBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer FogBuffer
{
	float fogStart;
	float fogEnd;
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
	float fogFactor : FOG;
};

PixelInputType FogVertexShader(VertexInputType input)
{
    PixelInputType output;
	float4 cameraPosition;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
	
	output.tex = input.tex;
    
    // Calculate the camera position.
    cameraPosition = mul(input.position, worldMatrix);
    cameraPosition = mul(cameraPosition, viewMatrix);

    // Calculate linear fog.    
    output.fogFactor = saturate((fogEnd - cameraPosition.z) / (fogEnd - fogStart));

    return output;
}

Texture2D shaderTexture;
SamplerState SampleType;

float4 FogPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
    float4 fogColor;
    float4 finalColor;

    // Sample the texture pixel at this location.
    textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // Set the color of the fog to grey.
    fogColor = float4(0.5f, 0.5f, 0.5f, 1.0f);

    // Calculate the final color using the fog effect equation.
    finalColor = input.fogFactor * textureColor + (1.0 - input.fogFactor) * fogColor;

	return finalColor;
}
