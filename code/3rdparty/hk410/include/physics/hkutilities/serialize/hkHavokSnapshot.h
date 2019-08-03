/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_HAVOKSNAPSHOTPACKFILE_H
#define HK_HAVOKSNAPSHOTPACKFILE_H

#include <hkserialize/packfile/hkPackfileReader.h>
#include <hkserialize/util/hkStructureLayout.h>
#include <hkserialize/packfile/hkPackfileWriter.h>

class hkFinishLoadedObjectRegistry;
class hkVtableClassRegistry;

/// This is a simple way to snapshot (serialize) the whole scene into a file.
/// This can be useful for debugging purposes. 
class hkHavokSnapshot
{
	public:

			/// Converts some objects to different types for snapshots.
		class ConvertListener : public hkPackfileWriter::AddObjectListener
		{
			public:

				~ConvertListener();
				virtual void addObjectCallback( ObjectPointer& p, ClassPointer& k );
				hkArray<hkReferencedObject*> m_objects;
		};

			/// Save a snapshot of the world to filename in packfile form (binary). 
			/// Returns true on success
			/// If you don't provide a target layout then the current host layout is assumed.
			/// NOTE: some objects are inherently not serializable because they point to 
			/// external memory. i.e. The vertex and index arrays of an hkMeshShape. When these objects
			/// are encountered, we convert them before saving. i.e hkMeshShape -> hkStorageMeshShape.
		static hkBool HK_CALL save(const class hkWorld* world, hkStreamWriter* writer, hkBool binaryFormat = true, const hkStructureLayout::LayoutRules* targetLayout = HK_NULL );

			/// Save a snapshot of a hkPhysicsData to filename.
		static hkBool HK_CALL save( const class hkPhysicsData* data, hkStreamWriter* writer, hkBool binaryFormat = true, const hkStructureLayout::LayoutRules* targetLayout = HK_NULL );

			/// Save a snapshot of a given object under a RootLevelContainer to the given stream
		static hkBool HK_CALL saveUnderRootLevel( const void* data, const hkClass& dataClass, hkStreamWriter* writer, hkBool binaryFormat = true, const hkStructureLayout::LayoutRules* targetLayout = HK_NULL );

			/// Save a snapshot of a given object to filename.
		static hkBool HK_CALL save( const void* data, const hkClass& dataClass, hkStreamWriter* writer, hkBool binaryFormat = true, const hkStructureLayout::LayoutRules* targetLayout = HK_NULL );

			/// Load a snapshot from a filename in packfile form. It will search
			/// the root level container in the file for a hkPhysicsData class. It will autodetect
			/// if the stream is a binary packfile or not (then assumed to be XML).
			/// NOTE: remember to remove the reference from allocatedData once you are finished using the loaded data.
		static class hkPhysicsData* HK_CALL load(class hkStreamReader* reader, hkPackfileReader::AllocatedData** allocatedData);
};

#endif // HK_HAVOKSNAPSHOTSESSION_H


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
