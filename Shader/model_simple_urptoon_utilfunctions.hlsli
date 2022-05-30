#include "nil_outline_utill.hlsli"

///////////////////////////////////////////////////////////////////////////////////////
// util shared functions
///////////////////////////////////////////////////////////////////////////////////////


// just like smoothstep(), but linear, not clamped
float invLerp(float from, float to, float value)
{
    return (value - from) / (to - from);
}
float invLerpClamp(float from, float to, float value)
{
    return saturate(invLerp(from, to, value));
}
// full control remap, but slower
float remap(float origFrom, float origTo, float targetFrom, float targetTo, float value)
{
    float rel = invLerp(origFrom, origTo, value);
    return lerp(targetFrom, targetTo, rel);
}


///////////////////////////////////////////////////////////////////////////////////////
// vertex shared functions
///////////////////////////////////////////////////////////////////////////////////////

float3 TransformPositionWSToOutlinePositionWS(float3 positionWS, float positionVS_Z, float3 normalWS)
{
    //you can replace it to your own method! Here we will write a simple world space method for tutorial reason, it is not the best method!
    float outlineExpandAmount = _OutlineWidth * GetOutlineCameraFovAndDistanceFixMultiplier(positionVS_Z);
    return positionWS + normalWS * outlineExpandAmount;
}


//////////////////////////////////////////////////////////////////////////////////////
// fragment shared functions (Step1: prepare data structs for lighting calculation)
///////////////////////////////////////////////////////////////////////////////////////


float4 GetFinalBaseColor(VS_OUT input)
{
    float4 _colour = albedo_texture.Sample(_BaseMap, input.texcoord) * input.color;
    return _colour;

}
float3 GetFinalEmissionColor(VS_OUT input)
{
    float3 result = 0;
    if (_UseEmission)
    { // emmision_texture.Sample(_EmissionMap, input.texcoord).rgb * _EmissionMapChannelMask * _EmissionColor.rgb;
        result = emmision_texture.Sample(_EmissionMap, input.texcoord).rgb * _EmissionMapChannelMask * _EmissionColor.rgb;
    }

    return result;
}
float GetFinalOcculsion(VS_OUT input)
{
    float result = 1;
    if (_UseOcclusion)
    {
        float4 texValue = oclusion_texture.Sample(_OcclusionMap, input.texcoord);
        float occlusionValue = dot(texValue, _OcclusionMapChannelMask);
        occlusionValue = lerp(0.1, occlusionValue, _OcclusionStrength);
        occlusionValue = invLerpClamp(_OcclusionRemapStart, _OcclusionRemapEnd, occlusionValue);
        result = occlusionValue;
    }

    return result;
}
void DoClipTestToTargetAlphaValue(float alpha)
{

    //clip(alpha - _Cutoff);

}
ToonSurfaceData InitializeSurfaceData(VS_OUT input)
{
    ToonSurfaceData output;

    // albedo & alpha
    float4 baseColorFinal = GetFinalBaseColor(input);
    output.albedo = baseColorFinal.rgb;
    output.alpha = baseColorFinal.a;
    DoClipTestToTargetAlphaValue(output.alpha); // early exit if possible

    // emission
    output.emission = GetFinalEmissionColor(input);

    // occlusion
    output.occlusion = GetFinalOcculsion(input);

    return output;
}
LightingData InitializeLightingData(VS_OUT input)
{
    LightingData lightingData;
    lightingData.positionWS = input.world_position.xyz;
    lightingData.viewDirectionWS = normalize(eye_position.xyz - lightingData.positionWS);
    lightingData.normalWS = normalize(input.world_normal); //interpolated normal is NOT unit vector, we need to normalize it

    return lightingData;
}




