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

struct VS_OUTPUT
{
    float3 pos : POSITION;
    float age : AGE;
};

struct GS_OUTPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float age : AGE;
};


[maxvertexcount(4)]
void main(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> output)
{
    float3 pos = input[0].pos;
    float age = input[0].age;
    
    float size = lerp(gStartSize, gEndSize, age);
    
    float3 right = normalize(gCameraRight) * size * 0.5f;
    float3 up = normalize(gCameraUp) * size * 0.5f;
    
    float3 p0 = pos - right - up;
    float3 p1 = pos - right + up;
    float3 p2 = pos + right - up;
    float3 p3 = pos + right + up;
    
    GS_OUTPUT v;
    v.age = age;
    
    v.pos = mul(float4(p0, 1.0f), gViewProj);
    v.uv = float2(0.0f, 1.0f);
    output.Append(v);
    
    v.pos = mul(float4(p1, 1.0f), gViewProj);
    v.uv = float2(0.0f, 0.0f);
    output.Append(v);
    
    v.pos = mul(float4(p2, 1.0f), gViewProj);
    v.uv = float2(1.0f, 1.0f);
    output.Append(v);
    
    v.pos = mul(float4(p3, 1.0f), gViewProj);
    v.uv = float2(1.0f, 0.0f);
    output.Append(v);
    
    output.RestartStrip();
}