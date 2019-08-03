/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkserialize/hkSerialize.h>

#include <hkbase/stream/hkStreamReader.h>
#include <hkbase/htl/hkPointerMap.h>
#include <hkserialize/packfile/binary/hkBinaryPackfileReader.h>
#include <hkserialize/packfile/binary/hkPackfileHeader.h>
#include <hkserialize/packfile/binary/hkPackfileSectionHeader.h>
#include <hkserialize/util/hkFinishLoadedObjectRegistry.h>
#include <hkserialize/util/hkStructureLayout.h>
#include <hkserialize/copier/hkObjectCopier.h>
#include <hkserialize/serialize/hkRelocationInfo.h>
#include <hkserialize/version/hkVersionRegistry.h>
#include <hkserialize/version/hkPackfileObjectUpdateTracker.h>
#include <hkserialize/util/hkObjectInspector.h>
#include <hkserialize/util/hkVtableClassRegistry.h>

#if 0 && defined(HK_DEBUG)
#	include <hkbase/fwd/hkcstdio.h>
using namespace std;
#	define PRINT(A) printf A
#else
#	define PRINT(A) // nothing
#endif

namespace
{
	typedef hkPointerMap<const void*, const hkClass*> ClassFromAddressMap;
	struct Location
	{
		int m_sectionIndex;
		int m_offset;
	};

	struct DataEntry
	{
		int m_sectionIndex;
		int m_offset;
		int m_classIndex;
	};
	
	void updateSectionHeaders( hkPackfileSectionHeader* sections, int num )
	{
		for( int i = 0; i < num; ++i )
		{
			hkPackfileSectionHeader& s = sections[i];
			// endOffset moves down a slot in v3 -> v4
			s.m_endOffset = s.m_importsOffset;
			// no imports/exports in v3
			s.m_importsOffset = s.m_endOffset;
			s.m_exportsOffset = s.m_endOffset;
		}
	}

	// Collects information about objects into hkVariant array.
	class PackfileObjectsListener: public hkObjectInspector::ObjectListener
	{
		public:
			PackfileObjectsListener( hkArray<hkVariant>& collectedObjects, const ClassFromAddressMap& classesForVObjects,
				int fileVersion, hkPackfileReader::UpdateFlagFromClassMap& updateFlagFromClass )
				: m_collectedObjects(collectedObjects), m_classesForVObjects(classesForVObjects),
				 m_fileVersion(fileVersion), m_updateFlagFromClass(updateFlagFromClass) { }
			virtual hkResult objectCallback( const void* objP, const hkClass& klass, hkArray<hkObjectInspector::Pointer>& containedPointers );

		private:
			hkArray<hkVariant>& m_collectedObjects; // we fill in this array
			const ClassFromAddressMap& m_classesForVObjects; // exact type of virtual objects
			enum Flags { PENDING=1, DONE=2 };
			hkPointerMap<const void*, Flags> m_seenObjects;
			int m_fileVersion;
			hkPackfileReader::UpdateFlagFromClassMap& m_updateFlagFromClass;
	};

	hkResult PackfileObjectsListener::objectCallback( const void* objP, const hkClass& klass, hkArray<hkObjectInspector::Pointer>& containedPointers )
	{
		HK_ASSERT(0x6b56a08d, m_seenObjects.getWithDefault(objP,PackfileObjectsListener::Flags(0)) < PackfileObjectsListener::DONE );
		{
			HK_ON_DEBUG(if (containedPointers.getSize() > 0))
			{
				PRINT(("WALK\t0x%p\t# %s(0x%x) at 0x%p\n", objP, klass.getName(), klass.getSignature(), &klass));
			}
			hkVariant& v = m_collectedObjects.expandOne();
			v.m_class = &klass;
			v.m_object = const_cast<void*>(objP);
			m_seenObjects.insert(objP, PackfileObjectsListener::DONE);
			int ptrIdx = 0;
			while (ptrIdx < containedPointers.getSize())
			{
				hkObjectInspector::Pointer& p = containedPointers[ptrIdx];
				void* objPtr = *(p.location);
				// update the klass for virtual objects
				// the klass must be correct for farther processing
				if( objPtr != HK_NULL && m_seenObjects.hasKey(objPtr) == false )
				{
					p.klass = m_classesForVObjects.getWithDefault(objPtr, p.klass);
					m_seenObjects.insert(objPtr, PackfileObjectsListener::PENDING );
					HK_ASSERT2(0x15a09058, p.klass != HK_NULL, "Cannot walk through an object of unknown type. Are you trying to version old packfile with no meta-data?");
					hkPackfileReader::updateMetaDataInplace( const_cast<hkClass*>(p.klass), m_fileVersion, m_updateFlagFromClass );
					PRINT(("PTR\t0x%p\t# %s(0x%x) at 0x%p\n", objPtr, p.klass->getName(), p.klass->getSignature(), p.klass));
					++ptrIdx;
				}
				else
				{
					containedPointers.removeAtAndCopy(ptrIdx);
				}
			}
		}
		return HK_SUCCESS;
	}

