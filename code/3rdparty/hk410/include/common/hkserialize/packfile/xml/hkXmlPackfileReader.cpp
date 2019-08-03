/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkserialize/hkSerialize.h>
#include <hkserialize/packfile/xml/hkXmlPackfileReader.h>
#include <hkserialize/serialize/xml/hkXmlObjectReader.h>
#include <hkserialize/serialize/hkRelocationInfo.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkserialize/util/xml/hkXmlParser.h>
#include <hkbase/htl/hkPointerMap.h>
#include <hkbase/stream/hkStreamReader.h>
#include <hkbase/stream/impl/hkLineNumberStreamReader.h>
#include <hkserialize/util/hkFinishLoadedObjectRegistry.h>
#include <hkserialize/util/hkStructureLayout.h>
#include <hkserialize/util/hkChainedClassNameRegistry.h>
#include <hkserialize/util/hkBuiltinTypeRegistry.h>
#include <hkserialize/util/hkPointerMultiMap.h>
#include <hkserialize/version/hkObjectUpdateTracker.h>
#include <hkserialize/version/hkVersionRegistry.h>
#include <hkserialize/version/hkPackfileObjectUpdateTracker.h>

extern const hkClass hkClassVersion1Class;

#if 0
#	include <hkbase/fwd/hkcstdio.h>
#	define TRACE(A) printf A
#else
#	define TRACE(A)
#endif

namespace
{
	struct StringPool : public hkReferencedObject
	{
		public:

			HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT);

			~StringPool()
			{
				for( hkStringMap<char*>::Iterator i = m_pool.getIterator();
					m_pool.isValid(i);
					i = m_pool.getNext(i) )
				{
					hkDeallocate( m_pool.getKey(i) );
				}
			}

			char* insert(const char* s)
			{
				char* r;
				if( m_pool.get( s, &r ) == HK_NULL )
				{
					return r;
				}
				else
				{
					r = hkString::strDup(s);
					m_pool.insert(r, r);
					return r;
				}
			}

			hkStringMap<char*> m_pool;
	};
}

class hkXmlPackfileUpdateTracker : public hkPackfileObjectUpdateTracker
{
	public:

		hkXmlPackfileUpdateTracker( hkPackfileData* data )
			: hkPackfileObjectUpdateTracker(data)
		{
		}

		int addPendingValue( void* address, int oldIndex )
		{
			return m_pointers.addPendingValue( address, oldIndex );
		}

		void realizePendingPointer( void* newObject, int startIndex )
		{
			int refIndex = startIndex;
			while( refIndex != -1 )
			{
				void* ptrToNew = m_pointers.getValue(refIndex);
				*reinterpret_cast<void**>(ptrToNew) = newObject;
				refIndex = m_pointers.getNextIndex(refIndex);
			}
			m_pointers.realizePendingPointer( newObject, startIndex );
		}
};

hkPackfileData* hkXmlPackfileReader::getPackfileData()
{
	return m_data;
}

hkArray<hkVariant>& hkXmlPackfileReader::getLoadedObjects()
{
	return m_loadedObjects;
}

hkObjectUpdateTracker& hkXmlPackfileReader::getUpdateTracker()
{
	return *m_tracker;
}

const char* hkXmlPackfileReader::getOriginalContentsVersion()
{
	return m_version;
}

hkXmlPackfileReader::hkXmlPackfileReader()
{
	// start reading into data section unless otherwise specified
	m_knownSections.pushBack( "__data__" );
	m_sectionTagToIndex.insert( m_knownSections.back(), 0 );
	m_data = new AllocatedData();
	m_version = HK_NULL;
	m_tracker = new hkXmlPackfileUpdateTracker(m_data);
}

hkXmlPackfileReader::~hkXmlPackfileReader()
{
	delete m_tracker;
	hkDeallocate<char>(m_version);
	m_data->removeReference();
}

const hkClass* hkXmlPackfileReader::getClassByName(
		const char* className,
		hkClassNameRegistry& classRegistry,
		hkStringMap<hkClass*>& partiallyLoadedClasses,
		hkPointerMap<const hkClass*, int>& offsetsRecomputed,
		int classVersion )
{
	hkStringMap<hkClass*>::Iterator it = partiallyLoadedClasses.findKey( className );
	if( partiallyLoadedClasses.isValid(it) ) // maybe recompute offsets from loaded instance
	{
		hkClass* c = partiallyLoadedClasses.getValue(it);
		partiallyLoadedClasses.remove(it);

		hkPackfileReader::updateMetaDataInplace( c, classVersion, m_updateFlagFromClass );
		
		hkStructureLayout layout;
		layout.computeMemberOffsetsInplace(*c, offsetsRecomputed);
		classRegistry.registerClass(c);
		return c;
	}
	else // loaded and computed already
	{
		return classRegistry.getClassByName(className);
	}
}

