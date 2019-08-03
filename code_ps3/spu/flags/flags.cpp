#include "flags.h"

//
//
// To oprimise !!! The most obvious thing
//    - unroll loop
//    - everything in CVector to avoid stupid copy. well, directly using intrinsics would be even better...
//    - array of structure
//


#include <ntlib_spu/debug_spu.h>
#include <util_spu.h>


template<class T>
inline T sign(T iVal)
{
	return iVal>=0?1:-1;
}

inline CPoint operator*(const CPoint& obVector, const CMatrix& obMatrix)
{
	v128	result;
	v128	lhsValue = obVector.QuadwordValue();
	result	= spu_mul(  obMatrix.GetRow0(), spu_splats(spu_extract(lhsValue,0)) );
	result	= spu_madd( obMatrix.GetRow1(), spu_splats(spu_extract(lhsValue,1)), result );
	result	= spu_madd( obMatrix.GetRow2(), spu_splats(spu_extract(lhsValue,2)), result );
	result	= spu_madd( obMatrix.GetRow3(), spu_splats(1.0f), result );

	return CPoint( result );
}

void Code::CopyPoints(const FlagIn& __restrict__ flagIn, PointDynamic* __restrict__ pCurrent, const PointDynamic* __restrict__ pBefore)
{
	// If time is too small, just copy the results
	for(uint16_t usFlatIndex = 0 ; usFlatIndex < flagIn.m_nbPoints ; ++usFlatIndex )
	{
		pCurrent[usFlatIndex].m_position = pBefore[usFlatIndex].m_position;
	}
}


void Code::ApplyForces(const FlagIn& __restrict__ flagIn,
		PointDynamic* __restrict__ pCurrent, const PointDynamic* __restrict__ pBefore, const PointDynamic* __restrict__ pEvenBefore, VertexDynamic* __restrict__ pMesh)
{
	// time
	float fTime = flagIn.m_global.m_fTime;
	float fDelta = flagIn.m_global.m_fDeltaTime;
	float fDeltaSquare = fDelta * fDelta;


	TangeantPlaneIndices* pTangeantPlaneIndices=flagIn.m_pTangeantPlaneIndices.Get();
	UNUSED( pTangeantPlaneIndices );
	CMatrix worldToObject = flagIn.m_worldToObject;
	
#if 0
	// parmam
	const PointStatic* pParam = flagIn.m_pGridStatic.Get();	// Kill this!
	
	// Generate a semi random wind direction
	float fTurnAround = 0.4f*fTime + 0.5*cos(1.0f*fTime+10.0f) + 0.2*cos(2.0f*fTime+12.0f);
	CVector windDirection(cos(fTurnAround),0.0f,sin(fTurnAround),0.0f);
	
	// Generate a regular gust
	float fpeek = wp.m_fGustPower * 0.5f *(cos(wp.m_fPeekFreq*fTime)-wp.m_fPeekTime)>0.0f?1.0f:0.0f;

	float fTemp1 = 1.0f + cos(wp.m_fMainFreq*fTime);
#endif

	float fDrag1 = 1.0f - flagIn.m_fDrag;
	float fDrag2 = 2.0f - flagIn.m_fDrag;


	// integrate force
	for(uint16_t usFlatIndex = 0 ; usFlatIndex < flagIn.m_nbPoints ; ++usFlatIndex )
	{
		//////////////////////////////////////////////////////////
		///// At this point, you might want to cull static point
		///// using a bitfield

		// wind computation
		CVector obNoise = pBefore[usFlatIndex].m_position * flagIn.m_obLocalTurbFreq;
		obNoise += flagIn.m_obLocalTurbVel * fTime;

		// Compute a local force
	//	float fVal = obNoise[0] + obNoise[1] + obNoise[2];

	//	CVector force(cos(fVal), 0.0f, sin(fVal), 0.0f);
		CVector force(cos(obNoise[0]), cos(obNoise[1]), cos(obNoise[2]), 0.0f);
		force *= flagIn.m_fLocalPower;

		// Add global force
		force += flagIn.m_obGlobalWind;
		force += flagIn.m_global.m_obGravity;

#if 0
		// get point on the grid to reconstruct normal
		// should be optimised by making the whole simulation in local space
		CPoint objPoint = (CPoint(pBefore[usFlatIndex].m_position));
		TangeantPlaneIndices tpIndices = pTangeantPlaneIndices[usFlatIndex];
		CPoint objPointBinormal =  (CPoint(pBefore[usFlatIndex + tpIndices.m_iBinormal].m_position));
		CPoint objPointTangent = (CPoint(pBefore[usFlatIndex + tpIndices.m_iTangeant].m_position));
		
		// create tangeant plane
		CDirection binormal = CDirection(objPointBinormal-objPoint);
	//	binormal *= sign(tpIndices.m_iBinormal);
		CDirection tangeant = CDirection(objPointTangent-objPoint);
	//	binormal *= sign(tpIndices.m_iTangeant);
		CDirection normal = binormal.Cross(tangeant);
		normal.Normalise();

		float fScale = (*(CVector*)&normal).Dot(force);
		force *= fScale;
#endif

		// Apply the forces and calculate
		float forceCoef = fDeltaSquare * flagIn.m_fInvMass;
		force *= forceCoef;

		// This is the verlet system implementation... 
		pCurrent[usFlatIndex].m_position = fDrag2 * pBefore[usFlatIndex].m_position
			- fDrag1 * pEvenBefore[usFlatIndex].m_position + force;
	}
}

