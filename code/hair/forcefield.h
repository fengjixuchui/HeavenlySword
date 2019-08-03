#ifndef _FORCEFIELD_H_
#define _FORCEFIELD_H_

#include "hair/lifeanddeath.h"
#include "hair/forcefieldresult.h"

class CEntity;
class TimeInfo;
class Transform;
class ForceField;


class ForceFieldInfluence;
class hkSimpleShapePhantom;
class ForceFieldListener;
class hkConvexShape;


//--------------------------------------------------
//!
//!	the bounding box of a force field
//! with a list of what it is influenced
//! update throught callback
//!
//--------------------------------------------------
class ForceFieldInfluence
{
public:
	ForceFieldInfluence (hkConvexShape* pConvexShape,
		const CPoint& worldPosition,
		const CQuat& worldRotation = CQuat(CONSTRUCT_IDENTITY));
	~ForceFieldInfluence();
	void SetPosition(const CPoint& worldPosition);
	CPoint GetPosition() const {return m_worldPosition;};
public:
	// event listener
	CScopedPtr<ForceFieldListener, Mem::MC_PROCEDURAL> m_pListener;

	// havok BB
	hkSimpleShapePhantom* m_pPhantom;
	
	// cached center and radius to avoid lookup in havok (am i a bit too much zealous ?)
	CPoint m_worldPosition;

	// container of all the influenced ForceFieldResult, which are a collidable
	typedef ntstd::Set<CEntity*> Container;
	
	// container of all the influenced ForceFieldResult, which are a collidable
	Container m_influenced;
};





//--------------------------------------------------
//!
//!	force field class.
//!	contain the math function which return the force value at one point
//!
//--------------------------------------------------
class ForceField
{
public:
	CScopedPtr<ForceFieldInfluence> m_pForceFieldInfluence;
public:	
	ForceField();
	virtual ~ForceField() {};
	
	// return true if it has a field
	bool HasInfluenceMask() {return m_pForceFieldInfluence;}
	
	// update the force field, is called before any query
	virtual void PerFrameUpdate(const TimeInfo& time) {UNUSED(time);}
	
	// get the force at the given position
	virtual CVector GetWorldForce(const CPoint& worldPosition) const = 0;
	
	// get the force at the given position
	inline CVector ComputeForOne(const CPoint& position) const { return GetWorldForce(position); }
	
	// loop on all the elem in m_pForceFieldInfluence and add the force
	// ntAssert if m_pForceFieldInfluence is null
	void ComputeForAll();

}; // end of class ForceField




//--------------------------------------------------
//!
//!	managed force field class.
//!	for force field with automatic update
//!
//--------------------------------------------------
class ForceFieldManaged: public ForceField, public TimeSequence
{
public:		
	ForceFieldManaged(const TimeSequence& ts): TimeSequence(ts) {};
	virtual ~ForceFieldManaged() {};
public:
	virtual void Next(const TimeInfo& time)
	{
		PerFrameUpdate(time);
	}
}; // end of class ForceField


class E3WindDef;

//--------------------------------------------------
//!
//!	force field manager
//! manage all the force field which are update throught the timemanager
//!
//--------------------------------------------------
class ForceFieldManager: public TimeSequenceManager, public Singleton<ForceFieldManager>
{
public:
	// draw debug
	void DrawDebug();

	// update
	void ComputeForAll();
	
	//! constructor
	ForceFieldManager();
	~ForceFieldManager();
	
	// handy function
	void AddGustOfWind(float fLifeDuration, const CPoint& begin, const CPoint& end, float fPower);
	void AddExplosion(float fLifeDuration, const CPoint& center, float fRadius, float fPower);
	void AddE3Wind(const E3WindDef* pDef);
	
	void AddInfluenced(CEntity*);
	void ResetForceField();
	void ApplyForce();

	// get the force e3 hack
	CVector GetWorldForce(const CPoint& worldPosition) const;

protected:

	// container of all the influenced ForceFieldResult, which are a collidable
	typedef ntstd::List<CEntity*, Mem::MC_PROCEDURAL> Container;
	
	// container of all the influenced ForceFieldResult, which are a collidable
	Container m_influenced;
}; // end of class ForceFieldManager














#endif // end of _FORCEFIELD_H_
