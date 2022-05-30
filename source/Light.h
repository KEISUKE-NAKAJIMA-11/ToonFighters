#pragma once
#include <DirectXMath.h>
using namespace DirectX;


struct SPOTLIGHT {
	float index;
	float range;//Œõ‚Ì“Í‚­”ÍˆÍ
	float type; //—LŒø‚©–³Œø‚©
	float inner_corn;
	float outer_corn;
	float dumy0;
	float dumy1;
	float dumy2;
	XMFLOAT4 pos;
	XMFLOAT4 color;
	XMFLOAT4 dir;
};

struct POINTLIGHT {
	float index;
	float range;//Œõ‚Ì“Í‚­”ÍˆÍ
	float type; //—LŒø‚©–³Œø‚©
	float dumy;
	XMFLOAT4 pos;
	XMFLOAT4 color;
};

class Light
{
private:
public:
	static const int POINTMAX = 32;
	static const int SPOTMAX = 32;

	static XMFLOAT4 LightDir;
	static XMFLOAT4 DirLightColor;
	static XMFLOAT4 Ambient;
	static POINTLIGHT PointLight[POINTMAX];
	static SPOTLIGHT SpotLight[SPOTMAX];

	static void Init();
	static void SetDirLight(XMFLOAT3 dir, XMFLOAT3 color);

	static void SetAmbient(XMFLOAT3 amb);

	static void SetPointLight(int index, XMFLOAT3 pos, XMFLOAT3 color, float range);

	static void SetSpotLight(int index, XMFLOAT3 pos, XMFLOAT3 color, XMFLOAT3 dir, float range, float inner, float outer);

};


