/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkserialize/hkSerialize.h>
#include <hkserialize/copier/hkDeepCopier.h>
#include <hkserialize/copier/hkObjectCopier.h>
#include <hkserialize/serialize/platform/hkPlatformObjectWriter.h>
#include <hkserialize/serialize/hkRelocationInfo.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkClassEnum.h>
#include <hkbase/htl/hkPointerMap.h>
#include <hkbase/stream/hkOArchive.h>
#include <hkbase/stream/hkStreambufFactory.h>
#include <hkbase/stream/hkStreamWriter.h>
#include <hkbase/stream/impl/hkOffsetOnlyStreamWriter.h>
#include <hkbase/memory/hkLocalArray.h>

// This cache remembers all classes which have had their
// layouts recomputed for the target platform.
struct hkPlatformObjectWriter::Cache
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT, hkPlatformObjectWriter::Cache);

	~Cache()
	{
		for( int i = 0; i < m_allocations.getSize(); ++i )
		{
			hkDeallocate( m_allocations[i] );
		}
	}

	hkClass* get( const hkClass* klass, const hkStructureLayout& layout )
	{
		void* ret = HK_NULL;
		if( m_platformClassFromHostClass.get(klass, &ret) == HK_FAILURE)
		{
			ret = hkDeepCopier::deepCopy( klass, hkClassClass, &m_platformClassFromHostClass );
			m_allocations.pushBack(ret);
			layout.computeMemberOffsetsInplace( *static_cast<hkClass*>(ret), m_platformClassComputed );
		}
		return static_cast<hkClass*>(ret);
	}

	hkPointerMap<const void*, void*> m_platformClassFromHostClass;
	hkPointerMap<const hkClass*, int> m_platformClassComputed;
	hkArray<void*> m_allocations;
};

hkPlatformObjectWriter::hkPlatformObjectWriter( const hkStructureLayout& layout )
{
	//XXX need to handle homogeneous arrays with differing offsets
	m_copier = new hkObjectCopier( hkStructureLayout::HostLayoutRules, layout );
	// if the source and target rules are identical, we don't need a cache
	m_cache = ( getLayout().getRules() == hkStructureLayout::HostLayoutRules )
		? HK_NULL
		: new Cache;
}

hkPlatformObjectWriter::~hkPlatformObjectWriter()
{
	m_copier->removeReference();
	if( m_cache )
	{
		delete m_cache;
	}
}

hkResult hkPlatformObjectWriter::writeObject( hkStreamWriter* writer, const void* dataIn, const hkClass& klass, hkRelocationInfo& reloc )
{
	const hkClass* classCopy = m_cache
		? m_cache->get(&klass, getLayout())
		: &klass;
	hkOffsetOnlyStreamWriter dummyWriter;
	HK_ON_DEBUG( hkStreamWriter* origWriter = writer);
	if( m_cache && hkString::strCmp(classCopy->getName(), "hkClass") == 0 )
	{
		//
		// The meta data also must be saved using structure layout for the specified platform.
		// We have to call the m_copier->copyObject(...) twice:
		// 1. Save the proper class data updated (member offsets) for specified structure layout
		// (using dummy relocation info - updated class data uses pointers
		// that are discarded later on).
		// 2. "Dummy" save the original class data updating the relocation info.
		//

		const hkClass* classData = m_cache->get(static_cast<const hkClass*>(dataIn), getLayout());

		// Set dummy writer to the actual data pointer position.
		dummyWriter.seek( writer->tell(), hkStreamWriter::STREAM_SET );

		// Save the updated class data into the real stream.
		hkRelocationInfo dummyReloc;
		m_copier->copyObject( classData, klass, writer, *classCopy, dummyReloc );

		writer = &dummyWriter;
	}
	m_copier->copyObject( dataIn, klass, writer, *classCopy, reloc );
	HK_ASSERT( 0x263de130,  writer == origWriter || writer->tell() == origWriter->tell() );
	return writer->isOk() ? HK_SUCCESS : HK_FAILURE;
}

hkResult hkPlatformObjectWriter::writeRaw( hkStreamWriter* writer, const void* data, int dataLen )
{
	return writer->write(data, dataLen) == dataLen
		? HK_SUCCESS
		: HK_FAILURE;
}

const hkStructureLayout& hkPlatformObjectWriter::getLayout() const
{
	return m_copier->getLayoutOut();
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
