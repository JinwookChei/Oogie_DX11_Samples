Texture2D g_Texture : register(t0);
Texture2D g_NormalMap : register(t1);
SamplerState g_Sampler : register(s0);

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

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD0;
    float2 uv : TEXCOORD1;
    float3x3 TBN : TEXCOORD2;
};

float3 UnpackNormal(float4 c)
{
    return normalize(c.xyz * 2.0f - 1.0f);
}


//After Use Normal Mapping
float4 main(PS_INPUT input) : SV_TARGET
{
    float4 colorTex = g_Texture.Sample(g_Sampler, input.uv);
    float4 normalTex = g_NormalMap.Sample(g_Sampler, input.uv);
    float3 unpackNormalTex = UnpackNormal(normalTex);
    float3 worldNormal = normalize(mul(unpackNormalTex, input.TBN));
    
    float3 N = normalize(worldNormal);
    float3 L = normalize(spotPosition - input.worldPos);
    float3 S = normalize(-spotDirection);
    
    float spotCos = dot(L, S);
    float spotEffect = smoothstep(spotAngle, spotAngle + 0.05, spotCos);
    
    float dist = length(spotPosition - input.worldPos);
    float att = saturate(1.0f - dist / spotRange);
    
    float diffuse = saturate(dot(N, L)) * spotEffect * att;
    float diffuseColor = diffuse * lightColor;
    
    float4 finalColor = colorTex * (diffuseColor + ambientColor);
    finalColor.a = colorTex.a; // 텍스처 알파 사용
    
    return finalColor;
}
