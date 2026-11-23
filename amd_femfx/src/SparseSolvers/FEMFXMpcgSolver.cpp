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

#include "FEMFXMpcgSolver.h"
#include "FEMFXParallelFor.h"
#include "FEMFXTetMesh.h"

namespace AMD
{
    void FmAllocMpcgSolverData(
        FmMpcgSolverData* data,
        uint maxVerts,
        uint maxVertAdjacentVerts)
    {
        data->kinematicFlags = new bool[maxVerts];
        data->PInvDiag = new FmSMatrix3[maxVerts];
        data->b = new FmSVector3[maxVerts];
        data->mass = new float[maxVerts];
        data->norms.Zero();
        data->solverStateOffset = 0;
        data->A.submats = new FmSMatrix3[maxVerts + maxVertAdjacentVerts];
        data->A.indices = new uint[maxVerts + maxVertAdjacentVerts];
        data->A.rowStarts = new uint[maxVerts + 1];
        data->A.numRows = 0;
        data->maxVertAdjacentVerts = maxVertAdjacentVerts;
        data->hasKinematicVerts = false;
    }

    void FmAllocMpcgSolverData(FmMpcgSolverData* data, const FmTetMesh& tetMesh)
    {
        FmAllocMpcgSolverData(data, tetMesh.maxVerts, tetMesh.maxVerts + tetMesh.vertConnectivity.numIncidentTetsTotal * 4);
    }

    void FmFreeMpcgSolverData(FmMpcgSolverData* data)
    {
        FM_DELETE_ARRAY(data->kinematicFlags);
        FM_DELETE_ARRAY(data->PInvDiag);
        FM_DELETE_ARRAY(data->b);
        FM_DELETE_ARRAY(data->mass);
        FM_DELETE_ARRAY(data->A.submats);
        FM_DELETE_ARRAY(data->A.indices);
        FM_DELETE_ARRAY(data->A.rowStarts);

        data->A.numRows = 0;
        data->maxVertAdjacentVerts = 0;
        data->solverStateOffset = 0;
        data->hasKinematicVerts = false;
    }

    struct FmMpcgParallelTaskData
    {
        FmMpcgSolverData* solverData;
        FmMpcgSolverDataTemps* temps;
        FmSVector3* solution;
        uint numRows;
        uint numTasks;
        float alpha;
        float deltaDivDeltaOld;
    };

    static uint FmGetMpcgNumTasks(uint numRows, uint numThreads)
    {
        // Adapted from https://github.com/ericjlarsen/FEMFX
        const uint minRowsPerTask = 1024;
        uint maxTasks = FmMaxUint(numThreads * 4, 1);
        uint numTasks = FmGetNumTasks(numRows, minRowsPerTask);
        return FmMinUint(numTasks, maxTasks);
    }

    static void FmGetMpcgTaskRange(uint* begin, uint* end, const FmMpcgParallelTaskData& taskData, int32_t taskIndex)
    {
        FmGetIndexRangeEvenDistribution(begin, end, (uint)taskIndex, taskData.numTasks, taskData.numRows);
    }

    static void FmRunMpcgParallelFor(
        const char* taskName,
        FmTaskFuncCallback taskFunc,
        FmMpcgParallelTaskData* taskData,
        FmParallelForCallback parallelFor)
    {
        if (parallelFor && taskData->numTasks > 1)
        {
            parallelFor(taskName, taskFunc, taskData, (int32_t)taskData->numTasks);
        }
        else
        {
            for (uint taskIndex = 0; taskIndex < taskData->numTasks; taskIndex++)
            {
                taskFunc(taskData, (int32_t)taskIndex, (int32_t)taskIndex + 1);
            }
        }
    }

    static void FmTaskMpcgSetup(void* inTaskData, int32_t inTaskBeginIndex, int32_t)
    {
        FmMpcgParallelTaskData& taskData = *(FmMpcgParallelTaskData*)inTaskData;
        FmMpcgSolverData& data = *taskData.solverData;
        FmMpcgSolverDataTemps& temps = *taskData.temps;

        uint begin, end;
        FmGetMpcgTaskRange(&begin, &end, taskData, inTaskBeginIndex);

        float bdelta = 0.0f;
        float delta = 0.0f;
        for (uint row3 = begin; row3 < end; row3++)
        {
            FmSVector3 sResult = FmInitSVector3(0.0f);
            FmSVector3 rResult = FmInitSVector3(0.0f);
            FmSVector3 pResult = FmInitSVector3(0.0f);

            if (!data.hasKinematicVerts || !data.kinematicFlags[row3])
            {
                uint rowStart = data.A.rowStarts[row3];
                uint rowEnd = data.A.rowStarts[row3 + 1];
                sResult = data.b[row3];
                rResult = sResult;

                FM_ASSERT(rowEnd >= rowStart);

                FmSVector3 ax = mul(data.A.submats[row3], taskData.solution[row3]);
                if (data.hasKinematicVerts && data.kinematicFlags[row3])
                {
                    sResult -= ax;
                }
                rResult -= ax;

                for (uint i = rowStart; i < rowEnd; i++)
                {
                    uint idx = data.A.indices[i];
                    ax = mul(data.A.submats[i], taskData.solution[idx]);
                    if (data.hasKinematicVerts && data.kinematicFlags[idx])
                    {
                        sResult -= ax;
                    }
                    rResult -= ax;
                }

                pResult = mul(data.PInvDiag[row3], rResult);
                bdelta += dot(sResult, mul(data.PInvDiag[row3], sResult));
                delta += dot(rResult, pResult);
            }

            temps.r[row3] = rResult;
            temps.p[row3] = pResult;
        }

        temps.bdeltaSums[inTaskBeginIndex] = bdelta;
        temps.deltaSums[inTaskBeginIndex] = delta;
    }

