#include "Physics/config.h"
#include "Physics/system.h"

#include "hair/forcefield.h"

#include "physics/havokincludes.h"
#include "physics/havokthreadutils.h"

#include "gfx/simplefunction.h"
#include "anim/transform.h"
#include "game/entitymanager.h"
#include "physics/world.h"
#include "forcefielditem.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkmath/hkmath.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include <hkdynamics/phantom/hkPhantomOverlapListener.h>
#include <hkcollide/shape/convex/hkConvexShape.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>
#endif

#include "Physics/maths_tools.h"

//--------------------------------------------------
//!
//!	small class to listen to event
//! Deano moved here to speed up compile times
//!
//--------------------------------------------------
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
class ForceFieldListener: public hkPhantomOverlapListener 
{
friend class ForceFieldInfluence;

public:
	ForceFieldListener(ForceFieldInfluence* p);
	virtual void collidableAddedCallback(   const hkCollidableAddedEvent& event );
	virtual void collidableRemovedCallback( const hkCollidableRemovedEvent& event );
public:
	ForceFieldInfluence* m_pForceFieldInfluence;
}; // end of class ForceFieldListener


namespace 
{
	template<class EVENT>
	inline CEntity* Collidable2Entity(const EVENT& event)
	{
		hkRigidBody* obRB = 0;
		hkPhantom*	obPH = 0;
		
		obRB = hkGetRigidBody(event.m_collidable);
		
		if( 0 == obRB)
		{
			obPH = hkGetPhantom(event.m_collidable);
		}
			
		if(obRB)
		{
			return static_cast<CEntity*>(obRB->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr());
		}
		else if(obPH)
		{
			return static_cast<CEntity*>(obPH->getProperty(Physics::PROPERTY_ENTITY_PTR).getPtr());
		}
		else
		{
			return static_cast<CEntity*>(0);
		}
	}
} // end of namespace 

ForceFieldListener::ForceFieldListener(ForceFieldInfluence* p):
	m_pForceFieldInfluence(p)
{
	// nothing
}

void ForceFieldListener::collidableAddedCallback(   const hkCollidableAddedEvent& event )
{
	ForceFieldInfluence::Container& cont = m_pForceFieldInfluence->m_influenced;
	CEntity* pEntity = Collidable2Entity(event);
	if(!pEntity)
	{
		return;
	}
	
	cont.insert(pEntity);
}

void ForceFieldListener::collidableRemovedCallback( const hkCollidableRemovedEvent& event )
{
	ForceFieldInfluence::Container& cont = m_pForceFieldInfluence->m_influenced;
	CEntity* pEntity = Collidable2Entity(event);
	if(!pEntity)
	{
		return;
	}
	ForceFieldInfluence::Container::iterator it = cont.find(pEntity);
	if(it!=cont.end())
	{
		cont.erase(it);
	}
}
#endif








#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

ForceFieldInfluence::ForceFieldInfluence(hkConvexShape* pConvexShape,
	const CPoint& worldPosition,const CQuat& worldRotation):
		m_worldPosition(worldPosition)
{
	hkTransform trf(Physics::MathsTools::CQuatTohkQuaternion(worldRotation), Physics::MathsTools::CPointTohkVector(worldPosition));
	m_pPhantom = HK_NEW hkSimpleShapePhantom( pConvexShape, trf, 0 );
	pConvexShape->removeReference();
	
	m_pListener.Reset(NT_NEW ForceFieldListener(this)); 
	m_pPhantom->addPhantomOverlapListener( m_pListener.Get() );
	
	Physics::CPhysicsWorld::Get().AddPhantom(m_pPhantom);
}

ForceFieldInfluence::~ForceFieldInfluence()
{
	m_pPhantom->removePhantomOverlapListener( m_pListener.Get() );
	if(Physics::CPhysicsWorld::Exists())
	{
		Physics::CPhysicsWorld::Get().RemovePhantom(m_pPhantom);
	}
}

