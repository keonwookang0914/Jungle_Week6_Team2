#pragma once
#include "Core/CoreTypes.h"
#include <Render/Scene/RenderBus.h>

struct FPostProcessViewDesc
{
    int32 X = 0;
    int32 Y = 0;
    int32 Width = 0;
    int32 Height = 0;
    
	// RenderBus의 값을 받아오는 멤버 변수들
	EViewMode  ViewMode = EViewMode::Lit;
    FShowFlags ShowFlags = {};
    FMatrix    View = FMatrix::Identity;
    FMatrix    Proj = FMatrix::Identity;
    float      NearPlane = 0.1f;
    float      FarPlane = 1000.0f;
};
