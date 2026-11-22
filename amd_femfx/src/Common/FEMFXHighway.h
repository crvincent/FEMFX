#pragma once

#include <stddef.h>

namespace AMD
{
    // Batched Highway kernels use an 8-lane, structure-of-arrays layout.  The
    // legacy SoA width may be four or eight; count always selects valid lanes.
    static const size_t FmHighwayMaxLanes = 8;

    void FmHighwayTransposeAosToSoa(
        size_t components,
        const float* aos,
        float* soa,
        size_t count);

    void FmHighwayEigenSymm3x3(
        const float* matrix,
        float* eigenvalues,
        float* eigenvectors,
        size_t count);

    void FmHighwaySvd3x3(
        const float* matrix,
        float* u,
        float* singularValues,
        float* v,
        const float* tolerance,
        size_t count);

    void FmHighwayComputeShapeParams(
        const float* positions,
        float* barycentricMatrix,
        float* determinant,
        size_t count);

    void FmHighwayComputeTetStrain(
        const float* barycentricMatrix,
        const float* offsets,
        float* strain,
        size_t count);

    void FmHighwayComputeTetStressAndStiffness(
        const float* barycentricMatrix,
        const float* volume,
        const float* youngsModulus,
        const float* poissonsRatio,
        float* stressMatrix,
        float* stiffnessMatrix,
        size_t count);

    void FmHighwayComputeRotatedStiffness(
        const float* stiffnessMatrix,
        const float* restPositions,
        const float* rotation,
        float* rotatedStiffnessMatrix,
        float* restForce,
        size_t count);
}
