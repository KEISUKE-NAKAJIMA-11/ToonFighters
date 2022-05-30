

struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float4 color    : COLOR;

	float4 world_position : WORLD_POSITION;
	float4 world_normal : WORLD_NORMAL;
	float2 Speed : TEXCOORD1;
    float3 VX : TEXCOORD2; //接空間用X軸
    float3 VY : TEXCOORD3; //接空間用Y軸
    float3 VZ : TEXCOORD4; //接空間用Z軸
};

cbuffer CbScene : register(b0)
{
    row_major float4x4 view;
    row_major float4x4 view_projection;
    row_major float4x4 prevview_projection;
    float4 light_direction;
    row_major float4x4 da;
    row_major float4x4 d2;
    float4 eye_position;
    float4 light_color;
    float4 outoline;
    float4 camera_front;

};

#define MAX_BONES 32
cbuffer CbMesh : register(b1)
{
	row_major float4x4	bone_transforms[MAX_BONES];

};

cbuffer CbSubset : register(b2)
{
    float4 material_color;
    row_major float4x4 world_view_projection;
    row_major float4x4 world;
    
    float4 xModelBaseColor; //モデルの色 rgb:color a:alpha
    float4 xModelEmissionColor; //モデル全体を発光させる rgb:color a:strength
    float4 xModelEmissionEdgeColor; //モデルの縁を発光させる rgb:color a:strength

    float3 xModelLocalLightVec; //モデルのローカルライトの向き
    float xModelEdgeThreshold; //モデルのリムライトの閾値

    float2 dammy; //表情用のテクスチャオフセット
    float xModelShadowBias; //シャドウマップの閾値
    float xModelToonThreshold; //トゥーンの明暗の閾値

    int xModelReceivedShadow; //シャドウを受けるかどうか 1:受ける 0:受けない
    float xModelZoffset; //モデルをz方向へいくつずらすかどうか
    float xModelpadding1;
    float xModelpadding2;
    
};

cbuffer CBEnvironment : register(b4)
{
    float4 xEnvironmentAmbientBrightColor; //rgb:トゥーンの明るい色にブレンドされる環境色 a:ブレンド率
    float4 xEnvironmentAmbientDarkColor; //rgb:トゥーンの暗い色（影も含む）にブレンドされる環境色 a:ブレンド率
    row_major float4x4 xEnvironmentGlobalLightVP; //ライトのビュープロジェクション行列
	
    float3 xEnvironmentGlobalLightVec; //影生成用のライトベクトル
    float xEnvironmentPadding1;
};

