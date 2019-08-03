#ifndef _HAIRDATA_PS3_H_
#define _HAIRDATA_PS3_H_

//--------------------------------------------------
//!
//!	\file hairdata_ps3.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifdef PLATFORM_PS3
#if defined(SN_TARGET_PS3_SPU)
	#include <ntlib_spu/basetypes_spu.h>
	#include <ntlib_spu/debug_spu.h>
	#include <ntlib_spu/vecmath_spu_ps3.h>
	#include <ntlib_spu/relativePointer.h>
	#include <ntlib_spu/fixmePointer.h>
#endif
#endif // PLATFORM_PS3

#include "core/rotationnalindex.h"
#include "core/bitmask.h"
#include "gfx/simplefunction.h"
// FIXME
#include <../code_ps3/ntlib_spu/relativePointer.h>


struct TmpVec2
{
// data
	float m_val[2];
// function
	inline TmpVec2(){m_val[0]=0.0f;m_val[1]=0.0f;}
	inline TmpVec2(float x, float y){m_val[0]=x;m_val[1]=y;}
	inline TmpVec2(const float* pVal){m_val[0]=pVal[0];m_val[1]=pVal[1];}
	inline float& operator[] (int i) {return m_val[i];}
	inline float operator[] (int i) const {return m_val[i];}
	inline TmpVec2 operator* (float f) const {return TmpVec2(m_val[0]*f,m_val[1]*f);}
	inline void operator*= (float f) {m_val[0]*=f;m_val[1]*=f;}
	inline TmpVec2 operator- (const TmpVec2& v) const {return TmpVec2(m_val[0]-v[0],m_val[1]-v[1]);}
	inline TmpVec2 operator+ (const TmpVec2& v) const {return TmpVec2(m_val[0]+v[0],m_val[1]+v[1]);}
	inline void operator-= (const TmpVec2& v) {m_val[0]-=v[0];m_val[1]-=v[1];}
}; // end of class TmpVec2



//#define FRANKHAIRPRINT

#ifdef FRANKHAIRPRINT
	#define FRANKHAIRPRINT_INT(s,i) ntPrintf("FRANKHAIR %s=%i\n",s,i);
	#define FRANKHAIRPRINT_MATRIX(s,m) ntPrintf("FRANKHAIR %s=(\n)",s);\
		FRANKHAIRPRINT_FLOAT4("",CVector(m[0])); \
		FRANKHAIRPRINT_FLOAT4("",CVector(m[1])); \
		FRANKHAIRPRINT_FLOAT4("",CVector(m[2])); \
		FRANKHAIRPRINT_FLOAT4("",CVector(m[3]));
	#define FRANKHAIRPRINT_FLOAT4(s,force) ntPrintf("FRANKHAIR %s=(%f,%f,%f,%f)\n",s,force[0],force[1],force[2],force[3]);
	#define FRANKHAIRPRINT_FLOAT3(s,force) ntPrintf("FRANKHAIR %s=(%f,%f,%f)\n",s,force[0],force[1],force[2]);
	#define FRANKHAIRPRINT_FLOAT2(s,force) ntPrintf("FRANKHAIR %s=(%f,%f)\n",s,force[0],force[1]);
	#define FRANKHAIRPRINT_FLOAT(s,f) ntPrintf("FRANKHAIR %s=%f\n",s,f);
	//#define FRANKHAIRPRINT_KILL(b) ntError_p(b, ("frankhair ending of the world"));
#define FRANKHAIRPRINT_KILL(b) if (!(b)) { ntPrintf("frankhair ending of the world"); ntBreakpoint(); } else {}
#else // NO FRANKHAIRPRINT
	#define FRANKHAIRPRINT_INT(s,i)
	#define FRANKHAIRPRINT_MATRIX(s,force)
	#define FRANKHAIRPRINT_FLOAT4(s,force)
	#define FRANKHAIRPRINT_FLOAT3(s,force)
	#define FRANKHAIRPRINT_FLOAT2(s,force)
	#define FRANKHAIRPRINT_FLOAT(s,f)
	#define FRANKHAIRPRINT_KILL(b)
#endif // FRANKHAIRPRINT



namespace ChainSPU
{








////////////////////////////////
////////////////////////////////
// spring
struct Spring
{
	uint16_t m_chainElemIndices[2];
	float m_fLength;
	float m_fSpringStiffness;
}; // end of class MetaBall
////////////////////////////////
////////////////////////////////







////////////////////////////////
////////////////////////////////
// one elem
struct ChainElemDef
{
	// forces
	float m_fFluid;
	float m_fWeight;
	float m_fStiffDotTreshold;
	float m_fStiff;

	// equation
	float m_fLength;
	float m_fInertiaMoment;
	float m_fDamp;
	
	// distance to root
	float m_fDistanceToRoot;
	
	// collision
	float m_fRadius;
	
	// parent to local transform
	CMatrix m_local;

	// hack to find back the right joint orent axis
	// the size could really be optimised...
	CMatrix m_extraRotInv;
	CMatrix m_extraRot;

