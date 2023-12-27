struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


float4 PSWired(VS_OUTPUT input) : SV_TARGET1
{
    return input.color;
}