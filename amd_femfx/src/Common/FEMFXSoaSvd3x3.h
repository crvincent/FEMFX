/*
MIT License

Copyright (c) 2019 Advanced Micro Devices, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "FEMFXVectorMath.h"
#include "FEMFXHighway.h"

namespace AMD
{
    template<class T>
    static FM_FORCE_INLINE void FmPackSoaMatrix3(
        float values[9][FmHighwayMaxLanes], const typename T::SoaMatrix3& matrix)
    {
        const typename T::SoaFloat* columns[3][3] = {
            { &matrix.col0.x, &matrix.col0.y, &matrix.col0.z },
            { &matrix.col1.x, &matrix.col1.y, &matrix.col1.z },
            { &matrix.col2.x, &matrix.col2.y, &matrix.col2.z }
        };
        for (int component = 0; component < 9; ++component)
        {
            for (uint lane = 0; lane < T::width; ++lane)
                values[component][lane] =
                    columns[component / 3][component % 3]->getSlice(lane);
        }
    }

    template<class T>
    static FM_FORCE_INLINE void FmUnpackSoaMatrix3(
        typename T::SoaMatrix3* matrix, const float values[9][FmHighwayMaxLanes])
    {
        typename T::SoaFloat* columns[3][3] = {
            { &matrix->col0.x, &matrix->col0.y, &matrix->col0.z },
            { &matrix->col1.x, &matrix->col1.y, &matrix->col1.z },
            { &matrix->col2.x, &matrix->col2.y, &matrix->col2.z }
        };
        for (int component = 0; component < 9; ++component)
        {
            for (uint lane = 0; lane < T::width; ++lane)
                columns[component / 3][component % 3]->setSlice(
                    lane, values[component][lane]);
        }
    }

    template<class T>
    static FM_FORCE_INLINE void FmPackSoaVector3(
        float values[3][FmHighwayMaxLanes], const typename T::SoaVector3& vector)
    {
        const typename T::SoaFloat* components[3] = {&vector.x, &vector.y, &vector.z};
        for (int component = 0; component < 3; ++component)
            for (uint lane = 0; lane < T::width; ++lane)
                values[component][lane] = components[component]->getSlice(lane);
    }

    template<class T>
    static FM_FORCE_INLINE void FmUnpackSoaVector3(
        typename T::SoaVector3* vector, const float values[3][FmHighwayMaxLanes])
    {
        typename T::SoaFloat* components[3] = {&vector->x, &vector->y, &vector->z};
        for (int component = 0; component < 3; ++component)
            for (uint lane = 0; lane < T::width; ++lane)
                components[component]->setSlice(lane, values[component][lane]);
    }

    // Runtime-dispatched Highway Jacobi eigensolver. The public SoA wrappers are
    // retained only as conversion boundaries for the existing runtime data.
    template<class T>
    void FmEigenSymm3x3CyclicJacobi(
        typename T::SoaVector3* eigenvals,
        typename T::SoaMatrix3* eigenvectors,
        const typename T::SoaMatrix3& symmMat)
    {
        float matrix[9][FmHighwayMaxLanes] = {};
        float values[3][FmHighwayMaxLanes] = {};
        float vectors[9][FmHighwayMaxLanes] = {};
        FmPackSoaMatrix3<T>(matrix, symmMat);
        FmHighwayEigenSymm3x3(&matrix[0][0], &values[0][0], &vectors[0][0], T::width);
        FmUnpackSoaVector3<T>(eigenvals, values);
        FmUnpackSoaMatrix3<T>(eigenvectors, vectors);
    }

    template<class T>
    void FmSvd3x3(
        typename T::SoaMatrix3* U,
        typename T::SoaVector3* sigma,
        typename T::SoaMatrix3* V,
        const typename T::SoaMatrix3& mat,
        typename T::SoaFloat givensTolerance = typename T::SoaFloat(FLT_EPSILON))
    {
        float matrix[9][FmHighwayMaxLanes] = {};
        float left[9][FmHighwayMaxLanes] = {};
        float values[3][FmHighwayMaxLanes] = {};
        float right[9][FmHighwayMaxLanes] = {};
        float tolerance[FmHighwayMaxLanes] = {};
        FmPackSoaMatrix3<T>(matrix, mat);
        for (uint lane = 0; lane < T::width; ++lane)
            tolerance[lane] = givensTolerance.getSlice(lane);
        FmHighwaySvd3x3(
            &matrix[0][0], &left[0][0], &values[0][0], &right[0][0],
            tolerance, T::width);
        FmUnpackSoaMatrix3<T>(U, left);
        FmUnpackSoaVector3<T>(sigma, values);
        FmUnpackSoaMatrix3<T>(V, right);
    }

    template<class T>
    typename T::SoaMatrix3 FmPseudoInverse(const typename T::SoaMatrix3& mat)
    {
        typename T::SoaMatrix3 U, V;
        typename T::SoaVector3 sigma;
        FmSvd3x3(&U, &sigma, &V, mat);

        typename T::SoaVector3 absSigma = abs(sigma);
        typename T::SoaFloat tol = FLT_EPSILON * 3.0f *
            FmMaxFloat(absSigma.x, FmMaxFloat(absSigma.y, absSigma.z));
        sigma.x = select(0.0f, 1.0f / sigma.x, absSigma.x > tol);
        sigma.y = select(0.0f, 1.0f / sigma.y, absSigma.y > tol);
        sigma.z = select(0.0f, 1.0f / sigma.z, absSigma.z > tol);
        return mul(V, mul(T::SoaMatrix3::scale(sigma), transpose(U)));
    }
}
