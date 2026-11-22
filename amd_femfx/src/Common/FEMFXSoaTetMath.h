/*
MIT License

Copyright (c) 2019 Advanced Micro Devices, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//---------------------------------------------------------------------------------------
// Structure-of-arrays SIMD implementation of tetrahedron math functions
//---------------------------------------------------------------------------------------

#pragma once

#include "FEMFXVectorMath.h"
#include "FEMFXTetMath.h"
#include "FEMFXSoaSvd3x3.h"

namespace AMD
{
    template<class T>
    static FM_FORCE_INLINE void FmSetSoaFromHighwayAos(typename T::SoaVector3* result, const FmVector3* aos)
    {
        float soa[3][FmHighwayMaxLanes] = {};
        FmHighwayTransposeAosToSoa(3, &aos[0].x, &soa[0][0], T::width);
        for (int component = 0; component < 3; ++component)
            for (uint lane = 0; lane < T::width; ++lane)
                (component == 0 ? result->x : component == 1 ? result->y : result->z).setSlice(lane, soa[component][lane]);
    }

    template<class T>
    static FM_FORCE_INLINE void FmSetSoaFromHighwayAos(typename T::SoaVector4* result, const FmVector4* aos)
    {
        float soa[4][FmHighwayMaxLanes] = {};
        FmHighwayTransposeAosToSoa(4, &aos[0].x, &soa[0][0], T::width);
        for (int component = 0; component < 4; ++component)
            for (uint lane = 0; lane < T::width; ++lane)
                (component == 0 ? result->x : component == 1 ? result->y : component == 2 ? result->z : result->w).setSlice(lane, soa[component][lane]);
    }

    template<class T>
    struct FmSoaTetShapeParams
    {
        typename T::SoaMatrix4 baryMatrix;   // Matrix that transforms a point within the tet to its barycentric coords
        typename T::SoaFloat det;            // Determinant of matrix m = [[1, 1, 1, 1], [x0, x1, x2, x3], [y0, y1, y2, y3], [z0, z1, z2, z3]] = 6 * volume

        inline typename T::SoaFloat GetA0() const { return baryMatrix.col0.x; }
        inline typename T::SoaFloat GetA1() const { return baryMatrix.col0.y; }
        inline typename T::SoaFloat GetA2() const { return baryMatrix.col0.z; }
        inline typename T::SoaFloat GetA3() const { return baryMatrix.col0.w; }

        inline typename T::SoaFloat GetB0() const { return baryMatrix.col1.x; }
        inline typename T::SoaFloat GetB1() const { return baryMatrix.col1.y; }
        inline typename T::SoaFloat GetB2() const { return baryMatrix.col1.z; }
        inline typename T::SoaFloat GetB3() const { return baryMatrix.col1.w; }

        inline typename T::SoaFloat GetC0() const { return baryMatrix.col2.x; }
        inline typename T::SoaFloat GetC1() const { return baryMatrix.col2.y; }
        inline typename T::SoaFloat GetC2() const { return baryMatrix.col2.z; }
        inline typename T::SoaFloat GetC3() const { return baryMatrix.col2.w; }

        inline typename T::SoaFloat GetArea0() const { return 0.5f*det*sqrtf(GetA0()*GetA0() + GetB0()*GetB0() + GetC0()*GetC0()); }
        inline typename T::SoaFloat GetArea1() const { return 0.5f*det*sqrtf(GetA1()*GetA1() + GetB1()*GetB1() + GetC1()*GetC1()); }
        inline typename T::SoaFloat GetArea2() const { return 0.5f*det*sqrtf(GetA2()*GetA2() + GetB2()*GetB2() + GetC2()*GetC2()); }
        inline typename T::SoaFloat GetArea3() const { return 0.5f*det*sqrtf(GetA3()*GetA3() + GetB3()*GetB3() + GetC3()*GetC3()); }
        inline typename T::SoaVector3 GetNormal0() const { return -normalize(FmInitVector3(GetA0(), GetB0(), GetC0())); }
        inline typename T::SoaVector3 GetNormal1() const { return -normalize(FmInitVector3(GetA1(), GetB1(), GetC1())); }
        inline typename T::SoaVector3 GetNormal2() const { return -normalize(FmInitVector3(GetA2(), GetB2(), GetC2())); }
        inline typename T::SoaVector3 GetNormal3() const { return -normalize(FmInitVector3(GetA3(), GetB3(), GetC3())); }
        inline typename T::SoaFloat GetVolume() const { return det * (1.0f / 6.0f); }

        inline void ComputeRestPositions(typename T::SoaVector3* outRestPosition0, typename T::SoaVector3* outRestPosition1, typename T::SoaVector3* outRestPosition2, typename T::SoaVector3* outRestPosition3) const
        {
            typename T::SoaMatrix4 restPosMat = inverse(baryMatrix);

            typename T::SoaVector3 restPosition0 = restPosMat.col0.getXYZ();
            typename T::SoaVector3 restPosition1 = restPosMat.col1.getXYZ();
            typename T::SoaVector3 restPosition2 = restPosMat.col2.getXYZ();
            typename T::SoaVector3 restPosition3 = restPosMat.col3.getXYZ();

            *outRestPosition0 = restPosition0 - restPosition3;
            *outRestPosition1 = restPosition1 - restPosition3;
            *outRestPosition2 = restPosition2 - restPosition3;
            *outRestPosition3 = typename T::SoaVector3(0.0f);
        }
    };

    // Stress matrix, relating position offsets of tet vertices to stress
    template<class T>
    struct FmSoaTetStressMatrix
    {
        typename T::SoaMatrix3 EBe00;  // E . Be which is 6 x 12 stress matrix
        typename T::SoaMatrix3 EBe01;
        typename T::SoaMatrix3 EBe02;
        typename T::SoaMatrix3 EBe03;
        typename T::SoaMatrix3 EBe10;
        typename T::SoaMatrix3 EBe11;
        typename T::SoaMatrix3 EBe12;
        typename T::SoaMatrix3 EBe13;
    };

    // Stiffness matrix, relating position offsets of tet vertices to internal force
    template<class T>
    struct FmSoaTetStiffnessMatrix
    {
        typename T::SoaMatrix3 Ke_diag0;   // Symmetric 12 x 12 stiffness matrix, submatrices on diagonal
        typename T::SoaMatrix3 Ke_diag1;
        typename T::SoaMatrix3 Ke_diag2;
        typename T::SoaMatrix3 Ke_diag3;
        typename T::SoaMatrix3 Ke_lower0;  // Symmetric 12 x 12 stiffness matrix, submatrices below diagonal
        typename T::SoaMatrix3 Ke_lower1;
        typename T::SoaMatrix3 Ke_lower2;
        typename T::SoaMatrix3 Ke_lower3;
        typename T::SoaMatrix3 Ke_lower4;
        typename T::SoaMatrix3 Ke_lower5;
    };

    // Rotated stiffness matrix.
    // Matrix combines steps of un-rotating deformed element positions, applying stiffness, and rotating forces back.
    // Reference: Muller and Gross, "Interactive Virtual Materials"
    template<class T>
    struct FmSoaTetRotatedStiffnessMatrix
    {
        typename T::SoaMatrix3 Keprime_diag0;    // Submatrices on diagonal
        typename T::SoaMatrix3 Keprime_diag1;
        typename T::SoaMatrix3 Keprime_diag2;
        typename T::SoaMatrix3 Keprime_diag3;
        typename T::SoaMatrix3 Keprime_lower0;   // Submatrices below diagonal
        typename T::SoaMatrix3 Keprime_lower1;
        typename T::SoaMatrix3 Keprime_lower2;
        typename T::SoaMatrix3 Keprime_lower3;
        typename T::SoaMatrix3 Keprime_lower4;
        typename T::SoaMatrix3 Keprime_lower5;
        typename T::SoaVector3 f0eprime0;        // Term of the internal force expression containing the rest positions
        typename T::SoaVector3 f0eprime1;
        typename T::SoaVector3 f0eprime2;
        typename T::SoaVector3 f0eprime3;
    };

    template<class T>
    void FmGetSlice(
        FmTetShapeParams* result,
        const FmSoaTetShapeParams<T>& soaResult,
        uint idx)
    {
        result->baryMatrix = FmGetSlice<T>(soaResult.baryMatrix, idx);
        result->det = soaResult.det.getSlice(idx);
    }

    template<class T>
    void FmSetSlice(FmSoaTetShapeParams<T>* soaData, uint idx, const FmTetShapeParams& data)
    {
        FmSetSlice<T>(&soaData->baryMatrix, idx, data.baryMatrix);
        soaData->det.setSlice(idx, data.det);
    }

    template<class T>
    void FmGetSlice(
        FmTetStressMatrix* result,
        const FmSoaTetStressMatrix<T>& soaResult,
        uint idx)
    {
        result->EBe00 = FmGetSlice<T>(soaResult.EBe00, idx);
        result->EBe01 = FmGetSlice<T>(soaResult.EBe01, idx);
        result->EBe02 = FmGetSlice<T>(soaResult.EBe02, idx);
        result->EBe03 = FmGetSlice<T>(soaResult.EBe03, idx);
        result->EBe10 = FmGetSlice<T>(soaResult.EBe10, idx);
        result->EBe11 = FmGetSlice<T>(soaResult.EBe11, idx);
        result->EBe12 = FmGetSlice<T>(soaResult.EBe12, idx);
        result->EBe13 = FmGetSlice<T>(soaResult.EBe13, idx);
    }

    template<class T>
    void FmGetSlice(
        FmTetStiffnessMatrix* result,
        const FmSoaTetStiffnessMatrix<T>& soaResult,
        uint idx)
    {
        result->Ke_diag0 = FmGetSlice<T>(soaResult.Ke_diag0, idx);
        result->Ke_diag1 = FmGetSlice<T>(soaResult.Ke_diag1, idx);
        result->Ke_diag2 = FmGetSlice<T>(soaResult.Ke_diag2, idx);
        result->Ke_diag3 = FmGetSlice<T>(soaResult.Ke_diag3, idx);

        result->Ke_lower0 = FmGetSlice<T>(soaResult.Ke_lower0, idx);
        result->Ke_lower1 = FmGetSlice<T>(soaResult.Ke_lower1, idx);
        result->Ke_lower2 = FmGetSlice<T>(soaResult.Ke_lower2, idx);
        result->Ke_lower3 = FmGetSlice<T>(soaResult.Ke_lower3, idx);
        result->Ke_lower4 = FmGetSlice<T>(soaResult.Ke_lower4, idx);
        result->Ke_lower5 = FmGetSlice<T>(soaResult.Ke_lower5, idx);
    }

    template<class T>
    void FmGetSlice(
        FmTetRotatedStiffnessMatrix* result,
        const FmSoaTetRotatedStiffnessMatrix<T>& soaResult,
        uint idx)
    {
        result->Keprime_diag0 = FmGetSlice<T>(soaResult.Keprime_diag0, idx);
        result->Keprime_diag1 = FmGetSlice<T>(soaResult.Keprime_diag1, idx);
        result->Keprime_diag2 = FmGetSlice<T>(soaResult.Keprime_diag2, idx);
        result->Keprime_diag3 = FmGetSlice<T>(soaResult.Keprime_diag3, idx);

        result->Keprime_lower0 = FmGetSlice<T>(soaResult.Keprime_lower0, idx);
        result->Keprime_lower1 = FmGetSlice<T>(soaResult.Keprime_lower1, idx);
        result->Keprime_lower2 = FmGetSlice<T>(soaResult.Keprime_lower2, idx);
        result->Keprime_lower3 = FmGetSlice<T>(soaResult.Keprime_lower3, idx);
        result->Keprime_lower4 = FmGetSlice<T>(soaResult.Keprime_lower4, idx);
        result->Keprime_lower5 = FmGetSlice<T>(soaResult.Keprime_lower5, idx);

        result->f0eprime0 = FmGetSlice<T>(soaResult.f0eprime0, idx);
        result->f0eprime1 = FmGetSlice<T>(soaResult.f0eprime1, idx);
        result->f0eprime2 = FmGetSlice<T>(soaResult.f0eprime2, idx);
        result->f0eprime3 = FmGetSlice<T>(soaResult.f0eprime3, idx);
    }

    template<class T>
    void FmSetSlice(FmSoaTetStiffnessMatrix<T>* soaMat, uint idx, const FmTetStiffnessMatrix& mat)
    {
        FmSetSlice<T>(&soaMat->Ke_diag0, idx, mat.Ke_diag0);
        FmSetSlice<T>(&soaMat->Ke_diag1, idx, mat.Ke_diag1);
        FmSetSlice<T>(&soaMat->Ke_diag2, idx, mat.Ke_diag2);
        FmSetSlice<T>(&soaMat->Ke_diag3, idx, mat.Ke_diag3);

        FmSetSlice<T>(&soaMat->Ke_lower0, idx, mat.Ke_lower0);
        FmSetSlice<T>(&soaMat->Ke_lower1, idx, mat.Ke_lower1);
        FmSetSlice<T>(&soaMat->Ke_lower2, idx, mat.Ke_lower2);
        FmSetSlice<T>(&soaMat->Ke_lower3, idx, mat.Ke_lower3);
        FmSetSlice<T>(&soaMat->Ke_lower4, idx, mat.Ke_lower4);
        FmSetSlice<T>(&soaMat->Ke_lower5, idx, mat.Ke_lower5);
    }

    template<class T>
    void FmComputeShapeParams(
        FmSoaTetShapeParams<T>* resultShapeParams,
        const typename T::SoaVector3& p0,
        const typename T::SoaVector3& p1,
        const typename T::SoaVector3& p2,
        const typename T::SoaVector3& p3)
    {
        const typename T::SoaVector3* positions[4] = {&p0, &p1, &p2, &p3};
        float packed[12][FmHighwayMaxLanes] = {};
        float bary[16][FmHighwayMaxLanes] = {};
        float det[FmHighwayMaxLanes] = {};
        for (int point = 0; point < 4; ++point)
            for (int component = 0; component < 3; ++component)
                for (uint lane = 0; lane < T::width; ++lane)
                    packed[point * 3 + component][lane] =
                        (component == 0 ? positions[point]->x : component == 1 ? positions[point]->y : positions[point]->z).getSlice(lane);
        FmHighwayComputeShapeParams(&packed[0][0], &bary[0][0], det, T::width);
        typename T::SoaVector4* columns[4] = {&resultShapeParams->baryMatrix.col0,
            &resultShapeParams->baryMatrix.col1, &resultShapeParams->baryMatrix.col2,
            &resultShapeParams->baryMatrix.col3};
        for (int column = 0; column < 4; ++column)
            for (int component = 0; component < 4; ++component)
                for (uint lane = 0; lane < T::width; ++lane)
                    (component == 0 ? columns[column]->x : component == 1 ? columns[column]->y : component == 2 ? columns[column]->z : columns[column]->w).setSlice(lane, bary[column * 4 + component][lane]);
        for (uint lane = 0; lane < T::width; ++lane)
            resultShapeParams->det.setSlice(lane, det[lane]);
    }

    template<class T>
    void FmComputeTetStrain(
        typename T::SoaVector3 strain[2],
        const FmSoaTetShapeParams<T>& shapeParams,
        const typename T::SoaVector3& unrotatedOffset0,
        const typename T::SoaVector3& unrotatedOffset1,
        const typename T::SoaVector3& unrotatedOffset2,
        const typename T::SoaVector3& unrotatedOffset3)
    {
        const typename T::SoaVector3* offsets[4] = {&unrotatedOffset0, &unrotatedOffset1, &unrotatedOffset2, &unrotatedOffset3};
        float bary[12][FmHighwayMaxLanes] = {};
        float packedOffsets[12][FmHighwayMaxLanes] = {};
        float result[6][FmHighwayMaxLanes] = {};
        typename T::SoaFloat coefficients[12] = {shapeParams.GetA0(), shapeParams.GetA1(), shapeParams.GetA2(), shapeParams.GetA3(),
            shapeParams.GetB0(), shapeParams.GetB1(), shapeParams.GetB2(), shapeParams.GetB3(),
            shapeParams.GetC0(), shapeParams.GetC1(), shapeParams.GetC2(), shapeParams.GetC3()};
        for (int i = 0; i < 12; ++i)
            for (uint lane = 0; lane < T::width; ++lane) bary[i][lane] = coefficients[i].getSlice(lane);
        for (int point = 0; point < 4; ++point)
            for (int component = 0; component < 3; ++component)
                for (uint lane = 0; lane < T::width; ++lane)
                    packedOffsets[point * 3 + component][lane] =
                        (component == 0 ? offsets[point]->x : component == 1 ? offsets[point]->y : offsets[point]->z).getSlice(lane);
        FmHighwayComputeTetStrain(&bary[0][0], &packedOffsets[0][0], &result[0][0], T::width);
        typename T::SoaFloat* output[6] = {&strain[0].x, &strain[0].y, &strain[0].z, &strain[1].x, &strain[1].y, &strain[1].z};
        for (int i = 0; i < 6; ++i)
            for (uint lane = 0; lane < T::width; ++lane) output[i]->setSlice(lane, result[i][lane]);
    }

    // Compute stress matrix from shape and material parameters
    template<class T>
    void FmComputeTetStressMatrix(
        FmSoaTetStressMatrix<T>* stressMat,
        const typename T::SoaVector4& baryMatrixCol0,
        const typename T::SoaVector4& baryMatrixCol1,
        const typename T::SoaVector4& baryMatrixCol2,
        typename T::SoaFloat youngsModulus,
        typename T::SoaFloat poissonsRatio)
    {
        float bary[12][FmHighwayMaxLanes] = {};
        float volume[FmHighwayMaxLanes] = {};
        float young[FmHighwayMaxLanes] = {};
        float poisson[FmHighwayMaxLanes] = {};
        float stress[72][FmHighwayMaxLanes] = {};
        float stiffness[90][FmHighwayMaxLanes] = {};
        const typename T::SoaFloat* coefficients[12] = {
            &baryMatrixCol0.x, &baryMatrixCol0.y, &baryMatrixCol0.z, &baryMatrixCol0.w,
            &baryMatrixCol1.x, &baryMatrixCol1.y, &baryMatrixCol1.z, &baryMatrixCol1.w,
            &baryMatrixCol2.x, &baryMatrixCol2.y, &baryMatrixCol2.z, &baryMatrixCol2.w};
        for (int i = 0; i < 12; ++i)
            for (uint lane = 0; lane < T::width; ++lane) bary[i][lane] = coefficients[i]->getSlice(lane);
        for (uint lane = 0; lane < T::width; ++lane)
        {
            volume[lane] = 1.0f;
            young[lane] = youngsModulus.getSlice(lane);
            poisson[lane] = poissonsRatio.getSlice(lane);
        }
        FmHighwayComputeTetStressAndStiffness(&bary[0][0], volume, young, poisson,
            &stress[0][0], &stiffness[0][0], T::width);
        typename T::SoaMatrix3* output[8] = {&stressMat->EBe00, &stressMat->EBe01, &stressMat->EBe02, &stressMat->EBe03,
            &stressMat->EBe10, &stressMat->EBe11, &stressMat->EBe12, &stressMat->EBe13};
        for (int matrix = 0; matrix < 8; ++matrix)
            FmUnpackSoaMatrix3<T>(output[matrix], &stress[matrix * 9]);
    }

    // Compute stress and stiffness matrix from shape and material parameters
    template<class T>
    void FmComputeTetStressAndStiffnessMatrix(
        FmSoaTetStressMatrix<T>* stressMat, FmSoaTetStiffnessMatrix<T>* stiffnessMat,
        const typename T::SoaVector4& baryMatrixCol0,
        const typename T::SoaVector4& baryMatrixCol1,
        const typename T::SoaVector4& baryMatrixCol2,
        typename T::SoaFloat volume, typename T::SoaFloat youngsModulus,
        typename T::SoaFloat poissonsRatio)
    {
        float bary[12][FmHighwayMaxLanes] = {};
        float volumePacked[FmHighwayMaxLanes] = {};
        float young[FmHighwayMaxLanes] = {};
        float poisson[FmHighwayMaxLanes] = {};
        float stress[72][FmHighwayMaxLanes] = {};
        float stiffness[90][FmHighwayMaxLanes] = {};
        const typename T::SoaFloat* coefficients[12] = {
            &baryMatrixCol0.x, &baryMatrixCol0.y, &baryMatrixCol0.z, &baryMatrixCol0.w,
            &baryMatrixCol1.x, &baryMatrixCol1.y, &baryMatrixCol1.z, &baryMatrixCol1.w,
            &baryMatrixCol2.x, &baryMatrixCol2.y, &baryMatrixCol2.z, &baryMatrixCol2.w};
        for (int i = 0; i < 12; ++i)
            for (uint lane = 0; lane < T::width; ++lane) bary[i][lane] = coefficients[i]->getSlice(lane);
        for (uint lane = 0; lane < T::width; ++lane)
        {
            volumePacked[lane] = volume.getSlice(lane);
            young[lane] = youngsModulus.getSlice(lane);
            poisson[lane] = poissonsRatio.getSlice(lane);
        }
        FmHighwayComputeTetStressAndStiffness(&bary[0][0], volumePacked, young, poisson,
            &stress[0][0], &stiffness[0][0], T::width);
        typename T::SoaMatrix3* stressOutput[8] = {&stressMat->EBe00, &stressMat->EBe01, &stressMat->EBe02, &stressMat->EBe03,
            &stressMat->EBe10, &stressMat->EBe11, &stressMat->EBe12, &stressMat->EBe13};
        typename T::SoaMatrix3* stiffnessOutput[10] = {&stiffnessMat->Ke_diag0, &stiffnessMat->Ke_diag1, &stiffnessMat->Ke_diag2, &stiffnessMat->Ke_diag3,
            &stiffnessMat->Ke_lower0, &stiffnessMat->Ke_lower1, &stiffnessMat->Ke_lower2, &stiffnessMat->Ke_lower3, &stiffnessMat->Ke_lower4, &stiffnessMat->Ke_lower5};
        for (int matrix = 0; matrix < 8; ++matrix)
            FmUnpackSoaMatrix3<T>(stressOutput[matrix], &stress[matrix * 9]);
        for (int matrix = 0; matrix < 10; ++matrix)
            FmUnpackSoaMatrix3<T>(stiffnessOutput[matrix], &stiffness[matrix * 9]);
    }

    template<class T>
    void FmComputeRotatedStiffnessMatrix(
        FmSoaTetRotatedStiffnessMatrix<T>* rotatedStiffnessMat,
        const FmSoaTetStiffnessMatrix<T>& stiffnessMat,
        const typename T::SoaVector3& tetRestPosition0,
        const typename T::SoaVector3& tetRestPosition1,
        const typename T::SoaVector3& tetRestPosition2,
        const typename T::SoaVector3& tetRestPosition3,
        const typename T::SoaMatrix3& tetRotation)
    {
        float stiffness[90][FmHighwayMaxLanes] = {};
        float positions[12][FmHighwayMaxLanes] = {};
        float rotation[9][FmHighwayMaxLanes] = {};
        float rotated[90][FmHighwayMaxLanes] = {};
        float force[12][FmHighwayMaxLanes] = {};
        const typename T::SoaMatrix3* inputMatrices[10] = {&stiffnessMat.Ke_diag0, &stiffnessMat.Ke_diag1, &stiffnessMat.Ke_diag2, &stiffnessMat.Ke_diag3,
            &stiffnessMat.Ke_lower0, &stiffnessMat.Ke_lower1, &stiffnessMat.Ke_lower2, &stiffnessMat.Ke_lower3, &stiffnessMat.Ke_lower4, &stiffnessMat.Ke_lower5};
        for (int matrix = 0; matrix < 10; ++matrix)
            FmPackSoaMatrix3<T>(&stiffness[matrix * 9], *inputMatrices[matrix]);
        const typename T::SoaVector3* inputPositions[4] = {&tetRestPosition0, &tetRestPosition1, &tetRestPosition2, &tetRestPosition3};
        for (int point = 0; point < 4; ++point)
            FmPackSoaVector3<T>(&positions[point * 3], *inputPositions[point]);
        FmPackSoaMatrix3<T>(rotation, tetRotation);
        FmHighwayComputeRotatedStiffness(&stiffness[0][0], &positions[0][0], &rotation[0][0],
            &rotated[0][0], &force[0][0], T::width);
        typename T::SoaMatrix3* outputMatrices[10] = {&rotatedStiffnessMat->Keprime_diag0, &rotatedStiffnessMat->Keprime_diag1, &rotatedStiffnessMat->Keprime_diag2, &rotatedStiffnessMat->Keprime_diag3,
            &rotatedStiffnessMat->Keprime_lower0, &rotatedStiffnessMat->Keprime_lower1, &rotatedStiffnessMat->Keprime_lower2, &rotatedStiffnessMat->Keprime_lower3, &rotatedStiffnessMat->Keprime_lower4, &rotatedStiffnessMat->Keprime_lower5};
        typename T::SoaVector3* outputForces[4] = {&rotatedStiffnessMat->f0eprime0, &rotatedStiffnessMat->f0eprime1, &rotatedStiffnessMat->f0eprime2, &rotatedStiffnessMat->f0eprime3};
        for (int matrix = 0; matrix < 10; ++matrix)
            FmUnpackSoaMatrix3<T>(outputMatrices[matrix], &rotated[matrix * 9]);
        for (int point = 0; point < 4; ++point)
            FmUnpackSoaVector3<T>(outputForces[point], &force[point * 3]);
    }

    template<class T>
    typename T::SoaMatrix3 FmComputeTetRotationPolarDecompSvd(
        const typename T::SoaVector3& tetDeformedPosition0,
        const typename T::SoaVector3& tetDeformedPosition1,
        const typename T::SoaVector3& tetDeformedPosition2,
        const typename T::SoaVector3& tetDeformedPosition3,
        const typename T::SoaMatrix4& tetRestBaryMatrix)
    {
        // Deformation of any point in tetrahedron = Barycentric2DeformedPos * OriginalPos2Barycentric
        typename T::SoaMatrix4 deformationTransform = mul(typename T::SoaMatrix4(
            typename T::SoaVector4(tetDeformedPosition0, typename T::SoaFloat(1.0f)),
            typename T::SoaVector4(tetDeformedPosition1, typename T::SoaFloat(1.0f)),
            typename T::SoaVector4(tetDeformedPosition2, typename T::SoaFloat(1.0f)),
            typename T::SoaVector4(tetDeformedPosition3, typename T::SoaFloat(1.0f))), tetRestBaryMatrix);

        typename T::SoaMatrix3 U, V;
        typename T::SoaVector3 sigma;
        FmSvd3x3<T>(&U, &sigma, &V, deformationTransform.getUpper3x3());

        return mul(U, transpose(V));
    }

    // Compute volume matrix for a tetrahedron, needed for deformation gradient
    template<class T>
    static FM_FORCE_INLINE typename T::SoaMatrix3 FmComputeTetVolumeMatrix(
        const typename T::SoaVector3& position0, 
        const typename T::SoaVector3& position1, 
        const typename T::SoaVector3& position2, 
        const typename T::SoaVector3& position3)
    {
        typename T::SoaMatrix3 volumeMatrix;
        volumeMatrix.col0 = position0 - position3;
        volumeMatrix.col1 = position1 - position3;
        volumeMatrix.col2 = position2 - position3;
        return volumeMatrix;
    }

    // Compute elastic deformation as the total deformation gradient multiplied by inverse of plastic deformation matrix.
    // Returns the SVD of this elastic deformation = U * Fhat * V^T, where each component of Fhat gives a stretch or compression factor.
    // The plastic deformation matrix can be updated using this decomposition.
    template<class T>
    static FM_FORCE_INLINE void FmComputeTetElasticDeformationGradient(
        typename T::SoaMatrix3* U,
        typename T::SoaVector3* Fhat,
        typename T::SoaMatrix3* V,
        const typename T::SoaVector3& tetDeformedPosition0,
        const typename T::SoaVector3& tetDeformedPosition1,
        const typename T::SoaVector3& tetDeformedPosition2,
        const typename T::SoaVector3& tetDeformedPosition3,
        const typename T::SoaMatrix3& restVolumeMatrixInv,
        const typename T::SoaMatrix3& plasticDeformationMatrixInv)
    {
        typename T::SoaMatrix3 volumeMatrix = FmComputeTetVolumeMatrix<T>(tetDeformedPosition0, tetDeformedPosition1, tetDeformedPosition2, tetDeformedPosition3);

        typename T::SoaMatrix3 F = mul(mul(volumeMatrix, restVolumeMatrixInv), plasticDeformationMatrixInv);

        FmSvd3x3<T>(U, Fhat, V, F);
    }
}