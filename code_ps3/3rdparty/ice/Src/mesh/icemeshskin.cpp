/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <stdlib.h>
#include <math.h>
#include "icemeshinternal.h"

using namespace Ice;
using namespace Ice::MeshProc;

#define MATRIX_SIZE (4*3)
#define MATRIX_BYTES (MATRIX_SIZE*sizeof(F32))

// Use this macro to convert a matrix byte offset to a matrix index,
// including masking out the "continue" bit at 0x4.
// In the C code, the matrix array is just a flat list of F32s, so
// the index of the Nth array is (N-1)*MATRIX_SIZE.
#define MATRIX_INDEX(byteOffset)      ( ((byteOffset) & ~0x4) * MATRIX_SIZE / MATRIX_BYTES )

#define MATRIX_NUMBER(byteOffset) (MATRIX_INDEX((byteOffset)) / MATRIX_SIZE)

#define X 0
#define Y 1
#define Z 2
#define W 3

#define POS0 (0*4)
#define POS1 (1*4)
#define POS2 (2*4)
#define POS3 (3*4)

// 4x3 matrix offsets 
// WARNING!! Indexes are Transposed -------------------
#define M00 (0*4+0)
#define M10 (0*4+1)
#define M20 (0*4+2)
#define M30 (0*4+3)
#define M01 (1*4+0)
#define M11 (1*4+1)
#define M21 (1*4+2)
#define M31 (1*4+3)
#define M02 (2*4+0)
#define M12 (2*4+1)
#define M22 (2*4+2)
#define M32 (2*4+3)

#define CONTROL_NEXT 0
#define CONTROL_DIFF1 2
#define CONTROL_SAME1 4
#define CONTROL_SAMEN 6
#define CONTROL_END 8
#define CONTROL_DIFFN 10

#ifndef __SPU__
static void Weight1Matrix(F32 *M0, F32 *M1, F32 *M2, F32 *M3, F32 *Wt)
{
	ICE_ASSERT(Wt[0] >= 0.0f && Wt[0] <= 1.0f);
	ICE_ASSERT(Wt[1] >= 0.0f && Wt[1] <= 1.0f);
	ICE_ASSERT(Wt[2] >= 0.0f && Wt[2] <= 1.0f);
	ICE_ASSERT(Wt[3] >= 0.0f && Wt[3] <= 1.0f);
	M1[M00] = M0[M00] * Wt[1];
	M1[M01] = M0[M01] * Wt[1];
	M1[M02] = M0[M02] * Wt[1];
	M1[M10] = M0[M10] * Wt[1];
	M1[M11] = M0[M11] * Wt[1];
	M1[M12] = M0[M12] * Wt[1];
	M1[M20] = M0[M20] * Wt[1];
	M1[M21] = M0[M21] * Wt[1];
	M1[M22] = M0[M22] * Wt[1];
	M1[M30] = M0[M30] * Wt[1];
	M1[M31] = M0[M31] * Wt[1];
	M1[M32] = M0[M32] * Wt[1];

	M2[M00] = M0[M00] * Wt[2];
	M2[M01] = M0[M01] * Wt[2];
	M2[M02] = M0[M02] * Wt[2];
	M2[M10] = M0[M10] * Wt[2];
	M2[M11] = M0[M11] * Wt[2];
	M2[M12] = M0[M12] * Wt[2];
	M2[M20] = M0[M20] * Wt[2];
	M2[M21] = M0[M21] * Wt[2];
	M2[M22] = M0[M22] * Wt[2];
	M2[M30] = M0[M30] * Wt[2];
	M2[M31] = M0[M31] * Wt[2];
	M2[M32] = M0[M32] * Wt[2];

	M3[M00] = M0[M00] * Wt[3];
	M3[M01] = M0[M01] * Wt[3];
	M3[M02] = M0[M02] * Wt[3];
	M3[M10] = M0[M10] * Wt[3];
	M3[M11] = M0[M11] * Wt[3];
	M3[M12] = M0[M12] * Wt[3];
	M3[M20] = M0[M20] * Wt[3];
	M3[M21] = M0[M21] * Wt[3];
	M3[M22] = M0[M22] * Wt[3];
	M3[M30] = M0[M30] * Wt[3];
	M3[M31] = M0[M31] * Wt[3];
	M3[M32] = M0[M32] * Wt[3];

	M0[M00] *= Wt[0];
	M0[M01] *= Wt[0];
	M0[M02] *= Wt[0];
	M0[M10] *= Wt[0];
	M0[M11] *= Wt[0];
	M0[M12] *= Wt[0];
	M0[M20] *= Wt[0];
	M0[M21] *= Wt[0];
	M0[M22] *= Wt[0];
	M0[M30] *= Wt[0];
	M0[M31] *= Wt[0];
	M0[M32] *= Wt[0];
}

