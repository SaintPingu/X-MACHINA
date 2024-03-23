struct VSInput_Tex {
    float3 PosL : POSITION;
    float2 UV   : UV;
};

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV : UV;
};

VSOutput_Tex VSTex(VSInput_Tex vin)
{
    VSOutput_Tex vout;
    
    vout.PosH = float4(vin.PosL * 2.f, 1.f);
    vout.UV = vin.UV;

    return vout;
}