#include "model_phone.hlsli"

float3 fresnel_schlick(float NoL, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - NoL, 5.0);
}


Texture2D diffuse_map : register(t0);
SamplerState diffuse_map_sampler_state : register(s0);

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

#if 1

	float4 diffuse_color = diffuse_map.Sample(diffuse_map_sampler_state, pin.texcoord);
	float alpha = diffuse_color.a;
#else

	float4 diffuse_color = float4(1, 1, 1, 1);
	float alpha = diffuse_color.a;
#endif
	//vectors
	float3 N = normalize(pin.world_normal.xyz);
	float3 L = normalize(-light_direction.xyz);
	float3 V = normalize(eye_position.xyz - pin.world_position.xyz);
    float3 R = normalize(reflect(-L, N));
    float3 H = normalize(L+V);

	float4 ambient_light = float4(0.2, 0.2, 0.2, 1.0);
	//irradiance:è∆ìx
	float3 irradiance = pin.color.rgb * max(0, dot(N, L)) + ambient_light.rgb;
#if 1
	float tones = 2;
	irradiance = ceil(irradiance * tones) / tones;
	//return float4(irradiance, 1);
#endif
	float3 reflectance = fresnel_schlick(max(0,dot(N,L)),float3(0.2,0.2,0.2));
	//debug::return float4(reflectance, 1);
	//exitance:åıë©î≠éUìx
	float3 diffuse_exitance = diffuse_color.rgb * irradiance * (1 - reflectance);
	float3 diffuse_radiance = diffuse_exitance;
	float smmothness = 1024;

	
	float4 specula_color = float4(1, 1, 1, 1);
	float3 specula_exitance = specula_color.rgb * irradiance;
	float3 specula_radiance = specula_exitance * pow(max(0,dot(N, H)), smmothness);
	//debug::return float4(specula_radiance, 1);
	//radiance :ãPìx
	float3 radiance = diffuse_radiance /*+ specula_radiance*/;
   // return float4(N, 1);
	#if 0
	//rim lighting
    float rim_contribution = smoothstep(0.6, 1.0, 1.0 - max(0, dot(V, N)));
    //float rim_contribution = step(-0.2, dot(V, N));
    //rim_contridution = pow(rim_contridution, 32);
    //rim_contridution = smoothstep(0.6, 1.0,rim_contridution);
    float3 rim_color = diffuse_color.rgb * rim_contribution * float3(1, 0, 0);
    radiance += rim_color;
	#endif
	return float4(radiance, alpha);
}
