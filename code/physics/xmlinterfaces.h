//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/system.h
//!	
//!	DYNAMICS COMPONENT:
//!		The system encapsulate the physical representation of an entity.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.07.11
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_XML_INTERFACES_INC
#define _DYNAMICS_XML_INTERFACES_INC

#include "config.h"
#include <hkbase\config\hkConfigVersion.h>

#include "anim/hierarchy.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "anim/hierarchy.h"
#include "rigidbody.h"
#include "maths_tools.h"
#include "world.h"
#include "core/exportstruct_clump.h"
#include "meshshape.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkutilities/collide/hkShapeShrinker.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkcollide/shape/cylinder/hkCylinderShape.h>
#include <hkcollide/shape/heightfield/hkHeightFieldShape.h>
#include <hkcollide/shape/list/hkListShape.h>
#include <hkcollide/shape/mesh/hkMeshShape.h>
#include <hkcollide/shape/transform/hkTransformShape.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/shape/convexvertices/hkConvexVerticesShape.h>
#include <hkcollide/shape/convexlist/hkConvexListShape.h>
#include <hkinternal/preprocess/convexhull/hkGeometryUtility.h>
#include <hkutilities/inertia/hkinertiatensorcomputer.h>

#include <hkdynamics\constraint\bilateral\hinge\hkHingeConstraintData.h>
#include <hkdynamics\constraint\hkConstraintInstance.h>
#include <hkdynamics/constraint/motor/hkConstraintMotor.h>
#include <hkdynamics\constraint\motor\position\hkPositionConstraintMotor.h>
#include <hkdynamics\constraint\motor\velocity\hkVelocityConstraintMotor.h>
#include <hkdynamics\constraint\motor\springdamper\hkSpringDamperConstraintMotor.h>
#include <hkdynamics\constraint\bilateral\ballandsocket\hkBallAndSocketConstraintData.h>
#include <hkdynamics\constraint\bilateral\limitedhinge\hkLimitedHingeConstraintData.h>
#include <hkdynamics\constraint\bilateral\prismatic\hkPrismaticConstraintData.h>
#include <hkdynamics\constraint\bilateral\stiffspring\hkStiffSpringConstraintData.h>
#include <hkdynamics\constraint\bilateral\wheel\hkWheelConstraintData.h>
#include <hkdynamics\constraint\breakable\hkBreakableConstraintData.h>
#include <hkdynamics\constraint\malleable\hkMalleableConstraintData.h>
#include <hkinternal/dynamics/constraint/bilateral/ragdoll/hkRagdollConstraintData.h>
#include <hkutilities/actions/spring/hkSpringAction.h>

#include <hkcollide/shape/mopp/hkMoppUtility.h>
#include <hkcollide/shape/mopp/hkMoppBvTreeShape.h>
#endif

#include "physicsLoader.h"


enum MOTION_TYPE { 
	STATIC = 0, 
	KEYFRAMED, 
	DYNAMIC, 
};
 
enum QUALITY { 
	HS_COLLIDABLE_QUALITY_FIXED = 0, 
	HS_COLLIDABLE_QUALITY_KEYFRAMED, 
	HS_COLLIDABLE_QUALITY_DEBRIS,
	HS_COLLIDABLE_QUALITY_MOVING, 
	HS_COLLIDABLE_QUALITY_CRITICAL, 
	HS_COLLIDABLE_QUALITY_BULLET 
};

#define EXTRA_RADIUS_FOR_CONVEX 0.02f // see hkConvexShape::setRadius 

// True if shape type was inhereted from hkConvexShape
inline bool IsConvex(hkShape * shape)
{
	switch (shape->getType())
	{
	case HK_SHAPE_CONVEX :
	case HK_SHAPE_SPHERE :
	case HK_SHAPE_CYLINDER :
	case HK_SHAPE_TRIANGLE :
	case HK_SHAPE_BOX :
	case HK_SHAPE_CAPSULE :
	case HK_SHAPE_CONVEX_VERTICES :
	case HK_SHAPE_CONVEX_PIECE :
	case HK_SHAPE_CONVEX_TRANSLATE :
	case HK_SHAPE_CONVEX_TRANSFORM :
		return true;
	default:
		return false;
	}
}

inline bool AreConvex(hkShape ** shape, int n)
{
	for(int i = 0; i < n; i++)
		if (!IsConvex(shape[i]))
			return false;

	return true;
}

// Returns true if shapes can be added into one hkConvexListShape
inline bool AreReadyForConvexListShape(hkShape ** shape, int n)
{
	if (!AreConvex(shape,n))
			return false;

	float radius = static_cast<hkConvexShape *>(shape[0])->getRadius();
	for(int i = 1; i < n; i++)
		if (fabs(static_cast<hkConvexShape *>(shape[i])->getRadius() - radius) > 0.01f)
			return false;

	return true;
}

//Creates hkConvexListShape if it is possible, hkListShape otherwise. 
hkShape * CreateListShape(hkShape ** list, int n);

//Creates hkConvexTransformShape if shape is convex, hkTransformShape otherwise.
// BEWARE: function also removeReference() from input shape. 
hkShape * CreateTransformShape(hkShape *shape, const hkTransform& trf);



class psShape
{
	public:
		//! Data. --------------------------------------------------------------------
		uint32_t m_iMaterialId;

		//! Accessors. ---------------------------------------------------------------
		uint32_t	GetMaterialId( void ) const				{ return m_iMaterialId; };
		void		SetMaterialId( const uint32_t &iMaterialId )	{ m_iMaterialId = iMaterialId; };

		virtual ~psShape() {};

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		virtual hkShape* BuildShape() {return NULL;};
		virtual hkShape* BuildStaticShape() {return BuildShape();};
#endif
};

class psShapeTrfLink
{
	public:
		int				m_trfIndex;
		psShape *		m_shape;
};

inline bool TransformWouldBeIdentity( const CQuat &rotation, const CPoint &translation )
{
	ntError( rotation.IsNormalised() );
	if ( !rotation.IsNormalised() )
	{
		// This is just a default pass through - shouldn't ever be here though.
		return false;
	}

	if ( fabsf( rotation.W() - 1.0f ) < 0.001f && translation.Length() < 0.001f )
	{
		return true;
	}

	return false;
}

