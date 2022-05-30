#include "model_phone.hlsli"

VS_OUT main(
	float4 position     : POSITION,
	float3 normal : NORMAL,
	float2 texcoord : TEXCOORD,
	//float4 color:COLOR,
	float4 bone_weights : WEIGHTS,
	uint4  bone_indices : BONES)
{

	float3 p = { 0, 0, 0 };
	float3 n = { 0, 0, 0 };
	for (int i = 0; i < 4; i++)
	{
		p += (bone_weights[i] * mul(position, bone_transforms[bone_indices[i]])).xyz;
		n += (bone_weights[i] * mul(float4(normal.xyz, 0), bone_transforms[bone_indices[i]])).xyz;
	}


	VS_OUT vout;
	float4 CurPos = mul(float4(p, 1.0f), view_projection);
    float4 PrevPos = mul(float4(p, 1.0f), prevview_projection);

	vout.world_position = float4(p.xyz, 1);
	vout.world_normal = float4(n.xyz, 1);

	vout.position = CurPos;

	PrevPos /= PrevPos.w;

	CurPos /= CurPos.w;


	vout.Speed = CurPos - PrevPos;
	vout.Speed *= 500;//本来は不要。効果を分かりやすくするためスピードを増幅（ひいてはブラーを増幅することに）


	return vout;

}