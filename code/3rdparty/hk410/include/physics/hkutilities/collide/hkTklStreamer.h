/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_TKL_STREAMER_H
#define HK_UTILITIES2_TKL_STREAMER_H


class hkOArchive;
class hkIArchive;
class hkOstream;
class hkIstream;
class hkSimpleMeshShape;

/// These class implements a very basic streaming of a hkSimpleMeshShape object to and from an
/// Archive (platform independent stream) in the tkl file format. This format is in the following form:
/// int: num vertices, triples of 32 bit floats for vertices, int: num triangles, triples of ints for triangle indices
/// btkl is the binary version of this format

class hkTklStreamer
{
public:
	
	/// Allocate and fill a hkSimpleMeshShape from an Input Stream of data in the tkl format
	static hkSimpleMeshShape* HK_CALL readStorageMeshFromTklStream(hkIstream &inputStream);
	
	/// Write a hkSimpleMeshShape to an Output Stream in tkl format. 
	static void HK_CALL writeStorageMeshShapeToTklStream(hkSimpleMeshShape* shape, hkOstream &outputStream);

	/// Allocate and fill a hkSimpleMeshShape from an Input Archive of data in the btkl format
	static hkSimpleMeshShape* HK_CALL readStorageMeshFromBtklArchive(hkIArchive &inputArchive);
	
	/// Write a hkSimpleMeshShape to an Output Archive in btkl format. 
	static void HK_CALL writeStorageMeshShapeToBtklArchive(hkSimpleMeshShape* shape, hkOArchive &outputArchive);
	
};

#endif // HK_UTILITIES2_TKL_STREAMER_H

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
