#include "chaincore.h"

#include "game/entityhero.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "gfx/simplefunction.h"
#include "core/timer.h"
#include "haircollision.h"
#include "anim/hierarchy.h"
#include "tbd/franktmp.h"
#include "core/visualdebugger.h"
#include "core/boostarray.inl"
#include "forcefielditem.h"
#include "core/gatso.h"
#include "gfx/renderer.h"
#include "chainreconstruction.h"
#include "objectdatabase/dataobject.h"
#include "hair/chain2render.h"
#include "hair/winddef.h"
#include "core/spericalcoordinate.h"

using namespace HairConstant;
using namespace DebugColour;
using namespace SimpleFunction;

#ifdef PLATFORM_PS3
	#include "exec/ppu/dmabuffer_ps3.h"
	#include "exec/ppu/exec_ps3.h"
	#include "exec/ppu/spuprogram_ps3.h"
	#include "exec/ppu/sputask_ps3.h"
	#include "exec/ppu/ElfManager.h"
#endif // PLATFORM_PS3


#ifdef PLATFORM_PC
#define isnan _isnan
#endif // PLATFORM_PC









// TODO: factorize
namespace 
{
	template<typename UINT>
	UINT GetNextAlignment(UINT loc,UINT alignment)
	{
		UINT mod = (loc % alignment);
		if(mod > 0)
		{
			return loc-mod+alignment;
		}
		else
		{
			return loc;
		}
	}
} // end of namespace 
















HairSentinel::HairSentinel():
	m_pDef(0),
	m_pSrcTransform(NULL),
	m_pDestTransform(NULL),
	m_pParent(0)
{

}
HairSentinel::~HairSentinel()
{
	// nothing
}


void HairSentinel::SetDef(const HairRootDef* pDef)
{
	m_pDef = pDef;
}

void HairSentinel::Init(Transform* pSrcTransform, Transform* pDestTransform)
{
	m_pSrcTransform = pSrcTransform;
	m_pDestTransform = pDestTransform;
	m_chainElem.InitSentinel(pDestTransform);
}
void HairSentinel::Reset(const HairStyleFromMaya* m_pMayaDef)
{
	const CMatrix& m = m_pSrcTransform->GetWorldMatrix();
	m_perFrameInfo.assign(PerFrameInfo(m));	
	m_perSubframeInfo.assign(PerSubFrameInfo());
	
	m_chainElem.ResetSentinel(m_pMayaDef);
}




void HairSentinel::Update(const RotPerFrame& frame)
{
	// compute per frame information
	const CMatrix& m = m_pSrcTransform->GetWorldMatrix();
	m_perFrameInfo[frame[m_iNbFrame-1]] = PerFrameInfo(m);

	// set desination matrix
	m_pDestTransform->SetLocalMatrix(m_pSrcTransform->GetLocalMatrix());
}

void HairSentinel::HighFreqUpdate(const RotPerFrame& frame, const RotPerSubFrame& subframe,
	const ChainRessource::RotIndex& p, float fDelta, float fLerp,
	const HairStyleFromWelder* pDefWelder, const HairStyleFromMaya* pMayaDef)
{
	m_chainElem.GetExtremity().m_position=GetWorldSpacePosition(frame,fLerp);
	m_chainElem.GetExtremity().m_speed = GetWorldSpaceSpeed(frame,fLerp,fDelta);
	m_chainElem.GetExtremity().UpdateAcceleration(
		GetWorldSpaceAcceleration(frame,fLerp,fDelta),pDefWelder->m_fAccelerationEcho);

	m_chainElem.UpdateSentinel(p, fDelta, pDefWelder, pMayaDef);

	// quat diff
	CQuat current = CQuat::Slerp(
		m_perFrameInfo[frame[m_iNbFrame-1]].m_quat,
		m_perFrameInfo[frame[m_iNbFrame-2]].m_quat,
		fLerp);
	m_perSubframeInfo[subframe[m_iNbSubframe-1]] = PerSubFrameInfo(current);
}


// get position
CVector HairSentinel::GetWorldSpacePosition(const RotPerFrame& frame, float fLerp) const
{
	return SimpleFunction::Lerp(
		m_perFrameInfo[frame[m_iNbFrame-2]].m_sim2World[3],
		m_perFrameInfo[frame[m_iNbFrame-1]].m_sim2World[3],
		fLerp);
}

// get position
CVector HairSentinel::GetWorldSpaceSpeed(const RotPerFrame& frame, float fLerp, float fDelta) const
{
	CVector speedLast = m_perFrameInfo[frame[m_iNbFrame-2]].m_sim2World[3]
	-m_perFrameInfo[frame[m_iNbFrame-3]].m_sim2World[3];
	CVector speedCurrent = m_perFrameInfo[frame[m_iNbFrame-1]].m_sim2World[3]
	-m_perFrameInfo[frame[m_iNbFrame-2]].m_sim2World[3];
	return SimpleFunction::Lerp(
		speedLast,
		speedCurrent,
		fLerp) / fDelta;
}

// get position
CVector HairSentinel::GetWorldSpaceAcceleration(const RotPerFrame& frame, float fLerp, float fDelta) const
{
	CVector accelerationLast =
		m_perFrameInfo[frame[m_iNbFrame-2]].m_sim2World[3]
		-2*m_perFrameInfo[frame[m_iNbFrame-3]].m_sim2World[3]
		+m_perFrameInfo[frame[m_iNbFrame-4]].m_sim2World[3];
	CVector accelerationCurrent = 
		m_perFrameInfo[frame[m_iNbFrame-1]].m_sim2World[3]
		-2*m_perFrameInfo[frame[m_iNbFrame-2]].m_sim2World[3]
		+m_perFrameInfo[frame[m_iNbFrame-3]].m_sim2World[3];

	return SimpleFunction::Lerp(
		accelerationLast,
		accelerationCurrent,
		fLerp) / (fDelta*fDelta);
}

CQuat HairSentinel::GetRotationDiff(const RotPerSubFrame& subframe)
{
	CQuat diff = ~(m_perSubframeInfo[subframe[m_iNbSubframe-2]].m_quat)
		* m_perSubframeInfo[subframe[m_iNbSubframe-1]].m_quat;
	return diff;
}












// null constructor
RootTracking::RootTracking(const HairStyleFromMaya* pMayaDef):
m_sentinels(pMayaDef->m_rootDependency.size())
{
	int iCount = 0;
	for(HairStyleFromMaya::RootDependency::const_iterator it = pMayaDef->m_rootDependency.begin();
		it != pMayaDef->m_rootDependency.end();
		it++)
	{
		m_sentinels[iCount].SetDef(*it);
		if((*it)->HasParent())
		{
			for(int iParent = 0 ; iParent < iCount ; iParent++ )
			{
				if(static_cast<const HairRootDef*>(m_sentinels[iCount].m_pDef->GetParentT()) == m_sentinels[iParent].m_pDef)
				{
					m_sentinels[iCount].m_pParent = &m_sentinels[iParent];
					break;
				}
			}
			ntAssert(m_sentinels[iCount].m_pParent);
		}
		iCount++;
	}
}

RootTracking::~RootTracking()
{
	// nothing
}

void RootTracking::DrawDefaultPosition(bool bDrawAxis)
{
#ifndef _GOLD_MASTER
	CDirection dec  = GetDebugWorldSpaceDecalage(ChainRessource::Get().GetGlobalDef().m_fDebugDecalage);
	CMatrix w2w(CONSTRUCT_IDENTITY);
	w2w.SetTranslation(CPoint(dec));
	float fSphereSize = ChainRessource::Get().GetGlobalDef().m_fSphereSize;
	float fAxisSize = ChainRessource::Get().GetGlobalDef().m_fSphereSize*2;

	CMatrix sim2world = GetFirstTransform()->GetWorldMatrix();
	
	for(int iSenti = 0 ; iSenti < m_sentinels.GetSize() ; iSenti++ )
	{
		HairSentinel& elem = m_sentinels[iSenti];
		CMatrix local2world = elem.m_pDef->GetPoseMatrix() * sim2world;
		if(bDrawAxis)
		{
			g_VisualDebug->RenderAxis( local2world * w2w , fAxisSize);
		}
		
		g_VisualDebug->RenderSphere(
			CQuat(CONSTRUCT_IDENTITY),
			local2world.GetTranslation()+dec,
			fSphereSize, DC_BLUE );
		
		if(m_sentinels[iSenti].m_pParent)
		{
			HairSentinel& elemParent = *(m_sentinels[iSenti].m_pParent);
			CMatrix local2worldParent = elemParent.m_pDef->GetPoseMatrix() * sim2world;
			g_VisualDebug->RenderLine(
				local2world.GetTranslation()+dec,
				local2worldParent.GetTranslation()+dec,
				DC_BLUE );
		}
	}
#endif
}

void RootTracking::DrawDebug(bool bDrawAxis)
{
#ifndef _GOLD_MASTER

	CDirection dec  = GetDebugWorldSpaceDecalage(ChainRessource::Get().GetGlobalDef().m_fDebugDecalage);
	CMatrix w2w(CONSTRUCT_IDENTITY);
	w2w.SetTranslation(CPoint(dec));
	float fSphereSize = ChainRessource::Get().GetGlobalDef().m_fSphereSize;
	float fAxisSize = ChainRessource::Get().GetGlobalDef().m_fSphereSize*2;

	for(int iSenti = 0 ; iSenti < m_sentinels.GetSize() ; iSenti++ )
	{
		ChainElem& elem = m_sentinels[iSenti].m_chainElem;
		
		if(bDrawAxis)
		{
			CMatrix world = m_sentinels[iSenti].m_pSrcTransform->GetWorldMatrix();
			CQuat qaux = m_sentinels[iSenti].m_pDef->GetRotateAxis();
			CMatrix maux(~qaux);
			g_VisualDebug->RenderAxis( maux * world * w2w , fAxisSize);
		}
		else
		{
			g_VisualDebug->RenderSphere(
				CQuat(CONSTRUCT_IDENTITY),
				CPoint(elem.GetExtremity().m_position)+dec,
				fSphereSize, DC_BLUE );
		}
		
		if(m_sentinels[iSenti].m_pParent)
		{
			ChainElem& elemParent = m_sentinels[iSenti].m_pParent->m_chainElem;
			g_VisualDebug->RenderLine(
				CPoint(elem.GetExtremity().m_position)+dec,
				CPoint(elemParent.GetExtremity().m_position)+dec,
				DC_BLUE );
		}
	}
#endif
}



// find sentinel
ChainElem* RootTracking::GetSentinel(const CHashedString& id)
{
	for(int iSenti = 0 ; iSenti < m_sentinels.GetSize() ; iSenti++ )
	{
		if(m_sentinels[iSenti].m_pDef->m_hashedid == id)
		{
			return &(m_sentinels[iSenti].m_chainElem);
		}
	}
	return static_cast<ChainElem*>(0);
}
ChainElem* RootTracking::GetSentinel(uint32_t uiId)
{
	ntAssert(uiId<uint32_t(m_sentinels.GetSize()));
	return &(m_sentinels[uiId].m_chainElem);
}








// reset value
void RootTracking::Init(const HairStyleFromMaya* pMayaDef,
	const CHierarchy* pSrcHierarchy, CHierarchy* pDestHierarchy)
{
	UNUSED(pMayaDef);
	
	ntAssert(pSrcHierarchy);
	ntAssert(pDestHierarchy);
	
	m_pSrcHierarchy = pSrcHierarchy;
	m_pDestHierarchy = pDestHierarchy;

	for(int iSenti = 0 ; iSenti < m_sentinels.GetSize() ; iSenti++ )
	{
		const HairRootDef* def = m_sentinels[iSenti].m_pDef;
		Transform* pSrcTrans = m_pSrcHierarchy->GetTransform(def->m_bindid);
		Transform* pDestTrans = m_pDestHierarchy->GetTransform(def->m_hashedid);
		ntAssert(pSrcTrans);
		ntAssert(pDestTrans);
		m_sentinels[iSenti].Init(pSrcTrans,pDestTrans);
	}
}

// reset value
void RootTracking::Reset(const HairStyleFromMaya* pMayaDef, const ChainReconstruction* pHairReconstruction)
{
	m_fCentrifuge = 0.0f;
	
	for(int iSenti = 0 ; iSenti < m_sentinels.GetSize() ; iSenti++ )
	{
		pHairReconstruction->SetInformation( &(m_sentinels[iSenti].m_chainElem) );
		m_sentinels[iSenti].Reset(pMayaDef);
	}
}

// update value
void RootTracking::Update()
{
	m_perFrameIndex++;

	for(int iSenti = 0 ; iSenti < m_sentinels.GetSize() ; iSenti++ )
	{
		m_sentinels[iSenti].Update(m_perFrameIndex);
	}
}

// update value
void RootTracking::HighFreqUpdate(float fLerp, float fDelta, const ChainRessource::RotIndex& p,
	const HairStyleFromWelder* pDef, const HairStyleFromMaya* pMayaDef)
{
	m_perSubFrameIndex++;

	for(int iSenti = 0 ; iSenti < m_sentinels.GetSize() ; iSenti++ )
	{
		m_sentinels[iSenti].HighFreqUpdate(m_perFrameIndex,m_perSubFrameIndex,
			p,fDelta,fLerp,
			pDef,pMayaDef);
	}
	
	ComputeCentrifuge(fDelta);
}


// get stupid debug decalage to see what's happening
void RootTracking::ComputeCentrifuge(float fDelta)
{
	// centrifuge stuff
	CQuat qDiff = m_sentinels[0].GetRotationDiff(m_perSubFrameIndex);

	qDiff.GetAxisAndAngle(m_axisAndAngle.m_axis,m_axisAndAngle.m_fAngle);
	m_axisAndAngle.m_fAngle /= fDelta;

	m_fCentrifuge *= ChainRessource::Get().GetGlobalDef().m_fCentrifugeEcho;
	m_fCentrifuge += m_axisAndAngle.m_fAngle;	
}

// get stupid debug decalage to see what's happening
CDirection RootTracking::GetDebugWorldSpaceDecalage(float fDec)
{
	return GetFirstTransform()->GetWorldMatrix().GetXAxis() * fDec;;
}

// get debug position
CPoint RootTracking::GetDebugWorldSpacePosition()
{
	return GetFirstTransform()->GetWorldMatrix().GetTranslation();
}

// get acceleration coef
float RootTracking::GetKillAccCoef(const HairStyleFromWelder* pDef, float fHighFreqProgress, float fDelta) const
{
	CVector v = m_sentinels[0].GetWorldSpaceSpeed(m_perFrameIndex,fHighFreqProgress,fDelta);
	CVector a = m_sentinels[0].GetWorldSpaceAcceleration(m_perFrameIndex,fHighFreqProgress,fDelta);

	float fKillAccCoefTreshold = pDef->m_fSpeedKillAcceleration / fDelta;

	float fAcceleration = a.Length();
	//float fSpeed = v.Length();
	
	float fKillAccCoef = SmoothStep(fKillAccCoefTreshold,0.0f,fAcceleration);
	return fKillAccCoef;
}

















