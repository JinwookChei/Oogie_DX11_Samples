cbuffer CBTransform : register(b0)
{
    matrix cb_World;
    matrix cb_View;
    matrix cb_Projection;
    matrix cb_WorldInvTranspose;
    
    float4 cb_CamPos;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 tangent : TANGENT;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 worldPos : TEXCOORD0;
    float2 uv : TEXCOORD1;
    float3x3 TBN : TEXCOORD2;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    float4 worldPosition = mul(float4(input.position, 1.0f), cb_World);     // 월드 포지션 ( 원점을 기준으로 얼마만큼 떨어져있나 )
    float4 viewPosition = mul(worldPosition, cb_View);                      // 뷰 포지션 ( 카메라를 기준으로 둔 포지션 ) ( 카메라 기준이란? 카메라의 포지션을 0, 0, 0 으로 본다 ) ( 카메라 기준이란? 카메라를 원점으로 만든다. )
    output.pos = mul(viewPosition, cb_Projection);
    output.color = input.color;
    
    float3 N = normalize(mul(input.normal, (float3x3) cb_World));
    float3 T = normalize(mul(input.tangent.xyz, (float3x3) cb_World));
    float3 B = normalize(cross(N, T) * input.tangent.w);
    
    output.normal = mul(float4(input.normal, 1.0f), cb_WorldInvTranspose);
    output.worldPos = worldPosition.xyz;
    output.uv = input.uv;
    output.TBN = float3x3(T, B, N);
    
    return output;
}
