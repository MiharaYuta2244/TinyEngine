#include "Object3d.hlsli"

struct TransformationMatrix
{
    row_major float4x4 WVP;
    row_major float4x4 World;
    row_major float4x4 WorldInverseTranspose;
};

StructuredBuffer<TransformationMatrix> gInstances : register(t0);

VertexShaderOutput main(float4 position : POSITION0, float2 texcoord : TEXCOORD0,
                        float3 normal : NORMAL0, uint instanceId : SV_InstanceID)
{
    TransformationMatrix instance = gInstances[instanceId];
    VertexShaderOutput output;
    output.position = mul(position, instance.WVP);
    output.texcoord = texcoord;
    output.normal = normalize(mul(normal, (float3x3)instance.WorldInverseTranspose));
    output.worldPosition = mul(position, instance.World).xyz;
    return output;
}
