/*
 *  Fast-Approximate Anti-Aliasing
 */
Texture2D Scene : register(t0); //Scene의 RTV를 SRV로 받음.
SamplerState LinearSampler : register(s0); // 선형 샘플러

cbuffer FXAACB : register(b7)
{
    float2 InvScreenSize; // (1/width, 1/height)
    float EdgeThreshold; // 예: 1.0 / 8.0
    float EdgeThresholdMin; // 예: 1.0 / 16.0
    float Subpix; // 예: 0.75
    float3 padding;
};

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

float Luma(float3 rgb)
{
    // 단순 구현, 표준 계수 사용
    return dot(rgb, float3(0.299, 0.587, 0.114));
}

PSInput VS(uint vertexId : SV_VertexID)
{
    PSInput output;

    float2 positions[3] =
    {
        float2(-1.0f, -1.0f),
        float2(-1.0f, 3.0f),
        float2(3.0f, -1.0f)
    };

    output.position = float4(positions[vertexId], 0.0f, 1.0f);
    return output;
}

float4 PS(PSInput input) : SV_TARGET
{
    float2 uv = input.uv;
    float2 px = InvScreenSize;
    
    // 현재 픽셀(rgbM)과 상하좌우(N/S/E/W)를 읽는다.
    float3 rgbM = Scene.Sample(LinearSampler, uv).rgb;
    float3 rgbN = Scene.Sample(LinearSampler, uv + float2(0, -px.y)).rgb;
    float3 rgbS = Scene.Sample(LinearSampler, uv + float2(0, px.y)).rgb;
    float3 rgbW = Scene.Sample(LinearSampler, uv + float2(-px.x, 0)).rgb;
    float3 rgbE = Scene.Sample(LinearSampler, uv + float2(px.x, 0)).rgb;
    
    //RGB -> LUma로 변경
    float lumaM = Luma(rgbM);
    float lumaN = Luma(rgbN);
    float lumaS = Luma(rgbS);
    float lumaE = Luma(rgbE);
    float lumaW = Luma(rgbW);
    
    float rangeMin = min(lumaM, min(min(lumaN, lumaS), min(lumaE, lumaW)));
    float rangeMax = max(lumaM, max(max(lumaN, lumaS), max(lumaE, lumaW)));
    float range = rangeMin - rangeMax;
    
    // 1. Edge(주변 픽셀과 비교했을 때, 휘도나 색이 급격하게 변하는 구역)가 아니라면 바로 반환.
    float threshold = max(EdgeThresholdMin, rangeMax * EdgeThreshold);
    if(range < threshold)
        return float4(rgbM, 1.0);
    
    // 2. 대각선도 읽어서 방향 판정 보강
    float3 rgbNW = Scene.Sample(LinearSampler, uv + float2(-px.x, -px.y)).rgb; // 북서
    float3 rgbNE = Scene.Sample(LinearSampler, uv + float2( px.x, -px.y)).rgb; // 북동
    float3 rgbSW = Scene.Sample(LinearSampler, uv + float2(-px.x,  px.y)).rgb; // 남서
    float3 rgbSE = Scene.Sample(LinearSampler, uv + float2( px.x,  px.y)).rgb; // 남동
    
    // 대각선 RGB -> Luma로 변경
    float lumaNW = Luma(rgbNW);
    float lumaNE = Luma(rgbNE);
    float lumaSW = Luma(rgbSW);
    float lumaSE = Luma(rgbSE);
    
    // Edge의 방향 판정 -> 경계가 가로인가, 세로인가? 
    // 픽셀 중심 통과 1픽셀 선 대응 목적 3x3 주변의 row/column별 high-pass크기 가중 합산
    // 가운데 줄/열에 더 큰 가중치를 준 이유는 중앙 row/column을 더 중요하게 보기 위해서
    float edgeVert = // 수직 Edge
        abs(0.25 * lumaNW - 0.5 * lumaN + 0.25 * lumaNE) +
        abs(0.50 * lumaW - 1.0 * lumaM + 0.50 * lumaE) +
        abs(0.25 * lumaSW - 0.5 * lumaS + 0.25 * lumaSE);

    float edgeHorz = // 수평 Edge
        abs(0.25 * lumaNW - 0.5 * lumaW + 0.25 * lumaSW) +
        abs(0.50 * lumaN - 1.0 * lumaM + 0.50 * lumaS) +
        abs(0.25 * lumaNE - 0.5 * lumaE + 0.25 * lumaSE);

    bool isHorizontal = (edgeHorz >= edgeVert);
    
    // 3. 에지에 수직한 방향으로 블렌드할 오프셋 계산
    float2 dir = isHorizontal ? float2(0.0, px.y) : float2(px.x, 0.0);
    
    // 양쪽 한 칸 샘플 기반
    /*
    Linear Filtering
    rgbNeg: C-1 * 05 + c0 * 0.5
    rgbPos: C0 * 0.5 + c1 * 0.5
    edgeBlend: 0.25*C-1 + 0.5*C0 + 0.25*C+1
    세게 blur 가 아니라, 중앙을 살린 약한 1D smoothing
    */
    float3 rgbNeg = Scene.Sample(LinearSampler, uv - dir * 0.5).rgb;
    float3 rgbPos = Scene.Sample(LinearSampler, uv + dir * 0.5).rgb;
    float3 edgeBlend = 0.5 * (rgbNeg + rgbPos);
    
    // sub-pixel aliasing 완화
    /*
    아주 얇은 디테일이 픽셀 격자보다 작거나 비슷해서 불안정하게 보이는 현상.
    1픽셀짜리 흰점, 얇은 철망 등이 예시
    미세한 깜빡임/점 잡음을 줄이기 위한 보조 판단.
    */
    // 현재 픽셀이 주변 평균에서 얼마나 튀는가?
    // 현재 픽셀의 튐 정도를 주변 전체 대비 크기로 정규화
    float lumaAvg = (lumaN + lumaS + lumaE + lumaW) * 0.25;
    // 얼마나 부드럽게 만들어야하나를 뜻하는 가중치
    // 정규화, 현재 픽셀의 튐 정도를 구역 전체 constrast와 비교해서 보는 것.
    // Saturate로 0~1로 clamping해서 바로 블렌드 가중치로 사용할 수 있게 함.
    float subpixAmount = saturate(abs(lumaAvg - lumaM) / max(range, 1e-4)); 
    subpixAmount *= Subpix;
    
    // 현재 픽셀을 그대로 둘지, 아니면 필터된 색으로 얼마나 끌어갈지 결정
    float3 result = lerp(rgbM, edgeBlend, subpixAmount);
    
    return float4(result, 1.0);
}