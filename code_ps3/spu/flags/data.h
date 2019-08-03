#ifndef _DATA_H_
#define _DATA_H_

//--------------------------------------------------
//!
//!	\file data.h
//!	data processed by the SPU
//!
//--------------------------------------------------

#ifdef SN_TARGET_PS3_SPU
#include <ntlib_spu/vecmath_spu_ps3.h>
#else
#include "core/vecmath_ps3.h"
#endif // SN_TARGET_PS3_SPU

#include "core/bitmask.h"
#include "ntlib_spu/relativePointer.h"


namespace FlagBinding
{
	typedef enum
	{
		BREAKPOINT = BITFLAG(1),
	} Flags;

	struct PointDynamic
	{
		CVector m_position;
	}; // end of class Point

	struct PointStatic
	{
		float m_fInvMass;
	}; // end of class Point

	struct Global
	{
		CVector m_obGravity;
		float m_fTime;
		float m_fDeltaTime;
	}; // end of class Global
	
	struct Constraint
	{
		uint16_t m_index_1;
		uint16_t m_index_2;
		float m_fRestLength;
	}; // end of class Constraint
	
	struct TangeantPlaneIndices
	{
		int8_t m_iBinormal;
		int8_t m_iTangeant;
	};

	//struct SkinCoef
	//{
	//	static const uint16_t g_maxInfluence = 12;
	//	uint16_t m_index[g_maxInfluence];
	//	float m_weight[g_maxInfluence];
	//}; // end of class SkinCoef

	
	////////////////////////////
	struct VertexDynamic
	{
		float m_position[3];
		float m_normal[3];
		float m_tangent[3];
		float m_binormal[3];
	};
	// unused here
	struct VertexStatic
	{
		float m_normalTexcoord[2];
		float m_diffuseTexcoord[2];
	};
	////////////////////////////

	struct FlagIn
	{
		// globals (valid for all flags)
		Global m_global;

		CVector m_obGlobalWind;
		CVector m_obLocalTurbFreq;
		CVector m_obLocalTurbVel;

		float m_fLocalPower;
		float m_fDrag;
		float m_fInvMass;

		// nb step for constraint
		uint8_t m_uiNbStep;
		// flags
		BitMask<Flags> m_flags;
		// matrix
		CMatrix m_worldToObject;
		CMatrix m_oldToNewWorld;
		// nb mesh elements
		uint16_t m_nbMeshElements;
		RelativePointer<TangeantPlaneIndices> m_pTangeantPlaneIndices;
		// points in physics siumulation
		uint16_t m_nbPoints;
		RelativePointer<PointStatic> m_pGridStatic;
		// grid constraint
		uint16_t m_nbConstraints;
		RelativePointer<Constraint> m_pConstraints;

		///////////////////////////////////////////
		// external position
		// this array might come from different location, so dma might as well be done manually from SPU
		// points in physics siumulation
		uint16_t m_nbExternalPoints;
		RelativePointer<PointStatic> m_pExternalStatic;
		RelativePointer<PointDynamic> m_pExternalDynamic;
		// grid constraint (first index is external)
		uint16_t m_nbExternalConstraints;
		RelativePointer<Constraint> m_pExternalConstraints;

	}; // end of class FlagIn

	struct FlagOut
	{
		CPoint m_positionMin;
		CPoint m_positionMax;

		FlagOut() : m_positionMin( CONSTRUCT_CLEAR ), m_positionMax( CONSTRUCT_CLEAR ) {}
	}; // end of class FlagIn

	// not a binding struct, made by the SPU at load time
	struct FlagDynamicArray
	{
		PointDynamic* m_pGridDynamicCurrent;
		const PointDynamic* m_pGridDynamicBefore;
		const PointDynamic* m_pGridDynamicEvenBefore;
	}; // end of class FlagDynamicArray
} // end of namespace FlagBinding

#ifdef SN_TARGET_PS3_SPU
using namespace FlagBinding;
#endif // SN_TARGET_PS3_SPU


#endif // end of _DATA_H_
