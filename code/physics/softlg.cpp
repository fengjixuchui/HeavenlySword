//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/softlg.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.10
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"

#include "softlg.h"
#include "gfx/meshinstance.h"

#include "physics/havokincludes.h"
#include "physics/verlet.h"

#include "element.h"
#include "behavior.h"
#include "rigidbody.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/renderablecomponent.h"
#include "core/exportstruct_clump.h"
#include "anim/hierarchy.h"
#include "anim/transform.h"
#include "physics/verletManager.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkvisualize/type/hkColor.h>
#endif

namespace Physics {

	// -  StaticLG ------------------------------------------------------------------------------------------------------------------
	SoftLG::SoftLG( const ntstd::String& p_name, CEntity* p_entity ) :
		LogicGroup( p_name, p_entity ),
		m_pobVerlet(0)
	{ }

	SoftLG::~SoftLG( )
	{
		//NT_DELETE_ARRAY( m_pNormal );
		//NT_DELETE_ARRAY( m_pTexture );
		//NT_DELETE( m_pobVerlet ); Renderable deleted by the Renderer manager
	}

	const LogicGroup::GROUP_TYPE SoftLG::GetType( ) const
	{
		return SOFT_BODY_LG;
	}
		
	void SoftLG::Update( const float p_timestep )
	{ 
		if(m_pobVerlet)
			m_pobVerlet->Update(p_timestep);
	}
		
	void SoftLG::Activate( bool activateInHavok )
	{
#ifdef MBREMOV
#if !defined( _PS3_RUN_WITHOUT_HAVOK_BUILD )
		if((m_pobVerlet == 0) && (VerletManager::Exists()))
		{
			m_pobVerlet = VerletManager::Get().CreateASimpleGenericVerletSystem( m_entity->GetHierarchy()->GetRootTransform(), m_def );
			if(m_pobVerlet) {
				m_pobVerlet->SetupRenderingData();

				// Get the renderable component of the holding entity
				CRenderableComponent* pobRenderable = m_entity->GetRenderableComponent();
				ntAssert( pobRenderable );

				// switch all its meshes off
				pobRenderable->DisableAllRenderables();

				// Give the ownership of this item to the parent renderable
				pobRenderable->AddAddtionalRenderable( m_pobVerlet->GetRenderable() );
			};
		}
		LogicGroup::Activate( activateInHavok );
#endif
#endif
	}

	void SoftLG::Deactivate( )
	{
		/*for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Deactivate( );
		}*/
		LogicGroup::Deactivate();
	}

	RigidBody* SoftLG::AddRigidBody( const BodyCInfo* /*p_info*/ )	
	{
		ntAssert( 0 );
		return 0;
	}

	SoftLG* SoftLG::Construct( CEntity* p_entity)
	{
		ntAssert( 0 );
		return 0;
	}

}
