/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MESH2_SHAPE_H
#define HK_COLLIDE2_MESH2_SHAPE_H

#include <hkcollide/shape/collection/hkShapeCollection.h>
#include <hkcollide/shape/triangle/hkTriangleShape.h>

extern hkReal hkConvexShapeDefaultRadius;
extern const hkClass hkExtendedMeshShapeClass;

class hkMeshMaterial;
class hkSimpleMeshShape;

#define HK_MESH2SHAPE_EXTRACT_TERMINAL_INDEX(shapeKey)   ( (shapeKey) & ( ~0U >> m_numBitsForSubpartIndex ) )
#define HK_MESH2SHAPE_EXTRACT_SUBPART_INDEX(shapeKey)    ( (shapeKey & 0x7fffffff) >> ( 32 - m_numBitsForSubpartIndex ) )
#define HK_MESH2SHAPE_IS_SUBPART_TYPE_SHAPES(shapeKey)   ( (shapeKey) & 0x80000000 )

/// A class for wrapping geometric collision detection information.  It can directly reference
/// sets of triangle strips with vertex striding, and either 16 or 32 bit indices to vertices.
/// It can also directly reference triangle soups, using three indices per triangle rather than one.
/// It also handles degenerate triangles internally, so no extra checking is required by the user.
/// The mesh shape creates hkTriangleShapes in the ShapeBuffer passed in to the getChildShape function.
/// It gives these shapes a radius (see hkConvexShape::getRadius()) as specified by the hkExtendedMeshShape::getRadius() function.
/// This class can also store lists of convex shapes.
class hkExtendedMeshShape: public hkShapeCollection
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO);

		HK_DECLARE_REFLECTION();

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

		enum SubpartType
		{
			SUBPART_TRIANGLES,
			SUBPART_SHAPE
		};

		struct Subpart
		{
			public:

				HK_DECLARE_REFLECTION();

			public:
				inline Subpart(SubpartType type);

			public:
				hkEnum<SubpartType,hkInt8> m_type;

					/// A type defining whether 8 or 16 bits are used to index material.
				hkEnum<MaterialIndexStridingType,hkInt8> m_materialIndexStridingType;

					/// a pointer, pointing to a strided array of material index (hkUint8), one index for each triangle.
					///  - You are limited to a maximum of 256 materials per subpart.
					///  - You are not forced to store those indices in a hkUint8 array, with the striding m_materialIndexStriding
					///    parameter you can extract this
					///  - If you do not want to use materials, simply set this element to HK_NULL
				const void* m_materialIndexBase; //+nosave

					/// The byte offset between two material indices (Usually sizeof(hkUint8) or...)
				int m_materialIndexStriding;

					/// The base for the material table, the byte offset between two hkMeshMaterials is defined by
					/// m_materialStriding
				const hkMeshMaterial* m_materialBase; //+nosave

					/// The byte offset between two hkMeshMaterials
				int m_materialStriding;

					/// The number of materials, only used for debug checking
				int	m_numMaterials;
		};

			/// A vertices subpart defines a triangle, a triangle list or a triangle strip.
		struct TrianglesSubpart : public hkExtendedMeshShape::Subpart
		{
			public:

				//+vtable(0)
				HK_DECLARE_REFLECTION();

			public:

					/// A partial initializing constructor. It will only set values in Debug
					/// apart from a default material that gets set in Release too.
				inline TrianglesSubpart(); 

					/// the number of triangles
				int	m_numTriangleShapes;

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


					/// The byte offset between two indices triples.
					///  - Eg. (Usually sizeof(hkUint16) if you use triangle strips
					///  - or 3 * sizeof(hkUint16) if you use independent triangles
				int m_indexStriding;

		};

			/// A shapes subpart defines a list of one or more convex shapes of type hkConvexShape.
		struct ShapesSubpart : public hkExtendedMeshShape::Subpart
		{
			public:

				//+vtable(0)
				HK_DECLARE_REFLECTION();

			public:

				 ShapesSubpart( const hkConvexShape*const* childShapes, int numChildShapes, const hkVector4& offset = hkVector4::getZero() );
				 ShapesSubpart( const hkConvexShape*const* childShapes, int numChildShapes, const hkTransform& transform );
				~ShapesSubpart();

			public:

				const hkConvexShape*const*	m_childShapes;
				int 						m_numChildShapes;

				hkBool						m_offsetSet;
				hkBool						m_rotationSet;
				hkTransform					m_transform;
		};

	public:

			/// Constructs a new hkExtendedMeshShape.
			/// This mesh supports triangle soups as well as shape soups.
			///    - The triangles are grouped in subparts and can be scaled and get a radius applied
			///    - The shapes can be grouped in subparts and can be translated on a per subpart basis.
			/// "numBitsForSubpart" is the number of bits used (in the 32 bit shape key) for the subpart index.
			/// Note that the highest bit is used as the type identifier, discerning whether this subpart consists
			/// of triangles or convex shapes.
			/// The remaining bits from the 32 bit shape key are used for the terminal index. By
			/// default numBitsForSubpartIndex is 12, which means the mesh shape can have 2^11 - 1
			/// subparts (0xffffffff is the "invalid" shape key) = 2047, and each subpart can have 2^20 triangles = 1048576.
			/// The subpart is stored in the high bits, so you can extract subpart/terminal indices like this:<br>
			/// int subpartIndex = key >> ( 32 - mymesh->getNumBitsForSubpartIndex() );<br>
			/// int terminalIndex = key & ( ~0U >> mymesh->getNumBitsForSubpartIndex() );
		hkExtendedMeshShape( hkReal radius = hkConvexShapeDefaultRadius, int numBitsForSubpartIndex = 12 );

			/// Destructor. Simply removes the references to all childShapes
		~hkExtendedMeshShape();


		//
		// Subpart access
		//

			/// Adds a triangle subpart. To modify member xxxx of this triangle subpart later on, call getTrianglesSubpartAt(int ).xxxx = yyyy.
		virtual void addTrianglesSubpart( const TrianglesSubpart& part );

			/// Adds a shape subpart. To modify member xxxx of this shape subpart later on, call getShapesSubpartAt(int ).xxxx = yyyy.
		virtual void addShapesSubpart( const ShapesSubpart& part );

			/// Returns the number of all triangle subparts.
		inline int getNumTrianglesSubparts() const;

			/// Returns the number of all shape subparts.
		inline int getNumShapesSubparts() const;

			/// Gets read/write access to a triangle subpart.
		inline TrianglesSubpart& getTrianglesSubpartAt( int i );

			/// Gets const access to a triangle subpart.
		inline const TrianglesSubpart& getTrianglesSubpartAt( int i ) const;

			/// Gets read/write access to a shape subpart.
		inline ShapesSubpart& getShapesSubpartAt( int i );

			/// Gets const access to a shape subpart.
		inline const ShapesSubpart& getShapesSubpartAt( int i ) const;

			/// Gets the subpart that a shape key belongs to.
		inline const Subpart& getSubPart( hkShapeKey shapeKey ) const;

			/// Gets the index of a subpart that a shape key belongs to.
			/// Note that this will only return the index within the subpart's specific type list, i.e. either the index in the triangles list or in the shapes list.
			/// You can get the subpart's type by calling getSubpartType().
		inline int getSubPartIndex( hkShapeKey shapeKey ) const;

			/// Get the terminal shape's index within the subpart.
		inline int getTerminalIndexInSubPart( hkShapeKey key ) const;

			/// Gets the number of bits of a shape key used to encode the subpart.
		inline hkInt32 getNumBitsForSubpartIndex() const;

			/// Get the type of the shape referenced by the supplied shape key.
		inline SubpartType getSubpartType( hkShapeKey key ) const;

		//
		// Scaling and radius access
		//

			/// Set the scaling of all triangle subparts of the shape
		inline const hkVector4&	getScaling() const;

		void setScaling( const hkVector4& scaling ) ;


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


			/// Because the hkExtendedMeshShape references into client data,
			/// it must create a new hkTriangleShape to return to the caller when this function is called.
			/// This triangle is stored in the char* buffer.
			/// Degenerate triangles in the client data are handled gracefully through this method.
		const hkShape* getChildShape( hkShapeKey key, ShapeBuffer& buffer ) const;

			// Gets the mesh material by shape key, or returns HK_NULL if m_materialIndexBase isn't defined
		inline const hkMeshMaterial* getMeshMaterial( hkShapeKey key ) const;
		
			// hkShapeCollection interface implementation 
		virtual hkUint32 getCollisionFilterInfo( hkShapeKey key ) const;


		//
		// hkShape interface
		//

			/// A precise but not very fast implementation of getting an AABB.
 		void getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out  ) const;

			/// The type of this class is a HK_SHAPE_COLLECTION. All shapes it returns are either hkTriangleShape or hkConvexShape.
		virtual hkShapeType getType() const;
		
		virtual void calcStatistics( hkStatisticsCollector* collector) const;

	public:

			/// This member variable to determine the maximum triangle size allowed.
			/// This defaults to 1e-7 (and is used to check against the triangle area squared). If you have algorithms
			/// that fail with triangles passed by this value, you can increase it to make the culling more aggressive.
		static hkReal m_triangleDengeneracyTolerance;


	protected:

		void assertTrianglesSubpartValidity( const TrianglesSubpart& part );
		void assertShapesSubpartValidity   ( const ShapesSubpart&    part );

	public:

		hkVector4							m_scaling;
		hkInt32								m_numBitsForSubpartIndex;

		hkArray<struct TrianglesSubpart>	m_trianglesSubparts;
		hkArray<struct ShapesSubpart>		m_shapesSubparts;

	public:

		hkExtendedMeshShape( hkFinishLoadedObjectFlag flag );

	protected:
			/// The radius can only be set on construction.
		hkReal m_radius;
		int    m_pad[3]; // pad so same layout on 4x[10]x

};


#include <hkcollide/shape/extendedmeshshape/hkExtendedMeshShape.inl>


#endif // HK_COLLIDE2_MESH2_SHAPE_H

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