	// Build up a map of global pointers
	class PackfilePointersMapListener: public hkObjectInspector::ObjectListener
	{
		public:
			PackfilePointersMapListener(hkBinaryPackfileUpdateTracker& tracker, const hkClassNameRegistry& classReg,
				int fileVersion, hkPackfileReader::UpdateFlagFromClassMap& updateFlagFromClass)
				: m_tracker(tracker), m_classReg(classReg), m_fileVersion(fileVersion), m_updateFlagFromClass(updateFlagFromClass) { }
				virtual hkResult objectCallback( const void* objP, const hkClass& klass, hkArray<hkObjectInspector::Pointer>& containedPointers );

		private:
			hkBinaryPackfileUpdateTracker& m_tracker; // we fill in this array
			const hkClassNameRegistry& m_classReg; // class registry to find exact type of virtual objects
			int m_fileVersion;
			hkPackfileReader::UpdateFlagFromClassMap& m_updateFlagFromClass;
	};

	hkResult PackfilePointersMapListener::objectCallback( const void* objP, const hkClass& klass, hkArray<hkObjectInspector::Pointer>& containedPointers )
	{
		int ptrIdx = 0;
		PRINT(("WALK\t0x%p\t# %s(0x%x) at 0x%p\n", objP, klass.getName(), klass.getSignature(), &klass));
		while( ptrIdx < containedPointers.getSize() )
		{
			hkObjectInspector::Pointer& p = containedPointers[ptrIdx];
			void* objPtr = *(p.location);
			m_tracker.objectPointedBy(objPtr, p.location);
			PRINT(("GPTR\t0x%p\tat\t0x%p\t# %s(0x%x) at 0x%p\n", objPtr, p.location, p.klass->getName(), p.klass->getSignature(), p.klass));
			if( objPtr != HK_NULL )
			{
				HK_ASSERT2(0x15a09058, p.klass != HK_NULL, "Cannot walk through an object of unknown type. Are you trying to version old packfile with no meta-data?");
				hkPackfileReader::updateMetaDataInplace( const_cast<hkClass*>(p.klass), m_fileVersion, m_updateFlagFromClass );
				if (p.klass->hasVtable())
				{
					// find exact class for the object
					// NOTE: The tracker.m_finish must have all the virtual object infos.
					const char* className = m_tracker.m_finish.getWithDefault(objPtr, 0);
					HK_ASSERT2(0x15a0905c, className != HK_NULL, "The virtual object at 0x" << objPtr << " is not found in the finish table, base class is " << p.klass->getName());
					p.klass = m_classReg.getClassByName(className);
					HK_ASSERT2(0x15a0905d, p.klass != HK_NULL, "The class " << className << " is not registered?");
					hkPackfileReader::updateMetaDataInplace( const_cast<hkClass*>(p.klass), m_fileVersion, m_updateFlagFromClass );
				}
				++ptrIdx;
			}
			else
			{
				containedPointers.removeAtAndCopy(ptrIdx);
			}
		}
		return HK_SUCCESS;
	}
}

hkBinaryPackfileReader::hkBinaryPackfileReader()
:	m_header(HK_NULL),
m_sections(HK_NULL),
m_streamOffset(0),
m_loadedObjects(HK_NULL),
m_tracker(HK_NULL)
{
	m_packfileData = new BinaryPackfileData();
}

hkBinaryPackfileReader::~hkBinaryPackfileReader()
{
	delete m_tracker;
	delete m_loadedObjects;
	m_packfileData->removeReference();
}

hkPackfileData* hkBinaryPackfileReader::getPackfileData()
{
	return m_packfileData;
}

hkResult hkBinaryPackfileReader::loadEntireFile(hkStreamReader* reader)
{
	if( loadFileHeader(reader) == HK_SUCCESS )
	{
		PRINT(("\n### FILE VERSION: %d, '%s'.\n", m_header->m_fileVersion, m_header->m_contentsVersion));
		if( loadSectionHeadersNoSeek(reader) == HK_SUCCESS )
		{
			for( int i = 0; i < m_header->m_numSections; ++i )
			{
				if( loadSectionNoSeek(reader, i) == HK_FAILURE )
				{
					return HK_FAILURE;
				}
			}
			if( fixupGlobalReferences() == HK_FAILURE )
			{
				return HK_FAILURE;
			}			
			return HK_SUCCESS;
		}
	}
	return HK_FAILURE;
}