void ExtremityState::UpdateAcceleration(const CVector& acceleration,float fCoef)
{
	m_acceleration*=fCoef;
	m_acceleration+=acceleration;
}






ChainElem::Dynamic ChainElem::Dynamic::operator * (float fCoef)
{
	return Dynamic(m_worldExtremity*fCoef,m_worldAngles*fCoef);
}
// addition
void ChainElem::Dynamic::operator += (const Dynamic& din)
{
	m_worldExtremity+=din.m_worldExtremity;
	m_worldAngles+=din.m_worldAngles;
}
// substarction
void ChainElem::Dynamic::operator -= (const Dynamic& din)
{
	m_worldExtremity-=din.m_worldExtremity;
	m_worldAngles-=din.m_worldAngles;
}








void ChainElem::UpdateExtremityState(const ChainRessource::RotIndex& p, float fDelta, const HairStyleFromWelder* pDef)
{
	m_extremity.m_position = m_dynamic[p[HAIR_CURRENT]].m_worldExtremity;
	m_extremity.m_speed = (m_dynamic[p[HAIR_CURRENT]].m_worldExtremity-m_dynamic[p[HAIR_BEFORE]].m_worldExtremity) / fDelta;
	//m_extremity.m_acceleration = (m_dynamic[p[HAIR_CURRENT]].m_worldExtremity
	//	- 2*m_dynamic[p[HAIR_BEFORE]].m_worldExtremity
	//	+ m_dynamic[p[HAIR_EVENBEFORE]].m_worldExtremity) / (fDelta * fDelta);
	
	// FIXME, more efficient, use speed differential
	CVector acceleration =
		(m_dynamic[p[HAIR_CURRENT]].m_worldExtremity
		- 2*m_dynamic[p[HAIR_BEFORE]].m_worldExtremity
		+ m_dynamic[p[HAIR_EVENBEFORE]].m_worldExtremity) / (fDelta * fDelta);
	m_extremity.UpdateAcceleration(acceleration, pDef->m_fAccelerationEcho);
}

const ExtremityState& ChainElem::GetRootExtremity()
{
	ntAssert(m_pParent);
	return m_pParent->GetExtremity();
}



// position speed and acceleration of the root
void ChainElem::SetSpring(ClothSpringInstance* pSpring)
{
	ntAssert(pSpring);
	m_pSpring=pSpring;
}

ChainElem::ChainElem()
	:m_pParent(0)
	,m_pSpring(0)
	,m_pExtremityTransform(0)
	,m_pDef(0)
	,m_lastRotationAxis(CONSTRUCT_CLEAR)
	,m_pClothSphere(0)
{
	// nothing
}

void ChainElem::SetExtremityTransform(Transform* pExtTrans)
{
	m_pExtremityTransform=pExtTrans;
}


//--------------------------------------------------
//!
//!	restart simulation.
//!	force simulation to start with the given angle with null
//! speed and acceleration
//!
//--------------------------------------------------

void ChainElem::Reset(const ChainRessource::RotIndex& p, Vec2 angle, int iSphereSize, const HairStyleFromWelder* pDefWelder)
{
	CVector ext = GetRootExtremity().m_position + m_pDef->m_fLength * SphericalCoordinate<Vec2>::GetBaseDir(angle);
	
	m_dynamic = DynamicArray(Dynamic(ext,angle));
	m_extremity = ExtremityState(ext);
	
	SetAxisMatrix(p);
	UpdateWorldMatrix(p,pDefWelder);
	
	m_heuritic.Reset(iSphereSize);
	
	m_correctedPosition.Reset();
}


CVector ChainElem::GetLocalForces(const HairStyleFromWelder* pDefWelder, const ChainRessource::RotIndex& p,
	const CVector& unitGravityForce, const CVector& wind, const CVector& artificial, float fDelta,
	const PerChainInstanceModifier& modifier)
{
	ntAssert(m_pDef);
	CVector localForces(CONSTRUCT_CLEAR);

	// build forces vector
	localForces += artificial;

	// wind forces
	CVector windForce = wind * m_pDef->GetFluid(pDefWelder);
	FRANKHAIRPRINT_FLOAT3("windForce",windForce);
	localForces += windForce;
	
	// fluid forces
	if(pDefWelder->m_bUseFluid)
	{
		// the fluid forces is the one who says that the hair moves like in a fluid
		// this is a simple friction : the fluid force is apposite to the speed
		CVector fluidForce = GetRootExtremity().m_speed * (-m_pDef->GetFluid(pDefWelder));
		FRANKHAIRPRINT_FLOAT3("fluidForce",fluidForce);
		localForces += fluidForce;
	}
	
	// local reference frame forces
	// when the root node is accelerating in one direction, the extremity want to go in the other direction
	if(pDefWelder->m_bUseAcceleration)
	{
		// high speed induce null acceleration propagation
		CVector accForce = GetRootExtremity().m_acceleration * ( -modifier.m_fAccCoef*pDefWelder->m_fAccelerationPropagation*m_pDef->GetWeight(pDefWelder) );
		FRANKHAIRPRINT_FLOAT3("accForce",accForce);
		localForces += accForce;
	}
	
	// gravity
	if(pDefWelder->m_bUseGravity)
	{
		CVector gravityForce = unitGravityForce * m_pDef->GetWeight(pDefWelder);
		FRANKHAIRPRINT_FLOAT3("gravityForce",gravityForce);
		localForces += gravityForce;
	}
	
	// parent stiffneff, make it straight (should add suport for not straight)
	if(pDefWelder->m_bUseParentStiff)
	{
		float fDotTreshold = m_pDef->GetParentStiffTreshold(pDefWelder);
		float fDot = m_lastRotationAxis[0].Dot(m_pParent->m_lastRotationAxis[0]);
		if(fDot<fDotTreshold)
		{
			float fCoef = LinearStep(-1.0f, fDotTreshold, fDot);
			CVector parentForces = m_pDef->GetParentStiff(pDefWelder) * fCoef * m_pParent->m_lastRotationAxis[0];
			FRANKHAIRPRINT_FLOAT3("parentForces",parentForces);
			localForces += parentForces;
		}
	}



	// pose stiffness, try to go back at rest position
	if(pDefWelder->m_bUsePoseStiff)
	{
		//CVector wordDiff = m_pDef->GetWorldPoseExtremity(sim2word)- m_dynamic[p[HAIR_BEFORE]].m_worldExtremity;
		CVector wordDiff = (m_pDef->m_local * m_pParent->m_local2world)[3] - m_dynamic[p[HAIR_BEFORE]].m_worldExtremity;
		CVector poseStiffForce = m_pDef->GetPoseStiff(pDefWelder) * wordDiff;
		localForces += poseStiffForce;
	}

	if(m_pSpring && pDefWelder->m_bUseSpring)
	{
		CVector current = GetDynamic()[p[HAIR_BEFORE]].m_worldExtremity;
		CVector springForce(CONSTRUCT_CLEAR);
		if(m_pSpring->HasPrev())
		{
			CVector forces(m_pSpring->GetPrev()->m_pElem->GetDynamic()[p[HAIR_BEFORE]].m_worldExtremity - current);
			forces.W() = 0.0f;
			
			float fLength = forces.Length();
			float fLengthDiff = fLength - m_pSpring->GetPrevLength();
			
			m_pSpring->m_debugCurrentLength[0] = fLength;
			
			// square: 1 for nornalise diff, one for having relative
			forces *= (pDefWelder->m_fSpringStiffness * fLengthDiff) / (fLength*fLength);
			
			FRANKHAIRPRINT_FLOAT3("auxspringforce",forces);
			springForce += forces;
		}
		if(m_pSpring->HasNext())
		{
			CVector forces(m_pSpring->GetNext()->m_pElem->GetDynamic()[p[HAIR_BEFORE]].m_worldExtremity - current);
			forces.W() = 0.0f;
			
			float fLength = forces.Length();
			float fLengthDiff = fLength - m_pSpring->GetNextLength();
			
			m_pSpring->m_debugCurrentLength[1] = fLength;
			
			// square: 1 for nornalise diff, one for having relative
			forces *= (pDefWelder->m_fSpringStiffness * fLengthDiff) / (fLength*fLength);
			
			FRANKHAIRPRINT_FLOAT3("auxspringforce",forces);
			springForce += forces;
		}
		FRANKHAIRPRINT_FLOAT3("springForce",springForce);
		localForces += springForce;			
	}

	// relax forces with collision stuff
	if(pDefWelder->m_bUseCollisionForce)
	{
		CollisionForce(p, localForces, pDefWelder, fDelta);
	}

	return localForces;
}




	

void ChainElem::Compute(Vec2 torque, const ChainRessource::RotIndex& p,
	const HairStyleFromWelder* pDefWelder, float fDelta, const PerChainInstanceModifier& modifier)
{
	ntAssert(m_pDef);
	
	// FIXME more efficient please
	float fInertiaMoment =  m_pDef->GetInertiaMoment(pDefWelder) * m_pDef->GetWeight(pDefWelder) * m_pDef->m_fLength * m_pDef->m_fLength;
	float fInvInertiaMoment = 1.0f / fInertiaMoment;
	
	FRANKHAIRPRINT_FLOAT2("BEFORE",m_dynamic[p[HAIR_BEFORE]].m_worldAngles);
	FRANKHAIRPRINT_FLOAT2("EVENBEFORE",m_dynamic[p[HAIR_EVENBEFORE]].m_worldAngles);

	// part of the acceleration (the other is what we want to compute)
	Vec2 part1 = 2.0f * m_dynamic[p[HAIR_BEFORE]].m_worldAngles - m_dynamic[p[HAIR_EVENBEFORE]].m_worldAngles;
	FRANKHAIRPRINT_FLOAT2("part1",part1);
	
	// damping:
	//float fPart2Coef = fDelta * m_pDef->GetDamp(pDefWelder) * fInvInertiaMoment;
	float fPart2Coef = fDelta * m_pDef->GetDamp(pDefWelder);
	if(!pDefWelder->m_bUseDamp)
	{
		fPart2Coef = 0.0f;
	}
	Vec2 part2 = - fPart2Coef * (m_dynamic[p[HAIR_BEFORE]].m_worldAngles - m_dynamic[p[HAIR_EVENBEFORE]].m_worldAngles);
	FRANKHAIRPRINT_FLOAT2("part2",part2);
	
	// force torque:
	float fPart3Coef = fDelta * fDelta * fInvInertiaMoment;
	Vec2 part3 = fPart3Coef * torque;
	FRANKHAIRPRINT_FLOAT2("part3",part3);
	
	// compute new angle:
	Vec2 newAngle = part1 + part2 + part3;
	
	// diff
	Vec2 diff = newAngle - m_dynamic[p[HAIR_BEFORE]].m_worldAngles;
	FRANKHAIRPRINT_FLOAT2("diff",diff);
	
	// mega clamp
	if(pDefWelder->m_bUseMegaDamp || modifier.m_bForceUseMegaDamp)
	{
		float fMegaDamp = pDefWelder->m_fMegaDamp;
		if(modifier.m_bForceUseMegaDamp)
		{
			fMegaDamp *= modifier.m_fMegaDampCoef;
		}
		diff *= fMegaDamp;
		FRANKHAIRPRINT_FLOAT2("diff",diff);
	}

	// classic
	if(pDefWelder->m_bUseClipAngle)
	{
		Vec2 diff = newAngle - m_dynamic[p[HAIR_BEFORE]].m_worldAngles;
		diff[0] = Sign(diff[0]) * clamp(abs(diff[0]),0.0f,pDefWelder->m_fClipAngle);
		diff[1] = Sign(diff[1]) * clamp(abs(diff[1]),0.0f,pDefWelder->m_fClipAngle);
		FRANKHAIRPRINT_FLOAT2("diff",diff);
		m_dynamic[p[HAIR_CURRENT]].m_worldAngles = m_dynamic[p[HAIR_BEFORE]].m_worldAngles + diff;
	}

	m_dynamic[p[HAIR_CURRENT]].m_worldAngles = m_dynamic[p[HAIR_BEFORE]].m_worldAngles + diff;
	
	// set m_worldExtremity
	CVector worldDec = SphericalCoordinate<Vec2>::GetBaseDir(m_dynamic[p[HAIR_CURRENT]].m_worldAngles) * m_pDef->m_fLength;
	m_dynamic[p[HAIR_CURRENT]].m_worldExtremity = GetRootExtremity().m_position + worldDec;
}

bool ChainElem::IsNumericOk(const ChainRessource::RotIndex& p)
{
	return (!isnan(m_dynamic[p[HAIR_CURRENT]].m_worldAngles[0]))
		&& (!isnan(m_dynamic[p[HAIR_CURRENT]].m_worldAngles[1]));
}

bool ChainElem::Finalise(const ChainRessource::RotIndex& p,
	const HairStyleFromWelder* pDefWelder, float fDelta)
{
	if(pDefWelder->m_bUsePositionCorrection && m_correctedPosition.IsCollidingAndInside())
	{
		// debug save
		m_lastPosition = m_dynamic[p[HAIR_CURRENT]].m_worldExtremity;
		ApplyDiff(p,m_correctedPosition.m_worldDiff,0.0f,pDefWelder->m_fCollisionCorrectionResize);
	}

	if(pDefWelder->m_bUseClothCollision && m_pClothSphere)
	{
		ClothCollisionCorrection(p,pDefWelder);
	}
	
	// set extremity (with latency) and rotation axis
	UpdateExtremityState(p,fDelta,pDefWelder);
	SetAxisMatrix(p);
	
	//m_heuritic.Update(ChainRessource::Get().GetGlobalDef().m_obDummy.W());
	if(pDefWelder->m_bUseCollisionHeuristic)
	{
		m_heuritic.Update(pDefWelder->m_fCollisionHeuristic);
	}
	
	return IsNumericOk(p);
}


// position correction	
void ChainElem::ClothCollisionCorrection(const ChainRessource::RotIndex& p, const HairStyleFromWelder* pDefWelder)
{
	ChainElem::Dynamic& din = m_dynamic[p[HAIR_CURRENT]];
	CVector spherePos = din.m_worldExtremity * m_pClothSphere->m_world2Sphere;
	FRANKHAIRPRINT_FLOAT3("currentPos",din.m_worldExtremity);
	FRANKHAIRPRINT_MATRIX("m_world2Sphere",m_pClothSphere->m_world2Sphere);
	spherePos.W() = 0.0f;
	FRANKHAIRPRINT_FLOAT3("spherePos",spherePos);
	
	float fSqNormCoef = pDefWelder->m_fClothCollision * pDefWelder->m_fClothCollision;
	float fSqLength = spherePos.LengthSquared() / fSqNormCoef;
	if(fSqLength>1.0f)
	{
		float fLength = sqrt(fSqLength);
		FRANKHAIRPRINT_FLOAT("fLength",fLength);
		spherePos *= pDefWelder->m_fClothCollision / fLength;
		spherePos.W() = 1.0f;
		CVector worldPos = spherePos * m_pClothSphere->m_sphere2Word;
		CVector worldDiff = worldPos - din.m_worldExtremity;
		FRANKHAIRPRINT_FLOAT3("worldDiff",worldDiff);
		ApplyDiff(p,worldDiff,0.0f,pDefWelder->m_fClothCollisionResize);
	}
}


