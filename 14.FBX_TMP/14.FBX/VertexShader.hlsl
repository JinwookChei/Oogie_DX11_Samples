cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    
    float4 lightColor;
    float4 ambientColor;
    
    float3 spotPosition;
    float spotRange;
    float3 spotDirection;
    float spotAngle;
}

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float3 worldPos : TEXCOORD0;
    float2 uv : TEXCOORD1;
    float3x3 TBN : TEXCOORD2;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    float4 worldPosition = mul(float4(input.position, 1.0f), World); 
    float4 viewPosition = mul(worldPosition, View); 
    output.position = mul(viewPosition, Projection);
    output.color = input.color;
    
    float3 N = normalize(mul(input.normal.xyz, (float3x3) World));
    float3 T = normalize(mul(input.tangent.xyz, (float3x3) World));
    float3 B = normalize(cross(N, T) * input.tangent.w);
    
    float3x3 normalMatrix = (float3x3) World;
    output.normal = mul(input.normal.xyz, normalMatrix);
    output.worldPos = worldPosition.xyz;
    output.uv = input.uv;
    output.TBN = float3x3(T, B, N);
    
    return output;
}