hkResult hkBinaryPackfileReader::loadEntireFileInplace( void* data, int dataSize )
{
	HK_ASSERT2( 0x78b7ad37, dataSize > hkSizeOf(hkPackfileHeader), "Packfile is too small" );
	HK_ASSERT2( 0x7d8aae85, (hkUlong(data) & 0xf) == 0, "Data needs to be 16 byte aligned");

	// Header
	hkPackfileHeader magic;
	hkPackfileHeader* header = (hkPackfileHeader*)data;
	if( (header->m_magic[0] == magic.m_magic[0])
		&& (header->m_magic[1] == magic.m_magic[1]) )
	{
		m_header = header;
	}
	else 
	{
		HK_ASSERT2(0xea701934, 0, "Unable to identify binary inplace data. Is this from a binary file? " );
		return HK_FAILURE;
	}
	HK_ASSERT2(0xfe567fe4, m_header->m_layoutRules[0] == hkStructureLayout::HostLayoutRules.m_bytesInPointer, "Trying to load a binary file with a different pointer size than this platform." );
	HK_ASSERT2(0xfe567fe5, m_header->m_layoutRules[1] == hkStructureLayout::HostLayoutRules.m_littleEndian, "Trying to load a binary file with a different endian than this platform." );
	HK_ASSERT2(0xfe567fe6, m_header->m_layoutRules[2] == hkStructureLayout::HostLayoutRules.m_reusePaddingOptimization, "Trying to load a binary file with a different padding optimization than this platform." );
	HK_ASSERT2(0xfe567fe7, m_header->m_layoutRules[3] == hkStructureLayout::HostLayoutRules.m_emptyBaseClassOptimization, "Trying to load a binary file with a different empty base class optimization than this platform." );

	// section headers
	m_sections = m_header->m_numSections > 0 ? (hkPackfileSectionHeader*)(header + 1) : HK_NULL;
	if( m_header->m_fileVersion < 4 )
	{
		updateSectionHeaders( m_sections, m_header->m_numSections );
	}

	// section data
	char* dataPtr = reinterpret_cast<char*>( data );
	m_sectionData.setSize(m_header->m_numSections);
	for( int sectionIndex = 0; sectionIndex < m_header->m_numSections; ++sectionIndex )
	{
		hkPackfileSectionHeader& section = m_sections[sectionIndex];

		int sectStart = section.m_absoluteDataStart;
		HK_ON_DEBUG( int sectSize = section.m_endOffset );
		HK_ASSERT2(0xff668fe7, ( (sectStart <= dataSize) && ((sectStart + sectSize) <= dataSize) ), "Inplace packfile data is too small. Is it corrupt?");

		// apply local fixups now
		char* dataBegin = dataPtr + sectStart;
		int* localFixups = reinterpret_cast<int*>(dataBegin + section.m_localFixupsOffset);
		for( int i = 0; i < section.getLocalSize() / hkSizeOf(hkInt32); i+=2 )
		{
			int srcOff = localFixups[i  ];
			if( srcOff == -1 ) continue;
			HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
			int dstOff = localFixups[i+1];
			void** addrSrc = reinterpret_cast<void**>(dataBegin+srcOff);
			void* dst = reinterpret_cast<void*>(dataBegin+dstOff);
			HK_ASSERT2( 0x75936f92, *addrSrc == HK_NULL,
				"Pointer has already been patched. Corrupt file or loadEntireFileInplace called multiple times?");
			*addrSrc = dst;
		}

		m_sectionData[sectionIndex] = dataBegin;
	}

	// todo: merge this with other block in loadEntireFile
	{
		int nameIndex = m_header->m_contentsClassNameSectionIndex;
		int nameOffset = m_header->m_contentsClassNameSectionOffset;

		if( nameIndex >= 0
			&& nameOffset >= 0
			&& m_header->m_fileVersion < 3)
		{
			// contentsClass used to point to a class instance
			// adjust it to point to the class name instead.
			const hkClass* klass = (const hkClass*)getSectionDataByIndex(nameIndex,nameOffset);
			const char* name = klass->getName();
			hkUlong off = hkUlong(name) - hkUlong( m_sectionData[nameIndex] );
			m_header->m_contentsClassNameSectionOffset = int(off);
		}
	}

	return fixupGlobalReferences();
}

