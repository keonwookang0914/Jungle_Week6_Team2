// FireBallShader.hlsl

#include "Common.hlsl"

// ─────────────────────────────────────────────
// FireBall 전용 상수버퍼 b10
// ─────────────────────────────────────────────
#define MAX_FIREBALL_COUNT 32

struct FFireBallConstants
{
    float4 WorldLocation; // 16 bytes
    float4 LinearColor; // 16 bytes
    float Intensity; //  4 bytes
    float Radius; //  4 bytes
    float RadiusFalloff; //  4 bytes
    float _padding; //  4 bytes
}; // = 48 bytes per entry

cbuffer FireBallBuffer : register(b10)
{
    row_major float4x4 InvViewProj;
    FFireBallConstants FireBalls[MAX_FIREBALL_COUNT];
    int FireBallCount;
    float3 _cbPadding;
};

// ─────────────────────────────────────────────
// Textures
// ─────────────────────────────────────────────
Texture2D SceneColorTexture : register(t0);
Texture2D DepthTexture : register(t1);
SamplerState PointSampler : register(s0);

// ─────────────────────────────────────────────
// HSV 변환 함수
// ─────────────────────────────────────────────
float3 RGBtoHSV(float3 c)
{
    float4 K = float4(0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f);
    float4 p = c.g < c.b ? float4(c.bg, K.wz) : float4(c.gb, K.xy);
    float4 q = c.r < p.x ? float4(p.xyw, c.r) : float4(c.r, p.yzx);
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10f;
    return float3(abs(q.z + (q.w - q.y) / (6.0f * d + e)), d / (q.x + e), q.x);
}

float3 HSVtoRGB(float3 c)
{
    float4 K = float4(1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0f - K.www);
    return c.z * lerp(K.xxx, saturate(p - K.xxx), c.y);
}

// ─────────────────────────────────────────────
// World Position 복원 함수 (Standard-Z)
// ─────────────────────────────────────────────
float3 ReconstructWorldPosition(float2 UV, float Depth)
{
    float4 NDCPos;
    NDCPos.x = UV.x * 2.0f - 1.0f;
    NDCPos.y = UV.y * -2.0f + 1.0f;
    NDCPos.z = Depth;
    NDCPos.w = 1.0f;

    float4 WorldPos = mul(NDCPos, InvViewProj);
    WorldPos /= WorldPos.w;

    return WorldPos.xyz;
}

// ─────────────────────────────────────────────
// Vertex Shader
// ─────────────────────────────────────────────
struct VSOutput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

VSOutput VS_Main(uint VertexID : SV_VertexID)
{
    VSOutput Output;
    Output.UV = float2((VertexID << 1) & 2, VertexID & 2);
    Output.Position = float4(Output.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return Output;
}

// ─────────────────────────────────────────────
// Pixel Shader
// ─────────────────────────────────────────────
float4 PS_Main(VSOutput Input) : SV_Target
{
    // 1. Depth 샘플링
    float Depth = DepthTexture.Sample(PointSampler, Input.UV).r;

    // Sky / Empty 픽셀 스킵
    if (Depth >= 1.0f)
    {
        return SceneColorTexture.Sample(PointSampler, Input.UV);
    }

    // 2. World Position 복원
    float3 WorldPos = ReconstructWorldPosition(Input.UV, Depth);

    // 3. SceneColor 샘플링
    float4 SceneColor = SceneColorTexture.Sample(PointSampler, Input.UV);

    // 4. 모든 FireBall 누적
    for (int i = 0; i < FireBallCount; ++i)
    {
        float3 Center = FireBalls[i].WorldLocation.xyz;
        float Dist = distance(WorldPos, Center);
        float Radius = FireBalls[i].Radius;
        float RadiusFalloff = FireBalls[i].RadiusFalloff;

        if (Dist > RadiusFalloff)
            continue;

        float Weight = 1.0f - smoothstep(Radius, RadiusFalloff, Dist);
        float BlendWeight = saturate(Weight * FireBalls[i].Intensity);
        float3 LightColor = FireBalls[i].LinearColor.rgb * FireBalls[i].Intensity;

        // HSV 공간에서 H, S만 FireBall 색으로 이동, V는 원본 유지
        float3 SceneHSV = RGBtoHSV(SceneColor.rgb);
        float3 LightHSV = RGBtoHSV(LightColor);

        float3 BlendedHSV;
        BlendedHSV.x = lerp(SceneHSV.x, LightHSV.x, BlendWeight); // H: 색조
        BlendedHSV.y = lerp(SceneHSV.y, LightHSV.y, BlendWeight); // S: 채도
        BlendedHSV.z = SceneHSV.z; // V: 명도 원본 유지

        SceneColor.rgb = HSVtoRGB(BlendedHSV);
    }

    return SceneColor;
}