    static void FmTaskMpcgUpdateP(void* inTaskData, int32_t inTaskBeginIndex, int32_t)
    {
        FmMpcgParallelTaskData& taskData = *(FmMpcgParallelTaskData*)inTaskData;
        FmMpcgSolverDataTemps& temps = *taskData.temps;

        uint begin, end;
        FmGetMpcgTaskRange(&begin, &end, taskData, inTaskBeginIndex);

        for (uint row3 = begin; row3 < end; row3++)
        {
            if (taskData.solverData->hasKinematicVerts && taskData.solverData->kinematicFlags[row3])
            {
                temps.p[row3] = FmInitSVector3(0.0f);
            }
            else
            {
                temps.p[row3] = temps.h[row3] + taskData.deltaDivDeltaOld * temps.p[row3];
            }
        }
    }

    static void FmTaskMpcgMultiply(void* inTaskData, int32_t inTaskBeginIndex, int32_t)
    {
        FmMpcgParallelTaskData& taskData = *(FmMpcgParallelTaskData*)inTaskData;
        FmMpcgSolverData& data = *taskData.solverData;
        FmMpcgSolverDataTemps& temps = *taskData.temps;

        uint begin, end;
        FmGetMpcgTaskRange(&begin, &end, taskData, inTaskBeginIndex);

        float pdots = 0.0f;
        for (uint row3 = begin; row3 < end; row3++)
        {
            if (data.hasKinematicVerts && data.kinematicFlags[row3])
            {
                temps.s[row3] = FmInitSVector3(0.0f);
                continue;
            }

            FmSVector3 rowResult = mul(data.A.submats[row3], temps.p[row3]);
            for (uint i = data.A.rowStarts[row3]; i < data.A.rowStarts[row3 + 1]; i++)
            {
                rowResult += mul(data.A.submats[i], temps.p[data.A.indices[i]]);
            }

            pdots += dot(temps.p[row3], rowResult);
            temps.s[row3] = rowResult;
        }

        temps.deltaSums[inTaskBeginIndex] = pdots;
    }

    static void FmTaskMpcgUpdate(void* inTaskData, int32_t inTaskBeginIndex, int32_t)
    {
        FmMpcgParallelTaskData& taskData = *(FmMpcgParallelTaskData*)inTaskData;
        FmMpcgSolverData& data = *taskData.solverData;
        FmMpcgSolverDataTemps& temps = *taskData.temps;

        uint begin, end;
        FmGetMpcgTaskRange(&begin, &end, taskData, inTaskBeginIndex);

        float delta = 0.0f;
        for (uint row3 = begin; row3 < end; row3++)
        {
            FmSVector3 rNew = temps.r[row3] - taskData.alpha * temps.s[row3];
            FmSVector3 hNew = mul(data.PInvDiag[row3], rNew);

            delta += dot(rNew, hNew);

            taskData.solution[row3] += taskData.alpha * temps.p[row3];
            temps.r[row3] = rNew;
            temps.h[row3] = hNew;
        }

        temps.deltaSums[inTaskBeginIndex] = delta;
    }

    static float FmSumMpcgTaskValues(const float* values, uint numValues)
    {
        float sum = 0.0f;
        for (uint i = 0; i < numValues; i++)
        {
            sum += values[i];
        }
        return sum;
    }

    uint FmRunMpcgSolve(
        FmSVector3* solution,
        FmMpcgSolverData* solverData,
        FmMpcgSolverDataTemps* solverDataTemps,
        float epsilon,
        uint maxIterations,
        FmParallelForCallback parallelFor,
        uint numThreads)
    {
        FM_TRACE_SCOPED_EVENT(MPCG_SOLVE);

        FmMpcgParallelTaskData taskData;
        taskData.solverData = solverData;
        taskData.temps = solverDataTemps;
        taskData.solution = solution;
        taskData.numRows = solverData->A.numRows;
        taskData.numTasks = FmGetMpcgNumTasks(taskData.numRows, numThreads);

        FmRunMpcgParallelFor("MpcgSetup", FmTaskMpcgSetup, &taskData, parallelFor);
        float bdelta = FmSumMpcgTaskValues(solverDataTemps->bdeltaSums, taskData.numTasks);
        float delta = FmSumMpcgTaskValues(solverDataTemps->deltaSums, taskData.numTasks);

        if (bdelta <= 0.0f)
        {
            return 0;
        }

        uint numIterations = 0;
        float deltaOld = delta;
        float tolerance = epsilon * epsilon * bdelta;

        while (delta > tolerance && numIterations < maxIterations)
        {
            if (numIterations > 0)
            {
                taskData.deltaDivDeltaOld = delta / deltaOld;
                FmRunMpcgParallelFor("MpcgUpdateP", FmTaskMpcgUpdateP, &taskData, parallelFor);
            }

            FmRunMpcgParallelFor("MpcgMultiply", FmTaskMpcgMultiply, &taskData, parallelFor);
            float pdots = FmSumMpcgTaskValues(solverDataTemps->deltaSums, taskData.numTasks);
            taskData.alpha = delta / pdots;

            deltaOld = delta;
            FmRunMpcgParallelFor("MpcgUpdate", FmTaskMpcgUpdate, &taskData, parallelFor);
            delta = FmSumMpcgTaskValues(solverDataTemps->deltaSums, taskData.numTasks);

            numIterations++;
        }

        return numIterations;
    }
}
