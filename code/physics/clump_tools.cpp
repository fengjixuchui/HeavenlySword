//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/clump_tools.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.06
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "physics/havokincludes.h"

#include "clump_tools.h"
#include "maths_tools.h"
#include "dynamicsallocator.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "anim/hierarchy.h"
#include "core/exportstruct_clump.h"
#include "gfx/clump.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkcollide/shape/mopp/hkMoppBvTreeShape.h>
#include <hkcollide/shape/convextransform/hkConvexTransformShape.h>
#include <hkcollide/shape/convexpiecemesh/hkConvexPieceMeshShape.h>
#include <hkinternal/collide/convexpiecemesh/hkConvexPieceStreamData.h>
#include <hkinternal/collide/convexpiecemesh/hkConvexPieceMeshBuilder.h>

#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkutilities/collide/hkShapeShrinker.h>
#include <hkcollide/shape/convex/hkConvexShape.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>
#include <hkutilities/inertia/hkinertiatensorcomputer.h>
#include <hkcollide/shape/mopp/hkMoppUtility.h>
#include <hkcollide\shape\collection\hkShapeCollection.h>
#include <hkcollide/shape/mesh/hkMeshShape.h>
#include <hkcollide/shape/list/hkListShape.h>
#endif

#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

#include "rigidbody.h"
#include "animatedlg.h"
#include "staticlg.h"
#include "singlerigidlg.h"
#include "spearlg.h"
#include "compoundlg.h"
#include "xmlinterfaces.h"

#define LOAD_FROM_PS_XML

#define THIN_BOX_INERTIA_MAX_FRAC 0.3f

namespace Physics {
	ntstd::String  ClumpTools::AlterFilename( const char * const filename )
	{	
		// replace appendix ... 	
		ntstd::String str( Util::NoLastExtension(filename));//, 0, strlen( filename ) - appendixLength );
#ifdef BINARY_PHYSICS_LOADER 
		str += ntstd::String( ".ps\0" );
#else
		str += ntstd::String( ".ps.xml\0" );
#endif

		// Make the filename all lower-case and replace / with \.
		ntstd::transform( str.begin(), str.end(), str.begin(), &ntstd::Tolower );
		ntstd::transform( str.begin(), str.end(), str.begin(), &ntstd::ConvertSlash );		

		return str;
	}

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	void ClumpTools::ComputeCentreOfMassAndInertiaTensor (hkShape* pobShape, hkRigidBodyCinfo& obInfo)
	{
		// This function sets the centre of mass to the centre of the object based on its AABB.
		// It then sets the inertia tensor to ensure the mass is properly distributed.

		// Calculate the centre of mass
		hkAabb obAABB;
		pobShape->getAabb(hkTransform::getIdentity(),0.0f,obAABB);
		obInfo.m_centerOfMass(0) += (obAABB.m_min(0) + obAABB.m_max(0)) * 0.5f;
		obInfo.m_centerOfMass(1) += (obAABB.m_min(1) + obAABB.m_max(1)) * 0.5f;
		obInfo.m_centerOfMass(2) += (obAABB.m_min(2) + obAABB.m_max(2)) * 0.5f;

		// Calculate inertia tensor (weight distribution)
		hkVector4 obHalfExtents;
		obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
		obHalfExtents.mul4(0.5f);
	
		hkMassProperties obMassProperties;
		obMassProperties.m_centerOfMass = obInfo.m_centerOfMass;
		obMassProperties.m_mass = obInfo.m_mass;
		
		const hkShape* pobChildShape = 0;
		if (pobShape->getType() == HK_SHAPE_TRANSFORM)
		{
			hkTransformShape* pobConvex = (hkTransformShape*)pobShape;
			pobChildShape = pobConvex->getChildShape();
		} 
		else if (pobShape->getType() == HK_SHAPE_CONVEX_TRANSFORM)
		{
			hkConvexTransformShape* pobConvex = (hkConvexTransformShape*)pobShape;
			pobChildShape = pobConvex->getChildShape();
		}

		if (pobShape->getType() == HK_SHAPE_CAPSULE)
		{
			hkCapsuleShape* pobCapsule = (hkCapsuleShape*)pobShape;
			hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobCapsule->getVertex(0), pobCapsule->getVertex(1), pobCapsule->getRadius(), obInfo.m_mass, obMassProperties);
		}
		else if (pobChildShape && pobChildShape->getType() == HK_SHAPE_CAPSULE)
		{
			hkCapsuleShape* pobCapsule = (hkCapsuleShape*)pobChildShape;
			hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobCapsule->getVertex(0), pobCapsule->getVertex(1), pobCapsule->getRadius(), obInfo.m_mass, obMassProperties);
		}
		else
		{
			// fight instability for thin_boxes --> fake box shape sizes
			float maxSize = max(obHalfExtents(0), max(obHalfExtents(1), obHalfExtents(2)));
			float invMaxSize = 1.0f / maxSize;

			for(int i = 0; i < 3; i++)
			{
				if (obHalfExtents(i) * invMaxSize < THIN_BOX_INERTIA_MAX_FRAC)
					obHalfExtents(i) = maxSize * THIN_BOX_INERTIA_MAX_FRAC;
			}

			hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,obInfo.m_mass,obMassProperties);
		}

		obInfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;

	}

	void ClumpTools::ComputeCentreOfMassAndInertiaTensorTransformed (hkShape* pobShape, hkRigidBodyCinfo& obInfo, hkTransform& obToto)
	{
		// This function sets the centre of mass to the centre of the object based on its AABB.
		// It then sets the inertia tensor to ensure the mass is properly distributed.

		// Calculate the centre of mass
		hkAabb obAABB;
		pobShape->getAabb(obToto,0.0f,obAABB);
		obInfo.m_centerOfMass(0) +=(obAABB.m_min(0) + obAABB.m_max(0)) * 0.5f;
		obInfo.m_centerOfMass(1) +=(obAABB.m_min(1) + obAABB.m_max(1)) * 0.5f;
		obInfo.m_centerOfMass(2) +=(obAABB.m_min(2) + obAABB.m_max(2)) * 0.5f;

		// Calculate inertia tensor (weight distribution)
		hkVector4 obHalfExtents;
		obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
		obHalfExtents.mul4(0.5f);

		hkMassProperties obMassProperties;
		obMassProperties.m_centerOfMass = obInfo.m_centerOfMass;
		obMassProperties.m_mass = obInfo.m_mass;
		hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,obInfo.m_mass,obMassProperties);
		obInfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;

	}
