#include "model_phone.hlsli"

//
//ポストプロセス（最終的にブラーをかける）バーテックスシェーダー
//
VS_OUT main(
	float4 position     : POSITION,
	float3 normal : NORMAL,
	float2 texcoord : TEXCOORD,
	//float4 color:COLOR,
	float4 bone_weights : WEIGHTS,
	uint4  bone_indices : BONES)
{
	VS_OUT output = (VS_OUT)0;
	output.position = position;
	output.texcoord = texcoord;

	return output;
}