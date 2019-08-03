///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper Class
//
//  (c) 2003 IDV, Inc.
//
//  This class is provided to illustrate one way to incorporate
//  SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
//  calls that must be made on a per tree basis are done by this class.
//  Calls that apply to all trees (i.e. static SpeedTreeRT functions)
//  are made in the functions in main.cpp.
//
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//

///////////////////////////////////////////////////////////////////////  
//  Include Files

#include <stdlib.h>
#include <stdio.h>





#include <stdlib.h>
#include <stdio.h>

#include "speedtree/SpeedTreeShaders_ps3.h"
#include "speedtree/SpeedTreeWrapper_ps3.h"
#include "speedtree/SpeedTreeUtil_ps3.h"
#include "speedtree/SpeedTreeManager_ps3.h"
#include "speedtree/SpeedTreeRenderable_ps3.h"
#include "speedtree/SpeedTreeForest_ps3.h"
#include "speedtree/SpeedTreeDef_ps3.h"


#include "gfx/texturemanager.h"
#include "gfx/graphicsdevice.h"
#include "gfx/fxmaterial.h"
#include "core/util.h"
#include "gfx/renderer.h"
#include "anim/transform.h"
#include "gfx/sector.h"
#include "gfx/materialinstance.h"

#include "area/arearesourcedb.h"

// Physics
#include "physics/world.h"
#include "physics/collisionbitfield.h"
#include "physics/physicsmaterial.h"

// Havok
//#include "hkcollide/hkcollide.h"
#include <hkdynamics/world/hkWorld.h>
#include <hkdynamics/entity/hkRigidBody.h>
#include <hkcollide/shape/sphere/hkSphereShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkcollide/shape/capsule/hkCapsuleShape.h>

#include "uniqueptrcontainer.h"

const float SPEEDTREE_SCALE_FACTOR	= 0.33f;


class CSpeedTreePhysicsRep
{
// Only havok physics for the trees
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	typedef ntstd::Vector<hkRigidBody*>	RigidBodyContainer;

public:
	CSpeedTreePhysicsRep(CSpeedTreeWrapper* tree)
	{
		ntAssert(tree);
		CSpeedTreeRT* speedTree = tree -> GetSpeedTree();
		if (!speedTree)
		{
			ntAssert(0);
			return;
		}

		hkWorld* world = Physics::CPhysicsWorld::Get().GetHavokWorldP();
		if (!world)
		{
			ntAssert(0); // no havok world
			return;
		}

		int numObjects = speedTree -> GetNumCollisionObjects();
		if (numObjects < 1)
		{
			return;
		}

		m_rigidBodies.reserve(numObjects);
		Physics::psPhysicsMaterial * trunkMat = Physics::PhysicsMaterialTable::Get().GetMaterialFromId(HASH_GET_CODE(HASH_STRING_SPEEDTREE_TRUNK)); 

		// We destinguish between different types of collision geometry by collision shapes
		// Spheres are assumed to be used for the frond so they dont collide with the player but may have activated collision listeners 
		// which will in turn add a local wind to simulate some leaves movement
		// Capsules must be used for the trunk as they are always collidable
		// Boxes... same as capsules so far
		for (unsigned int shape = 0; shape < (unsigned int)numObjects; ++ shape)
		{
			CSpeedTreeRT::ECollisionObjectType type;

			float position[3];
			float dimensions[3];
			float angles[3];

			speedTree -> GetCollisionObject(shape, type, position, dimensions, angles);
			for (unsigned int elem = 0; elem < 3; ++ elem)
			{
				position[elem] *= tree -> GetCollisionScale();
				dimensions[elem] *= tree -> GetCollisionScale();
			}

			Physics::EntityCollisionFlag iEntFlag; iEntFlag.base = 0;
			hkRigidBodyCinfo	rigidBodyInfo;

			hkShape*	newShape = NULL;
			switch (type)
			{
			case CSpeedTreeRT::CO_SPHERE:
				{
					// ignore the spheres for now
					continue;
					newShape = HK_NEW hkSphereShape(dimensions[0]);	

					//if you start to use it, add physics material
					break;
				}
			case CSpeedTreeRT::CO_BOX:
				{
					// ignore the boxes
					continue;

					hkVector4	halfExtents(dimensions[0] * 0.5f, dimensions[2] * 0.5f, dimensions[1] * 0.5f);
					newShape = HK_NEW hkBoxShape(halfExtents);
					position[1] += dimensions[2] * 0.5f;		
					//if you start to use it, add physics material
					break;
				}

			case CSpeedTreeRT::CO_CAPSULE:
				{
					hkVector4	vertexA(0, 0, 0);
					hkVector4	vertexB(0, dimensions[1], 0);
					newShape = HK_NEW hkCapsuleShape(vertexA, vertexB, dimensions[0]);	
					newShape->setUserData((hkUlong) trunkMat);
					break;
				}

			default:
				ntError_p(0, ("SpeedTree : Unknown collision object type"));
			}

			ntError_p(newShape, ("SpeedTree : Failed to create collision shape"));

			const float* treePos = tree -> GetPosition();

			if (!treePos)
			{
				ntAssert(0);
				continue;
			}


			iEntFlag.flags.i_am |= Physics::LARGE_INTERACTABLE_BIT;

			// I collide with
			iEntFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
												Physics::CHARACTER_CONTROLLER_ENEMY_BIT	    |
												Physics::RAGDOLL_BIT						|
												Physics::SMALL_INTERACTABLE_BIT				|
												Physics::LARGE_INTERACTABLE_BIT);


			hkVector4 bodyPosition(treePos[0] + position[0], treePos[1] + position[1], treePos[2] + position[2]);

			rigidBodyInfo.m_mass				= 0;
			rigidBodyInfo.m_motionType			= hkMotion::MOTION_FIXED;
			rigidBodyInfo.m_qualityType			= HK_COLLIDABLE_QUALITY_FIXED;
			rigidBodyInfo.m_shape				= newShape;
			rigidBodyInfo.m_position			= bodyPosition;
			rigidBodyInfo.m_collisionFilterInfo = iEntFlag.base;
			// set rotation

			hkRigidBody*	rigidBody = HK_NEW hkRigidBody(rigidBodyInfo);

			world -> addEntity( rigidBody, HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE );
			newShape -> removeReference();

			m_rigidBodies.push_back( rigidBody );

		} // for each collision object
	}

	~CSpeedTreePhysicsRep()
	{
		hkWorld* world = Physics::CPhysicsWorld::Get().GetHavokWorldP();
		if (!world)
		{
			ntAssert(0); // no havok world
			return;
		}

		for (RigidBodyContainer::iterator iter = m_rigidBodies.begin(); iter != m_rigidBodies.end(); ++ iter)
		{
			world -> removeEntity(*iter);
			HK_DELETE(*iter);
		}
	}