// ---------------------------------------------------------------
//	This class is used by the xml parser to load the physics data.
// ---------------------------------------------------------------
class psBoxShape : public psShape
{
	public:
		//! Data. --------------------------------------------------------------------
		CPoint		m_fTranslation;
		CQuat		m_fRotation;
		CVector		m_obHalfExtent;

		//! Accessors. ---------------------------------------------------------------
		CPoint		GetPosition( void ) const				{ return m_fTranslation; }
		void		SetPosition( const CPoint &obPoint )	{ m_fTranslation = obPoint; }

		CQuat		GetRotation( void ) const				{ return m_fRotation; }
		void		SetRotation( const CQuat& obRot )		{ m_fRotation = obRot; }

		CVector		GetHalfExtent( void ) const				{ return m_obHalfExtent; }
		void		SetHalfExtent( const CVector &obPoint )	{ m_obHalfExtent = obPoint; }

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		virtual hkShape* BuildShape()
		{
			m_fRotation.Normalise();

			hkBoxShape* shape		= HK_NEW hkBoxShape(	Physics::MathsTools::CVectorTohkVector( m_obHalfExtent ) );
			
			shape->setRadius( EXTRA_RADIUS_FOR_CONVEX );		
			// EEKK! this was only being done on the PC build which means all our collision have been different between
			// the two... this really needs to be done on export
			hkShapeShrinker::shrinkByConvexRadius( shape, 0 );

			Physics::PhysicsMaterialTable& matTable = Physics::PhysicsMaterialTable::Get();
#if HAVOK_SDK_VERSION_MAJOR == 4 && HAVOK_SDK_VERSION_MINOR == 0
			shape->setUserData(matTable.GetMaterialFromId(m_iMaterialId));
#else
			shape->setUserData((hkUlong) matTable.GetMaterialFromId( m_iMaterialId));
#endif // HAVOK_SDK_VERSION_MAJOR
			
			if ( TransformWouldBeIdentity( m_fRotation, m_fTranslation ) )
			{
				return shape;
			}

			hkTransform trf			= hkTransform(		Physics::MathsTools::CQuatTohkQuaternion( m_fRotation ) , 
														Physics::MathsTools::CPointTohkVector( m_fTranslation ) );

			hkShape* s = HK_NEW hkConvexTransformShape( shape, trf );			
			s->setUserData(shape->getUserData());
			shape->removeReference();
			
			return s;
		}
#endif
		
};

// ---------------------------------------------------------------
//	This class is used by the xml parser to load the physics data.
// ---------------------------------------------------------------
class psCapsuleShape : public psShape
{
	public:
		//! Data. --------------------------------------------------------------------
		CPoint		m_fTranslation;
		CQuat		m_fRotation;
		CVector		m_obVertexA;
		CVector		m_obVertexB;
		float		m_fRadius;

		//! Accessors. ---------------------------------------------------------------
		CPoint		GetPosition( void ) const				{ return m_fTranslation; }
		void		SetPosition( const CPoint &obPoint )	{ m_fTranslation = obPoint; }

		CQuat		GetRotation( void ) const				{ return m_fRotation; }
		void		SetRotation( const CQuat& obRot )		{ m_fRotation = obRot; }

		CVector		GetVertexA( void ) const				{ return m_obVertexA; }
		void		SetVertexA( const CVector &obPoint )	{ m_obVertexA = obPoint; }
			
		void		SetVertexB( const CVector &obPoint )	{ m_obVertexB = obPoint; }
		CVector		GetVertexB( void ) const				{ return m_obVertexB; }

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		virtual hkShape* BuildShape()
		{
			m_fRotation.Normalise();

			hkCapsuleShape* shape	= HK_NEW hkCapsuleShape(	Physics::MathsTools::CVectorTohkVector(m_obVertexA), 
																Physics::MathsTools::CVectorTohkVector(m_obVertexB), 
																m_fRadius );
			Physics::PhysicsMaterialTable& matTable = Physics::PhysicsMaterialTable::Get();
#if HAVOK_SDK_VERSION_MAJOR == 4 && HAVOK_SDK_VERSION_MINOR == 0
			shape->setUserData(matTable.GetMaterialFromId(m_iMaterialId));
#else
			shape->setUserData((hkUlong) matTable.GetMaterialFromId( m_iMaterialId));
#endif //HAVOK_SDK_VERSION_MINOR;


			// Don't apply a transform is we don't need one.
			if ( TransformWouldBeIdentity( m_fRotation, m_fTranslation ) )
			{
				return shape;
			}

			hkTransform trf			= hkTransform(			Physics::MathsTools::CQuatTohkQuaternion( m_fRotation ) , 
															Physics::MathsTools::CPointTohkVector( m_fTranslation ) );
            hkShape * s = HK_NEW hkConvexTransformShape( shape, trf );			
			s->setUserData(shape->getUserData());
			shape->removeReference();
			return s;
		}
#endif
};

// ---------------------------------------------------------------
//	This class is used by the xml parser to load the physics data.
// ---------------------------------------------------------------
class psCylinderShape : public psShape
{
	public:
		//! Data. --------------------------------------------------------------------
		CPoint		m_fTranslation;
		CQuat		m_fRotation;
		CVector		m_obVertexA;
		CVector		m_obVertexB;
		float		m_fRadius;

		//! Accessors. ---------------------------------------------------------------
		CPoint		GetPosition( void ) const				{ return m_fTranslation; }
		void		SetPosition( const CPoint &obPoint )	{ m_fTranslation = obPoint; }

		CQuat		GetRotation( void ) const				{ return m_fRotation; }
		void		SetRotation( const CQuat& obRot )		{ m_fRotation = obRot; }

		CVector		GetVertexA( void ) const				{ return m_obVertexA; }
		void		SetVertexA( const CVector &obPoint )	{ m_obVertexA = obPoint; }
			
