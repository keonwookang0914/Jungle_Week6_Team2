#include "DecalComponent.h"

#include <cmath>

#include "Core/ResourceManager.h"
#include "UI/EditorConsoleWidget.h"

DEFINE_CLASS(UDecalComponent, UPrimitiveComponent)
REGISTER_FACTORY(UDecalComponent)

UDecalComponent* UDecalComponent::Duplicate()
{
    UDecalComponent* NewComp = UObjectManager::Get().CreateObject<UDecalComponent>();

    NewComp->SetActive(this->IsActive());
    NewComp->SetOwner(nullptr);

    NewComp->SetRelativeLocation(this->GetRelativeLocation());
    NewComp->SetRelativeRotation(this->GetRelativeRotation());
    NewComp->SetRelativeScale(this->GetRelativeScale());

    NewComp->SetVisibility(this->IsVisible());
    NewComp->DecalTextureName = this->DecalTextureName;
    NewComp->CachedDecalTexture = this->GetCachedDecalTexture();

    return NewComp;
}

FMatrix UDecalComponent::GetDecalViewProjection() const
{
    // Scale이 half-extent라고 가정
    const FVector WorldScale = GetWorldScale();
    const FVector WorldExtent(std::abs(WorldScale.X), std::abs(WorldScale.Y), std::abs(WorldScale.Z));

    constexpr float NearPlane = 0.0f;
    const float FarPlane = WorldExtent.X * 2.0f;
    const float Width = WorldExtent.Y * 2.0f;
    const float Height = WorldExtent.Z * 2.0f;
    const FMatrix Projection =
        FMatrix::MakeOrthographicLH(Width, Height, NearPlane, FarPlane);

    const FVector Eye = GetWorldLocation() - (GetDecalForward() * WorldExtent.Z);
    const FVector Target = GetWorldLocation() + (GetDecalForward() * WorldExtent.Z);
    const FMatrix View = FMatrix::MakeViewLookAtLH(Eye, Target, GetUpVector());
    return View * Projection;
}

FVector UDecalComponent::GetDecalForward() const
{
    FMatrix WorldMatrix = GetWorldMatrix();
    FVector DecalForward = -WorldMatrix.GetForwardVector();
    return DecalForward.GetSafeNormal();
}

void UDecalComponent::UpdateWorldAABB() const
{
    const FVector Forward = GetForwardVector();
    const FVector Right = GetRightVector();
    const FVector Up = GetUpVector();
    const FVector WorldScale = GetWorldScale();
    const FVector DecalExtent(std::abs(WorldScale.X), std::abs(WorldScale.Y), std::abs(WorldScale.Z));

    const FVector AABBExtent(
        std::abs(Forward.X) * DecalExtent.X + std::abs(Right.X) * DecalExtent.Y + std::abs(Up.X) * DecalExtent.Z,
        std::abs(Forward.Y) * DecalExtent.X + std::abs(Right.Y) * DecalExtent.Y + std::abs(Up.Y) * DecalExtent.Z,
        std::abs(Forward.Z) * DecalExtent.X + std::abs(Right.Z) * DecalExtent.Y + std::abs(Up.Z) * DecalExtent.Z);

    const FVector WorldCenter = GetWorldLocation();

    WorldAABB.Min = WorldCenter - AABBExtent;
    WorldAABB.Max = WorldCenter + AABBExtent;
}

bool UDecalComponent::RaycastMesh(const FRay& Ray, FHitResult& OutHitResult)
{
    return false;
}
