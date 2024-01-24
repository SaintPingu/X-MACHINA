#include "Common.hlsl"

struct VSOutput_Canvas {
    float4 Position : SV_POSITION;
    float2 UV : UV;
};

VSOutput_Canvas VSCanvas(uint vertexID : SV_VertexID)
{
   
    VSOutput_Canvas output;
    
    switch (vertexID) {
        case 0:
        {
            output.Position = float4(-1.f, +1.f, 0.f, 1.f);
            output.UV = float2(0.f, 0.f);
        }
        
        break;
        case 1:
        {
            output.Position = float4(+1.f, +1.f, 0.f, 1.f);
            output.UV = float2(1.f, 0.f);
        }
        
        break;
        case 2:
        {
            output.Position = float4(+1.f, -1.f, 0.f, 1.f);
            output.UV = float2(1.f, 1.f);
        }
        
        break;
        case 3:
        {
            output.Position = float4(-1.f, +1.f, 0.f, 1.f);
            output.UV = float2(0.f, 0.f);
        }
        
        break;
        case 4:
        {
            output.Position = float4(+1.f, -1.f, 0.f, 1.f);
            output.UV = float2(1.f, 1.f);
        }
        
        break;
        case 5:
        {
            output.Position = float4(-1.f, -1.f, 0.f, 1.f);
            output.UV = float2(0.f, 1.f);
        }
        break;
        
        default:
            break;
    }
    
    output.Position = mul(output.Position, gMtxWorld);
    output.UV = mul(float3(output.UV, 1.f), (float3x3) (gMtxSprite)).xy;
    
    return output;
}