private:
	RigidBodyContainer	m_rigidBodies;

#endif

};













///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::CSpeedTreeWrapper

CSpeedTreeWrapper::CSpeedTreeWrapper(CSpeedTreeRT* pTree, CSpeedTreeForest* pForest)
	:m_pSpeedTree(pTree)
	,m_physicsRep(NULL)
    ,m_pGeometryCache(NULL)
	,m_minLODDistance(5.f)
	,m_maxLODDistance(15.f)
	,m_pInstanceOf(0)
	,m_pForest(pForest)
	,m_ulRenderBitVector(Speedtree_RenderAll)
	,m_scale(1.f)
	,m_collisionGeometryScale(SPEEDTREE_SCALE_FACTOR)
	,m_windOffset(0)
	,m_rotation(0)
#ifdef SPEEDTREE_COLLECT_STATS
	,m_def(NULL)
#endif
{
    //m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;
    //m_unNumWrappersActive++;
	for(int iIndex = 0 ; iIndex < 4 ; ++iIndex )
	{
		m_renderables[iIndex]=0;
	}
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::~CSpeedTreeWrapper

CSpeedTreeWrapper::~CSpeedTreeWrapper( )
{
	for(int iIndex = 0 ; iIndex < 4 ; ++iIndex )
	{
		if(m_renderables[iIndex])
		{
			CSector::Get().GetRenderables().RemoveRenderable(m_renderables[iIndex]);
			NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK, m_renderables[iIndex] );
		}
	}

	if(!m_pInstanceOf)
	{
		NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK,m_pTextureInfo);
	
		m_leafBuffers.Release();
		m_frondBuffers.Release();
		m_branchBuffers.Release();
																	  
		NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK,m_pGeometryCache);
	}
	
	// SpeedTreeRT has overloaded new
	delete m_pSpeedTree;

	NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK,m_physicsRep);

	if (m_Transform.GetParent())
	{
		m_Transform.RemoveFromParent();
	}


}

void RegisterSpeedTreeTexture(const char** textureNames, Texture::Ptr* texture, CSpeedTreeRT::ETextureLayers layer, uint32_t sectorBits)
{
	const char* fileName = textureNames[layer];
	if (!ntStr::IsNull(fileName))
	{
		const char* name = ntStr::GetString( Util::BaseName(fileName) );
		AreaResourceDB::Get().AddAreaResource( name, AreaResource::TEXTURE, sectorBits );
	}
}

void LoadSpeedTreeTexture(const char** textureNames, Texture::Ptr* texture, CSpeedTreeRT::ETextureLayers layer, bool ignoreAbsent = false, const char* templateName = NULL)
{
	const char* fileName = textureNames[layer];
	const char* missingFileName = "speedtree_missing_texture.dds";

	if (ntStr::IsNull(fileName))
	{
		if (ignoreAbsent || CSpeedTreeRT::TL_NORMAL == layer || CSpeedTreeRT::TL_DETAIL == layer)
		{
			// leave the texture NULL to signal to the rendererable that it is not used
			return;
		}
		else
		{
			if (templateName)
			{
				user_warn_msg(("SPEEDTREE : important texture layer %d not specified in %s\n", layer, templateName));
			}
			else
			{
				user_warn_msg(("SPEEDTREE : important texture layer %d not specified\n", layer));
			}
			fileName = missingFileName;
		}
	}

	*texture = TextureManager::Get().LoadTexture_Neutral( ntStr::GetString( Util::BaseName(fileName) ) );
	if (CSpeedTreeRT::TL_NORMAL != layer)
	{
		(*texture) -> m_Platform.GetTexture()->SetGammaCorrect(Gc::kGammaCorrectSrgb);
	}

}

// Stores the pitch from quaternion
void CSpeedTreeWrapper::SetRotation(const CQuat& quat)
{
	//float sinus = 2.f * (quat.X() * quat.Z() - quat.W() * quat.Y());
	//m_rotation = asin(sinus);
	float cosine = - quat.X() * quat.X() - quat.Y() * quat.Y() + quat.Z() * quat.Z() + quat.W() * quat.W();
	m_rotation	= acos(cosine);
}

