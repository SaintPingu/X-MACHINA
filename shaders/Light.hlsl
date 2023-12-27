#define MAX_SCENE_LIGHTS 32
#define SPOT_LIGHT 0
#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2
#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT

#include "VSResource.hlsl"
#include "PSResource.hlsl"

struct LIGHT
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular;
    float3 m_vPosition;
    float m_fFalloff;
    float3 m_vDirection;
    float m_fTheta; //cos(m_fTheta)
    float3 m_vAttenuation;
    float m_fPhi; //cos(m_fPhi)
    bool m_bEnable;
    int mType;
    float m_fRange;
    float padding;
};

cbuffer cbLights : register(b2)
{
    LIGHT gLights[MAX_SCENE_LIGHTS];
    float4 gcGlobalAmbientLight;
    
    float4 fogColor;
    float fogStart;
    float fogRange;
};





float4 DirectionalLight(int index, float3 vNormal, float3 vToCamera)
{
    float3 vToLight = -gLights[index].m_vDirection;
    float fDiffuseFactor = dot(vToLight, vNormal);
    float fSpecularFactor = 0.0f;
    if (fDiffuseFactor > 0.0f)
    {
        if (gMaterial.m_cSpecular.a != 0.0f)
        {
#ifdef _WITH_REFLECT
float3 vReflect = reflect(-vToLight, vNormal);
fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f),
gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
            float3 vHalf = normalize(vToCamera + vToLight);
#else
float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
            fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
        }
    }
    return ((gLights[index].m_cAmbient * gMaterial.m_cAmbient) +
            (gLights[index].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) +
            (gLights[index].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular));
}






float4 PointLight(int index, float3 vPosition, float3 vNormal, float3 vToCamera)
{
    float3 vToLight = gLights[index].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    if (fDistance <= gLights[index].m_fRange)
    {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance;
        float fDiffuseFactor = dot(vToLight, vNormal);
        if (fDiffuseFactor > 0.0f)
        {
            if (gMaterial.m_cSpecular.a != 0.0f)
            {
#ifdef _WITH_REFLECT
float3 vReflect = reflect(-vToLight, vNormal);
fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f),
gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
                float3 vHalf = normalize(vToCamera + vToLight);
#else
float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
                fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
            }
        }
        else
        {
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
        
        float fAttenuationFactor = 1.0f / dot(gLights[index].m_vAttenuation, float3(1.0f, fDistance / 2, fDistance * fDistance));
        
        float far = gLights[index].m_fRange - fDistance;
        far = min(1.0f, far * 0.1f);
        fAttenuationFactor *= far;
        fAttenuationFactor = max(0.0f, fAttenuationFactor);
        
        return (((gLights[index].m_cAmbient * gMaterial.m_cAmbient) +
                (gLights[index].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) +
                (gLights[index].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}






float4 SpotLight(int index, float3 vPosition, float3 vNormal, float3 vToCamera)
{
    float3 vToLight = gLights[index].m_vPosition - vPosition;
    float fDistance = length(vToLight);
    if (fDistance <= gLights[index].m_fRange)
    {
        float fSpecularFactor = 0.0f;
        vToLight /= fDistance;
        float fDiffuseFactor = dot(vToLight, vNormal);
        if (fDiffuseFactor > 0.0f)
        {
            if (gMaterial.m_cSpecular.a != 0.0f)
            {
#ifdef _WITH_REFLECT
                
    float3 vReflect = reflect(-vToLight, vNormal);
    fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f),
    gMaterial.m_cSpecular.a);
                
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
                float3 vHalf = normalize(vToCamera + vToLight);
#else
        float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
                fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
            }
        }
        else
        {
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
        
#ifdef _WITH_THETA_PHI_CONES
        float fAlpha = max(dot(-vToLight, gLights[index].m_vDirection), 0.0f);
        float fSpotFactor = pow(max(((fAlpha - gLights[index].m_fPhi) / (gLights[index].m_fTheta - gLights[index].m_fPhi)), 0.0f), gLights[index].m_fFalloff);
        
#else
    float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f),
    gLights[i].m_fFalloff);
        
#endif
        float fAttenuationFactor = 1.0f / dot(gLights[index].m_vAttenuation, float3(1.0f, fDistance, fDistance * fDistance));
        
        float far = gLights[index].m_fRange - fDistance;
        far = min(1.0f, far * 0.1f);
        fAttenuationFactor *= far;
        fAttenuationFactor = max(0.0f, fAttenuationFactor);
        
        return (((gLights[index].m_cAmbient * gMaterial.m_cAmbient) +
                (gLights[index].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) +
                (gLights[index].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor * fSpotFactor);
    }
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}



float4 Fog(float4 color, float3 position)
{
    float3 camPos = gf3CameraPosition;
    float3 posToCam = camPos - position;
    float distance = length(posToCam);
    
    float factor = min(saturate((distance - fogStart) / fogRange), 0.8f);
    return lerp(color, fogColor, factor);
}

float4 FogDistance(float4 color, float3 distance)
{
    float factor = min(saturate((distance - fogStart) / fogRange), 0.8f);
    return lerp(color, fogColor, factor);
}


float4 Lighting(float3 vPosition, float3 vNormal)
{
    float3 vCameraPosition = float3(gf3CameraPosition.x, gf3CameraPosition.y, gf3CameraPosition.z);
    float3 vToCamera = normalize(vCameraPosition - vPosition);
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    for (int i=0; i < MAX_SCENE_LIGHTS; i++)
    {
        if (gLights[i].m_bEnable)
        {
            if (gLights[i].mType == DIRECTIONAL_LIGHT)
            {
                cColor += DirectionalLight(i, vNormal, vToCamera);
            }
            else if (gLights[i].mType == POINT_LIGHT)
            {
                cColor += PointLight(i, vPosition, vNormal, vToCamera);
            }
            else if (gLights[i].mType == SPOT_LIGHT)
            {
                cColor += SpotLight(i, vPosition, vNormal, vToCamera);
            }
        }
    }
    
    cColor += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
    cColor += gMaterial.m_cEmissive;
    cColor.a = gMaterial.m_cDiffuse.a;

    return cColor;
}