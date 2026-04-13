#pragma once

#include "PrimitiveComponent.h"
#include "Core/ResourceTypes.h"
#include "Object/FName.h"

class UFireBallComponent : public UPrimitiveComponent
{
  public:
    DECLARE_CLASS(UFireBallComponent, UPrimitiveComponent)

    UFireBallComponent();

    // -- UPrimitiveComponent 필수 구현(순수 가상 함수) 오버라이드 --
    virtual UPrimitiveComponent* Duplicate() override;
    virtual void                 UpdateWorldAABB() const override;
    virtual bool                 RaycastMesh(const FRay& Ray, FHitResult& OutHitResult) override;
    virtual EPrimitiveType       GetPrimitiveType() const override;
// -- 프로퍼티 에디터 노출용 --
	virtual void GetEditableProperties(TArray<FPropertyDescriptor>& OutProps) override;

// -- Getters --
	float    GetIntensity() const { return Intensity; }
	float    GetRadius() const { return Radius; }
	float    GetRadiusFallOff() const { return RadiusFallOff; }
	FColor GetColor() const { return Color; }

private:

	float    Intensity = {1.0f};
	float    Radius = {50.0f}; 
	float    RadiusFallOff = {20.0f};
	FColor Color = {1.0f, 0.5f, 0.0f, 1.0f}; 
};
