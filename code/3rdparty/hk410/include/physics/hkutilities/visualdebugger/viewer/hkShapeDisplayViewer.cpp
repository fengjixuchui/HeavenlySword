/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/monitor/hkMonitorStream.h>
#include <hkutilities/visualdebugger/viewer/hkShapeDisplayViewer.h>
#include <hkutilities/visualdebugger/viewer/hkShapeDisplayBuilder.h>

#include <hkvisualize/shape/hkDisplayGeometry.h>
#include <hkvisualize/hkDebugDisplayHandler.h>
#include <hkvisualize/hkProcessFactory.h>

#include <hkdynamics/world/hkSimulationIsland.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>

#include <hkvisualize/hkVisualDebuggerDebugOutput.h>
#include <hkvisualize/type/hkColor.h>
#include <hkvisualize/shape/hkDisplayConvex.h>

static const unsigned int HK_DEFAULT_OBJECT_COLOR = hkColor::rgbFromChars( 240, 240, 240, 255 );
static const unsigned int HK_DEFAULT_FIXED_OBJECT_COLOR = hkColor::rgbFromChars( 120, 120, 120, 255 );

int hkShapeDisplayViewer::m_tag = 0;

hkProcess* HK_CALL hkShapeDisplayViewer::create(const hkArray<hkProcessContext*>& contexts )
{
	return new hkShapeDisplayViewer(contexts);
}

void HK_CALL hkShapeDisplayViewer::registerViewer()
{
	m_tag = hkProcessFactory::getInstance().registerProcess( getName(), create );
}

hkShapeDisplayViewer::hkShapeDisplayViewer( const hkArray<hkProcessContext*>& contexts )
: hkWorldViewerBase( contexts )
{
	m_enableShapeTransformUpdate = true;

	m_timeForDisplay = -1.f;

	int nc = contexts.getSize();
	for (int i=0; i < nc; ++i)
	{
		if ( hkString::strCmp(HK_DISPLAY_VIEWER_OPTIONS_CONTEXT, contexts[i]->getType() ) ==0 )
		{
			ShapeDisplayViewerOptions* options = static_cast<ShapeDisplayViewerOptions*>(contexts[i] );
			m_enableShapeTransformUpdate = options->m_enableShapeTransformUpdate;
			break;
		}
	}
}

void hkShapeDisplayViewer::init()
{
	if (m_context)
	{
		for (int i=0; i < m_context->getNumWorlds(); ++i)
		{
			addWorld( m_context->getWorld(i) );
		}
	}
}

hkShapeDisplayViewer::~hkShapeDisplayViewer()
{
	int ne = m_worldEntities.getSize();
	for (int i=(ne-1); i >= 0; --i) // backwards as remove alters array
	{
		removeWorld(i);
	}
}

void hkShapeDisplayViewer::worldRemovedCallback( hkWorld* world ) 
{ 
	int de = findWorld(world);
	if (de >= 0)
	{	
		removeWorld(de);
	}	
}

//World added listener. Should impl this in sub class, but call up to this one to get the listener reg'd.
void hkShapeDisplayViewer::worldAddedCallback( hkWorld* world )
{
	addWorld(world);	
}

void hkShapeDisplayViewer::addWorld(hkWorld* world)
{
	world->markForWrite();
		world->addEntityListener( this );
		world->addWorldPostSimulationListener( this );

	WorldToEntityData* wed = new WorldToEntityData;
	wed->world = world;
	m_worldEntities.pushBack(wed);

	// get all the active entities from the active simulation islands
	{
		const hkArray<hkSimulationIsland*>& activeIslands = world->getActiveSimulationIslands();

		for(int i = 0; i < activeIslands.getSize(); i++)
		{
			const hkArray<hkEntity*>& activeEntities = activeIslands[i]->getEntities();
			for(int j = 0; j < activeEntities.getSize(); j++)
			{
				entityAddedCallback( activeEntities[j] );
			}
		}
	}

	// get all the inactive entities from the inactive simulation islands
	{
		const hkArray<hkSimulationIsland*>& inactiveIslands = world->getInactiveSimulationIslands();

		for(int i = 0; i < inactiveIslands.getSize(); i++)
		{
			const hkArray<hkEntity*>& activeEntities = inactiveIslands[i]->getEntities();
			for(int j = 0; j < activeEntities.getSize(); j++)
			{
				entityAddedCallback( activeEntities[j] );
			}
		}
	}


	// get all the fixed bodies in the world
	if (world->getFixedIsland())
	{
		const hkArray<hkEntity*>& fixedEntities = world->getFixedIsland()->getEntities();
		for(int j = 0; j < fixedEntities.getSize(); j++)
		{
			entityAddedCallback( fixedEntities[j] );
		}
	}
	world->unmarkForWrite();
}

