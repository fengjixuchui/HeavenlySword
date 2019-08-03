/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkserialize/hkSerialize.h>
#include <hkserialize/util/hkLoader.h>

#include <hkbase/stream/hkIstream.h>
#include <hkbase/stream/hkStreamReader.h>

// Serialize includes
#include <hkserialize/util/hkRootLevelContainer.h>
#include <hkserialize/packfile/binary/hkBinaryPackfileReader.h>
#include <hkserialize/packfile/binary/hkPackfileHeader.h>
#include <hkserialize/packfile/xml/hkXmlPackfileReader.h>
#include <hkserialize/util/hkBuiltinTypeRegistry.h>
#include <hkserialize/version/hkVersionRegistry.h>
#include <hkserialize/version/hkVersionUtil.h>

hkLoader::~hkLoader()
{
	// We destruct all of the objects first in case the objects in one
	// packfile refer to those in another.
	for (int i=0; i < m_loadedData.getSize(); i++)
	{
		if (m_loadedData[i]->getReferenceCount() == 1)
		{
			m_loadedData[i]->callDestructors();
		}
	}
	for (int i=0; i < m_loadedData.getSize(); i++)
	{
		m_loadedData[i]->removeReference();
	}
	m_loadedData.setSize(0);
}

hkRootLevelContainer* hkLoader::load( const char* filename )
{
	return load( filename,  		
		hkBuiltinTypeRegistry::getInstance().getFinishLoadedObjectRegistry(),
		&hkVersionRegistry::getInstance() );
}

hkRootLevelContainer* hkLoader::load( class hkStreamReader* streamIn )
{
	return load( streamIn,  		
		hkBuiltinTypeRegistry::getInstance().getFinishLoadedObjectRegistry(),
		&hkVersionRegistry::getInstance() ); 
}

hkRootLevelContainer* hkLoader::load( const char* filename, hkFinishLoadedObjectRegistry* finish, hkVersionRegistry* version )
{
	return static_cast<hkRootLevelContainer*>( load( filename, hkRootLevelContainerClass, finish, version ) );
}

hkRootLevelContainer* hkLoader::load( class hkStreamReader* streamIn, hkFinishLoadedObjectRegistry* finish, hkVersionRegistry* version )
{
	return static_cast<hkRootLevelContainer*>( load( streamIn, hkRootLevelContainerClass, finish, version ) );
}

void* hkLoader::load( const char* filename, const hkClass& expectedTopLevelClass )
{
	return load(filename,
		expectedTopLevelClass,
		hkBuiltinTypeRegistry::getInstance().getFinishLoadedObjectRegistry(),
		&hkVersionRegistry::getInstance() );
}

void* hkLoader::load( class hkStreamReader* reader, const hkClass& expectedTopLevelClass )
{
	return load(reader,
		expectedTopLevelClass,
		hkBuiltinTypeRegistry::getInstance().getFinishLoadedObjectRegistry(),
		&hkVersionRegistry::getInstance() );
}

void* hkLoader::load( const char* filename, const hkClass& expectedClass, hkFinishLoadedObjectRegistry* finish, hkVersionRegistry* version )
{
	hkIfstream fileIn(filename);
	if (fileIn.isOk())
	{
		return load( fileIn.getStreamReader(), expectedClass, finish, version );
	}
	
	HK_WARN(0x5e543234, "Unable to open file " << filename);
	return HK_NULL;
}

void* hkLoader::load( class hkStreamReader* streamIn, const hkClass& expectedClass, hkFinishLoadedObjectRegistry* finish, hkVersionRegistry* version )
{
	hkPackfileReader* reader = HK_NULL;
	// Create the appropriate file reader
	switch( hkPackfileReader::detectFormat( streamIn ) )
	{
		case hkPackfileReader::FORMAT_BINARY:
			reader = new hkBinaryPackfileReader();
			break;
		case hkPackfileReader::FORMAT_XML:
			reader = new hkXmlPackfileReader();
			break;
		default:
			HK_WARN(0x5ef4a322, "Unable to load from stream ");
			return HK_NULL;
	}

	// Load the entire file
	if (reader->loadEntireFile( streamIn ) != HK_SUCCESS)
	{
		HK_WARN(0x5ef4a321, "Unable to load from stream ");
		return HK_NULL;
	}

	if( version && ( hkVersionUtil::updateToCurrentVersion( *reader, *version ) != HK_SUCCESS ) )
	{
		HK_WARN( 0x2ba02a15, "Unable to version contents, trying to continue anyway");
	}

	// Try to continue even if versioning failed.
	void* contents = reader->getContentsWithRegistry( expectedClass.getName(), finish );
	if( contents != HK_NULL )
	{
		// Get a handle to the memory allocated by the reader, and add a reference to it.
		// This allows us to delete the reader without destroying the loaded data
		hkPackfileData* packfileData = reader->getPackfileData();
		packfileData->addReference();
		m_loadedData.pushBack(packfileData);
	}

	// We can now delete the reader. Note we still keep a reference to the allocated data.
	delete reader;

	return contents;
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
