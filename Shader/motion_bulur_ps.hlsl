#include "model_phone.hlsli"
Texture2D g_texScene: register(t0);
Texture2D g_texVelocityMap: register(t1);
SamplerState g_samLinear : register(s0);
//
//ポストプロセス（最終的にブラーをかける）ピクセルシェーダー
//
float4 main(VS_OUT input) : SV_TARGET
{
	float4 FinalColor;

	float4 Velocity = g_texVelocityMap.Sample(g_samLinear, input.texcoord);
	float2 HalfVelocity = Velocity / 2;

	float NumSample = 6;
	for (float i = 0; i < NumSample; i++)
	{
		float2 lookup = input.texcoord + Velocity * (i / NumSample) - HalfVelocity;
		float4 Color = g_texScene.Sample(g_samLinear, lookup);
		FinalColor += Color;
	}
	FinalColor /= NumSample;

	return FinalColor;
}