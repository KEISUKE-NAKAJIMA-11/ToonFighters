#include "model_phone.hlsli"

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


    VS_OUT vout = (VS_OUT) 0;
    vout.position = float4(p, 1.0f);
	// SSL_03
    position.w = 1;
	//vout.world_position = mul(position, world);
	//vout.world_normal = normalize(mul(normal, world));
    vout.color.xyz = material_color.xyz;
    vout.color.w = material_color.w;
    vout.world_position = float4(p.xyz, 1);
    vout.world_normal = float4(n.xyz, 0);
    //Ú‹óŠÔ‚RŽ²‚Ìì¬
    float3 VZ = vout.world_normal;
    float3 VY = { 0, 1, 0.001 }; //‰¼
    float3 VX;
    VY = normalize(VY);
    VX = normalize(cross(VZ, VY)); //ŠOÏ

    vout.VX = VX;
    vout.VY = VY;
    vout.VZ = VZ;
    
    vout.texcoord = texcoord;
    return vout;
}
