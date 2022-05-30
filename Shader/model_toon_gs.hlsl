#include "model_toon.hlsli"
[maxvertexcount(6)]
void main(triangle VS_OUT input[3], inout TriangleStream<GS_OUT> output)
{
    GS_OUT element;
    //// 通常の面を作成 
    for (int i = 0; i < 3; i++)
    {
        element.position = mul(input[i].position, view_projection);
        element.world_normal = input[i].world_normal;
        element.world_position = input[i].world_position;
        element.texcoord = input[i].texcoord;
        element.color = input[i].color;
        element.Speed = input[i].Speed;
        element.VX = input[i].VX;
        element.VY = input[i].VY;
        element.VZ = input[i].VZ;
        output.Append(element); // ここで頂点を生成している 
    }
    output.RestartStrip(); // 追加した頂点で新しいプリミティブを生成する 
    // 裏面で拡大して出力を作成
    // ～考えてみましょう～
    for (int j = 2; j >= 0; --j)
    {
#if 1   // worldPos = mul(worldPos,worldMatrix);
        float3 world_normal = normalize(input[j].world_normal.xyz);
        float4 world_position = float4(input[j].world_position.xyz + world_normal * outoline.x, 1);
        element.position = mul(world_position, view_projection);
#else
        element.position = input[j].position + float4(input[j].normal.xyz, 0);
#endif 
        element.world_normal = float4(input[j].world_normal.xyz,1);
        element.world_position = world_position;
        element.texcoord = input[j].texcoord;
        element.color = float4(0, 0, 0, 1);
        element.Speed = input[j].Speed;
        element.VX = input[j].VX;
        element.VY = input[j].VY;
        element.VZ = input[j].VZ;
        output.Append(element); // ここで頂点を生成している 
    }
}
