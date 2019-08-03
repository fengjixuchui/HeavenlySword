/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "game/renderablecomponent.h"
#include "core/exportstruct_clump.h"
#include "anim/hierarchy.h"
#include "gfx/meshinstance.h"
#include "gfx/sector.h"
#include "gfx/shadowsystem.h"


#include "blendshapes/blendedmeshinstance.h"
#include "blendshapes/blendshapes.h"
#include "blendshapes/xpushapeblending.h"

#include "hair/hairinstance.h"

//--------------------------------------------------
//!
//!	CRenderableComponent::ctor
//!
//--------------------------------------------------
CRenderableComponent::CRenderableComponent( CHierarchy* pobHierarchy,
											bool bRenderOpaque,
											bool bShadowRecieve,
											bool bShadowCast,
											bool bCreatePushBuffers ) :
	m_bProcessingSuspended( true ),
	m_bDisabledByArea( false ),
	m_bDisabledByGame( false ),
	m_pobHierarchy( pobHierarchy )
{
	// add all meshes from this hierarchy
	const CClumpHeader* pobClumpHeader = pobHierarchy->GetClumpHeader();

	for(int iMesh = 0; iMesh < pobClumpHeader->m_iNumberOfMeshes; ++iMesh)
	{
		// get the mesh and transform
		const CMeshHeader* pobMeshHeader = &pobClumpHeader->m_pobMeshHeaderArray[ (uint32_t)iMesh ];
		const Transform* pobTransform = pobHierarchy->GetTransform( pobMeshHeader->m_iTransformIndex );

		if( (pobMeshHeader->m_iFlags & ClumpFlags::IS_HAIR_RENDERABLE) == 0 )
		{
			if ( XPUShapeBlending::Get().IsEnabled() && (pobMeshHeader->m_iFlags & ClumpFlags::IS_BLENDSHAPES_CAPABLE) )
			{
				// blended mesh instances are auto-registered with xpushapeblender upon creation
				m_obMeshes.push_back( NT_NEW_CHUNK( Mem::MC_GFX ) BlendedMeshInstance( pobTransform, pobMeshHeader, bRenderOpaque, bShadowRecieve, bShadowCast ) );
			}
			else 
			{
                m_obMeshes.push_back( NT_NEW_CHUNK( Mem::MC_GFX ) CMeshInstance( pobTransform, pobMeshHeader, bRenderOpaque, bShadowRecieve, bShadowCast, bCreatePushBuffers ) );		
			}

			m_obRenderables.push_back( m_obMeshes.back() );
		}
		// This is HAIR, special CRenderable
		else
		{
			CHairInstance* pHair = NT_NEW_CHUNK( Mem::MC_GFX ) CHairInstance( pobTransform, pobMeshHeader, bRenderOpaque, bShadowRecieve, bShadowCast );

			m_obMeshes.push_back( pHair );
			m_obRenderables.push_back( pHair );
		}
	}

// Helpfully debug to find why the level is sometime pig slow
//	ntPrintf( "Clump : 0x%x Renderables %d\n", pobClumpHeader->m_obClumpName, m_obRenderables.size() );

	AddToLevelRenderables();
}

//--------------------------------------------------
//!
//!	CRenderableComponent::dtor
//!
//--------------------------------------------------
CRenderableComponent::~CRenderableComponent()
{
	if (!m_bProcessingSuspended)
		RemoveFromLevelRenderables();

	while( !m_obRenderables.empty() )
	{
		NT_DELETE_CHUNK( Mem::MC_GFX, m_obRenderables.front() );
		m_obRenderables.pop_front();
	}
}

//--------------------------------------------------
//!
//!	CRenderableComponent::Force the creation of all
//! our push buffers and other area-specific resources. #
//!	May be done on creation of
//! parent renderable component, or after area load.
//!
//--------------------------------------------------
void	CRenderableComponent::CreateAreaResources()
{
	for (	MeshInstanceList::iterator it = m_obMeshes.begin();
			it != m_obMeshes.end(); ++it )
	{
		(*it)->CreateAreaResources();
	}
}