void hkXmlPackfileReader::handleInterObjectReferences(
	const char* newObjectName, // newly created object name
	void* newObject, // newly created object
	const hkRelocationInfo& reloc,
	const hkStringMap<void*>& nameToObject,
	hkStringMap<int>& unresolvedReferences )
{
	// fixup all pending references to this new object
	{
		hkStringMap<int>::Iterator iter = unresolvedReferences.findKey(newObjectName);
		if( unresolvedReferences.isValid(iter) )
		{
			int refIndex = unresolvedReferences.getValue(iter);
			m_tracker->realizePendingPointer( newObject, refIndex );
			unresolvedReferences.remove(iter);
		}
	}

	// remember any vtable fixups
	{
		if( reloc.m_finish.getSize() )
		{
			//HK_ASSERT(0x5519c131, reloc.m_finish.getSize() == 1 );
			const hkRelocationInfo::Finish& virt = reloc.m_finish[0];
			void* addr = static_cast<char*>(newObject) + virt.m_fromOffset;
			m_tracker->addFinish( addr, virt.m_className );
		}
	}
	
	// fix up any intra-file references in this object
	{
		for( int i = 0; i < reloc.m_imports.getSize(); ++i )
		{
			const hkRelocationInfo::Import& ext = reloc.m_imports[i];

			void* dest = HK_NULL;
			if( nameToObject.get(ext.m_identifier, &dest) == HK_SUCCESS )
			{
				// The external can be resolved immediately
				void* source = static_cast<char*>(newObject) + ext.m_fromOffset;
				
				// Add ourself to the list of objects pointing to dest
				m_tracker->objectPointedBy( dest, source );
			}
			else // pointed object not found yet, add it to pending list
			{
				int oldIndex = unresolvedReferences.getWithDefault(ext.m_identifier, -1);
				int newIndex = m_tracker->addPendingValue( static_cast<char*>(newObject) + ext.m_fromOffset, oldIndex );
				unresolvedReferences.insert( ext.m_identifier, newIndex );
			}
		}
	}
}

inline static hkResult readClassVersionAttributes(hkXmlParser::StartElement* startElement, int& classVersion, char**contentsVersion)
{
	const char* versionString;
	if ((versionString = startElement->getAttribute("classversion", HK_NULL)) != HK_NULL )
	{
		classVersion = hkString::atoi(versionString);
		switch( classVersion )
		{
			case 1:
			{
				*contentsVersion = hkString::strDup("Havok-3.0.0");
				break;
			}
			case 2:
			case 3:
			case 4:
			{
				const char* sdkVersionString = startElement->getAttribute("contentsversion", HK_NULL);
				*contentsVersion = ( sdkVersionString != HK_NULL)
					? hkString::strDup(sdkVersionString)
					: hkString::strDup("Havok-3.1.0");
				break;
			}
			default:
			{
				HK_ASSERT2(0x256758e6, 0, "Unsupported metadata version " << classVersion);
				return HK_FAILURE;
			}
		}
	}
	else
	{
		*contentsVersion = hkString::strDup("Havok-3.0.0");
	}
	return HK_SUCCESS;
}

hkResult hkXmlPackfileReader::loadEntireFile( hkStreamReader* rawStream )
{
	return loadEntireFileWithRegistry(rawStream, HK_NULL);
}