hkObjectUpdateTracker& hkBinaryPackfileReader::getUpdateTracker()
{
	if( m_tracker == HK_NULL )
	{
		// lazily create tracker if needed
		m_tracker = new hkBinaryPackfileUpdateTracker(m_packfileData);

		hkBool shouldWalkPointers = m_header->m_fileVersion == 1;
		
		PRINT(("\n### BUILD TRACKER\n"));

		for( int sectionIndex = 0; sectionIndex < m_header->m_numSections; ++sectionIndex )
		{
			if( m_sectionData[sectionIndex] ) // is section loaded?
			{
				hkPackfileSectionHeader& sect = m_sections[sectionIndex];
				char* dataBegin = static_cast<char*>(m_sectionData[sectionIndex]);
				if( shouldWalkPointers == false )
				{
					int* globalFixups = reinterpret_cast<int*>(dataBegin + sect.m_globalFixupsOffset );
					for( int i = 0; i < sect.getGlobalSize() / hkSizeOf(hkInt32); i += 3 )
					{
						int srcOff = globalFixups[i  ];
						if( srcOff == -1 ) continue;
						HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
						int dstSec = globalFixups[i+1];
						int dstOff = globalFixups[i+2];

						// automatically checks for dest section loaded
						void* dstPtr = getSectionDataByIndex(dstSec, dstOff);
						m_tracker->objectPointedBy( dstPtr, dataBegin+srcOff );
						PRINT(("GFIX\t0x%p\tat\t0x%p\n", dstPtr, (void*)(dataBegin+srcOff)));
					}
				}
				{
					int* finishFixups = reinterpret_cast<int*>(dataBegin + sect.m_virtualFixupsOffset);
					for( int i = 0; i < sect.getFinishSize() / hkSizeOf(hkInt32); i += 3 )
					{
						int srcOff = finishFixups[i  ];
						if( srcOff == -1 ) continue;
						HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
						int dstSec = finishFixups[i+1];
						int dstOff = finishFixups[i+2];

						void* srcPtr = getSectionDataByIndex(sectionIndex, srcOff);
						void* dstPtr = getSectionDataByIndex(dstSec, dstOff);
						m_tracker->addFinish( srcPtr, static_cast<char*>(dstPtr) );
					}
				}
			}
		}

		m_tracker->m_topLevelObject = getOriginalContents();
		m_tracker->m_topLevelClassName  = getOriginalContentsClassName();

		if( shouldWalkPointers )
		{
			const hkClassNameRegistry* classReg = getClassNameRegistry();
			PackfilePointersMapListener listener( *m_tracker, *classReg, m_header->m_fileVersion, m_updateFlagFromClass );
			const hkClass& originalContentsClass = *classReg->getClassByName(m_tracker->m_topLevelClassName);
			PRINT(("\nTRACK\t0x%p\t# %s(0x%x) at 0x%p\n\n", m_tracker->m_topLevelObject, originalContentsClass.getName(), originalContentsClass.getSignature(), &originalContentsClass));
			if (hkObjectInspector::walkPointers(m_tracker->m_topLevelObject, originalContentsClass, &listener) == HK_FAILURE)
			{
				HK_WARN(0x5a65ee8e, "Error occured while getting the loaded object list from packfile.");
			}
			classReg->removeReference();
		}
	}
	return *m_tracker;
}

const hkClassNameRegistry* hkBinaryPackfileReader::getClassNameRegistry()
{
	hkClassNameRegistry* loadedRegistry = HK_NULL;
	int numClassesRegistered = 0;
	if( (m_header->m_fileVersion >= 4) || (hkString::strCmp(m_header->m_contentsVersion,"Havok-4.0.0-b1")==0) )
	{
		// 400b1 and above have finish for all objects - we can reconstruct from this
		loadedRegistry = new hkClassNameRegistry();
		for( int sectionIndex = 0; sectionIndex < m_header->m_numSections; ++sectionIndex )
		{
			if( m_sectionData[sectionIndex] ) // is section loaded?
			{
				hkPackfileSectionHeader& sect = m_sections[sectionIndex];
				char* dataBegin = static_cast<char*>(m_sectionData[sectionIndex]);
				int* finishFixups = reinterpret_cast<int*>(dataBegin + sect.m_virtualFixupsOffset );
				for( int i = 0; i < sect.getFinishSize() / hkSizeOf(hkInt32); i += 3 )
				{
					int srcOff = finishFixups[i  ];
					if( srcOff == -1 ) continue;
					HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
					int dstSec = finishFixups[i+1];
					int dstOff = finishFixups[i+2];

					char* className = static_cast<char*>( getSectionDataByIndex(dstSec, dstOff) );
					if( hkString::strCmp(className, "hkClass") == 0 )
					{
						hkClass* klass = reinterpret_cast<hkClass*>(dataBegin+srcOff);
						HK_ASSERT( 0x15a0905a, klass != HK_NULL );
						const char* klassName = klass->getName();
						if( hkString::strCmp(klassName, "hkClass") != 0
							&& hkString::strCmp(klassName, "hkClassMember") != 0
							&& hkString::strCmp(klassName, "hkClassEnum") != 0
							&& hkString::strCmp(klassName, "hkClassEnumItem") != 0 )
						{
							numClassesRegistered += 1;
							hkPackfileReader::updateMetaDataInplace( klass, m_header->m_fileVersion, m_updateFlagFromClass );
							loadedRegistry->registerClass( klass, klass->getName() );
						}
					}
				}
			}
		}
	}
	else
	{
		// pre 400 had a special class index section because not all objects had finishing
		int classIndex = getSectionIndex("__classindex__");
		// section exists && non empty && loaded
		if( classIndex >= 0 && m_sections[classIndex].getDataSize() != 0 && m_sectionData[classIndex] != 0)
		{
			loadedRegistry = new hkClassNameRegistry();
			hkInt32* data = static_cast<hkInt32*>( m_sectionData[classIndex] );
			int maxEntries = m_sections[classIndex].getDataSize() / hkSizeOf(hkInt32);
			for( int i = 0; i < maxEntries; i += 2 )
			{
				int sec = data[i+0];
				if( sec == -1 ) break;
				int off = data[i+1];

				if( void* rawClass = getSectionDataByIndex(sec, off) )
				{
					numClassesRegistered += 1;
					hkClass* klass = static_cast<hkClass*>(rawClass);
					hkPackfileReader::updateMetaDataInplace( klass, m_header->m_fileVersion, m_updateFlagFromClass );
					loadedRegistry->registerClass( klass, klass->getName() );
				}
			}
		}
	}

	if( numClassesRegistered == 0 ) // maybe stripped metadata, but not index?
	{
		hkVersionRegistry& versionReg = hkVersionRegistry::getInstance();
		const hkClassNameRegistry* compiledRegistry = versionReg.getClassNameRegistry(getOriginalContentsVersion());
		if( loadedRegistry )
		{
			loadedRegistry->removeReference();
			loadedRegistry = HK_NULL;
		}
		HK_ASSERT2( 0x15a0905b, compiledRegistry != HK_NULL,
					"Can not get class registry for packfile with no meta-data and no compatiblity libraries." );
		compiledRegistry->addReference();
		return compiledRegistry;
	}
	else // add non serialized classes
	{
		const hkClass* specials[] = {&hkClassClass, &hkClassMemberClass, &hkClassEnumClass, &hkClassEnumItemClass, HK_NULL };
		if( m_header->m_fileVersion == 1 )
		{
			extern const hkClass hkClassVersion1Class;
			specials[0] = &hkClassVersion1Class;
			extern const hkClass hkClassMemberVersion1Class;
			specials[1] = &hkClassMemberVersion1Class;
		}
		for( int i = 0; specials[i] != HK_NULL; ++i )
		{
			loadedRegistry->registerClass(specials[i], specials[i]->getName() );
		}
		return loadedRegistry;
	}
}