#endif

	AnimatedLG* ClumpTools::ConstructAnimatedLGFromClump( CEntity* p_entity )
	{
		ntAssert_p(false,("This loader should not be used. How did we get here? PeterFe"));
		AnimatedLG* lg = NT_NEW AnimatedLG( p_entity->GetName(), p_entity );
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		CHierarchy* pobHierarchy = p_entity->GetHierarchy();

		const CClumpHeader* pobThisClump = pobHierarchy->GetClumpHeader();

		

		// Build a list of shapes per transforms
		ListShape* pobListShape = NT_NEW ListShape[ pobThisClump->m_iNumberOfTransforms ];

		ntstd::String filenameString = AlterFilename( ntStr::GetString(p_entity->GetClumpString()) );
		CHashedString psXMLFile = CHashedString(filenameString);

		if( !File::Exists( ntStr::GetString(filenameString) ) )
		{
			NT_DELETE( lg );
			NT_DELETE_ARRAY( pobListShape );
			return 0;
		}

		DataObject* obj = 0;
		
		obj = ObjectDatabase::Get().GetDataObjectFromName( psXMLFile );

		if( ( obj == 0 )  )
		{
			// Tell people what we're up to
			ntPrintf( "XML loading \'%s\'\n", ntStr::GetString(filenameString) );

			// Open the XML file in memory
			FileBuffer obFile( ntStr::GetString(filenameString), true );

			if ( !ObjectDatabase::Get().LoadDataObject( &obFile, filenameString ) )
			{
				ntError_p( false, ( "Failed to parse XML file '%s'", ntStr::GetString(filenameString) ) );
			}

			obj = ObjectDatabase::Get().GetDataObjectFromName( psXMLFile );
		}

		ObjectContainer* current = (ObjectContainer*) obj->GetBasePtr();
		
		for (	ObjectContainer::ObjectList::iterator obIt = current->m_ContainedObjects.begin(); 
				obIt != current->m_ContainedObjects.end(); 
				obIt++ )
		{
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psRigidBody") )
			{
				psRigidBody* pobBody = (psRigidBody*)((*obIt)->GetBasePtr());
				if( pobBody->m_shape != 0 )
				{
					BodyCInfo info = pobBody->GetBodyInfo( ntStr::GetString((*obIt)->GetName()), p_entity->GetHierarchy());
					// motion type must be dynamic otherwise there is an assert in AnimatedLG::AddRigidBody
					info.m_rigidBodyCinfo.m_motionType = hkMotion::MOTION_DYNAMIC;
					info.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
					if (info.m_rigidBodyCinfo.m_mass <= 0)
						info.m_rigidBodyCinfo.m_mass = 1;

					// ... animated rigid body must be large interactable... otherwise it will not collide with player

					Physics::EntityCollisionFlag obFlag; 
					obFlag.base = info.m_rigidBodyCinfo.m_collisionFilterInfo;
					obFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
					info.m_rigidBodyCinfo.m_collisionFilterInfo = obFlag.base;


					info.m_exceptionFlag.flags.exception_set |= Physics::IGNORE_ENTITY_PTR_BIT; 
					RigidBody* elem = lg->AddRigidBody( &info );					
					pobBody->m_associatedRigid = elem->GetHkRigidBody();
				}
			}

			/// +++++++++++++++++++++++++++++++++++++
			/// OBSOLETE WILL BE REMOVED
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psShapeTrfLink") )
			{
				psShapeTrfLink* pobBody = (psShapeTrfLink*)((*obIt)->GetBasePtr());

				//CMatrix obLocalSpaceOfVolume( obThisVolume.m_obRotation, obThisVolume.m_obTranslation );

				//CQuat obQuatRotation( obLocalSpaceOfVolume );
				//obQuatRotation.Normalise();

				hkShape* pobShape = pobBody->m_shape->BuildShape();//ConstructConvexShapeFromColprim( obThisVolume );

				/*hkVector4		obPosition = Physics::MathsTools::CPointTohkVector( obLocalSpaceOfVolume.GetTranslation() );
				hkQuaternion	obRotation = Physics::MathsTools::CQuatTohkQuaternion( obQuatRotation );

				hkTransform obLocalTransform( obRotation, obPosition );

				hkConvexTransformShape* pobPositionedShape = NT_NEW hkConvexTransformShape( pobShape, obLocalTransform );
				
				*/

				int iTransformIndex=pobBody->m_trfIndex;//obThisVolume.m_iTransform;
				pobListShape[ iTransformIndex ].obShapeList.push_back( pobShape/*pobPositionedShape*/ );				
			}

			/// OBSOLETE WILL BE REMOVED
		    ///---------------------------------------------------------
		}

		/// +++++++++++++++++++++++++++++++++++++
		/// OBSOLETE WILL BE REMOVED

		// Each transform is then associated to it's own body
		for(	int iCount=0; 
				iCount < pobThisClump->m_iNumberOfTransforms; 
				++iCount )
		{
			if( pobListShape[iCount].obShapeList.size() > 0 )
			{
				hkShape* pobAllShapes = 0;
				if (pobListShape[iCount].obShapeList.size() == 1)
				{
					pobAllShapes = *pobListShape[iCount].obShapeList.begin();
				}
				else
				{
					CScopedArray<hkShape*> apobShapeArray( NT_NEW hkShape*[ pobListShape[ iCount ].obShapeList.size() ] );
					int iShapeIndex = 0;
					ntstd::List<hkShape*>::iterator obShapes = pobListShape[ iCount ].obShapeList.begin();

					while( obShapes != pobListShape[ iCount ].obShapeList.end() )
					{
						apobShapeArray[ iShapeIndex ] = (*obShapes);
						++iShapeIndex;
						++obShapes;
					}

					pobAllShapes = CreateListShape( apobShapeArray.Get(), pobListShape[ iCount ].obShapeList.size() );

				}

				pobListShape[ iCount ].obShapeList.clear();

				// Create hkRigidBody and add it to the state

				Transform* pobBoneTransfom		= pobHierarchy->GetTransform( iCount );
				const CMatrix& obWorldMatrix	= pobBoneTransfom->GetWorldMatrix();

				CQuat obWorldRotation( obWorldMatrix );
				obWorldRotation.Normalise();

				hkVector4 obPosition	= Physics::MathsTools::CPointTohkVector( obWorldMatrix.GetTranslation() );
				hkQuaternion obRotation = Physics::MathsTools::CQuatTohkQuaternion( obWorldRotation );

				// ... build the Physics::BodyCInfo ...
				// We ideally need to be able to specify properties of each of the individual rigid bodies, but for time being use default properties

				Physics::BodyCInfo obInfo;
				
				obInfo.m_rigidBodyCinfo.m_position				= obPosition;
				obInfo.m_rigidBodyCinfo.m_rotation				= obRotation;
				obInfo.m_rigidBodyCinfo.m_motionType			= hkMotion::MOTION_DYNAMIC;
				obInfo.m_rigidBodyCinfo.m_qualityType			= HK_COLLIDABLE_QUALITY_MOVING; //HK_COLLIDABLE_QUALITY_KEYFRAMED; - Changed because it was causing problems with collision when the enetity was switched to dynamic motion
				obInfo.m_rigidBodyCinfo.m_restitution			= 0.01f;
				obInfo.m_rigidBodyCinfo.m_friction				= 0.9f;
				obInfo.m_rigidBodyCinfo.m_mass					= 5.0f;
				obInfo.m_rigidBodyCinfo.m_maxLinearVelocity		= 50.0f;
				obInfo.m_rigidBodyCinfo.m_maxAngularVelocity	= 5.0f;
				obInfo.m_rigidBodyCinfo.m_shape					= pobAllShapes;
				Physics::EntityCollisionFlag obFlag; obFlag.base = 0;
				obFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
				obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
										Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
										Physics::RAGDOLL_BIT						|
										Physics::SMALL_INTERACTABLE_BIT				|
										Physics::LARGE_INTERACTABLE_BIT				);
				obInfo.m_rigidBodyCinfo.m_collisionFilterInfo	= (int)obFlag.base;

				obInfo.m_exceptionFlag.flags.exception_set |= Physics::IGNORE_ENTITY_PTR_BIT; 

				// [Mus] - This may need to be updated at some point.
				ComputeCentreOfMassAndInertiaTensor( pobAllShapes, obInfo.m_rigidBodyCinfo );

				obInfo.m_name									= p_entity->GetName();
				obInfo.m_transform								= pobBoneTransfom;

				// ... and add it to the physic system.
				/*Physics::Element* element =*/ lg->AddRigidBody( &obInfo ); 
				//element->Activate();

			}
		}

		NT_DELETE_ARRAY( pobListShape );

		/// OBSOLETE WILL BE REMOVED
		///---------------------------------------------------------
