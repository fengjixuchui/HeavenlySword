/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkserialize/hkSerialize.h>
#include <hkserialize/packfile/hkPackfileWriter.h>
#include <hkserialize/serialize/hkRelocationInfo.h>
#include <hkserialize/util/hkVtableClassRegistry.h>
#include <hkserialize/packfile/binary/hkPackfileSectionHeader.h>
#include <hkserialize/serialize/platform/hkPlatformObjectWriter.h>
#include <hkserialize/util/hkBuiltinTypeRegistry.h>
#include <hkbase/stream/impl/hkOffsetOnlyStreamWriter.h>
#include <hkbase/config/hkConfigVersion.h>

extern const hkClass hkClassClass;
hkPackfileWriter::SectionTag hkPackfileWriter::SECTION_TAG_DATA = "__data__";
hkPackfileWriter::SectionTag hkPackfileWriter::SECTION_TAG_TYPES = "__types__";

hkPackfileWriter::hkPackfileWriter()
	:	m_contentsPtrPWIndex(-1),
		m_contentsClassPWIndex(-1),
		m_numDataInstances(0),
		m_numClassInstances(0)
{
	// Don't store these classes to avoid the chicken & egg problem.
	// We know their definitions from the fileVersion number.
	m_knownObjects.insert( &hkClassEnumClass, INDEX_IGNORE );
	m_knownObjects.insert( &hkClassMemberClass, INDEX_IGNORE );
	m_knownObjects.insert( &hkClassClass, INDEX_IGNORE );
}

hkPackfileWriter::~hkPackfileWriter()
{
	// assumes all m_knownSections are from a hkString::strDup()
	for (int c=0; c < m_knownSections.getSize(); ++c)
	{
		hkDeallocate<char>( m_knownSections[c] );
	}
}

void hkPackfileWriter::setContentsWithRegistry( const void* pointer, const hkClass& klass, const hkVtableClassRegistry* classRegistry, AddObjectListener* addListener)
{
	HK_ASSERT2( 0xb7b45a9d, m_pendingWrites.getSize() == 0,
			"You can't reuse hkPackfileWriters. Please make a new one for each setContents call.");
	// we want to record where is object ended up. 
	addObject( pointer, klass, classRegistry, addListener, SECTION_TAG_DATA );
	
	m_contentsPtrPWIndex = m_knownObjects.getWithDefault( pointer, -1 );
	m_contentsClassPWIndex = m_knownObjects.getWithDefault( &klass, -1 );
}						  

void hkPackfileWriter::setContents( const void* object, const hkClass& klass, AddObjectListener* addListen )
{
	setContentsWithRegistry(object, klass, hkBuiltinTypeRegistry::getInstance().getVtableClassRegistry(), addListen);
}

void hkPackfileWriter::addImport( const void* object, const char* id )
{
	m_knownObjects.insert( object, INDEX_IMPORT );
	m_imports.insert( object, id );
}

void hkPackfileWriter::addExport( const void* object, const char* id )
{
	HK_ASSERT2( 0, m_knownObjects.getWithDefault( object, INDEX_ERROR) != INDEX_ERROR, "Cannot export an unknown object");
	m_exports.insert( object, id );
}

hkUint32 hkPackfileWriter::sectionTagToIndex( SectionTag sectionTag )
{
	int index;
	if( m_sectionTagToIndex.get(sectionTag, &index) != HK_SUCCESS )
	{
		char* key = hkString::strDup(sectionTag); // this is a hkAllocate<char>
		index = m_knownSections.getSize();
		m_sectionTagToIndex.insert( key, index );
		m_knownSections.pushBack( key );
	}
	return hkUint32(index);
}