		void		SetVertexB( const CVector &obPoint )	{ m_obVertexB = obPoint; }
		CVector		GetVertexB( void ) const				{ return m_obVertexB; }

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		virtual hkShape* BuildShape()
		{
			m_fRotation.Normalise();

			hkCylinderShape* shape	= HK_NEW hkCylinderShape(	Physics::MathsTools::CVectorTohkVector(m_obVertexA), 
																Physics::MathsTools::CVectorTohkVector(m_obVertexB), 
																m_fRadius );

			Physics::PhysicsMaterialTable& matTable = Physics::PhysicsMaterialTable::Get();
#if HAVOK_SDK_VERSION_MAJOR == 4 && HAVOK_SDK_VERSION_MINOR == 0
			shape->setUserData(matTable.GetMaterialFromId(m_iMaterialId));
#else
			shape->setUserData((hkUlong) matTable.GetMaterialFromId( m_iMaterialId));
#endif //HAVOK_SDK_VERSION_MINOR			

			shape->setRadius( EXTRA_RADIUS_FOR_CONVEX);
			// EEKK! this was only being done on the PC build which means all our collision have been different between
			// the two... this really needs to be done on export
			hkShapeShrinker::shrinkByConvexRadius( shape, 0 );

			

			if ( TransformWouldBeIdentity( m_fRotation, m_fTranslation ) )
			{
				return shape;
			}

			hkTransform trf			= hkTransform(			Physics::MathsTools::CQuatTohkQuaternion( m_fRotation ) , 
															Physics::MathsTools::CPointTohkVector( m_fTranslation ) );

			hkShape* s = HK_NEW hkConvexTransformShape( shape, trf );			
			s->setUserData(shape->getUserData());
			shape->removeReference();
			return s;
		}
#endif	
};

// ---------------------------------------------------------------
//	This class is used by the xml parser to load the physics data.
// ---------------------------------------------------------------
class psSphereShape : public psShape
{
	public:

		//! Data. --------------------------------------------------------------------
		CPoint		m_fTranslation;
		CQuat		m_fRotation;
		float		m_fRadius;

		//! Accessors. ---------------------------------------------------------------
		CPoint		GetPosition( void ) const				{ return m_fTranslation; }
		void		SetPosition( const CPoint &obPoint )	{ m_fTranslation = obPoint; }

		CQuat		GetRotation( void ) const				{ return m_fRotation; }
		void		SetRotation( const CQuat& obRot )		{ m_fRotation = obRot; }

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		virtual hkShape* BuildShape()
		{
			m_fRotation.Normalise();

			hkSphereShape* shape	= HK_NEW hkSphereShape( m_fRadius );	

			Physics::PhysicsMaterialTable& matTable = Physics::PhysicsMaterialTable::Get();
#if HAVOK_SDK_VERSION_MAJOR == 4 && HAVOK_SDK_VERSION_MINOR == 0
			shape->setUserData(matTable.GetMaterialFromId(m_iMaterialId));
#else
			shape->setUserData((hkUlong) matTable.GetMaterialFromId( m_iMaterialId));
#endif //HAVOK_SDK_VERSION_MINOR

			// Don't apply a transform is we don't need one.
			if ( TransformWouldBeIdentity( m_fRotation, m_fTranslation ) )
			{
				return shape;
			}

			hkTransform trf			= hkTransform(	Physics::MathsTools::CQuatTohkQuaternion( m_fRotation ) , 
													Physics::MathsTools::CPointTohkVector( m_fTranslation ) );
			hkShape* s = HK_NEW hkConvexTransformShape( shape, trf );
			s->setUserData(shape->getUserData());
			shape->removeReference();
			return s;
		}
#endif	
};

// ---------------------------------------------------------------
//	This class is used by the xml parser to load the physics data.
// ---------------------------------------------------------------

class psListShape : public psShape
{
public:

	//! Data. --------------------------------------------------------------------

	typedef ntstd::List<void*,MC_PHYSICS> ShapeGuidList;
	ShapeGuidList		m_shapesGuids;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD	
	virtual hkShape* BuildShape()
	{
		hkShape** shapeArray = NT_NEW_ARRAY_CHUNK ( MC_PHYSICS ) hkShape*[m_shapesGuids.size()];

		ShapeGuidList::iterator it = m_shapesGuids.begin();
		for( unsigned int i = 0; i < m_shapesGuids.size(); i++ )
		{
			void* shapeptr = *it;
			psShape* shape = (psShape*)shapeptr;
			shapeArray[i] = shape->BuildShape();		    
			++it;
		}	
		hkShape* shape = CreateListShape( shapeArray, m_shapesGuids.size());
		NT_DELETE_ARRAY_CHUNK( MC_PHYSICS, shapeArray );
		return shape;
		
	};
#endif	
};

// ---------------------------------------------------------------
//	This class is used by the xml parser to load the physics data.
// ---------------------------------------------------------------
class psConvexVerticesShape : public psShape
{
public:

	//! Data. --------------------------------------------------------------------
	CPoint				m_fTranslation;
	CQuat				m_fRotation;
	int					m_eStride;
	int					m_iNumVertices;
	ntstd::List<float, MC_PHYSICS>	m_obVertexBuffer;

	//! Accessors. ---------------------------------------------------------------
	CPoint		GetPosition( void ) const				{ return m_fTranslation; };	
	void		SetPosition( const CPoint &obPoint )	{ m_fTranslation = obPoint; };