void CSpeedTreeWrapper::OnAreaLoadStart(const SpeedTreeXmlTree* treeDef)
{
	float pos[3] = {0, 0, 0};
	float dir[3] = {1.f, 0, 0};
	CSpeedTreeRT::SetCamera(pos, dir);

	uint32_t  nSeed		 = 1; //treeDef -> m_seed;

    // generate tree geometry
	bool bRes = m_pSpeedTree->Compute(NULL, nSeed);
	UNUSED(bRes);
	ntError_p(bRes, ("Cannot compute tree geometry: %s", ntStr::GetString(treeDef -> m_templateName)));

	////////////////////////////////////////////////////////////////////

    // get the dimensions
    m_pSpeedTree->GetBoundingBox(m_afBoundingBox);


    // setup the vertex and index buffers
    SetupBuffers( );

    m_pSpeedTree->DeleteBranchGeometry( );
    m_pSpeedTree->DeleteFrondGeometry( );
    m_pSpeedTree->DeleteLeafGeometry( );
	m_pSpeedTree->DeleteTransientData();

}


void CSpeedTreeWrapper::OnAreaLoadEnd(const SpeedTreeXmlTree* treeDef)
{
	// Get the textures
    // load branch textures
	//LoadSpeedTreeTexture(m_pTextureInfo->m_pBranchMaps, &m_textures.m_texBranchTexture, CSpeedTreeRT::TL_DIFFUSE, ntStr::GetString(treeDef -> m_templateName));
	//LoadSpeedTreeTexture(m_pTextureInfo->m_pBranchMaps, &m_textures.m_texBranchNormalTexture, CSpeedTreeRT::TL_NORMAL, ntStr::GetString(treeDef -> m_templateName));
	//LoadSpeedTreeTexture(m_pTextureInfo->m_pBranchMaps, &m_textures.m_texBranchDetailTexture, CSpeedTreeRT::TL_DETAIL, ntStr::GetString(treeDef -> m_templateName));

	//// load composite textures
	//LoadSpeedTreeTexture(m_pTextureInfo->m_pCompositeMaps, &m_textures.m_texCompositeTexture, CSpeedTreeRT::TL_DIFFUSE, ntStr::GetString(treeDef -> m_templateName));
	//LoadSpeedTreeTexture(m_pTextureInfo->m_pCompositeMaps, &m_textures.m_texCompositeNormalTexture, CSpeedTreeRT::TL_NORMAL, ntStr::GetString(treeDef -> m_templateName));

	LoadSpeedTreeTexture(m_pTextureInfo->m_pBranchMaps, &m_textures.m_texBranchTexture, CSpeedTreeRT::TL_DIFFUSE);
	LoadSpeedTreeTexture(m_pTextureInfo->m_pBranchMaps, &m_textures.m_texBranchNormalTexture, CSpeedTreeRT::TL_NORMAL);
	LoadSpeedTreeTexture(m_pTextureInfo->m_pBranchMaps, &m_textures.m_texBranchDetailTexture, CSpeedTreeRT::TL_DETAIL);

	// load composite textures
	LoadSpeedTreeTexture(m_pTextureInfo->m_pCompositeMaps, &m_textures.m_texCompositeTexture, CSpeedTreeRT::TL_DIFFUSE);
	LoadSpeedTreeTexture(m_pTextureInfo->m_pCompositeMaps, &m_textures.m_texCompositeNormalTexture, CSpeedTreeRT::TL_NORMAL);

	// load billboard texture
	LoadSpeedTreeTexture(m_pTextureInfo->m_pBillboardMaps, &m_textures.m_texBillboardTexture, CSpeedTreeRT::TL_DIFFUSE, true);
	LoadSpeedTreeTexture(m_pTextureInfo->m_pBillboardMaps, &m_textures.m_texBillboardNormalTexture, CSpeedTreeRT::TL_NORMAL, true);



}

///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::LoadTree

bool CSpeedTreeWrapper::LoadTree(const ntstd::String& pszSptFile, const SpeedTreeXmlTree* treeDef)
{
	// directx, so allow for flipping of the texture coordinate
    #ifdef WRAPPER_FLIP_T_TEXCOORD
        m_pSpeedTree->SetTextureFlip(true);
    #endif

    // load the tree file
	bool bRes = m_pSpeedTree->LoadTree( pszSptFile.c_str() );
	ntError_p(bRes, ("Cannot load tree: %s Error : %s\n", pszSptFile.c_str(), CSpeedTreeRT::GetCurrentError()));
	if (!bRes)
	{
		return false;
	}

	m_minLODDistance = treeDef -> m_fNearLodDistance;
	m_maxLODDistance = treeDef -> m_fFarLodDistance;

    // override the lighting method stored in the spt file
    m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
    m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
    m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);

    // set the wind method
    m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_GPU);
    m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_GPU);
    m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_GPU);

    m_pSpeedTree->SetNumLeafRockingGroups(1);

	float oldSize, oldVariance;
	m_pSpeedTree->GetTreeSize(oldSize, oldVariance);
	
	m_pSpeedTree -> SetTreeSize(oldSize * SPEEDTREE_SCALE_FACTOR, 0);


    // make the leaves rock in the wind
    m_pSpeedTree->SetLeafRockingState(true);

    // billboard setup
    #ifdef WRAPPER_BILLBOARD_MODE
        CSpeedTreeRT::SetDropToBillboard(true);								
    #else
        CSpeedTreeRT::SetDropToBillboard(false);
    #endif

    //// query & set materials
    //m_materials.m_cBranchMaterial.Set(m_pSpeedTree->GetBranchMaterial( ));
    //m_materials.m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial( ));
    //m_materials.m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial( ));


    // query textures
    m_pTextureInfo = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  CSpeedTreeRT::SMapBank;
    m_pSpeedTree->GetMapBank(*m_pTextureInfo);

	//ntstd::String pathName = Util::DirName(pszSptFile.c_str());

