//--------------------------------------------------
//!
//!	\file game/projectilemanager.cpp
//!	Projectile manager code
//!
//--------------------------------------------------


#include "game/projectilemanager.h"
#include "messagehandler.h"
#include "Physics/system.h"
#include "core/timer.h"
#include "game/entity.inl"


//--------------------------------------------------
//!
//!	ProjectileManager::ProjectileManager
//! Constructor
//!
//--------------------------------------------------
ProjectileManager::ProjectileManager()
{

}


//--------------------------------------------------
//!
//!	ProjectileManager::~ProjectileManager
//! Destructor
//!
//--------------------------------------------------
ProjectileManager::~ProjectileManager()
{
	// Clear the projectile list
	m_pProjectileList.clear();
}


//--------------------------------------------------
//!
//!	ProjectileManager::AddProjectileToList
//! Adds a projectile to the list
//!
//--------------------------------------------------
void ProjectileManager::AddProjectileToList(CEntity* pProjectile)
{
	ntAssert(pProjectile);

#ifdef _DEBUG
	// Check it's not already in the list
	ntstd::List<CEntity*>::const_iterator iter;
	ntstd::List<CEntity*>::const_iterator iterEnd = m_pProjectileList.end();

	for (iter = m_pProjectileList.begin(); iter != iterEnd; iter++)
	{
		if ( (*iter) == pProjectile )
		{
			ntAssert_p(false, ("Projectile is already in the list"));
		}
	}
#endif

	//ntPrintf("Adding projectile to list\n");

	// Add it to the list
	m_pProjectileList.push_back(pProjectile);
}


//--------------------------------------------------
//!
//!	ProjectileManager::RemoveProjectileFromList
//! Removes a projectile from the list
//!
//--------------------------------------------------
void ProjectileManager::RemoveProjectileFromList(CEntity* pProjectile)
{
	ntAssert(pProjectile);

	//ntPrintf("Removing projectile from list\n");

	// Remove it from the list
	m_pProjectileList.remove(pProjectile);
}


//--------------------------------------------------
//!
//!	ProjectileManager::SendMessageToProjectilesWithinSphere
//! Gameplay helper function to send a message to all projectiles
//! within a sphere.
//!
//--------------------------------------------------
void ProjectileManager::SendMessageToProjectilesWithinSphere(MessageID eMsg, const CPoint& obCentre, float fRadius)
{
	ntAssert(fRadius > 0.0f);

	float fTimeStep = CTimer::Get().GetGameTimeChange();

	ntstd::List<CEntity*>::const_iterator iter;
	ntstd::List<CEntity*>::const_iterator iterEnd = m_pProjectileList.end();

	Message msg(eMsg);

	// Go through all the projectiles in the list
	for (iter = m_pProjectileList.begin(); iter != iterEnd; iter++)
	{
		CEntity* pProjectile = 0;
		pProjectile = (*iter);
		ntAssert(pProjectile);

		CPoint obProjEndPoint( pProjectile->GetPosition() );
		
		CDirection obProjVelocity = pProjectile->GetPhysicsSystem()->GetLinearVelocity();
		
		CPoint obProjStartPoint( obProjEndPoint - ( obProjVelocity * fTimeStep ) );

		// Check if projectile is within the sphere
		if ( ProjectileWentThroughSphere( obProjStartPoint, obProjEndPoint, obCentre, fRadius ) )
		{
			// Send message to the projectile
			ntAssert(pProjectile->GetMessageHandler());

			ntPrintf("Sending message to projectile that went through sphere\n");

			pProjectile->GetMessageHandler()->QueueMessage(msg);
		}
	}
}


//--------------------------------------------------
//!
//!	ProjectileManager::ProjectileWentThroughSphere
//! Test to see if a projectile went through a sphere
//! Ray-Sphere test basically
//!
//--------------------------------------------------
bool ProjectileManager::ProjectileWentThroughSphere( const CPoint& obRayStart, const CPoint& obRayEnd, const CPoint& obSphereCentre, float fRadius )
{
	float fRadiusSq = fRadius * fRadius;
	CDirection obRayDir( obRayEnd - obRayStart );

	// Check if sphere is 'behind' the ray
	CDirection obStartToSphere( obSphereCentre - obRayStart );
	if ( obRayDir.Dot( obStartToSphere ) <= 0.0f )
	{
		// It is, so do radius check
		if ( obStartToSphere.LengthSquared() <= fRadiusSq )
			return true;

		return false;
	}

	// Check if sphere is 'in front' of the ray
	CDirection obEndToSphere( obSphereCentre - obRayEnd );
	if ( obRayDir.Dot( obEndToSphere ) >= 0.0f )
	{
		// It is, so do radius check
		if ( obEndToSphere.LengthSquared() <= fRadiusSq )
			return true;

		return false;
	}

	// So at this point, there is a point on the ray that creates a perpendicular to the sphere
	obRayDir.Normalise();
	CPoint obPointOnRay( obRayStart + ( obRayDir * obStartToSphere.Length() ) );
	CDirection obPointToSphere( obSphereCentre - obPointOnRay );

	if ( obPointToSphere.LengthSquared() <= fRadiusSq )
		return true;

	// Otherwise we are outside.
	return false;
}

