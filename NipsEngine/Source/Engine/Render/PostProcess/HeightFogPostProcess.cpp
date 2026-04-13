#include "HeightFogPostProcess.h"

bool UHeightFogPostProcess::IsEnabled(const FPostProcessViewDesc& ViewDesc) const
{
	return false;
}

void UHeightFogPostProcess::Execute(FD3DDevice* Device, ID3D11DeviceContext* Context, const FPostProcessViewDesc& ViewDesc, FRenderResources& Resources, const FRenderTargetSet& RenderTargets, ID3D11ShaderResourceView* SceneColorSRV, ID3D11RenderTargetView* OutputRTV)
{
}
