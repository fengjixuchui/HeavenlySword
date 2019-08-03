#ifndef _FORCEFIELDITEM_H_
#define _FORCEFIELDITEM_H_

#include "hair/forcefield.h"
#include "core/explicittemplate.h"
#include "game/anonymouscomponent.h"

class GlobalWind;

//--------------------------------------------------
//!
//!	gust of wind
//!
//--------------------------------------------------

class GustOfWind: public ForceFieldManaged
{
public:
	// power
	float m_fPowerRef;
	CPoint m_begin;
	CPoint m_end;
	CDirection m_halfSize;
	CDirection m_invHalfSize;
	CMatrix m_rotation;
	
	// force
	CVector m_worldForce;
	
public:
	//! constructor
	GustOfWind(const TimeSequence& ts, const CPoint& begin, const CPoint& end, float fPower, const CDirection& halfSize = CDirection(4.0f,3.0f,10.0f));
	
	// update the force field, is called before any query
	virtual void PerFrameUpdate(const TimeInfo& time);
	
	// get the force at the given position
	CVector GetWorldForce(const CPoint& worldPosition) const;
}; // end of class GustOfWind




 


//--------------------------------------------------
//!
//!	gust of wind
//!
//--------------------------------------------------

class Explosion: public ForceFieldManaged
{
public:
	///////////////////////////////////////////////////
	// static
	float m_fPowerRef;
	float m_fRadius;
	Vec2 m_waveWidth;
	// End static
	///////////////////////////////////////////////////
	
	///////////////////////////////////////////////////
	// per frame
	float m_fPower;
	Vec2 m_currentRadius;
	float m_fCurrentWidth;
	// End per frame
	///////////////////////////////////////////////////
	
	
public:
	//! constructor
	Explosion(const TimeSequence& ts, const CPoint& center, float fRadius, float fPower);
	
	// update the force field, is called before any query
	virtual void PerFrameUpdate(const TimeInfo& time);
	
	// get the force at the given position
	CVector GetWorldForce(const CPoint& worldPosition) const;
}; // end of class Explosion






//--------------------------------------------------
//!
//!	gust of wind
//!
//--------------------------------------------------

class E3Wind: public ForceFieldManaged
{
public:
	///////////////////////////////////////////////////
	// static
	const E3WindDef* m_pDef;
	CVector m_currentForce;
	// End per frame
	///////////////////////////////////////////////////
	
	
public:
	//! constructor
	E3Wind(const TimeSequence& ts, const E3WindDef* pDef);
	
	// update the force field, is called before any query
	virtual void PerFrameUpdate(const TimeInfo& time);
	
	// get the force at the given position
	CVector GetWorldForce(const CPoint& worldPosition) const;
}; // end of class Explosion





class ArtificialWindCoef
{
public:
	Array<Vec2,2> m_rightleft;			// [scee_st]: this is now chunked, but doesn't use mem alloc AFAICT
	Array<Vec2,2> m_back;
	float m_fBackShift;
	float m_fPower;

public:
	//! constructor
	ArtificialWindCoef();
}; // end of class ArtificialWindCoef

class ArtificialWindCoefPhase
{
public:
	// use per hero local wind
	ArtificialWindCoefPhase(int iSeed);

public:
	Array<float,2> m_rightleft;
	Array<float,2> m_back;
};

//--------------------------------------------------
//!
//!	artificial wind around the hero
//!
//--------------------------------------------------

class ArtificialWind: public ForceField, public CAnonymousEntComponent
{
protected:
	// hero transfrom
	const Transform* m_pTransform;
	
	// force
	CVector m_worldForce;
	
	// coe
	const ArtificialWindCoef* m_pDef;
	
	// per user part
	ArtificialWindCoefPhase m_phase;

	// time;
	TimeInfo m_info;
public:
	// get position
	CPoint GetWorldPosition();
	
	//! constructor
	ArtificialWind(const ntstd::String& name, const Transform* pTransform, const ArtificialWindCoef* pDef);
	virtual ~ArtificialWind();
	
	// set def
	void SetDef(const ArtificialWindCoef* pDef);
	
	// update the force field, is called before any query
	virtual void PerFrameUpdate(const TimeInfo& time);

	// update call from anonymous, is calling PerFrameUpdate
	virtual void Update( float fTimeStep );

	// get the force at the given position
	CVector GetWorldForce(const CPoint& worldPosition) const;

	mutable GlobalWind* m_pobGlobalWind;
	mutable bool		m_bGlobalWindSearched;

}; // end of class ArtificialWind




#endif // end of _FORCEFIELDITEM_H_
