#pragma once
#include "RenderBus.h"
#include "Render/PostProcess/PostProcessTypes.h"
#include "Render/Resource/MeshBufferManager.h"
#include "Spatial/WorldSpatialIndex.h"
#include <unordered_set>

class UWorld;
class AActor;
class UPrimitiveComponent;
class UGizmoComponent;
struct FFrustum;

class FRenderCollector {
public:
	struct FCullingStats
	{
		int32 TotalVisiblePrimitiveCount{0};
		int32 BVHPassedPrimitiveCount{0};
		int32 FallbackPassedPrimitiveCount{0};
	};

	struct FDecalStats
	{
		int32  ActiveDecals       = 0; // 이번 프레임에 처리된 UDecalComponent 수
		int32  CulledCandidates   = 0; // OBB 교차 실패로 제외된 StaticMesh 수
		int32  AffectedPrimitives = 0; // Draw Call 수 (OBB 교차 통과한 StaticMesh 수)
		double CollectTimeMs      = 0.0; // Decal 수집 루프 CPU 소요 시간 (ms)
	};

private:
	FMeshBufferManager MeshBufferManager;
	FWorldSpatialIndex::FPrimitiveFrustumQueryScratch FrustumQueryScratch;
	TArray<UPrimitiveComponent*> VisiblePrimitiveScratch;
	FCullingStats LastCullingStats;
	FDecalStats   LastDecalStats;
	FWorldSpatialIndex::FPrimitiveFrustumQueryScratch DecalQueryScratch;
	TArray<UPrimitiveComponent*> DecalCandidateScratch;
public:
	void Initialize(ID3D11Device* InDevice) { MeshBufferManager.Create(InDevice); }
	void Release() { MeshBufferManager.Release(); }

	void CollectWorld(UWorld* World, const FShowFlags& ShowFlags, EViewMode ViewMode, FRenderBus& RenderBus,
	                  const FFrustum* ViewFrustum = nullptr);
	void CollectSelection(const TArray<AActor*>& SelectedActors, const FShowFlags& ShowFlags, EViewMode ViewMode, FRenderBus& RenderBus,
	                      FOutlinePostProcessData& OutOutlineData);
	void CollectGizmo(UGizmoComponent* Gizmo, const FShowFlags& ShowFlags, FRenderBus& RenderBus, bool bIsActiveOperation);
	void CollectGrid(float GridSpacing, int32 GridHalfLineCount, FRenderBus& RenderBus, bool bOrthographic = false);
	const FCullingStats& GetLastCullingStats() const { return LastCullingStats; }
	const FDecalStats&   GetLastDecalStats()   const { return LastDecalStats; }

private:
	void ResetCullingStats();
	void ResetDecalStats();
	void CollectWorldWithFrustum(UWorld* World, const FFrustum& ViewFrustum, const FShowFlags& ShowFlags, EViewMode ViewMode,
	                             FRenderBus& RenderBus);
	void CollectFromActor(AActor* Actor, const FShowFlags& ShowFlags, EViewMode ViewMode, FRenderBus& RenderBus);
	bool CollectFromSelectedActor(AActor* Actor, const FShowFlags& ShowFlags, EViewMode ViewMode, FRenderBus& RenderBus);
	void CollectFromComponent(UPrimitiveComponent* Primitive, const FShowFlags& ShowFlags, EViewMode ViewMode, FRenderBus& RenderBus);
	void CollectBVHInternalNodeAABBs(UPrimitiveComponent* PrimitiveComponent, const FShowFlags& ShowFlags, FRenderBus& RenderBus,
	                                 std::unordered_set<int32>& SeenNodeIndices);
	void CollectAABBCommand(const FAABB& Box, const FColor& Color, FRenderBus& RenderBus);
	void CollectAABBCommand(UPrimitiveComponent* PrimitiveComponent, const FShowFlags& ShowFlags, FRenderBus& RenderBus);
};
