#include "Common.hlsl"  // ViewportInfoBuffer(b12), ClampPostProcessViewportUV, GetPostProcessFullUV

// ----------------------------------------------------------------
// Textures & Samplers
// t0 : SceneColor   (Ping-Pong Source)
// t1 : DepthBuffer  (RenderTargets.DepthSRV)
// ----------------------------------------------------------------
Texture2D SceneColorTex : register(t0);
Texture2D DepthTex : register(t1);
SamplerState PointSampler : register(s0);

// ----------------------------------------------------------------
// HeightFog Constant Buffer  b11
// C++ FHeightFogCBuffer 레이아웃과 1:1 매칭
//
// float3 WorldPosition    + float FogDensity        → 16 bytes
// float  FogHeightFalloff + float StartDistance
//   + float FogCutoffDistance + float FogMaxOpacity → 16 bytes
// float3 InscatteringColor + float _Pad0            → 16 bytes
// float3 CameraWorldPos    + float _Pad1            → 16 bytes
// float4x4 InvViewProj                              → 64 bytes
// Total : 128 bytes
// ----------------------------------------------------------------
cbuffer HeightFogBuffer : register(b11)
{
    float3 FogWorldPosition; // 컴포넌트 높이 기준점
    float FogDensity;

    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;

    float3 InscatteringColor;
    float _Pad0;

    float3 CameraWorldPos;
    float _Pad1;

    row_major float4x4 InvViewProj;
}

// ----------------------------------------------------------------
// Fullscreen Triangle  VS
// VB 없이 SV_VertexID 만으로 NDC[-1,1] 삼각형을 생성합니다.
//   ID=0 → (-1, -1)   좌하
//   ID=1 → (-1,  3)   좌상(클립 밖)
//   ID=2 → ( 3, -1)   우하(클립 밖)
// ----------------------------------------------------------------
struct FVSOutput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

FVSOutput VS_Main(uint VertexID : SV_VertexID)
{
    FVSOutput Output;

    float2 NDC;
    NDC.x = (VertexID == 2) ? 3.0f : -1.0f;
    NDC.y = (VertexID == 1) ? 3.0f : -1.0f;

    Output.Position = float4(NDC, 0.0f, 1.0f);

    // NDC → UV  (y 반전)
    Output.UV = NDC * float2(0.5f, -0.5f) + 0.5f;

    return Output;
}

// ----------------------------------------------------------------
// World Position 복원
// NDC depth(0~1) + InvViewProj → World Space Position
// ----------------------------------------------------------------
float3 ReconstructWorldPos(float2 UV, float Depth)
{
    // UV → NDC
    float2 NDC = UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);

    float4 ClipPos = float4(NDC, Depth, 1.0f);
    float4 WorldPos = mul(ClipPos, InvViewProj);
    return WorldPos.xyz / WorldPos.w;
}

// ----------------------------------------------------------------
// Exponential Height Fog
//
//   localDensity = FogDensity * exp( -FogHeightFalloff * max(WorldZ - FogZ, 0) )
//   viewDist     = max( dist(WorldPos, Camera) - StartDistance, 0 )
//                  [FogCutoffDistance > 0 이면 min(viewDist, CutoffDist)]
//   extinction   = 1 - exp( -localDensity * viewDist )
//   fogFactor    = clamp( extinction, 0, FogMaxOpacity )
// ----------------------------------------------------------------
float ComputeExponentialHeightFog(float3 WorldPos)
{
    // 1. 높이 기반 로컬 밀도
    float HeightAbove = max(WorldPos.z - FogWorldPosition.z, 0.0f);
    float LocalDensity = FogDensity * exp(-FogHeightFalloff * HeightAbove);

    // 2. 뷰 거리
    float ViewDist = max(length(WorldPos - CameraWorldPos) - StartDistance, 0.0f);

    if (FogCutoffDistance > 0.0f)
    {
        ViewDist = min(ViewDist, FogCutoffDistance);
    }

    // 3. Beer-Lambert 소광
    float Extinction = 1.0f - exp(-LocalDensity * ViewDist);
    return clamp(Extinction, 0.0f, FogMaxOpacity);
}

// ----------------------------------------------------------------
// Pixel Shader
// ----------------------------------------------------------------
float4 PS_Main(FVSOutput Input) : SV_Target
{
    // --- Common.hlsl 유틸로 UV 보정 (멀티 뷰포트 경계 clamp) ---
    float2 UV = GetPostProcessFullUV(Input.Position.xy);
    UV = ClampPostProcessViewportUV(UV);

    // 1. SceneColor / Depth 샘플링
    float4 SceneColor = SceneColorTex.Sample(PointSampler, UV);
    float Depth = DepthTex.Sample(PointSampler, UV).r;

    // 2. Depth == 1.0 이면 스카이박스(무한 원점) → 안개 미적용
    if (Depth >= 1.0f)
    {
        return SceneColor;
    }

    // 3. World Position 복원
    float3 WorldPos = ReconstructWorldPos(UV, Depth);

    // 4. 안개 계산
    float FogFactor = ComputeExponentialHeightFog(WorldPos);

    // 5. SceneColor ↔ InscatteringColor 보간
    float3 FoggedColor = lerp(SceneColor.rgb, InscatteringColor, FogFactor);

    return float4(FoggedColor, SceneColor.a);
}