hkUint32 hkPackfileWriter::findSectionFor(const void* pointer, const hkClass& klass, SectionTag sectionTag )
{
	hkUint32 sectionIndex = m_sectionOverrideByPointer.getWithDefault( pointer, hkUint32(-1) );

	if( sectionIndex == hkUint32(-1) )
	{
		const hkClass* k = &klass;
		while( 1 )
		{
			sectionIndex = m_sectionOverrideByType.getWithDefault( k->getName(), hkUint32(-1) );
			if( sectionIndex == hkUint32(-1) )
			{
				k = k->getParent();  // check parent
				if( k == HK_NULL ) // end of the line
				{
					if ( hkString::strCmp(klass.getName(),hkClassClass.getName()) == 0 )
					{
						sectionIndex = sectionTagToIndex(SECTION_TAG_TYPES);
					}
					if ( hkString::strCmp(klass.getName(),hkClassEnumClass.getName()) == 0 )
					{
						sectionIndex = sectionTagToIndex(SECTION_TAG_TYPES);
					}
					if ( hkString::strCmp(klass.getName(),hkClassMemberClass.getName()) == 0 )
					{
						sectionIndex = sectionTagToIndex(SECTION_TAG_TYPES);
					}
					else
					{
						sectionIndex = sectionTagToIndex(sectionTag); // always returns valid index
					}
					break;
				}
			}
			else
			{
				break; // got one
			}
		}
	}
	HK_ASSERT2(0x48ba6115, sectionIndex != hkUint32(-1), "Could not find section for requested pointer.");
	return sectionIndex;
}

static const hkClass* getExactClass(const void* instance, const hkClass& base, const hkVtableClassRegistry* reg )
{
	if ( base.hasVtable() )
	{
		return reg ? reg->getClassFromVirtualInstance(instance) : HK_NULL;
	}
	return &base;
}

void hkPackfileWriter::addPendingWrite( const void* pointer, const hkClass& klass, const void* pointerIn, const hkClass& klassIn, SectionTag sectionTag )
{
	PendingWrite data;
	data.m_pointer = pointer;
	data.m_klass = &klass;
	data.m_origPointer = pointerIn;
	data.m_origClass = &klassIn;
	data.m_sectionIndex = findSectionFor(pointerIn, klassIn, sectionTag);
	data.m_dataSize = -1;
	m_pendingWrites.pushBack( data );

	m_numClassInstances += (&klass == &hkClassClass) ? 1 : 0;
	m_numDataInstances  += (&klass == &hkClassClass) ? 0 : 1;
}

static void chasePointers( const void* pointer, const hkClass& klass, hkRelocationInfo& reloc)
{
	// Chase any referenced data
	// We do a fake save and look at the global fixups for pointers to chase.
	hkStructureLayout layout;
	hkPlatformObjectWriter writer( layout );

	hkOffsetOnlyStreamWriter stream;
	writer.writeObject( &stream, pointer, klass, reloc);
}

int hkPackfileWriter::notDuplicateMetaData(const void* pointer, const hkClass* klass)
{
	if( hkString::strCmp(klass->getName(),"hkClass") == 0 )
	{
		const hkClass* thisOne = static_cast<const hkClass*>(pointer);
		if( const hkClass* previous = m_knownClasses.getWithDefault(thisOne->getName(), HK_NULL) )
		{
			HK_ASSERT (0xe1493f15, thisOne != previous );
			HK_ASSERT (0x4d7ecdcd, m_knownObjects.hasKey(previous) );
			if( thisOne->getSignature() != previous->getSignature() )
			{
				HK_ERROR( 0x2518721c, "Conflicting metadata found. Perhaps you have called setContents "\
						"on data which has not been updated to the current version.");
			}
			m_knownObjects.insert(pointer, m_knownObjects.getWithDefault(previous,-1) );
			m_replacements.insert(pointer, previous);
			return 0;
		}
		else
		{
			m_knownClasses.insert(thisOne->getName(), thisOne);
			return 1;
		}
	}
	else if( hkString::strCmp(klass->getName(),"hkClassEnum") == 0 )
	{
		const hkClassEnum* thisOne = static_cast<const hkClassEnum*>(pointer);
		if( const hkClassEnum* previous = m_knownEnums.getWithDefault(thisOne->getName(), HK_NULL) )
		{
			HK_ASSERT(0, thisOne != previous );
			HK_ASSERT(0, thisOne->getSignature() == previous->getSignature() );
			HK_ASSERT(0, m_knownObjects.hasKey(previous) );
			m_knownObjects.insert(pointer, m_knownObjects.getWithDefault(previous,-1) );
			m_replacements.insert(pointer, previous);
			return 0;
		}
		else
		{
			m_knownEnums.insert(thisOne->getName(), thisOne);
			return 1;
		}
	}
	return 1;
}

