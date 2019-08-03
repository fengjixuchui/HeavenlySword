//--------------------------------------------------
//!
//!	\file LookAtComponent.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "LookAtComponent.h"
#include "LookAtConstraint.h"
#include "LookAtInfo.h"
#include "objectdatabase/dataobject.h"
#include "game/entity.h"
#include "game/entitymanager.h"
#include "game/query.h"
#include "anim/transform.h"
#include "core/visualdebugger.h"

typedef ntstd::List<CEntity*>			EntList_t;

static const CDirection			DEFAULT_FACING_DIR( 0, 1, 0 );
static const ntstd::String		DEFAULT_TRANSFORM("head");
static const ntstd::String		DEFAULT_TARGETNAME("Player_1");
static const float				DEFAULT_L_ANGLE				= 30.0f;
static const float				DEFAULT_SPEED				= 0.02f;
static const float				DEFAULT_WEIGHT				= 1.0f;
static const float				DEFAULT_FOV					= 45.0f;
static const float				DEFAULT_DISTANCE			= 1.0f;
static const u_int				DEFAULT_DEPTH				= 1;


START_STD_INTERFACE( LookAtCopyRotation )
	PUBLISH_VAR_AS( m_obFrom,			From )
	PUBLISH_VAR_AS( m_obTo,				To )
END_STD_INTERFACE


START_STD_INTERFACE( LookAtComponentDef )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obTransformName,		DEFAULT_TRANSFORM,		Transform )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fFOV,				DEFAULT_FOV,			FOV )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fDistance,			DEFAULT_DISTANCE,		Distance )
	PUBLISH_PTR_CONTAINER_AS(	 m_obConstraintDefs,							Constraints )
	PUBLISH_PTR_CONTAINER_AS(	 m_obCopyRotationNodes,							CopyRotationNodes )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bEnabled,			false,					Enabled )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bIsActive,			false,					Active )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bDebugRender,		true,					DebugRender )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bDebugRenderTarget,	false,					DebugRenderTarget )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bRefreshConstraints,	true,					Refresh )
END_STD_INTERFACE


LUA_EXPOSED_START( LookAtComponent )
	LUA_EXPOSED_METHOD( Enable, Enable, "", "", "" )
	LUA_EXPOSED_METHOD( Disable, Disable, "", "", "" )
	LUA_EXPOSED_METHOD( Reset, Reset, "", "", "" )
	LUA_EXPOSED_METHOD( LookAt, Lua_LookAt, "looks at something", "string entity, string transform", "" )
LUA_EXPOSED_END( LookAtComponent )



const CHashedString LookAtComponent::m_obInvalidTransName = CHashedString("__Invalid__");


LookAtComponent::LookAtComponent( CEntity* pobEntity,  LookAtComponentDef* pobDef )
: m_pobEntity( pobEntity )
, m_pobDef( pobDef )
, m_pobTargetEntity( 0 )
, m_pobTargetTrans( 0 )
, m_iFlags( 0 )
{
	ATTACH_LUA_INTERFACE(LookAtComponent);

	ntError( m_pobEntity );
	ntError( m_pobDef );
	ntAssert( pobEntity->IsCharacter() );

	RefreshConstraintChain();
}


LookAtComponent::~LookAtComponent()
{
	DestroyConstraintChain();
}


const Transform* LookAtComponent::GetTransform( const CEntity* pobEntity, const CHashedString& obTransformName )
{
	ntAssert( pobEntity );

	const Transform* pobTrans = pobEntity->GetTransformP( obTransformName );
	return pobTrans ? pobTrans : pobEntity->GetRootTransformP();	
}


