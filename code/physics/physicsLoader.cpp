//------------------------------------------------------------------------------------------
//!
//!	\file physicsLoader.cpp
//!
//------------------------------------------------------------------------------------------
#include "config.h"
#include "physicsLoader.h"
#ifdef BINARY_PHYSICS_LOADER
#include "ShapeUserData.h"
#endif
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkdynamics/world/hkPhysicsSystem.h>
#include <hkserialize/util/hkRootLevelContainer.h>
#include "xmlinterfaces.h"
#include "core/profiling.h"
#include <hkcollide/shape/convextranslate/hkConvexTranslateShape.h>

namespace Physics
{ 
	 
	/***************************************************************************************************
	*
	*	FUNCTION		PhysicsData::MakePlatformAnimName(LogicGroup& dest, const CloningParams& params )
	*
	*	DESCRIPTION		Generates plaform specific name for an animation
	*
	***************************************************************************************************/
	void PhysicsData::CloneIntoLogicGroup(LogicGroup& dest, const CloningParams& params) const
	{
		// 1.) Create clone of physics system
		hkPhysicsSystem * clonedSystem = m_physicsSystem->clone();

		// 2.) Add rigid bodies into group
		const hkArray< hkRigidBody * > & bodies = clonedSystem->getRigidBodies();
		CEntity * entity = dest.GetEntity();

		for(int i = 0; i < bodies.getSize(); i++)
		{
			hkRigidBody * body = bodies[i];

			if (params.m_largeSmallInteractable)
			{
				// it is small or large interactable... make decision according to volume				
				hkAabb obAabb;
				body->getCollidable()->getShape()->getAabb( hkTransform::getIdentity(), 0.01f, obAabb );
				hkVector4 obHalfExtent;
				obHalfExtent.setSub4(obAabb.m_max, obAabb.m_min);

				EntityCollisionFlag obCollisionFlag;
				obCollisionFlag.base = params.m_collisionFlags.base; 
	
				if ((obHalfExtent(0) * obHalfExtent(1) * obHalfExtent(3)) * 2.0f > LARGE_INTERACTABLE_VOLUME_THRESHOLD)
					obCollisionFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;
				else
					obCollisionFlag.flags.i_am = Physics::SMALL_INTERACTABLE_BIT;

				body->setCollisionFilterInfo(obCollisionFlag.base);
			}
			else
				body->setCollisionFilterInfo(params.m_collisionFlags.base);

			hkPropertyValue val( (int)params.m_filterExceptionFlags.base );
			body->addProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT, val );

			// static?
			if (params.m_static)
				body->setMotionType(hkMotion::MOTION_FIXED);

			// find transform for it in hierarchy			
			Transform * trans = (i < (int) m_transfromHash.size()) ? entity->GetHierarchy()->GetTransformFromHash(m_transfromHash[i]) : NULL;
			if (!trans)
				trans = entity->GetHierarchy()->GetRootTransform();

			RigidBody * rigidBody = RigidBody::ConstructRigidBody(body,entity,trans);

			dest.AddRigidBody(rigidBody);
		}
	};

	void PhysicsData::Release()
	{
		m_refCount--;
		if (m_refCount == 0)
		{
			CPhysicsLoader::Get().Unload(m_name);
			// delete it's self
			NT_DELETE_CHUNK(MC_PHYSICS, this);
		}		
	};

	PhysicsDataRef::PhysicsDataRef(PhysicsData* data) : m_data(data) 
	{
		if (m_data)
			m_data->AddRef();
	};


	PhysicsDataRef::PhysicsDataRef(const PhysicsDataRef& ref) : m_data(ref.m_data) 
	{
		if (m_data)
			m_data->AddRef();
	};

	const PhysicsDataRef& PhysicsDataRef::operator=(const PhysicsDataRef& ref)
	{
		if (m_data)
			m_data->Release();

		m_data = ref.m_data; 
		if (m_data)
			m_data->AddRef();		

		return *this;
	};

	const PhysicsDataRef& PhysicsDataRef::operator=( PhysicsData* data)
	{
		if (m_data)		
			m_data->Release();

		m_data = data; 
		if (m_data)
			m_data->AddRef();		

		return *this;
	};

	PhysicsDataRef::~PhysicsDataRef() 
	{
		if (m_data)
			m_data->Release();
	};



	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsLoader::FinishShape(hkShape * shape)
	*
	*	DESCRIPTION		Destructor
	*
	***************************************************************************************************/
