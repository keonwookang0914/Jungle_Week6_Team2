#include "FXAAPostProcess.h"
#include "Settings/EditorSettings.h"

bool FFXAAPostProcess::IsEnabled(const FRenderBus& Bus) const 
{ 
	return Bus.GetShowFlags().bEnableFXAA; 
}

void FFXAAPostProcess::Execute(ID3D11DeviceContext* Context, const FRenderBus& Bus, FRenderResources& Resources,
                               const FRenderTargetSet& RenderTargets, ID3D11ShaderResourceView* SceneColorSRV,
                               ID3D11RenderTargetView* OutputRTV)
{
    if (!bHasViewport || ViewportWidth <= 0.f || ViewportHeight <= 0)
        return;
    if (!Context || !SceneColorSRV || !OutputRTV)
        return;

    const float FullWidth = RenderTargets.Width;
    const float FullHeight = RenderTargets.Height;
	if (FullWidth <= 0.f || FullHeight <= 0.f)
        return;

	// Viewport가 1개의 texture를 공유한다.
	// 이번에 rendering할 viewport texture의 uv 정보를 constant Buffer에 담아서 보내야한다.
    FFxaaConstantBuffer FXAAConstants = {};

    // RenderTarget Size의 역수 (UV 정규화 목적)
	FXAAConstants.InvRenderTargetSize = FVector2(1.0f / FullWidth, 1.0f / FullHeight);

    // 해당 뷰포트의 Texture내의 SubUV 설정
    FXAAConstants.ViewportMinUV = FVector2(static_cast<float>(ViewportX) / FullWidth, 
										   static_cast<float>(ViewportY) / FullHeight);
    FXAAConstants.ViewportMaxUV = FVector2(static_cast<float>(ViewportX + ViewportWidth) / FullWidth,
                                           static_cast<float>(ViewportY + ViewportHeight) / FullHeight);

    const FEditorSettings& Settings = FEditorSettings::Get();
    FXAAConstants.EdgeThreshold = Settings.FXAA.EdgeThreshold;
    FXAAConstants.EdgeThresholdMin = Settings.FXAA.EdgeThresholdMin;
    FXAAConstants.Subpix = Settings.FXAA.Subpix;
	

	Context->OMSetRenderTargets(1, &OutputRTV, nullptr);
    Context->OMSetDepthStencilState(nullptr, 0);

	Resources.FxaaShader.Bind(Context);
    Resources.FxaaConstantBuffer.Update(Context, &FXAAConstants, sizeof(FFxaaConstantBuffer));

	// 상수버퍼 VS, PS에 연결
    ID3D11Buffer* CB = Resources.FxaaConstantBuffer.GetBuffer();
    // Constant Buffer 8번 슬롯 사용 (7번은 Decal이 사용 예정)
    Context->VSSetConstantBuffers(8, 1, &CB);
    Context->PSSetConstantBuffers(8, 1, &CB);

	// Source SRV 연결 + LInear Sampler 연결
    ID3D11SamplerState* Sampler = Resources.LinearSamplerState.Get();
    Context->PSSetShaderResources(0, 1, &SceneColorSRV);
    Context->PSSetSamplers(0, 1, &Sampler);

	// Draw 호출
    Context->Draw(3, 0);

	// NullSRV로 바인딩 해제.
    ID3D11ShaderResourceView* NullSRV = nullptr;
    Context->PSSetShaderResources(0, 1, &NullSRV);

	ClearViewportRect();
}

void FFXAAPostProcess::SetViewportRect(int32 InX, int32 InY, int32 InWidth, int32 InHeight) 
{ 
	ViewportX = InX;
    ViewportY = InY;
    ViewportWidth = InWidth;
    ViewportHeight = InHeight;
	bHasViewport = true;
}

void FFXAAPostProcess::ClearViewportRect() 
{
    ViewportX = 0;
    ViewportY = 0;
    ViewportWidth = 0;
    ViewportHeight = 0;
    bHasViewport = false;
}