	CQuat		GetRotation( void ) const				{ return m_fRotation; };
	void		SetRotation( const CQuat& obRot )		{ m_fRotation = obRot; };
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	virtual hkShape* BuildShape()
	{
		m_fRotation.Normalise();

		float* vertices = NT_NEW_ARRAY_CHUNK ( MC_PHYSICS ) float[m_iNumVertices*4];
		ntstd::List<float, MC_PHYSICS>::iterator it = m_obVertexBuffer.begin();
		for( int i = 0; i < m_iNumVertices*4; i++ )
		{
			vertices[i] = (*it);
			it++;
		}

		hkConvexVerticesShape* shape;
		hkArray<hkVector4> planeEquations;
		hkGeometry geom;

		hkStridedVertices stridedVerts;
		stridedVerts.m_numVertices = m_iNumVertices;
		stridedVerts.m_striding = m_eStride;
		stridedVerts.m_vertices = vertices;

		hkGeometryUtility::createConvexGeometry( stridedVerts, geom, planeEquations );

		stridedVerts.m_numVertices = geom.m_vertices.getSize();
		stridedVerts.m_striding = sizeof(hkVector4);
		stridedVerts.m_vertices = &(geom.m_vertices[0](0));

		shape = HK_NEW hkConvexVerticesShape(stridedVerts, planeEquations);
		NT_DELETE_ARRAY( vertices );

		shape->setRadius( EXTRA_RADIUS_FOR_CONVEX );
		Physics::PhysicsMaterialTable& matTable = Physics::PhysicsMaterialTable::Get();
#if HAVOK_SDK_VERSION_MAJOR == 4 && HAVOK_SDK_VERSION_MINOR == 0
		shape->setUserData(matTable.GetMaterialFromId(m_iMaterialId));
#else
		shape->setUserData((hkUlong) matTable.GetMaterialFromId( m_iMaterialId));
#endif //HAVOK_SDK_VERSION_MINOR


		if ( TransformWouldBeIdentity( m_fRotation, m_fTranslation ) )
		{
			return shape;
		}
		//hkConvexVerticesShape* shape	= NT_NEW hkConvexVerticesShape( m_fRadius );
		hkTransform trf			= hkTransform(	Physics::MathsTools::CQuatTohkQuaternion( m_fRotation ) , 
												Physics::MathsTools::CPointTohkVector( m_fTranslation ) );
		hkShape * s = HK_NEW hkConvexTransformShape( shape, trf );
		s->setUserData(shape->getUserData());
		shape->removeReference();
		return s;
	};
#endif	
};

// ---------------------------------------------------------------
//	This class is used by the xml parser to load the physics data.
// ---------------------------------------------------------------
//static int s_MemoryInStaticShapes = 0; 
class psMeshShape : public psShape
{
public:

	//! Data. --------------------------------------------------------------------
	CPoint				m_fTranslation;
	CQuat				m_fRotation;
	int					m_eStride;
	int					m_iNumVertices;
	int					m_iIndices;
	ntstd::List<int, MC_PHYSICS>			m_obIndexBuffer;
	ntstd::List<float, MC_PHYSICS>		m_obVertexBuffer;
	ntstd::List<uint32_t, MC_PHYSICS>	m_obMaterialBuffer;

	//! Accessors. ---------------------------------------------------------------
	CPoint		GetPosition( void ) const				{ return m_fTranslation; };	
	void		SetPosition( const CPoint &obPoint )	{ m_fTranslation = obPoint; };

	CQuat		GetRotation( void ) const				{ return m_fRotation; };
	void		SetRotation( const CQuat& obRot )		{ m_fRotation = obRot; };