hkArray<hkVariant>& hkBinaryPackfileReader::getLoadedObjects()
{
	if( m_loadedObjects == HK_NULL )
	{
		const hkClassNameRegistry* classReg = getClassNameRegistry();
		HK_COMPILE_TIME_ASSERT( sizeof(void*) <= 8 );

		PRINT(("\n### LOAD OBJs\n"));
		ClassFromAddressMap classFromObjectMap;
		for( int sectionIndex = 0; sectionIndex < m_header->m_numSections; ++sectionIndex )
		{
			if( m_sectionData[sectionIndex] ) // is section loaded?
			{
				hkPackfileSectionHeader& sect = m_sections[sectionIndex];
				char* dataBegin = static_cast<char*>(m_sectionData[sectionIndex]);
				int* finishFixups = reinterpret_cast<int*>(dataBegin + sect.m_virtualFixupsOffset );
				for( int i = 0; i < sect.getFinishSize() / hkSizeOf(hkInt32); i += 3 )
				{
					int srcOff = finishFixups[i  ];
					if( srcOff == -1 ) continue;
					HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
					int dstSec = finishFixups[i+1];
					int dstOff = finishFixups[i+2];

					// If __classnames__ has signatures, check them against the used registry.
					char* className = static_cast<char*>( getSectionDataByIndex(dstSec, dstOff) );
					const hkClass* klass = classReg->getClassByName(className);
					HK_ASSERT2( 0x15a09058, klass != HK_NULL, "Found an unregistered class " << className );
					// check if it is not the runtime class
					if( hkString::strCmp("hkClass", className) != 0
						&& hkString::strCmp("hkClassMember", className) != 0
						&& hkString::strCmp("hkClassEnum", className) != 0
						&& hkString::strCmp("hkClassEnumItem", className) != 0 )
					{
						if( dstOff != 0 && className[-1] == '\t' ) // signature exists
						{
							union { char c[4]; hkUint32 sig; } u;
							u.c[0] = className[-5];
							u.c[1] = className[-4];
							u.c[2] = className[-3];
							u.c[3] = className[-2];
							HK_ON_DEBUG(hkUlong sig = klass->getSignature());
							HK_ASSERT2(0x15a09059, hkUlong(sig) == u.sig, "Signature mismatch " << className << " (0x" << klass << ") - 0x" << (void*)sig << ", 0x" << (void*)((hkUlong)u.sig));
						}

						classFromObjectMap.insert(static_cast<void*>(dataBegin+srcOff), const_cast<hkClass*>(klass));
						PRINT(("LOAD\t0x%p\t# %s(0x%x) at 0x%p (%s)\n", static_cast<void*>(dataBegin+srcOff), klass->getName(), klass->getSignature(), klass, klass->hasVtable() ? "vTable" : "No vTable"));
					}
				}
			}
		}
		// lazily create loaded objects
		m_loadedObjects = new hkArray<hkVariant>();
		PackfileObjectsListener listener( *m_loadedObjects, classFromObjectMap, m_header->m_fileVersion, m_updateFlagFromClass );
		// reserve memory for all known objects
		m_loadedObjects->reserve(classFromObjectMap.getSize());
		const hkClass& originalContentsClass = *classReg->getClassByName(getOriginalContentsClassName());
		void* topObject = getOriginalContents();
		PRINT(("\n### WALK OBJs, start at 0x%p, %s(0x%x) at 0x%p\n", topObject, originalContentsClass.getName(), originalContentsClass.getSignature(), &originalContentsClass));
		if (hkObjectInspector::walkPointers(topObject, originalContentsClass, &listener) == HK_FAILURE)
		{
			HK_WARN(0x5a65ee8e, "Error occured while getting the loaded object list from packfile.");
		}
		classReg->removeReference();
	}
	return *m_loadedObjects;
}

