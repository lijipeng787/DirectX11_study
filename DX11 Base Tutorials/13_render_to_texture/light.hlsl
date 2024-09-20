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
   	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;
	
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	output.tex = input.tex;
	
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	
	output.normal = normalize(output.normal);
	
    return output;
}

Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
	float4 diffuseColor;
	float3 lightDirection;
};

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;
	
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	
	lightDir = -lightDirection;

	lightIntensity = saturate(dot(input.normal, lightDir));

	// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
	color = saturate(diffuseColor * lightIntensity);

	color = color * textureColor;
	
	return color;
}