// position correction	
void ChainElem::CollisionForce(const ChainRessource::RotIndex& p, CVector& forces, const HairStyleFromWelder* pDefWelder, float fDelta)
{
	if(!m_correctedPosition.IsColliding())
	{
		return;
	}
	
	///////////////////////////////////////////////////
	// Group unconstrained
	float fConstrainedDot = forces.Dot(m_correctedPosition.m_worldNorm);
	CVector unconstrainedForce = forces - fConstrainedDot * m_correctedPosition.m_worldNorm;
	
	// how much we want to decrease unconstrained (perpendicular)
	float fUnconstrainedCoef = Lerp(1.0f, pDefWelder->m_fUnconstrainedCoef,1.0f-m_correctedPosition.m_fOutCoef);
	unconstrainedForce *= fUnconstrainedCoef;

	// End of Group unconstrained
	///////////////////////////////////////////////////
	
	
	///////////////////////////////////////////////////
	// Group constrained

	// radial speed (how much the node is going directly into the sphere)
	CVector speed = (GetDynamic()[p[HairConstant::HAIR_BEFORE]].m_worldExtremity
		- GetDynamic()[p[HairConstant::HAIR_EVENBEFORE]].m_worldExtremity) / fDelta;
	speed.W() = 0.0f;
	float fRadialSpeed = m_correctedPosition.m_worldNorm.Dot(speed);
	
	float fPositionCorrection = pDefWelder->m_fPositionCorrection;
	
	// speed corection
	float fSpeedCorrection = fRadialSpeed * pDefWelder->m_fSpeedCorrection;
	
	// contrained 
	//float fTotal = (1.0f-m_correctedPosition.m_fOutCoef) * (fPositionCorrection + fSpeedCorrection);
	
	// replace constraint force in more efficient than actual one
	//CVector constraintForce = SimpleFunction::Sign(fConstrainedDot) * max(fTotal,abs(fConstrainedDot)) * m_correctedPosition.m_worldNorm;
	CVector constraintForce = Lerp(fPositionCorrection + fSpeedCorrection,fConstrainedDot,m_correctedPosition.m_fOutCoef)
		* m_correctedPosition.m_worldNorm;
	// End of Group constrained
	///////////////////////////////////////////////////
	
	//////////////////////////////
	// sum and set
	forces = unconstrainedForce + constraintForce;
	//forces = unconstrainedForce;
}


void ChainElem::DrawDebugCollision(const ChainRessource::RotIndex& p, const HairStyleFromWelder* pDefWelder)
{	
#ifndef _GOLD_MASTER

	if(!m_correctedPosition.IsColliding() || m_correctedPosition.IsOutside())
	{
		return;
	}

	CVector current = m_dynamic[p[HAIR_CURRENT]].m_worldExtremity;

	CVector beforeResized = m_lastPosition + m_correctedPosition.m_worldDiff;
	CVector before = m_lastPosition;
	
	if(ChainRessource::Get().GetGlobalDef().m_bDrawDummy2)
	{
		uint8_t iIntensity = (m_correctedPosition.m_list.size()>1)?150:250;

		//g_VisualDebug->RenderLine(CPoint(before),CPoint(beforeResized),yellow(iIntensity));
		g_VisualDebug->RenderLine(CPoint(current),CPoint(current+m_correctedPosition.m_worldNorm),yellow(iIntensity));
		
		g_VisualDebug->RenderSphere(CQuat(CONSTRUCT_IDENTITY),CPoint(before),
			ChainRessource::Get().GetGlobalDef().m_fSphereSize, yellow(iIntensity/3));
		g_VisualDebug->RenderSphere(CQuat(CONSTRUCT_IDENTITY),CPoint(beforeResized),
			ChainRessource::Get().GetGlobalDef().m_fSphereSize, yellow(iIntensity/2));
		g_VisualDebug->RenderSphere(CQuat(CONSTRUCT_IDENTITY),CPoint(current),
			ChainRessource::Get().GetGlobalDef().m_fSphereSize, yellow(iIntensity));
	}
		
	// draw all small decalage
	if(ChainRessource::Get().GetGlobalDef().m_bDrawDummy1)
	{
		for(CorrectedPosition::List::iterator it = m_correctedPosition.m_list.begin();
			it != m_correctedPosition.m_list.end();
			it++)
		{
			it->DrawDebug(before, pDefWelder);
		}
	}
#endif
}


namespace 
{
	inline Vec2 AngleMod(Vec2 v)
	{
		static const float INVCOEF = 1.0f / (2.0f * PI);
		static const float COEF = 2.0f * PI;
		
		Vec2 res = v;
		res[0] *= INVCOEF;
		res[1] *= INVCOEF;
		res[0] = floor(res[0]+0.5f);
		res[1] = floor(res[1]+0.5f);
		res[0] *= COEF;
		res[1] *= COEF;
		
		return res;
	}
} // end of namespace 

void ChainElem::SetChainReconstruction(ChainReconstructionInfo* pt)
{
	m_pChainReconstruction.Reset(pt);
}


// position correction	
void ChainElem::ApplyDiff(const ChainRessource::RotIndex& p, const CVector& worldDiff,
	float fKillAcc, float fResize)
{
	// compute new position
	CVector newPos = m_dynamic[p[HAIR_CURRENT]].m_worldExtremity + worldDiff;

	// get parent position
	CVector parenPos = GetParent()->GetDynamic()[p[HAIR_CURRENT]].m_worldExtremity;

	// get diff (the joint vertex)
	CVector parentDiff = newPos - parenPos;

	// spherical coordinate
	float fRadius; Vec2 angle;
	SphericalCoordinate<Vec2>::CartesianToSpherical(CDirection(parentDiff),angle,fRadius);

	// resize parent
	float fNewLength = Lerp(fRadius , m_pDef->m_fLength, fResize);
	parentDiff *= fNewLength / fRadius;

	// new angle
	Vec2 angleDiff = angle-m_dynamic[p[HAIR_BEFORE]].m_worldAngles;

	///////////////////////////////////////////////////
	// stuff not to worry about angle 2pi modulo issue
	angleDiff = AngleMod(angleDiff);
	angle -= angleDiff;
	// yes it's wrong if the hair are diverging. but it will be reset to a better position later
	if(!SphericalCoordinate<Vec2>::IsSphericalAngleBounded(angle-m_dynamic[p[HAIR_BEFORE]].m_worldAngles))
	{
		ntPrintf("hair screw");
	}
	///////////////////////////////////////////////////

	// cartesian -> spherical
	m_dynamic[p[HAIR_CURRENT]].m_worldAngles = angle;

	// kill acceleration
	m_dynamic[p[HAIR_BEFORE]].m_worldAngles =
		Lerp(angle,m_dynamic[p[HAIR_BEFORE]].m_worldAngles,1.0f * fKillAcc);
	m_dynamic[p[HAIR_EVENBEFORE]].m_worldAngles =
		Lerp(angle,m_dynamic[p[HAIR_EVENBEFORE]].m_worldAngles,1.0f * fKillAcc * fKillAcc);

	// position
	m_dynamic[p[HAIR_CURRENT]].m_worldExtremity = parenPos + parentDiff;

	FRANKHAIRPRINT_FLOAT2("angleDiff",angleDiff);
	FRANKHAIRPRINT_FLOAT3("positionDiff",parentDiff);
}


void ChainElem::RenormaliseAngle(const ChainRessource::RotIndex& p)
{
	Vec2 angles = m_dynamic[p[HAIR_CURRENT]].m_worldAngles;
	Vec2 angleMod = AngleMod(angles);
	for(int iPast = 0 ; iPast < HairConstant::HAIR_MEMORY_SIZE ; iPast++ )
	{
		m_dynamic[iPast].m_worldAngles -= angleMod;
	}
}


void ChainElem::DrawSpringDebug(const ChainRessource::RotIndex& p)
{
#ifndef _GOLD_MASTER
	if(m_pSpring)
	{
		CVector current = GetDynamic()[p[HAIR_CURRENT]].m_worldExtremity;
		
		uint32_t color;
		
		if(m_pSpring->HasPrev())
		{
			CVector prev =  m_pSpring->GetPrev()->m_pElem->GetDynamic()[p[HAIR_CURRENT]].m_worldExtremity;
			color = (m_pSpring->m_debugCurrentLength[0] < m_pSpring->m_length[0])?blue(100):blue(200);
			g_VisualDebug->RenderLine(CPoint(prev),CPoint(current),color);
		}
		
		if(m_pSpring->HasNext())
		{
			CVector next = m_pSpring->GetNext()->m_pElem->GetDynamic()[p[HAIR_CURRENT]].m_worldExtremity;
			color = (m_pSpring->m_debugCurrentLength[1] < m_pSpring->m_length[1])?blue(100):blue(200);
			g_VisualDebug->RenderLine(CPoint(next),CPoint(current),color);
		}
	}
#endif
}

// force -> torque
Vec2 ChainElem::ForceToTorque(const CVector& forces, const ChainRessource::RotIndex& p)
{
	Vec2 res = Vec2(
		forces.Dot(m_lastRotationAxis[1]),
		sin(m_dynamic[p[HAIR_BEFORE]].m_worldAngles[0]) * forces.Dot(m_lastRotationAxis[2]));

	return m_pDef->m_fLength * res;
}


void ChainElem::SetAxisMatrix(const ChainRessource::RotIndex& p)
{
	const Dynamic& dynamic = m_dynamic[p[HAIR_CURRENT]];
	SphericalCoordinate<Vec2>::SetMatrix(m_lastRotationAxis,dynamic.m_worldAngles);
}

//// Get X axis approximation
//CDirection ChainElem::GetXAxisApprox()
//{
//	return m_pParent->m_xAxisApprox;
//}

void ChainElem::UpdateWorldMatrix(const ChainRessource::RotIndex& p, const HairStyleFromWelder* pWelderDef)
{
	CGatso::Start("ChainElem::UpdateWorldMatrix");
	if(m_pDef->GetFreeze())
	{
		m_pChainReconstruction->UpdateWorldMatrixFreeze(this,pWelderDef);
/*		m_local = m_pDef->m_local;
		m_local2world = m_pDef->m_local * m_pParent->m_local2world;
		m_xAxisApprox = (GetMayaDef()->m_extraRotInv * m_local2world).GetXAxis();
*/	}
	else
	{
		m_pChainReconstruction->UpdateWorldMatrix(p,this,pWelderDef);

/*		// get x approximation
		CDirection exaux = GetXAxisApprox();
		
		// get y axis
		CVector root = m_pParent->m_local2world[3];
		CVector extremity = GetExtremityWithLatency(bUseLatency,fLatency,p);
		CDirection ey = CDirection(extremity - root);
		ey *= 1.0f / ey.Length();
		
		//CDirection ey = CDirection(m_lastRotationAxis[0]);
		CDirection ez = exaux.Cross(ey);
		ez *= 1.0f / ez.Length();
		m_xAxisApprox = ey.Cross(ez);
		m_xAxisApprox *= 1.0f / m_xAxisApprox.Length();
		
		// set world rotation
		m_local2world = CMatrix(CONSTRUCT_CLEAR);
		m_local2world.SetXAxis(m_xAxisApprox);
		m_local2world.SetYAxis(ey);
		m_local2world.SetZAxis(ez);
		
		// add extra rotation to match skin stuff (because of jamexport)
		// rotation in maya is [rotateAxis] * [jointOrient]
		// the transform is just [jointOrient]
		// warning: [rotateAxis1] * [jointOrient1] , [rotateAxis2] * [jointOrient2]
		// joint 1: transform = [jointOrient1]
		// joint 2: transform = [jointOrient2] * [rotateAxis1] * [jointOrient1]
		m_local2world = m_pDef->m_extraRot * m_local2world;
		
		// set world translation
		//m_local2world.SetTranslation(CPoint(m_extremity.m_position));
		m_local2world.SetTranslation(CPoint(extremity));
		
		// compute local		
		m_local = m_local2world * m_pParent->m_local2world.GetAffineInverse();
*/	}

	CGatso::Stop("ChainElem::UpdateWorldMatrix");
}

void ChainElem::SetLocalMatrixExtremityTransform()
{
	m_pExtremityTransform->SetLocalMatrix(m_local);
	m_pExtremityTransform->GetWorldMatrix();
}



Vec2 ChainElem::GetAngleFromHierarchy()
{
	CDirection diffWorld = CDirection(GetExtremityTransform()->GetWorldTranslation()-GetRootTransform()->GetWorldTranslation());
	float fRadius; Vec2 sphericalCoord;
	SphericalCoordinate<Vec2>::CartesianToSpherical(diffWorld,sphericalCoord,fRadius);
	return sphericalCoord;
}






Vec2 ChainElem::GetAngleFromPose(const CMatrix& sim2world)
{
	CMatrix local2world = m_pDef->GetPoseMatrix() * sim2world;
	CDirection diffWorld = CDirection(local2world.GetTranslation() - m_pParent->m_local2world.GetTranslation());
	float fRadius; Vec2 sphericalCoord;
	SphericalCoordinate<Vec2>::CartesianToSpherical(diffWorld,sphericalCoord,fRadius);
	return sphericalCoord;
}



CVector ChainElem::GetExtremityWithLatency(bool bUseLatency, float fLatency, const ChainRessource::RotIndex& p)
{
	if(!bUseLatency)
	{
		return m_dynamic[p[HairConstant::HAIR_CURRENT]].m_worldExtremity;
	}
	
	
	if(m_pDef->GetFreeze())
	{
		return m_dynamic[p[HairConstant::HAIR_CURRENT]].m_worldExtremity;
	}
	else
	{
		float fCoef = GetMayaDef()->m_fNormDistanceToRoot * fLatency;
		float fIndex = fCoef;
		int iIndex = static_cast<int>(ceil(fIndex));
		
		CPoint newpoint;
		if(iIndex>=HairConstant::HAIR_MEMORY_SIZE-1)
		{
			// the oldest
			return m_dynamic[p[0]].m_worldExtremity;
		}
		else
		{
			CVector newpoint1 = m_dynamic[p[HairConstant::HAIR_MEMORY_SIZE-1-iIndex]].m_worldExtremity;
			CVector newpoint2 = m_dynamic[p[HairConstant::HAIR_MEMORY_SIZE-2-iIndex]].m_worldExtremity;
			float fInterp = fIndex - ceil(fIndex);
			return SimpleFunction::Lerp(newpoint1,newpoint2,fInterp);
		}
	}
}



void ChainElem::InitSentinel(Transform* pTransform)
{
	m_pExtremityTransform = pTransform;
	m_pDef = 0;
	m_pParent = 0;
	m_local.Clear();
}

