struct VS_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

struct VS_SCREEN_RECT_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

VS_SCREEN_RECT_TEXTURED_OUTPUT VSScreen(uint nVertexID : SV_VertexID)
{
    VS_SCREEN_RECT_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT) 0;
    
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


    return (output);
}