	psMeshShape(){};
	virtual ~psMeshShape() {};

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	virtual hkShape* BuildShape()
	{
		m_fRotation.Normalise();

		float* vertices = 0;
		hkInt16* indexes = 0;
		hkUint8* materialIndexes = 0;
		Physics::hsMeshMaterial* materialBase =0;
		int numMaterials = 0;

		vertices = NT_NEW_CHUNK( MC_PHYSICS ) float[m_iNumVertices*3];
		//s_MemoryInStaticShapes += sizeof(float) * m_iNumVertices * 3;
		ntstd::List<float, MC_PHYSICS>::iterator it = m_obVertexBuffer.begin();
		for( int i = 0; i < m_iNumVertices; i++ )
		{
			vertices[i*3] = (*it);
			it++;
			vertices[i*3+1] = (*it);
			it++;
			vertices[i*3+2] = (*it);
			it++;
			it++;
		}

		indexes = NT_NEW_CHUNK( MC_PHYSICS ) hkInt16[m_iIndices];
		//s_MemoryInStaticShapes += sizeof(hkInt16) * m_iIndices;
		ntstd::List<int, MC_PHYSICS>::iterator it2 = m_obIndexBuffer.begin();
		for( int i = (m_iIndices-1); i >= 0; i-- )
		{
			indexes[i] = (hkInt16)(*it2);
			it2++;
		}

		if (m_obMaterialBuffer.size() > 0)
		{
			// Materials, get materials IDs and create mapping between mech material table and IDs.
			ntstd::Set<uint32_t> mapping;	
			for( ntstd::List<uint32_t, MC_PHYSICS>::iterator it = m_obMaterialBuffer.begin(); it != m_obMaterialBuffer.end(); ++it)
			{
				++it; // skip triangle number					
				mapping.insert( *it);
			}

			ntAssert(mapping.size() < 256); //havok supports only 256 materials for subpart
			numMaterials = mapping.size() + 1;

			// Create mesh materials. The first one is "dummy" means no material for triangle was set
			materialBase = NT_NEW_CHUNK( MC_PHYSICS ) Physics::hsMeshMaterial[mapping.size() + 1];
			//s_MemoryInStaticShapes += sizeof( Physics::hsMeshMaterial) * (mapping.size() + 1);
			Physics::PhysicsMaterialTable& matTable = Physics::PhysicsMaterialTable::Get();

			ntstd::Set<uint32_t>::iterator it = mapping.begin();
			for(unsigned int i = 0 ; i < mapping.size(); i++, ++it)
			{
				materialBase[i+1].SetMaterial(matTable.GetMaterialFromId(*it));
			}

			// Allocate memory for material indexes
			materialIndexes = NT_NEW_CHUNK( MC_PHYSICS ) hkUint8[m_iIndices / 3]; // number of triangles is m_iIndices / 3
			//s_MemoryInStaticShapes += sizeof( hkUint8) * (m_iIndices / 3);
			memset(materialIndexes, 0x0, m_iIndices / 3 * sizeof(*materialIndexes));

			for( ntstd::List<uint32_t, MC_PHYSICS>::iterator it = m_obMaterialBuffer.begin(); it != m_obMaterialBuffer.end(); ++it)
			{
				uint32_t triangleId = *it;
				++it; // skip triangle number
				materialIndexes[triangleId] = (hkUint8) distance(mapping.begin(),mapping.find(*it)) + 1;
			}
		}
		else
			numMaterials = 0; 


		Physics::hsMeshShape* shape = HK_NEW Physics::hsMeshShape();
		shape->setRadius( 0.0f ); 

		{
			hkMeshShape::Subpart part;

			//m_obVertexBuffer._Myhead
			//ntstd::List<float>::iterator it = m_obVertexBuffer.begin();
			part.m_vertexBase = vertices;
			part.m_vertexStriding = sizeof(float) * 3;
			part.m_numVertices = m_iNumVertices * 3;

			part.m_indexBase = indexes;
			part.m_indexStriding = sizeof(hkInt16) * 3;
			part.m_numTriangles = m_iIndices / 3;
			part.m_stridingType = hkMeshShape::INDICES_INT16;
			
			if (numMaterials > 0)
			{
				part.m_materialIndexStridingType =  hkMeshShape::MATERIAL_INDICES_INT8;
				part.m_materialIndexStriding = sizeof(hkUint8);
				part.m_materialIndexBase = materialIndexes;

				part.m_materialStriding = sizeof(Physics::hsMeshMaterial);
				part.m_materialBase = materialBase;

				part.m_numMaterials = numMaterials;
			}
			else
			{				
				part.m_materialIndexBase = HK_NULL;
				part.m_materialBase = HK_NULL;
				part.m_numMaterials = 0;
			}

			shape->addSubpart( part );
		}

		hkMoppFitToleranceRequirements obMft;
		hkMoppCode* pobMoppCode = hkMoppUtility::buildCode( shape, obMft );

		hkShape* ss = HK_NEW hkMoppBvTreeShape( shape, pobMoppCode );
		shape->removeReference();
		ss->setUserData(INVALID_MATERIAL);
		//NT_DELETE_ARRAY( vertices );

		// Don't apply a transform is we don't need one.
		if ( m_fTranslation.Length() < 0.001f && ( CVector( m_fRotation ) - CVector( 0.0f, 0.0f, 0.0f, 1.0f ) ).Length() < 0.001f )
		{
			return ss;
		}

		hkTransform trf			= hkTransform(	Physics::MathsTools::CQuatTohkQuaternion( m_fRotation ) , 
												Physics::MathsTools::CPointTohkVector( m_fTranslation ) );
		hkShape * s = HK_NEW hkTransformShape( ss, trf );
		ss->removeReference();
		s->setUserData(INVALID_MATERIAL);		
		return s;
	};

	
	virtual hkShape* BuildStaticShape()
	{
		m_fRotation.Normalise();

		float* vertices = 0;
		hkInt16* indexes = 0;
		hkUint8* materialIndexes = 0;
		Physics::hsMeshMaterial* materialBase =0;
		int numMaterials = 0;
							
		vertices = NT_NEW_CHUNK( MC_PHYSICS ) float[m_iNumVertices*3];
		//s_MemoryInStaticShapes += m_iNumVertices*3 * sizeof(float);

		ntstd::List<float, MC_PHYSICS>::iterator it = m_obVertexBuffer.begin();
		for( int i = 0; i < m_iNumVertices; i++ )
		{
			vertices[i*3] = (*it);
			it++;
			vertices[i*3+1] = (*it);
			it++;
			vertices[i*3+2] = (*it);
			it++;
			it++;
		}

		CMatrix trf( m_fRotation, m_fTranslation );

		for( int i = 0; i < m_iNumVertices; i++ )
		{
			int vert = i*3;
			CVector vertx( vertices[vert], vertices[vert + 1], vertices[vert + 2], 1.0f );
			vertx = vertx * trf;
			vertices[vert] = vertx.X();
			vertices[vert+1] = vertx.Y();
			vertices[vert+2] = vertx.Z();
		}

		indexes = NT_NEW_CHUNK( MC_PHYSICS ) hkInt16[m_iIndices];
		//s_MemoryInStaticShapes += m_iIndices * sizeof(hkInt16);
		ntstd::List<int, MC_PHYSICS>::iterator it2 = m_obIndexBuffer.begin();
		for( int i = (m_iIndices-1); i >= 0; i-- )
		{
			indexes[i] = (hkInt16)(*it2);
			it2++;
		}

		if (m_obMaterialBuffer.size() > 0)
		{
			// Materials, get materials IDs and create mapping between mech material table and IDs.
			ntstd::Set<uint32_t> mapping;	
			for( ntstd::List<uint32_t, MC_PHYSICS>::iterator it = m_obMaterialBuffer.begin(); it != m_obMaterialBuffer.end(); ++it)
			{
				++it; // skip triangle number					
				mapping.insert( *it);
			}

			ntAssert(mapping.size() < 256); //havok supports only 256 materials for subpart
			numMaterials = mapping.size() + 1;

			// Create mesh materials. The first one is "dummy" means no material for triangle was set
			materialBase = NT_NEW_CHUNK( MC_PHYSICS ) Physics::hsMeshMaterial[mapping.size() + 1];
			//s_MemoryInStaticShapes += sizeof( Physics::hsMeshMaterial) * (mapping.size() + 1);
			Physics::PhysicsMaterialTable& matTable = Physics::PhysicsMaterialTable::Get();

			ntstd::Set<uint32_t>::iterator it = mapping.begin();
			for(unsigned int i = 0 ; i < mapping.size(); i++, ++it)
			{
				materialBase[i+1].SetMaterial(matTable.GetMaterialFromId(*it));
			}

			// Allocate memory for material indexes
			materialIndexes = NT_NEW_CHUNK( MC_PHYSICS ) hkUint8[m_iIndices / 3]; // number of triangles is m_iIndices / 3
			//s_MemoryInStaticShapes += sizeof(hkUint8) * (m_iIndices / 3);
			memset(materialIndexes, 0x0, m_iIndices / 3 * sizeof(*materialIndexes));

			for( ntstd::List<uint32_t, MC_PHYSICS>::iterator it = m_obMaterialBuffer.begin(); it != m_obMaterialBuffer.end(); ++it)
			{
				uint32_t triangleId = *it;
				++it; // skip triangle number
				materialIndexes[triangleId] = (hkUint8) distance(mapping.begin(),mapping.find(*it)) + 1;
			}
		}
		else
			numMaterials = 0; 


		Physics::hsMeshShape* shape = HK_NEW Physics::hsMeshShape();
		shape->setRadius( 0.0f ); 

		{
			hkMeshShape::Subpart part;

			//m_obVertexBuffer._Myhead
			//ntstd::List<float>::iterator it = m_obVertexBuffer.begin();
			part.m_vertexBase = vertices;
			part.m_vertexStriding = sizeof(float) * 3;
			part.m_numVertices = m_iNumVertices * 3;

			part.m_indexBase = indexes;
			part.m_indexStriding = sizeof(hkInt16) * 3;
			part.m_numTriangles = m_iIndices / 3;
			part.m_stridingType = hkMeshShape::INDICES_INT16;

			if (numMaterials > 0)
			{
				part.m_materialIndexStridingType =  hkMeshShape::MATERIAL_INDICES_INT8;
				part.m_materialIndexStriding = sizeof(hkUint8);
				part.m_materialIndexBase = materialIndexes;

				part.m_materialStriding = sizeof(Physics::hsMeshMaterial);
				part.m_materialBase = materialBase;

				part.m_numMaterials = numMaterials;
			}
			else
			{				
				part.m_materialIndexBase = HK_NULL;
				part.m_materialBase = HK_NULL;
				part.m_numMaterials = 0;
			}

			shape->addSubpart( part );
		}	

		//ntPrintf("s_MemoryInStaticShapes %d\n", s_MemoryInStaticShapes);
					
		hkMoppFitToleranceRequirements obMft;
		hkMoppCode* pobMoppCode = hkMoppUtility::buildCode( shape, obMft );

		hkShape* s = HK_NEW hkMoppBvTreeShape( shape, pobMoppCode );
		shape->removeReference();
		s->setUserData(INVALID_MATERIAL);

		return s;	
		
		//shape->setUserData(INVALID_MATERIAL);
		//return shape;
	};
#endif		
};

