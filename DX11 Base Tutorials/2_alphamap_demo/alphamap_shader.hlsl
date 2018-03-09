cbuffer MatrixBuffer {
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInputType {
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

PixelInputType AlphaMapVertexShader(VertexInputType input) {


	input.position.w = 1.0f;

	PixelInputType output;
	
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	return output;
}

Texture2D shaderTextures[3];
SamplerState SampleType;

float4 AlphaMapPixelShader(PixelInputType input) : SV_TARGET
{
	// Get the pixel color from the first texture.
	float4 color1 = shaderTextures[0].Sample(SampleType, input.tex);
	
	// Get the pixel color from the second texture.
	float4 color2 = shaderTextures[1].Sample(SampleType, input.tex);
	
	// Get the alpha value from the alpha map texture.
	float4 alphaValue = shaderTextures[2].Sample(SampleType, input.tex);
	
	// Combine the two textures based on the alpha value.
	float4 blendColor = (alphaValue * color1) + ((1.0 - alphaValue) * color2);
	
	// Saturate the final color value.
	blendColor = saturate(blendColor);
	
	return blendColor;
}
