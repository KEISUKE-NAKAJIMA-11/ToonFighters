#ifndef Include_NiloOutlineUtil
#define Include_NiloOutlineUtil
#include "model_simple_urp_toon.hlsli"
float GetCameraFOV()
{
    float t = projection._m11;
    float Rad2Deg = 180 / 3.1415;
    float fov = atan(1.0f / t) * 2.0 * Rad2Deg;
    return fov;
}
float ApplyOutlineDistanceFadeOut(float inputMulFix)
{
    //make outline "fadeout" if character is too small in camera's view
    return saturate(inputMulFix);
}
float GetOutlineCameraFovAndDistanceFixMultiplier(float positionVS_Z)
{
    float cameraMulFix;

        ////////////////////////////////
        // Perspective camera case
        ////////////////////////////////

        // keep outline similar width on screen accoss all camera distance       
        cameraMulFix = abs(positionVS_Z);

        // can replace to a tonemap function if a smooth stop is needed
        cameraMulFix = ApplyOutlineDistanceFadeOut(cameraMulFix);

        // keep outline similar width on screen accoss all camera fov
        cameraMulFix *= GetCameraFOV();
    
    
    //else
    //{
    //    ////////////////////////////////
    //    // Orthographic camera case
    //    ////////////////////////////////
    //    float orthoSize = abs(unity_OrthoParams.y);
    //    orthoSize = ApplyOutlineDistanceFadeOut(orthoSize);
    //    cameraMulFix = orthoSize * 50; // 50 is a magic number to match perspective camera's outline width
    //}

    return cameraMulFix * 0.00005; // mul a const to make return result = default normal expand amount WS
}

#endif