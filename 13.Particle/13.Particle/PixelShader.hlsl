Texture2D gParticleTex : register(t0);
SamplerState gSampler : register(s0);

cbuffer constantBuffer : register(b0)
{
    float4x4 gViewProj;
    float3 gCameraRight;
    float gStartSize;
    
    float3 gCameraUp;
    float gEndSize;
    
    float4 gStartColor;
    float4 gEndColor;
}

struct GS_OUTPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float age : AGE;
};

float4 main(GS_OUTPUT input) : SV_TARGET
{
    float4 texColor = gParticleTex.Sample(gSampler, input.uv);
    
    float4 color = lerp(gStartColor, gEndColor, input.age);
    float alpha = saturate(1.0f - input.age);
    color.a *= alpha;

    return texColor * color;
}