void* hkBinaryPackfileReader::getContentsWithRegistry(
	const char* expectedClassName,
	const hkFinishLoadedObjectRegistry* finishRegistry)
{
	// Fixup vtables, etc.
	if( finishRegistry != HK_NULL )
	{
		finishLoadedObjects(*finishRegistry);
	}

	// Get toplevel
	void* topLevelObject = HK_NULL;
	const char* topLevelClassName = HK_NULL;
	if( m_tracker != HK_NULL ) // toplevel may have been versioned
	{
		topLevelObject = m_tracker->m_topLevelObject;
		topLevelClassName = m_tracker->m_topLevelClassName;
	}
	else
	{
		topLevelObject = getOriginalContents();
		topLevelClassName = getOriginalContentsClassName();
	}

	// Check contents are what we expect
	if ( expectedClassName != HK_NULL && topLevelClassName != HK_NULL )
	{
		if( hkString::strCmp( expectedClassName, topLevelClassName ) != 0 )
		{
			HK_WARN(0x599a0b0c, "Requested " << expectedClassName << " but file contains " << topLevelClassName );
			return HK_NULL;
		}
	}

	return topLevelObject;
}

const char* hkBinaryPackfileReader::getOriginalContentsVersion()
{
	if( m_header->m_contentsVersion[0] != char(-1) )
	{
		return m_header->m_contentsVersion;
	}
	else if( m_header->m_fileVersion == 1 )
	{
		return "Havok-3.0.0";
	}
	else if( m_header->m_fileVersion == 2 )
	{
		return "Havok-3.1.0";
	}
	else
	{
		HK_ASSERT(0x4d335da9, 0);
		return HK_NULL;
	}
}

hkResult hkBinaryPackfileReader::loadFileHeader(hkStreamReader* reader, hkPackfileHeader* dst )
{
	HK_ASSERT(0x78b8ad37, reader);

	// Store starting point for Havok data... will use this for all reads
	m_streamOffset = reader->seekTellSupported() ? reader->tell() : 0;

	if (!dst)
	{
		dst = hkAllocate<hkPackfileHeader>( 1, HK_MEMORY_CLASS_EXPORT  );
		m_packfileData->addAllocation(dst);
	}

	if( reader->read( dst, sizeof(hkPackfileHeader)) == sizeof(hkPackfileHeader) )
	{
		hkPackfileHeader* header = (hkPackfileHeader*)dst;
		hkPackfileHeader magic;
		if( (header->m_magic[0] == magic.m_magic[0])
			&& (header->m_magic[1] == magic.m_magic[1]) )
		{
			m_header = dst;
			return HK_SUCCESS;
		}
	}

	m_header = HK_NULL;
	HK_ASSERT2(0xea701934, 0, "Unable to identify binary file header. Is this a binary file? " );
	return HK_FAILURE;
}

const hkPackfileHeader& hkBinaryPackfileReader::getFileHeader() const
{
	HK_ASSERT2(0xea701935, m_header, "You must load the file header or an entire file first on a proper stream.");
	return *m_header;
}

int hkBinaryPackfileReader::getNumSections() const
{
	return m_header->m_numSections;
}

hkResult hkBinaryPackfileReader::loadSectionHeadersNoSeek( hkStreamReader* reader, hkPackfileSectionHeader* dst )
{
	HK_ASSERT2(0xfe567fe4, m_header->m_layoutRules[0] == hkStructureLayout::HostLayoutRules.m_bytesInPointer, "Trying to load a binary file with a different pointer size than this platform." );
	HK_ASSERT2(0xfe567fe5, m_header->m_layoutRules[1] == hkStructureLayout::HostLayoutRules.m_littleEndian, "Trying to load a binary file with a different endian than this platform." );
	HK_ASSERT2(0xfe567fe6, m_header->m_layoutRules[2] == hkStructureLayout::HostLayoutRules.m_reusePaddingOptimization, "Trying to load a binary file with a different padding optimization than this platform." );
	HK_ASSERT2(0xfe567fe7, m_header->m_layoutRules[3] == hkStructureLayout::HostLayoutRules.m_emptyBaseClassOptimization, "Trying to load a binary file with a different empty base class optimization than this platform." );

	if( dst == HK_NULL )
	{
		dst = hkAllocate<hkPackfileSectionHeader>( m_header->m_numSections, HK_MEMORY_CLASS_EXPORT );
		m_packfileData->addAllocation(dst);
	}
	int size = sizeof(hkPackfileSectionHeader) * m_header->m_numSections;
	HK_ASSERT(0x728eeb3a, reader);
	if( reader->read( dst, size ) == size )
	{
		m_sections = dst;
		m_sectionData.setSize( m_header->m_numSections, 0 );
		if( m_header->m_fileVersion < 4 )
		{
			updateSectionHeaders( m_sections, m_header->m_numSections );
		}
		return HK_SUCCESS;
	}
	return HK_FAILURE;
}