void LookAtComponent::LookAt( const CEntity* pobEntity, const CHashedString& obTransformName )
{
	m_pobTargetEntity = 0;
	m_pobTargetTrans = 0;

	// if we're asked to look at a valid entity...
	if ( pobEntity )
	{
		m_pobTargetEntity = pobEntity;
		if ( obTransformName != m_obInvalidTransName )		// get the preferred transform
		{
			m_pobTargetTrans = GetTransform( m_pobTargetEntity, obTransformName );
		}
		else if ( m_pobTargetEntity->GetLookAtInfo() )		// or the default look-at transform if present
		{
			m_pobTargetTrans = GetTransform( m_pobTargetEntity, CHashedString(m_pobTargetEntity->GetLookAtInfo()->m_obTransformName) );
		}
		else												// or simply the root transform
		{
			m_pobTargetTrans = m_pobTargetEntity->GetRootTransformP();
		}
	}
}


void LookAtComponent::Reset( void )
{
	m_pobTargetEntity = 0;
	m_pobTargetTrans = 0;
	

	// TODO_OZZ: reset constraints
}



void LookAtComponent::DestroyConstraintChain( void )
{
	for ( ConstraintList_t::iterator it = m_obConstraints.begin(); it != m_obConstraints.end(); ++it )
	{
		NT_DELETE( *it );
	}
	m_obConstraints.clear();
}
//--------------------------------------------------
//!
//!	Refreshes the hard LookAtConstraint list
//!	It goes through the soft list in the component
//! definion and creates constraints from these
//! 
//! \note DEBUG only please
//!
//--------------------------------------------------
void LookAtComponent::RefreshConstraintChain( void )
{
	ntError( m_pobEntity );
	ntError( m_pobDef );

	// clear our current constraints
	DestroyConstraintChain();

	
	//Transform* pobTransform = m_pobEntity->GetCharacterTransformP( m_pobDef->m_eBoneID );
	//ntError( pobTransform );

	//! yes, it is a const_cast. I could do GetHierarchy()->GetTransform() to bypass it as well so deal with it!
	Transform* pobTransform = const_cast<Transform*>( m_pobEntity->GetTransformP( CHashedString(m_pobDef->m_obTransformName) ) );
	if ( pobTransform )
	{
		ntstd::List<LookAtConstraintDef*>::const_iterator it = m_pobDef->m_obConstraintDefs.begin();
		// now let's work our way up the transform chain in reverse order and set our constraints
		while ( it != m_pobDef->m_obConstraintDefs.end() && pobTransform )
		{
			m_obConstraints.push_back( NT_NEW LookAtConstraint( pobTransform, *it++ ) );
			pobTransform = pobTransform->GetParent();
		}
	}
}

// TODO_OZZ: needs to scale turn/velociy according to timestep
void LookAtComponent::ApplyConstraintChain( float fTimeStep )
{
	for ( ConstraintList_t::iterator it = m_obConstraints.begin(); it != m_obConstraints.end() && !(*it)->Apply( m_pobTargetTrans, fTimeStep ); ++it );
}


CEntity* LookAtComponent::SelectNewTarget( void  )
{
	ntError( m_pobEntity );

	// our selection query
	CEntityQuery obQuery;

	// Create a segment volume to check within
	CEQCProximitySegment obSeg;
	obSeg.SetRadius( m_pobDef->m_fDistance );
	obSeg.SetAngle( m_pobDef->m_fFOV * DEG_TO_RAD_VALUE );
	obSeg.SetMatrix( m_pobEntity->GetMatrix() );
	obQuery.AddClause( obSeg );
	
	// make sure is lookable
	CEQLookable obLook;
	obQuery.AddClause( obLook );
	
	// but not our owner entity of course
	obQuery.AddExcludedEntity( *m_pobEntity );

	// Find some entities to look at
	CEntityManager::Get().FindEntitiesByType( obQuery, CEntity::EntType_AllButStatic );

	// if we found some lookable entites, see which one is more interesting
	CEntity* pEnt = 0;
	float minDistSq = FLT_MAX;
	if ( !obQuery.GetResults().empty() )
	{
		pEnt = obQuery.GetResults().front();
		for ( QueryResultsContainerType::iterator it = obQuery.GetResults().begin(); it != obQuery.GetResults().end(); ++it )
		{
			float distSq = ( pEnt->GetPosition() - (*it)->GetPosition() ).LengthSquared();
			int priorityDiff = pEnt->GetLookAtInfo()->m_iPriority - (*it)->GetLookAtInfo()->m_iPriority; 
			if ( priorityDiff > 0 )		// has higher(smaller) priority so look at it!
			{
				pEnt = *it;
				minDistSq = distSq;
			}
			else if ( priorityDiff == 0 && distSq < minDistSq )	// well , just as interesting but closer
			{
				pEnt = *it;
				minDistSq = distSq;;
			}
		}
	}		

	return pEnt;
}

