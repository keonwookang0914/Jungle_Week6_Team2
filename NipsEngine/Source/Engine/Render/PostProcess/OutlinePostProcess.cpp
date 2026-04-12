#include "OutlinePostProcess.h"

bool OutlinePostProcess::IsEnabled(const FPostProcessViewDesc& ViewDesc) const {
	return true; 
}

void OutlinePostProcess::Execute(ID3D11DeviceContext* Context, const FPostProcessViewDesc& ViewDesc,
                                 FRenderResources& Resources,
                                 const FRenderTargetSet& RenderTargets, ID3D11ShaderResourceView* SceneColorSRV,
                                 ID3D11RenderTargetView* OutputRTV)
{
    
}
