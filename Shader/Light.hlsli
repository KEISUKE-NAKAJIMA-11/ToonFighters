
#include "caluculation_function.hlsli"

//-----------------------------------------
//	スポットライト構造体
//-----------------------------------------
struct SPOTLIGHT
{
    float index;
    float range; //光の届く範囲
    float type; //有効か無効か
    float inner_corn;
    float outer_corn;
    float dummy0;
    float dummy1;
    float dummy2;
    float4 pos;
    float4 color;
    float4 dir;
};



//--------------------------------------------
//	ポイントライト構造体
//--------------------------------------------

struct POINTLIGHT
{
    float index;
    float range; //光の届く範囲
    float type; //有効か無効か
    float dummy;
    float4 pos;
    float4 color;
};

//--------------------------------------------
//	グローバル変数
//--------------------------------------------


static const int SPOTMAX = 32;


static const int POINTMAX = 32;
cbuffer CBPerFrame2 : register(b4)
{
    POINTLIGHT PointLight[POINTMAX];
    SPOTLIGHT SpotLight[SPOTMAX];
    float3 A;
    float dummy;
};

//--------------------------------------------
//	スポットライト関数
//--------------------------------------------

// P:
//ピクセルワールド位置
// N : ワールド法線(正規化済み)
// E: 視線方向ベクトル(正規化済み)
// Kd:
//拡散反射率
// Ks : 鏡面反射率

float3 AddSpotLight(float3 P, float3 N, float3 E, float3 Kd, float3 Ks)
{
    float3 SL;
    float3 SC;
    float influence = 1.0;
    float3 SD = (float3) 0;
    float3 SS = (float3) 0;
    for (int j = 0; j < SPOTMAX; j++)
    {
		//点灯していないライト除外
        if (SpotLight[j].type == 0)
            continue;
        SL = P - SpotLight[j].pos.xyz;
        float d = length(SL);
        float r = SpotLight[j].range;
		//届かないライト除外
        if (d > r)
            continue;
        float3 SFront = normalize(SpotLight[j].dir.xyz);
        SL = normalize(SL);
        float angle = dot(SL, SFront);
        float area = SpotLight[j].inner_corn - SpotLight[j].outer_corn;
        float influence2 = SpotLight[j].inner_corn - angle;
        influence2 = saturate(1.0 - influence2 / area);
        SC = SpotLight[j].color.rgb;
        SD += ToonDiffuse(N, SL, SC, Kd) * influence * influence2;
        SS += ToonSpecular(N, SL, SC, E, Ks, 20) * influence * influence2;
    }
    return (SD + SS);
}


//--------------------------------------------
//	ポイントライト関数
//--------------------------------------------

// P: ピクセルワールド位置
// N: ワールド法線(正規化済み)
// E: 視線方向ベクトル(正規化済み)
// Kd: 拡散反射率
// Ks: 鏡面反射率 

float3 AddPointLight(float3 P, float3 N, float3 E, float3 Kd, float3 Ks)
{
    float3 PL; //ポイントライトベクトル
    float3 PC; //ポイントライト色
    float influence; //ポイントライトの影響
    float3 PD = (float3) 0; //拡散反射
    float3 PS = (float3) 0; //鏡面反射
    for (int i = 0; i < POINTMAX; i++)
    {
		//点灯していないライト除外
        if (PointLight[i].type == 0)
            continue;
        PL = P - PointLight[i].pos.xyz;
        float d = length(PL);
        float r = PointLight[i].range;
		//届かないライト除外
        if (d > r)
            continue;
        influence = saturate(10.0f - d / r);
        PL = normalize(PL);
        PC = PointLight[i].color.rgb;
		//拡散光の加算		
        PD += ToonDiffuse(N, PL, PC, Kd) * influence;
		//鏡面光の加算
        PS += ToonSpecular(N, PL, PC, E, Ks, 0.1f) * influence;
    }
    return (PD + PS);
}

