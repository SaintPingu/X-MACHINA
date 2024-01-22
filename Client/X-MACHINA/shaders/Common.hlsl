#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_

#define MaterialInfo_Albedo			0x01
#define MaterialInfo_Specular		0x02
#define MaterialInfo_Normal			0x04
#define MaterialInfo_Metalic    	0x08
#define MaterialInfo_Emission		0x10
#define MaterialInfo_DetailAlbedo	0x20
#define MaterialInfo_DetailNormal	0x40
#define MaterialInfo_Sprite         0x80

#define gkMaxSceneLight         32

#define _WITH_LOCAL_VIEWER_HIGHLIGHT
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT

#define LightType_Spot          0
#define LightType_Directional   1
#define LightType_Point         2

struct LightInfo {
    float4 Ambient;
    float4 Diffuse;
    float4 Sepcular;
    
    float3 Position;
    float Falloff;
    
    float3 Direction;
    float Theta; //cos(Theta)
    
    float3 Attenuation;
    float Phi; //cos(Phi)
    
    float Range;
    float Padding;
    int Type;
    bool Enable;
};

struct MaterialInfo {
    float4  Ambient;
    float4  Diffuse;
    float4  Sepcular; //a = power
    float4  Emissive;
    
    //int     DiffuseMapIndex;
    //int     NormalMapIndex;
    //int     RoughnessMapIndex;
    //int     HeightMapIndex;
};

struct TestMaterial {
    float4  Ambient;
    float4  Diffuse;
    float4  Sepcular; //a = power
    float4  Emissive;
    
    int DiffuseMapIndex;
    int NormalMapIndex;
    int RoughnessMapIndex;
    int HeightMapIndex;
};

bool IsWhite(float4 color)
{
    return color.rgb == float3(1.f, 1.f, 1.f) && color.a == 1.f;
}

#define POST_PROCESSING
#endif