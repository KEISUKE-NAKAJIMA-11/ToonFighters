struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    float4 boneWeights : WEIGHTS;
    uint4 boneIndices : BONES;
};


struct VS_OUT
{
    // Vertex shader output
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;

    float4 world_position : WORLD_POSITION;
    float4 world_normal : WORLD_NORMAL;
    float4x4 world[4] : wmatrix;
    float2 Speed : TEXCOORD1;
    float3 VX : TEXCOORD2; //ê⁄ãÛä‘ópXé≤
    float3 VY : TEXCOORD3; //ê⁄ãÛä‘ópYé≤
    float3 VZ : TEXCOORD4; //ê⁄ãÛä‘ópZé≤
};

//cbuffer CbScene : register(b0)
//{
//    row_major float4x4 view_projection;
//    row_major float4x4 prevview_projection;
//    float4 light_direction;
//    float4 eye_position;
//    float4 light_color;
//    float4 outoline;
//};


cbuffer CbScene : register(b0)
{
    row_major float4x4 view_projection;
    row_major float4x4 prevview_projection;
    float4 light_direction;
    row_major float4x4 da;
    row_major float4x4 d2;
    float4 eye_position;
    float4 light_color;
    float4 outoline;
};


#define MAX_BONES 68
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