void Code::SatisfyConstraint(const FlagIn& __restrict__ flagIn, PointDynamic* __restrict__ pCurrent)
{
	//handy
	const Constraint* pConstraints = flagIn.m_pConstraints.Get();
	const PointStatic* pParam = flagIn.m_pGridStatic.Get();
	
	// loop on internal contraint
	for(uint16_t constraintIndex = 0 ; constraintIndex < flagIn.m_nbConstraints ; ++constraintIndex )
	{
		// get constraint's indices
		Constraint currentConstraint = pConstraints[constraintIndex];
		uint16_t index_1 = currentConstraint.m_index_1;
		uint16_t index_2 = currentConstraint.m_index_2;
		// get the 2 poiints
		CVector v1 = pCurrent[index_1].m_position;
		CVector v2 = pCurrent[index_2].m_position;
		// diff and length
		CVector vDelta = v2-v1;
		float fDeltalength = vDelta.Length();
		
		// length-change coef
		float fCoef1 = pParam[index_1].m_fInvMass;
		float fCoef2 = pParam[index_2].m_fInvMass;
		float fCoefSum = fCoef1 + fCoef2;
		
		// fix-up
		float fLengthchange = ( fDeltalength - currentConstraint.m_fRestLength ) / fDeltalength;
		pCurrent[index_1].m_position += vDelta * (fLengthchange * fCoef1 / fCoefSum);
		pCurrent[index_2].m_position -= vDelta * (fLengthchange * fCoef2 / fCoefSum);

		pCurrent[index_1].m_position *= CVector(1.0f, 1.0f, 1.0f, 0.0f);
		pCurrent[index_2].m_position *= CVector(1.0f, 1.0f, 1.0f, 0.0f);


	}

	//handy
	const Constraint* pExternalConstraints = flagIn.m_pExternalConstraints.Get();
	const PointStatic* pExternalParam = flagIn.m_pExternalStatic.Get();
	const PointDynamic* pExternalCurrent = flagIn.m_pExternalDynamic.Get();

	// loop on external contraint
	for(uint16_t constraintIndex = 0 ; constraintIndex < flagIn.m_nbExternalConstraints ; ++constraintIndex )
	{
		// get constraint's indices
		Constraint currentConstraint = pExternalConstraints[constraintIndex];
		uint16_t index_1 = currentConstraint.m_index_1;
		uint16_t index_2 = currentConstraint.m_index_2;
		// get the 2 poiints
		CVector v1 = pExternalCurrent[index_1].m_position;
		CVector v2 = pCurrent[index_2].m_position;
		// diff and length
		CVector vDelta = v2-v1;
		float fDeltalength = vDelta.Length();
		
		// length-change coef
		float fCoef1 = pExternalParam[index_1].m_fInvMass;
		float fCoef2 = pParam[index_2].m_fInvMass;
		float fCoefSum = fCoef1 + fCoef2;
		
		// fix-up
		float fLengthchange = ( fDeltalength - currentConstraint.m_fRestLength ) / fDeltalength;
		pCurrent[index_1].m_position += vDelta * (fLengthchange * fCoef1 / fCoefSum);
		pCurrent[index_2].m_position -= vDelta * (fLengthchange * fCoef2 / fCoefSum);

		pCurrent[index_1].m_position *= CVector(1.0f, 1.0f, 1.0f, 0.0f);
		pCurrent[index_2].m_position *= CVector(1.0f, 1.0f, 1.0f, 0.0f);

	}
}



