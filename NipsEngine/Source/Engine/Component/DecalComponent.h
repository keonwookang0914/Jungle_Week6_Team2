#pragma once

#include "PrimitiveComponent.h"
#include "Core/ResourceTypes.h"
#include "Core/ResourceManager.h"
#include "Object/FName.h"

// TODO: 어째서 UTexture2D가 아닐까요?
struct FTextureResource;

// TODO: DecalComponent를 맴버 변수로 갖는 ADecalActor를 구현한다.
class UDecalComponent : public UPrimitiveComponent
{
public:
    DECLARE_CLASS(UDecalComponent, UPrimitiveComponent)

	static constexpr EPrimitiveType PrimitiveType = EPrimitiveType::EPT_Decal;
    EPrimitiveType GetPrimitiveType() const override { return PrimitiveType; }

    virtual UDecalComponent* Duplicate() override;
    virtual UDecalComponent* DuplicateSubObjects() override { return this; }

    void UpdateWorldAABB() const override;
    bool RaycastMesh(const FRay& Ray, FHitResult& OutHitResult) override;

private:
    FName DecalTextureName;
    FTextureResource* CachedDecalTexture = { nullptr };

public:
    void SetDecalTextureName(FString NewName)
    {
        DecalTextureName = NewName;
        CachedDecalTexture = FResourceManager::Get().FindTexture(NewName);
    }
    FString GetDecalTextureName() { return DecalTextureName.ToString(); }
    FTextureResource* GetCachedDecalTexture()
    {
        if (CachedDecalTexture == nullptr) { CachedDecalTexture = FResourceManager::Get().FindTexture(DecalTextureName.ToString()); }
        return CachedDecalTexture;
    }
    FMatrix GetDecalViewProjection() const;
    // -X축이 Decal의 Forward
    FVector GetDecalForward() const;

    // float GetDepth() const { return GetWorldScale().X; }
    // float GetWidth() const { return GetWorldScale().Y; }
    // float GetHeight() const { return GetWorldScale().Z; }
};
