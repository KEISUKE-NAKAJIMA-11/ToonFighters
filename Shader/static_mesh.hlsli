// UNIT.12
struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	// UNIT.13
	float2 texcoord : TEXCOORD;
};

cbuffer CONSTANT_BUFFER : register(b0)
{
	row_major float4x4 world_view_projection;
	row_major float4x4 world_inverse_transpose;
	float4 material_color;
	float4 light_direction;
};