void Code::GenerateMesh(const FlagIn& __restrict__ flagIn, const PointDynamic* __restrict__ pCurrent,
		VertexDynamic* __restrict__ pMesh, FlagOut* __restrict__ pFlagOut)
{
	// tangeant plane indices
	TangeantPlaneIndices* pTangeantPlaneIndices=flagIn.m_pTangeantPlaneIndices.Get();

	// bounding box
	CMatrix worldToObject = flagIn.m_worldToObject;

	// init BB
	CPoint first = CPoint(pCurrent[0].m_position)*worldToObject;
	pFlagOut->m_positionMin = first;
	pFlagOut->m_positionMax = first;
	
	// reconstructing mesh with the assumption that the mesh is exactly the same than the dynamic points
	for(uint16_t uiVertexIndex = 0 ; uiVertexIndex < flagIn.m_nbMeshElements ; ++uiVertexIndex )
	{
		VertexDynamic& meshelem = pMesh[uiVertexIndex];
		
		// get point on the grid to reconstruct normal
		// should be optimised by making the whole simulation in local space
		CPoint objPoint = (CPoint(pCurrent[uiVertexIndex].m_position) * worldToObject);
		TangeantPlaneIndices tpIndices = pTangeantPlaneIndices[uiVertexIndex];
		//CPoint objPointBinormal = sign(tpIndices.m_iBinormal) * (CPoint(pCurrent[uiVertexIndex + tpIndices.m_iBinormal].m_position) * worldToObject);
		//CPoint objPointTangent = sign(tpIndices.m_iTangeant) * (CPoint(pCurrent[uiVertexIndex + tpIndices.m_iTangeant].m_position) * worldToObject);
		CPoint objPointBinormal =  (CPoint(pCurrent[uiVertexIndex + tpIndices.m_iBinormal].m_position) * worldToObject);
		CPoint objPointTangent = (CPoint(pCurrent[uiVertexIndex + tpIndices.m_iTangeant].m_position) * worldToObject);
		
		// create tangeant plane
		CDirection binormal = CDirection(objPointBinormal-objPoint);
		binormal *= sign(tpIndices.m_iBinormal);
		CDirection tangeant = CDirection(objPointTangent-objPoint);
		binormal *= sign(tpIndices.m_iTangeant);
		CDirection normal = binormal.Cross(tangeant);
		tangeant.Normalise();
		normal.Normalise();
		binormal = tangeant.Cross(normal);

		// update bounding box
		pFlagOut->m_positionMin = pFlagOut->m_positionMin.Min(objPoint);
		pFlagOut->m_positionMax = pFlagOut->m_positionMax.Max(objPoint);
		
		// position
		meshelem.m_position[0] = objPoint[0];
		meshelem.m_position[1] = objPoint[1];
		meshelem.m_position[2] = objPoint[2];
		// normal
		meshelem.m_normal[0] = normal[0];
		meshelem.m_normal[1] = normal[1];
		meshelem.m_normal[2] = normal[2];
		// tangent
		meshelem.m_tangent[0] = tangeant[0];
		meshelem.m_tangent[1] = tangeant[1];
		meshelem.m_tangent[2] = tangeant[2];
		// binormal
		meshelem.m_binormal[0] = binormal[0];
		meshelem.m_binormal[1] = binormal[1];
		meshelem.m_binormal[2] = binormal[2];
	}
}