#ifdef BINARY_PHYSICS_LOADER
	void CPhysicsLoader::FinishShape(hkShape * shape) 
	{
#ifdef PHYSICS_LOADER_410
		shape->setUserData((hkUlong) PhysicsMaterialTable::Get().GetMaterialFromId(shape->getUserData()));		
#else
		ShapeUserData * shapeData = reinterpret_cast<ShapeUserData *>(shape->getUserData());				
		if (shapeData)
#if HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1
			shape->setUserData((hkUlong) PhysicsMaterialTable::Get().GetMaterialFromId(shapeData->m_uiMaterialID));
#else
			shape->setUserData(PhysicsMaterialTable::Get().GetMaterialFromId(shapeData->m_uiMaterialID));
#endif //HAVOK_SDK_VERSION_MAJOR >= 4 && HAVOK_SDK_VERSION_MINOR >= 1
#endif //PHYSICS_LOADER_410

		switch (shape->getType())
		{
		case HK_SHAPE_LIST:
		case HK_SHAPE_CONVEX_LIST:
			{
				hkListShape * list = static_cast<hkListShape *>(shape);
				for(int i = 0; i < list->getNumChildShapes(); i++)
				{
					FinishShape(const_cast<hkShape *>(list->getChildShape(i)));
				}
				break;
			}
		case HK_SHAPE_CONVEX_TRANSLATE:
			{
				hkConvexTranslateShape * transShape = static_cast<hkConvexTranslateShape *>(shape);
				FinishShape(const_cast<hkConvexShape *>(transShape->getChildShape()));
				break;
			}
		case HK_SHAPE_CONVEX_TRANSFORM:
			{
				hkConvexTransformShape * transShape = static_cast<hkConvexTransformShape *>(shape);
				FinishShape(const_cast<hkConvexShape *>(transShape->getChildShape()));
				break;
			}
		case HK_SHAPE_TRANSFORM :
			{
				hkTransformShape * transShape = static_cast<hkTransformShape *>(shape);
				FinishShape(const_cast<hkShape *>(transShape->getChildShape()));
				break;
			}
		case HK_SHAPE_MOPP:
		case HK_SHAPE_BV_TREE:
			{
				hkBvTreeShape * bvShape = static_cast<hkBvTreeShape *>(shape);
				FinishShape(const_cast<hkShapeCollection *>(bvShape->getShapeCollection()));
				break;
			}
		case  HK_SHAPE_TRIANGLE_COLLECTION:
			{
				hkMeshShape * meshShape = static_cast<hkMeshShape *>(shape);
				hkMeshShape::Subpart& subpart = meshShape->getSubpartAt(0);

				if (subpart.m_numMaterials > 0)
				{
					// fixup the material table... 
					
					ntAssert(sizeof(hsMeshMaterial) == 8);
					ntAssert(sizeof(psPhysicsMaterial *) == 4);					
					ntAssert(subpart.m_materialStriding == sizeof(hsMeshMaterial));

					for(int i = 0; i < subpart.m_numMaterials; i++)
					{
						uint32_t& id = *(((uint32_t *)subpart.m_materialBase) + (2 * i) + 1);
						id = (uint32_t)PhysicsMaterialTable::Get().GetMaterialFromId(id);
					}
				}
			}
		default:;
		}
	}