static void Combine1Matrix(F32 *M0, F32 *M1, F32 *M2, F32 *M3, F32 *Wt, F32 *A0)
{
	ICE_ASSERT(Wt[0] >= 0.0f && Wt[0] <= 1.0f);
	ICE_ASSERT(Wt[1] >= 0.0f && Wt[1] <= 1.0f);
	ICE_ASSERT(Wt[2] >= 0.0f && Wt[2] <= 1.0f);
	ICE_ASSERT(Wt[3] >= 0.0f && Wt[3] <= 1.0f);
	M0[M00] += A0[M00] * Wt[0];
	M0[M01] += A0[M01] * Wt[0];
	M0[M02] += A0[M02] * Wt[0];
	M0[M10] += A0[M10] * Wt[0];
	M0[M11] += A0[M11] * Wt[0];
	M0[M12] += A0[M12] * Wt[0];
	M0[M20] += A0[M20] * Wt[0];
	M0[M21] += A0[M21] * Wt[0];
	M0[M22] += A0[M22] * Wt[0];
	M0[M30] += A0[M30] * Wt[0];
	M0[M31] += A0[M31] * Wt[0];
	M0[M32] += A0[M32] * Wt[0];

	M1[M00] += A0[M00] * Wt[1];
	M1[M01] += A0[M01] * Wt[1];
	M1[M02] += A0[M02] * Wt[1];
	M1[M10] += A0[M10] * Wt[1];
	M1[M11] += A0[M11] * Wt[1];
	M1[M12] += A0[M12] * Wt[1];
	M1[M20] += A0[M20] * Wt[1];
	M1[M21] += A0[M21] * Wt[1];
	M1[M22] += A0[M22] * Wt[1];
	M1[M30] += A0[M30] * Wt[1];
	M1[M31] += A0[M31] * Wt[1];
	M1[M32] += A0[M32] * Wt[1];

	M2[M00] += A0[M00] * Wt[2];
	M2[M01] += A0[M01] * Wt[2];
	M2[M02] += A0[M02] * Wt[2];
	M2[M10] += A0[M10] * Wt[2];
	M2[M11] += A0[M11] * Wt[2];
	M2[M12] += A0[M12] * Wt[2];
	M2[M20] += A0[M20] * Wt[2];
	M2[M21] += A0[M21] * Wt[2];
	M2[M22] += A0[M22] * Wt[2];
	M2[M30] += A0[M30] * Wt[2];
	M2[M31] += A0[M31] * Wt[2];
	M2[M32] += A0[M32] * Wt[2];

	M3[M00] += A0[M00] * Wt[3];
	M3[M01] += A0[M01] * Wt[3];
	M3[M02] += A0[M02] * Wt[3];
	M3[M10] += A0[M10] * Wt[3];
	M3[M11] += A0[M11] * Wt[3];
	M3[M12] += A0[M12] * Wt[3];
	M3[M20] += A0[M20] * Wt[3];
	M3[M21] += A0[M21] * Wt[3];
	M3[M22] += A0[M22] * Wt[3];
	M3[M30] += A0[M30] * Wt[3];
	M3[M31] += A0[M31] * Wt[3];
	M3[M32] += A0[M32] * Wt[3];
}

static void Weight4Matrices(F32 *M0, F32 *M1, F32 *M2, F32 *M3, F32 *Wt)
{
	ICE_ASSERT(Wt[0] >= 0.0f && Wt[0] <= 1.0f);
	ICE_ASSERT(Wt[1] >= 0.0f && Wt[1] <= 1.0f);
	ICE_ASSERT(Wt[2] >= 0.0f && Wt[2] <= 1.0f);
	ICE_ASSERT(Wt[3] >= 0.0f && Wt[3] <= 1.0f);
	M0[M00] *= Wt[0];
	M0[M01] *= Wt[0];
	M0[M02] *= Wt[0];
	M0[M10] *= Wt[0];
	M0[M11] *= Wt[0];
	M0[M12] *= Wt[0];
	M0[M20] *= Wt[0];
	M0[M21] *= Wt[0];
	M0[M22] *= Wt[0];
	M0[M30] *= Wt[0];
	M0[M31] *= Wt[0];
	M0[M32] *= Wt[0];

	M1[M00] *= Wt[1];
	M1[M01] *= Wt[1];
	M1[M02] *= Wt[1];
	M1[M10] *= Wt[1];
	M1[M11] *= Wt[1];
	M1[M12] *= Wt[1];
	M1[M20] *= Wt[1];
	M1[M21] *= Wt[1];
	M1[M22] *= Wt[1];
	M1[M30] *= Wt[1];
	M1[M31] *= Wt[1];
	M1[M32] *= Wt[1];

	M2[M00] *= Wt[2];
	M2[M01] *= Wt[2];
	M2[M02] *= Wt[2];
	M2[M10] *= Wt[2];
	M2[M11] *= Wt[2];
	M2[M12] *= Wt[2];
	M2[M20] *= Wt[2];
	M2[M21] *= Wt[2];
	M2[M22] *= Wt[2];
	M2[M30] *= Wt[2];
	M2[M31] *= Wt[2];
	M2[M32] *= Wt[2];

	M3[M00] *= Wt[3];
	M3[M01] *= Wt[3];
	M3[M02] *= Wt[3];
	M3[M10] *= Wt[3];
	M3[M11] *= Wt[3];
	M3[M12] *= Wt[3];
	M3[M20] *= Wt[3];
	M3[M21] *= Wt[3];
	M3[M22] *= Wt[3];
	M3[M30] *= Wt[3];
	M3[M31] *= Wt[3];
	M3[M32] *= Wt[3];
}

