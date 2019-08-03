/***************************************************************************************************
*
*	$Header:: /game/staticentity.h 8     18/08/03 13:51 Dean                                             $
*
*
***************************************************************************************************/

#ifndef	_ENTITY_INVISIBLE_WALL_H
#define	_ENTITY_INVISIBLE_WALL_H

#include "game/entity.h"
#include "physics/havokincludes.h"
#include <hkdynamics/collide/hkCollisionListener.h>
#include "physics/collisionbitfield.h"

class InvisibleWall;
namespace Physics 
{
	
	//! Collision listeners for InvisibleWall it kills passing KO if needed.
	class InvisibleWallsListener : public hkCollisionListener
	{
	public:	
		InvisibleWallsListener(InvisibleWall * wall) : m_wall(wall) {};
		void contactPointAddedCallback (hkContactPointAddedEvent &event); 
		void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event) {};
		void contactPointRemovedCallback( hkContactPointRemovedEvent& event ) {};

	protected:
		InvisibleWall * m_wall; 
	};
}

//! Implementation of invisibleWall. InvisibleWalls should be mainly use to stop characters but let items and other stuff passing
class InvisibleWall : public CEntity
{
	public:
		HAS_INTERFACE( InvisibleWall );
	    
		void	OnPostConstruct			();

		InvisibleWall();
		~InvisibleWall();

		// Ability to turn invisible wall on/off.
		void Activate( void );
		void Deactivate( void );

		int GetCollideWith() const; 
		void SetCollideWith(const int& );

		bool QStopAfterTouchCamera() const;
		void StopAfterTouchCamera(const bool& stop);

		bool QKillPassingKO() const;
		void KillPassingKO(const bool& pass); 

		bool QLetDeadRagdollPass() const;
		void LetDeadRagdollPass(const bool& pass);

		bool QLetSmallKOOfUnimportantPass() const;
		void LetSmallKOOfUnimportantPass(const bool& pass); 

		bool QLetBigKOOfUnimportantPass() const;
		void LetBigKOOfUnimportantPass(const bool& pass); 

		bool QLetSmallKOOfImportantPass() const;
		void LetSmallKOOfImportantPass(const bool& pass);

		bool QLetBigKOOfImportantPass() const;
		void LetBigKOOfImportantPass(const bool& pass);

		bool QOneSide() const;
		void OneSide(const bool& side);

	protected:
		void SetCollisionFilterInfo();
		
		Physics::InvisibleWallsListener * m_listener;
		Physics::AIWallCollisionFlag m_collisionFlags;

		bool m_bInitiallyActive;
};

#endif //_ENTITY_INVISIBLE_WALL_H
