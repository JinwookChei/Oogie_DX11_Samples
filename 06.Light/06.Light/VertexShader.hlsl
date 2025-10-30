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
    float3 normal : NORMAL;
    float2 UV : TEXCOORD;
    float4 tangent : TANGENT;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    float4 worldPosition = mul(float4(input.position, 1.0f), World); // 월드 포지션 ( 원점을 기준으로 얼마만큼 떨어져있나 )
    float4 viewPosition = mul(worldPosition, View); // 뷰 포지션 ( 카메라를 기준으로 둔 포지션 ) ( 카메라 기준이란? 카메라의 포지션을 0, 0, 0 으로 본다 ) ( 카메라 기준이란? 카메라를 원점으로 만든다. )
    output.pos = mul(viewPosition, Projection);
    
    output.color = input.color;
    
    float3x3 normalMatrix = (float3x3) World;
    output.normal = mul(input.normal, normalMatrix);
    output.worldPos = worldPosition.xyz;
    
    return output;
}

// Debug Normal
//PS_INPUT main(VS_INPUT input)
//{
//    PS_INPUT output = (PS_INPUT) 0;
    
//    float4 worldPosition = mul(float4(input.position, 1.0f), World); // 월드 포지션 ( 원점을 기준으로 얼마만큼 떨어져있나 )
//    float4 viewPosition = mul(worldPosition, View); // 뷰 포지션 ( 카메라를 기준으로 둔 포지션 ) ( 카메라 기준이란? 카메라의 포지션을 0, 0, 0 으로 본다 ) ( 카메라 기준이란? 카메라를 원점으로 만든다. )
//    output.pos = mul(viewPosition, Projection);
//    output.color = input.color;
    
//    float3x3 normalMatrix = (float3x3) World;
//    output.normal = input.normal;
//    output.worldPos = worldPosition.xyz;
    
//    return output;
//}