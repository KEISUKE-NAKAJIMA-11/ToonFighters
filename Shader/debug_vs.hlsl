
#include "debug.hlsli"

VS_OUT main(float4 position : POSITION)
{
    VS_OUT vout;
    vout.position = mul(position, worldViewProjection);
    vout.color = color;
    vout.color.a = color.a;
    return vout;
}