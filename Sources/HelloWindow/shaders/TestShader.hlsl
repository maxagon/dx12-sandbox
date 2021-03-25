
struct CBDesc
{
    float2 resolution;
};

ConstantBuffer<CBDesc> cb : register (b0);

struct InputPS
{
    float4 position : SV_Position;
};

InputPS MainVS(float4 position : POSITION)
{
    InputPS result;
    result.position = position;
    return result;
}

float4 MainPS(InputPS input) : SV_TARGET
{
    return float4(input.position.xy / cb.resolution, 0.3, 1.0);
}
