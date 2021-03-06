#include "bloom.hlsli"

Texture2D hdr_colour_map : register(t0);
SamplerState linear_border_sampler_state : register(s0);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	uint mip_level = 0, width, height, number_of_levels;
	hdr_colour_map.GetDimensions(mip_level, width, height, number_of_levels);
	const float aspect_ratio = width / height;

#if 1
	
	const float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
	const float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

	float4 fragment_colour = hdr_colour_map.Sample(linear_border_sampler_state, texcoord) * weight[0];
	for (int i = 1; i < 3; i++)
	{
		fragment_colour += hdr_colour_map.Sample(linear_border_sampler_state, texcoord + float2(0.0, offset[i] / height)) * weight[i];
		fragment_colour += hdr_colour_map.Sample(linear_border_sampler_state, texcoord - float2(0.0, offset[i] / height)) * weight[i];
	}
#else
	
	const float offset[2] = { 0.53805, 2.06278 };
	const float weight[2] = { 0.44908, 0.05092 };
	float4 fragment_colour = 0;
	for (int i = 0; i < 2; i++)
	{
		fragment_colour += hdr_colour_map.Sample(linear_border_sampler_state, texcoord + float2(0.0, offset[i]) / height) * weight[i];
		fragment_colour += hdr_colour_map.Sample(linear_border_sampler_state, texcoord - float2(0.0, offset[i]) / height) * weight[i];
	}
#endif
	return fragment_colour;
}
