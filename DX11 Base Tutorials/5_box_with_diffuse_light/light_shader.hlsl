cbuffer MatrixBuffer{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInputType{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

PixelInputType LightVertexShader(VertexInputType input) {

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

cbuffer LightBuffer {
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
	float padding;
};

float4 LightPixelShader(PixelInputType input) : SV_TARGET{

	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 color;

	textureColor = shaderTexture.Sample(SampleType, input.tex);
	
	color = ambientColor;
	
	lightDir = -lightDirection;
	
	lightIntensity = dot(input.normal, lightDir);
	
	if (lightIntensity > 0.0f) {
		color += (diffuseColor * lightIntensity);
	}
	
	color = saturate(color);
	
	color = color * textureColor;
	
	return color;
}