/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkserialize/hkSerialize.h>

#include <hkbase/config/hkConfigVersion.h>
#include <hkbase/htl/hkPointerMap.h>
#include <hkserialize/util/hkNativePackfileUtils.h>
#include <hkserialize/packfile/binary/hkPackfileSectionHeader.h>
#include <hkserialize/util/hkBuiltinTypeRegistry.h>
#include <hkserialize/util/hkStructureLayout.h>

#if 0 && defined(HK_DEBUG)
#	include <hkbase/fwd/hkcstdio.h>
using namespace std;
#	define TRACE(A) A
#else
#	define TRACE(A) // nothing
#endif

namespace
{
	typedef hkPointerMap<void*, const hkTypeInfo*> TrackedObjectsMap;
	static const int MAGIC = 0xD5109142;

	struct Location
	{
		int m_sectionIndex;
		int m_offset;
	};

	struct InplaceDataHeader
	{
		HK_ALIGN16(int m_magic);
		int m_contentsOffset;
		TrackedObjectsMap m_trackedObjects;
		hkArray<hkPackfileSectionHeader> m_sections;
	};

	HK_COMPILE_TIME_ASSERT( sizeof(InplaceDataHeader) % 16 == 0 );
}

static inline void* _getSectionDataByIndex( void* base, hkArray<hkPackfileSectionHeader>& sections, int sectionIndex, int offset )
{
	if( sections[sectionIndex].getDataSize() ) // not loaded
	{
		HK_ASSERT( 0x4895360e, offset >= 0 );
		HK_ASSERT( 0x4895360f, offset < sections[sectionIndex].getDataSize() );
		return hkAddByteOffset( base, sections[sectionIndex].m_absoluteDataStart + offset );
	}
	return HK_NULL;
}

#define VALIDATE(TEST, ERRMSG) if( (TEST)==false ) { if(errOut) *errOut = (ERRMSG); return HK_FAILURE; }
hkResult hkNativePackfileUtils::validatePackfileHeader( const void* packfileData, const char** errOut )
{
	VALIDATE( packfileData != HK_NULL, "Pointer is null" );
	const hkPackfileHeader* header = static_cast<const hkPackfileHeader*>(packfileData);
	hkPackfileHeader magic;
	VALIDATE( (header->m_magic[0] == magic.m_magic[0]) && (header->m_magic[1] == magic.m_magic[1]), "Missing packfile magic header. Is this from a binary file?");
	const hkStructureLayout::LayoutRules& rules = hkStructureLayout::HostLayoutRules;
	VALIDATE( header->m_layoutRules[0] == rules.m_bytesInPointer, "Trying to process a binary file with a different pointer size than this platform." );
	VALIDATE( header->m_layoutRules[1] == rules.m_littleEndian, "Trying to process a binary file with a different endian than this platform." );
	VALIDATE( header->m_layoutRules[2] == rules.m_reusePaddingOptimization, "Trying to process a binary file with a different padding optimization than this platform." );
	VALIDATE( header->m_layoutRules[3] == rules.m_emptyBaseClassOptimization, "Trying to process a binary file with a different empty base class optimization than this platform." );
	VALIDATE( (hkUlong(packfileData) & 0x3) == 0, "Packfile data source needs to be 4 byte aligned");
	VALIDATE( header->m_contentsVersion[0] != -1, "Packfile file format is too old" );
	VALIDATE( hkString::strCmp(header->m_contentsVersion, HAVOK_SDK_VERSION_STRING) == 0, "Packfile contents are not up to date" );

	return HK_SUCCESS;
}
#undef VALIDATE

int hkNativePackfileUtils::getRequiredBufferSize( const void* packfileData, int packfileSize )
{
	HK_ASSERT2( 0x78b7ad37, packfileSize > hkSizeOf(hkPackfileHeader), "Packfile is too small" );
	HK_ASSERT( 0x78b7ad38, validatePackfileHeader(packfileData, HK_NULL) == HK_SUCCESS );
	const hkPackfileHeader* header = static_cast<const hkPackfileHeader*>(packfileData);

	int dataSize = 0;
	int numTrackedObjects = 0;

	// calculate size of objects
	const hkPackfileSectionHeader* sections = header->m_numSections > 0 ? (const hkPackfileSectionHeader*)(header + 1) : HK_NULL;
	for( int i = 0; i < header->m_numSections; ++i )
	{
		// add data size
		dataSize += sections[i].getDataSize();
		// add number of tracked objects for this section (may be slight overestimate)
		numTrackedObjects += (sections[i].getFinishSize()) / (3*hkSizeOf(hkInt32));
		// add exports+imports
		dataSize += sections[i].getExportsSize() + sections[i].getImportsSize();
	}

	return sizeof(InplaceDataHeader) // header
		+ (sizeof(hkPackfileSectionHeader) * header->m_numSections) // section headers
		+ dataSize // object data + import/export
		+ TrackedObjectsMap::getSizeInBytesFor(numTrackedObjects); // size of the map data
}

