#pragma once
#include "PostProcessBase.h"
class FFXAAPostProcess : public IPostProcess
{

public:
    bool IsEnabled(const FRenderBus& Bus) const override;

	void Execute(ID3D11DeviceContext* Context, const FRenderBus& Bus, FRenderResources& Resources,
               const FRenderTargetSet& RenderTargets, ID3D11ShaderResourceView* SceneColorSRV,
               ID3D11RenderTargetView* OutputRTV) override;

	// Viewport Helper 함수
    void SetViewportRect(int32 InX, int32 InY, int32 InWidth, int32 InHeight);
    void ClearViewportRect();

private:
	// Post-Process를 진행할 Viewport Rect 크기 정보
	int32 ViewportX = 0;
    int32 ViewportY = 0;
    int32 ViewportWidth = 0;
    int32 ViewportHeight = 0;
    bool  bHasViewport = false;
};
