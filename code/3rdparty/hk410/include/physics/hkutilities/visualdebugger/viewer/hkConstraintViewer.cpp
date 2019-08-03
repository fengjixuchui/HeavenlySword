/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkmath/hkMath.h>

#include <hkutilities/visualdebugger/viewer/hkConstraintViewer.h>
#include <hkvisualize/hkDebugDisplayHandler.h>
#include <hkvisualize/hkProcessFactory.h>

#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>

#include <hkvisualize/hkDebugDisplay.h>
#include <hkvisualize/type/hkColor.h>
#include <hkdynamics/constraint/hkConstraintInstance.h>
#include <hkdynamics/constraint/breakable/hkBreakableConstraintData.h>
#include <hkdynamics/constraint/malleable/hkMalleableConstraintData.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkHingeDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkLimitedHingeDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkRagdollDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkPrismaticDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkWheelDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkStiffSpringDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkBallSocketDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkPointToPathDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkPointToPlaneDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkPulleyDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkConstraintChainDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkHingeLimitsDrawer.h>
#include <hkutilities/visualdebugger/viewer/constraint/hkRagdollLimitsDrawer.h>

int hkConstraintViewer::m_tag = 0;

void HK_CALL hkConstraintViewer::registerViewer()
{
	m_tag = hkProcessFactory::getInstance().registerProcess( getName(), create );
}

hkProcess* HK_CALL hkConstraintViewer::create(const hkArray<hkProcessContext*>& contexts)
{
	return new hkConstraintViewer(contexts);
}

hkConstraintViewer::hkConstraintViewer(const hkArray<hkProcessContext*>& contexts)
: hkWorldViewerBase( contexts )
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
		{
			hkWorld* world = m_context->getWorld(i);
	//		world->addConstraintListener( this );
			world->markForWrite();
			world->addWorldPostSimulationListener( this );
			world->unmarkForWrite();
		}
	}
}

hkConstraintViewer::~hkConstraintViewer()
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
		{
			hkWorld* world = m_context->getWorld(i);
	//		world->removeConstraintListener( this );
			world->markForWrite();
			world->removeWorldPostSimulationListener( this );
			world->unmarkForWrite();
		}
	}
}



void hkConstraintViewer::worldAddedCallback( hkWorld* world)
{
//	world->addConstraintListener( this );
	world->markForWrite();
	world->addWorldPostSimulationListener( this );
	world->unmarkForWrite();
}

void hkConstraintViewer::worldRemovedCallback( hkWorld* world)
{
//	world->removeConstraintListener( this );
	world->markForWrite();
	world->removeWorldPostSimulationListener( this );
	world->unmarkForWrite();
}

void hkConstraintViewer::postSimulationCallback( hkWorld* world )
{
	HK_TIMER_BEGIN("hkConstraintViewer", this);

	{
		const hkArray<hkSimulationIsland*>& islands = world->getActiveSimulationIslands();
		for ( int i = 0; i < islands.getSize(); ++i )
		{
			for ( int e = 0; e < islands[i]->getEntities().getSize(); e++)
			{
				hkEntity* entity = islands[i]->getEntities()[e];
				const hkArray<struct hkConstraintInternal>&  constraintMasters = entity->getConstraintMasters();

				for ( int c = 0; c < constraintMasters.getSize(); c++)
				{
					draw ( constraintMasters[c].m_constraint, m_displayHandler );
				}
			}
		}
	}

	{
		const hkArray<hkSimulationIsland*>& islands = world->getInactiveSimulationIslands();
		for ( int i = 0; i < islands.getSize(); ++i )
		{
			for ( int e = 0; e < islands[i]->getEntities().getSize(); e++)
			{
				hkEntity* entity = islands[i]->getEntities()[e];
				const hkArray<struct hkConstraintInternal>&  constraintMasters = entity->getConstraintMasters();
				for ( int c = 0; c < constraintMasters.getSize(); c++)
				{
					draw ( constraintMasters[c].m_constraint, m_displayHandler );
				}
			}
		}
	}

	HK_TIMER_END();
}


void hkConstraintViewer::draw(hkConstraintInstance* constraint, hkDebugDisplayHandler* displayHandler)
{
	HK_TIMER_BEGIN("draw", this);

	int type = constraint->getData()->getType();

	HK_ASSERT2(0x21d74fd9, displayHandler,"displayHandler is NULL");

	switch(type)
	{
		case hkConstraintData::CONSTRAINT_TYPE_BALLANDSOCKET:
		{
			hkBallSocketDrawer ballSocketDrawer;
			ballSocketDrawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_HINGE:
		{
			hkHingeDrawer hingeDrawer;
			hingeDrawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
		{
			hkLimitedHingeDrawer limitedHingeDrawer;
			limitedHingeDrawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_PRISMATIC:
		{
			hkPrismaticDrawer prismaticDrawer;
			prismaticDrawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_STIFFSPRING:
		{
			hkStiffSpringDrawer stiffSpringDrawer;
			stiffSpringDrawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_WHEEL:
		{
			hkWheelDrawer wheelDrawer;
			wheelDrawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_POINTTOPATH:
		{
			hkPointToPathDrawer pToPDrawer;
			pToPDrawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_POINTTOPLANE:
		{
			hkPointToPlaneDrawer pToPlaneDrawer;
			pToPlaneDrawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL:
		{
			hkRagdollDrawer ragdollDrawer;
			ragdollDrawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_BREAKABLE:
		{
			hkBreakableConstraintData* breakableConstraint = static_cast<hkBreakableConstraintData*>(constraint->getData());
			hkConstraintInstance fakeConstraint(constraint->getEntityA(), constraint->getEntityB(), breakableConstraint->getWrappedConstraintData() );
			draw(&fakeConstraint, displayHandler);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_MALLEABLE:
		{
			hkMalleableConstraintData* malleableConstraint = static_cast<hkMalleableConstraintData*>(constraint->getData());
			hkConstraintInstance fakeConstraint(constraint->getEntityA(), constraint->getEntityB(), malleableConstraint->getWrappedConstraintData() );
			draw(&fakeConstraint, displayHandler);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_PULLEY:
		{
			hkPulleyDrawer drawer;
			drawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;

		case hkConstraintData::CONSTRAINT_TYPE_STIFF_SPRING_CHAIN:
		case hkConstraintData::CONSTRAINT_TYPE_BALL_SOCKET_CHAIN:
		case hkConstraintData::CONSTRAINT_TYPE_POWERED_CHAIN:
		{
			hkConstraintChainDrawer drawer;
			drawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;
		case hkConstraintData::CONSTRAINT_TYPE_HINGE_LIMITS:
		{
			hkHingeLimitsDrawer drawer;
			drawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;
		case hkConstraintData::CONSTRAINT_TYPE_RAGDOLL_LIMITS:
		{
			hkRagdollLimitsDrawer drawer;
			drawer.drawConstraint(constraint, displayHandler, m_tag);
		}
		break;
	}

	HK_TIMER_END();


}



/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
