struct VSInput_Lighting {
    float3 PosW : POSITION;
    float2 UV   : UV;
};

struct VSOutput_Lighting {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

VSOutput_Lighting VSDirLighting(VSInput_Lighting vin)
{
    VSOutput_Lighting vout;
    
    vout.PosH = float4(vin.PosW * 2.f, 1.f);
    vout.UV = vin.UV;
    
    return vout;
}