void ChainElem::ResetSentinel(const HairStyleFromMaya* m_pMayaDef)
{
	ntAssert(!m_pDef);
	UNUSED(m_pMayaDef);
	
	m_local2world = m_pExtremityTransform->GetWorldMatrix();
	//07/04/2005 13:42:41
//	m_xAxisApprox =  m_local2world.GetXAxis();
	m_pChainReconstruction->UpdateSentinel(this);
	
	// angle
	CDirection diff = CDirection(m_local2world.GetTranslation() - m_pExtremityTransform->GetParent()->GetWorldMatrix().GetTranslation());
	Dynamic din;
	SphericalCoordinate<Vec2>::CartesianToSpherical(diff,din.m_worldAngles);
	
	// position
	din.m_worldExtremity =  m_local2world[3];

	
	// set dynamic and extremity
	m_dynamic = DynamicArray(din);
	m_extremity = ExtremityState(din.m_worldExtremity);
	
	// set rotation matrix
	SphericalCoordinate<Vec2>::SetMatrix(m_lastRotationAxis,din.m_worldAngles);
}

void ChainElem::UpdateSentinel(const ChainRessource::RotIndex& p, float fDelta,
	const HairStyleFromWelder* pDefWelder, const HairStyleFromMaya* m_pMayaDef)
{
	ntAssert(!m_pDef);
	UNUSED(m_pMayaDef);
	
	m_local2world = m_pExtremityTransform->GetWorldMatrix();
	
	// 07/04/2005 13:43:36
	//m_xAxisApprox =  m_local2world.GetXAxis();
	m_pChainReconstruction->UpdateSentinel(this);
	
	// angle (and set dynamic)
	CDirection diff = CDirection(m_local2world.GetTranslation() - m_pExtremityTransform->GetParent()->GetWorldMatrix().GetTranslation());
	SphericalCoordinate<Vec2>::CartesianToSpherical(diff,m_dynamic[p[HAIR_CURRENT]].m_worldAngles);

	// position (and set dynamic)
	m_dynamic[p[HAIR_CURRENT]].m_worldExtremity = m_local2world[3];
	
	// set extremity
	UpdateExtremityState(p,fDelta,pDefWelder);
	
	// set rotation matrix
	SetAxisMatrix(p);
}



void ChainElem::ComputeFreeze(const ChainRessource::RotIndex& p, float fDelta, const HairStyleFromWelder* pDefWelder)
{
	ntAssert(m_pDef);
	
	m_local2world = m_pDef->m_local * m_pParent->m_local2world;
	
	//07/04/2005 13:44:00
	//m_xAxisApprox = m_local2world.GetXAxis();
	
	// angle (and set dynamic)
	CDirection diff = CDirection(m_local2world.GetTranslation() - m_pParent->m_local2world.GetTranslation());
	SphericalCoordinate<Vec2>::CartesianToSpherical(diff,m_dynamic[p[HAIR_CURRENT]].m_worldAngles);

	// position (and set dynamic)
	m_dynamic[p[HAIR_CURRENT]].m_worldExtremity = m_local2world[3];
	
	// set extremity
	UpdateExtremityState(p,fDelta,pDefWelder);

	// set rotation matrix
	SetAxisMatrix(p);
}















PerChainInstanceModifier::PerChainInstanceModifier()
	:m_fPotentialExpulsionCoef(1.0f)
	,m_bForceUseMegaDamp(false)
	,m_fMegaDampCoef(0.5f)
	,m_fAccCoef(1.0f)
{
	
}





HairSpecialStuff::~HairSpecialStuff()
{
	// nothing
}
HairSpecialStuff::HairSpecialStuff()
	:m_pArtificialWind(0)
	,m_pSword(0)
	,m_pFloor(0)
	,m_pForWill(0)
{
	// nothing
}



void HairSpecialStuff::Update(const HairStyleFromWelder* pDefWelder, PerChainInstanceModifier& modifier)
{
	if(m_pFloor && (pDefWelder->m_eArtificialWind == ARTIFICIAL_HAIR_HERO))
	{
		if(m_pFloor->m_bOnTheFloor)
		{
			modifier.m_fPotentialExpulsionCoef = 0.1f;
			//modifier.m_fMegaDampCoef = 1.0f - m_pFloor->GetProgress();
			//modifier.m_bForceUseMegaDamp = true;
		}
		else
		{
			modifier.m_fPotentialExpulsionCoef = 1.0f;
			//modifier.m_bForceUseMegaDamp = false;
		}
	}
}












////////////////////////////////////////////////////
////////////////////////////////////////////////////

void OneChain::CreatePS3Data()
{
	ComputeLocation();
	AllocateDmaMemory();
#ifdef PLATFORM_PS3
	if(ChainRessource::Get().GetBitMask()[ChainRessource::F_USESPU])
	{
		m_pElf = ElfManager::Get().GetProgram("hair_spu_ps3.mod");
	}
	else
	{
		m_pElf = 0;
	}
#endif // PLATFORM_PS3
}
void OneChain::InitPS3Data()
{
	SetStaticInfo();
	UpdateDef();
	InitIterativeData();
	InitSPUDynamic(m_highFreqIndex);
	InitSentinel();

	// e3 hack
	const E3WindDef* pDef = ObjectDatabase::Get().GetPointerFromName<E3WindDef*>("e3windef");
	m_pGlobal->m_e3Hack.m_dummy1=pDef?pDef->m_dummy1:CVector(CONSTRUCT_CLEAR);
}

void OneChain::DestroyPS3Data()
{
	// Delete the memory allocated in AllocateDMAMemory.
	NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_pGlobal );
	m_pGlobal = NULL;
}



void OneChain::ComputeLocation()
{
	// one chain
	{
		// handy
		uint32_t uiNbSentinels = m_rootTracking.GetNbSentinel();
		uint32_t uiNbDynamics = m_dynamicjoints.GetSize();
		uint32_t uiNbSprings = m_pDefMaya->GetNbSpring();
		uint32_t uiNbAnticollision = GetNbAntiCollisionSpheres();

		// memory counter
		uint32_t uiCurrent = 0;
		
		m_uiDmaOneChainLocation[ONECHAIN] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + sizeof(ChainSPU::OneChain),16);

		m_uiDmaOneChainLocation[CHAINELEM_STATIC] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + (uiNbSentinels+uiNbDynamics)*sizeof(ChainSPU::ChainElemStatic),16);

		m_uiDmaOneChainLocation[CHAINELEM_DYNAMIC] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + (uiNbSentinels+uiNbDynamics)*sizeof(ChainSPU::ChainElemDynamic),16);

		m_uiDmaOneChainLocation[CHAINELEM_SENTINEL] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbSentinels*sizeof(ChainSPU::SentinelInfo),16);
	
		m_uiDmaOneChainLocation[CHAINELEM_SPRING] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbSprings*sizeof(ChainSPU::Spring),16);

		m_uiDmaOneChainLocation[CHAINELEM_SPRINGINDEX] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbSprings*2*sizeof(uint16_t),16);

		m_uiDmaOneChainLocation[CHAINELEM_ANTICOLLISION] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbAnticollision*sizeof(ChainSPU::AntiCollisionSphere),16);
		
		for(uint32_t usBuffer = 0 ; usBuffer < ChainSPU::g_usNbBuffer ; ++usBuffer )
		{
			m_uiDmaOneChainLocation[DYNAMIC_0_POS+2*usBuffer] = uiCurrent;
			uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + (uiNbSentinels+uiNbDynamics)*sizeof(CVector),16);
			m_uiDmaOneChainLocation[DYNAMIC_0_ANGLE+2*usBuffer] = uiCurrent;
			uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + (uiNbSentinels+uiNbDynamics)*sizeof(Vec2),16);
		}
	
		m_uiDmaOneChainLocation[ONECHAIN_MEM_SIZE] = uiCurrent;
	}

	
	// spheres
	{
		// handy
		uint32_t uiNbSpheres = GetNbCollisionSpheres();

		// memory counter
		uint32_t uiCurrent = 0;

		m_uiDmaMetaBallsLocation[METABALL] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + sizeof(ChainSPU::MetaBallGroup),16);

		m_uiDmaMetaBallsLocation[METABALL_ARRAY] = uiCurrent;
		uiCurrent = GetNextAlignment<uint32_t>(uiCurrent + uiNbSpheres*sizeof(ChainSPU::MetaBall),16);

		m_uiDmaMetaBallsLocation[METABALL_MEM_SIZE] = uiCurrent;
	}
}

void OneChain::AllocateDmaMemory()
{	
	// one chain
	{
		// total size and allocation
		m_pOneChainDmaMemory.Reset( NT_NEW_CHUNK (Mem::MC_PROCEDURAL) uint8_t[ m_uiDmaOneChainLocation[ONECHAIN_MEM_SIZE] ] );
	
		// chain elem
		m_pOneChain = NT_PLACEMENT_NEW(m_pOneChainDmaMemory.Get() + m_uiDmaOneChainLocation[ONECHAIN]) ChainSPU::OneChain;
		memset(m_pOneChain, 0, sizeof(ChainSPU::OneChain));
		m_pOneChain->m_pChainElemStatics = NT_PLACEMENT_NEW(m_pOneChainDmaMemory.Get() + m_uiDmaOneChainLocation[CHAINELEM_STATIC]) ChainSPU::ChainElemStatic;
		m_pOneChain->m_pChainElemDynamics = NT_PLACEMENT_NEW(m_pOneChainDmaMemory.Get() + m_uiDmaOneChainLocation[CHAINELEM_DYNAMIC]) ChainSPU::ChainElemDynamic;
		m_pOneChain->m_pSentinelInfos = NT_PLACEMENT_NEW(m_pOneChainDmaMemory.Get() + m_uiDmaOneChainLocation[CHAINELEM_SENTINEL]) ChainSPU::SentinelInfo;
		m_pOneChain->m_pSprings = NT_PLACEMENT_NEW(m_pOneChainDmaMemory.Get() + m_uiDmaOneChainLocation[CHAINELEM_SPRING]) ChainSPU::Spring;
		m_pOneChain->m_pSpringIndices = NT_PLACEMENT_NEW(m_pOneChainDmaMemory.Get() + m_uiDmaOneChainLocation[CHAINELEM_SPRINGINDEX]) uint16_t;
		m_pOneChain->m_pAntiCollisionSphere = NT_PLACEMENT_NEW(m_pOneChainDmaMemory.Get() + m_uiDmaOneChainLocation[CHAINELEM_ANTICOLLISION]) ChainSPU::AntiCollisionSphere;
		
		// dynamic
		for(uint32_t usBuffer = 0 ; usBuffer < ChainSPU::g_usNbBuffer ; ++usBuffer )
		{
			m_pOneChain->m_dynamic[usBuffer].m_pPositions = NT_PLACEMENT_NEW(m_pOneChainDmaMemory.Get() + m_uiDmaOneChainLocation[DYNAMIC_0_POS+(2*usBuffer+0)]) CVector;
			m_pOneChain->m_dynamic[usBuffer].m_angles = NT_PLACEMENT_NEW(m_pOneChainDmaMemory.Get() + m_uiDmaOneChainLocation[DYNAMIC_0_ANGLE+(2*usBuffer+0)]) TmpVec2;
		}
	}

	// sphere	
	{
		// total size and allocation
		// @todo needs CScopedArray
		m_pMetaBallsDmaMemory.Reset( NT_NEW_CHUNK (Mem::MC_PROCEDURAL) uint8_t[ m_uiDmaOneChainLocation[METABALL_MEM_SIZE] ] );

		m_pMetaBallGroup = NT_PLACEMENT_NEW(m_pMetaBallsDmaMemory.Get() + m_uiDmaMetaBallsLocation[METABALL]) ChainSPU::MetaBallGroup;
		m_pMetaBallGroup->m_pMetaBalls = NT_PLACEMENT_NEW(m_pMetaBallsDmaMemory.Get() + m_uiDmaMetaBallsLocation[METABALL_ARRAY]) ChainSPU::MetaBall;
	}

	// global
	m_pGlobal = NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) ChainSPU::Global;
}

uint32_t OneChain::GetIndexOf(const ChainElem* pElem)
{
	// is it a dynamic joint
	if( ((&m_dynamicjoints[0])<=(pElem)) && ((pElem)<=(&m_dynamicjoints[m_dynamicjoints.GetSize()-1])) )
	{
		return m_rootTracking.GetNbSentinel() + (reinterpret_cast<uintptr_t>(pElem)-reinterpret_cast<uintptr_t>(&m_dynamicjoints[0]))/sizeof(ChainElem);
	}
	// or a sentinel
	else
	{
		for(uint32_t uiSentinel = 0 ; uiSentinel < uint32_t(m_rootTracking.GetNbSentinel()); ++uiSentinel )
		{
			if(m_rootTracking.GetSentinel(uiSentinel) == pElem)
			{
				return uiSentinel;
			}
		}
		ntError_p(false, ("elem not in chain !"));
		return uint32_t(-1);
	}
}