#endif
	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsLoader::AlterFilenameExtension( const char * const filename )
	*
	*	DESCRIPTION		Change extension. Usually the input is clump filename. 
	*
	***************************************************************************************************/
	ntstd::String CPhysicsLoader::AlterFilenameExtension( const char * const filename )
	{	
		// replace appendix ... 	
		ntstd::String str( Util::NoLastExtension(filename));
#ifdef BINARY_PHYSICS_LOADER 
#ifdef PLATFORM_PS3
		str += ntstd::String( ".ps_ps3\0" );
#else
		str += ntstd::String( ".ps\0" );
#endif
#else
		str += ntstd::String( ".ps.xml\0" );
#endif

		// Make the filename all lower-case and replace / with \.
		ntstd::transform( str.begin(), str.end(), str.begin(), &ntstd::Tolower );
		ntstd::transform( str.begin(), str.end(), str.begin(), &ntstd::ConvertSlash );		

		return str;
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsLoader::MakePlatformPhysicsName( const char* pNeutralName, char* pPlatformName )
	*
	*	DESCRIPTION		Generates plaform specific name for an physics file
	*
	***************************************************************************************************/
	void	CPhysicsLoader::MakePlatformPhysicsName( const char* pNeutralName, char* pPlatformName )
	{

		// appends file system root and content directory
#ifdef PLATFORM_PS3
#ifdef BINARY_PHYSICS_LOADER
		Util::GetFiosFilePath_Platform( pNeutralName, pPlatformName );
#else
		Util::GetFiosFilePath( pNeutralName, pPlatformName );
#endif
#else
		Util::GetFiosFilePath( pNeutralName, pPlatformName );
#endif
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CAnimLoader::LoadAnim_Neutral( const char *pNeutralName )
	*
	*	DESCRIPTION		Looks up pcFilename in the map, if found, returns the existing animation
	*					header, if not found, loads the animation header then returns it.
	*
	***************************************************************************************************/
	PhysicsData* CPhysicsLoader::LoadPhysics_Neutral( const char *pNeutralName )
	{
		char pPlatformName[MAX_PATH];
		MakePlatformPhysicsName( pNeutralName, pPlatformName );
		return LoadPhysics_Platform( pPlatformName );
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CAnimLoader::GetAnim( const char *pcFilename )
	*
	*	DESCRIPTION		Looks up pcFilename in the map, if found, returns the existing animation
	*					header, if not found, loads the animation header then returns it.
	*
	***************************************************************************************************/
	PhysicsData* CPhysicsLoader::LoadPhysics_Platform( const char *pPlatformName )
	{
		LOAD_PROFILE( CPhysicsLoader_LoadPhysics_Platform )

		// Check  cache first.
		CHashedString name(pPlatformName);
		CacheMap::iterator it = m_cache.find( (uint32_t) name.GetValue());

		if( it != m_cache.end())				
			return (*it).second;

		// nope proceed with load	
#ifdef BINARY_PHYSICS_LOADER
		Util::SetToPlatformResources();
		if( !File::Exists( pPlatformName ) )
		{		
			Util::SetToNeutralResources();
			return 0;
		}

		PhysicsData * data = NT_NEW_CHUNK(MC_PHYSICS) PhysicsData(name);
		hkRootLevelContainer *container = data->m_loader.load(pPlatformName);
		if (!container)
		{
			Util::SetToNeutralResources();
			NT_DELETE_CHUNK(MC_PHYSICS, data);
			return 0;
		}

		data->m_physicsSystem = reinterpret_cast<hkPhysicsSystem *> (container->findObjectByType(hkPhysicsSystemClass.getName()));

		// yop physics system ready.. now go and fix rigid bodies
		const hkArray<hkRigidBody *> & bodies = data->m_physicsSystem->getRigidBodies();
		for(int i = 0; i < bodies.getSize(); i++)
		{
#ifdef PHYSICS_LOADER_410	
			hkRigidBody * body = bodies[i];
			if (body->hasProperty(0))
			{
				data->m_transfromHash.push_back(body->getProperty(0).getInt());
				body->removeProperty(0);
			}
			else
			{
				data->m_transfromHash.push_back(0);
			}
#else
			const hkShape * shape = bodies[i]->getCollidable()->getShape();
			ShapeUserData * shapeData = reinterpret_cast<ShapeUserData *>(shape->getUserData());
			data->m_transfromHash.push_back(shapeData->m_uiTransformHash);
#endif
		}

		// now finish the shapes... 
		for(int i = 0; i < bodies.getSize(); i++)
		{
			const hkShape * shape = bodies[i]->getCollidable()->getShape();
			FinishShape(const_cast<hkShape *>(shape));			
		}
#else
		Util::SetToNeutralResources();
		if( !File::Exists( pPlatformName ) )
		{					
			return 0;
		}


		DataObject* obj = 0;		
		obj = ObjectDatabase::Get().GetDataObjectFromName( name );

		if( ( obj == 0 ) )
		{
			// Tell people what we're up to
			ntPrintf( "XML loading \'%s\'\n", pPlatformName );

			// Open the XML file in memory
			FileBuffer obFile( pPlatformName, true );

			if ( !ObjectDatabase::Get().LoadDataObject( &obFile, pPlatformName ) )
			{
				ntError_p( false, ( "Failed to parse XML file '%s'", pPlatformName ) );
			}

			obj = ObjectDatabase::Get().GetDataObjectFromName( name );
		} 

		ObjectContainer* current = (ObjectContainer*) obj->GetBasePtr();
		

		/// +++++++++++++++++++++++++++++++++++++
		/// OBSOLETE WILL BE REMOVED
		ntstd::List<hkShape*> obShapeList;
		/// OBSOLETE WILL BE REMOVED
		///--------------------------------------

		PhysicsData * data = NT_NEW_CHUNK(MC_PHYSICS) PhysicsData(name);
		data->m_physicsSystem = HK_NEW hkPhysicsSystem;

		for (	ObjectContainer::ObjectList::iterator obIt = current->m_ContainedObjects.begin(); 
					obIt != current->m_ContainedObjects.end(); 
					obIt++ )
		{
			if ( 0 == strcmp( (*obIt)->GetClassName(), "psRigidBody") )
			{
				psRigidBody* pobBody = (psRigidBody*)((*obIt)->GetBasePtr());
				if( pobBody->m_shape != 0 )
				{
					BodyCInfo info = pobBody->GetBodyInfo( ntStr::GetString((*obIt)->GetName()), 0 );
					hkRigidBody * body = HK_NEW hkRigidBody(info.m_rigidBodyCinfo);
					body->setName(ntStr::GetString((*obIt)->GetName())); // who will deallocate this string????
					pobBody->m_associatedRigid = body;
					data->m_physicsSystem->addRigidBody(body);
					data->m_transfromHash.push_back(pobBody->m_uiTransformHash);
				}
			}
			
			/// +++++++++++++++++++++++++++++++++++++
			/// OBSOLETE WILL BE REMOVED
			else if ( 0 == strcmp( (*obIt)->GetClassName(), "psShapeTrfLink") )
			{
				psShapeTrfLink* pobBody = (psShapeTrfLink*)((*obIt)->GetBasePtr());
				
				// ... get the position and orientation ...
				data->m_transfromHash.push_back(0);

				// ... build the hkShape ...
				hkShape* pobShape = pobBody->m_shape->BuildShape(); //ConstructArbitraryShapeFromColprim( obThisVolume );
							
				obShapeList.push_back(pobShape);
			}	
			/// OBSOLETE WILL BE REMOVED
		    ///-------------------------------------------
			else if ( 0 == strcmp( (*obIt)->GetClassName(), "psConstraint_Ragdoll") )
			{
				psConstraint_Ragdoll* pobC = (psConstraint_Ragdoll*)((*obIt)->GetBasePtr());
				pobC->m_bUseRagdollHierarchy = true;
				hkConstraintInstance* instance = pobC->BuildConstraint( NULL );
				if( instance )
				{
					const char* pcJointName = ntStr::GetString((*obIt)->GetName());
					instance->setName(pcJointName);

					data->m_physicsSystem->addConstraint(instance);
				}
			}
			else if ( 0 == strcmp( (*obIt)->GetClassName(), "psConstraint_Hinge") )
			{
				psConstraint_Hinge* pobC = (psConstraint_Hinge*)((*obIt)->GetBasePtr());
				pobC->m_bUseRagdollHierarchy = true;
				hkConstraintInstance* instance = pobC->BuildConstraint( NULL );
				if( instance )
				{
					const char* pcName = ntStr::GetString((*obIt)->GetName());
					instance->setName(pcName);

					data->m_physicsSystem->addConstraint(instance);
				}
			}

		}

		/// +++++++++++++++++++++++++++++++++++++
		/// OBSOLETE WILL BE REMOVED
		if( obShapeList.size() != 0 )
		{
			user_warn_p(obShapeList.size() == 0, ("File %s is in old physics format. It needs to be exported in new format.", pPlatformName));

			hkShape* pobAllShapes; 
  
			if (obShapeList.size() == 1)
				pobAllShapes = *obShapeList.begin();
			else
			{   
				CScopedArray<hkShape*> apobShapeArray( NT_NEW hkShape*[obShapeList.size()] );

				int iShapeIndex = 0;
				ntstd::List<hkShape*>::iterator obShapes = obShapeList.begin();
				while(obShapes!=obShapeList.end())
				{
					apobShapeArray[iShapeIndex] = static_cast<hkShape*>(*obShapes);
					++iShapeIndex;
					++obShapes;
				}
				//Create a list shape for the rigid body
				pobAllShapes = CreateListShape( apobShapeArray.Get(), obShapeList.size() );				
			}			
			// Create a single rigid body for the whole shape			
			hkRigidBodyCinfo obInfo;
			
			obInfo.m_motionType	= hkMotion::MOTION_DYNAMIC;
			obInfo.m_qualityType	= HK_COLLIDABLE_QUALITY_MOVING;
			obInfo.m_mass			= 1.0f;

			hkAabb obAABB;
			pobAllShapes->getAabb( hkTransform::getIdentity(), 0.0f, obAABB);
			obInfo.m_centerOfMass(0)=(obAABB.m_min(0) + obAABB.m_max(0)) * 0.5f;
			obInfo.m_centerOfMass(1)=(obAABB.m_min(1) + obAABB.m_max(1)) * 0.5f;
			obInfo.m_centerOfMass(2)=(obAABB.m_min(2) + obAABB.m_max(2)) * 0.5f;

			hkMassProperties obMassProperties;
			obMassProperties.m_mass = obInfo.m_mass;

			hkVector4 obHalfExtents; obHalfExtents.setSub4(obAABB.m_max, obAABB.m_min);
			obHalfExtents.mul4(0.5f);

			hkInertiaTensorComputer::computeBoxVolumeMassProperties(obHalfExtents,obInfo.m_mass,obMassProperties);		
			obInfo.m_inertiaTensor = obMassProperties.m_inertiaTensor;
			
			obInfo.m_shape = pobAllShapes;

			Physics::EntityCollisionFlag obCollisionFlag;
			obCollisionFlag.base = 0;				
			obCollisionFlag.flags.i_am = Physics::LARGE_INTERACTABLE_BIT;			
			obCollisionFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
				Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
				Physics::RAGDOLL_BIT						|
				Physics::SMALL_INTERACTABLE_BIT				|
				Physics::LARGE_INTERACTABLE_BIT				|
				Physics::AI_WALL_BIT);

			obInfo.m_collisionFilterInfo = (int)obCollisionFlag.base;		

			hkRigidBody * body = HK_NEW hkRigidBody(obInfo);			
			data->m_physicsSystem->addRigidBody(body);
		}		
		/// OBSOLETE WILL BE REMOVED
		///--------------------------------------
#endif
		Util::SetToNeutralResources();
		m_cache[ name.GetValue() ] = data;			
		return data;		
	}

	/***************************************************************************************************
	*
	*	FUNCTION		CPhysicsLoader::Unload(const CHashedString & name)
	*
	*	DESCRIPTION		Looks up name in the map, if found, erase it.
	*
	***************************************************************************************************/
	void CPhysicsLoader::Unload(const CHashedString & name)
	{
		CacheMap::iterator it = m_cache.find( (uint32_t) name.GetValue());

		if( it != m_cache.end())
			m_cache.erase(it);
	}
}
