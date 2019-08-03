
#include "blendshapes/BlendShapesComponent.h"
#include "blendshapes/BlendShapes.h"
#include "blendshapes/blendedmeshinstance.h"
#include "blendshapes/anim/BSAnimator.h"
#include "blendshapes/blendshapes_managers.h"

#include "game/renderablecomponent.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "anim/hierarchy.h"
#include "anim/animator.h"
#include "game/luaglobal.h"


//--------------------------------------------------
//
//					Lua Interface
//
//--------------------------------------------------
LUA_EXPOSED_START(BlendShapesComponent)
	// Exposed components
	LUA_EXPOSED_METHOD_GET( BSAnimator, GetBSAnimator_nonconst, "BlendShape animator component. Note that bsanimator may be invalid afer PushSet/PopSet calls" )

	// Exposed methods
	LUA_EXPOSED_METHOD( Enable, Enable, "enables this component", "", "" )
	LUA_EXPOSED_METHOD( Disable, Disable, "disables this component", "", "" )
	LUA_EXPOSED_METHOD( PushSet, PushBSSet, "sets this bsclump as the current blendshapes set. Creates a new BSAnimator", "string bsclumpname", "" )
	LUA_EXPOSED_METHOD( PopSet, PopBSSet, "restores previous blendshapes set. Destroys current BSAnimator", "", "" )

LUA_EXPOSED_END(BlendShapesComponent)



// for the std::vector initial reserve
static const int BS_DEFAULT_NUMBER_OF_BSSETS = 2;


BlendShapesComponent::BlendShapesComponent( CEntity* pParent )
:	BSAnimShortcutContainer(),
//	m_obBSSets(),
	m_pobBSSet( 0 ),
	m_obBSMeshes(),
	m_pEntity( pParent ),
	m_pBSAnimator( 0 ),
	m_bEnabled( true )
{
	ATTACH_LUA_INTERFACE(BlendShapesComponent);

	ntError_p( m_pEntity, (__FUNCTION__"What entity?") );
	ntError_p( m_pEntity->IsBlendShapesCapable(), ("Entity %s is not blendshapes-capable so why install me?", m_pEntity->GetName().c_str()) );

	// [scee_st] now typedefed in the class itself for chunking
	CRenderableComponent::MeshInstanceList& rMeshes  = m_pEntity->GetRenderableComponent()->GetMeshInstances();

	// build our blendshape-capable mesh shortcut list
	for ( CRenderableComponent::MeshInstanceList::iterator it = rMeshes.begin(); it != rMeshes.end(); it++ )
	{
		if ( (*it)->IsShapeBlended() )
		{
			m_obBSMeshes.push_back( reinterpret_cast<BlendedMeshInstance*>(*it) );
		}	
	}
}


BlendShapesComponent::~BlendShapesComponent()
{
	//while ( !m_obBSSets.empty() )
	PopBSSet();

	DestroyBSAnimator();
}

void BlendShapesComponent::Update( float timeStep )
{
	if ( IsEnabled() && m_pBSAnimator )
	{
        m_pBSAnimator->Update( timeStep );
	}

	user_warn_p( m_pBSAnimator, ("Update without valid  BSAnimator\n") );
}


bool BlendShapesComponent::IsCompatible( const BSSetPtr_t pBlendShapes )const
{
	ntAssert( pBlendShapes );

	return ( pBlendShapes->GetClumpHierarchyKey() == m_pEntity->GetHierarchy()->GetHierarchyKey() &&
		pBlendShapes->GetNumOfMeshBSSets() == m_obBSMeshes.size() );
}


