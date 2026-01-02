#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    int enableFoging;
};

struct DirectionalLight
{
    float4 color; //!< ライトの色
    float3 direction; //!< ライトの向き
    float intensity; //!< 輝度
};

struct Camera
{
    float3 worldPosition;
};

struct FogParam
{
    float3 fogCenter;
    float radius;
    float3 fogColor;
    float fogIntensity;
};

struct TimeParam
{
    float time;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct PointLight
{
    float4 color; //ライトの色
    float3 position; //ライトの位置
    float intensity; //輝度
    float radius; //ライトの届く最大距離
    float decay; //減衰率
};

struct SpotLight
{
    float4 color; //ライトの色
    float3 position; //ライトの位置
    float intensity; //輝度
    float3 direction; //スポットライトの方向
    float distance; //ライトの届く最大距離
    float decay; //減衰率
    float cosAngle; //スポットライトの余弦
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<FogParam> gFogParam : register(b3);
ConstantBuffer<TimeParam> gTimeParam : register(b4);
ConstantBuffer<PointLight> gPointLight : register(b5);
ConstantBuffer<SpotLight> gSpotLight : register(b6);

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    // textureのa値が0のときにPixelを棄却
    if (textureColor.a < 0.5)
    {
        discard;
    }
    
    // Cameraへの方向を算出
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    
    float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
    float NDotH = dot(normalize(input.normal), halfVector);
    float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
    
    PixelShaderOutput output;
    if (gMaterial.enableLighting != 0) // Lightingする場合
    {
        // DirectionalLight計算
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = (NdotL * 0.5f + 0.5f) * (NdotL * 0.5f + 0.5f);
        
        // 拡散反射 directional
        float3 diffuseDirectionalLight = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        // 鏡面反射 directional
        float3 specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        
        // ポイントライト計算
        float3 pointLightDirection = normalize(gPointLight.position - input.worldPosition);
        float pointNdotL = dot(normalize(input.normal), pointLightDirection);
        float pointCos = saturate(pointNdotL);
        
        // ポイントライトからの距離
        float distance = length(gPointLight.position - input.worldPosition);
        // 距離減衰（二乗則）
        float attenuation = pow(saturate(-distance / gPointLight.radius + 1.0), gPointLight.decay);
        
        // 拡散反射 point
        float3 diffusePointLight = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * pointCos * gPointLight.intensity * attenuation;
        
        // PointLightの半ベクトル
        float3 pointHalfVector = normalize(pointLightDirection + toEye);
        float pointNDotH = dot(normalize(input.normal), pointHalfVector);
        float pointSpecularPow = pow(saturate(pointNDotH), gMaterial.shininess);
   
        // 鏡面反射 point
        float3 specularPointLight = gPointLight.color.rgb * gPointLight.intensity * pointSpecularPow * attenuation * float3(1.0f, 1.0f, 1.0f);
        
        // スポットライト計算
        float3 spotLightDirection = normalize(gSpotLight.position - input.worldPosition);
        float spotNdotL = dot(normalize(input.normal), spotLightDirection);
        float spotCos = saturate(spotNdotL);
    
        // スポットライトからの距離
        float spotDistance = length(gSpotLight.position - input.worldPosition);
        // スポットライトの減衰
        float spotAttenuation = pow(saturate(-spotDistance / gSpotLight.distance + 1.0), gSpotLight.decay);

        // スポットライトの角度減衰
        float spotDirectionCos = dot(-spotLightDirection, normalize(gSpotLight.direction));
        float angleAttenuation = smoothstep(0.0, gSpotLight.cosAngle, spotDirectionCos);
 
        // 拡散反射 spot
        float3 diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * spotCos * gSpotLight.intensity * spotAttenuation * angleAttenuation;
        
        // スポットライトの半ベクトル
        float3 spotHalfVector = normalize(spotLightDirection + toEye);
        float spotNDotH = dot(normalize(input.normal), spotHalfVector);
        float spotSpecularPow = pow(saturate(spotNDotH), gMaterial.shininess);
      
        // 鏡面反射 spot
        float3 specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * spotSpecularPow * spotAttenuation * angleAttenuation * float3(1.0f, 1.0f, 1.0f);

        // 拡散反射+鏡面反射
        output.color.rgb = diffuseDirectionalLight + specularDirectionalLight + diffusePointLight + specularPointLight/* + diffuseSpotLight + specularSpotLight*/;
        // アルファ
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else // Lightingしない場合。前回までと同じ演算
    {
        output.color = gMaterial.color * textureColor;
    }
    
    // output.colorのa値が0のときにPixelを棄却
    if (output.color.a == 0.0)
    {
        discard;
    }
  
    if (gMaterial.enableFoging != 0) // Fogingする場合
    {
      // フォグ係数の計算
        float distToFog = length(input.worldPosition - gFogParam.fogCenter);
        float fogFactor = saturate(1.0 - (distToFog / gFogParam.radius));
        fogFactor *= gFogParam.fogIntensity;
    
        // ノイズによる揺らぎ追加
        float2 noiseInput = input.worldPosition.xz * 0.3 + gTimeParam.time * 0.5;
        float noise = (sin(noiseInput.x) + cos(noiseInput.y)) * 0.5 + 0.5;
        fogFactor *= noise;
        fogFactor = lerp(1.0, fogFactor, gFogParam.fogIntensity); // fogIntensity=0で霧なし、1でフル
    
        // フォグ色とのブレンド
        output.color.rgb = lerp(gFogParam.fogColor, output.color.rgb, fogFactor);
    }
    
    return output;
}