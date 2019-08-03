/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkserialize/hkSerialize.h>
#include <hkbase/stream/hkStreamWriter.h>
#include <hkbase/htl/hkObjectArray.h>
#include <hkbase/htl/hkAlgorithm.h>
#include <hkserialize/packfile/xml/hkXmlPackfileWriter.h>
#include <hkserialize/serialize/xml/hkXmlObjectWriter.h>

hkXmlPackfileWriter::hkXmlPackfileWriter()
{
	// Types must come first since we cannot read objects
	// without their type info and we cannot seek to find it.
	addSection( SECTION_TAG_TYPES );
}

namespace
{
	struct PackfileNameFromAddress : public hkXmlObjectWriter::NameFromAddress
	{
		typedef hkPointerMap<const void*, int> IntFromPtrMap;
		typedef hkPointerMap<const void*, const void*> PtrFromPtrMap;
		typedef hkPointerMap<const void*, const char*> StrFromPtrMap;

		const IntFromPtrMap& m_knownObjects;
		const PtrFromPtrMap& m_replacements;
		const StrFromPtrMap& m_imports;
		IntFromPtrMap  m_indexFromAddress;

		PackfileNameFromAddress(
			const IntFromPtrMap& known,
			const PtrFromPtrMap& replace,
			const StrFromPtrMap& imports ) :
			m_knownObjects(known),
			m_replacements(replace),
			m_imports(imports)
		{
		}

		void ignore( const void* p )
		{
			m_indexFromAddress.insert(p,0);
		}

		int nameFromAddress(const void* address, char* name, int nameLen )
		{
			int index = 0;
			if( address )
			{
				while( const void* replacement = m_replacements.getWithDefault(address, HK_NULL) )
				{
					address = replacement;
				}
				const char* importName = m_imports.getWithDefault(address, HK_NULL);
				if( importName )
				{
					hkString::strNcpy( name, "@", nameLen );
					hkString::strNcpy( name+1, importName, nameLen-1 );
					return hkString::strLen(name);
				}

				index = m_indexFromAddress.getWithDefault( address, -1 );
				if( index == -1 )
				{
					// have not seen this pointer, should we ignore it?
					if( m_knownObjects.getWithDefault(address, 0) == -1 /*hkPackfileWriter::INDEX_IGNORE*/ )
					{
						index = 0;
					}
					else // create a new id
					{
						index = m_indexFromAddress.getSize() + 1;
						m_indexFromAddress.insert( address, index );
					}
				}
			}
			if( index )
			{
				return hkString::snprintf(name, nameLen, "#%04i", index);
			}
			else
			{
				hkString::strNcpy(name, "null", nameLen);
				return 4;
			}
		}
	};
}

hkResult hkXmlPackfileWriter::save( hkStreamWriter* stream, const Options& options )
{
	PackfileNameFromAddress namer( m_knownObjects, m_replacements, m_imports );
	hkXmlObjectWriter writer(namer);
	int classSectionIndex = static_cast<int>( sectionTagToIndex( SECTION_TAG_TYPES ) );
	
	// maybe skip all class objects
	if( options.m_writeMetaInfo == false )
	{
		for( int pendingIndex = 0; pendingIndex < m_pendingWrites.getSize(); ++pendingIndex )
		{
			PendingWrite& pending = m_pendingWrites[pendingIndex];
			if( pending.m_sectionIndex == classSectionIndex )
			{
				namer.ignore( pending.m_origPointer );
				if( pending.m_origPointer != pending.m_pointer)
				{
					namer.ignore( pending.m_pointer );
				}
			}
		}
	}

	// packfile header
	hkOstream os(stream);
	{
		os.printf("<?xml version=\"1.0\" encoding=\"ascii\"?>\n");
		const char* contentsversion = options.m_contentsVersion;
		char defaultContentsVersion[32];
		if( contentsversion == HK_NULL )
		{
			getCurrentVersion(defaultContentsVersion,sizeof(defaultContentsVersion));
			contentsversion = defaultContentsVersion;
		}
		os.printf("<hkpackfile classversion=\"4\" contentsversion=\"%s\">\n", contentsversion);
		writer.adjustIndent(1);
	}

	// write comments about ignored objects
	for( int skippedIndex = 0; skippedIndex < m_objectsWithUnregisteredClass.getSize(); ++skippedIndex )
	{
		hkVariant& v = m_objectsWithUnregisteredClass[skippedIndex];
		os.printf("\t<!-- Skipped %s at address %p -->\n", v.m_class->getName(), v.m_object );
	}
	
	// body of the packfile
	for( int sectionIndex = 0; sectionIndex < m_knownSections.getSize(); ++sectionIndex )
	{
		if( options.m_writeMetaInfo || sectionIndex != classSectionIndex )
		{
			os.printf("\n\t<hksection name=\"%s\">\n", m_knownSections[sectionIndex]);
			writer.adjustIndent(1);
			for( int pendingIndex = 0; pendingIndex < m_pendingWrites.getSize(); ++pendingIndex )
			{
				PendingWrite& pending = m_pendingWrites[pendingIndex];
				if( pending.m_sectionIndex == sectionIndex )
				{
					char namebuffer[256];
					namer.nameFromAddress( pending.m_origPointer, namebuffer, sizeof(namebuffer) );
					const char* attributes[5];

					char sigbuffer[2+8+1]; // (0x) + (ffffffff) + (nul)
					hkString::snprintf(sigbuffer, sizeof(sigbuffer), "%#08x", pending.m_klass->getSignature() );
					attributes[0] = "signature";
					attributes[1] = sigbuffer;
					if( const char* exported = m_exports.getWithDefault(pending.m_pointer, HK_NULL) )
					{
						attributes[2] = "export";
						attributes[3] = exported;
						attributes[4] = 0;
					}
					else
					{
						attributes[2] = 0;
					}
					writer.writeObjectWithElement( stream, pending.m_pointer, *pending.m_klass, namebuffer, attributes );
					os.printf("\n");
				}
			}
			writer.adjustIndent(-1);
			os.printf("\n\t</hksection>");
		}
	}

	// footer
	writer.adjustIndent(-1);
	os.printf("\n\n</hkpackfile>\n");
	
	return HK_SUCCESS;
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