void ForceFieldInfluence::SetPosition(const CPoint& worldPosition)
{
	m_worldPosition = worldPosition;
	m_pPhantom->setPosition(Physics::MathsTools::CPointTohkVector(worldPosition));
}


#endif








ForceField::ForceField()
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	:m_pForceFieldInfluence(0)
#endif
{
	// nothing
}


// get the force at the given position
void ForceField::ComputeForAll()
{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	ntAssert(m_pForceFieldInfluence!=0);
	for(ForceFieldInfluence::Container::iterator it = m_pForceFieldInfluence->m_influenced.begin();
		it != m_pForceFieldInfluence->m_influenced.end();
		it++)
	{
		if( (*it)->GetPhysicsSystem() )
		{
			ForceFieldResult& ffr = (*it)->GetPhysicsSystem()->GetForceResult();
			
			if(ffr.NotInfluenced())
			{
				ForceFieldManager::Get().AddInfluenced(*it);
				ffr.SetIsInfluenced();
			}
		
			ffr.AddForce(this,(*it));
		}
	}
#endif
}












void ForceFieldManager::DrawDebug()
{
	
}

ForceFieldManager::ForceFieldManager()
{
	// nothing
}


ForceFieldManager::~ForceFieldManager()
{
	// nothing
}


// update
void ForceFieldManager::ComputeForAll()
{
	SequenceSet::iterator it = m_sequenceSet.begin();
	SequenceSet::iterator end = m_sequenceSet.end();
	for(;it!=end;it++)
	{
		TimeSequence* pTmp = const_cast<TimeSequence*>(it->Get()); //??
		ForceFieldManaged* ff = static_cast<ForceFieldManaged*>(pTmp);
		if(ff->HasInfluenceMask())
		{
			ff->ComputeForAll();
		}
	}
}

void ForceFieldManager::AddE3Wind(const E3WindDef* pDef)
{
	E3Wind* pt = NT_NEW E3Wind(TimeSequence(1000000.0f,TimeSequence::F_PLAYWHENCREATED),pDef);
	this->AddNewSequence(pt);
}

void ForceFieldManager::AddGustOfWind(float fLifeDuration, const CPoint& begin, const CPoint& end, float fPower)
{
	GustOfWind* pt = NT_NEW GustOfWind(
		TimeSequence(fLifeDuration,TimeSequence::F_PLAYWHENCREATED),
		begin,end,fPower);
	this->AddNewSequence(pt);
}

void ForceFieldManager::AddExplosion(float fLifeDuration, const CPoint& center, float fRadius, float fPower)
{
	Explosion* pt = NT_NEW Explosion(
		TimeSequence(fLifeDuration,TimeSequence::F_PLAYWHENCREATED),
		center,fRadius,fPower);
	this->AddNewSequence(pt);
}

void ForceFieldManager::AddInfluenced(CEntity* pEntity)
{
	m_influenced.push_back(pEntity);
}
void ForceFieldManager::ResetForceField()
{
	m_influenced.clear();
	CEntityManager::Get().ResetForceField();
}
void ForceFieldManager::ApplyForce()
{
	for(Container::iterator it = m_influenced.begin();
		it != m_influenced.end();
		it++)
	{
		// apply havok force here
		// (*it)->GetDynamics()->ApplyForceField();
	}
}

// update
CVector ForceFieldManager::GetWorldForce(const CPoint& worldPosition) const
{
	CVector worldForce(CONSTRUCT_CLEAR);
	SequenceSet::const_iterator it = m_sequenceSet.begin();
	SequenceSet::const_iterator end = m_sequenceSet.end();
	for(;it!=end;it++)
	{
		const ForceFieldManaged* ff = static_cast<const ForceFieldManaged*>(it->Get());
		worldForce += ff->GetWorldForce(worldPosition);
	}
	
	return worldForce;
}
