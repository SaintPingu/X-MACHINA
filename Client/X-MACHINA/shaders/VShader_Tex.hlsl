struct VSInput_Tex {
    float3 PosW : POSITION;
    float2 UV   : UV;
};

struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV : UV;
};

VSOutput_Tex VSTex(VSInput_Tex vin)
{
    VSOutput_Tex vout;
    
    vout.PosH = float4(vin.PosW * 2.f, 1.f);
    vout.UV = vin.UV;

    return vout;
}