#include "forcefielditem.h"
#include "physics/config.h"
#include "physics/havokincludes.h"

#include "gfx/simplefunction.h"
#include "anim/transform.h"
#include "core/boostarray.inl"
#include "objectdatabase/dataobject.h"
#include "game/randmanager.h"
#include "tbd/franktmp.h"
#include "hair/winddef.h"

#include "physics/maths_tools.h"
#include "physics/globalwind.h"

using namespace SimpleFunction;
using namespace FrankMisc;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#endif

//! constructor
Explosion::Explosion(const TimeSequence& ts, const CPoint& center, float fRadius, float fPower)
	:ForceFieldManaged(ts)
	,m_fPowerRef(fPower)
	,m_fRadius(fRadius)
	,m_waveWidth(0.2f,0.5f)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	m_pForceFieldInfluence.Reset(NT_NEW ForceFieldInfluence( HK_NEW hkSphereShape(fRadius), center ));
#else
	UNUSED( center );
#endif
}

// update the force field, is called before any query
void Explosion::PerFrameUpdate(const TimeInfo& /*time*/)
{
	m_fCurrentWidth = m_fRadius * Lerp(m_waveWidth[0],m_waveWidth[1],GetProgress());
	m_currentRadius[1] = m_fRadius * (GetProgress()+m_waveWidth[0]) / (1.0f+m_waveWidth[0]);
	m_currentRadius[0] = m_currentRadius[1] - m_fCurrentWidth;
	m_fPower = m_fPowerRef * (1.0f - GetProgress());
}

// get the force at the given position
CVector Explosion::GetWorldForce(const CPoint& worldPosition) const
{
	Vec2 sqCurrentRadius = m_currentRadius*m_currentRadius;
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	CPoint dir = m_pForceFieldInfluence->GetPosition() - worldPosition;
#else
	UNUSED( worldPosition );
	CPoint dir(0.0f, 0.0f, 0.0f );
#endif
	float fSqDist = dir.LengthSquared();
	if( (fSqDist < sqCurrentRadius[0]) || (fSqDist > sqCurrentRadius[1]) )
	{
		return CVector(CONSTRUCT_CLEAR);
	}
	dir/=sqrt(fSqDist);
	float fPower = m_fPower * Saturate(1.0f - GetProgress());
	return fPower * CVector(dir.X(),dir.Y(),dir.Z(),0.0f);
}













namespace 
{
	
	inline float Step(float a, float x)
	{
		return (x<a) ? 0.0f : 1.0f;
	}
	
	inline float PosCos(float a, float x)
	{
		return Step(a,cos(x));
	}
	
	// when brutality ==0, very brtual
	float Impulse(float fX, float fMax,float fEnd)
	{
		float fFloarX = floor(fX);
		float fNormX = fX - fFloarX;
		
		if(fNormX<fMax)
		{
			return SmoothStep(0.0f,fMax,fNormX);
		}
		else if(fNormX<fEnd)
		{
			return 1.0f-SmoothStep(fMax,fEnd,fNormX);
		}
		else
		{
			return 0.0f;
		}
	}

	// when brutality ==0, very brtual
	float Impulse(const CVector& impulse,float fTime)
	{
		return impulse.X() * Impulse(impulse.Y()*fTime, impulse.Z(), impulse.W());
	}

} // end of namespace 

E3Wind::E3Wind(const TimeSequence& ts, const E3WindDef* pDef)
	:ForceFieldManaged(ts)
	,m_pDef(pDef)
	,m_currentForce(CONSTRUCT_CLEAR)
{
	// nothing
}

void E3Wind::PerFrameUpdate(const TimeInfo& time)
{
	float fTime = time.GetTime();
	//float fCoef = (PosCos(m_pDef->m_dummy.X()*fTime)
	//	+ PosCos(m_pDef->m_dummy.Y()*fTime)
	//	+ PosCos(m_pDef->m_dummy.Z()*fTime)) / 3.0f;
	 
	//float fCoef = m_pDef->m_fConstantPower;
	float fCoef = m_pDef->m_fConstantPower*(0.5f+0.5f*sin(0.1f*fTime));
	fCoef += Impulse(m_pDef->m_impulse1,fTime);
	fCoef += Impulse(m_pDef->m_impulse2,fTime);
	fCoef += Impulse(m_pDef->m_impulse3,fTime);
	fCoef += m_pDef->m_fPower;
	
	m_currentForce = CVector(fCoef * m_pDef->m_direction);
	m_currentForce.W() = 0.0f;
}

CVector E3Wind::GetWorldForce(const CPoint& /*worldPosition*/) const
{
	return m_currentForce;
}










//! constructor
GustOfWind::GustOfWind(const TimeSequence& ts, const CPoint& begin, const CPoint& end, float fPower, const CDirection& halfSize)
	:ForceFieldManaged(ts)
	,m_fPowerRef(fPower)
	,m_begin(begin)
	,m_end(end)
	,m_halfSize(halfSize)
	,m_rotation(CONSTRUCT_CLEAR)
{
	CPoint diff = end - begin;
	float fDist = diff.Length();

	CDirection x = CDirection(diff/fDist);
	CDirection yaux = CDirection(0.0f,1.0f,0.0f);
	CDirection z = x.Cross(yaux);
	CDirection y = z.Cross(x);
	m_rotation.SetXAxis(x);
	m_rotation.SetYAxis(y);
	m_rotation.SetZAxis(z);
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	m_pForceFieldInfluence.Reset(NT_NEW ForceFieldInfluence( HK_NEW hkBoxShape(Physics::MathsTools::CDirectionTohkVector(m_halfSize)),begin,CQuat(m_rotation)));
#endif
	m_worldForce = CVector(diff);
	m_worldForce.W() = 0.0f;
	m_worldForce *= fPower / fDist;
	
	m_invHalfSize = CDirection(1.0f / m_halfSize.X(),1.0f / m_halfSize.Y(), 1.0f / m_halfSize.Z());
}

