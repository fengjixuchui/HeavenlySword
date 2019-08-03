//**************************************************************************************
//
//	CopyController.cpp.
//
//**************************************************************************************

//**************************************************************************************
//	Include files.
//**************************************************************************************
#include "game/CopyController.h"
#include "game/movement.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entitymanager.h"

#include "objectdatabase/dataobject.h"

#include "anim/animator.h"

//**************************************************************************************
//**************************************************************************************
//	CopyControllerDef implementation.
//**************************************************************************************
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
START_STD_INTERFACE( CopyControllerDef )
	PUBLISH_VAR_AS( m_CopyEntityName, CopyEntityName )
END_STD_INTERFACE

//**************************************************************************************
//	
//**************************************************************************************
MovementController *CopyControllerDef::CreateInstance( CMovement *movement ) const
{
	return NT_NEW_CHUNK(Mem::MC_ENTITY) CopyController( movement, this );
}

//**************************************************************************************
//	
//**************************************************************************************
CopyControllerDef::CopyControllerDef()
{}

//**************************************************************************************
//**************************************************************************************
//	CopyController implementation.
//**************************************************************************************
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
bool CopyController::Update( float time_delta, const CMovementInput &input, const CMovementStateRef &currentState, CMovementState &predictedState )
{
	UNUSED( time_delta );
	UNUSED( input );
	UNUSED( currentState );
	UNUSED( predictedState );

	// If, for some weird reason, we don't have an entity to copy from, just return.
	if ( m_CopyEntity == NULL )
	{
		// If we've got no entity to copy, we may as well just say we're finished; so return true.
		return true;
	}

	//
	//	Flag all our animations as not updated.
	//
	for (	AnimationMap::iterator it = m_Animations.begin();
			it != m_Animations.end();
			++it )
	{
		( *it ).second.m_WasUpdated = false;
	}

	//
	//	Copy the state from the parent entity, marking which anims were updated.
	//
	const CAnimator *copy_animator = m_CopyEntity->GetAnimator_const();

	for ( uint32_t i=0;i<copy_animator->GetNumAnimations();i++ )
	{
		const CAnimationPtr anim = copy_animator->GetAnimation( i );
		float blend_weight = anim->GetBlendWeight();

		CHashedString name( anim->GetShortNameHash() );
		UpdateAnimation( name, blend_weight );
	}

	//
	//	Now go through our animations again. If they haven't been updated
	//	and they're active on the animator then remove them (because this
	//	means that they're no longer active on the copy entity's animator).6
	//
	for (	AnimationMap::iterator it = m_Animations.begin();
			it != m_Animations.end();
			++it )
	{
		if ( !( *it ).second.m_WasUpdated && ( *it ).second.m_Anim->IsActive() )
		{
			m_pobAnimator->RemoveAnimation( ( *it ).second.m_Anim );
		}
	}

	// Keep on copying...
	return false;			// ...not finished yet.
}

//**************************************************************************************
//	
//**************************************************************************************
void CopyController::UpdateAnimation( const CHashedString& shortName, float weight )
{
	CAnimationPtr anim;

	AnimationMap::iterator it = m_Animations.find( shortName.GetValue() );
	if ( it == m_Animations.end() )
	{
		// New animation - create and add it.
		anim = CreateAnimation( shortName );
	}
	else
	{
		// Existing animation - pull it out of the hash-table.
		anim = ( *it ).second.m_Anim;

		// Mark the anim as updated.
		( *it ).second.m_WasUpdated = true;
	}

	ntError( anim != NULL );

	// Now... does this animation exist on our CAnimator already?
	if ( !anim->IsActive() )
	{
		// Not already playing, so add it.
		m_pobAnimator->AddAnimation( anim );
	}

	anim->SetBlendWeight( weight );
}

//**************************************************************************************
//	
//**************************************************************************************
CAnimationPtr CopyController::CreateAnimation( const CHashedString& shortName )
{
	AnimationMap::iterator it = m_Animations.find( shortName.GetValue() );
	ntError_p( it == m_Animations.end(), ("Attempting to insert a duplicate animation.") );
	if ( it != m_Animations.end() )
	{
		// Gah! Already exists - return the existing one.
		return ( *it ).second.m_Anim;
	}

	CAnimationPtr anim = m_pobAnimator->CreateAnimation( shortName );
	m_Animations.insert( AnimationMap::value_type( shortName.GetValue(), UpdateTrackedAnim( anim, true ) ) );

	return anim;
}

//**************************************************************************************
//	
//**************************************************************************************
CopyController::CopyController( CMovement *movement, const CopyControllerDef *def )
:	MovementController	( movement )
,	m_CopyEntity		( NULL )
{
	ntError_p( !def->m_CopyEntityName.IsNull(), ("Invalid entity name!") );

	if ( !def->m_CopyEntityName.IsNull() )
	{
		m_CopyEntity = CEntityManager::Get().FindEntity( def->m_CopyEntityName );
		ntError_p( m_CopyEntity != NULL, ("Entity %s must be created before this controller!", ntStr::GetString(def->m_CopyEntityName)) );
	}
}

//**************************************************************************************
//	
//**************************************************************************************
CopyController::~CopyController()
{}