hkPackfileSectionHeader& hkBinaryPackfileReader::getSectionHeader(int idx)
{
	HK_ASSERT(0x3c0dde36, m_sections);
	return m_sections[idx];
}

hkResult hkBinaryPackfileReader::loadSectionNoSeek(hkStreamReader* reader, int sectionIndex, void* buf)
{
	HK_ASSERT2(0x1c8e05da, m_tracker == HK_NULL, "Can't do versioning and load/unload sections");
	hkPackfileSectionHeader& section = m_sections[sectionIndex];

	int sectsize = section.m_endOffset;
	if( buf == HK_NULL )
	{
		buf = hkAllocate<char>( sectsize, HK_MEMORY_CLASS_EXPORT );
		m_packfileData->addAllocation(buf);
	}

	if( reader->read(buf, sectsize ) == sectsize )
	{
		// apply local fixups now
		char* dataBegin = static_cast<char*>(buf);
		int* localFixups = reinterpret_cast<int*>(dataBegin + section.m_localFixupsOffset);
		for( int i = 0; i < section.getLocalSize() / hkSizeOf(hkInt32); i+=2 )
		{
			int srcOff = localFixups[i  ];
			if( srcOff == -1 ) continue;
			HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
			int dstOff = localFixups[i+1];

			*(hkUlong*)(dataBegin+srcOff) = hkUlong(dataBegin+dstOff);
		}

		// exports
		{
			hkArray<hkResource::Export> exports;
			section.getExports(buf, exports);
			for( int i = 0; i < exports.getSize(); ++i )
			{
				m_packfileData->addExport( exports[i].name, exports[i].data );
			}
		}
		// imports
		{
			hkArray<hkResource::Import> imports;
			section.getImports(buf, imports);
			for( int i = 0; i < imports.getSize(); ++i )
			{
				m_packfileData->addImport( imports[i].name, imports[i].location );
			}
		}

		m_sectionData[sectionIndex] = buf;

		int nameIndex = m_header->m_contentsClassNameSectionIndex;
		int nameOffset = m_header->m_contentsClassNameSectionOffset;

		if( sectionIndex == nameIndex
			&& nameOffset >= 0
			&& m_header->m_fileVersion < 3)
		{
			// contentsClass used to point to a class instance
			// adjust it to point to the class name instead.
			const hkClass* klass = (const hkClass*)getSectionDataByIndex(nameIndex,nameOffset);
			const char* name = klass->getName();
			hkUlong off = hkUlong(name) - hkUlong(buf);
			m_header->m_contentsClassNameSectionOffset = int(off);
		}
		return HK_SUCCESS;
	}
	return HK_FAILURE;
}

hkResult hkBinaryPackfileReader::loadSection(hkStreamReader* reader, int sectionIndex, void* buf)
{
	HK_ASSERT2(0x5b2c4574, reader != HK_NULL, "Read from a closed file." );
	HK_ASSERT2(0x09768fa4, reader->seekTellSupported(), "Need seek/tell for individual section load." );

	if( reader->seek( m_sections[sectionIndex].m_absoluteDataStart + m_streamOffset, hkStreamReader::STREAM_SET ) == HK_SUCCESS )
	{
		return loadSectionNoSeek( reader, sectionIndex, buf );
	}
	return HK_FAILURE;
}

void hkBinaryPackfileReader::BinaryPackfileData::freeSection(void* data)
{
	for( int i = 0; i < m_memory.getSize(); ++i )
	{
		// possibly need to deallocate memory
		if( m_memory[i] == data )
		{
			m_memory.removeAt(i);
			hkDeallocate<char>( static_cast<char*>(data));
			break;
		}
	}
}

hkResult hkBinaryPackfileReader::unloadSection(int unloadIndex)
{
	HK_ASSERT2(0x1c8e05da, m_tracker == HK_NULL, "Can't do versioning and load/unload sections");
	m_packfileData->freeSection(m_sectionData[unloadIndex]);
	m_sectionData[unloadIndex] = HK_NULL;

	return HK_SUCCESS;
}