//--------------------------------------------------
//!
//!	CRenderableComponent::Force the deletion of all
//! our push buffers and other area resources. 
//! May be done on dtor of
//! parent renderable component, or before area unload.
//!
//--------------------------------------------------
void	CRenderableComponent::ReleaseAreaResources()
{
	for (	MeshInstanceList::iterator it = m_obMeshes.begin();
			it != m_obMeshes.end(); ++it )
	{
		(*it)->ReleaseAreaResources();
	}
}

//--------------------------------------------------
//!
//!	CRenderableComponent::AddToLevelRenderables
//! Called by sector system when turning on the 
//! visibility of an entity.
//!
//--------------------------------------------------
void CRenderableComponent::AddToLevelRenderables()
{
	ntAssert_p( m_bProcessingSuspended, ("This renderable component is already visible"));

	for( RenderableList::iterator obIt = m_obRenderables.begin(); obIt != m_obRenderables.end(); ++obIt )
		CSector::Get().GetRenderables().AddRenderable( *obIt );

	m_bProcessingSuspended = false;
}

//--------------------------------------------------
//!
//!	CRenderableComponent::RemoveFromLevelRenderables
//! Called by sector system when turning off the 
//! visibility of an entity.
//!
//--------------------------------------------------
void CRenderableComponent::RemoveFromLevelRenderables()
{
	ntAssert_p( !m_bProcessingSuspended, ("This renderable component is not visible"));

	for( RenderableList::const_iterator obIt = m_obRenderables.begin(); obIt != m_obRenderables.end(); ++obIt )
		CSector::Get().GetRenderables().RemoveRenderable( *obIt );

	m_bProcessingSuspended = true;
}

//--------------------------------------------------
//!
//!	CRenderableComponent::AddAddtionalRenderable
//! Add further renderables to component
//!
//--------------------------------------------------
void CRenderableComponent::AddAddtionalRenderable( CRenderable* pobRenderable )
{
	// add to the current level... 
	if (!m_bProcessingSuspended)
		CSector::Get().GetRenderables().AddRenderable( pobRenderable );

	// add the renderable to the list
	m_obRenderables.push_back( pobRenderable );
}

//--------------------------------------------------
//!
//!	CRenderableComponent::EnableAllByTransform
//! switch everything on / off that hangs off this transform
//!
//--------------------------------------------------
void CRenderableComponent::EnableAllByTransform( const Transform* pobTransform, bool bEnable, bool bCheck )
{
	ntError_p( pobTransform, ("Transform MUST be valid here") );

	if( !bCheck )
	{
		for( RenderableList::const_iterator obIt = m_obRenderables.begin(); obIt != m_obRenderables.end(); ++obIt )
		{
			CRenderable* pobRenderable = *obIt;
			if (pobRenderable->GetTransform() == pobTransform)
			{
				pobRenderable->DisableRendering( !bEnable );
				pobRenderable->DisableShadowCasting( !bEnable );
//				pobRenderable->DisableShadowRecieving( !bEnable );
			}
		}
	}
	else
	{
		// this is slower but checks if something is already set
		for( RenderableList::const_iterator obIt = m_obRenderables.begin(); obIt != m_obRenderables.end(); ++obIt )
		{
			CRenderable* pobRenderable = *obIt;
			if (pobRenderable->GetTransform() == pobTransform)
			{
				if( pobRenderable->IsRendering() != bEnable ) pobRenderable->DisableRendering( !bEnable );
				if( pobRenderable->IsShadowCasting() != bEnable ) pobRenderable->DisableShadowCasting( !bEnable );
//				if( pobRenderable->IsShadowRecieving() != bEnable ) pobRenderable->DisableShadowRecieving( !bEnable );
			}
		}
	}
}

//--------------------------------------------------
//!
//!	CRenderableComponent::EnableAllByMeshName
//! switch everything on / off that belongs to this mesh
//!
//--------------------------------------------------
void CRenderableComponent::EnableAllByMeshName( const CHashedString& obName, bool bEnable )
{
	for( MeshInstanceList::const_iterator obIt = m_obMeshes.begin(); obIt != m_obMeshes.end(); ++obIt )		
	{
		// NB we don't early abort, as multiple meshes may have the same name...sigh...
		if (ntStr::GetHashKey((*obIt)->GetMeshHeader()->m_obNameHash) == ntStr::GetHashKey(obName))
		{
			(*obIt)->DisableRendering( !bEnable );
			(*obIt)->DisableShadowCasting( !bEnable );
	//		(*obIt)->DisableShadowRecieving( !bEnable );
		}
	}
}

