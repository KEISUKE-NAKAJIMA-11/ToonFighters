#include "model_simple_urptoon_utilfunctions.hlsli"

float4 main(VS_OUT pin) : SV_TARGET
{
    /////////////////////////////////////////////////////////////////////////////////////////
    // first prepare all data for lighting function
    //////////////////////////////////////////////////////////////////////////////////////////

    // fillin ToonSurfaceData struct:
    ToonSurfaceData surfaceData = InitializeSurfaceData(pin);

    // fillin LightingData struct:
    LightingData lightingData = InitializeLightingData(pin);
 
    // apply all lighting calculation
    float3 color = ShadeAllLights(surfaceData, lightingData);


     //color = ConvertSurfaceColorToOutlineColor(color);


   // color = Fog(color, pin);

    return float4(color, surfaceData.alpha);
}
