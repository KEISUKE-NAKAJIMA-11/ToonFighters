#include "model.hlsli"

VS_OUT main(
	float4 position     : POSITION,
	float3 normal : NORMAL,
	float2 texcoord : TEXCOORD,
	//float4 color:COLOR,
	float4 bone_weights : WEIGHTS,
	uint4  bone_indices : BONES
)
{
	float3 p = { 0, 0, 0 };
	float3 n = { 0, 0, 0 };
	for (int i = 0; i < 4; i++)
	{
		p += (bone_weights[i] * mul(position, bone_transforms[bone_indices[i]])).xyz;
		n += (bone_weights[i] * mul(float4(normal.xyz, 0), bone_transforms[bone_indices[i]])).xyz;
	}

	VS_OUT vout;
	vout.position = mul(float4(p, 1.0f), view_projection);

	float3 N = normalize(n);
	float3 L = normalize(-light_direction.xyz);
	float d = dot(L, N);
	vout.color.xyz = material_color.xyz * max(0, d);
	vout.color.w = material_color.w;
	vout.texcoord = texcoord;

	return vout;
}
