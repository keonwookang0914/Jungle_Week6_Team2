#include "Common.hlsl"

cbuffer DecalBuffer : register(b9)
{
    row_major float4x4 DecalViewProjection;
    float3 DecalForward;
    float Padding7_0;
};

Texture2D DecalTexture : register(t4);

SamplerState DecalSampler : register(s1);

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION; // ClipPos
    float3 worldPos : TEXCOORD0; // WorldPos
    float3 worldNormal : TEXCOORD1; // WorldNormal
};

PSInput VS(VSInput input)
{
    PSInput output;

    output.position = ApplyMVP(input.position);
    output.worldPos = mul(float4(input.position, 1.0f), Model).xyz;
    output.worldNormal = normalize(mul(input.normal, (float3x3)Model));

    return output;
}

float4 PS(PSInput input) : SV_TARGET
{
    // 월드 좌표를 데칼의 NDC 좌표로 변환
    float4 decalNDC = mul(float4(input.worldPos, 1.0f), DecalViewProjection);
    decalNDC.xyz /= decalNDC.w;

    if (any(abs(decalNDC.xyz) > 1.0f) || decalNDC.z < 0.0f)
    {
        return float4(0, 0, 0, 0); // 데칼 영역 밖
    }

    float normalDotDecalForward = dot(normalize(input.worldNormal), -DecalForward);
    if (normalDotDecalForward <= 0.0f)
    {
        return float4(0, 0, 0, 0);
    }

    float2 decalUV = decalNDC.xy * 0.5f + 0.5f; // NDC를 UV로 변환
    decalUV.y = 1.0f - decalUV.y; // Texture 좌표계는 Y축이 반대

    // TODO: decalNDC.z에 따라 alpha에 lerp 적용

    float4 decalColor = DecalTexture.Sample(DecalSampler, decalUV);
    if (decalColor.a <= 0.01f)
    {
        return float4(0, 0, 0, 0);
    }

    return decalColor;
}