// ---------------------------------------------------------------
//	This class is used by the xml parser to load the physics data.
// ---------------------------------------------------------------
class psRigidBody
{
public:

	hkRigidBody*			m_associatedRigid;

	psRigidBody()
	{
		m_associatedRigid = 0;
	}

	//! Data. --------------------------------------------------------------------
	Physics::BodyCInfo	m_info;
	
	psShape*			m_shape;
	unsigned int		m_uiTransformHash;

	// MOTION_DYNAMIC  = 2 MOTION_KEYFRAMED  = 1 MOTION_FIXED = 0
	int					m_eMotionType;
	int					m_eQualityType;

	CPoint				m_fTranslation;
	CQuat				m_fRotation;
	CPoint				m_fMassCenterOffset; 

	float				m_fMaxLinearVelocity;
	float				m_fMaxAngularVelocity;

	// Unused rigid body properties
	//ntstd::List<float>	m_obInertia;
	//bool				m_bInactive;	
	//bool				m_bDisabled;
	//bool				m_bBoundingVolume;
	//int					m_iNumShapes;
	//int					m_iCollisionFilter_I_Am;
	//int					m_iCollisionFilter_I_Collide_With;
	//ntstd::String			m_pcMaterialName;

	//! Accessors. ---------------------------------------------------------------

	CPoint		GetPosition( void ) const				{ return m_fTranslation; };	
	void		SetPosition( const CPoint &obPoint )	{ m_fTranslation = obPoint; };

	CQuat		GetRotation( void ) const				{ return m_fRotation; };
	void		SetRotation( const CQuat& obRot )		{ m_fRotation = obRot; };

	CPoint		GetMassCenterOffset( void ) const				{ return m_fMassCenterOffset; };
	void		SetMassCenterOffset( const CPoint &obMassCenter )	{ m_fMassCenterOffset = obMassCenter; };

	Physics::BodyCInfo GetBodyInfo(const char* name, CHierarchy* hierarchy, hkRigidBodyCinfo * info = NULL);
	Physics::BodyCInfo GetStaticBodyInfo(const char* name, CHierarchy* hierarchy);
};


class psConstraint
{
public:
	psConstraint() {
		m_bUseRagdollHierarchy = false;
	};
	virtual ~psConstraint() {};
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	virtual hkConstraintInstance * BuildConstraint( CEntity* pentity ) = 0;
	
	hkConstraintInstance* m_pobHavokConstraint;
#endif
	bool m_bUseRagdollHierarchy;
};

class psConstraint_Hinge : public psConstraint
{
public:
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	//! Data. --------------------------------------------------------------------
	psRigidBody*	m_BodyA;
	psRigidBody*	m_BodyB;
	CPoint			m_fPivotA;
	CPoint			m_fPivotB;
	CVector			m_vAxisA;
	CVector			m_vAxisB;
	CVector			m_vPerpAxisA;
	CVector			m_vPerpAxisB;
	bool			m_bIsLimited;
	bool			m_bBreakable;
	float			m_fLinearBreakingStrength;
	float			m_FRange;
	float			m_fMaxLimitAngleA;
	//! Accessors. ---------------------------------------------------------------
	CPoint		GetPivotA( void ) const				{ return m_fPivotA; };	
	void		SetPivotA( const CPoint &obPoint )	{ m_fPivotA = obPoint; };
	CPoint		GetPivotB( void ) const				{ return m_fPivotB; };	
	void		SetPivotB( const CPoint &obPoint )	{ m_fPivotB = obPoint; };
	CVector		GetAxisA( void ) const				{ return m_vAxisA; };	
	void		SetAxisA( const CVector &obPoint )	{ m_vAxisA = obPoint; };
	CVector		GetAxisB( void ) const				{ return m_vAxisB; };	
	void		SetAxisB( const CVector &obPoint )	{ m_vAxisB = obPoint; };
	CVector		GetPerpAxisA( void ) const				{ return m_vPerpAxisA; };	
	void		SetPerpAxisA( const CVector &obPoint )	{ m_vPerpAxisA = obPoint; };
	CVector		GetPerpAxisB( void ) const				{ return m_vPerpAxisB; };	
	void		SetPerpAxisB( const CVector &obPoint )	{ m_vPerpAxisB = obPoint; };

	virtual hkConstraintInstance* BuildConstraint( CEntity* pentity );
#endif
};

