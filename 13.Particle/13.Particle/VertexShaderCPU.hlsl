struct VS_INPUT
{
    float3 pos : POSITION;
    float age : AGE;
};

struct VS_OUTPUT
{
    float3 pos : POSITION;
    float age : AGE;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = input.pos;
    output.age = input.age;
    return output;
}