Texture2D hdr_colour_map : register(t0);


cbuffer CONSTANT_BUFFER : register(b0)
{
	float white_point = 3.0f; 
	float average_gray = 1.147f; 

	float brightness = 0.0;
	float contrast = 0.0;

	float hue = 0.0;
	float saturation = 0.0;


	float offset = 0.0;
	float darkness = 0.0;

	
	float3 colour_filter = float3(1, 1, 1);

	
	float amount = 0.0;
}


float3 brightness_contrast_effect(float3 fragment_colour, float brightness, float contrast)
{
		fragment_colour += brightness;
	if (contrast > 0.0)
	{
		fragment_colour = (fragment_colour - 0.5) / (1.0 - contrast) + 0.5;
	}
	else if (contrast < 0.0)
	{
		fragment_colour = (fragment_colour - 0.5) * (1.0 + contrast) + 0.5;
	}
	return fragment_colour;
}

float3 hue_saturation_effect(float3 fragment_colour, float hue, float saturation)
{
	
	float angle = hue * 3.14159265;
	float s = sin(angle), c = cos(angle);
	float3 weights = (float3(2.0 * c, -sqrt(3.0) * s - c, sqrt(3.0) * s - c) + 1.0) / 3.0;
	float len = length(fragment_colour);
	fragment_colour = float3(dot(fragment_colour, weights.xyz), dot(fragment_colour, weights.zxy), dot(fragment_colour, weights.yzx));
	float average = (fragment_colour.r + fragment_colour.g + fragment_colour.b) / 3.0;
	if (saturation > 0.0)
	{
		fragment_colour += (average - fragment_colour) * (1.0 - 1.0 / (1.001 - saturation));
	}
	else
	{
		fragment_colour += (average - fragment_colour) * (-saturation);
	}
	return fragment_colour;
}

float3 sepia_effect(float3 fragment_colour, float amount)
{

	fragment_colour.r = dot(fragment_colour, float3(1.0 - 0.607 * amount, 0.769 * amount, 0.189 * amount));
	fragment_colour.g = dot(fragment_colour, float3(0.349 * amount, 1.0 - 0.314 * amount, 0.168 * amount));
	fragment_colour.b = dot(fragment_colour, float3(0.272 * amount, 0.534 * amount, 1.0 - 0.869 * amount));
	return fragment_colour;
}

float3 vignette_effect(float3 fragment_colour, float2 uv, float offset, float darkness)
{
	
    uv = (uv - 0.5) * offset;
    fragment_colour = lerp(fragment_colour, (1.0 - darkness), dot(uv, uv));
	return fragment_colour;
}

float3 grayscale_effect(float3 fragment_colour)
{
	return dot(fragment_colour, float3(0.299, 0.587, 0.114));
}

float3 scanline_effect(float3 fragment_colour, uint screen_height, float2 texcoord, float pitch, float ratio = 0)
{
	return fragment_colour * ((fmod(texcoord.y * screen_height, pitch) > 1) ? ratio : 1);
}

Texture2D average_luminance : register(t1);
SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD, uint sample_index : SV_SAMPLEINDEX) : SV_TARGET
{

	float4 colour_map_colour = hdr_colour_map.Sample(point_sampler_state, texcoord);

	float luminance = dot(colour_map_colour.rgb, float3(0.2126, 0.7152, 0.0722));

	uint mip_level = 0, width, height, number_of_levels;
	average_luminance.GetDimensions(mip_level, width, height, number_of_levels);
	float average_luminance_sample = average_luminance.SampleLevel(point_sampler_state, float2(0.5, 0.5), number_of_levels - 1).x;

	// Tone mapping
	float3 fragment_colour = 0;
	colour_map_colour.rgb = max(0, colour_map_colour.rgb);
#if 1

	float scaled_luminance = average_gray / (average_luminance_sample + 0.001);
    float mapped_colour_scale = (scaled_luminance + (scaled_luminance * scaled_luminance) / (white_point * white_point)) / (1.0 + scaled_luminance);
	fragment_colour = colour_map_colour.rgb * mapped_colour_scale;

	
#else
	fragment_colour = colour_map_colour.rgb;
	fragment_colour *= average_gray / (average_luminance_sample + 0.001f);

	fragment_colour *= (1.0f + fragment_colour / white_point);
	fragment_colour /= (1.0f + fragment_colour);
#endif


	fragment_colour = brightness_contrast_effect(fragment_colour, brightness, contrast);
	fragment_colour = hue_saturation_effect(fragment_colour, hue, saturation);
	fragment_colour = sepia_effect(fragment_colour, amount);
	fragment_colour = vignette_effect(fragment_colour, texcoord, offset, darkness);


	return float4(fragment_colour * colour_filter, 1);
}