void* hkNativePackfileUtils::load( const void* packfileData, int packfileSize, void* outBuffer, int outBufferSize, const hkFinishLoadedObjectRegistry* userRegistry )
{
	// after loading, the output buffer looks like
	// InplaceDataHeader
	// array data of sections
	// Data from all sections concatenated
	// Exports from all sections concatenated
	// Imports from all sections concatenated
	// Map data from m_trackedObjects
	HK_ASSERT2(0xc1e7e32b, (hkUlong(outBuffer) & 0xf) == 0, "Output buffer needs to be 16 byte aligned");
	HK_ASSERT2(0x673e19d4, hkSizeOf(InplaceDataHeader) <= outBufferSize, "The buffer is too small.");
	HK_ASSERT( 0x78b7ad37, validatePackfileHeader(packfileData, HK_NULL) == HK_SUCCESS );

	const hkPackfileHeader* packfileHeader = static_cast<const hkPackfileHeader*>(packfileData);

	// section header
	const hkPackfileSectionHeader* inSections = packfileHeader->m_numSections > 0 ? (const hkPackfileSectionHeader*)(packfileHeader + 1) : HK_NULL;

	// init and fill buffer
	InplaceDataHeader* outBufferHeader = static_cast<InplaceDataHeader*>( outBuffer );
	outBufferHeader->m_magic = MAGIC;
	int outCurOffset = sizeof(InplaceDataHeader);
	new( &outBufferHeader->m_sections ) hkArray<hkPackfileSectionHeader>(
			static_cast<hkPackfileSectionHeader*>(hkAddByteOffset(outBuffer, outCurOffset)),
			packfileHeader->m_numSections,
			packfileHeader->m_numSections );
	hkArray<hkPackfileSectionHeader>& outSections = outBufferHeader->m_sections;
	outCurOffset += sizeof( hkPackfileSectionHeader ) * packfileHeader->m_numSections;

	// process sections data and initialize with data only
	for( int sectionIndex = 0; sectionIndex < packfileHeader->m_numSections; ++sectionIndex )
	{
		const hkPackfileSectionHeader& inSection = inSections[sectionIndex];
		hkPackfileSectionHeader& outSection = outSections[sectionIndex];

		hkString::memCpy(outSection.m_sectionTag, inSection.m_sectionTag, sizeof(inSection.m_sectionTag));
		outSection.m_nullByte = inSection.m_nullByte;
		outSection.m_absoluteDataStart = outCurOffset;
		outSection.m_localFixupsOffset = inSection.getDataSize();
		outSection.m_globalFixupsOffset = inSection.getDataSize();
		outSection.m_virtualFixupsOffset = inSection.getDataSize();
		outSection.m_exportsOffset = inSection.getDataSize();
		outSection.m_importsOffset = inSection.getDataSize() + inSection.getExportsSize();
		outSection.m_endOffset = inSection.getDataSize() + inSection.getExportsSize() + inSection.getImportsSize();

		HK_ASSERT2(0xff668fe7, ( (inSection.m_absoluteDataStart <= packfileSize) && ((inSection.m_absoluteDataStart + inSection.m_endOffset) <= packfileSize) ),
				"Inplace packfile data is too small. Is it corrupt?");
		HK_ASSERT2(0x673e19d4, ( outCurOffset + inSection.getDataSize() <= outBufferSize ), "The buffer is too small.");

		// copy section : data,exports,imports
		const char* inSectionBegin = (const char*)hkAddByteOffsetConst(packfileData, inSection.m_absoluteDataStart);
		char* outSectionBegin = (char*)hkAddByteOffset(outBuffer, outCurOffset);
		hkString::memCpy(outSectionBegin,
				inSectionBegin,
				inSection.getDataSize() );
		hkString::memCpy( outSectionBegin + outSection.m_exportsOffset,
				inSectionBegin + inSection.m_exportsOffset,
				inSection.getExportsSize() );
		hkString::memCpy( outSectionBegin + outSection.m_importsOffset,
				inSectionBegin + inSection.m_importsOffset,
				inSection.getImportsSize() );

		if (sectionIndex == packfileHeader->m_contentsSectionIndex)
		{
			outBufferHeader->m_contentsOffset = outCurOffset + packfileHeader->m_contentsSectionOffset;
		}

		// apply local fixups now
		const int* localFixups = reinterpret_cast<const int*>(inSectionBegin + inSection.m_localFixupsOffset);
		for( int i = 0; i < inSection.getLocalSize() / hkSizeOf(hkInt32); i+=2 )
		{
			int srcOff = localFixups[i  ];
			if( srcOff == -1 ) continue;
			HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
			int dstOff = localFixups[i+1];
			void** addrSrc = reinterpret_cast<void**>(outSectionBegin+srcOff);
			void* dst = reinterpret_cast<void*>(outSectionBegin+dstOff);
			HK_ASSERT2( 0x75936f92, *addrSrc == HK_NULL,
				"Pointer has already been patched. Corrupt file or loadEntireFileInplace called multiple times?");
			*addrSrc = dst;
		}

		outCurOffset += outSection.m_endOffset;
	}


	// apply global fixups now to objects
	for( int sectionIndex = 0; sectionIndex < packfileHeader->m_numSections; ++sectionIndex )
	{
		const hkPackfileSectionHeader& inSection = inSections[sectionIndex];
		char* outSectionBegin = (char*)hkAddByteOffset(outBuffer, outSections[sectionIndex].m_absoluteDataStart );

		const int* globalFixups = reinterpret_cast<const int*>( hkAddByteOffsetConst(packfileData, inSection.m_absoluteDataStart + inSection.m_globalFixupsOffset ) );
		for( int i = 0; i < inSection.getGlobalSize() / hkSizeOf(hkInt32); i += 3 )
		{
			int srcOff = globalFixups[i  ];
			if( srcOff == -1 ) continue;
			HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
			int dstSec = globalFixups[i+1];
			int dstOff = globalFixups[i+2];

			// automatically checks for dest section loaded
			void* dstPtr = _getSectionDataByIndex(outBuffer, outSections, dstSec, dstOff);
			*(hkUlong*)(outSectionBegin+srcOff) = hkUlong(dstPtr);
		}
	}

	new( &outBufferHeader->m_trackedObjects ) TrackedObjectsMap(
		hkAddByteOffset(outBuffer, outCurOffset),
		outBufferSize - outCurOffset );
	const hkFinishLoadedObjectRegistry* finishRegistry = userRegistry != HK_NULL
		? userRegistry
		: hkBuiltinTypeRegistry::getInstance().getFinishLoadedObjectRegistry();
	// finish objects
	for( int sectionIndex = 0; sectionIndex < packfileHeader->m_numSections; ++sectionIndex )
	{
		const hkPackfileSectionHeader& inSection = inSections[sectionIndex];
		void* outSectionBegin = hkAddByteOffset(outBuffer, outSections[sectionIndex].m_absoluteDataStart );
		const int* virtualFixups = reinterpret_cast<const int*>( hkAddByteOffsetConst( packfileData, inSection.m_absoluteDataStart + inSection.m_virtualFixupsOffset ) );
		for( int i = 0; i < inSection.getFinishSize()/hkSizeOf(hkInt32); i += 3 )
		{
			int srcOff = virtualFixups[i  ];
			if( srcOff == -1 ) continue;
			HK_ASSERT( 0xd207ae6b, (srcOff & (sizeof(void*)-1)) == 0 );
			int dstSec = virtualFixups[i+1];
			int dstOff = virtualFixups[i+2];

			// automatically checks for dest section loaded
			void* typeName = _getSectionDataByIndex(outBuffer, outSections, dstSec, dstOff);
			void* objAddress = hkAddByteOffset(outSectionBegin, srcOff);

			const hkTypeInfo* registeredType = finishRegistry->finishLoadedObject( objAddress, static_cast<char*>(typeName) );
			// save info to cleanup the object later on
			if (registeredType)
			{
				TRACE(printf("+ctor\t%s at %p.\n", registeredType->getTypeName(), objAddress));
				outBufferHeader->m_trackedObjects.insert(objAddress, registeredType);
			}
		}
	}
	HK_ASSERT(0x5716f93c, outBufferHeader->m_trackedObjects.wasReallocated() == false);

	return hkAddByteOffset(outBuffer, outBufferHeader->m_contentsOffset);
}

