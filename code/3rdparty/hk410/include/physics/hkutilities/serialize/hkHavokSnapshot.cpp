/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/stream/hkIstream.h>
#include <hkbase/stream/hkOstream.h>
#include <hkbase/stream/hkStreamReader.h>
#include <hkbase/class/hkClass.h>
#include <hkmath/hkMath.h>
#include <hkdynamics/hkDynamics.h>
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkutilities/serialize/hkPhysicsData.h>
#include <hkutilities/serialize/hkHavokSnapshot.h>

#include <hkserialize/packfile/binary/hkBinaryPackfileReader.h>
#include <hkserialize/packfile/binary/hkBinaryPackfileWriter.h>
#include <hkserialize/packfile/xml/hkXmlPackfileReader.h>
#include <hkserialize/packfile/xml/hkXmlPackfileWriter.h>
#include <hkserialize/util/hkRootLevelContainer.h>
#include <hkserialize/util/hkLoader.h>
#include <hkcollide/shape/storagemesh/hkStorageMeshShape.h>
#include <hkcollide/shape/storagesampledheightfield/hkStorageSampledHeightFieldShape.h>

hkPhysicsData* HK_CALL hkHavokSnapshot::load(hkStreamReader* reader, hkPackfileReader::AllocatedData** allocatedData)
{
	HK_ASSERT2(0x74de3808, reader, "Null hkStreamReader pointer was passed to hkHavokSnapshot::load");
	HK_ASSERT2(0x54c68870, allocatedData, "Null hkPackfileReader::AllocatedData pointer was passed to hkHavokSnapshot::load");

	// Fail quickly if null pointers were given
	if ( (!reader) || (!allocatedData) )
	{
		return HK_NULL;
	}

	hkLoader loader;
	hkRootLevelContainer* container = loader.load( reader );

	if (!container)
	{
		HK_WARN(0x764219fe, "Could not load a hkRootLevelContainer from given stream.");
		return HK_NULL;
	}

	// first search by type
	hkPhysicsData* data = static_cast<hkPhysicsData*>( container->findObjectByType( hkPhysicsDataClass.getName()) );
	if(data == HK_NULL)
	{
		// failing that, by variant name.
		const char* byName[] = { "SnapshotSave", "hkPhysicsData", HK_NULL };
		for( int i = 0; byName[i] != HK_NULL; ++i )
		{
			data = static_cast<hkPhysicsData*>( container->findObjectByName( byName[i] ) );
			if( data )
			{
				break;
			}
		}
	}
	
	if(data != HK_NULL)
	{
		*allocatedData = loader.m_loadedData[0];
		(*allocatedData)->addReference();
	}

	return data;
}

hkHavokSnapshot::ConvertListener::~ConvertListener()
{
	for( int i = 0; i < m_objects.getSize(); ++i )
	{
		delete m_objects[i];
	}
}

void hkHavokSnapshot::ConvertListener::addObjectCallback( ObjectPointer& p, ClassPointer& k )
{
	if( hkMeshShapeClass.isSuperClass(*k) && k != &hkStorageMeshShapeClass )
	{
		const hkMeshShape* mesh = static_cast<const hkMeshShape*>(p);
		hkStorageMeshShape* storage = new hkStorageMeshShape(mesh);
		m_objects.pushBack(storage);

		p = storage;
		k = &hkStorageMeshShapeClass;
	}
	else if( hkSampledHeightFieldShapeClass.isSuperClass(*k) && k != &hkStorageSampledHeightFieldShapeClass )
	{
		const hkSampledHeightFieldShape* sampled = static_cast<const hkSampledHeightFieldShape*>(p);
		hkShape* storage = new hkStorageSampledHeightFieldShape(sampled);
		m_objects.pushBack(storage);

		p = storage;
		k = &hkStorageSampledHeightFieldShapeClass;
	}
	else if( hkRigidBodyClass.isSuperClass(*k) )
	{
		const hkRigidBody* body = static_cast<const hkRigidBody*>(p);
		if( hkWorld* world = body->getWorld() )
		{
			if( world->getFixedRigidBody() == body )
			{
				p = HK_NULL;
				k = HK_NULL;
			}
		}
	}
}


hkBool HK_CALL hkHavokSnapshot::save(const hkWorld* world, hkStreamWriter* writer, hkBool binaryFormat, const hkStructureLayout::LayoutRules* targetLayout)
{
	// Note: because hkPhysicsData adds a ref to all rbs in the world, and removes the ref
	// on destruction, we have to:
	// Mark the world for write
	// Cast away the const of the world, so we can do this
	// Scope the hkPhysics data so that it goes out of scope and removes refs while the world
	// is still marked for write

	HK_ASSERT2(0x4bb93313, world, "Null hkWorld pointer passed to hkHavokSnapshot::save.");
	HK_ASSERT2(0x23ec02e2, writer, "Null hkStreamWriter passed to hkHavokSnapshot::save.");

	// Fail if any null pointers were given
	if ( (!world) || (!writer) )
	{
		return false;
	}

	hkWorld* mutableWorld = const_cast<hkWorld*>(world);
	hkBool ret;
	mutableWorld->markForWrite();
	{
		// Make a data struct to contain the world info.
		hkPhysicsData data;
		data.populateFromWorld( mutableWorld );

		ret = save( &data, writer, binaryFormat, targetLayout );
	}
	mutableWorld->unmarkForWrite();

	return ret;
}

hkBool HK_CALL hkHavokSnapshot::save( const hkPhysicsData* data, hkStreamWriter* writer, hkBool binaryFormat, const hkStructureLayout::LayoutRules* targetLayout)
{
	// Add this to our root level container object
	return saveUnderRootLevel(data, hkPhysicsDataClass, writer, binaryFormat, targetLayout); 
}

hkBool HK_CALL hkHavokSnapshot::saveUnderRootLevel( const void* data, const hkClass& dataClass, hkStreamWriter* writer, hkBool binaryFormat, const hkStructureLayout::LayoutRules* targetLayout )
{
	//assume data is the raw data, so create a named variant out of it
	hkRootLevelContainer::NamedVariant genericData(const_cast<char *>(dataClass.getName()), const_cast<void *>(data), &dataClass);

	hkRootLevelContainer container;
	container.m_numNamedVariants = 1;
	container.m_namedVariants = &genericData;

	return save(&container, hkRootLevelContainerClass, writer, binaryFormat, targetLayout);
}

hkBool HK_CALL hkHavokSnapshot::save( const void* data, const hkClass& dataClass, hkStreamWriter* writer, hkBool binaryFormat, const hkStructureLayout::LayoutRules* targetLayout)
{
	bool res = false;
	if (writer)
	{
		// Make an appropriate writer
		hkPackfileWriter* pw;
		if (binaryFormat)
			pw = new hkBinaryPackfileWriter;
		else
			pw = new hkXmlPackfileWriter;

		ConvertListener convertListener;

		// Set the top level struct
 		pw->setContents(data, dataClass, &convertListener );
		
		// Save it.
		hkPackfileWriter::Options o;
		union { hkUint32 i; char c[4]; } u;
		u.c[0] = 's'; u.c[1] = 'n'; u.c[2] = 'a'; u.c[3] = 'p';
		o.m_userTag = u.i;
		if (targetLayout)
			o.m_layout = hkStructureLayout( *targetLayout );

		res = pw->save( writer, o ) == HK_SUCCESS;
		pw->removeReference();
	}

	return res;
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