//--------------------------------------------------
//!
//!	CRenderableComponent::DisableAllRenderables
//! switch everything off
//!
//--------------------------------------------------
void CRenderableComponent::DisableAllRenderables()
{
	for(	RenderableList::const_iterator obIt = m_obRenderables.begin();
			obIt != m_obRenderables.end(); ++obIt )
	{
		(*obIt)->DisableRendering( true );
		(*obIt)->DisableShadowCasting( true );
	//	(*obIt)->DisableShadowRecieving( true );
	}
}


//--------------------------------------------------
//!
//!	CRenderableComponent::EnableAllRenderables
//! switch everything on
//!
//--------------------------------------------------
void CRenderableComponent::EnableAllRenderables()
{
	for(	RenderableList::const_iterator obIt = m_obRenderables.begin();
			obIt != m_obRenderables.end(); ++obIt )
	{
		(*obIt)->DisableRendering( false );
		(*obIt)->DisableShadowCasting( false );
//		(*obIt)->DisableShadowRecieving( false );
	}

}

//--------------------------------------------------
//!
//!	CRenderableComponent::DisableAllShadows
//! switch everything shadow related
//!
//--------------------------------------------------
void CRenderableComponent::DisableAllShadows()
{
	for(	RenderableList::const_iterator obIt = m_obRenderables.begin();
			obIt != m_obRenderables.end(); ++obIt )
	{
		(*obIt)->DisableShadowCasting( true );
	//	(*obIt)->DisableShadowRecieving( true );
	}
}
//--------------------------------------------------
//!
//!	CRenderableComponent::EnableRenderByMeshName
//! enable by mesh name
//!
//--------------------------------------------------
void CRenderableComponent::EnableRenderByMeshName( CHashedString obNameHash, bool bRender )
{
	for( MeshInstanceList::const_iterator obIt = m_obMeshes.begin(); obIt != m_obMeshes.end(); ++obIt )		
	{
		// NB we don't early abort, as multiple meshes may have the same name...sigh...
		CHashedString meshName = (*obIt)->GetMeshHeader()->m_obNameHash;
		if ( meshName == obNameHash)
		{
			//if (obNameHash == CHashedString(0xf18b9520))
			//{
			//	ntBreakpoint();
			//}

			(*obIt)->DisableRendering( !bRender );
	//		(*obIt)->DisableShadowRecieving( !bRender );
		}
	}
}

//--------------------------------------------------
//!
//!	CRenderableComponent::EnableShadowCastByMeshName
//! enable by mesh name
//!
//--------------------------------------------------
void CRenderableComponent::EnableShadowCastByMeshName( CHashedString obNameHash, bool bShadowCast )
{
	for( MeshInstanceList::const_iterator obIt = m_obMeshes.begin(); obIt != m_obMeshes.end(); ++obIt )		
	{
		// NB we don't early abort, as multiple meshes may have the same name...sigh...
		CHashedString meshName = (*obIt)->GetMeshHeader()->m_obNameHash;
		if (meshName == obNameHash)
		{
			//if (obNameHash == CHashedString(0xf18b9520))
			//{
			//	ntBreakpoint();
			//}

			(*obIt)->DisableShadowCasting( !bShadowCast );
		}
	}
}

//--------------------------------------------------
//!
//!	CRenderableComponent::GetMeshByMeshName
//! retrieve a mesh instance by name
//!
//--------------------------------------------------
const CMeshInstance* CRenderableComponent::GetMeshByMeshName( CHashedString obNameHash ) const
{
	for( MeshInstanceList::const_iterator obIt = m_obMeshes.begin(); obIt != m_obMeshes.end(); ++obIt )
	{
		if ((*obIt)->GetMeshHeader()->m_obNameHash == obNameHash)
			return (*obIt);
	}

	user_warn_p(0,("INVALID MESH NAME: %s", ntStr::GetString(obNameHash)));

	return NULL;
}

