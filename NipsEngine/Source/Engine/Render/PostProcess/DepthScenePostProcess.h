#pragma once

#include "Render/PostProcess/PostProcessBase.h"


// PostProcessDepthScene.h
#pragma once

#include "Render/PostProcess/PostProcessBase.h"

class FDepthScenePostProcess : public IPostProcess
{
  public:
    bool IsEnabled(const FRenderBus& Bus) const override;

    void Execute(ID3D11DeviceContext* Context, const FRenderBus& Bus, FRenderResources& Resources,
                 const FRenderTargetSet& RenderTargets, ID3D11ShaderResourceView* SceneColorSRV,
                 ID3D11RenderTargetView* OutputRTV) override;
};