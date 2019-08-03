/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkserialize/hkSerialize.h>
#include <hkbase/htl/hkStringMap.h>
#include <hkbase/htl/hkPointerMap.h>
#include <hkserialize/packfile/hkPackfileReader.h>
#include <hkserialize/copier/hkObjectCopier.h>
#include <hkserialize/serialize/hkRelocationInfo.h>
#include <hkserialize/util/hkBuiltinTypeRegistry.h>
#include <hkserialize/util/hkClassNameRegistry.h>
#include <hkserialize/version/hkVersionRegistry.h>
#include <hkserialize/packfile/binary/hkPackfileHeader.h>
#include <hkbase/stream/hkStreamReader.h>

#if 0 && defined(HK_DEBUG)
#	include <hkbase/fwd/hkcstdio.h>
	using namespace std;
#	define TRACE(A) A
#else
#	define TRACE(A) // nothing
#endif

extern const hkClass hkClassVersion1Class;

namespace
{
	enum
	{
		HK_METADATA_UPDATE_NONE = 0,
		HK_METADATA_UPDATE_CLASS1_VERSION = 1,
		HK_METADATA_UPDATE_TYPE_CSTRING = HK_METADATA_UPDATE_CLASS1_VERSION << 1
	};

	static void updatePointerCharToCString( hkClassMember& klassMem )
	{
		if (klassMem.getType() == hkClassMember::TYPE_POINTER
			&& klassMem.getSubType() == hkClassMember::TYPE_CHAR)
		{
			// PRINT(("\t%s", klassMem.getName()));
			klassMem.setType(hkClassMember::TYPE_CSTRING);
			klassMem.setSubType(hkClassMember::TYPE_VOID);
		}
		/*
		//
		// this code is just for demonstration purposes only
		// as an example of how to identify old way of
		// c-strings array declaration
		// SIMPLEARRAY / POINTER -> special "STRUCT"
		// with one 'char*' member POINTER / CHAR
		//
		// The new way to declare c-strings array is
		// SIMPLEARRAY / CSTRING
		// no need to specify a dummy structure anymore,
		// just use 'char**' member inplace
		//
		else if (klassMem.getType() == hkClassMember::TYPE_SIMPLEARRAY
		&& klassMem.getSubType() == hkClassMember::TYPE_POINTER
		&& klassMem.hasClass() == true)
		{
		const hkClass& stringStruct = klassMem.getStructClass();
		if (stringStruct.getNumDeclaredMembers() == 1
		&& stringStruct.getParent() == HK_NULL
		&& stringStruct.hasVtable() == false
		&& stringStruct.getDeclaredMember(0).hasClass() == false
		&& ( stringStruct.getDeclaredMember(0).getType() == hkClassMember::TYPE_POINTER
		&& stringStruct.getDeclaredMember(0).getSubType() == hkClassMember::TYPE_CHAR
		|| stringStruct.getDeclaredMember(0).getType() == hkClassMember::TYPE_CSTRING
		&& stringStruct.getDeclaredMember(0).getSubType() == hkClassMember::TYPE_VOID))
		{
		const hkClassMember& structMem = stringStruct.getDeclaredMember(0);

		HK_ASSERT(0, (structMem.getType() == hkClassMember::TYPE_POINTER
		&& structMem.getSubType() == hkClassMember::TYPE_CHAR) );

		HK_ASSERT(0, (structMem.getType() == hkClassMember::TYPE_CSTRING
		&& structMem.getSubType() == hkClassMember::TYPE_VOID) );

		klassMem.setSubType(hkClassMember::TYPE_CSTRING);
		}
		}
		*/
	}

	static void updatePointerCharToCString( hkClass* classInOut, hkPackfileReader::UpdateFlagFromClassMap& updateFlagFromClass )
	{
		hkInt32 updateFlags = updateFlagFromClass.getWithDefault( classInOut, HK_METADATA_UPDATE_NONE );
		
		if( (updateFlags & HK_METADATA_UPDATE_TYPE_CSTRING) == 0 )
		{
			updateFlagFromClass.insert( classInOut, updateFlags | HK_METADATA_UPDATE_TYPE_CSTRING );

			for (int i = 0; i < classInOut->getNumDeclaredMembers(); i++)
			{
				const hkClassMember& klassMem = classInOut->getDeclaredMember(i);
				if (klassMem.hasClass())
				{
					updatePointerCharToCString( const_cast<hkClass*>( &klassMem.getStructClass() ), updateFlagFromClass );
				}
				// PRINT(("-+-+- %s (0x%p):", klass.getName(), &klass));
				updatePointerCharToCString( *const_cast<hkClassMember*>( &klassMem ) );
				// PRINT(("\n", klass.getName()));
			}

			if (classInOut->getParent() != HK_NULL)
			{
				updatePointerCharToCString( classInOut->getParent(), updateFlagFromClass );
			}
		}
	}
	
