#pragma once

#include "FEMFXTriCcd.h"
#include "FEMFXTriIntersection.h"

#define FM_HIGHWAY_COLLISION_BATCH_CAPACITY 16

namespace AMD
{
    void FmHighwayTriIntersectionBatch(
        bool* pairIntersects,
        FmTriIntersectionPoints* intersectionPoints,
        const FmVector3* triA,
        const FmVector3* triB,
        size_t numPairs);

    void FmHighwayEdgeEdgeCcdBatch(
        FmCcdResult* results,
        const FmEdgeEdgePairData* pairs,
        float deltaTime,
        const FmCcdTerminationConditions& conditions,
        size_t numPairs);

    void FmHighwayVertexFaceCcdBatch(
        FmCcdResult* results,
        const FmVertexFacePairData* pairs,
        float deltaTime,
        const FmCcdTerminationConditions& conditions,
        size_t numPairs);

    void FmHighwayFaceVertexCcdBatch(
        FmCcdResult* results,
        const FmFaceVertexPairData* pairs,
        float deltaTime,
        const FmCcdTerminationConditions& conditions,
        size_t numPairs);
}