void hkNativePackfileUtils::unload( void* buffer, int bufferSize )
{
	InplaceDataHeader* bufferHeader = reinterpret_cast<InplaceDataHeader*>( buffer );
	HK_ASSERT2(0x673e19d4, hkSizeOf(InplaceDataHeader) <= bufferSize, "The buffer is too small.");
	HK_ASSERT2(0x673e19d5, bufferHeader->m_magic == MAGIC, "The buffer did not come from a load call.");
	bufferHeader->m_magic = 0;
	TrackedObjectsMap& trackedObjectsMap = bufferHeader->m_trackedObjects;

	// cleanup objects
	for( hkPointerMap<const void*, const hkTypeInfo*>::Iterator it = trackedObjectsMap.getIterator();
		trackedObjectsMap.isValid(it); it = trackedObjectsMap.getNext(it) )
	{
		if( hkTypeInfo::CleanupLoadedObjectFunction f = trackedObjectsMap.getValue(it)->getCleanupFunction() )
		{
			TRACE(printf("-dtor\t%s at %p...", trackedObjectsMap.getValue(it)->getTypeName(), trackedObjectsMap.getKey(it)));
			(*f)(trackedObjectsMap.getKey(it));
			TRACE(printf("done.\n"));
		}
	}
	bufferHeader->~InplaceDataHeader();
}

void hkNativePackfileUtils::getImportsExports(void* loadedBuffer, hkArray<hkResource::Export>& expOut, hkArray<hkResource::Import>& impOut )
{
	InplaceDataHeader& header = *static_cast<InplaceDataHeader*>(loadedBuffer);

	for( int sectionIndex = 0; sectionIndex < header.m_sections.getSize(); ++sectionIndex )
	{
		hkPackfileSectionHeader& section = header.m_sections[sectionIndex];
		void* sectionBegin = hkAddByteOffset(loadedBuffer, section.m_absoluteDataStart);
		section.getExports(sectionBegin, expOut);
		section.getImports(sectionBegin, impOut);
	}
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
