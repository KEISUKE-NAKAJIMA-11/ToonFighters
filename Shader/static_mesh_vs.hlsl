#include "static_mesh.hlsli"

VS_OUT main(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD/*UNIT.13*/)
{
	VS_OUT vout;
	vout.position = mul(position, world_view_projection);

	normal.w = 0;
	float4 N = normalize(mul(normal, world_inverse_transpose));

	float4 L = normalize(-light_direction);
	vout.color = /*material_color * */max(0, dot(L, N));
	vout.color.a = 1;// material_color.a;

	// UNIT.13
	vout.texcoord = texcoord;

	return vout;
}