//--------------------------------------------------
//!
//!	CRenderableComponent::ForceMaterial
//! Overide mesh material
//!
//--------------------------------------------------
void CRenderableComponent::ForceMaterial( CMaterial const* pobMaterial )
{
	for( MeshInstanceList::const_iterator obIt = m_obMeshes.begin(); obIt != m_obMeshes.end(); ++obIt )		
	{
		CMeshInstance* pobMeshInstance = *obIt;
		pobMeshInstance->ForceMaterial( pobMaterial );
	}
}

//--------------------------------------------------
//!
//!	CRenderableComponent::GetRenderable
//! retrieve specific material
//!
//--------------------------------------------------
const CRenderable* CRenderableComponent::GetRenderable (const Transform* pobTransform)
{
	for(RenderableList::iterator obIt=m_obRenderables.begin(); obIt!=m_obRenderables.end(); ++obIt)
	{
		if ((*obIt)->GetTransform()==pobTransform)
		{
			return (*obIt);
		}
	}

	return NULL;
}


//--------------------------------------------------
//!
//!	CRenderableComponent::GetPosition
//! retrieve position of parent heirachy (used by 
//! LOD, maybe that should be changed...)
//!
//--------------------------------------------------
const CPoint &CRenderableComponent::GetPosition() const
{
	return m_pobHierarchy->GetRootTransform()->GetWorldTranslation();
}

//--------------------------------------------------
//!
//!	CRenderableComponent::GetWorldSpaceAABB
//! Get the world space AABB that encompases all renderables.
//!
//--------------------------------------------------
CAABB CRenderableComponent::GetWorldSpaceAABB() const
{
	CAABB obResult(CONSTRUCT_INFINITE_NEGATIVE);

	for(RenderableList::const_iterator obIt=m_obRenderables.begin(); obIt!=m_obRenderables.end(); ++obIt)
	{
		CRenderable* pobRenderable=*obIt;
		CAABB obWorldSpaceAABB(pobRenderable->GetBounds());
		obWorldSpaceAABB.Transform(pobRenderable->GetTransform()->GetWorldMatrix());
		obResult.Union(obWorldSpaceAABB);
	}

	return obResult;
}


//-----------------------------------------------------------------------------------------
//!
//!	CRenderableComponent::AddRemoveAll_AreaSystem
//! Enable or disable the processing of this component from a resource based system.
//!
//-----------------------------------------------------------------------------------------
void CRenderableComponent::AddRemoveAll_AreaSystem( bool bAdd )
{
	// Store the flag
	m_bDisabledByArea = !bAdd;

	// Sort out what we are doing based on the flags 
	SetProcessing();
}

//-----------------------------------------------------------------------------------------
//!
//!	CRenderableComponent::AddRemoveAll_Game
//! Enable or disable the processing of this component from a game based system.
//!
//-----------------------------------------------------------------------------------------
void CRenderableComponent::AddRemoveAll_Game( bool bAdd )
{
	// Store the flag
	m_bDisabledByGame = !bAdd;

	// Sort out what we are doing based on the flags 
	SetProcessing();
}

//-----------------------------------------------------------------------------------------
//!
//!	CRenderableComponent::AddRemoveAll_Game
//! Enable or disable the processing of this component from a game based system.
//!
//-----------------------------------------------------------------------------------------
void CRenderableComponent::SetProcessing( void )
{
	// If we should be being processed but currently we are suspended...
	if	(
		(m_bProcessingSuspended) &&
		(!m_bDisabledByGame && !m_bDisabledByArea)
		)
	{
		AddToLevelRenderables();
		return;
	}

	// Otherwise, if we shouldn't be processed and we currently are being...
	else if (
			(!m_bProcessingSuspended) &&
			(m_bDisabledByGame || m_bDisabledByArea)
			)
	{
		RemoveFromLevelRenderables();
		return;
	}

	//ntError_p( false, "Is this bad?:%d %d %d\n", m_bProcessingSuspended, m_bDisabledByGame, m_bDisabledByArea );
}