hkResult hkXmlPackfileReader::loadEntireFileWithRegistry( hkStreamReader* rawStream, const hkClassNameRegistry* originalReg )
{
	TRACE(("\nLOAD\n"));
	hkLocalArray<char> buf( 0x4000 ); // 16k default buffer size.

	bool classRegistryAutoDetected = false;
	int currentSectionIndex = 0;
	hkLineNumberStreamReader stream(rawStream);
	hkXmlParser parser;
	hkXmlObjectReader reader(&parser);
	hkRelocationInfo reloc;
	hkXmlParser::Node* node;
	StringPool stringPool;

	hkPointerMap<const hkClass*,int> offsetsRecomputed; // retargeted classes
	hkStringMap<hkClass*> partiallyLoadedClasses; // classes partially loaded
	hkStringMap<int> unresolvedReferences; // map of string id to reference chain index
	hkStringMap<void*> nameToObject; // map of string id to object address
	hkChainedClassNameRegistry classRegistry( originalReg );
	classRegistry.registerClass( &hkClassVersion1Class );
	classRegistry.registerClass( &hkClassEnumClass );

	nameToObject.insert("null", HK_NULL);
	int classVersion = 1;

	while( parser.nextNode(&node, &stream) == HK_SUCCESS )
	{
		if( hkXmlParser::StartElement* startElement = node->asStart() )
		{
			if( startElement->name == "hkobject")
			{
				// peek at the type of the object
				const hkClass* klass = HK_NULL;
				const char* objectName = HK_NULL;
				const char* exportName = HK_NULL;
				{
					const char* className = startElement->getAttribute("class", HK_NULL );
					HK_ASSERT2(0x6087f47f,className != HK_NULL, "Found XML element without a 'class' attribute." );
					klass = getClassByName( className, classRegistry, partiallyLoadedClasses, offsetsRecomputed, classVersion );

					objectName = startElement->getAttribute("name", HK_NULL );
					HK_ASSERT2(0x6088e48f,objectName!= HK_NULL, "Found XML element with no 'name' attribute." );
					HK_ASSERT2(0x6088f48f,klass!= HK_NULL, "Found '" << objectName << "' with an unregistered type '" << className << "'");
					objectName = stringPool.insert(objectName);

					TRACE(("OBJ %s %s\n", objectName, className));
					exportName = startElement->getAttribute("export", HK_NULL );
					if( exportName ) // keep this for later
					{
						char* name = hkString::strDup(exportName);
						exportName = name;
						m_data->addAllocation(name);
					}
					const char* signature = startElement->getAttribute("signature", HK_NULL );
					if( signature )
					{
						hkUint32 lsig = hkString::atoi(signature);
						hkUint32 msig = klass->getSignature();
						HK_ASSERT2( 0x26fd9749, msig == lsig, "metadata signature mismatch" );
					}
				}

				parser.putBack(node);
				node = HK_NULL;

				// read the object into a temporary buffer
				buf.clear();
				if( reader.readObject(&stream, buf, *klass, reloc ) != HK_SUCCESS )
				{
					return HK_FAILURE;
				}
				HK_ASSERT2(0x2dbfd9dd, reloc.m_global.getSize() == 0, "Object has read global relocations during XML read." );

				// copy it into final location and apply local fixups
				void* object = hkAllocate<char>(buf.getSize(), HK_MEMORY_CLASS_EXPORT);
				hkString::memCpy(object, buf.begin(), buf.getSize() );
				reloc.applyLocalAndGlobal(object); // actually only local
				m_data->addAllocation(object);

				if( exportName )
				{
					m_data->addExport( exportName, object );
				}

				// object is now created
				nameToObject.insert( objectName, object );
				handleInterObjectReferences( objectName, object, reloc, nameToObject, unresolvedReferences );

				if( hkString::strCmp(klass->getName(), "hkClass") == 0 )
				{
					hkClass* k = reinterpret_cast<hkClass*>(object);
					partiallyLoadedClasses.insert(k->getName(), k);
				}
				else
				{
					hkVariant& v = m_loadedObjects.expandOne();
					v.m_class = klass;
					v.m_object = object;
					if( currentSectionIndex == 0 && m_tracker->m_topLevelObject == HK_NULL)
					{
						// first data object is contents
						m_tracker->m_topLevelObject = v.m_object;
						m_tracker->m_topLevelClassName = v.m_class->getName();
					}
				}

				reloc.clear();
			}
			else if( startElement->name == "hksection")
			{
				const char* sectName = startElement->getAttribute("name", "__data__");
				if( m_sectionTagToIndex.get(sectName, &currentSectionIndex) != HK_SUCCESS )
				{
					currentSectionIndex = m_knownSections.getSize();
					char* s = hkString::strDup(sectName);
					m_data->addAllocation( s );
					m_knownSections.pushBack( s );
					m_sectionTagToIndex.insert( m_knownSections.back(), currentSectionIndex );
				}
			}
			else if( startElement->name == "hkpackfile")
			{
				readClassVersionAttributes(startElement, classVersion, &m_version);
				if( classVersion >= 2 )
				{
					classRegistry.registerClass( &hkClassClass );
				}
				if( originalReg == HK_NULL )
				{
					// no class registry supplied, try to use a compiled in version.
					const hkClassNameRegistry* registry = hkVersionRegistry::getInstance().getClassNameRegistry(m_version);
					HK_ASSERT2( 0x74967ea2, registry != HK_NULL, "Class version " << m_version << " was not found in the hkVersionRegistry");
					HK_ASSERT2( 0x470b6e07, classRegistryAutoDetected == false, "Duplicate <packfile> element found");
					classRegistry.setNextRegistry(registry);
					classRegistryAutoDetected = true;
				}
			}
			else
			{
				HK_WARN( 0x5c4378f9, "Unhandled xml tag " << startElement->name);
				return HK_FAILURE;
			}
		}
		else if( hkXmlParser::EndElement* end = node->asEnd() )
		{
			HK_ASSERT2(0x68293525, end->name == "hksection" || end->name == "hkpackfile",
				"Mismatched xml end tag " << end->name);
		}
		else if (hkXmlParser::Characters* chars = node->asCharacters() )
		{
			chars->canonicalize();
			if (chars->text.getLength() > 0)
			{
				// Unidentified text outside of tags
				return HK_FAILURE;
			}
		}
		else
		{
			HK_ERROR(0x5ef4e5a3, "Unhandled tag in XML");
		}
		delete node;
	}

	hkBool unresolvedLocals = false;
	for( hkStringMap<int>::Iterator it = unresolvedReferences.getIterator(); unresolvedReferences.isValid(it); it = unresolvedReferences.getNext(it) )
	{
		const char* name = unresolvedReferences.getKey(it);
		if( name[0] == '#' )
		{
			// #foo identifiers are local to file
			HK_WARN(0x52a902e9, "undefined reference to '" << name << "'");
			unresolvedLocals = true;
		}
		else if( name[0] == '@' )
		{
			char* importName = hkString::strDup(name+1);
			name = importName;
			m_data->addAllocation(importName);

			int refIndex = unresolvedReferences.getValue(it);
			while( refIndex != -1 )
			{
				void* loc = m_tracker->m_pointers.getValue(refIndex);
				m_data->addImport( name, reinterpret_cast<void**>(loc) );
				refIndex = m_tracker->m_pointers.getNextIndex(refIndex);
			}
		}
		else
		{
			HK_ASSERT2( 0xafdee3b9, 0, "Unrecognised symbol name '" << name << "'" );
		}
	}
	HK_ASSERT2(0x371105b7, unresolvedLocals == false, "File has unresolved references");
	return HK_SUCCESS;
}