	typedef enum
	{
		ISFROZEN = BITFLAG(0),
	} Flags;
	BitMask<Flags,uint16_t> m_flags;	
}; // end of class ChainElemParam


struct CollisionForce
{
public:
	// cumulation of all the world difference
	CVector m_worldNorm;

	// out coeficient
	float m_fOutCoef;
public:
	// inline
	void SetNotInfluenced()
	{
		m_worldNorm = CVector(CONSTRUCT_CLEAR);
		m_fOutCoef = -1.0f;
	}
	// return true if under the influence of the field
	inline bool IsUnderInfluence() const
	{
		return m_fOutCoef>=0;
	}	
}; // end of class CorrectedPosition

struct CollisionDisplacement
{
public:
	// cumulation of all the world difference
	CVector m_worldDiff;
	
	// out coeficient
	float m_fPotential;

	// out coeficient
	float m_fRefLengthSquare;

public:
	// inline
	void SetNotColliding()
	{
		m_fPotential=-1.0f;
	}
	// true if colliding
	inline bool IsColliding() const
	{
		return m_fPotential>=1.0f;
	}	
}; // end of class CollisionDisplacement



//--------------------------------------------------
//!
//!	temporary data for per-joint.
//! only one instance of this struct is present in memory
//!
//--------------------------------------------------
struct CollisionCache
{
	struct SphereCache
	{
	public:
		// normal pointing to the exterior of the collision ellipsoid
		CVector m_worldNorm;

		// potential, defining a layer around the ellipsoid
		float m_fPotential;
				
		// negative means that the joint is in the sphere
		float m_fRefLengthSquare;
	public:
		void Set(const CVector& worldNorm, float fPotential, float fRefLengthSquare) 
		{
			m_fPotential=fPotential;
			m_worldNorm=worldNorm;
			m_fRefLengthSquare=fRefLengthSquare;
		}
	}; // end of class SphereCache
	

	// maximum number of collision
	static const uint16_t g_usCacheSize = 100;

	// maximum number of collision
	uint16_t m_usNbValidSphere;

	// array sphere;
	SphereCache m_sphereArray[g_usCacheSize];

}; // end of class CollisionDisplacement

struct ChainElemStatic
{
	// def
	ChainElemDef m_def;

	// self index
	uint16_t m_usSelfIndex;
	// parent index
	uint16_t m_usParentIndex;
	// parent index
	uint16_t m_usTransformIndex;

	// spring index
	uint16_t m_usNbSpring;
	uint16_t m_usFirstSpring;
	
	// anti-collision index
	uint16_t m_usAntiColIndex;
}; // end of class ChainElem

struct ChainElemDynamic
{
	// last rotation matrix
	CMatrix m_lastRotationAxis;	

	// collision result (must be kept from one frame to the other...)
	CollisionForce m_collisionForce;

	// modified acceleration
	CVector m_modifiedAcceleration;
}; // end of class ChainElem


struct ChainElemTmp
{
	// parent to local transform
	CMatrix m_local;

	// decomposed parent to local transform.
	struct _LocalDecomposed
	{
		CQuat	m_Rotation;
		CPoint	m_Translation;
	}
	m_LocalJoint;

	// from local repere to world repere
	CMatrix m_local2world;

	// x approximation
	CDirection m_xAxisApprox;
}; // end of class ChainElem

struct SentinelInfo
{
	CMatrix m_local2world;
	CVector m_current;
	CVector m_before;
	CDirection m_parentDiff;
}; // end of class ChainElem

// end of one elem
////////////////////////////////
////////////////////////////////















////////////////////////////////
////////////////////////////////
// one chain
struct ChainGroupDynamicSnapshot
{
	// current
	RelativePointer<CVector> m_pPositions;
	RelativePointer<TmpVec2> m_angles;
}; // end of class ChainElem


// dynamic buffer typedefs
static const uint16_t g_usNbBuffer = 3;
typedef RotationnalIndex<uint16_t,g_usNbBuffer> RotIndex;
typedef enum
{
	CURRENT = g_usNbBuffer-1,
	BEFORE = g_usNbBuffer-2,
	EVENBEFORE = g_usNbBuffer-3,
} TimeIndex;


struct ChainDef
{
	// nb step per frame
	uint16_t m_usNbStepPerFrame;

	// mask
	typedef enum
	{
		FLUID = BITFLAG(0),
		ACCELERATION = BITFLAG(1),
		STIFF = BITFLAG(2),
		GRAVITY = BITFLAG(3),
		POSE = BITFLAG(4),
		COLLISIONFORCE = BITFLAG(5),
		COLLISIONDISPLACEMENT = BITFLAG(6),
		ANYCOLLISION = BITFLAG(7),
		DAMP = BITFLAG(8),
		MEGADAMP = BITFLAG(9),
		CLIPANGLE = BITFLAG(10),
		SPRING = BITFLAG(11),
		ANTICOLLISIONDISPLACEMENT = BITFLAG(12),
		COLLISION_HEURISTIC = BITFLAG(13),
		WIND = BITFLAG(14),
		WIND_E3HACK = BITFLAG(15),
	} Flags;
	BitMask<Flags,uint16_t> m_flags;
	