	static void updateClassVersion1Inplace( hkClass* classInOut, hkPackfileReader::UpdateFlagFromClassMap& updateFlagFromClass )
	{
		hkInt32 updateFlags = updateFlagFromClass.getWithDefault( classInOut, HK_METADATA_UPDATE_NONE );

		if( (updateFlags & HK_METADATA_UPDATE_CLASS1_VERSION) == 0 )
		{
			updateFlagFromClass.insert( classInOut, updateFlags | HK_METADATA_UPDATE_CLASS1_VERSION );

			int voff = hkClassVersion1Class.getMemberByName("hasVtable")->getOffset();
			hkClass* k = classInOut;
			while( k->getParent() != HK_NULL )
			{
				*reinterpret_cast<void**>( reinterpret_cast<char*>(k) + voff) = HK_NULL;
				k = const_cast<hkClass*>( k->getParent() );
			}
			hkBool hasVtable = *(reinterpret_cast<char*>(k) + voff) != 0;
			if( hasVtable )
			{
				int ioff = hkClassVersion1Class.getMemberByName("numImplementedInterfaces")->getOffset();
				*reinterpret_cast<int*>( reinterpret_cast<char*>(k) + ioff ) += 1;
			}
			*reinterpret_cast<void**>( reinterpret_cast<char*>(k) + voff) = HK_NULL;
		
			for (int i = 0; i < classInOut->getNumDeclaredMembers(); i++)
			{
				const hkClassMember& klassMem = classInOut->getDeclaredMember(i);
				if (klassMem.hasClass())
				{
					updateClassVersion1Inplace( const_cast<hkClass*>(&klassMem.getStructClass()), updateFlagFromClass );
				}
			}

			if (classInOut->getParent() != HK_NULL)
			{
				updateClassVersion1Inplace( classInOut->getParent(), updateFlagFromClass );
			}
		}
	}
}
void* hkPackfileReader::getContents( const char* className )
{
	hkBuiltinTypeRegistry& builtin = hkBuiltinTypeRegistry::getInstance();
	return getContentsWithRegistry( className, builtin.getFinishLoadedObjectRegistry() );
}

void hkPackfileReader::updateMetaDataInplace( hkClass* classInOut, int fileVersion, UpdateFlagFromClassMap& updateFlagFromClass )
{
	if( fileVersion == 1 )
	{
		updateClassVersion1Inplace( classInOut, updateFlagFromClass );
	}
	if( fileVersion < 4 )
	{
		updatePointerCharToCString( classInOut, updateFlagFromClass );
	}
}

hkEnum<hkPackfileReader::FormatType,hkInt32> HK_CALL hkPackfileReader::detectFormat( hkStreamReader* reader )
{
	// Peek at first few bytes to check if the file is binary
	char buf[sizeof(hkPackfileHeader)];
	reader->setMark( sizeof(buf) );
	if( reader->read( buf, sizeof(buf)) != sizeof(buf) )
	{
		return FORMAT_ERROR; // could not read header
	}
	if( reader->rewindToMark() != HK_SUCCESS )
	{
		HK_WARN( 0x2fbceba8, "Could not rewind after peeking at file header.");
	}
	hkPackfileHeader actual; // the default magic values.
	hkPackfileHeader* loaded = reinterpret_cast<hkPackfileHeader*>(buf);
	if( (loaded->m_magic[0] == actual.m_magic[0]) && (loaded->m_magic[1] == actual.m_magic[1]) )
	{
		return FORMAT_BINARY;
	}

	char* p = buf;
	char* pend = p + sizeof(buf);
	for( ; p != pend && p[0] != '<'; ++p )
	{
	}
	if( p != pend )
	{
		// old pre 3.1 files are missing root tag
		const char* tags[] = { "<?xml", "<hkpackfile", "<hksection", HK_NULL };
		for(int i = 0; tags[i]; ++i )
		{
			int n = hkString::strLen(tags[i]);
			n = n < pend-p ? n : int(pend-p);
			if( hkString::strNcmp(p, tags[i], n) == 0 )
			{
				return FORMAT_XML;
			}
		}
	}
	return FORMAT_UNKNOWN;
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
