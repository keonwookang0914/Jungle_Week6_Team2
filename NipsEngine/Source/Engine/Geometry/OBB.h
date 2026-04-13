#pragma once

#include "Engine/Math/Vector.h"
#include "Engine/Geometry/AABB.h"
#include "Engine/Core/Containers/StaticArray.h"

struct FOBB
{
	FVector Center{};
	FVector Extents{};
	TStaticArray<FVector, 3> Axes{};

	FOBB() = default;
	FOBB(const FVector& InCenter, const FVector& InExtents, const TStaticArray<FVector, 3>& InAxes);

	FAABB ToAABB() const;
	bool IntersectAABB(const FAABB& AABB) const;
	bool IntersectAABBNarrow(const FAABB& AABB) const;
	bool IntersectAABBCrossAxesOnly(const FAABB& AABB) const;
};