#endif
		lg->Activate();
		return lg;
	}

	StaticLG* ClumpTools::ConstructStaticLGFromClump( CEntity* p_entity )
	{
		ntAssert_p(false,("This loader should not be used. How did we get here? PeterFe"));
		StaticLG* lg = NT_NEW StaticLG( p_entity->GetName(), p_entity );
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		CHierarchy* pobHierarchy = p_entity->GetHierarchy();

		ntstd::String psXMLFile = AlterFilename( ntStr::GetString(p_entity->GetClumpString()) );

		if( !File::Exists( psXMLFile.c_str() ) )
		{
			NT_DELETE( lg );
			return 0;
		}

		DataObject* obj = 0;		
		obj = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(psXMLFile) );

		if( ( obj == 0 ) )
		{
			// Tell people what we're up to
			ntPrintf( "XML loading \'%s\'\n", psXMLFile.c_str() );

			// Open the XML file in memory
			FileBuffer obFile( psXMLFile.c_str(), true );

			if ( !ObjectDatabase::Get().LoadDataObject( &obFile, psXMLFile ) )
			{
				ntError_p( false, ( "Failed to parse XML file '%s'", psXMLFile.c_str() ) );
			}

			obj = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(psXMLFile) );
		} 

		ObjectContainer* current = (ObjectContainer*) obj->GetBasePtr();
		
		for (	ObjectContainer::ObjectList::iterator obIt = current->m_ContainedObjects.begin(); 
				obIt != current->m_ContainedObjects.end(); 
				obIt++ )
		{
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psRigidBody") )
			{
				psRigidBody* pobBody = (psRigidBody*)((*obIt)->GetBasePtr());
				if( pobBody && pobBody->m_shape != 0 )
				{				
					// ... build the Physics::BodyCInfo ...		
					Physics::BodyCInfo obInfo = pobBody->GetStaticBodyInfo(p_entity->GetName().c_str(), p_entity->GetHierarchy());

					// ... static rigid body must be large interactable... otherwise it will not collide with player

					Physics::EntityCollisionFlag obFlag; 
					obFlag.base = obInfo.m_rigidBodyCinfo.m_collisionFilterInfo;
					obFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
					obInfo.m_rigidBodyCinfo.m_collisionFilterInfo = obFlag.base;

					
					// ... move body onto position and orientation ...
					CMatrix obWorldSpaceOfBone = obInfo.m_transform->GetWorldMatrix();

					obInfo.m_rigidBodyCinfo.m_position				= Physics::MathsTools::CPointTohkVector( obWorldSpaceOfBone.GetTranslation() );
					obInfo.m_rigidBodyCinfo.m_rotation				= Physics::MathsTools::CQuatTohkQuaternion( CQuat(obWorldSpaceOfBone) );
									
					obInfo.m_transform								= 0;

					// ... and add it to the physic system.
					Physics::Element* element = lg->AddRigidBody( &obInfo ); 
					element->Activate();
				}
			}

			/// ++++++++++++++++++++++++++++++++++++++++++++++++++++
			/// OBSOLETE WILL BE REMOVED

			if ( 0 == strcmp( (*obIt)->GetClassName(), "psShapeTrfLink") )
			{
				psShapeTrfLink* pobBody = (psShapeTrfLink*)((*obIt)->GetBasePtr());
				
				// ... get the position and orientation ...
				const Transform* pobBoneTransfom	= pobHierarchy->GetTransform( pobBody->m_trfIndex );
				CMatrix obWorldSpaceOfBone			= pobBoneTransfom->GetWorldMatrix();

				// ... build the hkShape ...
				hkShape* pobShape = pobBody->m_shape->BuildStaticShape(); //ConstructArbitraryShapeFromColprim( obThisVolume );

				Physics::EntityCollisionFlag obFlag; obFlag.base = 0;
				obFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
				obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
												Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
												Physics::RAGDOLL_BIT						|
												Physics::SMALL_INTERACTABLE_BIT				|
												Physics::LARGE_INTERACTABLE_BIT				);

			
				// ... build the Physics::BodyCInfo ...
				Physics::BodyCInfo obInfo;
				
				obInfo.m_rigidBodyCinfo.m_position				= Physics::MathsTools::CPointTohkVector( obWorldSpaceOfBone.GetTranslation() );//Physics::MathsTools::CPointTohkVector( obWorldSpaceOfVolume.GetTranslation() );
				obInfo.m_rigidBodyCinfo.m_rotation				= Physics::MathsTools::CQuatTohkQuaternion( CQuat(obWorldSpaceOfBone) );//Physics::MathsTools::CQuatTohkQuaternion( obRotation );
				obInfo.m_rigidBodyCinfo.m_motionType			= hkMotion::MOTION_FIXED;
				obInfo.m_rigidBodyCinfo.m_qualityType			= HK_COLLIDABLE_QUALITY_FIXED;
				obInfo.m_rigidBodyCinfo.m_friction				= 1.0f;
				obInfo.m_rigidBodyCinfo.m_shape					= pobShape;
				obInfo.m_rigidBodyCinfo.m_collisionFilterInfo	= (int)obFlag.base;	

				obInfo.m_name									= p_entity->GetName();
				obInfo.m_transform								= 0;

				// ... and add it to the physic system.
				Physics::Element* element = lg->AddRigidBody( &obInfo ); 
				element->Activate();
			}

			/// OBSOLETE WILL BE REMOVED
		    ///---------------------------------------------------------
		}
