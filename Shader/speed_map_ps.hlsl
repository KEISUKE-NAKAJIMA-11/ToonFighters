#include "model_phone.hlsli"

float4 main(VS_OUT input) :  SV_TARGET
{
	float4 FinalColor = float4(input.Speed.x,input.Speed.y,1,1);

	return FinalColor;
}