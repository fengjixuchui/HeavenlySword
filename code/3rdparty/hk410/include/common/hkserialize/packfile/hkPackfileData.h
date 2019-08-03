/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_PACKFILE_DATA_H
#define HK_PACKFILE_DATA_H

#include <hkserialize/resource/hkResource.h>
#include <hkbase/htl/hkPointerMap.h>

class hkTypeInfo;

	/// An interface to the data loaded by a packfile reader.
class hkPackfileData : public hkResource
{
	public:

			/// Create an empty object.
		hkPackfileData();

			// Inherited
		virtual ~hkPackfileData();

			// Inherited
		virtual void callDestructors();

			// Inherited
		virtual void getImportsExports( hkArray<Export>& expOut, hkArray<Import>& impOut );

			// Inherited
		virtual const char* getName() const { return m_name; }

			/// Set the name of this data.
		void setName(const char* name);

			/// Prevent destructors from being called.
			/// Normally destructors are automatically called to free any runtime
			/// allocations (typically array resizes). You can prevent this behavior
			/// if you are cleaning up the data in some other way such as explicit
			/// destructor calls.
		void disableDestructors() { m_trackedObjects.clear(); }

	protected:

		struct Chunk
		{
			Chunk(void* p, int n, HK_MEMORY_CLASS c) : pointer(p), numBytes(n), memClass(c) { }
			void* pointer;
			int numBytes;
			HK_MEMORY_CLASS memClass;
		};
	public:
		
		// Internal functions for use by packfile readers.

			// Add a normal allocation.
		void addAllocation(void* p) { m_memory.pushBack(p); }

			// Add a chunk allocation.
		void addChunk(void*p, int n, HK_MEMORY_CLASS c) { m_chunks.pushBack( Chunk(p,n,c) ); }

			// Track object 'o' for cleanup in callDestructors().
		void trackObject(void* o, const hkTypeInfo *t) { m_trackedObjects.insert(o, t); }

			// Make an object visible with a symbolic name.
		void addExport( const char* symbolName, void* object );

			// Link a location to the symbolic name.
		void addImport( const char* symbolName, void** location );

	protected:


		typedef hkPointerMap<void*, const hkTypeInfo*>	TrackedObjectMap;

	protected:

		char* m_name;
		TrackedObjectMap m_trackedObjects;
		hkArray<void*> m_memory;
		hkArray<Chunk> m_chunks;
		hkArray<Export> m_exports;
		hkArray<Import> m_imports;

		friend class hkPackfileObjectUpdateTracker;
};

#endif // HK_PACKFILE_DATA_H

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
