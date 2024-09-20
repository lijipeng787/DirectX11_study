cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
	matrix lightViewMatrix2;
	matrix lightProjectionMatrix2;
};

cbuffer LightBuffer2
{
    float3 lightPosition;
	float padding1;
    float3 lightPosition2;
	float padding2;
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
    float4 lightViewPosition : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
    float4 lightViewPosition2 : TEXCOORD3;
	float3 lightPos2 : TEXCOORD4;
};

PixelInputType ShadowVertexShader(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the position of the vertice as viewed by the light source.
    output.lightViewPosition = mul(input.position, worldMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightViewMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightProjectionMatrix);

	// Calculate the position of the vertice as viewed by the second light source.
    output.lightViewPosition2 = mul(input.position, worldMatrix);
    output.lightViewPosition2 = mul(output.lightViewPosition2, lightViewMatrix2);
    output.lightViewPosition2 = mul(output.lightViewPosition2, lightProjectionMatrix2);

    output.tex = input.tex;
    
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    output.normal = normalize(output.normal);

    // Calculate the position of the vertex in the world.
    worldPosition = mul(input.position, worldMatrix);

    // Determine the light position based on the position of the light and the position of the vertex in the world.
    output.lightPos = lightPosition.xyz - worldPosition.xyz;

    // Normalize the light position vector.
    output.lightPos = normalize(output.lightPos);

    // Determine the second light position based on the position of the light and the position of the vertex in the world.
    output.lightPos2 = lightPosition2.xyz - worldPosition.xyz;

    // Normalize the second light position vector.
    output.lightPos2 = normalize(output.lightPos2);

	return output;
}