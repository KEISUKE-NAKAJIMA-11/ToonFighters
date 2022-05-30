#include "2d_primitive_batch.hlsli"
float4 main(VS_OUT pin) : SV_TARGET
{
    return pin.color;
}