void hkPackfileWriter::addObject( const void* pointerIn, const hkClass& klassIn, const hkVtableClassRegistry* classRegistry, AddObjectListener* addListener, SectionTag sectionTag )
{
	HK_ASSERT( 0x45364543, pointerIn );
	HK_ASSERT( 0x45364544, &klassIn );

	// do we already have it?
	if( m_knownObjects.hasKey(pointerIn) == false )
	{
		if( const hkClass* derivedClass = getExactClass( pointerIn, klassIn, classRegistry ) )
		{
			const void* pointer = pointerIn;
			const hkClass* klass = derivedClass;

			if( addListener )
			{
				addListener->addObjectCallback( pointer, klass );
			}

			if( pointer != HK_NULL )
			{
				HK_ASSERT2( 0x6087f47e, klass != HK_NULL, "Trying to add an object with no class in packfile writer.");
				if( notDuplicateMetaData( pointer, klass )
				&& (pointerIn == pointer || m_knownObjects.hasKey(pointer)==false) ) // make sure replacement is not already known
				{
					// remember that we've encountered this pointer
					m_knownObjects.insert( pointerIn, m_pendingWrites.getSize() );
					if( pointer != pointerIn )
					{
						m_knownObjects.insert( pointer, m_pendingWrites.getSize() );
						m_replacements.insert( pointer, pointerIn );
					}

					// add write
					addPendingWrite( pointer, *klass, pointerIn, klassIn, sectionTag );

					// add all pointed objects
					hkRelocationInfo relocs;
					chasePointers( pointer, *klass, relocs );
					for( int i = 0; i < relocs.m_global.getSize(); ++i )
					{
						const void* ptr = relocs.m_global[i].m_toAddress;
						const hkClass* k = relocs.m_global[i].m_toClass;
						if( k )
						{
							addObject( ptr, *k, classRegistry, addListener, sectionTag );
						}
					}

					// add metadata
					addObject( klass, hkClassClass, classRegistry, addListener, SECTION_TAG_TYPES );
				}
			}
			else
			{
				m_knownObjects.insert( pointerIn, INDEX_IGNORE );
			}
		}
		else
		{
			HK_WARN(0x96846812, "Found an un-registered class derived from " << klassIn.getName()
				<< ". Derived class will not be serialized unless added to class registry.\n"
				<< "All saved pointers to this object will be set to NULL.\n"
				<< "Saved file will not generate warnings (or asserts) on load but NULL pointers may cause runtime crashes.");
			m_knownObjects.insert( pointerIn, INDEX_IGNORE );
			hkVariant v = { const_cast<void*>(pointerIn), &klassIn };
			m_objectsWithUnregisteredClass.pushBack( v );
		}
	}
}


void hkPackfileWriter::addSection(SectionTag sectionTag)
{
	sectionTagToIndex(sectionTag);
}

void hkPackfileWriter::setSectionForPointer( const void* ptr, SectionTag sectionTag )
{
	m_sectionOverrideByPointer.insert(ptr, sectionTagToIndex(sectionTag));
}

void hkPackfileWriter::setSectionForClass( const hkClass& k, SectionTag sectionTag )
{
	m_sectionOverrideByType.insert(k.getName(), sectionTagToIndex(sectionTag) );
}

void hkPackfileWriter::getCurrentVersion(char* buf, int bufLen)
{
	hkString::snprintf(buf, bufLen, "Havok-%i.%i.%i-%s", HAVOK_SDK_VERSION_MAJOR, HAVOK_SDK_VERSION_MINOR, HAVOK_SDK_VERSION_POINT, HAVOK_SDK_VERSION_RELEASE );
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