	// force construction
	float m_fAccelerationPropagation;
	float m_fUnconstrainedCoef;
	float m_fSpeedCorrection;
	float m_fPositionCorrection;

	// simulation equation
	float m_fMegaDamp;
	float m_fClipAngle;

	// collision detection
	float m_fPotentialCoef;
	float m_fPotentialEnd;
	
	// anticollision
	float m_fClothCollision;
	float m_fAntiCollisionResize;

	// range
	TmpVec2 m_collisionForceRange;
	float m_fPotentialExpulsion;
	float m_fCollisionCorrectionResize;
	
	// acceleration hack
	float m_fAccelerationEcho;
}; // end of class ChainDef





struct AntiCollisionSphere
{
	CMatrix m_world2Sphere;
	CMatrix m_sphere2Word;
}; // end of class AntiCollisionSphere


struct OneChain
{
	//uint32_t m_startFlag;

	// number of element
	uint16_t m_usNbChainElem;

	// the first element are sentinel
	uint16_t m_usFirstDynamicElem;
	
	// dynamic infomrmation (read/write)
	RelativePointer<ChainElemDynamic> m_pChainElemDynamics;
	
	// static information (read only)
	RelativePointer<ChainElemStatic> m_pChainElemStatics;
	
	// static information (read only)
	RelativePointer<SentinelInfo> m_pSentinelInfos;

	// first transform
	uint32_t m_uiFirstLocalJoint;
	uint32_t m_uiLocalJointStride;
	
	// main def
	ChainDef m_def;

	// wind
	CVector m_wind;

	// springs;
	uint16_t m_usNbSpring;
	RelativePointer<Spring> m_pSprings;
	RelativePointer<uint16_t> m_pSpringIndices;

	// anti-collision sphere;
	uint16_t m_usNbAntiCollisionSphere;
	RelativePointer<AntiCollisionSphere> m_pAntiCollisionSphere;
		
	// dynamic
	RotIndex m_rotationnalIndex;
	mutable ChainGroupDynamicSnapshot m_dynamic[g_usNbBuffer];
	
	// renormalisation of the angle
	uint16_t m_usNumRenormalised;

	// mask
	typedef enum
	{
		RESET_NEEDED = BITFLAG(0),
	} Flags;
	mutable BitMask<Flags,uint16_t> m_flags;

	//uint32_t m_stopFlag;
}; // end of class ChainElem


// end of one chain
////////////////////////////////
////////////////////////////////












////////////////////////////////
////////////////////////////////
// collision
struct MetaBallDef
{
	float m_fAverageRadius;
	CVector m_scale;
}; // end of class MetaBall
struct MetaBall
{
	MetaBallDef m_def;
	CMatrix m_world2Sphere;
}; // end of class MetaBall

struct MetaBallGroup
{
	uint16_t m_usNbMetaBall;
	RelativePointer<MetaBall> m_pMetaBalls;
}; // end of class MetaBall

////////////////////////////////
////////////////////////////////










struct E3HackDef
{
	CVector m_dummy1;
};


//struct ChainAndCollision
//{
//	FixmePointer<MetaBallGroup> m_pMetaBallGroup;
//	FixmePointer<ChainGroup> m_pChainGroup;
//}; // end of class OneChain

struct Global
{
	// gravity
	CVector m_gravity;
	
	// stuff for e3 hack
	E3HackDef m_e3Hack;

	// frame delta
	float m_fDelta;

	// absolute time
	float m_fTime;

	// mask
	typedef enum
	{
		DEBUG_BEGINEND = BITFLAG(0),
		DEBUG_BREAKPOINT = BITFLAG(1),
		DEBUG_PERFORMANCE = BITFLAG(2),
	} Flags;
	BitMask<Flags,uint16_t> m_flags;
}; // end of class MetaBall

} // end of namespace ChainSPU












namespace ChainSPU
{

// internal use
struct Dynamic
{
public:
	CVector* m_positions[g_usNbBuffer];
	TmpVec2* m_angles[g_usNbBuffer];
public:
	Dynamic(const OneChain* oneChain)
	{
		for(uint16_t usBuffer = 0 ; usBuffer < g_usNbBuffer ; ++usBuffer )
		{
			m_positions[usBuffer]=oneChain->m_dynamic[oneChain->m_rotationnalIndex[usBuffer]].m_pPositions.Get();
			m_angles[usBuffer]=oneChain->m_dynamic[oneChain->m_rotationnalIndex[usBuffer]].m_angles.Get();
		}
	}
}; // end of class Dynamic






////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// FCUNTIOJ
void ProcessOneChain(OneChain* oneChain, const MetaBallGroup& metaBallGroup, const Global& global);


} // end of namespace ChainSPU



#endif // end of _HAIRDATA_PS3_H_
