// UNIT.16
#include "skinned_mesh.hlsli"

VS_OUT main(
	float4 position : POSITION, 
	float4 normal : NORMAL, 
	float2 texcoord : TEXCOORD/*UNIT.17*/,
	float4 bone_weights : WEIGHTS/*UNIT.20*/,
	uint4  bone_indices : BONES/*UNIT.20*/)
{
	// UNIT.21
	float3 p = { 0, 0, 0 };
	float3 n = { 0, 0, 0 };
	for (int i = 0; i < 4; i++)
	{
		p += (bone_weights[i] * mul(position, bone_transforms[bone_indices[i]])).xyz;
		n += (bone_weights[i] * mul(float4(normal.xyz, 0), bone_transforms[bone_indices[i]])).xyz;
	}
	position = float4(p, 1.0f);
	normal = float4(n, 0.0f);

	VS_OUT vout;
	vout.position = mul(position, world_view_projection);

	normal.w = 0;
	float4 N = normalize(mul(normal, world));

	float4 L = normalize(-light_direction);
	vout.color = material_color * max(0, dot(L, N));
	vout.color.a = material_color.a;

	// UNIT.20
	/*float4 influence = { 0, 0, 0, 1 };
	for (int i = 0; i < 4; i++)
	{
		float weight = bone_weights[i];
		if (weight > 0.0f)
		{
			switch (bone_indices[i])
			{
			case 0:
				influence.r = weight;
				break;
			case 1:
				influence.g = weight;
				break;
			case 2:
				influence.b = weight;
				break;
			}
		}
	}
	 vout.color = influence;*/

	// UNIT.17
	vout.texcoord = texcoord;

	return vout;
}