void hkShapeDisplayViewer::entityAddedCallback( hkEntity* entity )
{
	const hkShape* shapeInProperty = reinterpret_cast<const hkShape*>( entity->getProperty( HK_PROPERTY_DISPLAY_SHAPE ).getPtr() );
	if( entity->getCollidable()->getShape() == HK_NULL && shapeInProperty == HK_NULL )
	{
		return;
	}

	// if we are a local viewer (not a vdb viewer) then we 
	// will ignore the bodies that have a display ptr already.
	bool isLocalViewer =  (m_inStream == HK_NULL) && (m_outStream == HK_NULL);		
	if (isLocalViewer)
	{
		hkPropertyValue v = entity->getProperty(HK_PROPERTY_DISPLAY_PTR);
		if (v.getPtr())
			return;
	}

	// figure out the right world list for it
	// We should have the world in our list
	hkWorld* world = entity->getWorld();
	int index = findWorld(world);
	if (index >= 0)
	{
		WorldToEntityData* wed = m_worldEntities[index];

	
		hkRigidBody* rigidBody = static_cast<hkRigidBody*>(entity);

		// create an array of display geometries from the collidable - use default display settings
		hkInplaceArray<hkDisplayGeometry*,8> displayGeometries;
		{
			hkGeometry* geom = reinterpret_cast<hkGeometry*>(rigidBody->getProperty(HK_PROPERTY_OVERRIDE_DEBUG_DISPLAY_GEOMETRY).getPtr());
			if ( geom )
			{
				rigidBody->removeProperty( HK_PROPERTY_OVERRIDE_DEBUG_DISPLAY_GEOMETRY );
				hkDisplayGeometry* dg = new hkDisplayConvex(geom);
				dg->getTransform().setIdentity();
				displayGeometries.pushBackUnchecked( dg );
			}
			else
			{
				hkShapeDisplayBuilder::hkShapeDisplayBuilderEnvironment env;
				hkShapeDisplayBuilder shapeBuilder(env);
				const hkShape* shape = shapeInProperty;
				if (!shape)
				{
					shape = rigidBody->getCollidable()->getShape();
				}
				shapeBuilder.buildDisplayGeometries( shape, displayGeometries );

				for(int i = (displayGeometries.getSize() - 1); i >= 0; i--)
				{
					if( (displayGeometries[i]->getType() == HK_DISPLAY_CONVEX) &&
						(displayGeometries[i]->getGeometry() == HK_NULL) )
					{
						HK_REPORT("Unable to build display geometry from hkShape geometry data");
						displayGeometries.removeAt(i);
					}
				}

				if ( shapeInProperty )
				{
					shapeInProperty->removeReference();
					entity->removeProperty( HK_PROPERTY_DISPLAY_SHAPE );
				}
			}
		}

		// send the display geometries off to the display handler
		if (displayGeometries.getSize() > 0)
		{
			hkUlong id = getId( rigidBody );

			wed->entitiesCreated.pushBack( id );
			m_displayHandler->addGeometry( displayGeometries, rigidBody->getTransform(), id, m_tag );

			int color = rigidBody->getProperty( HK_PROPERTY_DEBUG_DISPLAY_COLOR ).getInt();
			if ( 0 == color )
			{
				if( rigidBody->isFixed() )
				{
					color = HK_DEFAULT_FIXED_OBJECT_COLOR;
				}
				else
				{
					color = HK_DEFAULT_OBJECT_COLOR;
				}
			}
			m_displayHandler->setGeometryColor( color, id, m_tag );
		}

		// delete intermediate display geometries - we could cache these for duplication - TODO
		{
			for( int i = 0; i < displayGeometries.getSize(); ++i )
			{
				delete displayGeometries[i];
			}
		}
	}
}

