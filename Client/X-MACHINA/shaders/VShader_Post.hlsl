float4 VSPostProcessing(uint vertexID : SV_VertexID) : SV_POSITION
{
    if (vertexID == 0)
        return (float4(-1.f, +1.f, 0.f, 1.f));
    if (vertexID == 1)
        return (float4(+1.f, +1.f, 0.f, 1.f));
    if (vertexID == 2)
        return (float4(+1.f, -1.f, 0.f, 1.f));

    if (vertexID == 3)
        return (float4(-1.f, +1.f, 0.f, 1.f));
    if (vertexID == 4)
        return (float4(+1.f, -1.f, 0.f, 1.f));
    if (vertexID == 5)
        return (float4(-1.f, -1.f, 0.f, 1.f));

    return (float4(0, 0, 0, 0));
}