#ifdef SPEEDTREE_COLLECT_STATS
	m_def = const_cast<SpeedTreeXmlTree*>(treeDef);
#endif

    return true;
}

void CSpeedTreeWrapper::RegisterTextures(uint32_t	sectorBits)
{
	RegisterSpeedTreeTexture(m_pTextureInfo->m_pBranchMaps, &m_textures.m_texBranchTexture, CSpeedTreeRT::TL_DIFFUSE, sectorBits);
	RegisterSpeedTreeTexture(m_pTextureInfo->m_pBranchMaps, &m_textures.m_texBranchNormalTexture, CSpeedTreeRT::TL_NORMAL, sectorBits);
	RegisterSpeedTreeTexture(m_pTextureInfo->m_pBranchMaps, &m_textures.m_texBranchDetailTexture, CSpeedTreeRT::TL_DETAIL, sectorBits);

	// register composite textures
	RegisterSpeedTreeTexture(m_pTextureInfo->m_pCompositeMaps, &m_textures.m_texCompositeTexture, CSpeedTreeRT::TL_DIFFUSE, sectorBits);
	RegisterSpeedTreeTexture(m_pTextureInfo->m_pCompositeMaps, &m_textures.m_texCompositeNormalTexture, CSpeedTreeRT::TL_NORMAL, sectorBits);

	// register billboard textures
	RegisterSpeedTreeTexture(m_pTextureInfo->m_pBillboardMaps, &m_textures.m_texBillboardTexture, CSpeedTreeRT::TL_DIFFUSE, sectorBits);
	RegisterSpeedTreeTexture(m_pTextureInfo->m_pBillboardMaps, &m_textures.m_texBillboardNormalTexture, CSpeedTreeRT::TL_NORMAL, sectorBits);

}

void CSpeedTreeWrapper::CreatePhysicsRep()
{
	ntAssert(!m_physicsRep);
	m_physicsRep = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  CSpeedTreePhysicsRep(this);
}

///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBuffers

