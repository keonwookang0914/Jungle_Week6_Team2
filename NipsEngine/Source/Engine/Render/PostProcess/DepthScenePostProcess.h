#pragma once

#include "Render/PostProcess/PostProcessBase.h"

class FDepthScenePostProcess : public IPostProcess
{
  public:
    virtual bool IsEnabled(const FPostProcessViewDesc& ViewDesc) const override;

    virtual void Execute(ID3D11DeviceContext* Context, const FPostProcessViewDesc& ViewDesc,
                         FRenderResources&         Resources,
                         const FRenderTargetSet&   RenderTargets, // DepthStencilSRV, SelectionMaskSRV 포함
                         ID3D11ShaderResourceView* SceneColorSRV, // Ping-Pong Source (Color)
                         ID3D11RenderTargetView*   OutputRTV      // Ping-Pong Dest
                         ) override;
};