//--------------------------------------------------------------------------------------------------------
//!
//!	\file physics/collisionfilter.cpp
//!	
//!	PHYSICS COMPONENT:
//!		Implementation of our optimised physics filter
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.02.21
//!
//--------------------------------------------------------------------------------------------------------
#include "config.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#include "physics/havokincludes.h"
#include <hkbase/hkBase.h>

#include "collisionfilter.h"
#include "collisionbitfield.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/interactioncomponent.h"
#include "game/entityinfo.h"

#include "game/attacks.h"


#include "effect/psystem_utils.h"


#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/Motion/hkMotion.h>
#include <hkcollide/agent/hkCollidable.h>

#include <hkcollide\castutil\hkWorldRayCastInput.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include <hkdynamics/phantom/hkPhantomType.h>

#include <hkbase/monitor/hkMonitorStreamAnalyzer.h>
#include <hkcollide/shape/collection/hkShapeCollection.h>

#include "game/inputcomponent.h"

namespace Physics {

HSCollisionFilter::HSCollisionFilter()
{};

hkBool HSCollisionFilter::isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const
{
	HK_TIMER_BEGIN("nt_HSCollisionFilter", HK_NULL);

	bool bRet = false;

	hkRigidBody* obRBA		= 0; 
	hkRigidBody* obRBB		= 0; 
	hkPhantom* obPhantomA	= 0; 
	hkPhantom* obPhantomB	= 0; 


	// Get the type of collidable
	obRBA = hkGetRigidBody(&a);
	if(0 == obRBA)
		obPhantomA = hkGetPhantom(&a);

	obRBB = hkGetRigidBody(&b);
	if(0 == obRBB)
		obPhantomB = hkGetPhantom(&b);

	//DGF HACK HACK HACK - bit settings weren't consistently returning false, so had to do this for the moment to be sure
	CEntity* pobEntityA = 0;
	if( obRBA && obRBA->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityA = (CEntity*) obRBA->getProperty(PROPERTY_ENTITY_PTR).getPtr();
	else if( obPhantomA && obPhantomA->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityA = (CEntity*) obPhantomA->getProperty(PROPERTY_ENTITY_PTR).getPtr();

	CEntity* pobEntityB = 0;
	if( obRBB && obRBB->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityB = (CEntity*) obRBB->getProperty(PROPERTY_ENTITY_PTR).getPtr();
	else if( obPhantomB && obPhantomB->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityB = (CEntity*) obPhantomB->getProperty(PROPERTY_ENTITY_PTR).getPtr();	

	if ((pobEntityA && pobEntityA->GetAttackComponent() && pobEntityA->GetAttackComponent()->IsInSuperStyleSafetyTransition()) 
		|| 
		(pobEntityB && pobEntityB->GetAttackComponent() && pobEntityB->GetAttackComponent()->IsInSuperStyleSafetyTransition()))
		return false;

	FilterExceptionFlag filterA; filterA.base = 0;
	if( obRBA && obRBA->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterA.base = obRBA->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();
	else if( obPhantomA && obPhantomA->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterA.base = obPhantomA->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();

	FilterExceptionFlag filterB; filterB.base = 0;
	if( obRBB && obRBB->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterB.base = obRBB->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();
	else if( obPhantomB && obPhantomB->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterB.base = obPhantomB->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();	

	// Removed  && (pobEntityA->GetEntityInfo() && pobEntityB->GetEntityInfo()) in following test
	if(( pobEntityA && pobEntityB ) && !((filterA.flags.exception_set & IGNORE_ENTITY_FIGHT_BIT) || (filterB.flags.exception_set & IGNORE_ENTITY_FIGHT_BIT)))
	{
		// See if either of the characters are requesting that the collision be dropped
		if ( pobEntityA->GetAttackComponent() && pobEntityA->GetAttackComponent()->GetAttackCollisionBreak() )
		{
			// OK - now we are about to do a nasty check to see if the collision break requested
			// is with the other character we are dealing with here
			const CEntity* pobLockon = pobEntityA->GetAttackComponent()->GetCurrentTargetP();

			// If the character is locked on to the other then disable the collision
			if ( pobLockon && ( pobLockon == pobEntityB ) )
				return false;
		}
		else if ( pobEntityB->GetAttackComponent() && pobEntityB->GetAttackComponent()->GetAttackCollisionBreak() )
		{
			// OK - now we are about to do a nasty check to see if the collision break requested
			// is with the other character we are dealing with here
			const CEntity* pobLockon = pobEntityB->GetAttackComponent()->GetCurrentTargetP();

			// If the character is locked on to the other then disable the collision
			if ( pobLockon && ( pobLockon == pobEntityA ) )
				return false;
		}
	}

	if (( obPhantomA && ( obPhantomA->getType() == HK_PHANTOM_AABB ) ) ||
		( obPhantomB && ( obPhantomB->getType() == HK_PHANTOM_AABB ) ) )
	{
		return true;
	}

	if( (0 == obRBA) && (0 == obPhantomA) )
	{
		if(obRBB)
			bRet = isCollisionEnabledTriggerVSBody(&a,obRBB);
		if(obPhantomB)
			bRet = isCollisionEnabledTriggerVSPhantom(&a,obPhantomB);
	}
	else if( (0 == obRBB) && (0 == obPhantomB) )
	{
		if(obRBA)
			bRet = isCollisionEnabledTriggerVSBody(&b,obRBA);
		if(obPhantomA)
			bRet = isCollisionEnabledTriggerVSPhantom(&b,obPhantomA);
	}
	else if(!((obRBA || obPhantomA) && (obRBB || obPhantomB)))
	{
		bRet = false;
	}
	else if(0 == obRBA) 
	{
		if(0 == obRBB) 
		{
			bRet = isCollisionEnabledPhantomVSPhantom(obPhantomA, obPhantomB);

			/*if (bRet)
				ntPrintf("...accepted.\n");
			else
				ntPrintf("...rejected.\n");*/
		} 
		else 
		{
			bRet = isCollisionEnabledBodyVSPhantom(obRBB, obPhantomA);
		}
	} 
	else 
	{
		if(0 == obRBB) 
		{
			bRet = isCollisionEnabledBodyVSPhantom(obRBA, obPhantomB);
		} 
		else 
		{
			bRet = isCollisionEnabledBodyVSBody(obRBA, obRBB);
		}
	}
/*
	{
		CEntity* pobEntityA = 0;
		if( obRBA && obRBA->hasProperty(PROPERTY_ENTITY_PTR) )
			pobEntityA = (CEntity*) obRBA->getProperty(PROPERTY_ENTITY_PTR).getPtr();
		else if( obPhantomA && obPhantomA->hasProperty(PROPERTY_ENTITY_PTR) )
			pobEntityA = (CEntity*) obPhantomA->getProperty(PROPERTY_ENTITY_PTR).getPtr();
		CEntity* pobEntityB = 0;
		if( obRBB && obRBB->hasProperty(PROPERTY_ENTITY_PTR) )
			pobEntityB = (CEntity*) obRBB->getProperty(PROPERTY_ENTITY_PTR).getPtr();
		else if( obPhantomB && obPhantomB->hasProperty(PROPERTY_ENTITY_PTR) )
			pobEntityB = (CEntity*) obPhantomB->getProperty(PROPERTY_ENTITY_PTR).getPtr();*/	

	/*if (CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_P, KEYM_CTRL ) || CInputHardware::Get().GetKeyboardP()->IsKeyHeld( KEYC_P, KEYM_CTRL ))
	{
		if (bRet)
		{
			if (pobEntityA && pobEntityB)
				ntPrintf("Ent: %s/%s accepted.\n",pobEntityA->GetName().c_str(),pobEntityB->GetName().c_str());
			else if	(obRBA && obRBB)
				ntPrintf("RB: %s/%s accepted.\n",obRBA->getName(),obRBB->getName());
			else if (obPhantomA && obPhantomB)
				ntPrintf("Pha: %s/%s accepted.\n",obPhantomA->getName(),obPhantomB->getName());
			else
				ntPrintf("Something accepted.\n");
		}
		else
		{
			if (pobEntityA && pobEntityB)
				ntPrintf("Ent: %s/%s NOT accepted.\n",pobEntityA->GetName().c_str(),pobEntityB->GetName().c_str());
			else if	(obRBA && obRBB)
				ntPrintf("RB: %s/%s NOT accepted.\n",obRBA->getName(),obRBB->getName());
			else if (obPhantomA && obPhantomB)
				ntPrintf("Pha: %s/%s NOT accepted.\n",obPhantomA->getName(),obPhantomB->getName());
			else
				ntPrintf("Something NOT accepted.\n");
		}
	}*/

	HK_TIMER_END();

	return bRet;
}

#if HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1
hkBool HSCollisionFilter::isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& collectionB, hkShapeKey keyB  ) const
{
	// As long we dont use material information, this method is not going to be use to it's full potential. Backtrack to the root collidable for times being.
	return isCollisionEnabled(*(a.getRootCollidable()), *(b.getRootCollidable()));
	//return isCollisionEnabled(a, b);
};

hkBool HSCollisionFilter::isCollisionEnabled( const hkShapeRayCastInput& aInput,const hkShape& shp, const hkShapeContainer& bCollection, hkShapeKey bKey ) const
{
	// Let's read the current raycast filter info
	RaycastCollisionFlag obRayInfo;
	obRayInfo.base = aInput.m_filterInfo;

	EntityCollisionFlag infoB;
	infoB.base = bCollection.getCollisionFilterInfo(bKey);

	// Havok may leave some unintialised data, so take care of this
	if(	( obRayInfo.base == 0 )		||
		( infoB.base == 0 )			||
		( obRayInfo.base == 65536 )	||
		( infoB.base == 65536 )		)
	{
		return false;
	};

	// Deal with AI Wall
	if(	(obRayInfo.flags.i_collide_with & AI_WALL_BIT)	&&
		(infoB.flags.i_am & AI_WALL_BIT)				)
	{
		AIWallCollisionFlag aiwallFlagB; aiwallFlagB.base = infoB.base;
		if( (aiwallFlagB.flags.raycast_material & obRayInfo.flags.i_am) != 0)
			return true;

		return false;
	};

	// Else just rely on the usual flags settings
	if( (infoB.flags.i_am & obRayInfo.flags.i_collide_with) != 0) 
		return true;

	return false;
};
#else
hkBool HSCollisionFilter::isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeCollection& collectionB, hkShapeKey keyB  ) const
{
	// As long we dont use material information, this method is not going to be use to it's full potential. Backtrack to the root collidable for times being.
	return isCollisionEnabled(*(a.getRootCollidable()), *(b.getRootCollidable()));
	//return isCollisionEnabled(a, b);
};

hkBool HSCollisionFilter::isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShapeCollection& bCollection, hkShapeKey bKey ) const
{
	// Let's read the current raycast filter info
	RaycastCollisionFlag obRayInfo;
	obRayInfo.base = aInput.m_filterInfo;

	EntityCollisionFlag infoB;
	infoB.base = bCollection.getCollisionFilterInfo(bKey);

	// Havok may leave some unintialised data, so take care of this
	if(	( obRayInfo.base == 0 )		||
		( infoB.base == 0 )			||
		( obRayInfo.base == 65536 )	||
		( infoB.base == 65536 )		)
	{
		return false;
	};

	// Deal with AI Wall
	if(	(obRayInfo.flags.i_collide_with & AI_WALL_BIT)	&&
		(infoB.flags.i_am & AI_WALL_BIT)				)
	{
		AIWallCollisionFlag aiwallFlagB; aiwallFlagB.base = infoB.base;
		if( (aiwallFlagB.flags.raycast_material & obRayInfo.flags.i_am) != 0)
			return true;

		return false;
	};

	// Else just rely on the usual flags settings
	if( (infoB.flags.i_am & obRayInfo.flags.i_collide_with) != 0) 
		return true;

	return false;
};
#endif // HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1

hkBool HSCollisionFilter::isCollisionEnabled( const hkWorldRayCastInput& aInput, const hkCollidable& collidableB ) const
{
	// Let's read the current raycast filter info
	RaycastCollisionFlag obRayInfo;
	obRayInfo.base = aInput.m_filterInfo;

	EntityCollisionFlag infoB;
	infoB.base = collidableB./*getRootCollidable()->*/getCollisionFilterInfo();

	// Havok may leave some unintialised data, so take care of this
	if(	( obRayInfo.base == 0 )		||
		( infoB.base == 0 )			||
		( obRayInfo.base == 65536 )	||
		( infoB.base == 65536 )		)
	{
		return false;
	};

	// Deal with AI Wall
	if(	(obRayInfo.flags.i_collide_with & AI_WALL_BIT)	&&
		(infoB.flags.i_am & AI_WALL_BIT)				)
	{
		AIWallCollisionFlag aiwallFlagB; aiwallFlagB.base = infoB.base;
		if( (aiwallFlagB.flags.raycast_material & obRayInfo.flags.i_am) != 0) 
			return true;

		return false;
	};

	if( obRayInfo.flags.i_am & STATIC_ENVIRONMENT_BIT ) 
	{
		if( hkGetRigidBody(&collidableB) == 0 )
			return false;
		if( !hkGetRigidBody(&collidableB)->isFixed() )
			return false;
	}

	// Else just rely on the usual flags settings
	if( (infoB.flags.i_am & obRayInfo.flags.i_collide_with) != 0) 
		return true;

	return false;
};

hkBool HSCollisionFilter::isCollisionEnabledPhantomVSPhantom(const hkPhantom* obPhantomA, const hkPhantom* obPhantomB) const
{
	// We have two phantoms
	EntityCollisionFlag infoA;
	infoA.base = obPhantomA->getCollidable()->getCollisionFilterInfo();
	EntityCollisionFlag infoB; 
	infoB.base = obPhantomB->getCollidable()->getCollisionFilterInfo();

	// Havok may leave some unintialised data, so take care of this
	if(	( infoA.base == 0 )		||
		( infoB.base == 0 )		||
		( infoA.base == 65536 )	||
		( infoB.base == 65536 )	)
	{
		return false;
	};

	CEntity* pobEntityA = 0;
	if( obPhantomA->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityA = (CEntity*) obPhantomA->getProperty(PROPERTY_ENTITY_PTR).getPtr();

	CEntity* pobEntityB = 0; 
	if( obPhantomB->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityB = (CEntity*) obPhantomB->getProperty(PROPERTY_ENTITY_PTR).getPtr();

	FilterExceptionFlag filterA; filterA.base = 0;
	if( obPhantomA->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterA.base = obPhantomA->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();

	FilterExceptionFlag filterB; filterB.base = 0;
	if( obPhantomB->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterB.base = obPhantomB->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();	

	if((filterA.flags.exception_set & ALWAYS_RETURN_TRUE_BIT) || (filterB.flags.exception_set & ALWAYS_RETURN_TRUE_BIT))
		return true;

	if((filterA.flags.exception_set & ALWAYS_RETURN_FALSE_BIT) || (filterB.flags.exception_set & ALWAYS_RETURN_FALSE_BIT))
		return false;

	if((filterA.flags.exception_set & ONLY_FIXED_GEOM) || (filterB.flags.exception_set & ONLY_FIXED_GEOM))
		return false;

	// If it's the same entity, just return false
	if(!((filterA.flags.exception_set & IGNORE_ENTITY_PTR_BIT) || (filterB.flags.exception_set & IGNORE_ENTITY_PTR_BIT)))
	{
		if(pobEntityA == pobEntityB)
			return false;
	};

	if (
		( (filterA.flags.exception_set & IGNORE_CCs) && ((infoB.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT) || (infoB.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT)) ) ||
		( (filterB.flags.exception_set & IGNORE_CCs) && ((infoA.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT) || (infoA.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT)) )
		)
	{
		return false;
	}

	// If those two entities interact together
	if(pobEntityA && pobEntityB)
	{
		//ntPrintf("PP, %s vs %s",pobEntityA->GetName().c_str(),pobEntityB->GetName().c_str());
		
		if(!((filterA.flags.exception_set & IGNORE_ENTITY_INTERACT_BIT) || (filterB.flags.exception_set & IGNORE_ENTITY_INTERACT_BIT)))
		{
			if (pobEntityA->GetInteractionComponent())
			{
				if (!pobEntityA->GetInteractionComponent()->CanCollideWith(pobEntityB))
				{
					return false;
				};
			};
		};
	};

	// Then, rely on the flags
	if(	((infoA.flags.i_am & infoB.flags.i_collide_with) != 0) &&
		((infoB.flags.i_am & infoA.flags.i_collide_with) != 0) )
		return true;

	return false;
};

hkBool HSCollisionFilter::isCollisionEnabledBodyVSPhantom(const hkRigidBody* obRBA, const hkPhantom* obPhantomB) const
{
	// We got obRBA and obPhantomB
	EntityCollisionFlag infoA;
	infoA.base = obRBA->getCollidable()->getCollisionFilterInfo();
	EntityCollisionFlag infoB;
	infoB.base = obPhantomB->getCollidable()->getCollisionFilterInfo();

	// HACK for rigid body in middle of character controller, we don't need full character controller to collide
	if ( ((infoA.flags.i_am & Physics::CHARACTER_CONTROLLER_ENEMY_BIT) || (infoA.flags.i_am & Physics::CHARACTER_CONTROLLER_PLAYER_BIT))
		&&
		((infoB.flags.i_am & Physics::CHARACTER_CONTROLLER_ENEMY_BIT) || (infoB.flags.i_am & Physics::CHARACTER_CONTROLLER_PLAYER_BIT))
		&&
		obRBA->getMass() == 0.0f )
	{
		return false;
	}

	// Havok may leave some unintialised data, so take care of this
	if(	( infoA.base == 0 )		||
		( infoB.base == 0 )		||
		( infoA.base == 65536 )	||
		( infoB.base == 65536 )	)
	{
		return false;
	};

	CEntity* pobEntityA = 0;
	if( obRBA->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityA = (CEntity*) obRBA->getProperty(PROPERTY_ENTITY_PTR).getPtr();
		
	CEntity* pobEntityB = 0;
	if( obPhantomB->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityB = (CEntity*) obPhantomB->getProperty(PROPERTY_ENTITY_PTR).getPtr();	

	if( pobEntityA && pobEntityB)
	{
		FilterExceptionFlag filterA; filterA.base = 0;
		if( obRBA->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
			filterA.base = obRBA->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();

		FilterExceptionFlag filterB; filterB.base = 0;
		if( obPhantomB->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
			filterB.base = obPhantomB->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();	

		if((filterB.flags.exception_set & IGNORE_FIXED_GEOM) && (obRBA->getMotionType() == hkMotion::MOTION_FIXED))
			return false;

		if((filterB.flags.exception_set & ONLY_FIXED_GEOM) && (obRBA->isFixedOrKeyframed() == false))
			return false;
		
		if((filterA.flags.exception_set & ALWAYS_RETURN_TRUE_BIT) || (filterB.flags.exception_set & ALWAYS_RETURN_TRUE_BIT))
			return true;

		if((filterA.flags.exception_set & ALWAYS_RETURN_FALSE_BIT) || (filterB.flags.exception_set & ALWAYS_RETURN_FALSE_BIT))
			return false;

		if( filterA.flags.exception_set & COLLIDE_WITH_PLAYER_ONLY )
		{
			if( !pobEntityB->IsPlayer() )
				return false;
		}

		if( filterA.flags.exception_set & COLLIDE_WITH_NMEs_ONLY )
		{
			if( !pobEntityB->IsEnemy() )
				return false;
		}

		if (
			( (filterA.flags.exception_set & IGNORE_CCs) && ((infoB.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT) || (infoB.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT)) ) ||
			( (filterB.flags.exception_set & IGNORE_CCs) && ((infoA.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT) || (infoA.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT)) )
			)
		{
			return false;
		}

		// If those two entities interact together
		if(!((filterA.flags.exception_set & IGNORE_ENTITY_INTERACT_BIT) || (filterB.flags.exception_set & IGNORE_ENTITY_INTERACT_BIT)))
		{
			if (pobEntityA->GetInteractionComponent())
			{
				if (!pobEntityA->GetInteractionComponent()->CanCollideWith(pobEntityB))
				{
					return false;
				};
			};

			if (pobEntityB->GetInteractionComponent())
			{
				if (!pobEntityB->GetInteractionComponent()->CanCollideWith(pobEntityA))
				{
					return false;
				};
			};
		};

		// Special case one: Ragdoll at rest or activated ?
		if( (infoB.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT) || (infoB.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT) )
		{
			if(infoA.flags.i_am & RAGDOLL_BIT)
			{
				if(	(filterA.flags.exception_set & CC_AND_RAGDOLL_ALWAYS_BIT) || 
					(filterB.flags.exception_set & CC_AND_RAGDOLL_ALWAYS_BIT)	)
				{
					return true;
				};

				if(	(filterA.flags.exception_set & CC_AND_RAGDOLL_NEVER_BIT) || 
					(filterB.flags.exception_set & CC_AND_RAGDOLL_NEVER_BIT)	)
				{
					return false;
				};

				if(/*(obRBA->isActive()) &&*/ (pobEntityB != pobEntityA)) 
				{
					if(	((infoA.flags.i_am & infoB.flags.i_collide_with) != 0) &&
						((infoB.flags.i_am & infoA.flags.i_collide_with) != 0) )
						return true;
				};
				return false;
			}
			else if ((infoB.flags.i_collide_with & AI_WALL_BIT)!= 0 && (infoA.flags.i_am & AI_WALL_BIT) != 0 )
			{
				// Check AI walls and KO states. Allow KO-s to tunnel through wall
				AIWallCollisionFlag wall;
				wall.base = infoA.base;
				if (wall.flags.kill_passing_KO == 0) 
				{
					// collision will be refused in collision listener
					ChatacterControllerCollisionFlag characterFlags; 
					characterFlags.base = infoB.base;

					if ( characterFlags.flags.i_am_important == 0)
					{
						if ( (wall.flags.not_collide_with_KO_states_unimportant & characterFlags.flags.i_am_in_KO_state) != 0)
							return false;
					}
					else
					{
						if ( (wall.flags.not_collide_with_KO_states_important & characterFlags.flags.i_am_in_KO_state) != 0)
							return false;
					}
				}
			}
		};

		// Special case two: static vs trigger volumes ?
		if(infoB.flags.i_am & TRIGGER_VOLUME_BIT)
		{
			// Special case two: Do not consider static environment for trigger volumes
			if(obRBA->getMotionType() == hkMotion::MOTION_FIXED)
				return false;
		};
	
		// If it's the same entity, just return false
		if(!((filterA.flags.exception_set & IGNORE_ENTITY_PTR_BIT) || (filterB.flags.exception_set & IGNORE_ENTITY_PTR_BIT)))
		{
			if(pobEntityB == pobEntityA)
				return false;
		};
	};

	if(	((infoA.flags.i_am & infoB.flags.i_collide_with) != 0) &&
		((infoB.flags.i_am & infoA.flags.i_collide_with) != 0) )
		return true;

	return false;
};

hkBool HSCollisionFilter::isCollisionEnabledBodyVSBody(const hkRigidBody* obRBA, const hkRigidBody* obRBB) const
{
	// Enforce some basic common sense rules - ignore collisions between fixed and keyframed.
	// hmm is it needed? havok should filter it.
	hkMotion::MotionType AType = obRBA->getMotionType(); 
	hkMotion::MotionType BType = obRBB->getMotionType(); 
	
	if ( ( AType == hkMotion::MOTION_FIXED || AType == hkMotion::MOTION_KEYFRAMED ) &&
		 ( BType == hkMotion::MOTION_FIXED || BType == hkMotion::MOTION_KEYFRAMED ) )
	{
		return false;
	};


	// Havok may leave some unintialised data, so skip this, rather than processing in detail.
	EntityCollisionFlag infoA;
	infoA.base = obRBA->getCollidable()->getCollisionFilterInfo();
	EntityCollisionFlag infoB;
	infoB.base = obRBB->getCollidable()->getCollisionFilterInfo();

	if(	( infoA.base == 0 )		||  // collides with nothing!
		( infoB.base == 0 )		||
		( infoA.base == 65536 )	||	// collides with everything!
		( infoB.base == 65536 )	)
	{
		return false;
	};

	
	// Get the collidiing entities.
	CEntity* pobEntityA = 0;
	if( obRBA->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityA = (CEntity*) obRBA->getProperty(PROPERTY_ENTITY_PTR).getPtr();

	CEntity* pobEntityB = 0;
	if( obRBB->hasProperty(PROPERTY_ENTITY_PTR) )
		pobEntityB = (CEntity*) obRBB->getProperty(PROPERTY_ENTITY_PTR).getPtr();	

	//user_warn_p( pobEntityA && pobEntityB, ( "WTF?! We have a collision without an entity. Get Peter F on the case.\n" ) );
	// speedtree objects has rigid bodies without entities

	
	// Get the collision filter information.
	FilterExceptionFlag filterA; filterA.base = 0;
	if( obRBA->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterA.base = obRBA->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();		

	FilterExceptionFlag filterB; filterB.base = 0;
	if( obRBB->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterB.base = obRBB->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();	

	// Special exceptions.
	if((filterA.flags.exception_set & ALWAYS_RETURN_TRUE_BIT) || (filterB.flags.exception_set & ALWAYS_RETURN_TRUE_BIT))
		return true;

	if((filterA.flags.exception_set & ALWAYS_RETURN_FALSE_BIT) || (filterB.flags.exception_set & ALWAYS_RETURN_FALSE_BIT))
		return false;

	if((filterA.flags.exception_set & COLLIDE_WITH_CCs_ONLY) || (filterB.flags.exception_set & COLLIDE_WITH_CCs_ONLY))
		return false;		 
	   	
	
	// Stop ragdolls reacting to objects that have been picked up. (Otherwise, they ping out of the way!)
	/*if ( ( infoB.flags.i_am & SMALL_INTERACTABLE_BIT ) && ( infoA.flags.i_am & RAGDOLL_BIT )  )
	{
		if ( obRBB->getMotionType() == hkMotion::MOTION_KEYFRAMED )
			return false;
	}
	else if ( ( infoA.flags.i_am & SMALL_INTERACTABLE_BIT ) && ( infoB.flags.i_am & RAGDOLL_BIT ) )
	{
		if ( obRBA->getMotionType() == hkMotion::MOTION_KEYFRAMED )
			return false;
	}
	else*/
	
	
	// Ragdoll VS Ragdoll
	if( (infoA.flags.i_am & RAGDOLL_BIT) && (infoB.flags.i_am & RAGDOLL_BIT) && (pobEntityA == pobEntityB) )
	{
		RagdollCollisionFlag ragdollFlagA; ragdollFlagA.base = infoA.base;
		RagdollCollisionFlag ragdollFlagB; ragdollFlagB.base = infoB.base;
		switch(ragdollFlagA.flags.ragdoll_material)
		{
		case RAGDOLL_NO_COLLIDE:
			return false;
			break;
		case RAGDOLL_PELVIS_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_SPINE_00_MATERIAL: // Adjacent.
			case RAGDOLL_L_LEG_MATERIAL:	// Adjacent.
			case RAGDOLL_R_LEG_MATERIAL:	// Adjacent.
			case RAGDOLL_L_ARM_MATERIAL:	// Can't reach
			case RAGDOLL_R_ARM_MATERIAL:	// Can't reach
			case RAGDOLL_HEAD_MATERIAL:		// Can't reach
			case RAGDOLL_L_KNEE_MATERIAL:	// Can't reach
			case RAGDOLL_R_KNEE_MATERIAL:	// Can't reach
				return false;
				break;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_SPINE_00_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_PELVIS_MATERIAL:	// Adjacent.
			case RAGDOLL_HEAD_MATERIAL:		// Adjacent.
			case RAGDOLL_L_ARM_MATERIAL:	// Adjacent.
			case RAGDOLL_R_ARM_MATERIAL:	// Adjacent.
			case RAGDOLL_L_LEG_MATERIAL:	// Can't reach
			case RAGDOLL_R_LEG_MATERIAL:	// Can't reach
			case RAGDOLL_L_KNEE_MATERIAL:	// Can't reach
			case RAGDOLL_R_KNEE_MATERIAL:	// Can't reach
				return false;
				break;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_HEAD_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_SPINE_00_MATERIAL:	// Adjacent.
			case RAGDOLL_PELVIS_MATERIAL:	// Can't reach
			case RAGDOLL_L_LEG_MATERIAL:	// Can't reach
			case RAGDOLL_R_LEG_MATERIAL:	// Can't reach
			case RAGDOLL_L_KNEE_MATERIAL:	// Can't reach
			case RAGDOLL_R_KNEE_MATERIAL:	// Can't reach
				return false;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_L_ARM_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{  
			case RAGDOLL_SPINE_00_MATERIAL:	// Adjacent.
			case RAGDOLL_L_ELBOW_MATERIAL:	// Adjacent.
			case RAGDOLL_PELVIS_MATERIAL:  	// Can't reach
				return false;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_R_ARM_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_SPINE_00_MATERIAL:	// Adjacent.
			case RAGDOLL_R_ELBOW_MATERIAL:	// Adjacent.
			case RAGDOLL_PELVIS_MATERIAL:	// Can't reach
				return false;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_L_ELBOW_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_L_ARM_MATERIAL:	// Adjacent.
				return false;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_R_ELBOW_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_R_ARM_MATERIAL:	// Adjacent.
				return false;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_L_LEG_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_PELVIS_MATERIAL: 	// Adjacent.
			case RAGDOLL_L_KNEE_MATERIAL: 	// Adjacent.
			case RAGDOLL_HEAD_MATERIAL:		// Can't reach
			case RAGDOLL_SPINE_00_MATERIAL:	// Can't reach
				return false;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_R_LEG_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_PELVIS_MATERIAL: 	// Adjacent.
			case RAGDOLL_R_KNEE_MATERIAL: 	// Adjacent.
			case RAGDOLL_HEAD_MATERIAL:		// Can't reach
			case RAGDOLL_SPINE_00_MATERIAL:	// Can't reach
				return false;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_L_KNEE_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_L_LEG_MATERIAL:	// Adjacent.
			case RAGDOLL_HEAD_MATERIAL:		// Can't reach
			case RAGDOLL_PELVIS_MATERIAL: 	// Can't reach
			case RAGDOLL_SPINE_00_MATERIAL: // Can't reach
			case RAGDOLL_L_ARM_MATERIAL:	// Can't reach
			case RAGDOLL_R_ARM_MATERIAL:	// Can't reach
				return false;
			default:
				return true;
				break;
			};
			break;
		case RAGDOLL_R_KNEE_MATERIAL:
			switch(ragdollFlagB.flags.ragdoll_material)
			{
			case RAGDOLL_R_LEG_MATERIAL:	// Adjacent.
			case RAGDOLL_HEAD_MATERIAL:		// Can't reach
			case RAGDOLL_PELVIS_MATERIAL: 	// Can't reach
			case RAGDOLL_SPINE_00_MATERIAL: // Can't reach
			case RAGDOLL_L_ARM_MATERIAL:	// Can't reach
			case RAGDOLL_R_ARM_MATERIAL:	// Can't reach  
				return false;
			default:
				return true;
				break;
			};
			break;
		default:
			{
				return false;
			};
		};
	}
		
	
	if( !( ( filterA.flags.exception_set & IGNORE_ENTITY_INTERACT_BIT ) ||
		   ( filterB.flags.exception_set & IGNORE_ENTITY_INTERACT_BIT ) ) )
	{
		// If not specified to ignore, reject collisions between interacting entities.
		if (pobEntityA && pobEntityA->GetInteractionComponent() && !pobEntityA->GetInteractionComponent()->CanCollideWith(pobEntityB))
		{
			return false;
		}
	}
	
	
	// Finally, rely on the collide_with flags
	if(	((infoA.flags.i_am & infoB.flags.i_collide_with) != 0) &&
		((infoB.flags.i_am & infoA.flags.i_collide_with) != 0) )
	{
		// special cases for characters colliding with invisible walls.
		EntityCollisionFlag infoWall 		= ( infoA.flags.i_am & AI_WALL_BIT ) ? infoA : infoB;
		EntityCollisionFlag infoCharacter 	= ( infoA.flags.i_am & (CHARACTER_CONTROLLER_PLAYER_BIT | CHARACTER_CONTROLLER_ENEMY_BIT | RAGDOLL_BIT) ) ? infoA : infoB;
		
		if ( infoWall.flags.i_am & AI_WALL_BIT && 
			 infoCharacter.flags.i_am & (CHARACTER_CONTROLLER_PLAYER_BIT | CHARACTER_CONTROLLER_ENEMY_BIT | RAGDOLL_BIT) )
		{
			// Check AI walls and KO states. Allow KO-s to tunnel through wall
			AIWallCollisionFlag wall;
			wall.base = infoWall.base;

			// check if wall collides with dead ragdolls. 
			if (wall.flags.not_collide_with_dead_ragdolls && (infoCharacter.flags.i_am & RAGDOLL_BIT) )
			{
				RagdollCollisionFlag ragdollFlags;
				ragdollFlags.base = infoCharacter.base;
				if (ragdollFlags.flags.character_dead)
					return false;
			}

			if (wall.flags.kill_passing_KO == 0) 
			{
				// collision will be refused in collision listener otherwise
				ChatacterControllerCollisionFlag characterFlags;
				characterFlags.base = infoCharacter.base;

				if ( characterFlags.flags.i_am_important == 0)
				{
					if ( (wall.flags.not_collide_with_KO_states_unimportant & characterFlags.flags.i_am_in_KO_state) != 0)
						return false;
				}
				else
				{
					if ( (wall.flags.not_collide_with_KO_states_important & characterFlags.flags.i_am_in_KO_state) != 0)
						return false;
				}
			}
		}

		return true;
	}

	return false;
};


hkBool HSCollisionFilter::isCollisionEnabledTriggerVSBody( const hkCollidable* a, const hkRigidBody* b) const
{
	// We got obRBA and obPhantomB
	EntityCollisionFlag infoA;
	infoA.base = b->getCollidable()->getCollisionFilterInfo();
	EntityCollisionFlag infoB;
	infoB.base = a->getCollisionFilterInfo();

	// Havok may leave some unintialised data, so take care of this
	if(	( infoA.base == 0 )		||
		( infoB.base == 0 )		||
		( infoA.base == 65536 )	||
		( infoB.base == 65536 )	)
	{
		return true;
	};

	FilterExceptionFlag filterA; filterA.base = 0;
	if( b->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterA.base = b->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();	

	if(filterA.flags.exception_set & ALWAYS_RETURN_TRUE_BIT)
		return true;

	if(filterA.flags.exception_set & ALWAYS_RETURN_FALSE_BIT) 
		return false;

	// Special case two: static vs trigger volumes ?
	if(infoB.flags.i_am & TRIGGER_VOLUME_BIT)
	{
		// Special case two: Do not consider static environment for trigger volumes
		if(b->getMotionType() == hkMotion::MOTION_FIXED)
			return false;
	};

	if(	((infoA.flags.i_am & infoB.flags.i_collide_with) != 0) &&
		((infoB.flags.i_am & infoA.flags.i_collide_with) != 0) )
		return true;

	return false;
};

hkBool HSCollisionFilter::isCollisionEnabledTriggerVSPhantom( const hkCollidable* a, const hkPhantom* b) const
{
		// We have two phantoms
	EntityCollisionFlag infoA;
	infoA.base = a->getCollisionFilterInfo();
	EntityCollisionFlag infoB; 
	infoB.base = b->getCollidable()->getCollisionFilterInfo();

	// Havok may leave some unintialised data, so take care of this
	if(	( infoA.base == 0 )		||
		( infoB.base == 0 )		||
		( infoA.base == 65536 )	||
		( infoB.base == 65536 )	)
	{
		return true;
	};

	FilterExceptionFlag filterB; filterB.base = 0;
	if( b->hasProperty(PROPERTY_FILTER_EXCEPTION_INT) )
		filterB.base = b->getProperty(PROPERTY_FILTER_EXCEPTION_INT).getInt();

	if(filterB.flags.exception_set & ALWAYS_RETURN_TRUE_BIT)
		return true;

	if(filterB.flags.exception_set & ALWAYS_RETURN_FALSE_BIT)
		return false;

	// Then, rely on the flags
	if(	((infoA.flags.i_am & infoB.flags.i_collide_with) != 0) &&
		((infoB.flags.i_am & infoA.flags.i_collide_with) != 0) )
		return true;

	return false;
};

} // Physics namespace

#endif
		

