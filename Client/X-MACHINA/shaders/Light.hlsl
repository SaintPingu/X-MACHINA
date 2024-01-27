#include "Common.hlsl"

bool IsWhite(float4 color)
{
    return color.rgb == float3(1.f, 1.f, 1.f) && color.a == 1.f;
}

float4 DirectionalLight(int index, MaterialInfo mat, float3 vNormal, float3 vToCamera)
{
    float3 vToLight = -gPassCB.Lights[index].Direction;
    float diffuseFactor = dot(vToLight, vNormal);
    float specularFactor = 0.f;
    if (diffuseFactor > 0.f)
    {
        if (mat.Sepcular.a != 0.f)
        {
#ifdef _WITH_REFLECT
            float3 vReflect = reflect(-vToLight, vNormal);
            specularFactor = pow(max(dot(vReflect, vToCamera), 0.f), mat.Sepcular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHT
            float3 vHalf = normalize(vToCamera + vToLight);
#else
float3 vHalf = float3(0.f, 1.f, 0.f);
#endif
            specularFactor = pow(max(dot(vHalf, vNormal), 0.f), mat.Sepcular.a);
#endif
        }
    }
    return ((gPassCB.Lights[index].Ambient * mat.Ambient) +
            (gPassCB.Lights[index].Diffuse * diffuseFactor * mat.Diffuse) +
            (gPassCB.Lights[index].Sepcular * specularFactor * mat.Sepcular));
}

float4 PointLight(int index, MaterialInfo mat, float3 vPosition, float3 vNormal, float3 vToCamera)
{
    float3 vToLight = gPassCB.Lights[index].Position - vPosition;
    float distance = length(vToLight);
    if (distance <= gPassCB.Lights[index].Range)
    {
        float specularFactor = 0.f;
        vToLight /= distance;
        float diffuseFactor = dot(vToLight, vNormal);
        if (diffuseFactor > 0.f)
        {
            if (mat.Sepcular.a != 0.f)
            {
#ifdef _WITH_REFLECT
float3 vReflect = reflect(-vToLight, vNormal);
specularFactor = pow(max(dot(vReflect, vToCamera), 0.f),
mat.Sepcular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHT
                float3 vHalf = normalize(vToCamera + vToLight);
#else
float3 vHalf = float3(0.f, 1.f, 0.f);
#endif
                specularFactor = pow(max(dot(vHalf, vNormal), 0.f), mat.Sepcular.a);
#endif
            }
        }
        else
        {
            return float4(0.f, 0.f, 0.f, 0.f);
        }
        
        float attenuationFactor = 1.f / dot(gPassCB.Lights[index].Attenuation, float3(1.f, distance / 2, distance * distance));
        
        float far = gPassCB.Lights[index].Range - distance;
        far = min(1.f, far * 0.1f);
        attenuationFactor *= far;
        attenuationFactor = max(0.f, attenuationFactor);
        
        return (((gPassCB.Lights[index].Ambient * mat.Ambient) +
                (gPassCB.Lights[index].Diffuse * diffuseFactor * mat.Diffuse) +
                (gPassCB.Lights[index].Sepcular * specularFactor * mat.Sepcular)) * attenuationFactor);
    }
    return float4(0.f, 0.f, 0.f, 0.f);
}






float4 SpotLight(int index, MaterialInfo mat, float3 vPosition, float3 vNormal, float3 vToCamera)
{
    float3 vToLight = gPassCB.Lights[index].Position - vPosition;
    float distance = length(vToLight);
    if (distance <= gPassCB.Lights[index].Range)
    {
        float specularFactor = 0.f;
        vToLight /= distance;
        float diffuseFactor = dot(vToLight, vNormal);
        if (diffuseFactor > 0.f)
        {
            if (mat.Sepcular.a != 0.f)
            {
#ifdef _WITH_REFLECT
                
    float3 vReflect = reflect(-vToLight, vNormal);
    specularFactor = pow(max(dot(vReflect, vToCamera), 0.f), mat.Sepcular.a);
                
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHT
                float3 vHalf = normalize(vToCamera + vToLight);
#else
        float3 vHalf = float3(0.f, 1.f, 0.f);
#endif
                specularFactor = pow(max(dot(vHalf, vNormal), 0.f), mat.Sepcular.a);
#endif
            }
        }
        else
        {
            return float4(0.f, 0.f, 0.f, 0.f);
        }
        
#ifdef _WITH_THETA_PHI_CONES
        float fAlpha = max(dot(-vToLight, gPassCB.Lights[index].Direction), 0.f);
        float fSpotFactor = pow(max(((fAlpha - gPassCB.Lights[index].Phi) / (gPassCB.Lights[index].Theta - gPassCB.Lights[index].Phi)), 0.f), gPassCB.Lights[index].Falloff);
        
#else
    float fSpotFactor = pow(max(dot(-vToLight, gLights[i].Direction), 0.f),
    gLights[i].Falloff);
        
#endif
        float attenuationFactor = 1.f / dot(gPassCB.Lights[index].Attenuation, float3(1.f, distance, distance * distance));
        
        float far = gPassCB.Lights[index].Range - distance;
        far = min(1.f, far * 0.1f);
        attenuationFactor *= far;
        attenuationFactor = max(0.f, attenuationFactor);
        
        return (((gPassCB.Lights[index].Ambient * mat.Ambient) +
                (gPassCB.Lights[index].Diffuse * diffuseFactor * mat.Diffuse) +
                (gPassCB.Lights[index].Sepcular * specularFactor * mat.Sepcular)) * attenuationFactor * fSpotFactor);
    }
    return float4(0.f, 0.f, 0.f, 0.f);
}



float4 Fog(float4 color, float3 position)
{
    float3 camPos = gPassCB.CameraPos;
    float3 posToCam = camPos - position;
    float distance = length(posToCam);
    
    float factor = min(saturate((distance - gPassCB.FogStart) / gPassCB.FogRange), 0.8f);
    return lerp(color, gPassCB.FogColor, factor);
}

float4 FogDistance(float4 color, float3 distance)
{
    float factor = min(saturate((distance - gPassCB.FogStart) / gPassCB.FogRange), 0.8f);
    return lerp(color, gPassCB.FogColor, factor);
}


float4 Lighting(MaterialInfo mat, float3 vPosition, float3 vNormal)
{
    float3 vCameraPosition = float3(gPassCB.CameraPos.x, gPassCB.CameraPos.y, gPassCB.CameraPos.z);
    float3 vToCamera = normalize(vCameraPosition - vPosition);
    float4 color = float4(0.f, 0.f, 0.f, 0.f);
    
    for (int i=0; i < gkMaxSceneLight; i++)
    {
        if (gPassCB.Lights[i].Enable)
        {
            if (gPassCB.Lights[i].Type == LightType_Directional)
            {
                color += DirectionalLight(i, mat, vNormal, vToCamera);
            }
            else if (gPassCB.Lights[i].Type == LightType_Point)
            {
                color += PointLight(i, mat, vPosition, vNormal, vToCamera);
            }
            else if (gPassCB.Lights[i].Type == LightType_Spot)
            {
                color += SpotLight(i, mat, vPosition, vNormal, vToCamera);
            }
        }
    }
    
    color += (gPassCB.GlobalAmbient * mat.Ambient);
    color += mat.Emissive;
    color.a = mat.Diffuse.a;

    return color;
}