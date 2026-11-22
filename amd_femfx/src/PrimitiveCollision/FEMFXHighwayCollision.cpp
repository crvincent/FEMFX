#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "FEMFXHighwayCollision.cpp"

#include "hwy/foreach_target.h"
#include "hwy/highway.h"

#include "FEMFXHighwayCollision.h"

HWY_BEFORE_NAMESPACE();

namespace AMD
{
namespace HWY_NAMESPACE
{
    namespace hn = hwy::HWY_NAMESPACE;

    template<class Kernel, class Data, class Output>
    static void FmHighwayForEachPair(
        Output* output,
        const Data* input,
        size_t numPairs,
        Kernel kernel)
    {
        const hn::ScalableTag<float> d;
        const size_t lanes = hn::Lanes(d);

        for (size_t pairIdx = 0; pairIdx < numPairs; pairIdx += lanes)
        {
            const size_t active = (numPairs - pairIdx < lanes) ?
                numPairs - pairIdx : lanes;
            for (size_t lane = 0; lane < active; lane++)
            {
                kernel(output + pairIdx + lane, input[pairIdx + lane]);
            }
        }
    }

    static void FmTriIntersectionBatchImpl(
        bool* pairIntersects,
        FmTriIntersectionPoints* intersectionPoints,
        const FmVector3* triA,
        const FmVector3* triB,
        size_t numPairs)
    {
        struct Pair
        {
            const FmVector3* a;
            const FmVector3* b;
        };
        const hn::ScalableTag<float> d;
        const size_t lanes = hn::Lanes(d);

        for (size_t pairIdx = 0; pairIdx < numPairs; pairIdx += lanes)
        {
            const size_t active = (numPairs - pairIdx < lanes) ?
                numPairs - pairIdx : lanes;
            for (size_t lane = 0; lane < active; lane++)
            {
                const Pair pair = { triA + 3 * (pairIdx + lane), triB + 3 * (pairIdx + lane) };
                pairIntersects[pairIdx + lane] = FmComputeTriIntersection(
                    intersectionPoints + pairIdx + lane, pair.a, pair.b);
            }
        }
    }

    static void FmEdgeEdgeCcdBatchImpl(
        FmCcdResult* results,
        const FmEdgeEdgePairData* pairs,
        float deltaTime,
        const FmCcdTerminationConditions& conditions,
        size_t numPairs)
    {
        FmHighwayForEachPair(
            results, pairs, numPairs,
            [deltaTime, &conditions](FmCcdResult* output, const FmEdgeEdgePairData& input)
            {
                FmEdgeEdgeCcd(output, input, deltaTime, conditions);
            });
    }

    static void FmVertexFaceCcdBatchImpl(
        FmCcdResult* results,
        const FmVertexFacePairData* pairs,
        float deltaTime,
        const FmCcdTerminationConditions& conditions,
        size_t numPairs)
    {
        FmHighwayForEachPair(
            results, pairs, numPairs,
            [deltaTime, &conditions](FmCcdResult* output, const FmVertexFacePairData& input)
            {
                FmVertexFaceCcd(output, input, deltaTime, conditions);
            });
    }

    static void FmFaceVertexCcdBatchImpl(
        FmCcdResult* results,
        const FmFaceVertexPairData* pairs,
        float deltaTime,
        const FmCcdTerminationConditions& conditions,
        size_t numPairs)
    {
        FmHighwayForEachPair(
            results, pairs, numPairs,
            [deltaTime, &conditions](FmCcdResult* output, const FmFaceVertexPairData& input)
            {
                FmFaceVertexCcd(output, input, deltaTime, conditions);
            });
    }
}
}

HWY_AFTER_NAMESPACE();

#if HWY_ONCE
namespace AMD
{
    HWY_EXPORT(FmTriIntersectionBatchImpl);
    HWY_EXPORT(FmEdgeEdgeCcdBatchImpl);
    HWY_EXPORT(FmVertexFaceCcdBatchImpl);
    HWY_EXPORT(FmFaceVertexCcdBatchImpl);

    void FmHighwayTriIntersectionBatch(
        bool* pairIntersects,
        FmTriIntersectionPoints* intersectionPoints,
        const FmVector3* triA,
        const FmVector3* triB,
        size_t numPairs)
    {
        HWY_DYNAMIC_DISPATCH(FmTriIntersectionBatchImpl)(
            pairIntersects, intersectionPoints, triA, triB, numPairs);
    }

    void FmHighwayEdgeEdgeCcdBatch(
        FmCcdResult* results,
        const FmEdgeEdgePairData* pairs,
        float deltaTime,
        const FmCcdTerminationConditions& conditions,
        size_t numPairs)
    {
        HWY_DYNAMIC_DISPATCH(FmEdgeEdgeCcdBatchImpl)(
            results, pairs, deltaTime, conditions, numPairs);
    }

    void FmHighwayVertexFaceCcdBatch(
        FmCcdResult* results,
        const FmVertexFacePairData* pairs,
        float deltaTime,
        const FmCcdTerminationConditions& conditions,
        size_t numPairs)
    {
        HWY_DYNAMIC_DISPATCH(FmVertexFaceCcdBatchImpl)(
            results, pairs, deltaTime, conditions, numPairs);
    }

    void FmHighwayFaceVertexCcdBatch(
        FmCcdResult* results,
        const FmFaceVertexPairData* pairs,
        float deltaTime,
        const FmCcdTerminationConditions& conditions,
        size_t numPairs)
    {
        HWY_DYNAMIC_DISPATCH(FmFaceVertexCcdBatchImpl)(
            results, pairs, deltaTime, conditions, numPairs);
    }
}
#endif
