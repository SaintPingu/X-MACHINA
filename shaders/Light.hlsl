#include "VSResource.hlsl"
#include "PSResource.hlsl"

#define _WITH_LOCAL_VIEWER_HIGHLIGHT
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT

#define gkMaxSceneLight         32
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

cbuffer cbLights : register(b2) {
    LightInfo gLights[gkMaxSceneLight];
    float4 gGlobalAmbient;
    
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
};





float4 DirectionalLight(int index, float3 vNormal, float3 vToCamera)
{
    float3 vToLight = -gLights[index].Direction;
    float diffuseFactor = dot(vToLight, vNormal);
    float specularFactor = 0.f;
    if (diffuseFactor > 0.f)
    {
        if (gMaterial.Sepcular.a != 0.f)
        {
#ifdef _WITH_REFLECT
            float3 vReflect = reflect(-vToLight, vNormal);
            specularFactor = pow(max(dot(vReflect, vToCamera), 0.f), gMaterial.Sepcular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHT
            float3 vHalf = normalize(vToCamera + vToLight);
#else
float3 vHalf = float3(0.f, 1.f, 0.f);
#endif
            specularFactor = pow(max(dot(vHalf, vNormal), 0.f), gMaterial.Sepcular.a);
#endif
        }
    }
    return ((gLights[index].Ambient * gMaterial.Ambient) +
            (gLights[index].Diffuse * diffuseFactor * gMaterial.Diffuse) +
            (gLights[index].Sepcular * specularFactor * gMaterial.Sepcular));
}






float4 PointLight(int index, float3 vPosition, float3 vNormal, float3 vToCamera)
{
    float3 vToLight = gLights[index].Position - vPosition;
    float distance = length(vToLight);
    if (distance <= gLights[index].Range)
    {
        float specularFactor = 0.f;
        vToLight /= distance;
        float diffuseFactor = dot(vToLight, vNormal);
        if (diffuseFactor > 0.f)
        {
            if (gMaterial.Sepcular.a != 0.f)
            {
#ifdef _WITH_REFLECT
float3 vReflect = reflect(-vToLight, vNormal);
specularFactor = pow(max(dot(vReflect, vToCamera), 0.f),
gMaterial.Sepcular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHT
                float3 vHalf = normalize(vToCamera + vToLight);
#else
float3 vHalf = float3(0.f, 1.f, 0.f);
#endif
                specularFactor = pow(max(dot(vHalf, vNormal), 0.f), gMaterial.Sepcular.a);
#endif
            }
        }
        else
        {
            return float4(0.f, 0.f, 0.f, 0.f);
        }
        
        float attenuationFactor = 1.f / dot(gLights[index].Attenuation, float3(1.f, distance / 2, distance * distance));
        
        float far = gLights[index].Range - distance;
        far = min(1.f, far * 0.1f);
        attenuationFactor *= far;
        attenuationFactor = max(0.f, attenuationFactor);
        
        return (((gLights[index].Ambient * gMaterial.Ambient) +
                (gLights[index].Diffuse * diffuseFactor * gMaterial.Diffuse) +
                (gLights[index].Sepcular * specularFactor * gMaterial.Sepcular)) * attenuationFactor);
    }
    return float4(0.f, 0.f, 0.f, 0.f);
}






float4 SpotLight(int index, float3 vPosition, float3 vNormal, float3 vToCamera)
{
    float3 vToLight = gLights[index].Position - vPosition;
    float distance = length(vToLight);
    if (distance <= gLights[index].Range)
    {
        float specularFactor = 0.f;
        vToLight /= distance;
        float diffuseFactor = dot(vToLight, vNormal);
        if (diffuseFactor > 0.f)
        {
            if (gMaterial.Sepcular.a != 0.f)
            {
#ifdef _WITH_REFLECT
                
    float3 vReflect = reflect(-vToLight, vNormal);
    specularFactor = pow(max(dot(vReflect, vToCamera), 0.f), gMaterial.Sepcular.a);
                
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHT
                float3 vHalf = normalize(vToCamera + vToLight);
#else
        float3 vHalf = float3(0.f, 1.f, 0.f);
#endif
                specularFactor = pow(max(dot(vHalf, vNormal), 0.f), gMaterial.Sepcular.a);
#endif
            }
        }
        else
        {
            return float4(0.f, 0.f, 0.f, 0.f);
        }
        
#ifdef _WITH_THETA_PHI_CONES
        float fAlpha = max(dot(-vToLight, gLights[index].Direction), 0.f);
        float fSpotFactor = pow(max(((fAlpha - gLights[index].Phi) / (gLights[index].Theta - gLights[index].Phi)), 0.f), gLights[index].Falloff);
        
#else
    float fSpotFactor = pow(max(dot(-vToLight, gLights[i].Direction), 0.f),
    gLights[i].Falloff);
        
#endif
        float attenuationFactor = 1.f / dot(gLights[index].Attenuation, float3(1.f, distance, distance * distance));
        
        float far = gLights[index].Range - distance;
        far = min(1.f, far * 0.1f);
        attenuationFactor *= far;
        attenuationFactor = max(0.f, attenuationFactor);
        
        return (((gLights[index].Ambient * gMaterial.Ambient) +
                (gLights[index].Diffuse * diffuseFactor * gMaterial.Diffuse) +
                (gLights[index].Sepcular * specularFactor * gMaterial.Sepcular)) * attenuationFactor * fSpotFactor);
    }
    return float4(0.f, 0.f, 0.f, 0.f);
}



float4 Fog(float4 color, float3 position)
{
    float3 camPos = gCameraPos;
    float3 posToCam = camPos - position;
    float distance = length(posToCam);
    
    float factor = min(saturate((distance - gFogStart) / gFogRange), 0.8f);
    return lerp(color, gFogColor, factor);
}

float4 FogDistance(float4 color, float3 distance)
{
    float factor = min(saturate((distance - gFogStart) / gFogRange), 0.8f);
    return lerp(color, gFogColor, factor);
}


float4 Lighting(float3 vPosition, float3 vNormal)
{
    float3 vCameraPosition = float3(gCameraPos.x, gCameraPos.y, gCameraPos.z);
    float3 vToCamera = normalize(vCameraPosition - vPosition);
    float4 color = float4(0.f, 0.f, 0.f, 0.f);
    
    for (int i=0; i < gkMaxSceneLight; i++)
    {
        if (gLights[i].Enable)
        {
            if (gLights[i].Type == LightType_Directional)
            {
                color += DirectionalLight(i, vNormal, vToCamera);
            }
            else if (gLights[i].Type == LightType_Point)
            {
                color += PointLight(i, vPosition, vNormal, vToCamera);
            }
            else if (gLights[i].Type == LightType_Spot)
            {
                color += SpotLight(i, vPosition, vNormal, vToCamera);
            }
        }
    }
    
    color += (gGlobalAmbient * gMaterial.Ambient);
    color += gMaterial.Emissive;
    color.a = gMaterial.Diffuse.a;

    return color;
}