void OneChain::SetStaticInfo()
{
	// misc
//	m_pOneChain->m_uiFirstTransform = reinterpret_cast<uint32_t>( &m_pHierarchy->GetTransform(0)->GetLocalMatrix() );
//	m_pOneChain->m_uiTranformStride = sizeof(Transform);
	m_pOneChain->m_uiFirstLocalJoint = reinterpret_cast< uint32_t >( m_pHierarchy->m_pJointArray );
	m_pOneChain->m_uiLocalJointStride = sizeof( GpJoint );

	m_pOneChain->m_usNbSpring = uint16_t(m_pDefMaya->GetNbSpring());
	
	m_pOneChain->m_usFirstDynamicElem = uint16_t(m_rootTracking.GetNbSentinel());
	m_pOneChain->m_usNbChainElem = uint16_t(m_pOneChain->m_usFirstDynamicElem + m_dynamicjoints.GetSize());
	m_pOneChain->m_usNbAntiCollisionSphere = uint16_t(GetNbAntiCollisionSpheres());
	
	// set id:
	ChainSPU::ChainElemStatic* pChainElemStatics = m_pOneChain->m_pChainElemStatics.Get();
	for(uint32_t iSentinel = 0 ; iSentinel < m_pOneChain->m_usFirstDynamicElem ; ++iSentinel )
	{
		ChainSPU::ChainElemStatic& dst = pChainElemStatics[iSentinel];
		dst.m_usSelfIndex = uint16_t(iSentinel);
		dst.m_usParentIndex = uint16_t(-1); // invalid
		dst.m_usTransformIndex = uint16_t(-1); // invalid
		//spring
		dst.m_usFirstSpring = uint16_t(-1); // invalid
		dst.m_usNbSpring = 0;
		// anticolision
		dst.m_usAntiColIndex = uint16_t(-1);
	}
	for(uint32_t uiJoint = m_pOneChain->m_usFirstDynamicElem ; uiJoint < m_pOneChain->m_usNbChainElem ; ++uiJoint )
	{
		ChainSPU::ChainElemStatic& dst = pChainElemStatics[uiJoint];
		dst.m_usSelfIndex = uint16_t(uiJoint);
		dst.m_usParentIndex = uint16_t(GetIndexOf(m_dynamicjoints[uiJoint-m_pOneChain->m_usFirstDynamicElem].GetParent()));
		dst.m_usTransformIndex = uint16_t((reinterpret_cast<uint32_t>(m_dynamicjoints[uiJoint-m_pOneChain->m_usFirstDynamicElem].GetExtremityTransform())-reinterpret_cast<uint32_t>(m_pHierarchy->GetTransform(0)))/sizeof(Transform));
		//spring
		dst.m_usFirstSpring = uint16_t(-1); // invalid
		dst.m_usNbSpring = 0;
		// anticolision
		dst.m_usAntiColIndex = uint16_t(-1);
	}

	// collision
	m_pMetaBallGroup->m_usNbMetaBall = uint16_t(GetNbCollisionSpheres());
	ChainSPU::MetaBall* pMetaBalls = m_pMetaBallGroup->m_pMetaBalls.Get();
	for(uint32_t uiSphere = 0 ; uiSphere < m_pMetaBallGroup->m_usNbMetaBall ; ++uiSphere )
	{
		ChainSPU::MetaBall& metaBall = pMetaBalls[uiSphere];
		metaBall.m_def = m_pCollisionSpheres->GetSphere(uiSphere)->GetDef()->GetSpuDef();
	}

	// spring
	{
		ChainSPU::Spring* pSprings = m_pOneChain->m_pSprings.Get();
		uint16_t usCount = 0;

		const HairStyleFromMaya::SpringList& list = m_pDefMaya->m_springList;
		// creating spring circle
		for(HairStyleFromMaya::SpringList::const_iterator it = list.begin();
			it != list.end();
			it++)
		{
			const ClothSpringSetDef& springSetDef = *(*it);
			ntAssert(springSetDef.m_container.size()>0);
			
			// loop in the chain and create spring
			int iLastGameId = GetMayaDef()->GetGameIndexFromMayaIndex(*springSetDef.m_container.begin());
			for(ClothSpringSetDef::Container::const_iterator it = ++springSetDef.m_container.begin();
				it != springSetDef.m_container.end();
				it++)
			{
				pSprings[usCount].m_chainElemIndices[0] = uint16_t(iLastGameId) + m_pOneChain->m_usFirstDynamicElem;
				iLastGameId = GetMayaDef()->GetGameIndexFromMayaIndex(*it);
				pSprings[usCount].m_chainElemIndices[1] = uint16_t(iLastGameId) + m_pOneChain->m_usFirstDynamicElem;
				pSprings[usCount].m_fLength = 0.0f;
				pSprings[usCount].m_fSpringStiffness = m_pDefWelder->m_fSpringStiffness;
				++usCount;
			}
			
			// eventually clsoe the loop
			if( springSetDef.m_bIsCircle )
			{
				pSprings[usCount].m_chainElemIndices[0] = uint16_t(iLastGameId) + m_pOneChain->m_usFirstDynamicElem;
				pSprings[usCount].m_chainElemIndices[1] = uint16_t(GetMayaDef()->GetGameIndexFromMayaIndex(*springSetDef.m_container.begin())) + m_pOneChain->m_usFirstDynamicElem;
				pSprings[usCount].m_fLength = 0.0f;
				pSprings[usCount].m_fSpringStiffness = m_pDefWelder->m_fSpringStiffness;
				++usCount;
			}
		}
		ntAssert(usCount==m_pOneChain->m_usNbSpring);
	}


	// finalise spring
	{
		ChainSPU::Spring* pSprings = m_pOneChain->m_pSprings.Get();
		
		typedef ntstd::List<uint32_t, Mem::MC_PROCEDURAL> SpringPerElemList;
		typedef ntstd::Map<uint32_t,SpringPerElemList, ntstd::less<uint32_t>, Mem::MC_PROCEDURAL > SpringPerElem;
		SpringPerElem springsPerChainElem;
		for(uint32_t uiSpring = 0 ; uiSpring < m_pOneChain->m_usNbSpring ; ++uiSpring )
		{
			// compute length
			pSprings[uiSpring].m_fLength = ClothSpringInstance::DistanceBetweenPose(
				&m_dynamicjoints[pSprings[uiSpring].m_chainElemIndices[0]-m_pOneChain->m_usFirstDynamicElem],
				&m_dynamicjoints[pSprings[uiSpring].m_chainElemIndices[1]-m_pOneChain->m_usFirstDynamicElem]);
			for(uint32_t uiExtremity = 0 ; uiExtremity < 2 ; ++uiExtremity )
			{
				// get the chainelem id of this extrremity
				uint16_t usChainElemIndex = pSprings[uiSpring].m_chainElemIndices[uiExtremity];
				pSprings[uiSpring].m_chainElemIndices[uiExtremity] = usChainElemIndex;
				// add this spring if to the element
				if(springsPerChainElem.find(usChainElemIndex)==springsPerChainElem.end())
				{
					SpringPerElemList l; l.push_back(uiSpring);
					springsPerChainElem[usChainElemIndex]=l;
				}
				else
				{
					springsPerChainElem[usChainElemIndex].push_back(uiSpring);
				}

			}
		}
		// create reference
		uint16_t usCount = 0;
		uint16_t* pSpringIndices = m_pOneChain->m_pSpringIndices.Get();
		for(SpringPerElem::iterator it = springsPerChainElem.begin();
			it != springsPerChainElem.end();
			++it)
		{
			ChainSPU::ChainElemStatic& dst = pChainElemStatics[it->first];
			// frst spring index
			dst.m_usFirstSpring=uint16_t(usCount);
			// then iterative on the registered spring for that element
			dst.m_usNbSpring=uint16_t(it->second.size());
			for(SpringPerElemList::iterator it2 = it->second.begin();
				it2 != it->second.end();
				++it2)
			{
				ntAssert(usCount<m_pOneChain->m_usNbSpring*2);
				pSpringIndices[usCount]= uint16_t(*it2);
				++usCount;
			}
		}
		ntAssert(usCount==m_pOneChain->m_usNbSpring*2);
	}

	// anticollision
	{
		uint16_t usCount = 0;
		for(ClothColprimInstanceSet::Vector::iterator it = m_clothSpheres->m_vector.begin();
			it != m_clothSpheres->m_vector.end();
			it++)
		{
			ClothColprimInstance* pCi = (*it);
			int iGameIndex = m_pDefMaya->GetGameIndexFromMayaIndex(pCi->m_pDef->m_parentId);
			ntAssert(m_dynamicjoints.IsInRange(iGameIndex));
			pChainElemStatics[iGameIndex+m_pOneChain->m_usFirstDynamicElem].m_usAntiColIndex = usCount;
			++usCount;
		}
	}
}

void OneChain::InitIterativeData()
{
	// renormlisartion index (one angle is normalised per frame)
	m_pOneChain->m_usNumRenormalised=0;

	// copy dynamic
	for(uint16_t usBuffer = 0 ; usBuffer < ChainSPU::g_usNbBuffer ; ++usBuffer )
	{
		// buffer id
		uint32_t usSPURotBuffer = uint32_t(m_pOneChain->m_rotationnalIndex[usBuffer]);
		uint32_t usPCRotBuffer = m_highFreqIndex[usBuffer + (HAIR_MEMORY_SIZE - uint32_t(ChainSPU::g_usNbBuffer))];

		// handy pointer
		CVector* pPositions = m_pOneChain->m_dynamic[usSPURotBuffer].m_pPositions.Get();
		TmpVec2* pAngles = m_pOneChain->m_dynamic[usSPURotBuffer].m_angles.Get();

		// sentinel
		for(uint16_t uiJoint = 0 ; uiJoint < m_pOneChain->m_usFirstDynamicElem ; ++uiJoint )
		{
			const ChainElem& elem = *(m_rootTracking.GetSentinel(uiJoint));
			pPositions[uiJoint]=elem.GetDynamic()[usPCRotBuffer].m_worldExtremity;
			pAngles[uiJoint]=TmpVec2(elem.GetDynamic()[usPCRotBuffer].m_worldAngles.begin());
		}
		// other
		for(uint16_t uiJoint = m_pOneChain->m_usFirstDynamicElem ; uiJoint < m_pOneChain->m_usNbChainElem ; ++uiJoint )
		{
			const ChainElem& elem = m_dynamicjoints[uiJoint-m_pOneChain->m_usFirstDynamicElem];
			pPositions[uiJoint]=elem.GetDynamic()[usPCRotBuffer].m_worldExtremity;
			pAngles[uiJoint]=TmpVec2(elem.GetDynamic()[usPCRotBuffer].m_worldAngles.begin());
		}
	}
}

void OneChain::InitSPUDynamic(const ChainRessource::RotIndex& p)
{
	// init ChainSPU::ChainElemDynamic
	ChainSPU::ChainElemDynamic* pChainElemDynamics = m_pOneChain->m_pChainElemDynamics.Get();
	for(uint32_t uiJoint = 0 ; uiJoint < m_pOneChain->m_usFirstDynamicElem ; ++uiJoint )
	{
		ChainSPU::ChainElemDynamic& dst = pChainElemDynamics[uiJoint];
		const ChainElem& elem = *(m_rootTracking.GetSentinel(uiJoint));
		dst.m_collisionForce.SetNotInfluenced(); // waste of bandwith
		dst.m_lastRotationAxis = elem.GetRotationAxisMatrix();
		dst.m_modifiedAcceleration = elem.GetExtremity().m_acceleration;
	}

	for(uint32_t uiJoint = m_pOneChain->m_usFirstDynamicElem ; uiJoint < m_pOneChain->m_usNbChainElem ; ++uiJoint )
	{
		ChainSPU::ChainElemDynamic& dst = pChainElemDynamics[uiJoint];
		const ChainElem& elem = m_dynamicjoints[uiJoint-m_pOneChain->m_usFirstDynamicElem];
		dst.m_collisionForce.SetNotInfluenced();
		SphericalCoordinate<Vec2>::SetMatrix(dst.m_lastRotationAxis,elem.GetDynamic()[p[HAIR_CURRENT]].m_worldAngles);
		dst.m_modifiedAcceleration = CVector(CONSTRUCT_CLEAR);
	}
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////









////////////////////////////////////////////////////
////////////////////////////////////////////////////
//update
void OneChain::UpdateDef()
{
	m_pOneChain->m_def = m_pDefWelder->GetSPUDef();

	ChainSPU::ChainElemStatic* pChainElemStatics = m_pOneChain->m_pChainElemStatics.Get();

	// waste of bandwith (but not too much)
	for(uint32_t uiJoint = 0 ; uiJoint < uint32_t(m_pOneChain->m_usFirstDynamicElem) ; ++uiJoint )
	{
		pChainElemStatics[uiJoint].m_def = HairStyleElemFromMaya::GetDefaultSPUDef();
	}
	for(uint32_t uiJoint = m_pOneChain->m_usFirstDynamicElem ; uiJoint < uint32_t(m_pOneChain->m_usNbChainElem) ; ++uiJoint )
	{
		ntAssert_p(m_dynamicjoints[uiJoint-m_pOneChain->m_usFirstDynamicElem].HasDef(), ("joint without def"));
		pChainElemStatics[uiJoint].m_def = m_dynamicjoints[uiJoint-m_pOneChain->m_usFirstDynamicElem].GetMayaDef()->GetSPUDef(*m_pDefWelder);
	}
}


void OneChain::InitSentinel()
{
	ChainSPU::SentinelInfo* pSentinelInfos = m_pOneChain->m_pSentinelInfos.Get();
	for(uint32_t uiSentinel = 0 ; uiSentinel < m_pOneChain->m_usFirstDynamicElem ; ++uiSentinel )
	{
		ChainSPU::SentinelInfo& dst =  pSentinelInfos[uiSentinel];
		const Transform* pTransform = (m_rootTracking.GetSentinel(uiSentinel))->GetExtremityTransform();
		const CMatrix& tmpMatrix = pTransform->GetWorldMatrix();
		dst.m_current = tmpMatrix[3];
	}
}

void OneChain::UpdateSentinel()
{
	ChainSPU::SentinelInfo* pSentinelInfos = m_pOneChain->m_pSentinelInfos.Get();
	for(uint32_t uiSentinel = 0 ; uiSentinel < m_pOneChain->m_usFirstDynamicElem ; ++uiSentinel )
	{
		ChainSPU::SentinelInfo& dst =  pSentinelInfos[uiSentinel];
		const Transform* pTransform = (m_rootTracking.GetSentinel(uiSentinel))->GetExtremityTransform();
		dst.m_local2world = pTransform->GetWorldMatrix();
		dst.m_before = dst.m_current;
		dst.m_current = dst.m_local2world[3];
		dst.m_parentDiff = CDirection(pTransform->GetWorldTranslation() - pTransform->GetParent()->GetWorldTranslation());
	}
}

void OneChain::UpdateCollisionData()
{
	// collision
	ChainSPU::MetaBall* pMetaBalls = m_pMetaBallGroup->m_pMetaBalls.Get();
	for(uint32_t uiSphere = 0 ; uiSphere < m_pMetaBallGroup->m_usNbMetaBall ; ++uiSphere )
	{
		ChainSPU::MetaBall& metaBall = pMetaBalls[uiSphere];
		metaBall.m_world2Sphere = m_pCollisionSpheres->GetSphere(uiSphere)->GetWorld2Sphere();
	}
}

void OneChain::UpdateAntiCollisionData()
{
	ChainSPU::AntiCollisionSphere* pAntiCollisionSphere = m_pOneChain->m_pAntiCollisionSphere.Get();
	for(uint32_t usAntiCol = 0 ; usAntiCol < m_pOneChain->m_usNbAntiCollisionSphere ; ++usAntiCol )
	{
		ClothColprimInstance* pCi = m_clothSpheres->m_vector[usAntiCol];
		ChainSPU::AntiCollisionSphere& dst = pAntiCollisionSphere[usAntiCol];
		dst.m_sphere2Word = pCi->m_sphere2Word;
		dst.m_world2Sphere = pCi->m_world2Sphere;
	}
}

void OneChain::UpdateSPU(const CVector& wind, const CVector& unitGravityForce)
{
	// Adjust the number of iterations depending on how slow the game is running
	const float	maxTimeStep = 1.f / 60.f;
	float		frameTime = m_timeInfo.GetTimeSpeed();

	int numIterations = (int)(frameTime / maxTimeStep + 0.5f);

	numIterations = numIterations > 0 ? numIterations : 1;

	m_pOneChain->m_def.m_usNbStepPerFrame = (uint16_t) numIterations;
	m_pGlobal->m_fDelta = frameTime / numIterations;

	m_pGlobal->m_fTime = m_timeInfo.GetTime();
	m_pGlobal->m_gravity = unitGravityForce;
	m_pGlobal->m_flags.Set(ChainSPU::Global::DEBUG_PERFORMANCE , ChainRessource::Get().GetBitMask()[ChainRessource::F_SPU_DEBUG_PERFORMANCE]);
	m_pGlobal->m_flags.Set(ChainSPU::Global::DEBUG_BREAKPOINT , ChainRessource::Get().GetBitMask()[ChainRessource::F_SPU_DEBUG_BREAKPOINT]);
	m_pGlobal->m_flags.Set(ChainSPU::Global::DEBUG_BEGINEND , ChainRessource::Get().GetBitMask()[ChainRessource::F_SPU_DEBUG_BEGINEND]);

	m_pOneChain->m_wind = wind;

	// welder not working on PS3
	//UpdateDef();

	UpdateSentinel();
	if(m_pCollisionSpheres)
	{
		UpdateCollisionData();
	}
	if(m_clothSpheres)
	{
		UpdateAntiCollisionData();
	}
}

// reset static value (from the def)
void OneChain::PutDataIntoHierarchySPU()
{
	// WHAT WE REALLY NEED HERE IS A FUNCTION UPDATE THE WHOLE HIERARCHHY !!!!!!!!!!!!!!!!!!
	for(uint32_t iJoint = m_pOneChain->m_usFirstDynamicElem ; iJoint < m_pOneChain->m_usNbChainElem ; ++iJoint )
	{
		// handy
		ChainElem& elem = m_dynamicjoints[iJoint-m_pOneChain->m_usFirstDynamicElem];
		Transform* pExtremityTransform = elem.GetExtremityTransform();
		
		// set and update
		pExtremityTransform->SetLocalMatrix(pExtremityTransform->GetLocalMatrix());
		pExtremityTransform->GetWorldMatrix();
		
	}
}
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////










//
void OneChain::TurnOn()
{
	m_mask.Set(F_ISACTIVE);
}

//
void OneChain::TurnOff()
{
	m_mask.Unset(F_ISACTIVE);
}

//
bool OneChain::Toggle()
{
	if(m_mask.CheckFlag(F_ISACTIVE))
	{
		TurnOff();
	}
	else
	{
		TurnOn();
	}
	return m_mask.CheckFlag(F_ISACTIVE);
}

//! constructor
OneChain::OneChain(CEntity* pSelf,
		const ntstd::String& collisionName,
		const HairStyleFromWelder* pDefWelder,
		const HairStyleFromMaya* pDefMaya)
	:m_pHierarchy(pSelf->GetHierarchy())
	,m_rootTracking(pDefMaya)
	,m_pDefWelder(pDefWelder)
	,m_pDefMaya(pDefMaya)
	,m_dynamicjoints(pDefMaya->GetNbElem())
	,m_pCollisionSpheres(0)
	,m_pSelf(pSelf)
	,m_iSubFrames(0)
	,m_NumRenormalised(0)
{
	// check parent
	ntAssert(pSelf->GetHierarchy());
	ntAssert(m_pSelf->GetParentEntity());
	ntAssert(m_pSelf->GetParentEntity()->GetHierarchy());

	// construct collision data
	InstallCollision(collisionName);

	// get pointer to static origin of hair
	m_rootTracking.Init(pDefMaya, m_pSelf->GetParentEntity()->GetHierarchy(), pSelf->GetHierarchy());
	
	// copy topology of the hierarchy to the local structure
	Link();
	
	m_mask.Set(F_ISACTIVE);
	m_mask.Set(F_NEEDTOGRABDATAFROMPOSE);
	
	// anti collision
	ClothColprimInstanceSet* pCloth = NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) ClothColprimInstanceSet(HierarchyPriorityList(GetParentHierarchy(),GetHierarchy())
		,GetMayaDef(), GetWelderDef());
	m_clothSpheres.Reset(pCloth);
	AssignClothCollision();
	
	// spring
	AssignSpring();
	
	// special per-type stuff
	PerType();
	
	// ps3 stuff
	CreatePS3Data();

	// register with definitions
	Register();
}