void BlendShapesComponent::SetBSSet( BSSetPtr_t pBS )
{
	// no empty sets please...
	if ( !pBS )
	{
		user_warn_msg(("%s: Cannot set an empty Blendshapes set!\n", m_pEntity->GetName().c_str() ) );
		return;
	}

	if ( pBS != GetCurrentBSSet() )
	{
#ifndef _RELEASE
		if ( !IsCompatible( pBS ) )
		{
			uint32_t bsCHash = pBS->GetClumpNameHash();
			uint32_t bsNHash = pBS->GetNameHash();
			uint32_t bsKey = pBS->GetClumpHierarchyKey();
			uint32_t bsNumSets = pBS->GetNumOfMeshBSSets();
			uint32_t entKey = m_pEntity->GetHierarchy()->GetHierarchyKey();
			uint32_t entNumSets = m_obBSMeshes.size();

			user_warn_msg( ("BSCOMPONENT - %s: Cannot set BSSet. Set is incompatible. BSSetName: %i, BSSetClumpName: %i, BSSetHierarchyKey: %i, BSSetNumOfBSMeshes: %i, EntityHierarchyKey %i, EntityNumOfBSMeshes: %i", m_pEntity->GetName().c_str(), bsNHash, bsCHash, bsKey, bsNumSets, entKey, entNumSets ) );
			return;
		}
#endif

		// push the newly created bsset into the set stack
		//m_obBSSets.push_back( pBS );
		m_pobBSSet = pBS;

		// we assume the blended mesh instances are in the same order as the mesh blendshapes
		// note that unique mesh names within the clump were requested but this introduced 
		// a bug in higher level code when people can enable/disable meshes by name
		// I still think relying on the mesh instances push order is wrong as there
		// are exposed methods to remove/insert instances/rendereables in the code but
		// will have to do for now...
		BlendedMeshCollection_t::const_iterator meshIt = m_obBSMeshes.begin();
		for ( u_int iMeshBS = 0; iMeshBS < pBS->GetNumOfMeshBSSets(); ++iMeshBS )
		{
			MeshBSSetPtr_t pMeshBS = pBS->GetMeshBSSetByIndex( iMeshBS );
			(*meshIt)->SetBlendShapes( pMeshBS );
			++meshIt;
		}

		// instance a new animator for the current set
		CreateBSAnimator() ;
	}
	else
	{
		user_warn_msg( (__FUNCTION__"BSSet(%u) is already the current one", pBS->GetNameHash() ) );
	}
}

bool BlendShapesComponent::PushBSSet( const char* fileName )
{
	ntAssert( fileName );

	BSClumpHeaderPtr_t pHeader = BSClumpManager::Get().Load_Neutral(fileName);
	ntError_p( pHeader, ("Entity %s couldn't load bsclump %s\n", m_pEntity->GetName().c_str(), fileName) );

	// push new header if different only if we have no current set, or the set is different from the one being pushed
	if ( !GetCurrentBSSet() || GetCurrentBSSet()->GetNameHash() != pHeader->m_nameHash )
	{
		BSSetPtr_t pBS = NT_NEW_CHUNK(Mem::MC_PROCEDURAL)  BSSet( pHeader, m_pEntity );
		SetBSSet( pBS );
		return true;
	}

	return false;
}


void BlendShapesComponent::PopBSSet( void )
{
	//if ( !m_obBSSets.empty() )
	if ( m_pobBSSet )
	{

		DestroyBSAnimator();
		//m_pBSAnimator->Disable();
		
		// remove blendshapes for every blendedmeshinstance
		for ( BlendedMeshCollection_t::iterator meshIt = m_obBSMeshes.begin(); meshIt != m_obBSMeshes.end(); ++meshIt )
			(*meshIt)->RemoveBlendShapes();

		// remove from current set from our "stack"
		//NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_obBSSets.back() );
		//m_obBSSets.pop_back();
		NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, m_pobBSSet );
		m_pobBSSet = 0;
	}
	else
	{
		user_warn_msg((__FUNCTION__"BlendShapesComponent does not have any BSSets to remove!!!\n"));
	}

	//// now, if we still have a previous set, restore it
	//if ( !m_obBSSets.empty() && m_obBSSets.back() )
	//{
	//	BSSetPtr_t pSet = m_obBSSets.back();
	//	m_obBSSets.pop_back();
	//	SetBSSet( pSet );
	//}
}


BSSetPtr_t	BlendShapesComponent::GetCurrentBSSet( void )
{
	//return m_obBSSets.empty() ? BSSetPtr_t() : m_obBSSets.back();
	return m_pobBSSet;
}	


void BlendShapesComponent::DebugPrint( void )
{
#ifndef _RELEASE
	m_pBSAnimator->DebugPrint();
	m_pEntity->GetAnimator()->DebugPrint();
#endif
}



BSAnimShortcutContainer::BSAnimShortcutCollection::iterator BlendShapesComponent::RemoveBSAnim( BSAnimShortcutCollection::iterator it )
{
	m_pBSAnimator->Stop( it->first );
	return BSAnimShortcutContainer::RemoveBSAnim( it );
}

void BlendShapesComponent::DestroyBSAnimator( void )
{
	if ( m_pBSAnimator )
	{
		m_pBSAnimator->~BSAnimator();
		m_pBSAnimator = 0;
	}
}

void BlendShapesComponent::CreateBSAnimator( void )
{
	DestroyBSAnimator();
	m_pBSAnimator = NT_PLACEMENT_NEW( m_pBSAnimatorMemory ) BSAnimator( m_pEntity, GetCurrentBSSet(), this ) ;
}



//eof