#endif
		return lg;
	}

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	SingleRigidLG *ClumpTools::ConstructRigidLGFromClump( CEntity *p_entity, CHierarchy *pobHierarchy, const ntstd::String &name, hkRigidBodyCinfo* pobInfo )
	{
		ntAssert_p(false,("This loader should not be used. How did we get here? PeterFe"));
		SingleRigidLG* lg = NT_NEW SingleRigidLG( name, p_entity );

		ntstd::List<hkShape*> obShapesInRigid;
		ntstd::String psXMLFile = AlterFilename( ntStr::GetString(p_entity->GetClumpString()) );

		if( !File::Exists( psXMLFile.c_str() ) )
		{
			NT_DELETE( lg );
			return 0;
		}

		DataObject* obj = 0;
		
		obj = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(psXMLFile) );

		if( ( obj == 0 ) )
		{
			// Tell people what we're up to
			ntPrintf( "XML loading \'%s\'\n", psXMLFile.c_str() );

			// Open the XML file in memory
			FileBuffer obFile( psXMLFile.c_str(), true );

			if ( !ObjectDatabase::Get().LoadDataObject( &obFile, psXMLFile ) )
			{
				ntError_p( false, ( "Failed to parse XML file '%s'", psXMLFile.c_str() ) );
			}

			obj = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(psXMLFile) );
		} 

		ObjectContainer* current = (ObjectContainer*) obj->GetBasePtr();
		
		for (	ObjectContainer::ObjectList::iterator obIt = current->m_ContainedObjects.begin(); 
				obIt != current->m_ContainedObjects.end(); 
				obIt++ )
		{
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psRigidBody") )
			{
				psRigidBody* pobBody = (psRigidBody*)((*obIt)->GetBasePtr());
				if( pobBody->m_shape != 0 )
				{
					BodyCInfo info = pobBody->GetBodyInfo( ntStr::GetString((*obIt)->GetName()), p_entity->GetHierarchy(), pobInfo );
					RigidBody* elem = lg->AddRigidBody( &info );
					// [Mus[ - Should it be here ?
					elem->Activate();
					pobBody->m_associatedRigid = elem->GetHkRigidBody();
				}
			}

			/// ++++++++++++++++++++++++++++++++++++++++++++++++++++
			/// OBSOLETE WILL BE REMOVED
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psShapeTrfLink") )
			{
				

				psShapeTrfLink* pobBody = (psShapeTrfLink*)((*obIt)->GetBasePtr());
				 
				hkShape* pobShape = pobBody->m_shape->BuildShape();
	
				// Need to see if this is a box, and if it is, if it is thin, and if it is whether we need to restrict the angular velocity so it doesn't go mental
				hkShape* pobTestShape = pobShape;
				if ( pobTestShape->getType() == HK_SHAPE_BOX ||
					pobTestShape->getType() == HK_SHAPE_TRANSFORM )
				{
					psBoxShape* pobHSBoxShape = 0;
					if (pobTestShape->getType() == HK_SHAPE_BOX)
					{
						pobHSBoxShape = (psBoxShape*)pobBody->m_shape;
					}
					else
					{
						hkTransformShape* pobTestTransformShape = (hkTransformShape*)pobTestShape;
						if (pobTestTransformShape->getChildShape()->getType() == HK_SHAPE_BOX)
							pobHSBoxShape = (psBoxShape*)pobBody->m_shape;
					}

					if (pobHSBoxShape)
					{
						// Check extents to see if it's thin, and restrict angular velocity accordingly if so
						float x = pobHSBoxShape->m_obHalfExtent.X();
						float y = pobHSBoxShape->m_obHalfExtent.Y();
						float z = pobHSBoxShape->m_obHalfExtent.Z();

						// If the relative size of extents is over a certain value, we need to treat this box specially
						float fThinThreshold = 3.0f;
						if ((x/y > fThinThreshold || y/x > fThinThreshold) ||
							(x/z > fThinThreshold || z/x > fThinThreshold) ||
							(y/z > fThinThreshold || z/y > fThinThreshold) )
						{
							if (pobInfo->m_maxAngularVelocity > 4.0f)
								pobInfo->m_maxAngularVelocity = 4.0f;
							pobInfo->m_motionType = hkMotion::MOTION_THIN_BOX_INERTIA;
							if (pobInfo->m_angularDamping < 0.05f)
								pobInfo->m_angularDamping = 0.05f;
							pobInfo->m_angularDamping *= 6.0f;
							if (pobInfo->m_angularDamping > 1.0f)
							{
								pobInfo->m_angularDamping = 0.95f;
							}
						}
					}
				}

				obShapesInRigid.push_back( pobShape );
			}
			/// OBSOLETE WILL BE REMOVED
			/// -------------------------------------------------
			
		}

		/// ++++++++++++++++++++++++++++++++++++++++++++++++++++
		/// OBSOLETE WILL BE REMOVED
		if( obShapesInRigid.size() != 0 )
		{
			hkShape* pobAllShapes; 
  
			if (obShapesInRigid.size() == 1)
				pobAllShapes = *obShapesInRigid.begin();
			else
			{   
				CScopedArray<hkShape*> apobShapeArray( NT_NEW hkShape*[obShapesInRigid.size()] );

				int iShapeIndex = 0;
				ntstd::List<hkShape*>::iterator obShapes = obShapesInRigid.begin();
				while(obShapes!=obShapesInRigid.end())
				{
					apobShapeArray[iShapeIndex] = static_cast<hkShape*>(*obShapes);
					++iShapeIndex;
					++obShapes;
				}
				//Create a list shape for the rigid body
				pobAllShapes = CreateListShape( apobShapeArray.Get(), obShapesInRigid.size() );				
			}			

			CMatrix obWorldSpaceOfEntity = pobHierarchy->GetRootTransform()->GetWorldMatrix();

			// Create a single rigid body for the whole shape

			Physics::BodyCInfo obInfo;

			if ( pobInfo==NULL ) // If no rigid body definition has been defined, create a default one
			{
				CQuat obRotation( obWorldSpaceOfEntity );

				obInfo.m_rigidBodyCinfo.m_position		= Physics::MathsTools::CPointTohkVector( obWorldSpaceOfEntity.GetTranslation() );
				obInfo.m_rigidBodyCinfo.m_rotation		= Physics::MathsTools::CQuatTohkQuaternion(obRotation);
				obInfo.m_rigidBodyCinfo.m_motionType	= hkMotion::MOTION_DYNAMIC;
				obInfo.m_rigidBodyCinfo.m_qualityType	= HK_COLLIDABLE_QUALITY_MOVING;
				obInfo.m_rigidBodyCinfo.m_mass			= 1.0f;
			
				hkAabb obAABB;
				pobAllShapes->getAabb( hkTransform::getIdentity(), 0.0f, obAABB);
				obInfo.m_rigidBodyCinfo.m_centerOfMass(0)=(obAABB.m_min(0) + obAABB.m_max(0)) * 0.5f;
				obInfo.m_rigidBodyCinfo.m_centerOfMass(1)=(obAABB.m_min(1) + obAABB.m_max(1)) * 0.5f;
				obInfo.m_rigidBodyCinfo.m_centerOfMass(2)=(obAABB.m_min(2) + obAABB.m_max(2)) * 0.5f;

				hkMassProperties obMassProperties;
				obMassProperties.m_mass = obInfo.m_rigidBodyCinfo.m_mass;

				hkVector4 obHalfExtents; obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
				obHalfExtents.mul4(0.5f);

				hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,obInfo.m_rigidBodyCinfo.m_mass,obMassProperties);
				if (obInfo.m_rigidBodyCinfo.m_motionType == hkMotion::MOTION_THIN_BOX_INERTIA)
				{
					// If we're a thin box, make the inertia tensor BIG to try to combat spinniness
					const float THIN_BOX_INERTIA_TENSOR_MULTIPLIER = 3.0f;
					obMassProperties.m_inertiaTensor.getColumn(0).mul4(THIN_BOX_INERTIA_TENSOR_MULTIPLIER);
					obMassProperties.m_inertiaTensor.getColumn(1).mul4(THIN_BOX_INERTIA_TENSOR_MULTIPLIER);
					obMassProperties.m_inertiaTensor.getColumn(2).mul4(THIN_BOX_INERTIA_TENSOR_MULTIPLIER);
				}
				obInfo.m_rigidBodyCinfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;

				//POSSIBLE CALLBACK ON MAKING A CONTACT SO CAN CHANGE THE FRICTION VALUES

				obInfo.m_rigidBodyCinfo.m_shape = pobAllShapes;

				Physics::EntityCollisionFlag obCollisionFlag;
				obCollisionFlag.base = 0;				
				obCollisionFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;			
				obCollisionFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
					Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
					Physics::RAGDOLL_BIT						|
					Physics::SMALL_INTERACTABLE_BIT				|
					Physics::LARGE_INTERACTABLE_BIT				);

				obInfo.m_rigidBodyCinfo.m_collisionFilterInfo = (int)obCollisionFlag.base;
				
			}
			else
			{
				obInfo.m_rigidBodyCinfo = *pobInfo;

				//obInfo.m_rigidBodyCinfo.m_motionType	= hkMotion::MOTION_DYNAMIC;
				//obInfo.m_rigidBodyCinfo.m_qualityType	= HK_COLLIDABLE_QUALITY_MOVING;

				CQuat obRotation( obWorldSpaceOfEntity );

				obInfo.m_rigidBodyCinfo.m_position = Physics::MathsTools::CPointTohkVector( obWorldSpaceOfEntity.GetTranslation() );
				obInfo.m_rigidBodyCinfo.m_rotation = Physics::MathsTools::CQuatTohkQuaternion( obRotation );

				obInfo.m_rigidBodyCinfo.m_shape = pobAllShapes;

				Physics::EntityCollisionFlag obCollisionFlag;
				obCollisionFlag.base = 0;				
				obCollisionFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;			
				obCollisionFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
					Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
					Physics::RAGDOLL_BIT						|
					Physics::SMALL_INTERACTABLE_BIT				|
					Physics::LARGE_INTERACTABLE_BIT				);
				obInfo.m_rigidBodyCinfo.m_collisionFilterInfo = (int)obCollisionFlag.base;

				hkAabb obAABB;
				pobAllShapes->getAabb( hkTransform::getIdentity(), 0.0f, obAABB);
				obInfo.m_rigidBodyCinfo.m_centerOfMass(0)=(obAABB.m_min(0) + obAABB.m_max(0)) * 0.5f;
				obInfo.m_rigidBodyCinfo.m_centerOfMass(1)=(obAABB.m_min(1) + obAABB.m_max(1)) * 0.5f;
				obInfo.m_rigidBodyCinfo.m_centerOfMass(2)=(obAABB.m_min(2) + obAABB.m_max(2)) * 0.5f;

				hkMassProperties obMassProperties;
				obMassProperties.m_mass = obInfo.m_rigidBodyCinfo.m_mass;

				hkVector4 obHalfExtents; obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
				obHalfExtents.mul4(0.5f);

				hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,obInfo.m_rigidBodyCinfo.m_mass,obMassProperties);
				if (pobInfo->m_motionType == hkMotion::MOTION_THIN_BOX_INERTIA)
				{
					// If we're a thin box, make the inertia tensor BIG to try to combat spinniness
					const float THIN_BOX_INERTIA_TENSOR_MULTIPLIER = 3.0f;
					obMassProperties.m_inertiaTensor.getColumn(0).mul4(THIN_BOX_INERTIA_TENSOR_MULTIPLIER);
					obMassProperties.m_inertiaTensor.getColumn(1).mul4(THIN_BOX_INERTIA_TENSOR_MULTIPLIER);
					obMassProperties.m_inertiaTensor.getColumn(2).mul4(THIN_BOX_INERTIA_TENSOR_MULTIPLIER);
				}
				obInfo.m_rigidBodyCinfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;
			}

			//obInfo.m_rigidBodyCinfo.m_linearDamping = 0.1f;
			//obInfo.m_rigidBodyCinfo.m_angularDamping = 0.7f;
			//obInfo.m_rigidBodyCinfo.m_restitution = 0.1f;


			obInfo.m_name									= p_entity->GetName();
			obInfo.m_transform								= pobHierarchy->GetRootTransform();


			Physics::Element* element = lg->AddRigidBody( &obInfo ); 
			element->Activate();
		}		
		/// OBSOLETE WILL BE REMOVED
		/// --------------------------------------------------------


		return lg;
	}