// get the number of collision spheres
int OneChain::GetNbCollisionSpheres() const
{
	if(m_pCollisionSpheres)
	{
		return m_pCollisionSpheres->GetSize();
	}
	else
	{
		return 0;
	}
}
// get the number of collision spheres
int OneChain::GetNbAntiCollisionSpheres() const
{
	if(m_clothSpheres)
	{
		return m_clothSpheres->GetSize();
	}
	else
	{
		return 0;
	}
}



void OneChain::InstallCollision(const ntstd::String& name)
{
	if(name.empty())
	{
		return;
	}
#if 1
	CEntity* pHero =  const_cast<CEntity*>(m_pSelf->GetParentEntity());
	const HairSphereSetDef* pDef = ChainRessource::Get().GetCollisionSet(name);
	if(pDef)
	{
		// @todo [scee_st] deleted by a different bit of code...
		m_pCollisionSpheres = NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) CollisionSphereSet(name,pDef,m_pSelf->GetParentEntity());
		pHero->SetCollisionSphereSet(m_pCollisionSpheres);
	}
	else
	{
		user_warn_msg(("CreateComponent_Hair: Bad collision name: %s" , name.c_str() ));
	}
#else
	CEntity* pHero =  const_cast<CEntity*>(m_pSelf->GetParentEntity());
	CAnonymousEntComponent* pAnonymous = pHero->FindAnonComponent(name);
	if(pAnonymous)
	{
		m_pCollisionSpheres = static_cast<CollisionSphereSet*>(pAnonymous);
	}
	else
	{
		const HairSphereSetDef* pDef = ChainRessource::Get().GetCollisionSet(name);
		if(pDef)
		{
			m_pCollisionSpheres = NT_NEW CollisionSphereSet(name,pDef,m_pSelf->GetParentEntity());
			pHero->AddAnonComponentFront(m_pCollisionSpheres);
		}
		else
		{
			user_warn_msg(("CreateComponent_Hair: Bad collision name: %s" , name.c_str() ));
		}
	}
#endif
}

void OneChain::InstallForWill()
{
	ntAssert(!HasIronChain());
	m_special.m_pForWill.Reset(NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) ChainAnimation2Render());
	
	for(int iJoint = 0 ; iJoint < m_dynamicjoints.GetSize() ; iJoint++ )
	{
		ChainElem* pElem = &m_dynamicjoints[iJoint];
		if(pElem->GetMayaDef()->IsLeaf())
		{
			ChainAnimation2Render::ExternalCurve curve;
			while(!pElem->GetMayaDef()->GetFreeze())
			{
				curve.push_back(pElem);
				pElem = pElem->GetParent();
			}
			curve.push_back(pElem);
			m_special.m_pForWill->Add(curve);
		}
	}
	
}

// ALEXEY_TODO : this std::string is fucking rubbish, coming from the horrible anonComponent map.
// Fix the anoncomponents and then clean up what has spread from it
void OneChain::InstallArtificialWind(const ntstd::String& name)
{
#if 1
	CEntity* pHero =  const_cast<CEntity*>(m_pSelf->GetParentEntity());
	ArtificialWind* pAnonymous = pHero->GetArtificialWind();
	if(pAnonymous)
	{
		m_special.m_pArtificialWind = pAnonymous;
	}
	else
	{
		const ArtificialWindCoef* pDef =
			ObjectDatabase::Get().GetPointerFromName<ArtificialWindCoef*>(CHashedString(name));
		if(pDef)
		{
			ArtificialWind* pArtificialWind = NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) ArtificialWind(name, m_rootTracking.GetFirstTransform(), pDef);
			pHero->SetArtificialWind(pArtificialWind);
			m_special.m_pArtificialWind = pArtificialWind;
		}
		else
		{
			user_warn_msg(( "Cannot find artificial wind: %s", name.c_str()));
		}
	}
#else
	CEntity* pHero =  const_cast<CEntity*>(m_pSelf->GetParentEntity());
	CAnonymousEntComponent* pAnonymous = pHero->FindAnonComponent(name);
	if(pAnonymous)
	{
		m_special.m_pArtificialWind = static_cast<ArtificialWind*>(pAnonymous);
	}
	else
	{
		const ArtificialWindCoef* pDef =
			ObjectDatabase::Get().GetPointerFromName<ArtificialWindCoef*>(CHashedString(name));
		if(pDef)
		{
			ArtificialWind* pArtificialWind = NT_NEW ArtificialWind(name, m_rootTracking.GetFirstTransform(), pDef);
			pHero->AddAnonComponent(pArtificialWind);
			m_special.m_pArtificialWind = pArtificialWind;
		}
		else
		{
			user_warn_msg(( "Cannot find artificial wind: %s", name.c_str()));
		}
	}
#endif
}

void OneChain::InstallSleeveShrinker()
{
	SleeveStuff* pSleeve = NT_NEW SleeveStuff(
		m_pSelf->GetParentEntity()->GetHierarchy(),
		m_special.m_pSword);
	m_clothSpheres->m_pShrinker.Reset(pSleeve);
}

void OneChain::InstallSwordCollision(const ntstd::String& name, bool bInsertIntoCollision)
{
	UNUSED(name);
	UNUSED(bInsertIntoCollision);
#if 1
	// Now Fixed...
	// It wasn't that the swords had changed names, they hadn't, rather the basic sword heroine doesn't have two swords
	// at all that affect the hair collision
	Hero* pHero =  const_cast<Hero*>((const Hero*)m_pSelf->GetParentEntity());

	if(!pHero->GetLeftWeapon())
		return;

	CollisionSword* pAnonymous = pHero->GetCollisionSword();
	if(pAnonymous)
	{
		m_special.m_pSword = pAnonymous;
	}
	else
	{
		const SwordCollisionDef* pDef =
			ObjectDatabase::Get().GetPointerFromName<SwordCollisionDef*>(CHashedString(name));
		if(pDef)
		{
			CollisionSword* pCollisionSword = NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) CollisionSword(name,pDef,pHero);
			pHero->SetCollisionSword(pCollisionSword);
			m_special.m_pSword = pCollisionSword;
		}
		else
		{
			user_warn_msg(( "Cannot find sword collision wind: %s", name.c_str()));
		}
	}
	
	if(bInsertIntoCollision && m_special.m_pSword && m_pCollisionSpheres)
	{
		m_pCollisionSpheres->AddSpecialSphere(m_special.m_pSword->GetSphere());
	}
#else
	
	// Now Fixed...
	// It wasn't that the swords had changed names, they hadn't, rather the basic sword heroine doesn't have two swords
	// at all that affect the hair collision
	Hero* pHero =  const_cast<Hero*>((const Hero*)m_pSelf->GetParentEntity());

	if(!pHero->GetLeftWeapon())
		return;

	CAnonymousEntComponent* pAnonymous = pHero->FindAnonComponent(name);
	if(pAnonymous)
	{
		m_special.m_pSword = static_cast<CollisionSword*>(pAnonymous);
	}
	else
	{
		const SwordCollisionDef* pDef =
			ObjectDatabase::Get().GetPointerFromName<SwordCollisionDef*>(CHashedString(name));
		if(pDef)
		{
			CollisionSword* pCollisionSword = NT_NEW CollisionSword(name,pDef,pHero);
			pHero->AddAnonComponent(pCollisionSword);
			m_special.m_pSword = pCollisionSword;
		}
		else
		{
			user_warn_msg(( "Cannot find sword collision wind: %s", name.c_str()));
		}
	}
	
	if(bInsertIntoCollision && m_special.m_pSword && m_pCollisionSpheres)
	{
		m_pCollisionSpheres->AddSpecialSphere(m_special.m_pSword->GetSphere());
	}
#endif
}	

void OneChain::InstallFloorCollision(const ntstd::String& name, Transform* pTransfrom)
{
#if 1
	CEntity* pHero =  const_cast<CEntity*>(m_pSelf->GetParentEntity());
	CollisionFloor* pAnonymous = pHero->GetCollisionFloor();
	if(pAnonymous)
	{
		m_special.m_pFloor = pAnonymous;
	}
	else
	{
		const FloorCollisionDef* pDef = ObjectDatabase::Get().GetPointerFromName<FloorCollisionDef*>(CHashedString(name));
		if(pDef)
		{
			CollisionFloor* pCollisionFloor = NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) CollisionFloor(name, pHero, pDef, pTransfrom);
			pHero->SetCollisionFloor(pCollisionFloor);
			m_special.m_pFloor = pCollisionFloor;
		}
		else
		{
			user_warn_msg(( "Cannot find floor collision wind: %s", name.c_str()));
		}
	}

	if(m_special.m_pFloor && m_pCollisionSpheres)
	{
		m_pCollisionSpheres->AddSpecialSphere(m_special.m_pFloor->GetSphere());
	}
	else
	{
		user_warn_msg(( "no collision instanciated"));
	}
#else
	CEntity* pHero =  const_cast<CEntity*>(m_pSelf->GetParentEntity());
	CAnonymousEntComponent* pAnonymous = pHero->FindAnonComponent(name);
	if(pAnonymous)
	{
		m_special.m_pFloor = static_cast<CollisionFloor*>(pAnonymous);
	}
	else
	{
		const FloorCollisionDef* pDef = ObjectDatabase::Get().GetPointerFromName<FloorCollisionDef*>(CHashedString(name));
		if(pDef)
		{
			CollisionFloor* pCollisionFloor = NT_NEW CollisionFloor(name, pHero, pDef, pTransfrom);
			pHero->AddAnonComponent(pCollisionFloor);
			m_special.m_pFloor = pCollisionFloor;
		}
		else
		{
			user_warn_msg(( "Cannot find floor collision wind: %s", name.c_str()));
		}
	}

	if(m_special.m_pFloor && m_pCollisionSpheres)
	{
		m_pCollisionSpheres->AddSpecialSphere(m_special.m_pFloor->GetSphere());
	}
	else
	{
		user_warn_msg(( "no collision instantiated"));
	}
#endif
}	



void OneChain::PerType()
{
	// per type setting
	switch(m_pDefWelder->m_eArtificialWind)
	{
	case ARTIFICIAL_HAIR_HERO:
		{
			InstallArtificialWind(ntstd::String("artificialwind_default"));
			InstallSwordCollision(ntstd::String("swordCollision_default"),true);			
			InstallFloorCollision(ntstd::String("floorCollision_default"), m_pSelf->GetHierarchy()->GetRootTransform());
			break;
		} 
	case ARTIFICIAL_HAIR_SLEEVE:
		{
			InstallArtificialWind(ntstd::String("artificialwind_default"));
			InstallSwordCollision(ntstd::String("swordCollision_default"),false);
			InstallSleeveShrinker();
			//m_pChainReconstruction.Reset(NT_NEW ChainReconstructionSleeve(m_pHierarchy->GetParent()));
			break;
		}
	case ARTIFICIAL_HAIR_TASSLE:
		{
			InstallArtificialWind(ntstd::String("artificialwind_default"));
			m_pChainReconstruction.Reset(NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) ChainReconstructionTassle());

			InstallFloorCollision(ntstd::String("floorCollision_default"), m_pSelf->GetHierarchy()->GetRootTransform());
			break;
		}
	case ARTIFICIAL_HAIR_CHAIN:
		{
			InstallForWill();
			InstallFloorCollision(ntstd::String("floorCollision_default"), m_pSelf->GetHierarchy()->GetRootTransform());
			break;
		}
	default:
		{
			break;
		}
	}
		
	if(!m_pChainReconstruction)
	{
		m_pChainReconstruction.Reset(NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) ChainReconstructionStd());
	}
	
	for(int iJoint = 0 ; iJoint < m_dynamicjoints.GetSize() ; iJoint++ )
	{
		m_pChainReconstruction->SetInformation(&m_dynamicjoints[iJoint]);
	}
}

