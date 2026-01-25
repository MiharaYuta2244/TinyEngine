#include "Object3d.hlsli"

struct OutlineData
{
    float4 color;
    float thickness;
};

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
ConstantBuffer<OutlineData> gOutline : register(b7);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    // 法線方向に押し出す
    float4 offsetPosition = input.position;
    offsetPosition.xyz += input.normal * gOutline.thickness;
    
    output.position = mul(offsetPosition, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul((float3x3) gTransformationMatrix.WorldInverseTranspose, input.normal));
    output.worldPosition = mul(offsetPosition, gTransformationMatrix.World).xyz;
    
    return output;
}