cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer LightBuffer2
{
    float3 lightPosition;
	float padding;
};

cbuffer ReflectionBuffer
{
	matrix reflectionMatrix;
	float reflectionBlend;
	float3 reflectionPadding;
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
    float4 viewPosition : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
    float4 reflectionPosition : TEXCOORD3;
    float reflectionFactor : TEXCOORD4;
};

PixelInputType SoftShadowVertexShader(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the position of the vertice as viewed by the camera in a separate variable.
    output.viewPosition = output.position;

    output.tex = input.tex;
    
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    output.normal = normalize(output.normal);

    // Calculate the position of the vertex in the world.
    worldPosition = mul(input.position, worldMatrix);

    // Determine the light position based on the position of the light and the position of the vertex in the world.
    output.lightPos = lightPosition.xyz - worldPosition.xyz;

    // Normalize the light position vector.
    output.lightPos = normalize(output.lightPos);

	// Calculate reflection projection coordinates
	matrix reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
	reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);
	output.reflectionPosition = mul(input.position, reflectProjectWorld);
	output.reflectionFactor = reflectionBlend;

	return output;
}