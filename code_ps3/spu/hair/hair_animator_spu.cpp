#include "hair_animator_spu.h"


#include "core/spericalcoordinate.h"
#include <math.h>

#if defined(SN_TARGET_PS3_SPU)
	#include <util_spu.h>
	#include <spu_intrinsics.h>
	#include <ntDma.h>
#endif

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//
// to optimise: hint for all the if
// check vecmath use
// BIG ONE: unroll loop (probably no time for that one
// if you do it, be aware that each joint must coimputed AFTER his parent is completed
// it means: tricky depepedncy
//
// a lot a branch hint as well, that's very important actually consdering the amount of branch
//
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

//#include <vmx2spu.h>

//#define CHECK_NANS

#ifdef CHECK_NANS

inline bool IsNan(float f)
{
	union { float f_; unsigned int ui_; } data;
	data.f_ = f;
	unsigned int u = data.ui_ & 0x7f800000;

	return (u == 0x7f800000) || fabs(f) > 1e6;
}

#include "core/nanchecker.h"


void CheckOneChain(ChainSPU::OneChain* chain)
{
	unsigned int numElems = chain -> m_usNbChainElem;
	ChainSPU::ChainElemDynamic* dynamicElems = chain -> m_pChainElemDynamics.Get();  

	CVector* positions[ChainSPU::g_usNbBuffer] = { chain -> m_dynamic[0].m_pPositions.Get(), chain -> m_dynamic[1].m_pPositions.Get(), chain -> m_dynamic[2].m_pPositions.Get()};
	TmpVec2* angles[ChainSPU::g_usNbBuffer] = { chain -> m_dynamic[0].m_angles.Get(), chain -> m_dynamic[1].m_angles.Get(), chain -> m_dynamic[2].m_angles.Get()};


	for (unsigned int elem = 0; elem < numElems; ++ elem )
	{
		CHECK_FOR_NAN(dynamicElems[elem].m_modifiedAcceleration);
		//CHECK_FOR_NAN(dynamicElems[elem].m_lastRotationAxis);
		CHECK_FOR_NAN(dynamicElems[elem].m_collisionForce.m_worldNorm);
		CHECK_FOR_NAN(dynamicElems[elem].m_collisionForce.m_fOutCoef);

		for (int buffer = 0; buffer < ChainSPU::g_usNbBuffer; ++ buffer)
		{
			CHECK_FOR_NAN(positions[buffer][elem]);
			CHECK_FOR_NAN(angles[buffer][elem][0]);
			CHECK_FOR_NAN(angles[buffer][elem][1]);
		}

	}

}

typedef CheckedPtr<ChainSPU::OneChain, &CheckOneChain>  OneChainPtr;
#else
typedef ChainSPU::OneChain*	OneChainPtr;
#define CHECK_FOR_NAN(arg)
#endif



#ifdef PLATFORM_PC
void MatrixMultiply(CMatrix& res, const CMatrix& a, const CMatrix& b)
{
	res=a*b;
}
#endif // PLATFORM_PC



inline void SetWToZero(CVector& v)
{
	v *= CVector(1.0f,1.0f,1.0f,0.0f);
}
inline void SetW(CVector& v, float f)
{
	v *= CVector(1.0f,1.0f,1.0f,0.0f);
	v += CVector(0.0f,0.0f,0.0f,f);
}