void* hkXmlPackfileReader::getContentsWithRegistry(
		const char* expectedClassName,
		const hkFinishLoadedObjectRegistry* finishRegistry )
{
	if( finishRegistry != HK_NULL )
	{
		for( int loadedIndex = 0; loadedIndex < m_loadedObjects.getSize(); ++loadedIndex )
		{
			hkVariant& loadedObj = m_loadedObjects[loadedIndex];
			if( const char* finishName = m_tracker->m_finish.getWithDefault(loadedObj.m_object, HK_NULL) )
			{
				HK_ASSERT(0x4c45f944, hkString::strCmp(finishName, loadedObj.m_class->getName()) == 0 );
				const hkTypeInfo* registeredType = finishRegistry->finishLoadedObject( loadedObj.m_object, finishName );
				// save info to cleanup the object later on
				if (registeredType)
				{
					// HK_ON_DEBUG(printf("+ctor\t%s at %p.\n", registeredType->getTypeName(), loadedObj.m_object));
					m_data->trackObject(loadedObj.m_object, registeredType);
				}
			}
		}
	}

	const char* topLevelClassName = m_tracker->m_topLevelClassName;
	if( expectedClassName != HK_NULL
		&& hkString::strCmp(expectedClassName, topLevelClassName) != 0 )
	{
		HK_WARN(0x599a0b0c, "Requested " << expectedClassName << " but file contains " << topLevelClassName );
		return HK_NULL;
	}

	return m_tracker->m_topLevelObject;
}

const char* hkXmlPackfileReader::getContentsClassName()
{
	return m_tracker->m_topLevelClassName;
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
