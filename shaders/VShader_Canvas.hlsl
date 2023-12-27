#include "VSResource.hlsl"

struct VS_CANVAS_INPUT
{
    float3 position : POSITION;
    float2 uv : UV;
};

struct VS_CANVAS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

VS_CANVAS_OUTPUT VSCanvas(uint nVertexID : SV_VertexID)
{
    //VS_CANVAS_OUTPUT output;
    
    //output.position = mul(float4(input.position, 1.0f), gmtxWorld);
    //output.uv = input.uv;
    
    VS_CANVAS_OUTPUT output;
    
    switch (nVertexID)
    {
        case 0:
        {
                output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
                output.uv = float2(0.0f, 0.0f);
                break;
            }
        case 1:
        {
                output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);
                output.uv = float2(1.0f, 0.0f);
                break;
            }
        case 2:
        {
                output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
                output.uv = float2(1.0f, 1.0f);
                break;
            }
        case 3:
        {
                output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
                output.uv = float2(0.0f, 0.0f);
                break;
            }
        case 4:
        {
                output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
                output.uv = float2(1.0f, 1.0f);
                break;
            }
        case 5:
        {
                output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
                output.uv = float2(0.0f, 1.0f);
                break;
            }
        
    }
    output.position = mul(output.position, gmtxWorld);
    output.uv = mul(float3(output.uv, 1.0f), (float3x3) (gmtxSprite)).xy;
    
    return (output);
}