static void Combine4Matrices(F32 *M0, F32 *M1, F32 *M2, F32 *M3, F32 *Wt, F32 *A0, F32 *A1, F32 *A2, F32* A3)
{
	ICE_ASSERT(Wt[0] >= 0.0f && Wt[0] <= 1.0f);
	ICE_ASSERT(Wt[1] >= 0.0f && Wt[1] <= 1.0f);
	ICE_ASSERT(Wt[2] >= 0.0f && Wt[2] <= 1.0f);
	ICE_ASSERT(Wt[3] >= 0.0f && Wt[3] <= 1.0f);
	M0[M00] += A0[M00] * Wt[0];
	M0[M01] += A0[M01] * Wt[0];
	M0[M02] += A0[M02] * Wt[0];
	M0[M10] += A0[M10] * Wt[0];
	M0[M11] += A0[M11] * Wt[0];
	M0[M12] += A0[M12] * Wt[0];
	M0[M20] += A0[M20] * Wt[0];
	M0[M21] += A0[M21] * Wt[0];
	M0[M22] += A0[M22] * Wt[0];
	M0[M30] += A0[M30] * Wt[0];
	M0[M31] += A0[M31] * Wt[0];
	M0[M32] += A0[M32] * Wt[0];

	M1[M00] += A1[M00] * Wt[1];
	M1[M01] += A1[M01] * Wt[1];
	M1[M02] += A1[M02] * Wt[1];
	M1[M10] += A1[M10] * Wt[1];
	M1[M11] += A1[M11] * Wt[1];
	M1[M12] += A1[M12] * Wt[1];
	M1[M20] += A1[M20] * Wt[1];
	M1[M21] += A1[M21] * Wt[1];
	M1[M22] += A1[M22] * Wt[1];
	M1[M30] += A1[M30] * Wt[1];
	M1[M31] += A1[M31] * Wt[1];
	M1[M32] += A1[M32] * Wt[1];

	M2[M00] += A2[M00] * Wt[2];
	M2[M01] += A2[M01] * Wt[2];
	M2[M02] += A2[M02] * Wt[2];
	M2[M10] += A2[M10] * Wt[2];
	M2[M11] += A2[M11] * Wt[2];
	M2[M12] += A2[M12] * Wt[2];
	M2[M20] += A2[M20] * Wt[2];
	M2[M21] += A2[M21] * Wt[2];
	M2[M22] += A2[M22] * Wt[2];
	M2[M30] += A2[M30] * Wt[2];
	M2[M31] += A2[M31] * Wt[2];
	M2[M32] += A2[M32] * Wt[2];

	M3[M00] += A3[M00] * Wt[3];
	M3[M01] += A3[M01] * Wt[3];
	M3[M02] += A3[M02] * Wt[3];
	M3[M10] += A3[M10] * Wt[3];
	M3[M11] += A3[M11] * Wt[3];
	M3[M12] += A3[M12] * Wt[3];
	M3[M20] += A3[M20] * Wt[3];
	M3[M21] += A3[M21] * Wt[3];
	M3[M22] += A3[M22] * Wt[3];
	M3[M30] += A3[M30] * Wt[3];
	M3[M31] += A3[M31] * Wt[3];
	M3[M32] += A3[M32] * Wt[3];
}

static void CalculateCofactorMatrix(F32 *CM0, F32 *M0)
{
	CM0[M00] = M0[M11]*M0[M22] - M0[M12]*M0[M21];
	CM0[M01] = M0[M12]*M0[M20] - M0[M10]*M0[M22];
	CM0[M02] = M0[M10]*M0[M21] - M0[M11]*M0[M20];
	CM0[M10] = M0[M02]*M0[M21] - M0[M01]*M0[M22];
	CM0[M11] = M0[M00]*M0[M22] - M0[M02]*M0[M20];
	CM0[M12] = M0[M01]*M0[M20] - M0[M00]*M0[M21];
	CM0[M20] = M0[M01]*M0[M12] - M0[M02]*M0[M11];
	CM0[M21] = M0[M02]*M0[M10] - M0[M00]*M0[M12];
	CM0[M22] = M0[M00]*M0[M11] - M0[M01]*M0[M10];
}

static void CalculateCofactorMatrices(F32 *CM0, F32 *CM1, F32 *CM2, F32 *CM3, F32 *M0, F32 *M1, F32 *M2, F32 *M3)
{
	CalculateCofactorMatrix(CM0, M0);
	CalculateCofactorMatrix(CM1, M1);
	CalculateCofactorMatrix(CM2, M2);
	CalculateCofactorMatrix(CM3, M3);
}

