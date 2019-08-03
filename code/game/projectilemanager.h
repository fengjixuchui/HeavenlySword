//--------------------------------------------------
//!
//!	\file entitymanager.h
//!	Singleton that looks after all the entities in the game
//!
//--------------------------------------------------

#if !defined( GAME_PROJECTILEMANAGER_H )
#define GAME_PROJECTILEMANAGER_H

#include "game/entity.h"
#include "game/messages.h"

/***************************************************************************************************
*
*	CLASS			ProjectileManager
*
*	DESCRIPTION		Currently this is simply a list of all active projectiles in the world and some
*					helper functions that are used for gameplay purposes.
*
***************************************************************************************************/
class	ProjectileManager : public Singleton<ProjectileManager>
{
public:

	ProjectileManager();
	~ProjectileManager();

	void	AddProjectileToList(CEntity* pProjectile);

	void	RemoveProjectileFromList(CEntity* pProjectile);

	// Gameplay helper functions
	void	SendMessageToProjectilesWithinSphere(MessageID eMsg, const CPoint& obCentre, float fRadius);

private:

	// Ray Sphere test
	bool	ProjectileWentThroughSphere( const CPoint& obRayStart, const CPoint& obRayEnd, const CPoint& obSphereCentre, float fRadius );

	ntstd::List<CEntity*>	m_pProjectileList;
};


#endif // GAME_PROJECTILEMANAGER_H
