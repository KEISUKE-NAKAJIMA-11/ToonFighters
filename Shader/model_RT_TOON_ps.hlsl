#include "model_RT_TOON.hlsli"
#include "Light.hlsli"



Texture2D diffuse_map : register(t0);
SamplerState diffuse_map_sampler_state : register(s0);
Texture2D normal_map : register(t1);
SamplerState normal_map_sampler_state : register(s1);


sampler shadow_sampler = sampler_state
{
    Texture = <shadow_texture>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};



float4 main(VS_OUT pin) : SV_TARGET
{
    float4 color = diffuse_map.Sample(diffuse_map_sampler_state, pin.texcoord) * pin.color;
	
    
    
    return color;

}