static void ProcessSinglePosition(
	F32 *pOPos, 
	F32 *pPos, 
	F32 *M0, 
	F32 *M1, 
	F32 *M2, 
	F32 *M3)
{
	F32 V[3];

	V[X] = pPos[POS0+X]*M0[M00] + pPos[POS0+Y]*M0[M10] + pPos[POS0+Z]*M0[M20] + M0[M30];
	V[Y] = pPos[POS0+X]*M0[M01] + pPos[POS0+Y]*M0[M11] + pPos[POS0+Z]*M0[M21] + M0[M31];
	V[Z] = pPos[POS0+X]*M0[M02] + pPos[POS0+Y]*M0[M12] + pPos[POS0+Z]*M0[M22] + M0[M32];
	pOPos[POS0+X] = V[X];
	pOPos[POS0+Y] = V[Y];
	pOPos[POS0+Z] = V[Z];

	V[X] = pPos[POS1+X]*M1[M00] + pPos[POS1+Y]*M1[M10] + pPos[POS1+Z]*M1[M20] + M1[M30];
	V[Y] = pPos[POS1+X]*M1[M01] + pPos[POS1+Y]*M1[M11] + pPos[POS1+Z]*M1[M21] + M1[M31];
	V[Z] = pPos[POS1+X]*M1[M02] + pPos[POS1+Y]*M1[M12] + pPos[POS1+Z]*M1[M22] + M1[M32];
	pOPos[POS1+X] = V[X];
	pOPos[POS1+Y] = V[Y];
	pOPos[POS1+Z] = V[Z];

	V[X] = pPos[POS2+X]*M2[M00] + pPos[POS2+Y]*M2[M10] + pPos[POS2+Z]*M2[M20] + M2[M30];
	V[Y] = pPos[POS2+X]*M2[M01] + pPos[POS2+Y]*M2[M11] + pPos[POS2+Z]*M2[M21] + M2[M31];
	V[Z] = pPos[POS2+X]*M2[M02] + pPos[POS2+Y]*M2[M12] + pPos[POS2+Z]*M2[M22] + M2[M32];
	pOPos[POS2+X] = V[X];
	pOPos[POS2+Y] = V[Y];
	pOPos[POS2+Z] = V[Z];

	V[X] = pPos[POS3+X]*M3[M00] + pPos[POS3+Y]*M3[M10] + pPos[POS3+Z]*M3[M20] + M3[M30];
	V[Y] = pPos[POS3+X]*M3[M01] + pPos[POS3+Y]*M3[M11] + pPos[POS3+Z]*M3[M21] + M3[M31];
	V[Z] = pPos[POS3+X]*M3[M02] + pPos[POS3+Y]*M3[M12] + pPos[POS3+Z]*M3[M22] + M3[M32];
	pOPos[POS3+X] = V[X];
	pOPos[POS3+Y] = V[Y];
	pOPos[POS3+Z] = V[Z];
}

enum WCondition {kTrash=0, kPreserve=1};
static void ProcessSingleNormal(
	F32 *pOPos, 
	F32 *pPos, 
	F32 *M0, 
	F32 *M1, 
	F32 *M2, 
	F32 *M3,
	WCondition saveW)
{
	F32 V[3];
	F32 fInvLen;

	V[X] = pPos[POS0+X]*M0[M00] + pPos[POS0+Y]*M0[M10] + pPos[POS0+Z]*M0[M20];
	V[Y] = pPos[POS0+X]*M0[M01] + pPos[POS0+Y]*M0[M11] + pPos[POS0+Z]*M0[M21];
	V[Z] = pPos[POS0+X]*M0[M02] + pPos[POS0+Y]*M0[M12] + pPos[POS0+Z]*M0[M22];
	fInvLen = 1.f / sqrtf(V[X]*V[X] + V[Y]*V[Y] + V[Z]*V[Z]);
	pOPos[POS0+X] = V[X] * fInvLen;
	pOPos[POS0+Y] = V[Y] * fInvLen;
	pOPos[POS0+Z] = V[Z] * fInvLen;

	V[X] = pPos[POS1+X]*M1[M00] + pPos[POS1+Y]*M1[M10] + pPos[POS1+Z]*M1[M20];
	V[Y] = pPos[POS1+X]*M1[M01] + pPos[POS1+Y]*M1[M11] + pPos[POS1+Z]*M1[M21];
	V[Z] = pPos[POS1+X]*M1[M02] + pPos[POS1+Y]*M1[M12] + pPos[POS1+Z]*M1[M22];
	fInvLen = 1.f / sqrtf(V[X]*V[X] + V[Y]*V[Y] + V[Z]*V[Z]);
	pOPos[POS1+X] = V[X] * fInvLen;
	pOPos[POS1+Y] = V[Y] * fInvLen;
	pOPos[POS1+Z] = V[Z] * fInvLen;

	V[X] = pPos[POS2+X]*M2[M00] + pPos[POS2+Y]*M2[M10] + pPos[POS2+Z]*M2[M20];
	V[Y] = pPos[POS2+X]*M2[M01] + pPos[POS2+Y]*M2[M11] + pPos[POS2+Z]*M2[M21];
	V[Z] = pPos[POS2+X]*M2[M02] + pPos[POS2+Y]*M2[M12] + pPos[POS2+Z]*M2[M22];
	fInvLen = 1.f / sqrtf(V[X]*V[X] + V[Y]*V[Y] + V[Z]*V[Z]);
	pOPos[POS2+X] = V[X] * fInvLen;
	pOPos[POS2+Y] = V[Y] * fInvLen;
	pOPos[POS2+Z] = V[Z] * fInvLen;

	V[X] = pPos[POS3+X]*M3[M00] + pPos[POS3+Y]*M3[M10] + pPos[POS3+Z]*M3[M20];
	V[Y] = pPos[POS3+X]*M3[M01] + pPos[POS3+Y]*M3[M11] + pPos[POS3+Z]*M3[M21];
	V[Z] = pPos[POS3+X]*M3[M02] + pPos[POS3+Y]*M3[M12] + pPos[POS3+Z]*M3[M22];
	fInvLen = 1.f / sqrtf(V[X]*V[X] + V[Y]*V[Y] + V[Z]*V[Z]);
	pOPos[POS3+X] = V[X] * fInvLen;
	pOPos[POS3+Y] = V[Y] * fInvLen;
	pOPos[POS3+Z] = V[Z] * fInvLen;

	if(saveW == kPreserve)
	{
		pOPos[POS0+W] = pPos[POS0+W];
		pOPos[POS1+W] = pPos[POS1+W];
		pOPos[POS2+W] = pPos[POS2+W];
		pOPos[POS3+W] = pPos[POS3+W];
	}
}

