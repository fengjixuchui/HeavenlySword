
#include "Physics/config.h"

#include "ai/aidebugwall.h"
#include "ai/ainavgraphmanager.h"

#include "core/visualdebugger.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkMath/hkMath.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkdynamics/entity/hkRigidBodyCinfo.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/Motion/hkMotion.h>
#endif

#include "physics/world.h"
#include "physics/havokincludes.h"


/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

AIDebugWall::AIDebugWall() : m_obWorldMin(CONSTRUCT_CLEAR), m_obWorldMax(CONSTRUCT_CLEAR)
{
	CommonConstructor();
}


/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

AIDebugWall::AIDebugWall( const CPoint &worldMin, const CPoint &worldMax ) :m_obWorldMin(worldMin), m_obWorldMax(worldMax)
{
	CommonConstructor();
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		There are one or two places in the code where a AIDebugWall is constructed
*					temporarily (finding the intersection between 2 nodes for example) and in these
*					situations it's not desirable for the node to be added to any of the node lists.
*					There is, no doubt, a better way of doing it than this, but for now this
*					constructor with a boolean parameter will suffice.
*
***************************************************************************************************/

AIDebugWall::AIDebugWall( const bool bTemp ) : m_obWorldMin(CONSTRUCT_CLEAR), m_obWorldMax(CONSTRUCT_CLEAR)
{
	if (bTemp)
	{
		CommonConstructor();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CommonConstructor	
*
*	DESCRIPTION		The two constructors differ only by initialiser list, so common code is in here
*
***************************************************************************************************/

void AIDebugWall::CommonConstructor()
{
	// add to the NavGraphManager's wall list
	ntAssert( CAINavGraphManager::Exists() );
	CAINavGraphManager::Get().AddWall( this );
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void AIDebugWall::PostConstruct()
{
	CPoint obCentre;
	this->GetCentre( obCentre );

	CPoint obExtents;
	this->GetExtents( obExtents );
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	hkVector4 obHKExtents( obExtents.X(), obExtents.Y(), obExtents.Z());
	hkBoxShape* pobShape=HK_NEW hkBoxShape( obHKExtents , 0 );

	hkRigidBodyCinfo obInfo;

	obInfo.m_position = hkVector4(obCentre.X(), obCentre.Y(), obCentre.Z());
	 
	obInfo.m_rotation = hkQuaternion(0,0,0,1);

	obInfo.m_motionType = hkMotion::MOTION_FIXED;
	obInfo.m_qualityType = HK_COLLIDABLE_QUALITY_FIXED;
	
	obInfo.m_shape = pobShape;

	hkRigidBody* pobThisRigidBody = HK_NEW hkRigidBody(obInfo);

	pobThisRigidBody->getCollidableRw()->setAllowedPenetrationDepth(0.01f);

	//hkPropertyValue val((void*)CEntityManager::Get().GetPlayer());
	//pobThisRigidBody->addProperty(Physics::PROPERTY_ENTITY_PTR, val);

	Physics::EntityCollisionFlag obEntityCFlag; obEntityCFlag.base = 0;	
	obEntityCFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
				
	obEntityCFlag.flags.i_collide_with	= (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
											Physics::CHARACTER_CONTROLLER_ENEMY_BIT	);
				
	pobThisRigidBody->getCollidableRw()->setCollisionFilterInfo(obEntityCFlag.base);

	// add to world
	Physics::CPhysicsWorld::Get().AddEntity(pobThisRigidBody,HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE);
#endif
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

AIDebugWall::~AIDebugWall()
{
}

/***************************************************************************************************
*
*	FUNCTION		
*
*	DESCRIPTION		
*
***************************************************************************************************/

void	
AIDebugWall::PaulsDebugRender() const
{
#ifndef _GOLD_MASTER
	//g_VisualDebug->RenderAABB( m_obWorldMin, m_obWorldMax, 0xFFFFFFFF,
	//								DPF_WIREFRAME|DPF_NOCULLING );

	g_VisualDebug->RenderAABB( m_obWorldMin, m_obWorldMax, 0x8FFFFFFF );
#endif
}


void
AIDebugWall::MakeMinMax( float afMin[3], float afMax[3] ) const
{
	afMin[0] = ntstd::Min( m_obWorldMin.X(), m_obWorldMax.X() );
	afMin[1] = ntstd::Min( m_obWorldMin.Y(), m_obWorldMax.Y() );
	afMin[2] = ntstd::Min( m_obWorldMin.Z(), m_obWorldMax.Z() );

	afMax[0] = ntstd::Max( m_obWorldMin.X(), m_obWorldMax.X() );
	afMax[1] = ntstd::Max( m_obWorldMin.Y(), m_obWorldMax.Y() );
	afMax[2] = ntstd::Max( m_obWorldMin.Z(), m_obWorldMax.Z() );
}



