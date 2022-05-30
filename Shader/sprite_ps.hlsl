Texture2D diffuse_map : register(t0);
SamplerState diffuse_map_sampler_state : register(s0);

// UNIT.02
#include "sprite.hlsli"
float4 main(VS_OUT pin) : SV_TARGET
{
	//return pin.color;

	// UNIT.04
	return diffuse_map.Sample(diffuse_map_sampler_state, pin.texcoord) * pin.color;
}
