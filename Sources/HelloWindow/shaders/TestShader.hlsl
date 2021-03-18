
struct InputPS
{
    float3 position;
};

InputPS MainVS(float4 position : SV_POSITION)
{
    PSInput result;

    result.position = position.xyz;
    return result;
}

float4 MainPS(InputPS input) : SV_TARGET
{
    return float4(input.position, 1.0);
}
