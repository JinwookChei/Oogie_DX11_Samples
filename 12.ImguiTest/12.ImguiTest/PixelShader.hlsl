Texture2D g_Texture : register(t0);
Texture2D g_NormalMap : register(t1);
SamplerState g_Sampler : register(s0);

cbuffer CBTransform : register(b0)
{
    matrix cb_World;
    matrix cb_View;
    matrix cb_Projection;
    matrix cb_WorldInvTranspose;
    
    float4 cb_CamPos;
};

cbuffer CBLight : register(b1)
{
    float4 cb_LightDiffuse;
    float4 cb_LightSpecular;
    float4 cb_LightAmbient;

    float3 cb_LightDir;
    float cb_Pad0;
};

cbuffer CBMaterial : register(b2)
{
    float4 cb_MaterialAmbient;
    float4 cb_MaterialDiffuse;
    float4 cb_MaterialSpecular;
    float4 cb_MaterialEmissive;

    float cb_Shininess;
    float3 cb_MaterialPad;
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

float3 UnpackNormal(float4 c)
{
    return normalize(c.xyz * 2.0f - 1.0f);
}

//After Use Normal Mapping
//float4 main(PS_INPUT input) : SV_TARGET
//{
//    float4 colorTex = g_Texture.Sample(g_Sampler, input.uv);
//    float4 normalTex = g_NormalMap.Sample(g_Sampler, input.uv);
//    float3 unpackNormalTex = UnpackNormal(normalTex);
//    float3 worldNormal = normalize(mul(unpackNormalTex, input.TBN));
    
//    float3 N = normalize(worldNormal);
//    float3 L = normalize(spotPosition - input.worldPos);
//    float3 S = normalize(-spotDirection);
    
//    float spotCos = dot(L, S);
//    float spotEffect = smoothstep(spotAngle, spotAngle + 0.05, spotCos);
    
//    float dist = length(spotPosition - input.worldPos);
//    float att = saturate(1.0f - dist / spotRange);
    
//    float diffuse = saturate(dot(N, L)) * spotEffect * att;
//    float diffuseColor = diffuse * lightColor;
    
//    float4 finalColor = colorTex * (diffuseColor + ambientColor);
//    finalColor.a = colorTex.a; // 텍스처 알파 사용
    
//    return finalColor;
//    return float4(input.normal, 1.0f);
//}


// Diffuse Term
//float4 main(PS_INPUT input) : SV_TARGET
//{
//    float3 normal = input.normal;
//    float3 lightDir = -cb_LightDir;
//    float nDotL = dot(normalize(normal), normalize(lightDir));
//    float nDotlClamp = saturate(nDotL);
    
//    float3 diffuseLight = nDotlClamp * cb_LightDiffuse.rgb * cb_MaterialDiffuse.rgb;
//    float4 finalColor = float4(diffuseLight, 1.0);
   
//    return finalColor;
//}


// Specular Term
//float4 main(PS_INPUT input) : SV_TARGET
//{
//    float3 normal = input.normal;
//    float3 lightDir = -cb_LightDir;
    
//    // 반사 벡터(reflection)
//    float nDotL = dot(normalize(normal), normalize(lightDir));
//    float3 reflecDir = 2 * normal * nDotL - lightDir;
//    reflecDir = normalize(reflecDir);
    
//    float3 viewDir;
//    viewDir.x = cb_View._13;
//    viewDir.y = cb_View._23;
//    viewDir.z = cb_View._33;
//    viewDir = -normalize(viewDir);
    
//    float rDotV = saturate(dot(reflecDir, viewDir));
//    float adjShininess = pow(rDotV, cb_Shininess);
    
//    float3 specularColor = adjShininess * cb_LightSpecular.rgb * cb_MaterialSpecular.rgb;
    
//    float4 finalColor = input.color + float4(specularColor, 0.0f);
//    return finalColor;
//}



float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal = input.normal;
    float3 lightDir = -cb_LightDir;
    float nDotL = dot(normalize(normal), normalize(lightDir));
    
    // Diffuse
    float nDotlClamp = saturate(nDotL);
    float3 diffuseColor = nDotlClamp * cb_LightDiffuse.rgb * cb_MaterialDiffuse.rgb;
    
    // Specular
    float3 reflecDir = normalize(2 * normal * nDotL - lightDir);
    float3 objectToEye = normalize(cb_CamPos.xyz - input.worldPos);
    float rDotV = saturate(dot(reflecDir, objectToEye));
    
    float adjShininess = pow(rDotV, cb_Shininess);
    float3 specularColor = adjShininess * cb_LightSpecular.rgb * cb_MaterialSpecular.rgb;
    
    // Ambient
    float3 ambientColor = cb_LightAmbient * cb_MaterialAmbient;
    //float3 ambientColor = cb_LightAmbient * cb_MaterialDiffuse ;
    
    // Emissive
    float3 emissiveColor = cb_MaterialEmissive;
    
    
    float4 finalColor = float4(diffuseColor, 1.0f) + float4(specularColor, 0.0f) + float4(ambientColor, 0.0f) + float4(emissiveColor, 0.0f);
    
    
    return finalColor;
}