class psConstraint_P2P : public psConstraint
{
public:
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	//! Data. --------------------------------------------------------------------
	psRigidBody*	m_BodyA;
	psRigidBody*	m_BodyB;
	CPoint			m_fPivotA;
	CPoint			m_fPivotB;
	bool			m_bBreakable;
	float			m_fLinearBreakingStrength;
	int				m_iType;
	//! Accessors. ---------------------------------------------------------------
	CPoint		GetPivotA( void ) const				{ return m_fPivotA; };	
	void		SetPivotA( const CPoint &obPoint )	{ m_fPivotA = obPoint; };
	CPoint		GetPivotB( void ) const				{ return m_fPivotB; };	
	void		SetPivotB( const CPoint &obPoint )	{ m_fPivotB = obPoint; };

	virtual hkConstraintInstance* BuildConstraint( CEntity* pentity )
	{
		hkConstraintData* data;
		hkBallAndSocketConstraintData* data2 = HK_NEW hkBallAndSocketConstraintData;

#ifdef CONSTRAINT_CLUMP_SPACE
	
		/*data2->setInBodySpace (	Physics::MathsTools::CPointTohkVector(m_fPivotA),  
									Physics::MathsTools::CPointTohkVector(m_fPivotB));  */
		
		CPoint pivot = m_fPivotA + m_fPivotB;
		pivot *= 0.5f;

		CMatrix m2;
		m2.SetFromQuat(		pentity->GetHierarchy()->GetBindPoseJointRotation( 0 ) );
		m2.SetTranslation(	pentity->GetHierarchy()->GetBindPoseJointTranslation( 0 ) );
		m2 = m2.GetFullInverse();

		pivot = pivot * m2;


		CMatrix m = pentity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
		pivot = pivot * m;
		//pivot -= pentity->GetHierarchy()->GetRootTransform()->GetWorldMatrix().GetTranslation();


		hkTransform transformOutA;
		m_BodyA->m_associatedRigid->approxTransformAt( Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutA ); 
		hkTransform transformOutB;
		m_BodyB->m_associatedRigid->approxTransformAt( Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutB ); 
		data2->setInWorldSpace( transformOutA, 
								transformOutB, 
								Physics::MathsTools::CPointTohkVector(pivot) );



		/*if( m_bBreakable )
		{
			hkBreakableConstraintData* breaker = NT_NEW hkBreakableConstraintData( data, Physics::CPhysicsWorld::Get().GetHavokWorldP() );
			breaker->setThreshold( m_fLinearBreakingStrength );
			breaker->setRemoveWhenBroken(true);
			data = breaker;

		}*/
#else
		data2->setInBodySpace(Physics::MathsTools::CPointTohkVector(m_fPivotA), 
			Physics::MathsTools::CPointTohkVector(m_fPivotB));

#endif
		data = data2;
		/* What is it for?
		if( m_BodyA->m_associatedRigid->hasProperty( 1000 ) == false )
		{
			if( m_BodyA->m_associatedRigid->getMass() != 0.0f )
			{
				hkMatrix3 m;
				m_BodyA->m_associatedRigid->getInertiaLocal(m);
				m.mul( 10.0f );
				m_BodyA->m_associatedRigid->setInertiaLocal( m );
				hkPropertyValue val;
				val.setInt( 1 );
				m_BodyA->m_associatedRigid->addProperty( 1000, val );
			}
		}

		if( m_BodyB->m_associatedRigid->hasProperty( 1000 ) == false )
		{
			if( m_BodyB->m_associatedRigid->getMass() != 0.0f  )
			{
				hkMatrix3 m;
				m_BodyB->m_associatedRigid->getInertiaLocal(m);
				m.mul( 10.0f );
				m_BodyB->m_associatedRigid->setInertiaLocal( m );
				hkPropertyValue val;
				val.setInt( 1 );
				m_BodyB->m_associatedRigid->addProperty( 1000, val );
			}
		}*/
		
		m_pobHavokConstraint = HK_NEW hkConstraintInstance( m_BodyA->m_associatedRigid,  m_BodyB->m_associatedRigid, data );
		return m_pobHavokConstraint;
	}
#endif
};


class psConstraint_Spring : public psConstraint
{
public:
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	//! Data. --------------------------------------------------------------------
	psRigidBody*	m_BodyA;
	psRigidBody*	m_BodyB;
	CPoint			m_fPivotA;
	CPoint			m_fPivotB;
	
	float			m_fRestLength;
	float			m_fStiffness;
	float			m_fDamping;
	bool			m_bActOnCompression;
	bool			m_bActOnExtension;
	//! Accessors. ---------------------------------------------------------------
	CPoint		GetPivotA( void ) const				{ return m_fPivotA; };	
	void		SetPivotA( const CPoint &obPoint )	{ m_fPivotA = obPoint; };
	CPoint		GetPivotB( void ) const				{ return m_fPivotB; };	
	void		SetPivotB( const CPoint &obPoint )	{ m_fPivotB = obPoint; };

	virtual hkConstraintInstance* BuildConstraint( CEntity* pentity )
	{
		hkSpringAction * action = HK_NEW hkSpringAction( m_BodyA->m_associatedRigid,  m_BodyB->m_associatedRigid );

		CMatrix m2;
		m2.SetFromQuat(		pentity->GetHierarchy()->GetBindPoseJointRotation( 0 ) );
		m2.SetTranslation(	pentity->GetHierarchy()->GetBindPoseJointTranslation( 0 ) );
		m2 = m2.GetFullInverse();

		m_fPivotA = m_fPivotA * m2;
		m_fPivotB = m_fPivotB * m2;


		CMatrix m = pentity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
		m_fPivotA = m_fPivotA * m;

		m_fPivotB = m_fPivotB * m;

		action->setPositionsInWorldSpace( Physics::MathsTools::CPointTohkVector(m_fPivotA), Physics::MathsTools::CPointTohkVector(m_fPivotB) );
		action->setDamping( m_fDamping );
		action->setOnCompression( m_bActOnCompression );
		action->setOnExtension( m_bActOnExtension );
		action->setRestLength ( m_fRestLength );
		action->setStrength( m_fStiffness );
		Physics::CPhysicsWorld::Get().AddAction( action );

		action = HK_NEW hkSpringAction( m_BodyB->m_associatedRigid,  m_BodyA->m_associatedRigid );
		action->setPositionsInWorldSpace( Physics::MathsTools::CPointTohkVector(m_fPivotB), Physics::MathsTools::CPointTohkVector(m_fPivotA) );
		action->setDamping( m_fDamping );
		action->setOnCompression( m_bActOnCompression );
		action->setOnExtension( m_bActOnExtension );
		action->setRestLength ( m_fRestLength );
		action->setStrength( m_fStiffness );
		Physics::CPhysicsWorld::Get().AddAction( action );

		m_pobHavokConstraint = 0;
		return m_pobHavokConstraint;
	}
#endif
};