hkResult hkBinaryPackfileReader::fixupGlobalReferences()
{
	for( int sectionIndex = 0; sectionIndex < m_header->m_numSections; ++sectionIndex )
	{
		if( m_sectionData[sectionIndex] ) // is section loaded?
		{
			hkPackfileSectionHeader& sect = m_sections[sectionIndex];
			char* dataBegin = static_cast<char*>(m_sectionData[sectionIndex]);
			int* globalFixups = reinterpret_cast<int*>(dataBegin + sect.m_globalFixupsOffset );
			for( int i = 0; i < sect.getGlobalSize() / hkSizeOf(hkInt32); i += 3 )
			{
				int srcOff = globalFixups[i  ];
				if( srcOff == -1 ) continue;
				HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
				int dstSec = globalFixups[i+1];
				int dstOff = globalFixups[i+2];

				// automatically checks for dest section loaded
				void* dstPtr = getSectionDataByIndex(dstSec, dstOff);
				*(hkUlong*)(dataBegin+srcOff) = hkUlong(dstPtr);
			}
		}
	}
	return HK_SUCCESS;
}

hkResult hkBinaryPackfileReader::finishLoadedObjects(const hkFinishLoadedObjectRegistry& finishObjects )
{
	PRINT(("\n### FINISH OBJs\n"));
	if( m_tracker == HK_NULL )
	{
		// The usual case: no versioning has been done.
		// Thus the fixups in the file are all valid
		for( int sectionIndex = 0; sectionIndex < m_header->m_numSections; ++sectionIndex )
		{
			if( m_sectionData[sectionIndex] ) // is section loaded?
			{
				hkPackfileSectionHeader& sect = m_sections[sectionIndex];
				char* dataBegin = static_cast<char*>(m_sectionData[sectionIndex]);
				int* finishFixups = reinterpret_cast<int*>(dataBegin + sect.m_virtualFixupsOffset );
				for( int i = 0; i < sect.getFinishSize() / hkSizeOf(hkInt32); i += 3 )
				{
					int srcOff = finishFixups[i  ];
					if( srcOff == -1 ) continue;
					HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
					int dstSec = finishFixups[i+1];
					int dstOff = finishFixups[i+2];

					// automatically checks for dest section loaded
					void* dstPtr = getSectionDataByIndex(dstSec, dstOff);
					const hkTypeInfo* registeredType = finishObjects.finishLoadedObject( dataBegin+srcOff, static_cast<char*>(dstPtr) );
					// save info to cleanup the object later on
					if (registeredType)
					{
						PRINT(("CTOR\t0x%p\t# %s\n", dataBegin+srcOff, registeredType->getTypeName()));
						m_packfileData->trackObject(dataBegin+srcOff, registeredType);
					}
				}
			}
		}
	}
	else
	{
		// Versioning may have moved/replaced objects. We need to use the tracker
		// to get the latest location of all objects
		typedef hkPointerMap<void*, const char*> Map;
		Map& pmap = m_tracker->m_finish;
		for( Map::Iterator it = pmap.getIterator(); pmap.isValid(it); it = pmap.getNext(it) )
		{
			const hkTypeInfo* registeredType = finishObjects.finishLoadedObject( pmap.getKey(it), pmap.getValue(it) );
			// save info to cleanup the object later on
			if (registeredType)
			{
				PRINT(("CTOR\t0x%p\t# %s\n", pmap.getKey(it), registeredType->getTypeName()));
				m_packfileData->trackObject(pmap.getKey(it), registeredType);
			}
		}
	}
	return HK_SUCCESS;
}

int hkBinaryPackfileReader::getSectionIndex( SectionTag sectionTag )
{
	for( int i = 0; i < m_header->m_numSections; ++i )
	{
		if( hkString::strCmp(m_sections[i].m_sectionTag, sectionTag ) == 0 )
		{
			return i;
		}
	}
	return -1;
}

void* hkBinaryPackfileReader::getSectionDataByIndex( int sectionIndex, int offset )
{
	if( m_sectionData[sectionIndex] != HK_NULL )
	{
		return static_cast<char*>(m_sectionData[sectionIndex]) + offset;
	}
	return HK_NULL;
}


void* hkBinaryPackfileReader::getOriginalContents()
{
	int si = m_header->m_contentsSectionIndex;
	int so = m_header->m_contentsSectionOffset;
	// v1 packfiles have m_contentsSection* == -1
	return (si >= 0) && (so >= 0)
		? getSectionDataByIndex(si,so)
		: getSectionDataByIndex(getSectionIndex("__data__"), 0);
}

const char* hkBinaryPackfileReader::getOriginalContentsClassName()
{
	int si = m_header->m_contentsClassNameSectionIndex;
	int so = m_header->m_contentsClassNameSectionOffset;
	return (si >= 0) && (so >= 0)
		? reinterpret_cast<const char*>(getSectionDataByIndex(si,so))
		: HK_NULL;
}

const char* hkBinaryPackfileReader::getContentsClassName()
{
	return ( m_tracker != HK_NULL )
		? m_tracker->m_topLevelClassName // toplevel may have been versioned
		: getOriginalContentsClassName();
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
