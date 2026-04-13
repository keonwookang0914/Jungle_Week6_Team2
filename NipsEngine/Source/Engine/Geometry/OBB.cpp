#include "OBB.h"
#include "Engine/Math/Matrix.h"

FOBB::FOBB(const FVector& InCenter, const FVector& InExtents, const TStaticArray<FVector, 3>& InAxes)
	: Center(InCenter), Extents(InExtents), Axes(InAxes)
{
}

FAABB FOBB::ToAABB() const
{
	FVector WorldExtents = FVector::Zero();
	for (uint32 i = 0; i < 3; ++i)
	{
		WorldExtents.X += std::abs(Extents[i] * Axes[i].X);
		WorldExtents.Y += std::abs(Extents[i] * Axes[i].Y);
		WorldExtents.Z += std::abs(Extents[i] * Axes[i].Z);
	}

	return FAABB(Center - WorldExtents, Center + WorldExtents);
}

// Separating Axis Theorem
// AABB 3개 축, OBB 3개 축, AABB 축과 OBB 축의 외적 9개 축에 대해 투영하여 겹치는지 검사
bool FOBB::IntersectAABB(const FAABB& AABB) const
{
	const FVector T = AABB.GetCenter() - Center;

	const FVector AABBExtents = AABB.GetExtent();
	const FVector OBBExtents = Extents;

	const TStaticArray<FVector, 3> AABBAxes = { FVector::UnitX(), FVector::UnitY(), FVector::UnitZ() };

	// AABB의 로컬 축(월드 축) 3개에 대해 검사
	for (uint32 i = 0; i < 3; ++i)
	{
		// R_A : R_A: AABB의 해당 축 방향 투영 반지름
		float R = AABBExtents[i];


		// R_B : OBB를 AABB 축에 투영
		// OBB의 각 로컬 축(Axes[j])이 AABB 축(AABBAxes[i])에 기여하는 정도를 계산
		for (uint32 j = 0; j < 3; ++j)
		{
			//R += std::abs(OBBExtents[j] * FVector::DotProduct(AABBAxes[i], Axes[j]));
			R += std::abs(OBBExtents[j] * Axes[j][i]);
		}

		if (std::abs(T[i]) > R) { return false; }
	}

	// OBB의 기저벡터에 대해 AABB 투영
	for (uint32 i = 0; i < 3; ++i)
	{
		FVector OBBAxis = Axes[i]; // OBB 축

		// R_B : OBB의 해당 축 방향 반지름
		float R = OBBExtents[i];

		// R_A : AABB를 OBB 축에 투영
		R += std::abs(AABBExtents.X * OBBAxis.X);
		R += std::abs(AABBExtents.Y * OBBAxis.Y);
		R += std::abs(AABBExtents.Z * OBBAxis.Z);

		// T를 OBB 축에 투영
		float ProjectedT = std::abs(FVector::DotProduct(T, OBBAxis));
		if (ProjectedT > R) { return false; }
	}

	// AABB 축과 OBB 축의 외적에 대해 투영
	for (uint32 i = 0; i < 3; ++i) // AABB의 축
	{
		for (uint32 j = 0; j < 3; ++j) // OBB의 축
		{
			FVector L = FVector::CrossProduct(AABBAxes[i], Axes[j]);
			// 두 축이 거의 평행하면 이 축은 무시 (이미 AABB와 OBB 축에 대해 검사했으므로)
			if (L.SizeSquared() < 1e-6f) { continue; }


			float R{ 0.0f };
			// R_A : AABB를 외적 축 L에 투영
			R += std::abs(AABBExtents[0] * L[0]);
			R += std::abs(AABBExtents[1] * L[1]);
			R += std::abs(AABBExtents[2] * L[2]);

			// R_B : OBB를 외적 축 L에 투영
			R += std::abs(OBBExtents[0] * FVector::DotProduct(L, Axes[0]));
			R += std::abs(OBBExtents[1] * FVector::DotProduct(L, Axes[1]));
			R += std::abs(OBBExtents[2] * FVector::DotProduct(L, Axes[2]));

			// T를 축 L에 투영
			float ProjectedT = std::abs(FVector::DotProduct(T, L));
			if (ProjectedT > R) { return false; }
		}
	}

	return true;
}

// Separating Axis Theorem
// OBB의 AABB로 이미 검사했으므로 OBB 축에 대해서만 검사
// OBB 3개 축, AABB 축과 OBB 축의 외적 9개 축에 대해 투영하여 겹치는지 검사
bool FOBB::IntersectAABBNarrow(const FAABB& AABB) const
{
	const FVector T = AABB.GetCenter() - Center;

	const FVector AABBExtents = AABB.GetExtent();
	const FVector OBBExtents = Extents;

	const TStaticArray<FVector, 3> AABBAxes = { FVector::UnitX(), FVector::UnitY(), FVector::UnitZ() };

	// OBB의 기저벡터에 대해 AABB 투영
	for (uint32 i = 0; i < 3; ++i)
	{
		FVector OBBAxis = Axes[i]; // OBB 축

		// R_B : OBB의 해당 축 방향 반지름
		float R = OBBExtents[i];

		// R_A : AABB를 OBB 축에 투영
		R += std::abs(AABBExtents.X * OBBAxis.X);
		R += std::abs(AABBExtents.Y * OBBAxis.Y);
		R += std::abs(AABBExtents.Z * OBBAxis.Z);

		// T를 OBB 축에 투영
		float ProjectedT = std::abs(FVector::DotProduct(T, OBBAxis));
		if (ProjectedT > R) { return false; }
	}

	// AABB 축과 OBB 축의 외적에 대해 투영
	for (uint32 i = 0; i < 3; ++i) // AABB의 축
	{
		for (uint32 j = 0; j < 3; ++j) // OBB의 축
		{
			FVector L = FVector::CrossProduct(AABBAxes[i], Axes[j]);
			// 두 축이 거의 평행하면 이 축은 무시 (이미 AABB와 OBB 축에 대해 검사했으므로)
			if (L.SizeSquared() < 1e-6f) { continue; }


			float R{ 0.0f };
			// R_A : AABB를 외적 축 L에 투영
			R += std::abs(AABBExtents[0] * L[0]);
			R += std::abs(AABBExtents[1] * L[1]);
			R += std::abs(AABBExtents[2] * L[2]);

			// R_B : OBB를 외적 축 L에 투영
			R += std::abs(OBBExtents[0] * FVector::DotProduct(L, Axes[0]));
			R += std::abs(OBBExtents[1] * FVector::DotProduct(L, Axes[1]));
			R += std::abs(OBBExtents[2] * FVector::DotProduct(L, Axes[2]));

			// T를 축 L에 투영
			float ProjectedT = std::abs(FVector::DotProduct(T, L));
			if (ProjectedT > R) { return false; }
		}
	}

	return true;
}