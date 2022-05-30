#include "Light.h"
#include <d3d11.h>

XMFLOAT4 Light::LightDir(1, 1, 1, 1);
XMFLOAT4 Light::DirLightColor(1, 1, 1, 1);
XMFLOAT4 Light::Ambient(0.2f, 0.2f, 0.2f, 1);
POINTLIGHT Light::PointLight[POINTMAX];
SPOTLIGHT Light::SpotLight[SPOTMAX];

void Light::Init()
{
	ZeroMemory(PointLight, sizeof(POINTLIGHT) * POINTMAX);
	ZeroMemory(SpotLight, sizeof(SPOTLIGHT) * SPOTMAX);

}


void Light::SetDirLight(XMFLOAT3 dir, XMFLOAT3 color)
{
	float d = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

	if (d > 0) { dir.x /= d; dir.y /= d; dir.z /= d; }
	LightDir = XMFLOAT4(dir.x, dir.y, dir.z, 0);
	DirLightColor = XMFLOAT4(color.x, color.y, color.z, 1);

}
void Light::SetAmbient(XMFLOAT3 amb)
{
	Ambient = { amb.x,amb.y,amb.z,0 };

}
void Light::SetPointLight(int index, XMFLOAT3 pos, XMFLOAT3 color, float range)
{
	if (index < 0) return;
	if (index >= POINTMAX)return;
	PointLight[index].index = (float)index;
	PointLight[index].range = range;
	PointLight[index].type = 1.0f;
	PointLight[index].dumy = 0.0f;
	PointLight[index].pos = XMFLOAT4(pos.x, pos.y, pos.z, 0);
	PointLight[index].color = XMFLOAT4(color.x, color.y, color.z, 0);

}
void Light::SetSpotLight(int index, XMFLOAT3 pos, XMFLOAT3 color, XMFLOAT3 dir,
	float range, float inner_corn, float outer_corn)
{
	if (index < 0) return;
	if (index >= SPOTMAX)return;
	float d = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if (d > 0) {
		dir.x /= d; dir.y /= d; dir.z /= d;
	}

	SpotLight[index].index = (float)index;
	SpotLight[index].range = range;
	SpotLight[index].type = 1.0f;
	SpotLight[index].inner_corn = inner_corn;
	SpotLight[index].outer_corn = outer_corn;
	SpotLight[index].dumy0 = 0.0f;
	SpotLight[index].dumy1 = 0.0f;
	SpotLight[index].dumy2 = 0.0f;

	SpotLight[index].pos = XMFLOAT4(pos.x, pos.y, pos.z, 0);
	SpotLight[index].color = XMFLOAT4(color.x, color.y, color.z, 0);
	SpotLight[index].dir = XMFLOAT4(dir.x, dir.y, dir.z, 0);
}


