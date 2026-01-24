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

cbuffer AnimBuffer : register(b1)
{
    float4x4 g_BoneTransforms[114];
}

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
    uint4 boneIndices : BLENDINDICES;
    float4 blendWeights : BLENDWEIGHT;
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
    
    float4x4 skinTransform = 0;
    skinTransform += mul(input.blendWeights.x, g_BoneTransforms[(uint) input.boneIndices.x]);
    skinTransform += mul(input.blendWeights.y, g_BoneTransforms[(uint) input.boneIndices.y]);
    skinTransform += mul(input.blendWeights.z, g_BoneTransforms[(uint) input.boneIndices.z]);
    skinTransform += mul(input.blendWeights.w, g_BoneTransforms[(uint) input.boneIndices.w]);

    float4 skinnedPos = mul(float4(input.position, 1.0f), skinTransform);
    float4 worldPos = mul(skinnedPos, World);
    float4 viewPos = mul(worldPos, View);
    output.position = mul(viewPos, Projection);
    output.worldPos = worldPos.xyz;
    output.color = input.color;
    output.uv = input.uv;
    
    
    float3 N = normalize(mul(input.normal.xyz, (float3x3) skinTransform));
    float3 T = normalize(mul(input.tangent.xyz, (float3x3) skinTransform));

    // World (scale 없음 가정)
    N = normalize(mul(N, (float3x3) World));
    T = normalize(mul(T, (float3x3) World));

    // Gram-Schmidt
    T = normalize(T - N * dot(N, T));
    float3 B = cross(N, T) * input.tangent.w;

    output.normal = N;
    output.TBN = float3x3(T, B, N);
    return output;
}


//PS_INPUT main(VS_INPUT input)
//{
//    PS_INPUT output = (PS_INPUT) 0;
    
//    float4 skinnedPos = (float4) 0.0f;
//    for (int i = 0; i < 4; ++i)
//    {
//        skinnedPos += input.blendWeights[i] * mul(float4(input.position, 1.0f), g_BoneTransforms[input.boneIndices[i]]);
//    }
    
//    float4 worldPos = mul(skinnedPos, World);
//    float4 viewPos = mul(worldPos, View);
//    output.position = mul(viewPos, Projection);
//    output.worldPos = worldPos.xyz;
//    output.color = input.color;
//    output.uv = input.uv;
    
//    return output;
//}



//PS_INPUT main(VS_INPUT input)
//{
//    PS_INPUT output = (PS_INPUT) 0;
    
//    float4 worldPosition = mul(float4(input.position, 1.0f), World);
//    float4 viewPosition = mul(worldPosition, View);
//    output.position = mul(viewPosition, Projection);
//    output.color = input.color;
    
//    float3 N = normalize(mul(input.normal.xyz, (float3x3) World));
//    float3 T = normalize(mul(input.tangent.xyz, (float3x3) World));
//    float3 B = normalize(cross(N, T) * input.tangent.w);
    
//    float3x3 normalMatrix = (float3x3) World;
//    output.normal = mul(input.normal.xyz, normalMatrix);
//    output.worldPos = worldPosition.xyz;
//    output.uv = input.uv;
//    output.TBN = float3x3(T, B, N);
    
//    return output;
//}