void LookAtComponent::Update( float fTimeStep )
{
	if ( m_pobDef->m_bRefreshConstraints )
		RefreshConstraintChain();
	
	if ( IsEnabled() )
	{
		// if component's type is active, search for the most interesting entity to look-at
		if ( m_pobDef->m_bIsActive ) 
		{
			LookAt( SelectNewTarget() );
		}
		
		// apply the constraints in succession 
		ApplyConstraintChain( fTimeStep );

		// finally, process copy rotation nodes with resync
		ProcessCopyRotationNodes( true );
	}
	else
	{
		for ( ConstraintList_t::iterator it = m_obConstraints.begin(); it != m_obConstraints.end(); ++it )
		{
			(*it)->Reset();
		}
	}

	if ( m_pobDef->m_bDebugRender )
		DebugRender();
}

void LookAtComponent::ProcessCopyRotationNodes( bool bResync )
{
	for ( ntstd::List<LookAtCopyRotation*>::const_iterator it = m_pobDef->m_obCopyRotationNodes.begin(); it != m_pobDef->m_obCopyRotationNodes.end(); ++it )
	{
		const Transform* pobFrom = m_pobEntity->GetTransformP((*it)->m_obFrom);
		Transform* pobTo = const_cast<Transform*>( m_pobEntity->GetTransformP((*it)->m_obTo) );
		if ( pobFrom && pobTo )
		{
			CMatrix obRot = pobTo->GetLocalMatrix();
			obRot.SetFromQuat( CQuat(pobFrom->GetLocalMatrix()) );
			pobTo->SetLocalMatrix( obRot );

			if ( bResync )
			{
				pobTo->Resynchronise();
			}				
		}
	}
}

void LookAtComponent::Lua_LookAt( CHashedString obEntityName, CHashedString obTransformName )
{
	CEntity* pEnt = CEntityManager::Get().FindEntity( obEntityName );
	LookAt( pEnt, obTransformName );
}



#include "core/visualdebugger.h"
void LookAtComponent::DebugRender( void )
{	
#ifndef _GOLD_MASTER

	const Transform* pTrans = m_pobEntity->GetTransformP( CHashedString(m_pobDef->m_obTransformName) );
	if ( pTrans )
	{
		const CMatrix& localToWorld = pTrans->GetWorldMatrix();
		const CMatrix worldToLocal = localToWorld.GetAffineInverse();

		u_int colour = IsEnabled() ? DC_GREY : DC_BLACK;
		if ( m_pobTargetTrans && IsEnabled() )
		{
			colour = DC_GREEN;
			if (  m_pobDef->m_bDebugRenderTarget )
			{
				//g_VisualDebug->RenderSphere(m_pobTargetTrans->GetWorldRotation(), m_pobTargetTrans->GetWorldTranslation(), 0.14f, DC_BLUE );
				Transform* pobTransform = const_cast<Transform*>( m_pobEntity->GetTransformP( CHashedString(m_pobDef->m_obTransformName) ) );
				if ( pobTransform )
					g_VisualDebug->RenderLine( pobTransform->GetWorldTranslation() , m_pobTargetTrans->GetWorldTranslation(), DC_GREEN );
			}
		}

		const CDirection facingDir(0,0,1);
		//draw_cone( localToWorld, pTrans->GetWorldTranslation() * worldToLocal, facingDir, m_pobDef->m_fFOV, 0.666f/*m_pobDef->m_fDistance*/, colour );
	}

	for ( ConstraintList_t::iterator it = m_obConstraints.begin(); it != m_obConstraints.end(); ++it )
	{
		(*it)->DebugRender();
	}

#endif
}



//eof