// find a chain elem according to his name
void OneChain::AssignSpring()
{
	const HairStyleFromMaya::SpringList& list = m_pDefMaya->m_springList;
	m_clothSpringContainer.reserve(list.size());
	
	// creating spring circle
	for(HairStyleFromMaya::SpringList::const_iterator it = list.begin();
		it != list.end();
		it++)
	{
		m_clothSpringContainer.push_back(ClothSpringSetInstance(*it));
		ClothSpringSetInstance& csi = m_clothSpringContainer[m_clothSpringContainer.size()-1];
		
		// fill one of them with value
		const ClothSpringSetDef::Container& listint = csi.m_pDef->m_container;
		int iCount = 1;
		for(ClothSpringSetDef::Container::const_iterator it = listint.begin();
			it != listint.end();
			it++)
		{
			int iGameId = GetMayaDef()->GetGameIndexFromMayaIndex(*it);
			ChainElem* pElem = &m_dynamicjoints[iGameId];
			csi.m_container[iCount].m_pElem = pElem;
			pElem->SetSpring(&csi.m_container[iCount]);
			iCount++;
		}
		
		if(csi.m_pDef->m_bIsCircle)
		{
			csi.m_container[0] = csi.m_container[csi.m_container.size()-2];
			csi.m_container[csi.m_container.size()-1] = csi.m_container[1];
		}
		
		Pixel2 range(1,csi.m_container.size()-2);
		if(csi.m_pDef->m_bIsCircle)
		{
			range = Pixel2(0,csi.m_container.size()-1);
		}
		
		CMatrix id(CONSTRUCT_IDENTITY);
		for(int iElem = range[0] ; iElem < range[1] ; iElem++ )
		{
			csi.m_container[iElem].ComputeNext();
		}
	}
}

void OneChain::DrawSpringDebug()
{
	for(int iChainElem = 0 ; iChainElem < m_dynamicjoints.GetSize() ; iChainElem++ )
	{
		ChainElem& elem = m_dynamicjoints[iChainElem];
		elem.DrawSpringDebug(m_highFreqIndex);
	}
}

void OneChain::AssignClothCollision()
{
	for(ClothColprimInstanceSet::Vector::iterator it = m_clothSpheres->m_vector.begin();
		it != m_clothSpheres->m_vector.end();
		it++)
	{
		ClothColprimInstance* pCi = (*it);
		int iGameIndex = m_pDefMaya->GetGameIndexFromMayaIndex(pCi->m_pDef->m_parentId);
		ntAssert(m_dynamicjoints.IsInRange(iGameIndex));
		//ChainElem* pElem = Find(pCi->m_pDef->m_parentId);
		//ntAssert(pElem);
		ChainElem* pElem = &m_dynamicjoints[iGameIndex];
		pElem->LinkCloth(pCi);
	}
}


const CHierarchy* OneChain::GetParentHierarchy() const
{
	ntAssert(m_pSelf->GetParentEntity());
	ntAssert(m_pSelf->GetParentEntity()->GetHierarchy());
	return m_pSelf->GetParentEntity()->GetHierarchy();
}

// destructor
OneChain::~OneChain()
{
	// nothing
	UnRegister();

	NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_pGlobal );
	NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_pCollisionSpheres );
}

void OneChain::Register()
{
	ChainRessource::Get().Register(this);
	m_pDefWelder->GetRegister().Register(this);
	m_pDefMaya->GetRegister().Register(this);
}
void OneChain::UnRegister()
{
	ChainRessource::Get().Unregister(this);
	m_pDefWelder->GetRegister().Unregister(this);

	// dont unregister as this object will be released on level shutdown BEFORE we are destroyed
//	m_pDefMaya->GetRegister().Unregister(this);
}

// reset static value (from the def)
void OneChain::GetDataFromHierarchy()
{
	m_rootTracking.Reset(m_pDefMaya,m_pChainReconstruction.Get());
	for(int iChainElem = 0 ; iChainElem < m_dynamicjoints.GetSize() ; iChainElem++ )
	{
		// get angle from hierarchy
		ChainElem& elem = m_dynamicjoints[iChainElem];
		Vec2 angles = elem.GetAngleFromHierarchy();
		
		// reset
		elem.Reset(m_highFreqIndex, angles, GetNbCollisionSpheres(), m_pDefWelder);
	}
}


// reset static value (from the def)
void OneChain::GetDataFromPose()
{
	m_rootTracking.Reset(m_pDefMaya,m_pChainReconstruction.Get());
	for(int iChainElem = 0 ; iChainElem < m_dynamicjoints.GetSize() ; iChainElem++ )
	{
		ChainElem& elem = m_dynamicjoints[iChainElem];
		Vec2 angles = elem.GetAngleFromPose(m_rootTracking.GetFirstTransform()->GetWorldMatrix());
		//angles = Vec2(0.0f,0.0f);
		
		// reset
		elem.Reset(m_highFreqIndex, angles, GetNbCollisionSpheres(), m_pDefWelder);
	}
}



// reset static value (from the def)
void OneChain::PutDataIntoHierarchy()
{
	// loop on all elem
	for(int iChainElem = 0 ; iChainElem < m_dynamicjoints.GetSize() ; iChainElem++ )
	{
		ChainElem& elem = m_dynamicjoints[iChainElem];
		elem.SetLocalMatrixExtremityTransform();
		//elem.GetExtremityTransform()->SetFlags(0);
	}
}


// link style elem and chain elem together using hierarchy
void OneChain::Link()
{
	m_pDefMaya->ResetTmp();
	ntAssert(static_cast<int>(m_pDefMaya->m_jointDependency.size()) < m_pHierarchy->GetTransformCount());
	
	// link chain elem and maya def
	// iterate on def
	int iElem = 0;
	for(HairStyleFromMaya::JointDependency::const_iterator it = m_pDefMaya->m_jointDependency.begin();
		it != m_pDefMaya->m_jointDependency.end();
		it++)
	{
		// get def
		const HairStyleElemFromMaya* pDef = *it;
		
		// get transform according to def name
		Transform* pTransform = m_pHierarchy->GetTransform(pDef->m_name.c_str());
		ntAssert(pTransform);
		
		// set transform in ChainElem
		m_dynamicjoints[iElem].SetExtremityTransform(pTransform);
		
		// record pointer in transform because it's a nice way
		pDef->m_pTmp = &m_dynamicjoints[iElem];
		
		// parent is a joint
		if(pDef->HasParent())
		{
			ntAssert(pDef->GetParent()->m_pTmp);
			m_dynamicjoints[iElem].SetParent(pDef->GetParent()->m_pTmp);
		}
		// parent is a sentinel
		else
		{
			ChainElem* pElem = m_rootTracking.GetSentinel(pDef->m_hashedparent);
			ntAssert(pElem);
			m_dynamicjoints[iElem].SetParent(pElem);
		}
		
		// link ChainElem to 
		m_dynamicjoints[iElem].SetDef(pDef);
		
		// increment counter
		iElem++;
	}
}



CVector OneChain::GetWind()
{
	// the force field is a simple procedural animation system
	CVector perChainWorldWind(CONSTRUCT_CLEAR);
	if(m_pDefWelder->m_bUseWind)
	{
		//const ForceFieldResult& ffr = m_pSelf->GetParentEntity()->GetDynamics()->GetForceResult();
		//perChainWorldWind += ffr.GetWorldForce();
		
		//e3 hack
		if(ForceFieldManager::Exists())
		{
			CPoint wp = m_rootTracking.GetFirstTransform()->GetWorldTranslation();
			perChainWorldWind += ForceFieldManager::Get().GetWorldForce(wp);
		}
		
	}
	// and now artificla wind, which is stuck on the heroine and is suppose to add
	// some "randomness"
	if(m_pDefWelder->m_bUseArtificialWind && m_special.m_pArtificialWind)
	{
		perChainWorldWind += m_special.m_pArtificialWind->GetWorldForce(m_special.m_pArtificialWind->GetWorldPosition());
	}
	// draw wind
	if(ChainRessource::Get().GetGlobalDef().m_bDrawWind)
	{
		DrawWindDebug(perChainWorldWind);
	}
	
	return perChainWorldWind;
}

void OneChain::Update(float fTimeStep)
{
	// return if not active
	if(!m_mask.CheckFlag(F_ISACTIVE))
	{
		return;
	}

	// debug
	static uint32_t uiCount = 0;
	++uiCount;
	FRANKHAIRPRINT_INT("frame", uiCount);
	//FRANKHAIRPRINT_KILL(uiCount<2);
	// debug
	
	// gatso
	CGatso::Start("OneChain::Update");
	
	// time change, bounded to a given interval
	// the physic engine is NOT design to handle very big or very small time step
	Vec2 timeChangeRange(0.01f / 30.0f, 1.0f / 30.0f);
	if(fTimeStep < timeChangeRange[0])
	{
		m_timeInfo.GetMask().Set(TimeInfo::F_PAUSE);
		return;
	}
	else
	{
		m_timeInfo.GetMask().Unset(TimeInfo::F_PAUSE);
		if(fTimeStep > timeChangeRange[1])
		{
			fTimeStep = timeChangeRange[1];
		}
		m_timeInfo.UpdateTimeChange(fTimeStep);
	}
	float fHighFreqDelta = m_timeInfo.GetTimeSpeed() / m_pDefWelder->m_iNbStepPerFrame;

	//updateGatso.Tick(1);

	// a part of the hair hierachy is directly stuck on
	// the parent hierarchy. This is "sentinel".
	// they makes the algorithm a bit clearer by removing
	// a lot of "if"
	// this call insure that the sentinel get properly stuck with
	// their correspondant in the parent hierarchy
	m_rootTracking.Update();


	//updateGatso.Tick(2);

	// here is some kind of late initialisation
	// this is dine here to make sure the animation
	// is properly updated. Not clean, but
	if(m_mask.CheckFlag(F_NEEDTOGRABDATAFROMPOSE))
	{
		GetDataFromPose();
		InitPS3Data();
		m_mask.Unset(F_NEEDTOGRABDATAFROMPOSE);
	}
	

	// get the wind from the force field
	CVector perChainWorldWind = GetWind();
	
	// gravity
	CVector unitGravityForce = ChainRessource::Get().GetGlobalDef().m_obGravitation;
	
	//updateGatso.Tick(3);

	// update anti-collision sphere
	// this is usually used with cloth
	// having that here is pure shit, because the collision system is NOT
	// updated that way, this is bad bad bad
	// the two collision system should be handle by the chain singleton
	// to simplify this fuckink big mess
	// this would also make everybody's life esaier by removing the anonimous component
	// but them, all the chain-related anonymous component must be wiped out
	// another way of doing that nicely would be to have a hard-coded hair entity class
	m_clothSpheres->Update();
	
	//updateGatso.Tick(4);

	// the "special" is containing all the extra code used for different purpose.
	m_special.Update(m_pDefWelder, m_modifier);
	
	// the chain simulation is only a joint 1D joint system, no 3D frame involve
	// so at the end of the time-step, the simulation reconstruct the 3d frame using
	// some automatic rule. This rule can require some per-frame information
	// these kind of informatio are querry here:
	// note that this system is heavily relying in inheritance, and must be rethink
	// for PS3...
	m_pChainReconstruction->Update();

	//ntPrintf("Hair frame: %i\n", uiCount);

	if(ChainRessource::Get().GetBitMask()[ChainRessource::F_USESPU])
	{
		//updateGatso.Tick(5);
		UpdateSPU(perChainWorldWind, unitGravityForce);
		//updateGatso.Tick(6);

		// debug
		ChainSPU::SentinelInfo* pSentinelInfos = m_pOneChain->m_pSentinelInfos.Get();
		for(uint32_t uiSentinel = 0 ; uiSentinel < m_pOneChain->m_usFirstDynamicElem ; ++uiSentinel )
		{
			ChainSPU::SentinelInfo& info =  pSentinelInfos[uiSentinel];
			UNUSED(info);
			FRANKHAIRPRINT_MATRIX("SentinelInfo m_local2world",info.m_local2world);
		}
		// debug

		// to force update of the hierarhcy after completion
		Transform* pTransform = m_pHierarchy->GetRootTransform();
		pTransform->SetLocalMatrix(pTransform->GetLocalMatrix());

#ifdef PLATFORM_PS3
		DMABuffer oneChain(m_pOneChain, m_uiDmaOneChainLocation[ONECHAIN_MEM_SIZE]-m_uiDmaOneChainLocation[ONECHAIN]);
		DMABuffer collision(m_pMetaBallGroup, m_uiDmaMetaBallsLocation[METABALL_MEM_SIZE]-m_uiDmaMetaBallsLocation[METABALL]);
		DMABuffer global(m_pGlobal, sizeof(ChainSPU::Global));

		uint32_t uiCurrent = 0;
		SPUTask spu_task( m_pElf );
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputAndOutput, oneChain ), uiCurrent++ ); // to optimize
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, collision ), uiCurrent++ );
		spu_task.SetArgument( SPUArgument( SPUArgument::Mode_InputOnly, global ), uiCurrent++ );
		
		Exec::RunTask( &spu_task );
		//CGatso::Start( "HAIR_SPU_TIME"  );
		//spu_task.StallForJobToFinish();
#else
		ProcessOneChain(m_pOneChain, *m_pMetaBallGroup, *m_pGlobal);
#endif // PLATFORM_PS3		

		pTransform->ForceResynchronise();

		if(m_pOneChain->m_flags[ChainSPU::OneChain::RESET_NEEDED])
		{
			m_pOneChain->m_flags.Set(ChainSPU::OneChain::RESET_NEEDED,false);
			m_mask.Set(F_NEEDTOGRABDATAFROMPOSE,true);
			ntPrintf("hair/cloth went crazy, reseting to default position, count == %i\n", uiCount);
#ifdef PLATFORM_PS3
//			ntBreakpoint();
#endif
		}
		//updateGatso.Tick(7);
	}
	else
	{
		//updateGatso.Tick(5);
		// high frequency update on all element
		// could be the other way around (first loop on elem, subloop on time)
		for(int m_iSubFrames = 0 ; m_iSubFrames < m_pDefWelder->m_iNbStepPerFrame ; m_iSubFrames++ )
		{
			FRANKHAIRPRINT_INT("subframe", m_iSubFrames);
			HighFreqUpdate(fHighFreqDelta, perChainWorldWind, unitGravityForce, m_timeInfo);
		}
		//updateGatso.Tick(6);
		
		// update the hierarchy
		PutDataIntoHierarchy();

		//updateGatso.Tick(7);
	}

	// renormalisation process (a bit of a hack really)
	//this should be done on PS3 ASAP
	if(false)
	{
		m_NumRenormalised = (m_NumRenormalised+1) % m_dynamicjoints.GetSize();
		m_dynamicjoints[m_NumRenormalised].RenormaliseAngle(m_highFreqIndex);
	}

	//updateGatso.Tick(8);
	CGatso::Stop("OneChain::Update");
}