#endif

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	SingleRigidLG* ClumpTools::ConstructRigidLGFromClump( CEntity* p_entity,  hkRigidBodyCinfo* pobInfo )
	{
		return ConstructRigidLGFromClump( p_entity, p_entity->GetHierarchy(), p_entity->GetName(), pobInfo );
	}

	CompoundLG* ClumpTools::ConstructCompoundLGFromClump( CEntity* p_entity, hkRigidBodyCinfo* pobInfo )
	{
		ntAssert_p(false,("This loader should not be used. How did we get here? PeterFe"));
		CompoundLG* lg = NT_NEW CompoundLG( p_entity->GetName(), p_entity );

		CHierarchy* pobHierarchy = p_entity->GetHierarchy();
		ntAssert( pobHierarchy );

		const CClumpHeader* pobThisClump = p_entity->GetHierarchy()->GetClumpHeader();	

		ListShape obMainShapeList;
		ListShape* pobListShape=NT_NEW ListShape [pobThisClump->m_iNumberOfTransforms];

		ntstd::String psXMLFile = AlterFilename( ntStr::GetString(p_entity->GetClumpString()) );

		if( !File::Exists( psXMLFile.c_str() ) )
		{
			NT_DELETE( lg );
			return 0;
		}

		DataObject* obj = 0;
		obj = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(psXMLFile) );

		if( ( obj == 0 ) )
		{
			// Tell people what we're up to
			ntPrintf( "XML loading \'%s\'\n", psXMLFile.c_str() );

			// Open the XML file in memory
			FileBuffer obFile( psXMLFile.c_str(), true );

			if ( !ObjectDatabase::Get().LoadDataObject( &obFile, psXMLFile ) )
			{
				ntError_p( false, ( "Failed to parse XML file '%s'", psXMLFile.c_str() ) );
			}

			obj = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(psXMLFile) );
		} 
	
		ObjectContainer* current = (ObjectContainer*) obj->GetBasePtr();
		
		for (	ObjectContainer::ObjectList::iterator obIt = current->m_ContainedObjects.begin(); 
				obIt != current->m_ContainedObjects.end(); 
				obIt++ )
		{
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psRigidBody") )
			{
				psRigidBody* pobBody = (psRigidBody*)((*obIt)->GetBasePtr());
				if( pobBody->m_shape != 0 )
				{
					BodyCInfo obInfo = pobBody->GetBodyInfo(ntStr::GetString((*obIt)->GetName()), p_entity->GetHierarchy(), pobInfo);					

					RigidBody* body = RigidBody::ConstructRigidBody( p_entity, &obInfo);
					lg->m_bodiesList.push_back( body );								
				} 
			}

			/// ++++++++++++++++++++++++++++++++++++++++++++++++++++
			/// OBSOLETE WILL BE REMOVED
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psShapeTrfLink") )
			{
				psShapeTrfLink* pobBody = (psShapeTrfLink*)((*obIt)->GetBasePtr());
				
				
				{	// Build our shape for the single rigid body
					//hkShape* pobShape = ConstructShapeFromColprim(obThisVolume);
					hkShape* pobShape = pobBody->m_shape->BuildShape();//ConstructConvexShapeFromColprim(obThisVolume);
				
					//Add the transform shape to the list shape
					obMainShapeList.obShapeList.push_back(pobShape);
				}

				{	// Build a sub body
					
					hkShape* pobShape =  pobBody->m_shape->BuildShape();//ConstructConvexShapeFromColprim(obThisVolume);

					int iTransformIndex=pobBody->m_trfIndex;

					pobListShape[iTransformIndex].obShapeList.push_back(pobShape);
				}
			}
			
			/// OBSOLETE WILL BE REMOVED
			/// ----------------------------------------------

		}

		// ----- Step 2: Create the main body -----


		if (obMainShapeList.obShapeList.size() == 0)
		{
			// We have a set of rigid bodies. Now se need to create the main body, an envelope of those bodies. 			 
			// The main body properties (f.e. friction, restitution) will be average values. The shape of main bodies
			// will be the sum of all shapes. 
			
			CScopedArray<hkShape*> apobShapeArray( NT_NEW hkShape*[lg->m_bodiesList.size()] );
			Physics::BodyCInfo mainInfo;
			mainInfo.m_rigidBodyCinfo.m_mass = 0;
			mainInfo.m_rigidBodyCinfo.m_linearDamping = 0;
			mainInfo.m_rigidBodyCinfo.m_angularDamping = 0;
			mainInfo.m_rigidBodyCinfo.m_friction = 0;
			mainInfo.m_rigidBodyCinfo.m_restitution = 0;

			int iShapeIndex = 0; 
			for(ntstd::List<RigidBody*>::iterator it = lg->m_bodiesList.begin(); it != lg->m_bodiesList.end(); ++it, ++iShapeIndex)
			{
				hkRigidBody * body = (*it)->GetHkRigidBody();

				hkShape * pobShape = const_cast<hkShape *>(body->getCollidable()->getShape());
				pobShape->addReference();

				/// add trasform shape to shift the shape to correct position
				const CMatrix& locTransform = (*it)->GetTransform()->GetLocalMatrix();
				hkTransform trf = Physics::MathsTools::CMatrixTohkTransform(locTransform );																
				pobShape = CreateTransformShape( pobShape, trf );

				apobShapeArray[iShapeIndex] = pobShape;

				// sum masses
				mainInfo.m_rigidBodyCinfo.m_mass += body->getMass();
				hkVector4 addMassCenter;
				addMassCenter.setTransformedPos(trf, body->getCenterOfMassInWorld());
				mainInfo.m_rigidBodyCinfo.m_centerOfMass.addMul4(body->getMass(), addMassCenter);

				mainInfo.m_rigidBodyCinfo.m_linearDamping += body->getLinearDamping();
				mainInfo.m_rigidBodyCinfo.m_angularDamping += body->getAngularDamping();
				mainInfo.m_rigidBodyCinfo.m_friction += body->getMaterial().getFriction();
				mainInfo.m_rigidBodyCinfo.m_restitution += body->getMaterial().getRestitution();
				if (mainInfo.m_rigidBodyCinfo.m_maxLinearVelocity <  body->getMaxLinearVelocity())
					mainInfo.m_rigidBodyCinfo.m_maxLinearVelocity = body->getMaxLinearVelocity();

				if (mainInfo.m_rigidBodyCinfo.m_maxAngularVelocity <  body->getMaxAngularVelocity())
					mainInfo.m_rigidBodyCinfo.m_maxAngularVelocity = body->getMaxAngularVelocity();		
			}

			// mass center is average of mass center weight by mass. 
			mainInfo.m_rigidBodyCinfo.m_centerOfMass.mul4(1.0f / mainInfo.m_rigidBodyCinfo.m_mass);

			int n = lg->m_bodiesList.size();

			mainInfo.m_rigidBodyCinfo.m_linearDamping /= n;
			mainInfo.m_rigidBodyCinfo.m_angularDamping /= n;
			mainInfo.m_rigidBodyCinfo.m_friction /= n;
			mainInfo.m_rigidBodyCinfo.m_restitution /= n; 
			mainInfo.m_rigidBodyCinfo.m_shape = CreateListShape(apobShapeArray.Get(),n);

			Physics::EntityCollisionFlag obFlag; obFlag.base = 0;
			obFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
			obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
					Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
					Physics::RAGDOLL_BIT						|
					Physics::SMALL_INTERACTABLE_BIT				|
					Physics::LARGE_INTERACTABLE_BIT				);


			mainInfo.m_rigidBodyCinfo.m_collisionFilterInfo = (int)obFlag.base;
			mainInfo.m_rigidBodyCinfo.m_motionType	= hkMotion::MOTION_DYNAMIC;
			mainInfo.m_rigidBodyCinfo.m_qualityType	= HK_COLLIDABLE_QUALITY_MOVING;
			
			// mass properties, for a moment take simply box  inertia
			hkAabb obAABB;
			mainInfo.m_rigidBodyCinfo.m_shape->getAabb(hkTransform::getIdentity(),0.0f,obAABB);		
			hkVector4 obHalfExtents;
			obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
			obHalfExtents.mul4(0.5f);			

			hkMassProperties obMassProperties;
			obMassProperties.m_centerOfMass = mainInfo.m_rigidBodyCinfo.m_centerOfMass;
			obMassProperties.m_mass = mainInfo.m_rigidBodyCinfo.m_mass;

			hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,mainInfo.m_rigidBodyCinfo.m_mass,obMassProperties);
			mainInfo.m_rigidBodyCinfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;

			if (pobInfo)
			{
				// override some body values with values suggested by source info
				mainInfo.m_rigidBodyCinfo.m_motionType = pobInfo->m_motionType;
				mainInfo.m_rigidBodyCinfo.m_qualityType = pobInfo->m_qualityType;
				mainInfo.m_rigidBodyCinfo.m_maxLinearVelocity = pobInfo->m_maxLinearVelocity;
				mainInfo.m_rigidBodyCinfo.m_maxAngularVelocity = pobInfo->m_maxAngularVelocity;
			}

			mainInfo.m_transform = p_entity->GetHierarchy()->GetRootTransform();
			mainInfo.m_name = p_entity->GetName();

			lg->AddRigidBody(&mainInfo);
		}
		else
		/// ++++++++++++++++++++++++++++++++++++++++++++++++++++
		/// OBSOLETE WILL BE REMOVED		
		{
			if( obMainShapeList.obShapeList.size() != 0 )
			{
				hkShape* pobAllShapes = 0;
				if (obMainShapeList.obShapeList.size() == 1)
				{
					pobAllShapes = *obMainShapeList.obShapeList.begin();
				}
				else 
				{
					CScopedArray<hkShape*> apobShapeArray( NT_NEW hkShape*[obMainShapeList.obShapeList.size()] );

					int iShapeIndex = 0;
					ntstd::List<hkShape*>::iterator obShapes = obMainShapeList.obShapeList.begin();
					while(obShapes!=obMainShapeList.obShapeList.end())
					{
						apobShapeArray[iShapeIndex] = (*obShapes);
						++iShapeIndex;
						++obShapes;
					}

					// Create a list shape for the single rigid body
					pobAllShapes = CreateListShape(apobShapeArray.Get(),obMainShapeList.obShapeList.size());
				}

				Physics::BodyCInfo obInfo;

				if (pobInfo) // Copy user defined data if specified
				{
					obInfo.m_rigidBodyCinfo=*pobInfo;
				}

				Transform* pobRoot=p_entity->GetHierarchy()->GetRootTransform();

				const CMatrix& obWorldMatrix(pobRoot->GetWorldMatrix());
				CQuat obRotation(obWorldMatrix);
				obRotation.Normalise();

				obInfo.m_rigidBodyCinfo.m_position = hkVector4(obWorldMatrix.GetTranslation().X(),obWorldMatrix.GetTranslation().Y(),obWorldMatrix.GetTranslation().Z());
				//obInfo.m_rotation = hkQuaternion(obRotation.X(),obRotation.Y(),obRotation.Z(),obRotation.W());
				hkQuaternion rotation = Physics::MathsTools::CQuatTohkQuaternion(obRotation);
				obInfo.m_rigidBodyCinfo.m_rotation = rotation;

				Physics::EntityCollisionFlag obFlag; obFlag.base = 0;
				obFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
				obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
					Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
					Physics::RAGDOLL_BIT						|
					Physics::SMALL_INTERACTABLE_BIT				|
					Physics::LARGE_INTERACTABLE_BIT				);


				obInfo.m_rigidBodyCinfo.m_collisionFilterInfo = (int)obFlag.base;
				obInfo.m_rigidBodyCinfo.m_shape=pobAllShapes;

				// TEMPORARY: This bit of code ensures the center of mass is at the center of the shape.
				// At some point, we will want the artists to be able to define where the center of mass is themselves.
				hkAabb obAABB;
				pobAllShapes->getAabb(hkTransform::getIdentity(),0.0f,obAABB);

				// Calculate inertia tensor (weight distribution)
				hkVector4 obHalfExtents;
				obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
				obInfo.m_rigidBodyCinfo.m_centerOfMass(0)=(obAABB.m_min(0) + obAABB.m_max(0)) * 0.5f;
				obInfo.m_rigidBodyCinfo.m_centerOfMass(1)=(obAABB.m_min(1) + obAABB.m_max(1)) * 0.5f;
				obInfo.m_rigidBodyCinfo.m_centerOfMass(2)=(obAABB.m_min(2) + obAABB.m_max(2)) * 0.5f;
				obHalfExtents.mul4(0.5f);

				obInfo.m_rigidBodyCinfo.m_friction = 0.4f;
				obInfo.m_rigidBodyCinfo.m_restitution = 0.4f;
				obInfo.m_rigidBodyCinfo.m_linearDamping = 0.1f;
				obInfo.m_rigidBodyCinfo.m_angularDamping = 0.7f;

				hkMassProperties obMassProperties;
				obMassProperties.m_centerOfMass = obInfo.m_rigidBodyCinfo.m_centerOfMass;
				obMassProperties.m_mass = obInfo.m_rigidBodyCinfo.m_mass;
				hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,obInfo.m_rigidBodyCinfo.m_mass,obMassProperties);
				obInfo.m_rigidBodyCinfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;
				obInfo.m_rigidBodyCinfo.m_inertiaTensor.mul( 0.7f );
				obInfo.m_rigidBodyCinfo.m_motionType	= hkMotion::MOTION_DYNAMIC;
				obInfo.m_rigidBodyCinfo.m_qualityType	= HK_COLLIDABLE_QUALITY_MOVING;
				obInfo.m_transform = pobRoot;

				lg->AddRigidBody( &obInfo );
			}

			// ----- Step 3: Create rigid bodies for each transform using the listshapes we've created -----

			float fMaxLinearVelocity;
			float fMaxAngularVelocity;

			if (pobInfo)
			{
				fMaxLinearVelocity=pobInfo->m_maxLinearVelocity;
				fMaxAngularVelocity=pobInfo->m_maxAngularVelocity;
			}
			else
			{
				fMaxLinearVelocity=200.0f; // Havok default
				fMaxAngularVelocity=100.0f; // Havok default
			}

			for(int iCount=0; iCount<pobThisClump->m_iNumberOfTransforms; ++iCount)
			{
				// Create an hkShapeList for this transform

				if (pobListShape[iCount].obShapeList.size()>0)
				{

					hkShape* pobAllShapes = 0;

					if (pobListShape[iCount].obShapeList.size() == 1)
					{
						pobAllShapes = *pobListShape[iCount].obShapeList.begin();
					}
					else 
					{
						CScopedArray<hkShape*> apobShapeArray( NT_NEW hkShape*[pobListShape[iCount].obShapeList.size()] );

						int iShapeIndex = 0;

						ntstd::List<hkShape*>::iterator obShapes = pobListShape[iCount].obShapeList.begin();

						while(obShapes!=pobListShape[iCount].obShapeList.end())
						{
							apobShapeArray[iShapeIndex] = (*obShapes);
							++iShapeIndex;
							++obShapes;
						}

						pobAllShapes = CreateListShape(apobShapeArray.Get(),pobListShape[iCount].obShapeList.size());
					}

										

					pobListShape[iCount].obShapeList.clear();

					// Create hkRigidBody and add it to the state

					Transform* pobBoneTransfom = pobHierarchy->GetTransform(iCount);
					const CMatrix& obWorldMatrix = pobBoneTransfom->GetWorldMatrix();
					CQuat obWorldRotation(obWorldMatrix);
					obWorldRotation.Normalise();

					Physics::BodyCInfo obInfo;

					obInfo.m_rigidBodyCinfo.m_position.set(obWorldMatrix.GetTranslation().X(),obWorldMatrix.GetTranslation().Y(),obWorldMatrix.GetTranslation().Z());
					obInfo.m_rigidBodyCinfo.m_rotation = Physics::MathsTools::CQuatTohkQuaternion(obWorldRotation);

					obInfo.m_rigidBodyCinfo.m_motionType = hkMotion::MOTION_DYNAMIC; // We set this initially to ensure the rigid body is properly initialised
					obInfo.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;

					obInfo.m_rigidBodyCinfo.m_restitution = 0.01f;
					obInfo.m_rigidBodyCinfo.m_mass = 10;
					obInfo.m_rigidBodyCinfo.m_shape = pobAllShapes;

					Physics::EntityCollisionFlag obFlag; obFlag.base = 0;
					obFlag.flags.i_am = Physics::SMALL_INTERACTABLE_BIT;
					obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
						Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
						Physics::RAGDOLL_BIT						|
						Physics::SMALL_INTERACTABLE_BIT				|
						Physics::LARGE_INTERACTABLE_BIT				);	
					obInfo.m_rigidBodyCinfo.m_collisionFilterInfo = (int)obFlag.base;

					obInfo.m_rigidBodyCinfo.m_maxLinearVelocity = fMaxLinearVelocity;
					obInfo.m_rigidBodyCinfo.m_maxAngularVelocity = fMaxAngularVelocity;

					hkAabb obAabb;
					pobAllShapes->getAabb( hkTransform::getIdentity(), 0.01f, obAabb );
					hkVector4 obCentre(obAabb.m_min);
					obCentre.add4(obAabb.m_max);
					obCentre.mul4(0.5f);

					// Check extents to see if it's thin, and restrict angular velocity accordingly if so
					float x = obCentre(0) - obAabb.m_min(0);
					float y = obCentre(1) - obAabb.m_min(1);
					float z = obCentre(2) - obAabb.m_min(2);

					// If the relative size of extents is over a certain value, we need to treat this box specially
					float fThinThreshold = 3.0f;
					if ((x/y > fThinThreshold || y/x > fThinThreshold) ||
						(x/z > fThinThreshold || z/x > fThinThreshold) ||
						(y/z > fThinThreshold || z/y > fThinThreshold) )
					{
						if (obInfo.m_rigidBodyCinfo.m_maxAngularVelocity > 4.0f)
							obInfo.m_rigidBodyCinfo.m_maxAngularVelocity = 4.0f;
						obInfo.m_rigidBodyCinfo.m_motionType = hkMotion::MOTION_THIN_BOX_INERTIA;
						if (obInfo.m_rigidBodyCinfo.m_angularDamping < 0.05f)
							obInfo.m_rigidBodyCinfo.m_angularDamping = 0.05f;
						obInfo.m_rigidBodyCinfo.m_angularDamping *= 6.0f;
						if (obInfo.m_rigidBodyCinfo.m_angularDamping > 1.0f)
						{
							obInfo.m_rigidBodyCinfo.m_angularDamping = 0.95f;
						}
					}

					ComputeCentreOfMassAndInertiaTensor(pobAllShapes,obInfo.m_rigidBodyCinfo);

					obInfo.m_exceptionFlag.flags.exception_set |= Physics::IGNORE_ENTITY_PTR_BIT;

					obInfo.m_transform = pobBoneTransfom;

					RigidBody* body = RigidBody::ConstructRigidBody( p_entity, &obInfo);
					lg->m_bodiesList.push_back( body );

					// Create our rigid body
					/*hkRigidBody* pobThisRigidBody = NT_NEW hkRigidBody(obInfo);

					const CRenderable* pobRenderable=m_pobParentEntity->GetRenderableComponent()->GetRenderable(pobBoneTransfom);

					// Register this rigid body with the dynamics animated state

					pobNewState->RegisterSubBody(pobThisRigidBody,pobBoneTransfom,pobRenderable);*/
				}
			}
		}

		NT_DELETE_ARRAY( pobListShape );

		/// OBSOLETE WILL BE REMOVED
		/// ----------------------------------------------


		lg->Activate();

		return lg;
	}

	SpearLG* ClumpTools::ConstructSpearLGFromClump( CEntity* p_entity, hkRigidBodyCinfo* pobInfo )
	{
		ntAssert_p(false,("This loader should not be used. How did we get here? PeterFe")); 
		SpearLG* lg = NT_NEW SpearLG( p_entity->GetName(), p_entity );	

		CHierarchy* pobHierarchy=p_entity->GetHierarchy();

		const CClumpHeader* pobThisClump = pobHierarchy->GetClumpHeader();
		UNUSED( pobThisClump);

		//user_error_p(pobThisClump->m_iNumberOfColprims != 0, ("Error: Entity %s has no colprims!\n",p_entity->GetName().c_str()));
		//ntAssert(pobThisClump->m_iNumberOfColprims != 0); // Ensure this rigid body has colprims!

		//*
		Physics::EntityCollisionFlag obCollisionFlag;
		obCollisionFlag.base = 0;
		obCollisionFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
		obCollisionFlag.flags.i_collide_with = (	//Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													//Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
													//Physics::RAGDOLL_BIT						|
													Physics::SMALL_INTERACTABLE_BIT				|
													Physics::LARGE_INTERACTABLE_BIT				);
		//*/

		ListShape obShapesInRigid;

		ntstd::String filenameString = AlterFilename( ntStr::GetString(p_entity->GetClumpString()) );
		CHashedString psXMLFile = CHashedString(filenameString);

		if( !File::Exists( ntStr::GetString(filenameString) ) )
		{
			NT_DELETE( lg );
			return 0;
		}

		DataObject* obj = 0;
		
		obj = ObjectDatabase::Get().GetDataObjectFromName( psXMLFile );

		if( ( obj == 0 ) )
		{
			// Tell people what we're up to
			ntPrintf( "XML loading \'%s\'\n", ntStr::GetString(filenameString) );

			// Open the XML file in memory
			FileBuffer obFile( ntStr::GetString(filenameString), true );

			if ( !ObjectDatabase::Get().LoadDataObject( &obFile, filenameString ) )
			{
				ntError_p( false, ( "Failed to parse XML file '%s'", ntStr::GetString(filenameString) ) );
			}

			obj = ObjectDatabase::Get().GetDataObjectFromName( psXMLFile );
		}
	
		ObjectContainer* current = (ObjectContainer*) obj->GetBasePtr();
		
		for (	ObjectContainer::ObjectList::iterator obIt = current->m_ContainedObjects.begin(); 
				obIt != current->m_ContainedObjects.end(); 
				obIt++ )
		{
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psRigidBody") )
			{
				psRigidBody* pobBody = (psRigidBody*)((*obIt)->GetBasePtr());
				if( pobBody->m_shape != 0 )
				{
					
					BodyCInfo info = pobBody->GetBodyInfo( ntStr::GetString((*obIt)->GetName()), p_entity->GetHierarchy(), pobInfo);
                    info.m_rigidBodyCinfo.m_collisionFilterInfo = obCollisionFlag.base;
					RigidBody* elem = lg->AddRigidBody( &info );
					// [Mus[ - Should it be here ?
					elem->Activate();
					pobBody->m_associatedRigid = elem->GetHkRigidBody();
				}
			}
			/// +++++++++++++++++++++++++++++++++++++
			/// OBSOLETE WILL BE REMOVED
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psShapeTrfLink") )
			{
				psShapeTrfLink* pobBody = (psShapeTrfLink*)((*obIt)->GetBasePtr());

				//TEMPORARY UNTIL THIS IS A HASHED XML OBJECT VALUE
				//if((obThisVolume.m_pcType==0)||(strcmp(obThisVolume.m_pcType,"")==0))
				{
					//Using the bone information find the entity space position of the volume
					/*const Transform* pobBoneTransfom =pobHierarchy->GetTransform(obThisVolume.m_iTransform);
					
					CMatrix obLocalSpaceOfBone(pobBoneTransfom->GetWorldMatrix());

					CMatrix obWorldSpaceOfRoot(pobHierarchy->GetRootTransform()->GetWorldMatrix());

					CMatrix obInverseWorld(obWorldSpaceOfRoot.GetAffineInverse());
					
					obLocalSpaceOfBone = obInverseWorld*obLocalSpaceOfBone;

					CMatrix obBoneOffset(obThisVolume.m_obRotation,obThisVolume.m_obTranslation);
					CMatrix obLocalSpaceOfVolume = obBoneOffset*obLocalSpaceOfBone;*/

					//hkShape* pobShape = ConstructShapeFromColprim(obThisVolume);
					hkShape* pobShape = pobBody->m_shape->BuildShape();//ConstructConvexShapeFromColprim(obThisVolume);

					//Create a transform shape from the shape and the entity space position
					/*hkVector4 obPosition(obLocalSpaceOfVolume.GetTranslation().X(),
										obLocalSpaceOfVolume.GetTranslation().Y(),
										obLocalSpaceOfVolume.GetTranslation().Z());

					CQuat obQuatRotation(obLocalSpaceOfVolume);
					obQuatRotation.Normalise();
					//hkQuaternion obRotation(obQuatRotation.X(),obQuatRotation.Y(),obQuatRotation.Z(),obQuatRotation.W());
					hkQuaternion obRotation = Physics::MathsTools::CQuatTohkQuaternion(obQuatRotation); 


					hkTransform obLocalTransform(obRotation,obPosition);

					//hkTransformShape* pobPositionedShape = NT_NEW hkTransformShape(pobShape, obLocalTransform);
					hkConvexTransformShape* pobPositionedShape = NT_NEW hkConvexTransformShape(pobShape, obLocalTransform);*/

					//Add the transform shape to the list shape
					obShapesInRigid.obShapeList.push_back(pobShape);
				}
			}
			
			/// OBSOLETE WILL BE REMOVED
			/// ------------------------------------	
		}

		/// ++++++++++++++++++++++++++++++++++++++++++
		/// OBSOLETE WILL BE REMOVED			
		if( obShapesInRigid.obShapeList.size() != 0 )
		{
			hkShape* pobAllShapes = 0;
			if ( obShapesInRigid.obShapeList.size() == 1 )
			{
				pobAllShapes = *obShapesInRigid.obShapeList.begin();

			}
			else 
			{
				CScopedArray<hkShape*> apobShapeArray( NT_NEW hkShape*[obShapesInRigid.obShapeList.size()] );

				int iShapeIndex = 0;
				ntstd::List<hkShape*>::iterator obShapes = obShapesInRigid.obShapeList.begin();
				while(obShapes!=obShapesInRigid.obShapeList.end())
				{
					apobShapeArray[iShapeIndex] = (*obShapes);
					++iShapeIndex;
					++obShapes;
				}

				//Create a list shape for the rigid body
				pobAllShapes = CreateListShape(apobShapeArray.Get(),obShapesInRigid.obShapeList.size());
			}

			CMatrix obWorldSpaceOfEntity = pobHierarchy->GetRootTransform()->GetWorldMatrix();

			// Create a single rigid body for the whole shape

			//hkRigidBody* pobRigidBody=NULL;

			if (pobInfo==NULL) // If no rigid body definition has been defined, create a default one
			{
				Physics::BodyCInfo obInfo;

				obInfo.m_rigidBodyCinfo.m_position = hkVector4(obWorldSpaceOfEntity.GetTranslation().X(),
											obWorldSpaceOfEntity.GetTranslation().Y(),
											obWorldSpaceOfEntity.GetTranslation().Z());

				CQuat obRotation(obWorldSpaceOfEntity);

				obInfo.m_rigidBodyCinfo.m_rotation = Physics::MathsTools::CQuatTohkQuaternion(obRotation);

				obInfo.m_rigidBodyCinfo.m_motionType = hkMotion::MOTION_DYNAMIC;
				obInfo.m_rigidBodyCinfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;

				obInfo.m_rigidBodyCinfo.m_mass = 1.0f;
				
				ComputeCentreOfMassAndInertiaTensor(pobAllShapes,obInfo.m_rigidBodyCinfo);

				//POSSIBLE CALLBACK ON MAKING A CONTACT SO CAN CHANGE THE FRICTION VALUES

				obInfo.m_rigidBodyCinfo.m_shape = pobAllShapes;
				obInfo.m_rigidBodyCinfo.m_collisionFilterInfo = (int)obCollisionFlag.base;
				obInfo.m_transform = pobHierarchy->GetRootTransform();
				
				lg->AddRigidBody( &obInfo );
			}
			else
			{
				// Fill in other required information

				pobInfo->m_position = hkVector4(obWorldSpaceOfEntity.GetTranslation().X(),
											obWorldSpaceOfEntity.GetTranslation().Y(),
											obWorldSpaceOfEntity.GetTranslation().Z());

				CQuat obRotation(obWorldSpaceOfEntity);
				obRotation.Normalise();

				pobInfo->m_rotation = Physics::MathsTools::CQuatTohkQuaternion(obRotation);

				pobInfo->m_motionType = hkMotion::MOTION_DYNAMIC;
				pobInfo->m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;

				pobInfo->m_shape = pobAllShapes;
				pobInfo->m_collisionFilterInfo = (int)obCollisionFlag.base;
				//pobInfo->m_allowedPenetrationDepth=0.2f;

				/*
				hkMassProperties obMassProperties;

				hkAabb aabb;
				hkTransform trf; trf.setIdentity();
				pobAllShapes->getAabb(trf,0.01f,aabb);

				obMassProperties.m_centerOfMass = hkVector4(0.f, 0.f, 0.f);
				obMassProperties.m_mass = pobInfo->m_mass;

				hkVector4 obHalfExtents; obHalfExtents.setSub4(aabb.m_max, aabb.m_min);
				obHalfExtents.mul4(0.5f);

				hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,pobInfo->m_mass,obMassProperties);
				pobInfo->m_inertiaTensor = obMassProperties.m_inertiaTensor;
				*/

				ComputeCentreOfMassAndInertiaTensor(pobAllShapes,*pobInfo);

				Physics::BodyCInfo obInfo;
				obInfo.m_rigidBodyCinfo = (*pobInfo);
				obInfo.m_transform = pobHierarchy->GetRootTransform();

				lg->AddRigidBody( &obInfo );

			}
		}
		/// OBSOLETE WILL BE REMOVED
		/// ------------------------------------	

		
		lg->Activate();
		return lg;
	}
#endif
}
