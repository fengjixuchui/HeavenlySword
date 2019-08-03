/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SERIALIZE_OBJECT_COPIER_H
#define HK_SERIALIZE_OBJECT_COPIER_H

#include <hkserialize/util/hkStructureLayout.h>
#include <hkserialize/serialize/hkObjectWriter.h>

class hkOArchive;

/// Copy objects using hkClass information.
/// This is an extremely general purpose copier which can
/// convert on the fly between layouts for different compilers,
/// platforms with different pointer sizes and endianness.
class hkObjectCopier : public hkReferencedObject
{
	public:

			/// Create an hkObjectCopier which will copy between the specified layouts.
		hkObjectCopier(const hkStructureLayout& layoutIn, const hkStructureLayout& layoutOut);

			/// Destroy an hkObjectCopier.
		virtual ~hkObjectCopier();

			/// Copy a single object using class information.
			/// The object data is appended to "dataOut". Relocations
			/// are appended to "reloc".
		virtual hkResult copyObject(const void* dataIn, const hkClass& klassIn,
			hkStreamWriter* dataOut, const hkClass& klassOut, hkRelocationInfo& reloc );

			/// Get the source layout.
		const hkStructureLayout& getLayoutOut() const { return m_layoutOut; }

			/// Get the target layout.
		const hkStructureLayout& getLayoutIn() const { return m_layoutIn; }

	private:

		void writeZero( hkOArchive& oa, const hkClassMember& member );
		int saveBody( const void* dataIn, const hkClass& klassIn,
			hkOArchive& dataOut, const hkClass& klassOut );
		void saveExtras( const void* dataIn, const hkClass& klassIn,
			hkOArchive& dataOut, const hkClass& klassOut,
			int classStart, hkRelocationInfo& fixups, int level = 0 );

			/// 
		virtual const hkClass* lookupClass( const hkClass& klass )
		{
			return &klass;
		}

	protected:

		hkStructureLayout m_layoutIn;
		hkStructureLayout m_layoutOut;
		hkBool m_byteSwap;
};

#endif //HK_SERIALIZE_OBJECT_COPIER_H

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
