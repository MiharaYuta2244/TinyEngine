#include "Object3d.hlsli"

struct OutlineData
{
    float4 color;
    float thickness;
};

ConstantBuffer<OutlineData> gOutline : register(b7);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gOutline.color;
    return output;
}