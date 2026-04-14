/* Constant Buffers */

cbuffer FrameBuffer : register(b0)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float bIsWireframe;
    float3 WireframeRGB;
}

cbuffer PerObjectBuffer : register(b1)
{
    row_major float4x4 Model;
    float4 PrimitiveColor;
};

cbuffer GizmoBuffer : register(b2)
{
    float4 GizmoColorTint;
    uint bIsInnerGizmo;
    uint bClicking;
    uint SelectedAxis;
    float HoveredAxisOpacity;
};


cbuffer OverlayBuffer : register(b3)
{
    float2 OverlayCenterScreen;
    float2 ViewportSize;

    float OverlayRadius;
    float3 Padding2;

    float4 OverlayColor;
};

cbuffer EditorBuffer : register(b4)
{
    float4 CameraPosition;
    uint EditorFlag;
    float3 Padding3;
};

cbuffer OutlineConstants : register(b5)
{
    float4 OutlineColor;
    float OutlineThicknessPixels;
    float2 OutlineViewportSize;
    float OutlinePadding0;
};

// cbuffer StaticMeshBuffer : register(b6)
// cbuffer DecalBuffer : register(b9)
// cbuffer ViewportInfoBuffer : register(b12)

// 멀티 뷰포트 post process에서 공통으로 사용하는 viewport 정보.
// sampling은 전체 render target 기준으로 하되, 현재 viewport 경계를 알기 위해
// 원점/크기/역해상도를 함께 넘긴다.
cbuffer ViewportInfoBuffer : register(b12)
{
    float2 InvFullRenderTargetSize;
    float2 ViewportOriginPixels;

    float2 ViewportSizePixels;
    float2 ViewportInfoPadding0;
};

float4 ApplyMVP(float3 pos)
{
    float4 world = mul(float4(pos, 1.0f), Model);
    float4 view = mul(world, View);
    return mul(view, Projection);
}

float3x3 Inverse3x3(float3x3 m)
{
    float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
              - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
              + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    float invDet = 1.0 / det;

    float3x3 result;
    result[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
    result[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * invDet;
    result[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;
    result[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * invDet;
    result[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
    result[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * invDet;
    result[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
    result[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * invDet;
    result[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;
    return result;
}

float2 GetPostProcessFullPixelCoord(float2 svPosition)
{
    // 현재 post process 경로에서는 SV_Position이 이미 full render target pixel 기준이다.
    return svPosition;
}

float2 GetPostProcessFullUV(float2 svPosition)
{
    return GetPostProcessFullPixelCoord(svPosition) * InvFullRenderTargetSize;
}

float2 GetPostProcessViewportMinUV()
{
    return ViewportOriginPixels * InvFullRenderTargetSize;
}

float2 GetPostProcessViewportMaxUV()
{
    return (ViewportOriginPixels + ViewportSizePixels) * InvFullRenderTargetSize;
}

float2 ClampPostProcessViewportUV(float2 uv)
{
    float2 viewportMinUV = GetPostProcessViewportMinUV();
    float2 viewportMaxUV = GetPostProcessViewportMaxUV();
    // Neighbor sample이 다른 viewport를 읽지 않도록 half pixel margin으로 clamp한다.
    float2 halfPixel = InvFullRenderTargetSize * 0.5f;
    return clamp(uv, viewportMinUV + halfPixel, viewportMaxUV - halfPixel);
}
