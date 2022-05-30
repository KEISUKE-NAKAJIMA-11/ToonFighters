#include "toonliting.hlsli"
#include "Light.hlsli"

float3 fresnel_schlick(float NoL, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - NoL, 5.0);
}

#define DEGAMMA(x)		pow(abs(x), 2.2)		//色を暗くする
#define GAMMA(x)		pow(abs(x), 1.0 / 2.2)	//色の明るくする

Texture2D diffuse_map : register(t0);
SamplerState diffuse_map_sampler_state : register(s0);
Texture2D normal_map : register(t1);
SamplerState normal_map_sampler_state : register(s1);
Texture2D ShadowMapTex : register(t2); //シャドウマップ
SamplerState shadowSampler : register(s2);

float4 main(VS_OUT pin) : SV_TARGET
{
    float shadowBlend = 0;
    
    //ライティング定数
    float4 BaseColor = DEGAMMA(diffuse_map.Sample(diffuse_map_sampler_state, pin.texcoord)) * xEnvironmentAmbientBrightColor;
    float4 ShadowColor = BaseColor /*DEGAMMA(ShadowColorTex.Sample(texSampler, input.Tex))*/ * xEnvironmentAmbientDarkColor;
    float4 EmissionColor = /*EmissionTex.Sample(texSampler, input.Tex)*/float4(0.0f, 0.0f, 0.0f, 1.0f);
    const float4 LitAmount = /*LitTex.Sample(texSampler, input.Tex)*/float4(0.01f, 1.0f, 0.0f, 1.0f);

	//拡散反射
    const float3 N = normalize(pin.world_normal);
    const float3 F = normalize(camera_front.xyz);
    const float3 L = normalize(light_direction.xyz);
    const float3 E = normalize(eye_position.xyz - pin.world_position);
    float D = dot(L, N) * LitAmount.g;
    D = step(xModelToonThreshold, D);

	//スペキュラ反射
    const float3 R = reflect(L, N);
    float S = clamp(dot(R, E), 0, 1);
    S = pow(S * LitAmount.b, LitAmount.r); //LitAmount.rは0より大きくすること

	//発光色
    const float LimFactor = 1.0f - abs(dot(N, F)); //1:edge 0:not edge
    float edgeBlend = step(xModelEdgeThreshold, LimFactor);
    EmissionColor += lerp(xModelEmissionColor, xModelEmissionEdgeColor, edgeBlend);
	
	//リムライト
    const float LimLight = step(0.75f, LimFactor) * 0.05f;
    
    float3 PL = (float3) 0;
    PL = AddPointLight(pin.world_position.xyz, N, E, D, S);
	
	//色の計算
    float4 color = lerp(BaseColor, ShadowColor, D);

    color.rgb = lerp(color.rgb, xEnvironmentAmbientBrightColor.rgb, LimLight * LitAmount.a);
    color.rgb += (EmissionColor.rgb * EmissionColor.a) + (S * xEnvironmentAmbientBrightColor.rgb) + PL;
    color = float4(lerp(color * xModelBaseColor, ShadowColor, shadowBlend).rgb, color.a * xModelBaseColor.a);
	
	//出力
    return color;
}
