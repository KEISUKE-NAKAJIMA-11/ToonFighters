#include "model_simple_urptoon_utilfunctions.hlsli"

#define TRANSFORM_TEX(tex,name) (tex.xy * name##_ST.xy + name##_ST.zw)

VS_OUT main(
	float4 position : POSITION,
	float3 normal : NORMAL,
	float2 texcoord : TEXCOORD,
	//float4 color:COLOR,
	float4 bone_weights : WEIGHTS,
	uint4 bone_indices : BONES
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
    vout.world_position = float4(p.xyz, 1);
    vout.world_normal = float4(n.xyz, 1);
    
    //float3 positionWS = vout.world_position.xyz;
    
  //  positionWS = TransformPositionWSToOutlinePositionWS(vout.world_position.xyz, vout.position.z, vout.world_normal.xyz);
    
  //  vout.position = mul(float4(positionWS.xyz, 1.0f), view_projection);
    
	// SSL_03
    position.w = 1;
	//vout.world_position = mul(position, world);
	//vout.world_normal = normalize(mul(normal, world));
    vout.color.xyz = material_color.xyz;
    vout.color.w = material_color.w;
    

 


 
    vout.texcoord = texcoord;
    
    

    ////// [Read ZOffset mask texture]
    ////// we can't use tex2D() in vertex shader because ddx & ddy is unknown before rasterization, 
    ////// so use tex2Dlod() with an explict mip level 0, put explict mip level 0 inside the 4th component of param uv)
    //float outlineZOffsetMaskTexExplictMipLevel = 0;
    //float outlineZOffsetMask = albedo_texture.SampleLevel(_OutlineZOffsetMaskTex, float4(vout.texcoord, 0, outlineZOffsetMaskTexExplictMipLevel), 1.0f).r; //we assume it is a Black/White texture

    ////// [Remap ZOffset texture value]
    ////// flip texture read value so default black area = apply ZOffset, because usually outline mask texture are using this format(black = hide outline)
    //outlineZOffsetMask = 1 - outlineZOffsetMask;
    //outlineZOffsetMask = invLerpClamp(_OutlineZOffsetMaskRemapStart, _OutlineZOffsetMaskRemapEnd, outlineZOffsetMask); // allow user to flip value or remap

    ////// [Apply ZOffset, Use remapped value as ZOffset mask]
    //vout.position = NiloGetNewClipPosWithZOffset(vout.position, _OutlineZOffset * outlineZOffsetMask + 0.03 * _IsFace);

    
    
    return vout;
}