void OneChain::HighFreqUpdate(float fDelta, const CVector& wind, const CVector& unitGravityForce, const TimeInfo& time)
{
	UNUSED(time);
	
	// expulsion coef
	float fPotentialExpulsion = m_pDefWelder->GetPotentialExpulsion(m_modifier.m_fPotentialExpulsionCoef);
	
	// float centrifuge power
	float fCentrifugePower = m_pDefWelder->m_fCentrifuge * LinearStep(
		ChainRessource::Get().GetGlobalDef().m_obDummy.Z(),
		ChainRessource::Get().GetGlobalDef().m_obDummy.W(), m_rootTracking.GetCentrifugePower());
	
	// get float progree within a frame
	float fHighFreqProgress = (m_iSubFrames+1) / static_cast<float>(m_pDefWelder->m_iNbStepPerFrame);
	
	// update high frequency rotationnal index
	m_highFreqIndex++;
	
	// update root tracking
	m_rootTracking.HighFreqUpdate(fHighFreqProgress,fDelta,m_highFreqIndex, m_pDefWelder, m_pDefMaya);
	
	// kill acceleration coef
	m_modifier.m_fAccCoef = m_rootTracking.GetKillAccCoef(m_pDefWelder,fHighFreqProgress,fDelta);
	
	// debug draw
#ifndef _GOLD_MASTER
	if(false)
	{
		g_VisualDebug->RenderLine(m_rootTracking.GetDebugWorldSpacePosition(),
			m_rootTracking.GetDebugWorldSpacePosition()+CDirection(0.0f,fCentrifugePower,0.0f),yellow(20));
	}
#endif

	// loop in all elements
	bool isNumericOk = true;
	for(int iElem = 0 ; iElem < m_dynamicjoints.GetSize() ; iElem++ )
	{
		FRANKHAIRPRINT_INT("elem",iElem+m_rootTracking.GetNbSentinel());

		// convenient
		ChainElem& elem = m_dynamicjoints[iElem];		
		
		// artificial forces
		CVector artificialForce(CONSTRUCT_CLEAR);
		if(m_pDefWelder->m_bUseCentrifuge)
		{
			CGatso::Start("m_bUseCentrifuge");
			CVector centrifuge = elem.GetExtremity().m_position - m_rootTracking.GetFirstTransform()->GetWorldMatrix()[3];
			artificialForce += centrifuge * fCentrifugePower;
			CGatso::Stop("m_bUseCentrifuge");
		}
		
		// pass if elem if freeze
		if(elem.GetMayaDef()->GetFreeze())
		{
			// compute new angles
			elem.ComputeFreeze(m_highFreqIndex, fDelta, m_pDefWelder);
		}
		else
		{
			CVector forces = elem.GetLocalForces(m_pDefWelder, m_highFreqIndex, unitGravityForce, wind,artificialForce, fDelta, m_modifier);
			forces[3]=0.0f;
			FRANKHAIRPRINT_FLOAT4("force",forces);
			//// e3 hack
			//else
			//{
			//	float fWindCoef = 1.0f;
			//	const E3WindDef* pDef = ObjectDatabase::Get().GetPointerFromName<E3WindDef*>("e3windef");
			//	if(pDef)
			//	{
			//		fWindCoef = pDef->m_dummy1.X()
			//			+ pDef->m_dummy1.Y() * cos(pDef->m_dummy1.Z() * elem.GetMayaDef()->m_fNormDistanceToRoot
			//				+ pDef->m_dummy1.W() * time.GetTime());
			//	}
			//	forces = elem.GetLocalForces(m_pDefWelder, m_highFreqIndex, unitGravityForce,
			//		wind * fWindCoef, artificialForce, fDelta, m_modifier);
			//}
		
			
			// debug draw force
#ifndef _GOLD_MASTER
			if(ChainRessource::Get().GetGlobalDef().m_bDrawForce)
			{
				g_VisualDebug->RenderLine(CPoint(elem.GetExtremity().m_position),
					CPoint(elem.GetExtremity().m_position+forces), DC_CYAN );
			}
#endif
			
			// force -> torque
			Vec2 torque = elem.ForceToTorque(forces,m_highFreqIndex);
			FRANKHAIRPRINT_FLOAT2("torque",torque);
			
			// debug draw torque
#ifndef _GOLD_MASTER
			if(ChainRessource::Get().GetGlobalDef().m_bDrawTorque)
			{
				CVector vtorque = elem.GetRotationAxisMatrix()[1] * torque[0] + elem.GetRotationAxisMatrix()[2] * torque[1];
				g_VisualDebug->RenderLine(CPoint(elem.GetExtremity().m_position),
					CPoint(elem.GetExtremity().m_position+vtorque*2.0f), DC_PURPLE );
			}
#endif
			
			// compute new angles
			elem.Compute(torque, m_highFreqIndex, m_pDefWelder, fDelta, m_modifier);
			FRANKHAIRPRINT_FLOAT2("angle",elem.GetDynamic()[m_highFreqIndex[HAIR_CURRENT]].m_worldAngles);
			FRANKHAIRPRINT_FLOAT4("position",elem.GetDynamic()[m_highFreqIndex[HAIR_CURRENT]].m_worldExtremity);
			
			// collision detection
			if(m_pDefWelder->m_bUsePositionCorrection || m_pDefWelder->m_bUseCollisionForce)
			{
				if(m_pCollisionSpheres)
				{
					m_pCollisionSpheres->CollisionDetection(elem, m_highFreqIndex[HAIR_CURRENT],
						m_pDefWelder, fPotentialExpulsion);
				}
			}

			// compute new angles
			isNumericOk = elem.Finalise(m_highFreqIndex, m_pDefWelder, fDelta);

			FRANKHAIRPRINT_FLOAT4("acceleration",elem.GetExtremity().m_acceleration);
			
			// break is something went wrong
			if(!isNumericOk)
			{
				break;
			}
		}
		
		// update world matrix...
		// could do that per frame, but I need the local matrix for frozen joint
		// ... FIXME
		elem.UpdateWorldMatrix(m_highFreqIndex,m_pDefWelder);
		
		FRANKHAIRPRINT_FLOAT2("angleFinal",elem.GetDynamic()[m_highFreqIndex[HAIR_CURRENT]].m_worldAngles);
		FRANKHAIRPRINT_FLOAT4("positionFinal",elem.GetDynamic()[m_highFreqIndex[HAIR_CURRENT]].m_worldExtremity);

		FRANKHAIRPRINT_MATRIX("local",elem.GetLocalMatrix());

		// if is nan
		if(!isNumericOk)
		{
			one_time_assert_p( 0xFA, false, ("%s: Hair went crazy, reseting position", this->GetName().c_str()));
			GetDataFromPose();
			return;
		}
	}
}


void OneChain::DrawDummy()
{
	if(m_special.m_pForWill)
	{
		m_special.m_pForWill->DebugDraw();
	}
}



// draw default position
void OneChain::DrawDefaultPosition()
{
#ifndef _GOLD_MASTER
	m_rootTracking.DrawDefaultPosition(false);
	
	//float fSphereSize = 0.005f;
	//float fAxisSize = 0.05f;
	
	CDirection dec  = m_rootTracking.GetDebugWorldSpaceDecalage(ChainRessource::Get().GetGlobalDef().m_fDebugDecalage);
	CMatrix sim2World = m_rootTracking.GetFirstTransform()->GetWorldMatrix();
	for(int iElem = 0 ; iElem < m_dynamicjoints.GetSize() ; iElem++ )
	{
		ChainElem& elem = m_dynamicjoints[iElem];
		float fSphereSize = ChainRessource::Get().GetGlobalDef().m_bUseSphereSize?
			ChainRessource::Get().GetGlobalDef().m_fSphereSize : elem.GetMayaDef()->GetRadius(m_pDefWelder);
		CMatrix m = elem.GetMayaDef()->GetWorldPose(sim2World);

		CPoint def = CPoint(m[3]);
		g_VisualDebug->RenderSphere(
			CQuat(CONSTRUCT_IDENTITY),
			def+dec,
			fSphereSize, DC_CYAN );
	}
#endif
}


// draw debug collision information
void OneChain::DrawDebugCollision()
{
	for(int iElem = 0 ; iElem < m_dynamicjoints.GetSize() ; iElem++ )
	{
		ChainElem& elem = m_dynamicjoints[iElem];
		elem.DrawDebugCollision(m_highFreqIndex,m_pDefWelder);
	}	
}

void OneChain::DrawDebug(bool bDrawAxis)
{
#ifndef _GOLD_MASTER
	m_rootTracking.DrawDebug(bDrawAxis);

	
	CDirection dec  = m_rootTracking.GetDebugWorldSpaceDecalage(ChainRessource::Get().GetGlobalDef().m_fDebugDecalage);	
	for(int iElem = 0 ; iElem < m_dynamicjoints.GetSize() ; iElem++ )
	{
		ChainElem& elem = m_dynamicjoints[iElem];
		
		CPoint newpoint = CPoint(elem.GetExtremityWithLatency(
			m_pDefWelder->m_bUseLatency, m_pDefWelder->m_fLatency, m_highFreqIndex));
		CPoint oldpoint;
		if(elem.ParentHasDef())
		{
			oldpoint = CPoint(elem.GetParent()->GetExtremityWithLatency(
				m_pDefWelder->m_bUseLatency, m_pDefWelder->m_fLatency, m_highFreqIndex));
		}
		else
		{
			oldpoint = CPoint(elem.GetRootExtremity().m_position);
		}
		g_VisualDebug->RenderLine(oldpoint+dec,newpoint+dec,white());
		
		//float fCoef = elem.GetMayaDef()->m_fDistanceToRoot / m_pDefMaya->m_fMaxDistanceToRoot;
		
		uint32_t color = green();
		if(elem.m_correctedPosition.IsColliding())
		{
			if(elem.m_correctedPosition.IsOutside())
			{
				float fCoef = SimpleFunction::Saturate(elem.m_correctedPosition.m_fOutCoef);
				color = blue(static_cast<uint8_t>(255.0f*fCoef));
			}
			else
			{
				color = red();
			}
		}
	}
#endif
}

	
// draw wind debug
void OneChain::DrawWindDebug(const CVector& wind)
{
#ifndef _GOLD_MASTER
	CPoint p = CPoint(m_rootTracking.GetFirstTransform()->GetWorldTranslation());
	g_VisualDebug->RenderLine(p,p+CPoint(wind),white());
#endif
}

void OneChain::DrawDebugRawHierarchy()
{
#ifndef _GOLD_MASTER
	CDirection dec = m_rootTracking.GetDebugWorldSpaceDecalage(ChainRessource::Get().GetGlobalDef().m_fDebugDecalage);
	g_VisualDebug->RenderHierarchy(*GetHierarchy(),0.05f,dec);
#endif
}

void OneChain::DrawDebugHierarchy()
{	
#ifndef _GOLD_MASTER
	CDirection dec = m_rootTracking.GetDebugWorldSpaceDecalage(ChainRessource::Get().GetGlobalDef().m_fDebugDecalage);
	CMatrix w2w(CONSTRUCT_IDENTITY);
	w2w.SetTranslation(CPoint(dec));

	float fAxisSize = 0.05f;
	
	Pixel2 range(0, m_dynamicjoints.GetSize());
	if(ChainRessource::Get().GetGlobalDef().m_bUseDebugRange)
	{
		range = ChainRessource::Get().GetGlobalDef().GetDebugRangeWithin(range);
	}
	
	for(int iElem = range[0] ; iElem < range[1] ; iElem++ )
	{
		ChainElem& elem = m_dynamicjoints[iElem];
		float fSphereSize = ChainRessource::Get().GetGlobalDef().m_bUseSphereSize?
			ChainRessource::Get().GetGlobalDef().m_fSphereSize : elem.GetMayaDef()->GetRadius(m_pDefWelder);
		
		CPoint old = elem.GetRootTransform()->GetWorldTranslation();
		const CMatrix& world = elem.GetExtremityTransform()->GetWorldMatrix();
		g_VisualDebug->RenderSphere(CQuat(CONSTRUCT_IDENTITY), CPoint(world[3])+dec, fSphereSize, red(100));
		g_VisualDebug->RenderLine(old+dec,CPoint(world[3])+dec,white());
		g_VisualDebug->RenderAxis( elem.GetMayaDef()->m_extraRotInv * world * w2w , fAxisSize);
	}
#endif
}


// reset from def
// only if the iterator are the same
void OneChain::TmpResetStyleData(const HairStyleFromMaya* pDefMaya)
{
	m_pDefMaya = pDefMaya;
	//CAudioManager::Get().PlayDebugSound(1);
}


// draw collision sphere
void OneChain::DrawCollisionSpheres() const
{
	if(m_pCollisionSpheres)
	{
		m_pCollisionSpheres->DrawDebugSpheres();
	}
}



// draw collision sphere
void OneChain::DrawAntiCollisionSpheres() const
{
	m_clothSpheres->DrawDebug(m_pDefWelder->m_fClothCollision);
}




// draw main frame
void OneChain::DrawMainFrame()
{
#ifndef _GOLD_MASTER
	CMatrix m = m_rootTracking.GetFirstTransform()->GetWorldMatrix();
	//m.SetTranslation(m.GetTranslation() + CPoint(GetSentinel().GetRoot(m_highFreqIndex)));
	g_VisualDebug->RenderAxis( m, 1.5f);
#endif
}


ChainAnimation2Render* OneChain::GetIronChain()
{
	ntAssert(HasIronChain());
	return m_special.m_pForWill.Get();
}

bool OneChain::HasIronChain()
{
	return m_special.m_pForWill;
}


#ifdef CHECK_HAIR_NANS

void OneChain::CheckOneChain(ChainSPU::OneChain* chain)
{
	unsigned int numElems = chain -> m_usNbChainElem;
	ChainSPU::ChainElemDynamic* dynamicElems = chain -> m_pChainElemDynamics.Get();  

	CVector* positions[ChainSPU::g_usNbBuffer] = { chain -> m_dynamic[0].m_pPositions.Get(), chain -> m_dynamic[1].m_pPositions.Get(), chain -> m_dynamic[2].m_pPositions.Get()};
	TmpVec2* angles[ChainSPU::g_usNbBuffer] = { chain -> m_dynamic[0].m_angles.Get(), chain -> m_dynamic[1].m_angles.Get(), chain -> m_dynamic[2].m_angles.Get()};

	for (unsigned int elem = 0; elem < numElems; ++ elem )
	{
		CHECK_FOR_NAN(dynamicElems[elem].m_modifiedAcceleration);
		CHECK_FOR_NAN(dynamicElems[elem].m_lastRotationAxis);
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

#endif
