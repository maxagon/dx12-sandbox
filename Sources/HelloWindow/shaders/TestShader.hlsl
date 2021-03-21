
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
    return float4(input.position.xyz, 1.0);
}
