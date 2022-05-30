#include "model_toon.hlsli"
#include "Light.hlsli"

float3 fresnel_schlick(float NoL, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - NoL, 5.0);
}


Texture2D diffuse_map : register(t0);
SamplerState diffuse_map_sampler_state : register(s0);
Texture2D normal_map : register(t1);
SamplerState normal_map_sampler_state : register(s1);

#define DEGAMMA(x)		pow(abs(x), 2.2)		//色を暗くする
#define GAMMA(x)		pow(abs(x), 1.0 / 2.2)	//色の明るくする

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



float4 main(GS_OUT pin) : SV_TARGET
{
    
 #if 1
    float shadowBlend = 0;
  //  if (xModelReceivedShadow > 0)
  //  {
  //      float4 LightSpacePos = mul(float4(pin.world_position, 1.0f), xEnvironmentGlobalLightVP);
  //      LightSpacePos /= LightSpacePos.w;
  //      LightSpacePos.x = LightSpacePos.x * 0.5f + 0.5f;
  //      LightSpacePos.y = 1.0f - (LightSpacePos.y * 0.5f + 0.5f);

		////x4ソフトシャドウ
  //      shadowBlend =  ShadowMapTex.SampleCmpLevelZero(shadowSampler, LightSpacePos.xy, LightSpacePos.z - xModelShadowBias);
  //      shadowBlend += ShadowMapTex.SampleCmpLevelZero(shadowSampler, LightSpacePos.xy, LightSpacePos.z - xModelShadowBias, int2(1, 0));
  //      shadowBlend += ShadowMapTex.SampleCmpLevelZero(shadowSampler, LightSpacePos.xy, LightSpacePos.z - xModelShadowBias, int2(-1, 0));
  //      shadowBlend += ShadowMapTex.SampleCmpLevelZero(shadowSampler, LightSpacePos.xy, LightSpacePos.z - xModelShadowBias, int2(0, 1));
  //      shadowBlend += ShadowMapTex.SampleCmpLevelZero(shadowSampler, LightSpacePos.xy, LightSpacePos.z - xModelShadowBias, int2(0, -1));
  //      shadowBlend /= 5.0f;
  //  }
	
    
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
	
	//色の計算
    float4 color = lerp(BaseColor, ShadowColor, D);
    color.rgb = lerp(color.rgb, xEnvironmentAmbientBrightColor.rgb, LimLight * LitAmount.a);
    color.rgb += (EmissionColor.rgb * EmissionColor.a) + (S * xEnvironmentAmbientBrightColor.rgb);
    color = float4(lerp(color * xModelBaseColor, ShadowColor, shadowBlend).rgb, color.a * xModelBaseColor.a);
	
	//出力
    return color;
    
    
    
    /////////////////////////////////////////////////////////////////
  //  return float4(A, 1.0f);
    
    
 #else   
    
    //return diffuse_map.Sample(diffuse_map_sampler_state, pin.texcoord) * pin.color;
    float4 color = diffuse_map.Sample(diffuse_map_sampler_state, pin.texcoord);

    float3 E = normalize(eye_position.xyz - pin.world_position);
    float3 L = normalize(light_direction.xyz);
    
    
    float3 VX = normalize(pin.VX);
    float3 VY = normalize(pin.VY);
    float3 VZ = normalize(pin.VZ);
	//接空間からワールド空間へ変換する行列
    float3x3 mat = { VX, VY, VZ };

    float3 N = normal_map.Sample(normal_map_sampler_state, pin.texcoord).rgb;
    N = N * 2.0 - 1.0;
	//ノーマルテクスチャ法線をワールドへ変換
    N = normalize(mul(N, mat));
    
	// 環境光
    //float3 A = A;

	//拡散反射
    float3 C = light_color.rgb;
    float3 Kd = float3(1, 1, 1);
    float3 D = ToonLight(N, L, C, Kd);

   
    
	//鏡面反射
    float3 Ks = float3(0.1f, 0.1f, 0.1f);
    float3 S = ToonSpecular(N, L, C, E, Ks, 0.1f);
    
    	// ポイントライト計算
    float3 PL = (float3) 0;
    PL = AddPointLight(pin.world_position.xyz, N, E, Kd, Ks);
    //return float4(PL,1.0f);
    color *= (pin.color * float4(A + D + S + PL, 1.0));
    return color;

  #endif
}



