// UNIT.16
struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	// UNIT.17
	float2 texcoord : TEXCOORD;
};

#define MAX_BONES 32
cbuffer CONSTANT_BUFFER : register(b0)
{
	row_major float4x4 world_view_projection;
	row_major float4x4 world;
	float4 material_color;
	float4 light_direction;
	// UNIT.21
	row_major float4x4 bone_transforms[MAX_BONES];
};
