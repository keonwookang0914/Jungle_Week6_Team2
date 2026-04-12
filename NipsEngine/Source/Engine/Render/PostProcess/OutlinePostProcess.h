#pragma once
#include "Render/PostProcess/PostProcessBase.h"

class FOutlinePostProcess : public IPostProcess
{

public:
    EBlendState GetBlendState() const override { return EBlendState::AlphaBlend; }

    bool IsEnabled(const FPostProcessViewDesc& ViewDesc) const override;

	void Execute(ID3D11Device* Device, ID3D11DeviceContext* Context, const FPostProcessViewDesc& ViewDesc, FRenderResources& Resources,
               const FRenderTargetSet& RenderTargets, ID3D11ShaderResourceView* SceneColorSRV,
               ID3D11RenderTargetView* OutputRTV) override;
};