class psConstraint_StiffSpring : public psConstraint
{
public:
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	//! Data. --------------------------------------------------------------------
	psRigidBody*	m_BodyA;
	psRigidBody*	m_BodyB;
	CPoint			m_fPivotA;
	CPoint			m_fPivotB;
	bool			m_bBreakable;
	float			m_fLinearBreakingStrength;
	float			m_fRestLength;
	//! Accessors. ---------------------------------------------------------------
	CPoint		GetPivotA( void ) const				{ return m_fPivotA; };	
	void		SetPivotA( const CPoint &obPoint )	{ m_fPivotA = obPoint; };
	CPoint		GetPivotB( void ) const				{ return m_fPivotB; };	
	void		SetPivotB( const CPoint &obPoint )	{ m_fPivotB = obPoint; };

	virtual hkConstraintInstance* BuildConstraint( CEntity* pentity )
	{
		hkConstraintData* data;
		hkStiffSpringConstraintData* data2 = HK_NEW hkStiffSpringConstraintData;

#ifdef CONSTRAINT_CLUMP_SPACE
		CMatrix m2;
		m2.SetFromQuat(		pentity->GetHierarchy()->GetBindPoseJointRotation( 0 ) );
		m2.SetTranslation(	pentity->GetHierarchy()->GetBindPoseJointTranslation( 0 ) );
		m2 = m2.GetFullInverse();

		m_fPivotA = m_fPivotA * m2;
		m_fPivotB = m_fPivotB * m2;

		CMatrix m = pentity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
		m_fPivotA = m_fPivotA * m;
		m_fPivotB = m_fPivotB * m;

		hkTransform transformOutA;
		m_BodyA->m_associatedRigid->approxTransformAt( Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutA ); 
		hkTransform transformOutB;
		m_BodyB->m_associatedRigid->approxTransformAt( Physics::CPhysicsWorld::Get().GetFrameTime(), transformOutB ); 

		data2->setInWorldSpace( transformOutA, 
								transformOutB, 
								Physics::MathsTools::CPointTohkVector(m_fPivotA),
								Physics::MathsTools::CPointTohkVector(m_fPivotB));
#else
#if HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1
			data2->setInBodySpace( Physics::MathsTools::CPointTohkVector(m_fPivotA),
							  Physics::MathsTools::CPointTohkVector(m_fPivotB),
							  m_fRestLength); 
#else

		data2->setInBodySpace(hkTransform(), 
							  hkTransform(),
							  Physics::MathsTools::CPointTohkVector(m_fPivotA),
							  Physics::MathsTools::CPointTohkVector(m_fPivotB)); // transform are only used to determine spring lenght but we know it      
#endif
#endif
		data2->setSpringLength( m_fRestLength );
		data = data2;

		if( m_bBreakable )
		{
			hkBreakableConstraintData* breaker = HK_NEW hkBreakableConstraintData( data, Physics::CPhysicsWorld::Get().GetHavokWorldP() );
			breaker->setThreshold( m_fLinearBreakingStrength );
			breaker->setRemoveWhenBroken(true);
			data = breaker;

		}
		
		m_pobHavokConstraint = HK_NEW hkConstraintInstance( m_BodyA->m_associatedRigid,  m_BodyB->m_associatedRigid, data );
		return m_pobHavokConstraint;
	}
#endif
};

class psConstraint_Ragdoll : public psConstraint
{
public:
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	//! Data. --------------------------------------------------------------------
	psRigidBody*	m_BodyA;
	psRigidBody*	m_BodyB;
	CPoint			m_fPivotA;
	CPoint			m_fPivotB;
	
	CVector			m_vTwistAxisA;
	CVector			m_vTwistAxisB;
	CVector			m_vPlaneAxisA;
	CVector			m_vPlaneAxisB;

	bool			m_bBreakable;
	float			m_fLinearBreakingStrength;

	float			m_fTwistMin;
	float			m_fTwistMax;
	float			m_fConeMin;
	float			m_fConeMax;
	float			m_fPlaneMin;
	float			m_fPlaneMax;
	float			m_fFriction;

	//! Accessors. ---------------------------------------------------------------
	CPoint		GetPivotA( void ) const				{ return m_fPivotA; };	
	void		SetPivotA( const CPoint &obPoint )	{ m_fPivotA = obPoint; };
	CPoint		GetPivotB( void ) const				{ return m_fPivotB; };	
	void		SetPivotB( const CPoint &obPoint )	{ m_fPivotB = obPoint; };
	
	CVector		GetTwistA( void ) const				{ return m_vTwistAxisA; };	
	void		SetTwistA( const CVector &obPoint )	{ m_vTwistAxisA = obPoint; };
	CVector		GetTwistB( void ) const				{ return m_vTwistAxisB; };	
	void		SetTwistB( const CVector &obPoint )	{ m_vTwistAxisB = obPoint; };
	
	CVector		GetPlaneAxisA( void ) const				{ return m_vPlaneAxisA; };	
	void		SetPlaneAxisA( const CVector &obPoint )	{ m_vPlaneAxisA = obPoint; };
	CVector		GetPlaneAxisB( void ) const				{ return m_vPlaneAxisB; };	
	void		SetPlaneAxisB( const CVector &obPoint )	{ m_vPlaneAxisB = obPoint; };

	virtual hkConstraintInstance* BuildConstraint( CEntity* pentity );
#endif
};

#endif // _DYNAMICS_XML_INTERFACES_INC
