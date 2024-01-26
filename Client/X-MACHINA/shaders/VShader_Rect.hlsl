struct VSOutput_Tex {
    float4 PosH : SV_POSITION;
    float2 UV   : UV;
};

struct VSOutput_ScreenTex {
    float4 PosH : SV_POSITION;
    float2 UV : UV;
};

VSOutput_ScreenTex VSScreen(uint vertexID : SV_VertexID)
{
    VSOutput_ScreenTex output = (VSOutput_Tex) 0;
    
    switch (vertexID)
    {
        case 0:
        {
            output.PosH = float4(-1.f, +1.f, 0.f, 1.f);
            output.UV = float2(0.f, 0.f);
            break;
        }
        case 1:
        {
            output.PosH = float4(+1.f, +1.f, 0.f, 1.f);
            output.UV = float2(1.f, 0.f);
            break;
        }
        case 2:
        {
            output.PosH = float4(+1.f, -1.f, 0.f, 1.f);
            output.UV = float2(1.f, 1.f);
            break;
        }
        case 3:
        {
            output.PosH = float4(-1.f, +1.f, 0.f, 1.f);
            output.UV = float2(0.f, 0.f);
            break;
        }
        case 4:
        {
            output.PosH = float4(+1.f, -1.f, 0.f, 1.f);
            output.UV = float2(1.f, 1.f);
            break;
        }
        case 5:
        {
            output.PosH = float4(-1.f, -1.f, 0.f, 1.f);
            output.UV = float2(0.f, 1.f);
            break;
        }
        
    }

    return output;
}