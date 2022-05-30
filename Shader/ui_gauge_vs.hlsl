#include "ui_gauge.hlsli"

VS_OUT main(float4 pos : POSITION)
{
    VS_OUT output;
    output.position = pos;
    output.color = material_color;

    return output;
}