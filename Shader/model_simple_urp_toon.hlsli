// all sampler2D don't need to put inside CBUFFER 
Texture2D albedo_texture : register(t0);
Texture2D emmision_texture : register(t1);
Texture2D oclusion_texture : register(t2);
Texture2D outlinemask_texture : register(t3);

SamplerState _BaseMap : register(s0);
SamplerState _EmissionMap : register(s1);
SamplerState _OcclusionMap : register(s2);
SamplerState _OutlineZOffsetMaskTex : register(s3);



struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;

    float4 world_position : WORLD_POSITION;
    float4 world_normal : WORLD_NORMAL;
    float2 Speed : TEXCOORD1;
};



struct GS_OUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;

    float4 world_position : WORLD_POSITION;
    float4 world_normal : WORLD_NORMAL;
    float2 Speed : TEXCOORD1;
};



cbuffer CbScene : register(b0)
{
    row_major float4x4 view_projection;
    row_major float4x4  projection;
    float4 light_direction;
    row_major float4x4 da;
    row_major float4x4 d2;
    float4 eye_position;
    float4 light_color;
    float4 outoline;
};


#define MAX_BONES 32
cbuffer CbMesh : register(b1)
{
    row_major float4x4 bone_transforms[MAX_BONES];
};

cbuffer CbSubset : register(b2)
{
    float4 material_color;
    row_major float4x4 world_view_projection;
    row_major float4x4 world;
};



cbuffer CbUrptoon : register(b3)
{


    // base color
    float4 _BaseMap_ST;
    float4 _BaseColor;//36
    
        // high level settings
    float _IsFace; //4
     // alpha
    float _Cutoff;//4
        // emission
    float _UseEmission;
    
    float _EmissionMulByBaseColor;
    

    float3 _EmissionColor;
    float dummy1;

    float3 _EmissionMapChannelMask;//32
    float dummy2;

    // occlusion
    float _UseOcclusion;
    float _OcclusionStrength;
    float _OcclusionRemapStart;
    float _OcclusionRemapEnd; //32
    
    float4 _OcclusionMapChannelMask;

    
    float _CelShadeMidPoint; //16
    float _CelShadeSoftness;
    // shadow mapping
    float _ReceiveShadowMappingAmount; //8
    float _ReceiveShadowMappingPosOffset;
    

    // lighting
    float3 _IndirectLightMinColor;
    float dummy3;
 
    float3 _ShadowMapColor;//16
    float dummy4;

    // outline
    float _OutlineWidth;
    float _OutlineZOffset;
    float _OutlineZOffsetMaskRemapStart;
    float _OutlineZOffsetMaskRemapEnd; //12
    
    
    float3 _OutlineColor;//16
    float dummy5;

   

};


struct ToonSurfaceData
{
    float3 albedo;
    float alpha;
    float3 emission;
    float occlusion;
};

struct LightingData
{
    float3 normalWS;
    float3 positionWS;
    float3 viewDirectionWS;
    float4 shadowCoord;
};