float3 ShadeGI(ToonSurfaceData surfaceData, LightingData lightingData)
{
    // hide 3D feeling by ignoring all detail SH (leaving only the constant SH term)
    // we just want some average envi indirect color only
    float3 averageSH = float3(1,1,1);

    // can prevent result becomes completely black if lightprobe was not baked 
    averageSH = max(_IndirectLightMinColor, averageSH);

    // occlusion (maximum 50% darken for indirect to prevent result becomes completely black)
    float indirectOcclusion = lerp(1, surfaceData.occlusion, 0.5);
    return averageSH * indirectOcclusion;
}

// Most important part: lighting equation, edit it according to your needs, write whatever you want here, be creative!
// This function will be used by all direct lights (directional/point/spot)
float3 ShadeSingleLight(ToonSurfaceData surfaceData, LightingData lightingData, bool isAdditionalLight)
{
    float3 N = lightingData.normalWS;
    float3 L = light_direction;
    

    float NoL = dot(N, L);

    float lightAttenuation = 1;

    // light's distance & angle fade for point light & spot light (see GetAdditionalPerObjectLight(...) in Lighting.hlsl)
    // Lighting.hlsl -> https://github.com/Unity-Technologies/Graphics/blob/master/com.unity.render-pipelines.universal/ShaderLibrary/Lighting.hlsl
    float distanceAttenuation = min(4, lightAttenuation /*light.distanceAttenuation*/); //clamp to prevent light over bright if point/spot light too close to vertex

    // N dot L
    // simplest 1 line cel shade, you can always replace this line by your own method!
    float litOrShadowArea = smoothstep(_CelShadeMidPoint - _CelShadeSoftness, _CelShadeMidPoint + _CelShadeSoftness, NoL);

    // occlusion
    litOrShadowArea *= surfaceData.occlusion;

    // face ignore celshade since it is usually very ugly using NoL method
    litOrShadowArea = _IsFace ? lerp(0, 0.5, litOrShadowArea) : litOrShadowArea;

    // light's shadow map
    //litOrShadowArea *= lerp(1, light.shadowAttenuation, _ReceiveShadowMappingAmount);

    float3 litOrShadowColor = lerp(_ShadowMapColor, 1, litOrShadowArea);

    float3 lightAttenuationRGB = litOrShadowColor * distanceAttenuation;

    // saturate() light.color to prevent over bright
    // additional light reduce intensity since it is additive
    return saturate(light_color.xyz) * lightAttenuationRGB * (isAdditionalLight ? 0.25 : 1);
}

float3 ShadeEmission(ToonSurfaceData surfaceData, LightingData lightingData)
{
    float3 emissionResult = lerp(surfaceData.emission, surfaceData.emission * surfaceData.albedo, _EmissionMulByBaseColor); // optional mul albedo
    return emissionResult;
}

float3 CompositeAllLightResults(half3 indirectResult, half3 mainLightResult, half3 additionalLightSumResult, half3 emissionResult, ToonSurfaceData surfaceData, LightingData lightingData)
{
    // [remember you can write anything here, this is just a simple tutorial method]
    // here we prevent light over bright,
    // while still want to preserve light color's hue
    float3 rawLightSum = max(indirectResult, mainLightResult + additionalLightSumResult); // pick the highest between indirect and direct light
    return surfaceData.albedo * rawLightSum + emissionResult;
}



// Push an imaginary vertex towards camera in view space (linear, view space unit), 
// then only overwrite original positionCS.z using imaginary vertex's result positionCS.z value
// Will only affect ZTest ZWrite's depth value of vertex shader

// Useful for:
// -Hide ugly outline on face/eye
// -Make eyebrow render on top of hair
// -Solve ZFighting issue without moving geometry
float4 NiloGetNewClipPosWithZOffset(float4 originalPositionCS, float viewSpaceZOffsetAmount)
{
 
        ////////////////////////////////
        //Perspective camera case
        ////////////////////////////////
        float2 ProjM_ZRow_ZW = projection[2].zw;
        float modifiedPositionVS_Z = -originalPositionCS.w + -viewSpaceZOffsetAmount; // push imaginary vertex
        float modifiedPositionCS_Z = modifiedPositionVS_Z * ProjM_ZRow_ZW[0] + ProjM_ZRow_ZW[1];
        originalPositionCS.z = modifiedPositionCS_Z * originalPositionCS.w / (-modifiedPositionVS_Z); // overwrite positionCS.z
        return originalPositionCS;
    
    //else
    //{
    //    ////////////////////////////////
    //    //Orthographic camera case
    //    ////////////////////////////////
    //    originalPositionCS.z += -viewSpaceZOffsetAmount / _ProjectionParams.z; // push imaginary vertex and overwrite positionCS.z
    //    return originalPositionCS;
    //}
}



