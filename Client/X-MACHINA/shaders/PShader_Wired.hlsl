struct VSOutput {
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};


float4 PSWired(VSOutput input) : SV_TARGET1
{
    return input.Color;
}