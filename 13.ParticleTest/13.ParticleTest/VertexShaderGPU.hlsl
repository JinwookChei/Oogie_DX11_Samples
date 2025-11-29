struct ParticleData
{
    float3 pos;
    float age;
    float3 vel;
    float lifeTime;
};

StructuredBuffer<ParticleData> gParticles : register(t1);

struct VS_OUTPUT
{
    float3 pos : POSITION;
    float age : AGE;
};

VS_OUTPUT main(uint vertexID : SV_VertexID)
{
    ParticleData p = gParticles[vertexID];
    
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = p.pos;
    output.age = p.age;
    
    return output;
}