float3 ShadeAllLights(ToonSurfaceData surfaceData, LightingData lightingData)
{
    // Indirect lighting
    float3 indirectResult = ShadeGI(surfaceData, lightingData);

    //////////////////////////////////////////////////////////////////////////////////
    // Light struct is provided by URP to abstract light shader variables.
    // It contains light's
    // - direction
    // - color
    // - distanceAttenuation 
    // - shadowAttenuation
    //
    // URP take different shading approaches depending on light and platform.
    // You should never reference light shader variables in your shader, instead use the 
    // -GetMainLight()
    // -GetLight()
    // funcitons to fill this Light struct.
    //////////////////////////////////////////////////////////////////////////////////

    //==============================================================================================
    // Main light is the brightest directional light.
    // It is shaded outside the light loop and it has a specific set of variables and shading path
    // so we can be as fast as possible in the case when there's only a single directional light
    // You can pass optionally a shadowCoord. If so, shadowAttenuation will be computed.


    float3 shadowTestPosWS = lightingData.positionWS + light_direction.xyz * (_ReceiveShadowMappingPosOffset + _IsFace);
#ifdef _MAIN_LIGHT_SHADOWS
    // compute the shadow coords in the fragment shader now due to this change
    // https://forum.unity.com/threads/shadow-cascades-weird-since-7-2-0.828453/#post-5516425

    // _ReceiveShadowMappingPosOffset will control the offset the shadow comparsion position, 
    // doing this is usually for hide ugly self shadow for shadow sensitive area like face
    float4 shadowCoord = TransformWorldToShadowCoord(shadowTestPosWS);
    mainLight.shadowAttenuation = MainLightRealtimeShadow(shadowCoord);
#endif 

    // Main light
    float3 mainLightResult = ShadeSingleLight(surfaceData, lightingData, false);

    //==============================================================================================
    // All additional lights

    float3 additionalLightSumResult = 0;

#ifdef _ADDITIONAL_LIGHTS
    // Returns the amount of lights affecting the object being renderer.
    // These lights are culled per-object in the forward renderer of URP.
    int additionalLightsCount = GetAdditionalLightsCount();
    for (int i = 0; i < additionalLightsCount; ++i)
    {
        // Similar to GetMainLight(), but it takes a for-loop index. This figures out the
        // per-object light index and samples the light buffer accordingly to initialized the
        // Light struct. If ADDITIONAL_LIGHT_CALCULATE_SHADOWS is defined it will also compute shadows.
        int perObjectLightIndex = GetPerObjectLightIndex(i);
        Light light = GetAdditionalPerObjectLight(perObjectLightIndex, lightingData.positionWS); // use original positionWS for lighting
        light.shadowAttenuation = AdditionalLightRealtimeShadow(perObjectLightIndex, shadowTestPosWS); // use offseted positionWS for shadow test

        // Different function used to shade additional lights.
        additionalLightSumResult += ShadeSingleLight(surfaceData, lightingData, light, true);
    }
#endif
    //==============================================================================================

    // emission
    float3 emissionResult = ShadeEmission(surfaceData, lightingData);

    return CompositeAllLightResults(indirectResult, mainLightResult, additionalLightSumResult, emissionResult, surfaceData, lightingData);
}



float3 ConvertSurfaceColorToOutlineColor(float3 originalSurfaceColor)
{
    return originalSurfaceColor * _OutlineColor;
}
