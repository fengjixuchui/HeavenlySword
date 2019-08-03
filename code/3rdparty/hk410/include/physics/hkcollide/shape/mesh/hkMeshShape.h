/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MESH_SHAPE_H
#define HK_COLLIDE2_MESH_SHAPE_H

#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>

extern hkReal hkConvexShapeDefaultRadius;
extern const hkClass hkMeshShapeClass;

class hkMeshMaterial;
class hkSimpleMeshShape;

/// A class for wrapping geometric collision detection information.  It can directly reference
/// sets of triangle strips with vertex striding, and either 16 or 32 bit indices to vertices.
/// It can also directly reference triangle soups, using three indices per triangle rather than one.
/// It also handles degenerate triangles internally, so no extra checking is required by the user.
/// The mesh shape creates hkTriangleShapes in the ShapeBuffer passed in to the getChildShape function.
/// It gives these shapes a radius (see hkConvexShape::getRadius())  as specified by the hkMeshShape::getRadius() function.
class hkMeshShape: public hkShapeCollection
{
	public:

		HK_DECLARE_REFLECTION();

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO);


			/// Constructs a new hkMeshShape.
			/// "numBitsForSubpart" is the number of bits used (in the 32 bit shape key) for the subpart index.
			/// The remaining bits from the 32 bit shape key are used for the triangle index. By
			/// default numBitsForSubpartIndex is 12, which means the mesh shape can have 2^12 - 1
			/// subparts (0xffffffff is the "invalid" shape key) = 4095, and each subpart can have 2^20 triangles = 1048576.
			/// The subpart is stored in the high bits, so you can extract subpart/triangle indices like this:<br>
			/// int subPart = key >> ( 32 - mymesh->getNumBitsForSubpartIndex() );<br>
			/// int triIndex = key & ( 0xffffffff >> mymesh->getNumBitsForSubpartIndex() );
		hkMeshShape( hkReal radius = hkConvexShapeDefaultRadius, int numBitsForSubpartIndex = 12 );


		//
		// Subpart access
		//

		struct Subpart;

			/// Adds a subpart. To modify this subpart later on, call getSubpartAt(int ).xxxx = xxxx.
		virtual void addSubpart( const Subpart& part );

			/// Returns the number of subparts.
		inline int getNumSubparts() const;

			/// Gets read/write access to a subpart.
		inline Subpart& getSubpartAt( int i );

			/// Gets const access to a subpart.
		inline const Subpart& getSubpartAt( int i ) const;

			/// Gets the subpart that a shape key belongs to.
		inline const Subpart& getSubPart( hkShapeKey shapeKey ) const;

			/// Gets the number of bits of a shape key used to encode the subpart.
		inline hkInt32 getNumBitsForSubpartIndex() const;

		//
		// Scaling and radius access
		//

			/// Set the scaling of the mesh shape
		void setScaling( const hkVector4& scaling );

			/// Get the scaling of the mesh shape
		inline const hkVector4&	getScaling() const;


			/// Gets the extra radius for every triangle.
		inline hkReal getRadius() const;

			/// Sets the extra radius for every triangle.
		inline void setRadius(hkReal r );

		//
		// hkShape Collection interface 
		//


			/// Get the first child shape key.
		virtual hkShapeKey getFirstKey() const;

			/// This function implements hkShapeCollection::getNextKey
			/// NOTE: This function calls hkTriangleUtil::isDegenerate to make sure no keys for degenerate triangles are returned
			/// If you are implementing your own mesh shape, your getNextKey function must make sure that it similarly does
			/// not return keys for degenerate triangles. You can use the hkTriangleUtil::isDegenerate utility function to check whether
			/// triangles are valid.
		virtual hkShapeKey getNextKey( hkShapeKey oldKey ) const;


			/// Because the hkMeshShape references into client data,
			/// it must create a new hkTriangleShape to return to the caller when this function is called.
			/// This triangle is stored in the char* buffer.
			/// Degenerate triangles in the client data are handled gracefully through this method.
		const hkShape* getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const;

			/// Gets the mesh material by shape key or -1 if there is no material indices.
		inline int getMaterialIndex( hkShapeKey key ) const;

			/// Gets the mesh material by shape key, or returns HK_NULL if m_materialIndexBase isn't defined
		inline const hkMeshMaterial* getMeshMaterial( hkShapeKey key ) const;
		
			/// Returns getMeshMaterial(key)->m_filterInfo or zero if there is no material for the key.
		virtual hkUint32 getCollisionFilterInfo( hkShapeKey key ) const;


		//
		// hkShape interface
		//

			/// A precise but not very fast implementation of getting an AABB.
 		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			/// The type of this class is a HK_TRIANGLE_COLLECTION. This means that all shapes it returns
			/// are hkTriangleShape.
		virtual hkShapeType getType() const;
		
		virtual void calcStatistics( hkStatisticsCollector* collector) const;

	public:

			/// This member variable to determine the maximum triangle size allowed.
			/// This defaults to 1e-7 (and is used to check against the triangle area squared). If you have algorithms
			/// that fail with triangles passed by this value, you can increase it to make the culling more aggressive.
		static hkReal m_triangleDengeneracyTolerance;


	public:

			/// The striding of mesh indices
		enum IndexStridingType
		{
			INDICES_INVALID, // default, will raise assert.
				/// 16 bit "short" striding
			INDICES_INT16,
				/// 32 bit "int" striding
			INDICES_INT32,
			INDICES_MAX_ID
		};

		enum MaterialIndexStridingType
		{
			MATERIAL_INDICES_INVALID,
			MATERIAL_INDICES_INT8,
			MATERIAL_INDICES_INT16,
			MATERIAL_INDICES_MAX_ID
		};

			/// A subpart defines a triangle, a triangle list or a triangle strip.
		struct Subpart
		{
			HK_DECLARE_REFLECTION();

				/// A partial initializing constructor. It will only set values in Debug
				/// apart from a default material that gets set in Release too.
			inline Subpart(); 

				//
				//	Vertex information
				//
				/// A pointer to the first vertex, defined by three floats.
			const float*  m_vertexBase; //+nosave

				/// The byte offset between two consecutive vertices (usually 12, 16 or more).
			int		m_vertexStriding;

				/// The number of vertices.
			int		m_numVertices;

				//
				// Triangle Index Information
				//

				/// A pointer to triples of vertex indices.
				/// Used to be a union type, but to make
				/// auto serialization possible, we leave it as 
				/// a void* here.
			const void*	m_indexBase; //+nosave
				
				/// A type defining whether 16 or 32 bits are used to index vertices.
			hkEnum<IndexStridingType,hkInt8> m_stridingType;

				/// A type defining whether 8 or 16 bits are used to index material.
			hkEnum<MaterialIndexStridingType,hkInt8> m_materialIndexStridingType;

				/// The byteoffset between two indices triples.
				///  - Eg. (Usually sizeof(hkUint16) if you use triangle strips
				///  - or 3 * sizeof(hkUint16) if you use independent triangles
			int m_indexStriding;

				/// The number of index triples, which is equal to the number of triangles.
			int	m_numTriangles;

				//
				//	Per triangle material Id info
				//

				/// Pointer to a strided array of material index (hkUint8 or hkUint16), one index for each triangle.
				///  - You are limited to a maximum of 256 or 65535 materials per subpart.
				///  - The indices may be stored in an interleaved array by setting m_materialIndexStriding appropriately.
				///  - If you do not want to use materials, simply set this element to HK_NULL
			const void* m_materialIndexBase; //+nosave

				/// The byteoffset between two material indices
				/// This will be sizeof(hkUint8) or sizeof(hkUint16) for non-interleaved arrays.
			int m_materialIndexStriding;


				/// The base for the material table, the byteoffset between two hkMeshMaterials is defined by
				/// m_materialStriding. If you are storing your materials externally and not using per-triangle filtering
				/// set this element to HK_NULL. The material array may be shared between meshes.
			const hkMeshMaterial* m_materialBase; //+nosave

				/// The byteoffset between two hkMeshMaterials
			int m_materialStriding;

				/// The number of materials, only used for debug checking
			int	m_numMaterials;
		};


	public:

		hkVector4					m_scaling;
		hkInt32						m_numBitsForSubpartIndex;


		//hkInplaceArray<struct Subpart,1> m_subparts;
		hkArray<struct Subpart> m_subparts;

	public:

		hkMeshShape( hkFinishLoadedObjectFlag flag );

	protected:
			/// The radius can only be set on construction.
		hkReal						m_radius;
		int m_pad[3]; // pad so same layout on 4x[10]x

		void assertSubpartValidity( const Subpart& part );
};


#include <hkcollide/shape/mesh/hkMeshShape.inl>


#endif // HK_COLLIDE2_MESH_SHAPE_H

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