void SkinPos(F32 *pMatrices, F32 *pWeights, U16 *pSame, U16 *pDiff, U8 *pControl, F32 *pPos)
{
	ICE_ASSERT(pPos != NULL);

	F32 *pOPos     = pPos;

	// Regular Matrices
	F32 M0[MATRIX_SIZE];
	F32 M1[MATRIX_SIZE];
	F32 M2[MATRIX_SIZE];
	F32 M3[MATRIX_SIZE];
	F32 A0[MATRIX_SIZE];
	F32 A1[MATRIX_SIZE];
	F32 A2[MATRIX_SIZE];
	F32 A3[MATRIX_SIZE];

	while(*pControl != CONTROL_END)
	{
		switch(*pControl)
		{
			// Do Nothing
		case CONTROL_NEXT:
			break;
			// Diff 1
		case CONTROL_DIFF1:
			ICE_ASSERT((pDiff[0] & 0x3) == 0);
			ICE_ASSERT((pDiff[1] & 0x3) == 0);
			ICE_ASSERT((pDiff[2] & 0x3) == 0);
			ICE_ASSERT((pDiff[3] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
			memcpy(M1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
			memcpy(M2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
			memcpy(M3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
			
			pDiff += 4;
			break;
			// Same 1
		case CONTROL_SAME1:
			ICE_ASSERT((pSame[0] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
			memcpy(M1, M0, MATRIX_BYTES);
			memcpy(M2, M0, MATRIX_BYTES);
			memcpy(M3, M0, MATRIX_BYTES);

			pSame += 1;
			break;
			// Same n
		case CONTROL_SAMEN:
			ICE_ASSERT((pSame[0] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
			Weight1Matrix(M0, M1, M2, M3, pWeights);
			
			while((*pSame) & 0x4)
			{
				pWeights += 4;
				pSame += 1;			

				ICE_ASSERT((pSame[0] & 0x3) == 0);
				memcpy(A0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
				Combine1Matrix(M0, M1, M2, M3, pWeights, A0);
			}
			pWeights += 4;
			pSame += 1;

			break;
			// Diff n
		case CONTROL_DIFFN:
			// Each group of four diff-N matrix indexes is stored in
			// an unusual 1,0,3,2 order!
			ICE_ASSERT((pDiff[0] & 0x3) == 0);
			ICE_ASSERT((pDiff[1] & 0x3) == 0);
			ICE_ASSERT((pDiff[2] & 0x3) == 0);
			ICE_ASSERT((pDiff[3] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
			memcpy(M1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
			memcpy(M2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
			memcpy(M3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
			Weight4Matrices(M0, M1, M2, M3, pWeights);

			while((*(pDiff+1)) & 0x4) // check pDiff+1, which is actually the "first" vertex in this group of 4.
			{
				pWeights += 4;
				pDiff += 4;

				ICE_ASSERT((pDiff[0] & 0x3) == 0);
				ICE_ASSERT((pDiff[1] & 0x3) == 0);
				ICE_ASSERT((pDiff[2] & 0x3) == 0);
				ICE_ASSERT((pDiff[3] & 0x3) == 0);
				memcpy(A0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
				memcpy(A1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
				memcpy(A2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
				memcpy(A3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
				Combine4Matrices(M0,M1,M2,M3,pWeights,A0,A1,A2,A3);
			}
			pWeights += 4;
			pDiff += 4;

			break;
		}

		ProcessSinglePosition(pOPos, pPos, M0, M1, M2, M3);
		pPos += 0x10;
		pOPos += 0x10;

		++pControl;
	}
}

void SkinPosNorm(F32 *pMatrices, F32 *pWeights, U16 *pSame, U16 *pDiff, U8 *pControl, F32 *pPos, F32 *pNormal)
{
	ICE_ASSERT(pPos != NULL);
	ICE_ASSERT(pNormal != NULL);

	F32 *pOPos     = pPos;
	F32 *pONormal  = pNormal;

	// Regular Matrices
	F32 M0[MATRIX_SIZE];
	F32 M1[MATRIX_SIZE];
	F32 M2[MATRIX_SIZE];
	F32 M3[MATRIX_SIZE];
	F32 A0[MATRIX_SIZE];
	F32 A1[MATRIX_SIZE];
	F32 A2[MATRIX_SIZE];
	F32 A3[MATRIX_SIZE];
	// Cofactor Matrices
	F32 CM0[MATRIX_SIZE];
	F32 CM1[MATRIX_SIZE];
	F32 CM2[MATRIX_SIZE];
	F32 CM3[MATRIX_SIZE];

	while(*pControl != CONTROL_END)
	{
		switch(*pControl)
		{
			// Do Nothing
		case CONTROL_NEXT:
			break;
			// Diff 1
		case CONTROL_DIFF1:
			ICE_ASSERT((pDiff[0] & 0x3) == 0);
			ICE_ASSERT((pDiff[1] & 0x3) == 0);
			ICE_ASSERT((pDiff[2] & 0x3) == 0);
			ICE_ASSERT((pDiff[3] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
			memcpy(M1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
			memcpy(M2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
			memcpy(M3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
			
			CalculateCofactorMatrices(CM0, CM1, CM2, CM3, M0, M1, M2, M3);

			pDiff += 4;
			break;
			// Same 1
		case CONTROL_SAME1:
			ICE_ASSERT((pSame[0] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
			memcpy(M1, M0, MATRIX_BYTES);
			memcpy(M2, M0, MATRIX_BYTES);
			memcpy(M3, M0, MATRIX_BYTES);
			
			CalculateCofactorMatrix(CM0, M0);
			memcpy(CM1, CM0, MATRIX_BYTES);
			memcpy(CM2, CM0, MATRIX_BYTES);
			memcpy(CM3, CM0, MATRIX_BYTES);

			pSame += 1;
			break;
			// Same n
		case CONTROL_SAMEN:
			ICE_ASSERT((pSame[0] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
			Weight1Matrix(M0, M1, M2, M3, pWeights);

			while((*pSame) & 0x4)
			{
				pWeights += 4;
				pSame += 1;

				ICE_ASSERT((pSame[0] & 0x3) == 0);
				memcpy(A0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
				Combine1Matrix(M0,M1,M2,M3,pWeights,A0);
			}
			pWeights += 4;
			pSame += 1;


			CalculateCofactorMatrices(CM0, CM1, CM2, CM3, M0, M1, M2, M3);
			break;
			// Diff n
		case CONTROL_DIFFN:
			// Each group of four diff-N matrix indexes is stored in
			// an unusual 1,0,3,2 order!
			ICE_ASSERT((pDiff[0] & 0x3) == 0);
			ICE_ASSERT((pDiff[1] & 0x3) == 0);
			ICE_ASSERT((pDiff[2] & 0x3) == 0);
			ICE_ASSERT((pDiff[3] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
			memcpy(M1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
			memcpy(M2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
			memcpy(M3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
			Weight4Matrices(M0,M1,M2,M3,pWeights);

			while((*(pDiff+1)) & 0x4) // check pDiff+1, which is actually the "first" vertex in this group of 4.
			{
				pWeights += 4;
				pDiff += 4;

				ICE_ASSERT((pDiff[0] & 0x3) == 0);
				ICE_ASSERT((pDiff[1] & 0x3) == 0);
				ICE_ASSERT((pDiff[2] & 0x3) == 0);
				ICE_ASSERT((pDiff[3] & 0x3) == 0);
				memcpy(A0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
				memcpy(A1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
				memcpy(A2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
				memcpy(A3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
				Combine4Matrices(M0,M1,M2,M3,pWeights,A0,A1,A2,A3);
			}
			pWeights += 4;
			pDiff += 4;

			CalculateCofactorMatrices(CM0, CM1, CM2, CM3, M0, M1, M2, M3);

			break;
		}

		ProcessSinglePosition(pOPos, pPos, M0, M1, M2, M3);
		pPos += 0x10;
		pOPos += 0x10;
		
		ProcessSingleNormal(pONormal, pNormal, CM0, CM1, CM2, CM3, kPreserve);
		pNormal += 0x10;
		pONormal += 0x10;

		++pControl;
	}
}

void SkinPosNormTan(F32 *pMatrices, F32 *pWeights, U16 *pSame, U16 *pDiff, U8 *pControl, F32 *pPos, F32 *pNormal, F32 *pTangent)
{
	ICE_ASSERT(pPos != NULL);
	ICE_ASSERT(pNormal != NULL);
	ICE_ASSERT(pTangent != NULL);

	F32 *pOPos     = pPos;
	F32 *pONormal  = pNormal;
	F32 *pOTangent = pTangent;

	// Regular Matrices
	F32 M0[MATRIX_SIZE];
	F32 M1[MATRIX_SIZE];
	F32 M2[MATRIX_SIZE];
	F32 M3[MATRIX_SIZE];
	F32 A0[MATRIX_SIZE];
	F32 A1[MATRIX_SIZE];
	F32 A2[MATRIX_SIZE];
	F32 A3[MATRIX_SIZE];
	// Cofactor Matrices
	F32 CM0[MATRIX_SIZE];
	F32 CM1[MATRIX_SIZE];
	F32 CM2[MATRIX_SIZE];
	F32 CM3[MATRIX_SIZE];

	while(*pControl != CONTROL_END)
	{
		switch(*pControl)
		{
			// Do Nothing
		case CONTROL_NEXT:
			break;
			// Diff 1
		case CONTROL_DIFF1:
			ICE_ASSERT((pDiff[0] & 0x3) == 0);
			ICE_ASSERT((pDiff[1] & 0x3) == 0);
			ICE_ASSERT((pDiff[2] & 0x3) == 0);
			ICE_ASSERT((pDiff[3] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
			memcpy(M1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
			memcpy(M2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
			memcpy(M3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);

			CalculateCofactorMatrices(CM0, CM1, CM2, CM3, M0, M1, M2, M3);

			pDiff += 4;
			break;
			// Same 1
		case CONTROL_SAME1:
			ICE_ASSERT((pSame[0] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
			memcpy(M1, M0, MATRIX_BYTES);
			memcpy(M2, M0, MATRIX_BYTES);
			memcpy(M3, M0, MATRIX_BYTES);
			
			CalculateCofactorMatrix(CM0,M0);
			memcpy(CM1, CM0, MATRIX_BYTES);
			memcpy(CM2, CM0, MATRIX_BYTES);
			memcpy(CM3, CM0, MATRIX_BYTES);

			pSame += 1;
			break;
			// Same n
		case CONTROL_SAMEN:
			ICE_ASSERT((pSame[0] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
			Weight1Matrix(M0, M1, M2, M3, pWeights);

			while((*pSame) & 0x4)
			{
				pWeights += 4;
				pSame += 1;

				ICE_ASSERT((pSame[0] & 0x3) == 0);
				memcpy(A0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
				Combine1Matrix(M0,M1,M2,M3,pWeights,A0);
			}
			pWeights += 4;
			pSame += 1;


			CalculateCofactorMatrices(CM0, CM1, CM2, CM3, M0, M1, M2, M3);

			break;
			// Diff n
		case CONTROL_DIFFN:
			// Each group of four diff-N matrix indexes is stored in
			// an unusual 1,0,3,2 order!
			ICE_ASSERT((pDiff[0] & 0x3) == 0);
			ICE_ASSERT((pDiff[1] & 0x3) == 0);
			ICE_ASSERT((pDiff[2] & 0x3) == 0);
			ICE_ASSERT((pDiff[3] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
			memcpy(M1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
			memcpy(M2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
			memcpy(M3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
			Weight4Matrices(M0,M1,M2,M3,pWeights);

			while((*(pDiff+1)) & 0x4) // check pDiff+1, which is actually the "first" vertex in this group of 4.
			{
				pWeights += 4;
				pDiff += 4;

				ICE_ASSERT((pDiff[0] & 0x3) == 0);
				ICE_ASSERT((pDiff[1] & 0x3) == 0);
				ICE_ASSERT((pDiff[2] & 0x3) == 0);
				ICE_ASSERT((pDiff[3] & 0x3) == 0);
				memcpy(A0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
				memcpy(A1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
				memcpy(A2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
				memcpy(A3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
				Combine4Matrices(M0,M1,M2,M3,pWeights,A0,A1,A2,A3);
			}
			pWeights += 4;
			pDiff += 4;

			CalculateCofactorMatrices(CM0, CM1, CM2, CM3, M0, M1, M2, M3);

			break;
		}

		ProcessSinglePosition(pOPos, pPos, M0, M1, M2, M3);
		pPos += 0x10;
		pOPos += 0x10;
		
		ProcessSingleNormal(pONormal, pNormal, CM0, CM1, CM2, CM3, kPreserve);
		pNormal += 0x10;
		pONormal += 0x10;
		
		ProcessSingleNormal(pOTangent, pTangent, M0, M1, M2, M3, kPreserve);
		pTangent += 0x10;
		pOTangent += 0x10;

		++pControl;
	}
}

void SkinPosNormTanDisp(F32 *pMatrices, F32 *pWeights, U16 *pSame, U16 *pDiff, U8 *pControl, F32 *pPos, F32 *pNormal, F32 *pTangent, F32 *pDisp)
{
	ICE_ASSERT(pPos != NULL);
	ICE_ASSERT(pNormal != NULL);
	ICE_ASSERT(pTangent != NULL);
	ICE_ASSERT(pDisp != NULL);

	F32 *pOPos     = pPos;
	F32 *pONormal  = pNormal;
	F32 *pOTangent = pTangent;
	F32 *pODisp    = pDisp;

	// Regular Matrices
	F32 M0[MATRIX_SIZE];
	F32 M1[MATRIX_SIZE];
	F32 M2[MATRIX_SIZE];
	F32 M3[MATRIX_SIZE];
	F32 A0[MATRIX_SIZE];
	F32 A1[MATRIX_SIZE];
	F32 A2[MATRIX_SIZE];
	F32 A3[MATRIX_SIZE];
	// Cofactor Matrices
	F32 CM0[MATRIX_SIZE];
	F32 CM1[MATRIX_SIZE];
	F32 CM2[MATRIX_SIZE];
	F32 CM3[MATRIX_SIZE];

	while(*pControl != CONTROL_END)
	{
		switch(*pControl)
		{
			// Do Nothing
		case CONTROL_NEXT:
			break;
			// Diff 1
		case CONTROL_DIFF1:
			ICE_ASSERT((pDiff[0] & 0x3) == 0);
			ICE_ASSERT((pDiff[1] & 0x3) == 0);
			ICE_ASSERT((pDiff[2] & 0x3) == 0);
			ICE_ASSERT((pDiff[3] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
			memcpy(M1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
			memcpy(M2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
			memcpy(M3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
			
			CalculateCofactorMatrices(CM0, CM1, CM2, CM3, M0, M1, M2, M3);

			pDiff += 4;
			break;
			// Same 1
		case CONTROL_SAME1:
			ICE_ASSERT((pSame[0] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
			memcpy(M1, M0, MATRIX_BYTES);
			memcpy(M2, M0, MATRIX_BYTES);
			memcpy(M3, M0, MATRIX_BYTES);
			
			CalculateCofactorMatrix(CM0, M0);
			memcpy(CM1, CM0, MATRIX_BYTES);
			memcpy(CM2, CM0, MATRIX_BYTES);
			memcpy(CM3, CM0, MATRIX_BYTES);

			pSame += 1;
			break;
			// Same n
		case CONTROL_SAMEN:
			ICE_ASSERT((pSame[0] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
			Weight1Matrix(M0,M1,M2,M3,pWeights);

			while((*pSame) & 0x4)
			{
				pWeights += 4;
				pSame += 1;

				ICE_ASSERT((pSame[0] & 0x3) == 0);
				memcpy(A0, &pMatrices[MATRIX_INDEX(pSame[0])], MATRIX_BYTES);
				Combine1Matrix(M0,M1,M2,M3,pWeights,A0);
			}
			pWeights += 4;
			pSame += 1;


			CalculateCofactorMatrices(CM0, CM1, CM2, CM3, M0, M1, M2, M3);

			break;
			// Diff n
		case CONTROL_DIFFN:
			// Each group of four diff-N matrix indexes is stored in
			// an unusual 1,0,3,2 order!
			ICE_ASSERT((pDiff[0] & 0x3) == 0);
			ICE_ASSERT((pDiff[1] & 0x3) == 0);
			ICE_ASSERT((pDiff[2] & 0x3) == 0);
			ICE_ASSERT((pDiff[3] & 0x3) == 0);
			memcpy(M0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
			memcpy(M1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
			memcpy(M2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
			memcpy(M3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
			Weight4Matrices(M0,M1,M2,M3,pWeights);

			while((*(pDiff+1)) & 0x4) // check pDiff+1, which is actually the "first" vertex in this group of 4.
			{
				pWeights += 4;
				pDiff += 4;

				ICE_ASSERT((pDiff[0] & 0x3) == 0);
				ICE_ASSERT((pDiff[1] & 0x3) == 0);
				ICE_ASSERT((pDiff[2] & 0x3) == 0);
				ICE_ASSERT((pDiff[3] & 0x3) == 0);
				memcpy(A0, &pMatrices[MATRIX_INDEX(pDiff[1])], MATRIX_BYTES);
				memcpy(A1, &pMatrices[MATRIX_INDEX(pDiff[0])], MATRIX_BYTES);
				memcpy(A2, &pMatrices[MATRIX_INDEX(pDiff[3])], MATRIX_BYTES);
				memcpy(A3, &pMatrices[MATRIX_INDEX(pDiff[2])], MATRIX_BYTES);
				Combine4Matrices(M0,M1,M2,M3,pWeights,A0,A1,A2,A3);
			}
			pWeights += 4;
			pDiff += 4;

			CalculateCofactorMatrices(CM0, CM1, CM2, CM3, M0, M1, M2, M3);

			break;
		}

		ProcessSinglePosition(pOPos, pPos, M0, M1, M2, M3);
		pPos += 0x10;
		pOPos += 0x10;
		
		ProcessSingleNormal(pONormal, pNormal, CM0, CM1, CM2, CM3, kPreserve);
		pNormal += 0x10;
		pONormal += 0x10;
		
		ProcessSingleNormal(pOTangent, pTangent, M0, M1, M2, M3, kPreserve);
		pTangent += 0x10;
		pOTangent += 0x10;
		
		ProcessSingleNormal(pODisp, pDisp, CM0, CM1, CM2, CM3, kTrash);
		pDisp += 0x10;
		pODisp += 0x10;

		++pControl;
	}
}
#endif // __SPU__

#ifndef __SPU__
void CmdSkinObject(U32 *pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);
	F32 *pMatrices = (F32 *)pStaticMem[kStaticMatrixPtr];
	F32 *pWeights  = (F32 *)pStaticMem[kStaticSkinWeightPtr];
	U16 *pSame     = (U16 *)pStaticMem[kStaticSkinSamePtr];
	U16 *pDiff     = (U16 *)pStaticMem[kStaticSkinDiffPtr];
	U8  *pControl  =  (U8 *)pStaticMem[kStaticSkinControlPtr];
	ICE_ASSERT(pMatrices != NULL);
	ICE_ASSERT(pControl != NULL);

	// Here, miscFlags indicates which attributes to skin.
	// Not all combinations of bits are supported.
	//
	// bit 0: Position
	// bit 1: Normal
	// bit 2: Tangent
	// bit 3: Displacement Normal
	U8 miscFlags        = (cmdQuad1.m_data.u8.B & 0x0F);
	switch(miscFlags)
	{
	case 0x0:
		// No skinning...not technically an error, but why issue the
		// command if you're not going to do anything?
		break;
	case 0x1:
		SkinPos(pMatrices, pWeights, pSame, pDiff, pControl,
			(F32 *)pStaticMem[kStaticUniformPosPtr]);
		break;
	case 0x3:
		SkinPosNorm(
			pMatrices, pWeights, pSame, pDiff, pControl,
			(F32 *)pStaticMem[kStaticUniformPosPtr],
			(F32 *)pStaticMem[kStaticUniformNormPtr]);
		break;
	case 0x7:
		SkinPosNormTan(
			pMatrices, pWeights, pSame, pDiff, pControl,
			(F32 *)pStaticMem[kStaticUniformPosPtr],
			(F32 *)pStaticMem[kStaticUniformNormPtr],
			(F32 *)pStaticMem[kStaticUniformTanPtr]);
		break;
	case 0x9:
		SkinPosNorm(
			pMatrices, pWeights, pSame, pDiff, pControl,
			(F32 *)pStaticMem[kStaticUniformPosPtr],
			(F32 *)pStaticMem[kStaticUniformDnormPtr]); // pass in displacement normals instead of normals.
		break;
	case 0xF:
		SkinPosNormTanDisp(
			pMatrices, pWeights, pSame, pDiff, pControl,
			(F32 *)pStaticMem[kStaticUniformPosPtr],
			(F32 *)pStaticMem[kStaticUniformNormPtr],
			(F32 *)pStaticMem[kStaticUniformTanPtr],
			(F32 *)pStaticMem[kStaticUniformDnormPtr]);
		break;
	default:
		ICE_ASSERT(0);// Invalid skinning flags in miscFlags
		break;
	}

	// If DM was performed prior to skinning, then we need to free memory all the way back to before the
	// DM information, otherwise we just clear memory back to before the skinning information.
	if (pStaticMem[kStaticDmInfoPtr] != 0)
		MeshMemFree(pStaticMem, pStaticMem[kStaticDiscretePmInfoPtr]);
	else
		MeshMemFree(pStaticMem, pStaticMem[kStaticSkinControlPtr]);
}
#endif // __SPU__