void hkShapeDisplayViewer::entityRemovedCallback( hkEntity* entity )
{
	if( entity->getCollidable()->getShape() == HK_NULL )
	{
		return;
	}

	hkWorld* world = entity->getWorld();
	int worldIndex = findWorld(world);
	if (worldIndex >= 0)
	{
		WorldToEntityData* wed = m_worldEntities[worldIndex];

		hkRigidBody* rigidBody = static_cast<hkRigidBody*>(entity);

		hkUlong id = getId( rigidBody );

		// remove the geometry from the displayHandler
		m_displayHandler->removeGeometry(id, m_tag);

		// remove the id from the list of 'owned' created entities
		const int index = wed->entitiesCreated.indexOf(id);
		if(index >= 0)
		{
			wed->entitiesCreated.removeAt(index);
		}
	}
}

void hkShapeDisplayViewer::synchronizeTransforms(hkDebugDisplayHandler* displayHandler, hkWorld* world )
{
	hkReal timeForDisplay = m_timeForDisplay > 0 ? m_timeForDisplay : world->getCurrentTime() ;

	// update the transform for all active entities (in all the active simulation islands)
	{
		const hkArray<hkSimulationIsland*>& activeIslands = world->getActiveSimulationIslands();

		for(int i = 0; i < activeIslands.getSize(); i++)
		{
			const hkArray<hkEntity*>& activeEntities = activeIslands[i]->getEntities();
			for(int j = 0; j < activeEntities.getSize(); j++)
			{
				hkRigidBody* rigidBody = static_cast<hkRigidBody*>(activeEntities[j]);
				hkUlong id = getId( rigidBody );
				
				hkTransform transform;
				rigidBody->approxTransformAt( timeForDisplay, transform );
				displayHandler->updateGeometry(transform, id, m_tag);
			}
		}
	}

	// update the transform for all inactive entities (in all the inactive simulation islands)
	if(0)
	{
		const hkArray<hkSimulationIsland*>& inactiveIslands = world->getInactiveSimulationIslands();

		for(int i = 0; i < inactiveIslands.getSize(); i++)
		{
			const hkArray<hkEntity*>& inactiveEntities = inactiveIslands[i]->getEntities();
			for(int j = 0; j < inactiveEntities.getSize(); j++)
			{
				hkRigidBody* rigidBody = static_cast<hkRigidBody*>(inactiveEntities[j]);
				hkUlong id = getId( rigidBody );
				displayHandler->updateGeometry(rigidBody->getTransform(), id, m_tag);
			}
		}
	}
}

void hkShapeDisplayViewer::synchronizeTransforms(hkWorld* world )
{
	synchronizeTransforms( m_displayHandler, world );
}

void hkShapeDisplayViewer::postSimulationCallback( hkWorld* world )
{
	HK_TIMER_BEGIN("hkShapeDisplayViewer", this);

	if ( !m_enableShapeTransformUpdate)
	{
		HK_TIMER_END();
		return;
	}

	synchronizeTransforms( m_displayHandler, world );

	HK_TIMER_END();
}

void hkShapeDisplayViewer::removeAllGeometries(int worldIndex)
{
	WorldToEntityData* wed = m_worldEntities[worldIndex];
	for(int i = 0; i < wed->entitiesCreated.getSize(); i++)
	{
		m_displayHandler->removeGeometry(wed->entitiesCreated[i], m_tag);
	}
	wed->entitiesCreated.setSize(0);
}

int hkShapeDisplayViewer::findWorld( hkWorld* world )
{
	int ne = m_worldEntities.getSize();
	for (int i=0; i < ne; ++i)
	{
		if (m_worldEntities[i]->world == world)
			return i;
	}
	return -1;
}

void hkShapeDisplayViewer::removeWorld( int i )
{
	m_worldEntities[i]->world->markForWrite();
		m_worldEntities[i]->world->removeEntityListener( this );
		m_worldEntities[i]->world->removeWorldPostSimulationListener( this );
		removeAllGeometries(i);
	m_worldEntities[i]->world->unmarkForWrite();

	delete m_worldEntities[i];
	m_worldEntities.removeAt(i);
	// other base listeners handled in worldviewerbase
}

void hkShapeDisplayViewer::inactiveEntityMovedCallback( hkEntity* entity )
{
	hkRigidBody* rigidBody = static_cast<hkRigidBody*>(entity);
	hkUlong id = getId( rigidBody );
	m_displayHandler->updateGeometry(rigidBody->getTransform(), id, m_tag);
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