void CSpeedTreeWrapper::SetupBuffers(void)
{
    // read all the geometry for highest LOD into the geometry cache
    m_pSpeedTree->SetLodLevel(1.0f);
    if (m_pGeometryCache == NULL)
        m_pGeometryCache = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  CSpeedTreeRT::SGeometry;
    m_pSpeedTree->GetGeometry(*m_pGeometryCache);

    // setup the buffers for each tree part
    SetupBranchBuffers( );
    SetupFrondBuffers( );
    SetupLeafBuffers( );
    //SetupBillboardBuffers( );
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::MakeInstance

CSpeedTreeWrapper* CSpeedTreeWrapper::MakeInstance(void)
{
    // create a NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  object
    CSpeedTreeWrapper* pInstance = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  CSpeedTreeWrapper( m_pSpeedTree->MakeInstance(), m_pForest );

    // make an instance of this object's SpeedTree
    if (pInstance->m_pSpeedTree)
    {
		// use the same materials
		pInstance->m_materials = m_materials;
		pInstance->m_textures = m_textures;
		pInstance->m_pTextureInfo = m_pTextureInfo;

		// use the same geometry cache
		pInstance->m_pGeometryCache = m_pGeometryCache;

		// use the same buffers
		pInstance->m_leafBuffers = m_leafBuffers;
		pInstance->m_frondBuffers = m_frondBuffers;
		pInstance->m_branchBuffers = m_branchBuffers;
		// dynamic buffer, only use one per-object
		//pInstance->m_billboardBuffers = SpeedTreeBillboardBuffers::Create();


		// NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  stuff
		NT_MEMCPY(pInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
		pInstance->m_pInstanceOf = this;
		//m_vInstances.push_back(pInstance);
		pInstance->m_collisionGeometryScale = m_collisionGeometryScale;

		pInstance->m_minLODDistance = m_minLODDistance;
		pInstance->m_maxLODDistance = m_maxLODDistance;
    }
    else
    {
        ntPrintf( "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError( ));
        NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK, pInstance );
        pInstance = NULL;
    }

    return pInstance;
}


SpeedTreeStat CSpeedTreeWrapper::GetStat()
{
	return SpeedTreeStat(
		m_renderables[SPEEDTREE_LEAF]?m_renderables[SPEEDTREE_LEAF]->GetTriangleCount():0,
		m_renderables[SPEEDTREE_FROND]?m_renderables[SPEEDTREE_FROND]->GetTriangleCount():0,
		m_renderables[SPEEDTREE_BRANCH]?m_renderables[SPEEDTREE_BRANCH]->GetTriangleCount():0,
		m_renderables[SPEEDTREE_BILLBOARD]?m_renderables[SPEEDTREE_BILLBOARD]->GetTriangleCount():0);
}

unsigned long CSpeedTreeWrapper::GetRenderBitVector()
{
	return m_ulRenderBitVector & m_pForest->GetRenderBitVector();
}

float CSpeedTreeWrapper::GetSize() const
{
	float size = 0;
	float variance = 0;

	m_pSpeedTree -> GetTreeSize(size, variance);

	return size;
}

//inline void ToggleRenderable(RenderableSpeedTree* renderable, bool enable)
//{
//	if (renderable && renderable -> IsRendering() != enable)
//	{
//		renderable -> DisableRendering(!enable);
//	}
//
//}


void CSpeedTreeWrapper::Enable(bool enable)
{
	//ToggleRenderable(m_renderables[SPEEDTREE_BRANCH], enable);
	//ToggleRenderable(m_renderables[SPEEDTREE_FROND], enable);
	//ToggleRenderable(m_renderables[SPEEDTREE_LEAF], enable);
	if (enable) m_ulRenderBitVector = Speedtree_RenderAll;
	else		m_ulRenderBitVector = 0;
}

 
void CSpeedTreeWrapper::SetRenderable(bool visible)
{
	
	const float* pPositionAux = m_pSpeedTree->GetTreePosition();
	CPoint position(pPositionAux[0],pPositionAux[1],pPositionAux[2]);
	m_Transform.SetLocalMatrix(CMatrix(CQuat(CONSTRUCT_IDENTITY),position));

	//m_boundingBox[0] += position.X();
	//m_boundingBox[1] += position.Y();
	//m_boundingBox[2] += position.Z();

	//m_boundingBox[3] += position.X();
	//m_boundingBox[4] += position.Y();
	//m_boundingBox[5] += position.Z();

	if(m_leafBuffers.IsValid())
	{
		m_renderables[SPEEDTREE_LEAF] = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  RenderableSpeedtreeLeaf(this, visible);
	}
	if(m_frondBuffers.IsValid())
	{
		m_renderables[SPEEDTREE_FROND] = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  RenderableSpeedtreeFrond(this, visible);
	}
	if(m_branchBuffers.IsValid())
	{
		m_renderables[SPEEDTREE_BRANCH] = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  RenderableSpeedtreeBranch(this, visible);
	}
//	if(m_billboardBuffers.IsValid())
//	{
////		m_renderables[SPEEDTREE_BILLBOARD] = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  RenderableSpeedtreeBillboard(this);
//	}
	
	
	
	
	// init
	for(int iIndex = 0 ; iIndex < 4 ; ++iIndex )
	{
		if(m_renderables[iIndex])
		{
			m_renderables[iIndex]->Initialise();
			CSector::Get().GetRenderables().AddRenderable(m_renderables[iIndex]);
		}
	}
}













///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupLeafBuffers

void CSpeedTreeWrapper::SetupLeafBuffers(void)
{
	unsigned int numLODLevels = m_pGeometryCache->m_nNumLeafLods < SpeedTree::g_MaxNumLODs ? m_pGeometryCache->m_nNumLeafLods : SpeedTree::g_MaxNumLODs;
	m_leafBuffers.m_usNumLods = numLODLevels;

	const unsigned int numVertsPerLeaf = 4;
	unsigned int numSpeedWindAngles = 1;

	if (m_pForest)
	{
		const CSpeedWind* wind = m_pForest -> GetWind();
		if (wind)
		{
			numSpeedWindAngles = wind -> GetNumLeafAngleMatrices();
		}
	}
    
	unsigned int validLods = 0;
    // check if this tree has branches
    for ( unsigned int lod = 0; lod < numLODLevels; ++ lod)
    {
		CSpeedTreeRT::SGeometry::SLeaf* pLeaves = &m_pGeometryCache->m_pLeaves[lod];

		unsigned int numLeaves = pLeaves->m_nNumLeaves;

		if (numLeaves > 0)
		{

			validLods ++;
			const unsigned int numFields = 6;
			GcStreamField	fields[numFields];
			NT_PLACEMENT_NEW (&fields[0]) GcStreamField( FwHashedString("input.vPosition"), 0, Gc::kFloat, 3 );
			NT_PLACEMENT_NEW (&fields[1]) GcStreamField( FwHashedString("input.vDiffuseTexCoords"), 12, Gc::kFloat, 3 ); // uv + corner index
			NT_PLACEMENT_NEW (&fields[2]) GcStreamField( FwHashedString("input.vWindParams"), 24, Gc::kFloat, 4 );
			NT_PLACEMENT_NEW (&fields[3]) GcStreamField( FwHashedString("input.vDimensions"), 40, Gc::kFloat, 4 );
			NT_PLACEMENT_NEW (&fields[4]) GcStreamField( FwHashedString("input.vAngles"), 56, Gc::kFloat, 4 );
			NT_PLACEMENT_NEW (&fields[5]) GcStreamField( FwHashedString("input.vNormal"), 72, Gc::kFloat, 3 );


			m_leafBuffers.m_pVertexBuffers[lod] = RendererPlatform::CreateVertexStream(	numVertsPerLeaf * numLeaves, 
																		sizeof(SpeedTreeLeafVertex),
																		numFields,
																		fields,
																		Gc::kStaticBuffer );
			unsigned int vbOffset = 0;
			for (unsigned int leaf = 0; leaf < numLeaves; ++ leaf)
			{
				const CSpeedTreeRT::SGeometry::SLeaf::SCard* pCard = pLeaves -> m_pCards + pLeaves -> m_pLeafCardIndices[leaf];

				if (!pCard -> m_pMesh)
				{
					for (unsigned int vert = 0; vert < numVertsPerLeaf; ++ vert)
					{
						SpeedTreeLeafVertex aux(
							vert,
							&pLeaves->m_pCenterCoords[leaf*3],
							&pLeaves->m_pNormals[leaf*12 + vert * 3],
							&pCard->m_pTexCoords[vert * 2],
							pCard->m_fWidth,
							pCard->m_fHeight,
							pCard->m_afPivotPoint,
							pCard->m_afAngleOffsets,
							(float)(leaf % numSpeedWindAngles),
							pLeaves->m_pDimming[leaf],
							(float)pLeaves->m_pWindMatrixIndices[0][leaf],
							pLeaves->m_pWindWeights[0][leaf],
							(float)pLeaves->m_pWindMatrixIndices[1][leaf],
							pLeaves->m_pWindWeights[1][leaf]
							);

						m_leafBuffers.m_pVertexBuffers[lod]->Write( &aux, vbOffset, sizeof(SpeedTreeLeafVertex));
						vbOffset += sizeof(SpeedTreeLeafVertex);
					}
				}
			}
		}

    }

	if (0 == validLods)
	{
		m_leafBuffers.m_usNumLods = 0;
	}

}


template <class BufferType>
void SetupIndexedGeometry(CSpeedTreeRT::SGeometry::SIndexed* geometry, BufferType& buffer)
{
	ntAssert(geometry -> m_nNumLods <= SpeedTree::g_MaxNumLODs);
	unsigned int unNumLodLevels = geometry -> m_nNumLods < SpeedTree::g_MaxNumLODs ? geometry -> m_nNumLods : SpeedTree::g_MaxNumLODs;
	unsigned int unNumIndices = 0;

	unsigned int numStrips[SpeedTree::g_MaxNumLODs];

	int maxStripLen = 0;
    for (unsigned int lod = 0; lod < unNumLodLevels; ++ lod)
    {
		ntAssert(geometry->m_pNumStrips[lod] <= SpeedTree::g_MaxNumStrips);
		numStrips[lod] = geometry->m_pNumStrips[lod] < SpeedTree::g_MaxNumStrips ? geometry->m_pNumStrips[lod] : SpeedTree::g_MaxNumStrips;
		for (unsigned int strip = 0; strip < numStrips[lod]; ++ strip)
		{
			if (geometry -> m_pStripLengths[lod][strip] > maxStripLen)
			{
				maxStripLen = geometry -> m_pStripLengths[lod][strip];
			}
			buffer.m_IndexCounts[lod][strip]	= geometry -> m_pStripLengths[lod][strip];
			unNumIndices						+= geometry -> m_pStripLengths[lod][strip];
		}
    }

	if (0 == unNumIndices)
	{
		return;
	}

#ifdef SPEEDTREE_USE_32BIT_INDICES
	Gc::StreamIndexType indexType = Gc::kIndex32;
	unsigned int indexSize = 4;
#else
	Gc::StreamIndexType indexType = Gc::kIndex16;
	unsigned int indexSize = 2;
	ntAssert(buffer.m_pVertexBuffer -> GetCount() <= 65535);
	uint16_t* tempBuffer = (uint16_t*)alloca(maxStripLen * 2);
#endif

	// allocate a buffer
	buffer.m_pIndexBuffer = RendererPlatform::CreateIndexStream( indexType, unNumIndices, Gc::kStaticBuffer );

	unsigned int offset = 0;
    for (unsigned int lod = 0; lod < unNumLodLevels; ++lod)
    {
		for (unsigned int strip = 0; strip < numStrips[lod]; ++ strip)
		{
#ifndef SPEEDTREE_USE_32BIT_INDICES
			for (unsigned int index = 0; index < buffer.m_IndexCounts[lod][strip]; ++ index)
			{
				tempBuffer[index] = (uint16_t)geometry -> m_pStrips[lod][strip][index];
			}
			buffer.m_pIndexBuffer->Write( tempBuffer, offset, buffer.m_IndexCounts[lod][strip] * indexSize);
#else
			buffer.m_pIndexBuffer->Write( geometry -> m_pStrips[lod][strip], offset, buffer.m_IndexCounts[lod][strip] * indexSize);
#endif
			offset += buffer.m_IndexCounts[lod][strip] * indexSize;
		}
	}

}




///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupFrondBuffers

void CSpeedTreeWrapper::SetupFrondBuffers(void)
{
    CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);

    // check if this tree has branches
    if (pFronds->m_nNumVertices > 1)
    {
		const unsigned int numFields = 4;
		GcStreamField	fields[numFields];
		NT_PLACEMENT_NEW (&fields[0]) GcStreamField( FwHashedString("input.vPosition"), 0, Gc::kFloat, 3 );
		NT_PLACEMENT_NEW (&fields[1]) GcStreamField( FwHashedString("input.vDiffuseTexCoords"), 12, Gc::kFloat, 2 );
		NT_PLACEMENT_NEW (&fields[2]) GcStreamField( FwHashedString("input.vWindParams"), 20, Gc::kFloat, 4 );
		NT_PLACEMENT_NEW (&fields[3]) GcStreamField( FwHashedString("input.vNormal"), 36, Gc::kFloat, 3 );

		m_frondBuffers.m_pVertexBuffer = RendererPlatform::CreateVertexStream(	pFronds->m_nNumVertices, 
																	sizeof(SpeedTreeFrondVertex),
																	numFields,
																	fields,
																	Gc::kStaticBuffer );


        for (unsigned int i = 0; i < m_frondBuffers.m_pVertexBuffer->GetCount(); ++i)
        {
			// ALEXEY_ST
			SpeedTreeFrondVertex aux(
				&pFronds->m_pCoords[i*3],
				&pFronds->m_pNormals[i*3],
				&pFronds->m_pTexCoords[CSpeedTreeRT::TL_DIFFUSE][i * 2],
				(float)pFronds->m_pWindMatrixIndices[0][i],
				(float)pFronds->m_pWindWeights[0][i],
				(float)pFronds->m_pWindMatrixIndices[1][i],
				(float)pFronds->m_pWindWeights[1][i]
				);

			m_frondBuffers.m_pVertexBuffer->Write( &aux, i*sizeof(SpeedTreeFrondVertex), sizeof(SpeedTreeFrondVertex));
		}
 
		SetupIndexedGeometry(pFronds, m_frondBuffers);

    }


	/*
    // reference to frond structure 
    CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);

    // check if this tree has fronds
    if (pFronds->m_usVertexCount > 1)
    {
		const SpeedTreeGameLink& gameLink = SpeedTreeManager::Get().GetLink(SPEEDTREE_FROND);
		m_frondBuffers.m_pVertexBuffer = RendererPlatform::CreateVertexStream(	pFronds->m_usVertexCount, 
																	sizeof(SpeedTreeFrondVertex),
																	gameLink.m_iNbStreamElem,
																	gameLink.m_gcVertexElem.Get(),
																	Gc::kStaticBuffer );
		
		CScopedArray<SpeedTreeFrondVertex> pVertexBuffer(NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  SpeedTreeFrondVertex[m_frondBuffers.m_pVertexBuffer->GetCount()]);
		for (unsigned short i = 0; i < m_frondBuffers.m_pVertexBuffer->GetCount(); ++i)
        {
			pVertexBuffer[i] = SpeedTreeFrondVertex(
			//SpeedTreeFrondVertex aux(
				&(pFronds->m_pCoords[i*3]),
				&(pFronds->m_pNormals[i*3]),
				&(pFronds->m_pTexCoords0[i*2]),
				&(pFronds->m_pTexCoords1[i*2]),
				pFronds->m_pWindMatrixIndices[i],
				pFronds->m_pWindWeights[i]);
			//m_frondBuffers.m_pVertexBuffer->Write(&aux,i*sizeof(SpeedTreeFrondVertex),sizeof(SpeedTreeFrondVertex));
        }
		m_frondBuffers.m_pVertexBuffer->Write(pVertexBuffer.Get());

        // create and fill the index counts for each LOD
        m_frondBuffers.m_unNumLods = m_pSpeedTree->GetNumFrondLodLevels( );
		m_frondBuffers.m_pIndexBuffers.Reset( NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  IBHandle[m_frondBuffers.m_unNumLods] );

		//m_branchBuffers.m_pIndexBuffer->Write( pBranches->m_pStrips[0] );
		for (unsigned int i = 0; i < m_frondBuffers.m_unNumLods; ++i)
        {
            // force update of geometry for this LOD
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, static_cast<short>(i));
			
            // check if this LOD has fronds
			uint32_t iCount = 0;
            if (pFronds->m_usNumStrips > 0)
                iCount = pFronds->m_pStripLengths[0];

			if(iCount > 0)
			{
				m_frondBuffers.m_pIndexBuffers[i] = RendererPlatform::CreateIndexStream(Gc::kIndex16, 
																	iCount,
																	Gc::kStaticBuffer );
				// fill the index buffer for all of them(same one !!!!!!) todo
				m_frondBuffers.m_pIndexBuffers[i]->Write(pFronds->m_pStrips[0]);
			}
        }
        // force update of geometry to highest LOD
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);
    }
	*/
}






///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBranchBuffers

void CSpeedTreeWrapper::SetupBranchBuffers(void)
{
    // reference to the branch structure
    CSpeedTreeRT::SGeometry::SIndexed* pBranches = &(m_pGeometryCache->m_sBranches);

    // check if this tree has branches
    if (pBranches->m_nNumVertices > 1)
    {
		const unsigned int numFields = 8;
		GcStreamField	fields[numFields];
		NT_PLACEMENT_NEW (&fields[0]) GcStreamField( FwHashedString("input.vPosition"), 0, Gc::kFloat, 3 );
		NT_PLACEMENT_NEW (&fields[1]) GcStreamField( FwHashedString("input.vDiffuseTexCoords"), 12, Gc::kFloat, 2 );
		NT_PLACEMENT_NEW (&fields[2]) GcStreamField( FwHashedString("input.vWindParams"), 20, Gc::kFloat, 4 );
		NT_PLACEMENT_NEW (&fields[3]) GcStreamField( FwHashedString("input.vNormal"), 36, Gc::kFloat, 3 );
		NT_PLACEMENT_NEW (&fields[4]) GcStreamField( FwHashedString("input.vBinormal"), 48, Gc::kFloat, 3 );
		NT_PLACEMENT_NEW (&fields[5]) GcStreamField( FwHashedString("input.vTangent"), 60, Gc::kFloat, 3 );
		NT_PLACEMENT_NEW (&fields[6]) GcStreamField( FwHashedString("input.vNormalTexCoords"), 72, Gc::kFloat, 2 );
		NT_PLACEMENT_NEW (&fields[7]) GcStreamField( FwHashedString("input.vDetailTexCoords"), 80, Gc::kFloat, 2 );

		m_branchBuffers.m_pVertexBuffer = RendererPlatform::CreateVertexStream(	pBranches->m_nNumVertices, 
																	sizeof(SpeedTreeBranchVertex),
																	numFields,
																	fields,
																	Gc::kStaticBuffer );


        for (unsigned int i = 0; i < m_branchBuffers.m_pVertexBuffer->GetCount(); ++i)
        {
			// ALEXEY_ST
			SpeedTreeBranchVertex aux(
				&pBranches->m_pCoords[i*3],
				&pBranches->m_pNormals[i*3],
				&pBranches->m_pTexCoords[CSpeedTreeRT::TL_DIFFUSE][i * 2],
				&pBranches->m_pTexCoords[CSpeedTreeRT::TL_NORMAL][i * 2],
				&pBranches->m_pTexCoords[CSpeedTreeRT::TL_DETAIL][i * 2],
				(float)pBranches->m_pWindMatrixIndices[0][i],
				(float)pBranches->m_pWindWeights[0][i],
				(float)pBranches->m_pWindMatrixIndices[1][i],
				(float)pBranches->m_pWindWeights[1][i],
				&pBranches->m_pBinormals[i * 3],
				&pBranches->m_pTangents[i * 3]);
			m_branchBuffers.m_pVertexBuffer->Write( &aux, i*sizeof(SpeedTreeBranchVertex), sizeof(SpeedTreeBranchVertex));
		}
 
		SetupIndexedGeometry(pBranches, m_branchBuffers);

    }
}

///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBranchBuffers

void CSpeedTreeWrapper::SetupBillboardBuffers(void)
{
	//m_billboardBuffers = SpeedTreeBillboardBuffers::Create();

	//const SpeedTreeGameLink& gameLink = SpeedTreeManager::Get().GetLink(SPEEDTREE_BILLBOARD);

	//m_billboardBuffers.m_pVertexBufferForCamera = RendererPlatform::CreateVertexStream(4*3, // 4 vertex, 3 billboard
	//																sizeof(SpeedTreeBillboardVertex),
	//																gameLink.m_iNbStreamElem,
	//																gameLink.m_gcVertexElem.Get(),
	//																Gc::kScratchBuffer ); // dynamicbufferCHANGE

	//m_billboardBuffers.m_pVertexBufferForLight = RendererPlatform::CreateVertexStream(4*3, 
	//																sizeof(SpeedTreeBillboardVertex),
	//																gameLink.m_iNbStreamElem,
	//																gameLink.m_gcVertexElem.Get(),
	//																Gc::kScratchBuffer ); // dynamicbufferCHANGE
}
















///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::Advance

void CSpeedTreeWrapper::UpdateLod()
{
	m_pSpeedTree->ComputeLodLevel();
	for(int iIndex = 0 ; iIndex < 4 ; ++iIndex )
	{
		if(m_renderables[iIndex])
		{
			m_renderables[iIndex]->SetLodCulling();
		}
	}
}


//CSpeedTreeRT::SGeometry* CSpeedTreeWrapper::UpdateGeometryCache(
//	unsigned long ulBitVector,
//	short sOverrideBranchLodValue,
//	short sOverrideFrondLodValue,
//	short sOverrideLeafLodValue)
//{
//	m_pSpeedTree->GetGeometry(*m_pGeometryCache, ulBitVector,sOverrideBranchLodValue,sOverrideFrondLodValue,sOverrideLeafLodValue);
//	return m_pGeometryCache;
//}





///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::CleanUpMemory

void CSpeedTreeWrapper::CleanUpMemory(void)
{
    //if (!m_bIsInstance)
    //    m_pSpeedTree->DeleteTransientData( );
	m_pSpeedTree->DeleteTransientData( );
}

unsigned int CSpeedTreeWrapper::GetVertexFootprint()
{
	unsigned int footprint = 0;
	footprint += m_leafBuffers.GetVertexFootprint();
	footprint += m_frondBuffers.GetVertexFootprint();
	footprint += m_branchBuffers.GetVertexFootprint();

	return footprint;
}

unsigned int CSpeedTreeWrapper::GetIndexFootprint()
{
	unsigned int footprint = 0;
	footprint += m_frondBuffers.GetIndexFootprint();
	footprint += m_branchBuffers.GetIndexFootprint();

	return footprint;
}

unsigned int GetSpecificTextureFootprint(CUniquePtrContainer& texCont, Texture::Ptr texture, const char* fileName)
{
	unsigned int footprint = 0;
	if (texture.IsValid())
	{
		if (!texCont.GetPtr(texture.Get()))
		{
			//footprint = (unsigned int)GcTexture::QueryResourceSizeInBytes(
			//	texture -> GetType(),
			//	texture -> GetMipCount(),
			//	texture -> GetWidth(),
			//	texture -> GetHeight(),
			//	texture -> GetDepth(),
			//	texture -> GetFormat(),
			//	texture -> GetPitch() );
			if (!ntStr::IsNull(fileName))
			{
				if (TextureManager::Get().Loaded_Neutral( ntStr::GetString( Util::BaseName(fileName) ) ) )
				{
					footprint = texture -> CalculateVRAMFootprint();
					texCont.AddPtr(texture.Get());
				}
			}
		}
	}

	return footprint;
}
unsigned int CSpeedTreeWrapper::GetTextureFootprint(CUniquePtrContainer& texCont)
{
	unsigned int footprint = 0;
	footprint += GetSpecificTextureFootprint(texCont, m_textures.m_texBranchDetailTexture, m_pTextureInfo->m_pBranchMaps[CSpeedTreeRT::TL_DETAIL]);
	footprint += GetSpecificTextureFootprint(texCont, m_textures.m_texBranchNormalTexture, m_pTextureInfo->m_pBranchMaps[CSpeedTreeRT::TL_NORMAL]);
	footprint += GetSpecificTextureFootprint(texCont, m_textures.m_texBranchTexture, m_pTextureInfo->m_pBranchMaps[CSpeedTreeRT::TL_DIFFUSE]);
	footprint += GetSpecificTextureFootprint(texCont, m_textures.m_texCompositeNormalTexture, m_pTextureInfo->m_pCompositeMaps[CSpeedTreeRT::TL_NORMAL]);
	footprint += GetSpecificTextureFootprint(texCont, m_textures.m_texCompositeTexture, m_pTextureInfo->m_pCompositeMaps[CSpeedTreeRT::TL_DIFFUSE]);
	footprint += GetSpecificTextureFootprint(texCont, m_textures.m_texBillboardTexture, m_pTextureInfo->m_pBillboardMaps[CSpeedTreeRT::TL_DIFFUSE]);
	footprint += GetSpecificTextureFootprint(texCont, m_textures.m_texBillboardNormalTexture, m_pTextureInfo->m_pBillboardMaps[CSpeedTreeRT::TL_NORMAL]);

	return footprint;
}