namespace ChainSPU
{
using namespace SimpleFunction;



// position correction	
void CollisionRelaxForce(CVector& forces, const ChainElemStatic& elemStatic, const ChainElemDynamic& elemDynamic, const Dynamic& dynamic, const OneChainPtr oneChain, const Global& global)
{
	// handy
	const CollisionForce& colForce = elemDynamic.m_collisionForce;
	ntAssert(colForce.IsUnderInfluence());

	///////////////////////////////////////////////////
	// Group unconstrained
	float fConstrainedDot = forces.Dot(colForce.m_worldNorm);
	CVector unconstrainedForce = forces - fConstrainedDot * colForce.m_worldNorm;
	
	// how much we want to decrease unconstrained (perpendicular)
	float fUnconstrainedCoef = Lerp(1.0f, oneChain->m_def.m_fUnconstrainedCoef,1.0f-colForce.m_fOutCoef);
	unconstrainedForce *= fUnconstrainedCoef;

	// End of Group unconstrained
	///////////////////////////////////////////////////
	
	
	///////////////////////////////////////////////////
	// Group constrained

	// radial speed (how much the node is going directly into the sphere) TODO: redundant with speed in dynamic ?
	CVector speed = (dynamic.m_positions[BEFORE][elemStatic.m_usSelfIndex]-dynamic.m_positions[EVENBEFORE][elemStatic.m_usSelfIndex]) / global.m_fDelta;
	SetWToZero(speed);
	float fRadialSpeed = colForce.m_worldNorm.Dot(speed);
	
	// corection coef
	float fPositionCorrection = oneChain->m_def.m_fPositionCorrection;
	float fSpeedCorrection = fRadialSpeed * oneChain->m_def.m_fSpeedCorrection;
	
	// contrained 
	CVector constraintForce = Lerp(fPositionCorrection + fSpeedCorrection,fConstrainedDot,colForce.m_fOutCoef) * colForce.m_worldNorm;
	// End of Group constrained
	///////////////////////////////////////////////////
	
	//////////////////////////////
	// sum and set
	forces = unconstrainedForce + constraintForce;
	//forces = unconstrainedForce;
}	




CVector GetLocalForce(const ChainElemStatic& elemStatic, const ChainElemDynamic& elemDynamic, const ChainElemDynamic& parentDynamic, const Dynamic& dynamic, const OneChainPtr oneChain, const Global& global, unsigned int subFrame)
{	
	// clean build
	CVector localForces(CONSTRUCT_CLEAR);
	
	// wind
	if(oneChain->m_def.m_flags[ChainDef::WIND])
	{
		// wind
		CVector windForce = oneChain->m_wind * elemStatic.m_def.m_fFluid;
		// fluid forces
		if(oneChain->m_def.m_flags[ChainDef::WIND_E3HACK])
		{
			float time = global.m_fTime + global.m_fDelta * subFrame;
			float fWindCoef = 1.0f + 0.2f * cos(10.0f * elemStatic.m_def.m_fDistanceToRoot + 10.0f * time);
			windForce*=fWindCoef;
		}
		localForces += windForce;
	}

	// fluid forces
	if(oneChain->m_def.m_flags[ChainDef::FLUID])
	{
		// the fluid forces is the one who says that the hair moves like in a fluid
		// this is a simple friction : the fluid force is apposite to the speed
		CVector fluidForce = (dynamic.m_positions[CURRENT][elemStatic.m_usParentIndex]-dynamic.m_positions[BEFORE][elemStatic.m_usParentIndex]) * (-elemStatic.m_def.m_fFluid / global.m_fDelta);
		FRANKHAIRPRINT_FLOAT3("fluidForce",fluidForce);
		localForces += fluidForce;
	}
	
	// local reference frame forces
	// when the root node is accelerating in one direction, the extremity want to go in the other direction
	if(oneChain->m_def.m_flags[ChainDef::ACCELERATION])
	{
		// high speed induce null acceleration propagation
		CVector accForce = parentDynamic.m_modifiedAcceleration * (-oneChain->m_def.m_fAccelerationPropagation * elemStatic.m_def.m_fWeight);
		FRANKHAIRPRINT_FLOAT3("accForce",accForce);
		localForces += accForce;
	}
	
	// gravity
	if(oneChain->m_def.m_flags[ChainDef::GRAVITY])
	{
		CVector gravityForce = global.m_gravity * elemStatic.m_def.m_fWeight;
		FRANKHAIRPRINT_FLOAT3("gravityForce",gravityForce);
		localForces += gravityForce;
	}
	
	// parent stiffneff, make it straight (should add suport for not straight)
	if(oneChain->m_def.m_flags[ChainDef::STIFF])
	{
		float fDotTreshold = elemStatic.m_def.m_fStiffDotTreshold;
		float fDot = CDirection(elemDynamic.m_lastRotationAxis[0]).Dot(CDirection(parentDynamic.m_lastRotationAxis[0]));
		if(fDot<fDotTreshold)
		{
			float fCoef = LinearStep(-1.0f, fDotTreshold, fDot);
			CVector parentForces =  (elemStatic.m_def.m_fStiff*fCoef) * CVector(parentDynamic.m_lastRotationAxis[0]);
			FRANKHAIRPRINT_FLOAT3("parentForces",parentForces);
			localForces += parentForces;
		}
	}

	// spring
	if(oneChain->m_def.m_flags[ChainDef::SPRING] && elemStatic.m_usNbSpring>0)
	{
		CVector current = dynamic.m_positions[BEFORE][elemStatic.m_usSelfIndex];
		CVector springForce(CONSTRUCT_CLEAR);
		Spring* pSpring = oneChain->m_pSprings.Get();
		uint16_t* pSpringIndices = oneChain->m_pSpringIndices.Get();
		for(uint16_t usSpring = 0 ; usSpring < elemStatic.m_usNbSpring ; ++usSpring )
		{
			// get spring
			uint16_t usIndex = pSpringIndices[elemStatic.m_usFirstSpring+usSpring];
			Spring& spring = pSpring[usIndex];
			
			// what is the other index (call that a hack if you want?
			uint16_t usOtherIndex = spring.m_chainElemIndices[0]==elemStatic.m_usSelfIndex
				?spring.m_chainElemIndices[1]
				:spring.m_chainElemIndices[0];

			// get diff
			CVector auxSpringForce(dynamic.m_positions[BEFORE][usOtherIndex] - current);
			SetWToZero(auxSpringForce);
			
			// transform diff into force
			float fLength = auxSpringForce.Length();
			float fLengthDiff = fLength - spring.m_fLength;
			auxSpringForce *= (spring.m_fSpringStiffness * fLengthDiff) / (fLength*fLength);
			FRANKHAIRPRINT_FLOAT3("auxspringforce",auxSpringForce);
			
			// add force to springForce
			springForce += auxSpringForce;
		}
		FRANKHAIRPRINT_FLOAT3("springForce",springForce);
		localForces += springForce;
	}

	// relax forces with collision stuff
	if(oneChain->m_def.m_flags[ChainDef::COLLISIONFORCE] && elemDynamic.m_collisionForce.IsUnderInfluence())
	{
		CollisionRelaxForce(localForces, elemStatic, elemDynamic, dynamic, oneChain, global);
	}
	
	return localForces;
}






// force -> torque
TmpVec2 ForceToTorque(const CVector& force, const ChainElemStatic& elemStatic, const ChainElemDynamic& elemDynamic, const Dynamic& dynamic)
{
	TmpVec2 res(CDirection(force).Dot(CDirection(elemDynamic.m_lastRotationAxis[1])),
		sin(dynamic.m_angles[BEFORE][elemStatic.m_usSelfIndex][0]) * CDirection(force).Dot(CDirection(elemDynamic.m_lastRotationAxis[2])));

	return res * elemStatic.m_def.m_fLength;
}

// compute new position
void ComputeNewPosition(const TmpVec2& torque, const ChainElemStatic& elemStatic, const Dynamic& dynamic, const OneChainPtr oneChain, const Global& global)
{
	// FIXME more efficient please
	float fInertiaMoment =  elemStatic.m_def.m_fInertiaMoment * elemStatic.m_def.m_fWeight * elemStatic.m_def.m_fLength * elemStatic.m_def.m_fLength;
	float fInvInertiaMoment = 1.0f / fInertiaMoment;

	FRANKHAIRPRINT_FLOAT2("BEFORE",dynamic.m_angles[BEFORE][elemStatic.m_usSelfIndex]);
	FRANKHAIRPRINT_FLOAT2("EVENBEFORE",dynamic.m_angles[EVENBEFORE][elemStatic.m_usSelfIndex]);

	// part of the acceleration (the other is what we want to compute)
	TmpVec2 part1 = dynamic.m_angles[BEFORE][elemStatic.m_usSelfIndex]*2.0f - dynamic.m_angles[EVENBEFORE][elemStatic.m_usSelfIndex];
	FRANKHAIRPRINT_FLOAT2("part1",part1);

	// damping:
	TmpVec2 part2 = oneChain->m_def.m_flags[ChainDef::DAMP]
		? (dynamic.m_angles[BEFORE][elemStatic.m_usSelfIndex] - dynamic.m_angles[EVENBEFORE][elemStatic.m_usSelfIndex]) * (-global.m_fDelta*elemStatic.m_def.m_fDamp)
		: TmpVec2(0.0f,0.0f);
	FRANKHAIRPRINT_FLOAT2("part2",part2);

	// force torque:
	float fPart3Coef = global.m_fDelta * global.m_fDelta * fInvInertiaMoment;
	TmpVec2 part3 = torque*fPart3Coef;
	FRANKHAIRPRINT_FLOAT2("part3",part3);
	
	// compute new angle:
	TmpVec2 newAngle = part1 + part2 + part3;

	CHECK_FOR_NAN(newAngle[0]);
	
	// diff
	TmpVec2 diff = newAngle - dynamic.m_angles[BEFORE][elemStatic.m_usSelfIndex];
	FRANKHAIRPRINT_FLOAT2("diff",diff);

	CHECK_FOR_NAN(diff[0]);
	
	// mega clamp
	if(oneChain->m_def.m_flags[ChainDef::MEGADAMP])
	{
		diff *= oneChain->m_def.m_fMegaDamp;
		FRANKHAIRPRINT_FLOAT2("diff",diff);
	}

	CHECK_FOR_NAN(diff[0]);

	// classic
	if(oneChain->m_def.m_flags[ChainDef::CLIPANGLE])
	{
		diff = TmpVec2(
			Sign(diff[0]) * ntstd::Clamp(fabs(diff[0]),0.0f,oneChain->m_def.m_fClipAngle),
			Sign(diff[1]) * ntstd::Clamp(fabs(diff[1]),0.0f,oneChain->m_def.m_fClipAngle));
		//diff[0] = Sign(diff[0]) * clamp(abs(diff[0]),0.0f,oneChain->m_def.m_fClipAngle);
		//diff[1] = Sign(diff[1]) * clamp(abs(diff[1]),0.0f,oneChain->m_def.m_fClipAngle);
		FRANKHAIRPRINT_FLOAT2("diff",diff);
	}

	CHECK_FOR_NAN(diff[0]);

	dynamic.m_angles[CURRENT][elemStatic.m_usSelfIndex] = dynamic.m_angles[BEFORE][elemStatic.m_usSelfIndex] + diff;
	
	// set m_worldExtremity
	CVector worldDec = SphericalCoordinate<TmpVec2>::GetBaseDir(dynamic.m_angles[CURRENT][elemStatic.m_usSelfIndex]) * elemStatic.m_def.m_fLength;
	dynamic.m_positions[CURRENT][elemStatic.m_usSelfIndex] = dynamic.m_positions[CURRENT][elemStatic.m_usParentIndex] + worldDec;
}



struct PerJointColInfo
{
public:
	float m_fPotentialCoef;
	float m_fPotentialEnd;
	float m_fJointRadius;
public:
	PerJointColInfo(float fPotentialCoef, float fPotentialEnd, float fJointRadius) 
		:m_fPotentialCoef(fPotentialCoef)
		,m_fPotentialEnd(fPotentialEnd)
		,m_fJointRadius(fJointRadius)
	{
		// nothing
	}
}; // en of PerJointColInfo






float Potential2Distance(float fPotential, float fPotentialEnd)
{
	float fDist = fPotentialEnd - sqrt(fPotential) * (fPotentialEnd - 1.0f);
	return fDist;
}

float Distance2Potential(float fNormDistance, float fPotentialEnd)
{
	float fPotentialAux = (fPotentialEnd - fNormDistance) / (fPotentialEnd - 1.0f);
	return fPotentialAux*fPotentialAux;
}
float Potential2Decalage(float fPotential, float fPotentialEnd, float fRefLength)
{
	float fNormDist = Potential2Distance(fPotential, fPotentialEnd);
	if(fNormDist < 1.0f)
	{
		return fRefLength * (1.0f - fNormDist);
	}
	else
	{
		return 0.0f;
	}
}


float PerSphereCollisionDetection(CollisionCache& collisionCache, const MetaBall& metaBall, const PerJointColInfo& perJointColInfo,
	const ChainElemStatic& elemStatic, const Dynamic& dynamic)
{
	// goto unit sphere to detect collision
	CVector spherePos = dynamic.m_positions[CURRENT][elemStatic.m_usSelfIndex] * metaBall.m_world2Sphere;
	SetWToZero(spherePos);
	
	FRANKHAIRPRINT_FLOAT3("spherePos",spherePos);


	// very basic approximation of how to make artificially the ellispoid bigger
	// when the radisu of the hair is bigger
	float fJointRadiusRatio = perJointColInfo.m_fJointRadius / metaBall.m_def.m_fAverageRadius;
	float fRadiusCoef = perJointColInfo.m_fPotentialCoef + fJointRadiusRatio;
	
	// distance between the center of the elipsoid and the joint (center of its sphere)
	float fSqCenter2Center = spherePos.LengthSquared() / (fRadiusCoef * fRadiusCoef);
	FRANKHAIRPRINT_FLOAT("fSqCenter2Center",fSqCenter2Center);
	
	// normalised distance treshold
	float fSqCenter2CenterTreshold = perJointColInfo.m_fPotentialEnd * perJointColInfo.m_fPotentialEnd;
	FRANKHAIRPRINT_FLOAT("fSqCenter2CenterTreshold",fSqCenter2CenterTreshold);
	
	// return if not close enough
	if(fSqCenter2Center > fSqCenter2CenterTreshold)
	{
		return fSqCenter2Center;
	}	
		
	// length between ellisoid center and joint
	float fspherePosLen = sqrt(fSqCenter2Center);
	FRANKHAIRPRINT_FLOAT("fspherePosLen",fspherePosLen);
	
	// potential
	float fPotential = Distance2Potential(fspherePosLen,perJointColInfo.m_fPotentialEnd);
	FRANKHAIRPRINT_FLOAT("fPotential",fPotential);
	
	// position of the point in the surface on the small sphere
	CVector sphereNormal = spherePos / fspherePosLen;
	FRANKHAIRPRINT_FLOAT3("sphereNormal",sphereNormal);
	
	
	// prepare coefficient for distance ref
	CVector sphereNormalAux = sphereNormal * sphereNormal;
	FRANKHAIRPRINT_FLOAT3("sphereNormalAux",sphereNormalAux);

	// reference distance
	float fDistanceAux = sphereNormalAux.Dot(metaBall.m_def.m_scale) * fRadiusCoef;
	float fSqRefAux = fDistanceAux*fDistanceAux;
	FRANKHAIRPRINT_FLOAT("fSqRefAux",fSqRefAux);
	
	// compute normal
	CVector normalisedPlane  = sphereNormal;
	SetWToZero(normalisedPlane);
	SetW(normalisedPlane,-fRadiusCoef);
	CMatrix world2SphereTranspose = metaBall.m_world2Sphere.GetTranspose();
	CVector worldNormal  = normalisedPlane * world2SphereTranspose;
	SetWToZero(worldNormal);
	worldNormal *= (1.0f/worldNormal.Length());
	FRANKHAIRPRINT_FLOAT3("worldNormal",worldNormal);

	// insert collision data object
	//if(!isnan(fSqRefAux))
	//{
	collisionCache.m_sphereArray[collisionCache.m_usNbValidSphere].Set(worldNormal,fPotential,fSqRefAux);
	++collisionCache.m_usNbValidSphere;
	ntAssert(collisionCache.m_usNbValidSphere<=CollisionCache::g_usCacheSize);
	//}
	
	// return heuristic
	return fSqCenter2Center; 	
}




void FinaliseDetection(CollisionDisplacement& collisionDisplacement, CollisionForce& collisionForce, float fPotentialExpulsion,
	const CollisionCache& collisionCache, const OneChainPtr oneChain)
{
	static const float g_potentialEpsilon = 0.00001f;
	
	// reset some value
	float fTotalFakePotential = 0.0f;
	collisionDisplacement.m_fPotential = 0.0f;
	collisionDisplacement.m_fRefLengthSquare = 0.0f;
	collisionForce.m_worldNorm = CVector(CONSTRUCT_CLEAR);
	
	for(uint16_t usSphere = 0 ; usSphere < collisionCache.m_usNbValidSphere ; ++usSphere )
	{
		const CollisionCache::SphereCache& sphereCache = collisionCache.m_sphereArray[usSphere];

		float fFakePotential =  ntstd::Max(g_potentialEpsilon,sphereCache.m_fPotential);
		fTotalFakePotential += fFakePotential;
		collisionDisplacement.m_fPotential += sphereCache.m_fPotential;
		collisionDisplacement.m_fRefLengthSquare += fFakePotential * sphereCache.m_fRefLengthSquare;
		collisionForce.m_worldNorm += fFakePotential * sphereCache.m_worldNorm;

		FRANKHAIRPRINT_FLOAT("m_fPotential[]",sphereCache.m_fPotential);
		FRANKHAIRPRINT_FLOAT("m_fRefLengthSquare[]",sphereCache.m_fRefLengthSquare);
		FRANKHAIRPRINT_FLOAT3("m_worldNorm[]",sphereCache.m_worldNorm);
	}

	// normalise normal
	collisionForce.m_worldNorm *= (1.0f/collisionForce.m_worldNorm.Length());
	collisionDisplacement.m_fRefLengthSquare *= (1.0f/fTotalFakePotential);
	

	// compute distance from implicit surface (hacky aproximation)
	float fRefLength = sqrt(collisionDisplacement.m_fRefLengthSquare);
	fRefLength *= fPotentialExpulsion;
	float fDist = Potential2Decalage(collisionDisplacement.m_fPotential,oneChain->m_def.m_fPotentialEnd,fRefLength);
	
	// compute world diff from old to new
	collisionDisplacement.m_worldDiff = collisionForce.m_worldNorm * fDist;
	
	// outcoef, when do the layer stop
	collisionForce.m_fOutCoef = LinearStep(oneChain->m_def.m_collisionForceRange[0],oneChain->m_def.m_collisionForceRange[1],collisionDisplacement.m_fPotential);

	FRANKHAIRPRINT_FLOAT("m_fPotential",collisionDisplacement.m_fPotential);
	FRANKHAIRPRINT_FLOAT("m_fRefLengthSquare",collisionDisplacement.m_fRefLengthSquare);
	FRANKHAIRPRINT_FLOAT("m_fOutCoef",collisionForce.m_fOutCoef);
	FRANKHAIRPRINT_FLOAT3("m_worldDiff",collisionDisplacement.m_worldDiff);
}




// position correction	
void CollisionDetection(CollisionCache& collisionCache, CollisionDisplacement& collisionDisplacement, CollisionForce& collisionForce, float fPotentialExpulsion,
	const MetaBallGroup& metaBallGroup, const ChainElemStatic& elemStatic, const Dynamic& dynamic, const OneChainPtr oneChain)
{
	PerJointColInfo perJointColInfo(
		oneChain->m_def.m_fPotentialCoef,
		oneChain->m_def.m_fPotentialEnd,
		elemStatic.m_def.m_fRadius);

	collisionCache.m_usNbValidSphere = 0;
	
	MetaBall* pMetaBalls = metaBallGroup.m_pMetaBalls.Get();
	for(uint32_t iSphere = 0 ; iSphere < metaBallGroup.m_usNbMetaBall ; iSphere++ )
	{
		PerSphereCollisionDetection(collisionCache, pMetaBalls[iSphere], perJointColInfo, elemStatic, dynamic);
	}	
	FRANKHAIRPRINT_INT("collision",collisionCache.m_usNbValidSphere);

	//ntPrintf("collisionCache.m_usNbValidSphere = %i\n",collisionCache.m_usNbValidSphere);
	if(collisionCache.m_usNbValidSphere>0)
	{
		FinaliseDetection(collisionDisplacement, collisionForce, fPotentialExpulsion, collisionCache, oneChain);
	}
	else
	{
		collisionForce.SetNotInfluenced();
		collisionDisplacement.SetNotColliding();
	}
}



// Moved constants here as GCC4 with -O0 was screwing them up
// when they were defined at function scope.
//namespace AngleModConsts
//{
//	static const float PI = 3.14159265f;
//	static const float INVCOEF = 1.0f / (2.0f * PI);
//	static const float COEF = 2.0f * PI;
//}
// Alexey: had to make those to be defines thanks to the gcc4
#define PI		3.14159265f
#define INVCOEF 0.1591549430918953f
#define COEF	6.2831853072f

inline TmpVec2 AngleMod(TmpVec2 v)
{
	TmpVec2 res = v;
	res[0] *= /*AngleModConsts::*/INVCOEF;
	res[1] *= /*AngleModConsts::*/INVCOEF;
	res[0] = floor(res[0]+0.5f);
	res[1] = floor(res[1]+0.5f);
	res[0] *= /*AngleModConsts::*/COEF;
	res[1] *= /*AngleModConsts::*/COEF;
	
	return res;
}



// position correction	
void ApplyDiff(const CVector& worldDiff, float fKillAcc, float fResize, const ChainElemStatic& elemStatic, const Dynamic& dynamic, const OneChainPtr oneChain)
{
	//ntPrintf("worldDiff=%f,%f,%f\n",worldDiff.X(),worldDiff.Y(),worldDiff.Z());
	
	// compute new position
	CVector newPos = dynamic.m_positions[CURRENT][elemStatic.m_usSelfIndex] + worldDiff;
	
	// get parent position
	CVector parenPos = dynamic.m_positions[CURRENT][elemStatic.m_usParentIndex];
	
	// get diff (the joint vertex)
	CVector parentDiff = newPos - parenPos;
	
	// spherical coordinate
	TmpVec2 angle; float fRadius;
	SphericalCoordinate<TmpVec2>::CartesianToSpherical(CDirection(parentDiff),angle,fRadius);
	
	// resize parent
	float fNewLength = Lerp(fRadius , elemStatic.m_def.m_fLength, fResize);
	parentDiff *= fNewLength / fRadius;
	
	// new angle
	//TmpVec2 angle = TmpVec2(sc[1],sc[2]);
	TmpVec2 angleCurrent = dynamic.m_angles[CURRENT][elemStatic.m_usSelfIndex];
	TmpVec2 angleDiff = angle-angleCurrent;
	
	///////////////////////////////////////////////////
	// stuff not to worry about angle 2pi modulo issue
	TmpVec2 angleRest = AngleMod(angleDiff);
	angle -= angleRest;
	if(!SphericalCoordinate<TmpVec2>::IsSphericalAngleBounded(angle-angleCurrent))
	{
		oneChain->m_flags.Set(OneChain::RESET_NEEDED,true);
		ntPrintf("Crazy hair: new=(%f,%f) old=(%f,%f)\n",angle[0],angle[1],angleCurrent[0],angleCurrent[1]);
		//ntBreakpoint();
	}
	//ntAssert(SphericalCoordinate<TmpVec2>::IsSphericalAngleBounded(angle-angleCurrent));
	///////////////////////////////////////////////////
	
	// cartesian -> spherical
	dynamic.m_angles[CURRENT][elemStatic.m_usSelfIndex] = angle;
	
	// kill acceleration
	dynamic.m_angles[BEFORE][elemStatic.m_usSelfIndex] = Lerp(angle,dynamic.m_angles[BEFORE][elemStatic.m_usSelfIndex],1.0f * fKillAcc);
	dynamic.m_angles[EVENBEFORE][elemStatic.m_usSelfIndex] = Lerp(angle,dynamic.m_angles[EVENBEFORE][elemStatic.m_usSelfIndex],1.0f * fKillAcc * fKillAcc);
	
	// position
	dynamic.m_positions[CURRENT][elemStatic.m_usSelfIndex] = parenPos + parentDiff;
	
	FRANKHAIRPRINT_FLOAT2("angleDiff",angleDiff);
	FRANKHAIRPRINT_FLOAT3("positionDiff",parentDiff);
}



void SetModifiedAcceleration(ChainElemDynamic& elemDynamic, const Dynamic& dynamic, const ChainElemStatic& elemStatic, const OneChainPtr oneChain, const Global& global)
{	
	// FIXME, more efficient, use speed differential
	CVector acceleration =
		(dynamic.m_positions[CURRENT][elemStatic.m_usSelfIndex]
		- 2*dynamic.m_positions[BEFORE][elemStatic.m_usSelfIndex]
		+ dynamic.m_positions[EVENBEFORE][elemStatic.m_usSelfIndex]) / (global.m_fDelta * global.m_fDelta);

	CHECK_FOR_NAN(acceleration);
	// hack to keep acceleration nice and smooth
	elemDynamic.m_modifiedAcceleration = elemDynamic.m_modifiedAcceleration*oneChain->m_def.m_fAccelerationEcho + acceleration;
}

void SetAxisMatrix(ChainElemDynamic& elemDynamic, const ChainElemStatic& elemStatic, const Dynamic& dynamic)
{
	SphericalCoordinate<TmpVec2>::SetMatrix(elemDynamic.m_lastRotationAxis,dynamic.m_angles[CURRENT][elemStatic.m_usSelfIndex]);
}

///////////////////////////////////////////////////////////////////////////////////////
// update transform
void UpdateWorldMatrix(ChainElemTmp& elemTmp, const ChainElemTmp& parentTmp, const ChainElemStatic& elemStatic, const Dynamic& dynamic)
{
	// get x approximation
	CDirection exaux = parentTmp.m_xAxisApprox;
	FRANKHAIRPRINT_FLOAT3("exaux",exaux);

	// get y axis
	CVector root = CVector(parentTmp.m_local2world[3]);
	FRANKHAIRPRINT_FLOAT3("root",root);
	CVector extremity = dynamic.m_positions[CURRENT][elemStatic.m_usSelfIndex];
	CDirection ey = CDirection(extremity - root);
	ey *= 1.0f / ey.Length();

	//CDirection ey = CDirection(m_lastRotationAxis[0]);
	CDirection ez = exaux.Cross(ey);
	ez *= 1.0f / ez.Length();
	exaux = ey.Cross(ez);
	FRANKHAIRPRINT_FLOAT3("before",exaux);

	exaux.Normalise();
	elemTmp.m_xAxisApprox=exaux;

	FRANKHAIRPRINT_FLOAT3("ey",ey);
	FRANKHAIRPRINT_FLOAT3("m_xAxisApprox",elemTmp.m_xAxisApprox);

	// set world rotation
	CMatrix local2world(CONSTRUCT_CLEAR);
	local2world.SetXAxis(elemTmp.m_xAxisApprox);
	local2world.SetYAxis(ey);
	local2world.SetZAxis(ez);
	FRANKHAIRPRINT_FLOAT3("x",elemTmp.m_xAxisApprox);
	FRANKHAIRPRINT_FLOAT3("y",ey);
	FRANKHAIRPRINT_FLOAT3("z",ez);
	FRANKHAIRPRINT_MATRIX("local2world",local2world);

	// add extra rotation to match skin stuff (because of jamexport)
	// rotation in maya is [rotateAxis] * [jointOrient]
	// the transform is just [jointOrient]
	// warning: [rotateAxis1] * [jointOrient1] , [rotateAxis2] * [jointOrient2]
	// joint 1: transform = [jointOrient1]
	// joint 2: transform = [jointOrient2] * [rotateAxis1] * [jointOrient1]
	local2world = elemStatic.m_def.m_extraRot * local2world;

	// set world translation
	//m_local2world.SetTranslation(CPoint(m_extremity.m_position));
	local2world.SetTranslation(CPoint(extremity));

	// set
	elemTmp.m_local2world = local2world;
	elemTmp.m_local = elemTmp.m_local2world * parentTmp.m_local2world.GetAffineInverse();

	FRANKHAIRPRINT_MATRIX("DYNA: parent_local2world",parentTmp.m_local2world);
	FRANKHAIRPRINT_MATRIX("DYNA: local2world",elemTmp.m_local2world);
	FRANKHAIRPRINT_MATRIX("DYNA: local",elemTmp.m_local);
	FRANKHAIRPRINT_FLOAT4("DYNA: m_xAxisApprox",elemTmp.m_xAxisApprox);
}
// update transform
///////////////////////////////////////////////////////////////////////////////////////


void SetFrozenJoint(ChainElemTmp* pChainTmps, Dynamic& dynamic, const ChainElemStatic& elemStatic, ChainElemDynamic& elemDynamic,
	const OneChainPtr oneChain, const Global& global)
{
	// handy
	const ChainElemTmp& parentTmp = pChainTmps[elemStatic.m_usParentIndex];
	ChainElemTmp& elemTmp = pChainTmps[elemStatic.m_usSelfIndex];

	// set local to wrold matrix
	elemTmp.m_local2world = elemStatic.m_def.m_local * parentTmp.m_local2world;
	
	// angle (and set dynamic)
	CDirection diff = CDirection(elemTmp.m_local2world.GetTranslation() - parentTmp.m_local2world.GetTranslation());
	SphericalCoordinate<TmpVec2>::CartesianToSpherical(diff,dynamic.m_angles[CURRENT][elemStatic.m_usSelfIndex]);

	// position (and set dynamic)
	dynamic.m_positions[CURRENT][elemStatic.m_usSelfIndex] = CVector(elemTmp.m_local2world[3]);
	
	// set extremity and rotation matrix
	SetModifiedAcceleration(elemDynamic, dynamic, elemStatic, oneChain, global);
	SetAxisMatrix(elemDynamic, elemStatic, dynamic);

	// compute local
	elemTmp.m_local = elemStatic.m_def.m_local;
	// get x approximation
	CMatrix tmp;
	MatrixMultiply(tmp, elemStatic.m_def.m_extraRotInv, elemTmp.m_local2world);
	elemTmp.m_xAxisApprox = tmp.GetXAxis();
	
	FRANKHAIRPRINT_MATRIX("FREE: elemStatic.m_def.m_local",elemStatic.m_def.m_local);
	FRANKHAIRPRINT_MATRIX("FREE: parentTmp.m_local2world",parentTmp.m_local2world);

	FRANKHAIRPRINT_MATRIX("FREE: m_extraRotInv",elemStatic.m_def.m_extraRotInv);
	FRANKHAIRPRINT_MATRIX("FREE: local2world",elemTmp.m_local2world);
	FRANKHAIRPRINT_MATRIX("FREE: local",elemTmp.m_local);
	FRANKHAIRPRINT_FLOAT4("FREE: m_xAxisApprox",elemTmp.m_xAxisApprox);
}


//// this could be avoided by a smarter spu allocation system
//void InitSentinel(ChainElemTmp* pChainTmps, OneChainPtr oneChain)
//{
//	ChainElemTmp* pChainElemSentinel = oneChain->m_pChainElemSentinel.Get();
//	for(uint32_t iSentinel = 0 ; iSentinel < oneChain->m_usFirstDynamicElem ; ++iSentinel )
//	{
//		// tmp value
//		pChainTmps[iSentinel].m_local = pChainElemSentinel[iSentinel].m_local;
//		pChainTmps[iSentinel].m_local2world = pChainElemSentinel[iSentinel].m_local2world;
//		pChainTmps[iSentinel].m_xAxisApprox = pChainElemSentinel[iSentinel].m_xAxisApprox;
//	}
//}



// anti-correction	
void ApplyAntiCollision(const AntiCollisionSphere* pAntiCollisionSphere, Dynamic& dynamic, const ChainElemStatic& elemStatic, const OneChainPtr oneChain)
{
	const AntiCollisionSphere& antiCollisionSphere = pAntiCollisionSphere[elemStatic.m_usAntiColIndex];
	
	CVector currentPos = dynamic.m_positions[CURRENT][elemStatic.m_usSelfIndex];
	FRANKHAIRPRINT_FLOAT3("currentPos",currentPos);
	FRANKHAIRPRINT_MATRIX("m_world2Sphere",antiCollisionSphere.m_world2Sphere);
	CVector spherePos = currentPos * antiCollisionSphere.m_world2Sphere;
	SetWToZero(spherePos);
	FRANKHAIRPRINT_FLOAT3("spherePos",spherePos);

	float fSqLength = spherePos.LengthSquared() / (oneChain->m_def.m_fClothCollision*oneChain->m_def.m_fClothCollision);
	if(fSqLength>1.0f)
	{
		float fLength = sqrt(fSqLength);
		FRANKHAIRPRINT_FLOAT("fLength",fLength);
		spherePos *= oneChain->m_def.m_fClothCollision / fLength;
		SetW(spherePos,1.0f);
		CVector worldPos = spherePos * antiCollisionSphere.m_sphere2Word;
		CVector worldDiff = worldPos - currentPos;
		FRANKHAIRPRINT_FLOAT3("worldDiff",worldDiff);
		ApplyDiff(worldDiff, 0.0f, oneChain->m_def.m_fAntiCollisionResize, elemStatic, dynamic, oneChain);
	}
}





void RenormaliseAngles(Dynamic& dynamic, OneChainPtr oneChain)
{
	uint16_t usNbDynamicJoint = oneChain->m_usNbChainElem-oneChain->m_usFirstDynamicElem;
	uint16_t usNbRenomalised = ntstd::Max((uint16_t)(usNbDynamicJoint/5),(uint16_t)1);
	for(uint16_t iDynamic = 0 ; iDynamic < usNbRenomalised ; ++iDynamic )
	{
		uint16_t iIndex = oneChain->m_usFirstDynamicElem + ((oneChain->m_usNumRenormalised+iDynamic) % usNbDynamicJoint);
		TmpVec2 angles = dynamic.m_angles[CURRENT][iIndex];
		TmpVec2 angleMod = AngleMod(angles);

		for(int iPast = 0 ; iPast < g_usNbBuffer ; iPast++ )
		{
			dynamic.m_angles[iPast][iIndex] -= angleMod;
		}
	}
	oneChain->m_usNumRenormalised = (oneChain->m_usNumRenormalised+usNbRenomalised) % usNbDynamicJoint;
}

#define CHECK_POSITION	CHECK_FOR_NAN(dynamic.m_positions[CURRENT][3])

void SubFrameUpate(OneChainPtr oneChain, ChainElemTmp* pChainTmps, const MetaBallGroup& metaBallGroup, const Global& global, uint_least16_t usSubFrane)
{
	// whithin frame progress
	float fHighFreqProgress = (usSubFrane+1) / static_cast<float>(oneChain->m_def.m_usNbStepPerFrame);
	// update high frequency rotationnal index
	oneChain->m_rotationnalIndex++;
	
	// per joint cache
	CollisionCache collisionCache;
	CollisionDisplacement collisionDisplacement;

	// make some pointer absolute
	ChainElemDynamic* pChainElemDynamics = oneChain->m_pChainElemDynamics.Get();
	const AntiCollisionSphere* pAntiCollisionSphere = oneChain->m_pAntiCollisionSphere.Get();;
	const SentinelInfo* pSentinelInfos = oneChain->m_pSentinelInfos.Get();;
	const ChainElemStatic* pChainElemStatics = oneChain->m_pChainElemStatics.Get();
	Dynamic dynamic(oneChain);
	
	// loop on chain sentinel
	for(uint16_t usSentinel = 0 ; usSentinel < oneChain->m_usFirstDynamicElem ; usSentinel++ )
	{
		// handy
		ChainElemTmp& chainTmp = pChainTmps[usSentinel];
		ChainElemDynamic& elemDynamic = pChainElemDynamics[usSentinel];
		const ChainElemStatic& elemStatic = pChainElemStatics[usSentinel];
		const SentinelInfo& sentinelInfo = pSentinelInfos[usSentinel];

		// set new position
		dynamic.m_positions[CURRENT][usSentinel] = SimpleFunction::Lerp(sentinelInfo.m_before, sentinelInfo.m_current, fHighFreqProgress);

		CHECK_POSITION;

		SetModifiedAcceleration(elemDynamic, dynamic, elemStatic, oneChain, global);

		CHECK_POSITION;

		// angle
		SphericalCoordinate<TmpVec2>::CartesianToSpherical(sentinelInfo.m_parentDiff,dynamic.m_angles[CURRENT][usSentinel]);

		CHECK_POSITION;

		SetAxisMatrix(elemDynamic, elemStatic, dynamic);

		CHECK_POSITION;

		// tmp info
		chainTmp.m_local = CMatrix(CONSTRUCT_CLEAR);
		chainTmp.m_local2world = sentinelInfo.m_local2world;
		chainTmp.m_xAxisApprox =  sentinelInfo.m_local2world.GetXAxis();

		FRANKHAIRPRINT_MATRIX("SENT: local2world",chainTmp.m_local2world);
		FRANKHAIRPRINT_MATRIX("SENT: local",chainTmp.m_local);
		FRANKHAIRPRINT_FLOAT4("SENT: m_xAxisApprox",chainTmp.m_xAxisApprox);

		CHECK_POSITION;
	}

	// loop on chain elemDynamic
	for(uint16_t iElem = oneChain->m_usFirstDynamicElem ; iElem < oneChain->m_usNbChainElem ; iElem++ )
	{
		FRANKHAIRPRINT_INT("elem",iElem);
		//ntPrintf("elem=%i\n",iElem);

		// convenient
		const ChainElemStatic& elemStatic = pChainElemStatics[iElem];		
		ChainElemDynamic& elemDynamic = pChainElemDynamics[iElem];
		ChainElemDynamic& parentElemDynamic = pChainElemDynamics[elemStatic.m_usParentIndex];		

		CHECK_POSITION;

		if(!elemStatic.m_def.m_flags[ChainElemDef::ISFROZEN])
		{
			// get local forces				
			CVector force = GetLocalForce(elemStatic, elemDynamic, parentElemDynamic, dynamic, oneChain, global, usSubFrane);
			SetWToZero(force);
			FRANKHAIRPRINT_FLOAT4("force",force);

			CHECK_POSITION;

			// new angle
			TmpVec2 torque = ForceToTorque(force, elemStatic, elemDynamic, dynamic);

			CHECK_POSITION;

			FRANKHAIRPRINT_FLOAT2("torque",torque);
			ComputeNewPosition(torque, elemStatic, dynamic, oneChain, global);

			CHECK_POSITION;

			FRANKHAIRPRINT_FLOAT2("angle",dynamic.m_angles[CURRENT][iElem]);
			FRANKHAIRPRINT_FLOAT4("position",dynamic.m_positions[CURRENT][iElem]);
			
			// collision detection
			if(oneChain->m_def.m_flags[ChainDef::ANYCOLLISION])
			{
				CollisionDetection(collisionCache, collisionDisplacement, elemDynamic.m_collisionForce, oneChain->m_def.m_fPotentialExpulsion, metaBallGroup, elemStatic, dynamic, oneChain);
			}

			CHECK_POSITION;
			
			// apply displacement
			//ntPrintf("collisionDisplacement.m_fPotential = %f\n", collisionDisplacement.m_fPotential);
			if(oneChain->m_def.m_flags[ChainDef::COLLISIONDISPLACEMENT] &&  collisionDisplacement.IsColliding())
			{
				ApplyDiff(collisionDisplacement.m_worldDiff, 0.0f, oneChain->m_def.m_fCollisionCorrectionResize, elemStatic, dynamic, oneChain);
			}

			CHECK_POSITION;
			if(oneChain->m_def.m_flags[ChainDef::ANTICOLLISIONDISPLACEMENT] && elemStatic.m_usAntiColIndex!=uint16_t(-1))
			{
				ApplyAntiCollision(pAntiCollisionSphere, dynamic, elemStatic, oneChain);
			}

			CHECK_POSITION;

			// set extremity (with latency) and rotation axis
			SetModifiedAcceleration(elemDynamic, dynamic, elemStatic, oneChain, global);

			CHECK_POSITION;

			SetAxisMatrix(elemDynamic, elemStatic, dynamic);
			FRANKHAIRPRINT_FLOAT4("acceleration",elemDynamic.m_modifiedAcceleration);

			CHECK_POSITION;

			// compute matrix
			const ChainElemTmp& parentTmp = pChainTmps[elemStatic.m_usParentIndex];
			ChainElemTmp& elemTmp = pChainTmps[elemStatic.m_usSelfIndex];

			CHECK_POSITION;

			// the update workd matrix function depends on the kind of reconstruction we are doing.
			UpdateWorldMatrix(elemTmp, parentTmp, elemStatic, dynamic);

			CHECK_POSITION;
		}
		else
		{
			// set frozen joint
			SetFrozenJoint(pChainTmps, dynamic, elemStatic, elemDynamic, oneChain, global);

			CHECK_POSITION;
		}

		FRANKHAIRPRINT_FLOAT2("angleFinal",dynamic.m_angles[CURRENT][iElem]);
		FRANKHAIRPRINT_FLOAT4("positionFinal",dynamic.m_positions[CURRENT][iElem]);

		FRANKHAIRPRINT_MATRIX("local",pChainTmps[elemStatic.m_usSelfIndex].m_local);
	}
	
	// renormalise a couple of angle
	RenormaliseAngles(dynamic,oneChain);
}


void ProcessOneChain(ChainSPU::OneChain * oneChain_p, const MetaBallGroup& metaBallGroup, const Global& global)
{
	OneChainPtr oneChain(oneChain_p);

	// temporary data
#ifdef SN_TARGET_PS3_SPU
	ChainElemTmp* pChainTmps = static_cast<ChainElemTmp*>( Allocate(oneChain->m_usNbChainElem*sizeof(ChainElemTmp)) );
#else
	ChainElemTmp* pChainTmps = NT_NEW_ARRAY ChainElemTmp[oneChain->m_usNbChainElem];
#endif // SN_TARGET_PS3_SPU


	// loop on subframe
	for(uint_least16_t usSubFrame = 0 ; usSubFrame < oneChain->m_def.m_usNbStepPerFrame ; ++usSubFrame )
	{
		FRANKHAIRPRINT_INT("subframe", usSubFrame);
		SubFrameUpate(oneChain, pChainTmps, metaBallGroup, global,usSubFrame);
	}

	// Setup the decomposed local joints.
	for ( uint16_t idx=oneChain->m_usFirstDynamicElem;idx<oneChain->m_usNbChainElem;idx++ )
	{
		pChainTmps[ idx ].m_LocalJoint.m_Rotation = CQuat( pChainTmps[ idx ].m_local );
		pChainTmps[ idx ].m_LocalJoint.m_Translation = pChainTmps[ idx ].m_local.GetTranslation();
	}

#ifdef SN_TARGET_PS3_SPU
	// Frank seemed to just queue DMAs up under the id 0 hopeing it wasn't in use
	// and never stalled on it. 99% of proberly worked but I've added the safety stall now to be safe
	// I suspect a list would be better
	ntDMA_ID id = ntDMA::GetFreshID();
#endif
	
	// copy local matrice to hierarchy
	const ChainElemStatic* pChainElemStatics = oneChain->m_pChainElemStatics.Get();
	for(uint16_t uiJoint = oneChain->m_usFirstDynamicElem ; uiJoint < oneChain->m_usNbChainElem ; uiJoint++ )
	{
		uint32_t uiLocalJointOffset = oneChain->m_uiFirstLocalJoint + pChainElemStatics[uiJoint].m_usTransformIndex*oneChain->m_uiLocalJointStride;
#ifdef SN_TARGET_PS3_SPU
		// note I think its okay to reuse the id, as the queue will stall when its full and so you can just
		// keep pumping them out under the same id... as long as you stall afterward to ensure the queue has 
		// emptied before passing control to the next task (which may use the same id as you)
		ntDMA::Params dmaParams2;
		dmaParams2.Init32( static_cast<void*>(&pChainTmps[uiJoint].m_LocalJoint), uiLocalJointOffset, sizeof(ChainElemTmp::_LocalDecomposed), id);
		ntDMA::DmaToPPU( dmaParams2 );
#else
		memcpy(reinterpret_cast<void*>(uiLocalJointOffset),&pChainTmps[uiJoint].m_LocalJoint,sizeof(ChainElemTmp::_LocalDecomposed));
#endif // PLATFORM_PS3
	}
#ifdef SN_TARGET_PS3_SPU
	ntDMA::StallForCompletion( id );
	ntDMA::FreeID( id );
#endif

	// delete temporary data
#ifdef SN_TARGET_PS3_SPU
	// nothing
#else
	NT_DELETE( pChainTmps );
#endif // PLATFORM_PS3
}




} // end of namespace ChainSPU

