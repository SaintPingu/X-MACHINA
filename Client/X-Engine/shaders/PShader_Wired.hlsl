struct VSOutput {
    float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};


float4 PSWired(VSOutput input) : SV_TARGET
{
    return input.Color;
}