cbuffer ComputeConstantBuffer : register(b1)
{
    float gDeltaTime;
    uint maxParticles;
    float gTime;
    uint gSpawnMode;
};

struct ParticleData
{
    float3 pos;
    float age;
    float3 vel;
    float lifeTime;
};

RWStructuredBuffer<ParticleData> gParticlesRW : register(u0);

float Hash(float x)
{
    return frac(sin(x * 12.9898f) * 43758.5453f);
}

void SpawnParticle(inout ParticleData p, uint idx)
{
    float seed = (float) idx + gTime * 10.0f;
    float r1 = Hash(seed * 1.3f);
    float r2 = Hash(seed * 2.1f);
    float r3 = Hash(seed * 3.7f);

    p.pos = float3(0, 0, 0);
    p.vel = float3(
    (r1 - 0.5f) * 2.0f,
    5.0f + r2 * 2.0f,
    (r3 - 0.5f) * 2.0f);
    
    p.lifeTime = 2.0f;
    p.age = 0.0f;
}

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint idx = dispatchThreadId.x;
    if (maxParticles <= idx)
    {
        return;
    }

    ParticleData p = gParticlesRW[idx];
    float spawnRate = 60.0f;
    float spawnProb = spawnRate * gDeltaTime / (float) maxParticles;
    if (1 <= p.age)
    {
        if (gSpawnMode == 0)
        {
            // Æø¹ß
            SpawnParticle(p, idx);
        }
        else
        {
            // cpu ºÐ¼ö
            float r = Hash((float) idx + gTime);
            if (r < spawnProb)
            {
                SpawnParticle(p, idx);
            }
            else
            {
                p.age = 1.0f;
            }
        }
    }
    else
    {
        float life = p.lifeTime;
        float ageSec = p.age * life + gDeltaTime;
        if (life <= ageSec)
        {
            p.age = 1.0f;
        }
        else
        {
            p.age = ageSec / life;
            p.pos += p.vel * gDeltaTime;
            p.vel.y -= 9.8f * 0.5f * gDeltaTime;
        }
    }
    
    gParticlesRW[idx] = p;
}