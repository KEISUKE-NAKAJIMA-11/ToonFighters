struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer CONSTANT_BUFFER : register(b0)
{
    float4 material_color;
};