// get the force at the given position
CVector GustOfWind::GetWorldForce(const CPoint& worldPosition) const
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	CDirection diff = CDirection(worldPosition - m_pForceFieldInfluence->GetPosition());
#else
	UNUSED( worldPosition );
	CDirection diff( 0.0f, 0.0f, 0.0f );
#endif
	CDirection localDiff = diff * m_rotation;
	localDiff *= m_invHalfSize;
	float fXCoef =1.0f + sin(3.1415f * (1.0f + localDiff.X()));
	fXCoef *= (1.0f + fXCoef) / 2.0f;
	float fLifeCoef = NormalisedSmoothPeek(0.1f,0.9f,GetProgress());
	return fXCoef * fLifeCoef * m_worldForce;
}


// update the force field, is called before any query
void GustOfWind::PerFrameUpdate(const TimeInfo& /*time*/)
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	m_pForceFieldInfluence->SetPosition(Lerp(m_begin,m_end,GetProgress()));
#endif
}















START_CHUNKED_INTERFACE( ArtificialWindCoef, Mem::MC_PROCEDURAL )
	PUBLISH_VAR_AS(m_rightleft[0][0], lateral1_amplitude)
	PUBLISH_VAR_AS(m_rightleft[0][2], lateral1_frequency)
	
	PUBLISH_VAR_AS(m_rightleft[1][0], lateral2_amplitude)
	PUBLISH_VAR_AS(m_rightleft[1][2], lateral2_frequency)
	
	PUBLISH_VAR_AS(m_back[0][0], back1_amplitude)
	PUBLISH_VAR_AS(m_back[0][2], back1_frequency)
							   
	PUBLISH_VAR_AS(m_back[1][0], back2_amplitude)
	PUBLISH_VAR_AS(m_back[1][2], back2_frequency)
	
	PUBLISH_VAR_AS(m_fBackShift, forceBack)
	PUBLISH_VAR_AS(m_fPower, power)
END_STD_INTERFACE



ArtificialWindCoef::ArtificialWindCoef()
{
	m_rightleft[0]=Vec2(1.0f, 1.5f);
	m_rightleft[1]=Vec2(1.0f, 2.5f);
	m_back[0]=Vec2(0.5f, 1.5f);
	m_back[1]=Vec2(0.5f, 2.5f);
}

	



// use per hero local wind
ArtificialWindCoefPhase::ArtificialWindCoefPhase(int iSeed)
{
	RandManager::ESeed(iSeed);
	m_back[1] = erandf(3.0f );
	m_back[0] = erandf(3.0f );
	m_rightleft[1] = erandf(3.0f );
	m_rightleft[0] = erandf(3.0f );
}




// set def
void ArtificialWind::SetDef(const ArtificialWindCoef* pDef)
{
	m_pDef = pDef;
}

// update call from anonymous, is calling PerFrameUpdate
void ArtificialWind::Update( float fTimeStep )
{
	m_info.UpdateTimeChange(fTimeStep);
	PerFrameUpdate(m_info);
}


//! constructor
ArtificialWind::ArtificialWind(const ntstd::String& name, const Transform* pTransform, const ArtificialWindCoef* pDef)
	:CAnonymousEntComponent(name)
	,m_pTransform(pTransform)
	,m_worldForce(CONSTRUCT_CLEAR)
	,m_phase(GetSeedFromPointer(pTransform))
	,m_pobGlobalWind(0)
	,m_bGlobalWindSearched(false)
{
	SetDef(pDef);
}

ArtificialWind::~ArtificialWind()
{
	// nothing	
}

// update the force field, is called before any query
void ArtificialWind::PerFrameUpdate(const TimeInfo& time)
{
	float fTime = time.GetTime();
	
	float fRightleft=0.0f;
	for(int iCoef = 0 ; iCoef < m_pDef->m_rightleft.size() ; iCoef++ )
	{
		Vec2 m = m_pDef->m_rightleft[iCoef];
		fRightleft += m[0] * sin( m_phase.m_rightleft[iCoef] + m[1] * fTime);
	}

	float fback = m_pDef->m_fBackShift;
	for(int iCoef = 0 ; iCoef < m_pDef->m_back.size() ; iCoef++ )
	{
		Vec2 m = m_pDef->m_back[iCoef];
		fback += m[0]*sin( m_phase.m_back[iCoef] + m[1] * fTime);
	}
	
	CVector rightleft = m_pTransform->GetWorldMatrix()[0] * fRightleft;
	CVector back = - m_pTransform->GetWorldMatrix()[2] * fback;
	
	m_worldForce = m_pDef->m_fPower * (back + rightleft);
	m_worldForce.W() = 0.0f;
}

// get the force at the given position
CVector ArtificialWind::GetWorldForce(const CPoint& worldPosition) const
{
	if (!m_bGlobalWindSearched)
	{
		// This is a quick hack for the Army level. 
		m_pobGlobalWind = ObjectDatabase::Get().GetPointerFromName<GlobalWind*>("Hair_GlobalWind");
		m_bGlobalWindSearched = true;
	}

	if (m_pobGlobalWind)
	{
		CVector	worldForce = m_pobGlobalWind->GetForce(worldPosition);
		return worldForce;
	}
	return m_worldForce;
}



// get position
CPoint ArtificialWind::GetWorldPosition()
{
	return m_pTransform->GetWorldTranslation();
}


