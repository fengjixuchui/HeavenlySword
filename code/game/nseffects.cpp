//------------------------------------------------------------------------------------------
//!
//!	nsmanager.h
//!
//! Ninja Sequence and Cutscene Manager header
//!
//------------------------------------------------------------------------------------------

#include "game/nsmanager.h"
#include "game/nseffects.h"

#include "camera/camman.h"
#include "camera/camview.h"

#include "anim/hierarchy.h"
#include "anim/transform.h"

#include "effect/psystem_utils.h"
#include "effect/effect_manager.h"

#include "objectdatabase/dataobject.h"

bool NSIndicatorEffects::g_bResourcesLoaded = false;
void* NSIndicatorEffects::g_indicatorDefs[NSIndicatorEffects::NSFX_COUNT];
void* NSIndicatorEffects::g_successDefs[NSIndicatorEffects::NSFX_COUNT];
void* NSIndicatorEffects::g_failureDefs[NSIndicatorEffects::NSFX_COUNT];

//------------------------------------------------------------------------------------------
//!
//!	NSIndicatorEffects ctor / dtor
//!
//------------------------------------------------------------------------------------------
NSIndicatorEffects::NSIndicatorEffects()
{
	Reset();

	for (int i = 0; i < NSFX_COUNT; i++)
		m_effectStatus[i] = NSFX_INACTIVE;

	// build definiton tables
	// Anticipates having specialised effects for each type for button

	if (NSIndicatorEffects::g_bResourcesLoaded == false)
	{
		NSIndicatorEffects::g_bResourcesLoaded = true;

		g_indicatorDefs[NSFX_PRESS_DIR_N]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Direction");
		g_indicatorDefs[NSFX_PRESS_DIR_NE]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Direction");
		g_indicatorDefs[NSFX_PRESS_DIR_E]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Direction");
		g_indicatorDefs[NSFX_PRESS_DIR_SE]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Direction");
		g_indicatorDefs[NSFX_PRESS_DIR_S]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Direction");
		g_indicatorDefs[NSFX_PRESS_DIR_SW]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Direction");
		g_indicatorDefs[NSFX_PRESS_DIR_W]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Direction");
		g_indicatorDefs[NSFX_PRESS_DIR_NW]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Direction");
		g_indicatorDefs[NSFX_COUNTER]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Counter");
		g_indicatorDefs[NSFX_GRAB]			= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Grab");
		g_indicatorDefs[NSFX_ACTION]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Action");
		g_indicatorDefs[NSFX_ATTACK]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Attack");

		g_successDefs[NSFX_PRESS_DIR_N]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_PRESS_DIR_NE]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_PRESS_DIR_E]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_PRESS_DIR_SE]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_PRESS_DIR_S]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_PRESS_DIR_SW]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_PRESS_DIR_W]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_PRESS_DIR_NW]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_COUNTER]			= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_GRAB]			= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_ACTION]			= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		g_successDefs[NSFX_ATTACK]			= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_Fail");
		
		g_failureDefs[NSFX_PRESS_DIR_N]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_PRESS_DIR_NE]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_PRESS_DIR_E]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_PRESS_DIR_SE]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_PRESS_DIR_S]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_PRESS_DIR_SW]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_PRESS_DIR_W]		= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_PRESS_DIR_NW]	= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_COUNTER]			= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_GRAB]			= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_ACTION]			= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
		g_failureDefs[NSFX_ATTACK]			= ObjectDatabase::Get().GetPointerFromName<void*>("NinjaSeq_OK");
	}
}

NSIndicatorEffects::~NSIndicatorEffects()
{
	Reset();
}

//------------------------------------------------------------------------------------------
//!
//!	NSIndicatorEffects
//! Signal all these effects to cleanup ASAP
//!
//------------------------------------------------------------------------------------------
void NSIndicatorEffects::Reset()
{
	for (int i = 0; i < NSFX_COUNT; i++)
	{
		if (m_effectStatus[i] != NSFX_INACTIVE)
			EffectManager::Get().KillEffectNow( m_effectIDs[i] );

		m_effectIDs[i] = 0;
	}

	m_bInWindow = false;
}

//------------------------------------------------------------------------------------------
//!
//!	NSIndicatorEffects
//! Central update of effect manager class
//!
//------------------------------------------------------------------------------------------
void NSIndicatorEffects::Update(	bool bInWindow,
									const ntstd::Vector< NSConditionStorage* >& conditionList )
{
	UNUSED(bInWindow);
	UNUSED(conditionList);

#ifdef USE_PARTICLE_INDICATORS

	// always update our transforms, regardless of window activity
	UpdateTransforms();

	// enter or exit our window
	if ((m_bInWindow == false) && (bInWindow == true))
	{
		// create any effects that may be required
		m_bInWindow = true;
		SetStatus( NSFX_ACTIVE, conditionList );
	}
	else if ((m_bInWindow == true) && (bInWindow == false))
	{
		// signal effects to end when ready
		for (int i = 0; i < NSFX_COUNT; i++)
			SetEffectStatus( (NS_INDICATOR_EFFECT)i, NSFX_INACTIVE );

		m_bInWindow = false;
	}

#endif
}

//------------------------------------------------------------------------------------------
//!
//!	NSIndicatorEffects
//! Central update of effect manager class
//!
//------------------------------------------------------------------------------------------
void NSIndicatorEffects::SetStatus(	NS_EFFECT_STATUS status,
									const ntstd::Vector< NSConditionStorage* >& conditionList )
{
	UNUSED(status);
	UNUSED(conditionList);

#ifdef USE_PARTICLE_INDICATORS

	// dont care if we're not active
	if (m_bInWindow == false)
		return;

	// find effect to create
	for (	NSCondList::const_iterator it = conditionList.begin();
			it != conditionList.end(); ++it )
	{
		NSConditionStorage* pStore = *it;
		if ( pStore->m_eCommand == NSConditionStorage::COM_TEST_INPUT )
		{
			for(	ntstd::Vector<NSCommand>::iterator keyRequred = pStore->m_obCommandInputList.begin();
					keyRequred != pStore->m_obCommandInputList.end(); ++keyRequred )
			{
				switch( (*keyRequred) ) 
				{
				case NSCommand::NS_MOVEUP:		SetEffectStatus( NSFX_PRESS_DIR_N,	status ); break;
				case NSCommand::NS_UPRIGHT:		SetEffectStatus( NSFX_PRESS_DIR_NE,	status ); break;
				case NSCommand::NS_MOVERIGHT:	SetEffectStatus( NSFX_PRESS_DIR_E,	status ); break;
				case NSCommand::NS_DOWNRIGHT:	SetEffectStatus( NSFX_PRESS_DIR_SE,	status ); break;
				case NSCommand::NS_MOVEDOWN:	SetEffectStatus( NSFX_PRESS_DIR_S,	status ); break;
				case NSCommand::NS_DOWNLEFT:	SetEffectStatus( NSFX_PRESS_DIR_SW,	status ); break;
				case NSCommand::NS_MOVELEFT:	SetEffectStatus( NSFX_PRESS_DIR_W,	status ); break;
				case NSCommand::NS_UPLEFT:		SetEffectStatus( NSFX_PRESS_DIR_NW,	status ); break;
				case NSCommand::NS_COUNTER:		SetEffectStatus( NSFX_COUNTER,		status ); break;
				case NSCommand::NS_GRAB:		SetEffectStatus( NSFX_GRAB,			status ); break;
				case NSCommand::NS_ACTION:		SetEffectStatus( NSFX_ACTION,		status ); break;
				case NSCommand::NS_ATTACK:		SetEffectStatus( NSFX_ATTACK,		status ); break;
				}
			}
		}
	}

#endif
}

//------------------------------------------------------------------------------------------
//!
//!	NSIndicatorEffects::SetEffectStatus
//! State transition to new effect
//!
//------------------------------------------------------------------------------------------
void NSIndicatorEffects::SetEffectStatus( NS_INDICATOR_EFFECT type, NS_EFFECT_STATUS status )
{
	ntAssert_p( type < NSFX_COUNT, ("Unrecognised effect type") );
	if (status == m_effectStatus[type])
		return;

	// release old effect as we know its not the same as this one
	if (m_effectStatus[type] != NSFX_INACTIVE)
		EffectManager::Get().KillEffectWhenReady( m_effectIDs[type] );

	// retrieve the definiton of the next one to create, if present
	void* pEffectDef = 0;
	switch (status)
	{
	case NSFX_ACTIVE:		pEffectDef = g_indicatorDefs[type];	break;
	case NSFX_SUCCEEDED:	pEffectDef = g_successDefs[type];	break;
	case NSFX_FAILDED:		pEffectDef = g_failureDefs[type];	break;
	default: break;
	}

	// create it
	if (pEffectDef)
		m_effectIDs[type] = PSystemUtils::ConstructParticleEffect( pEffectDef, &m_effectTransforms[type] );

	m_effectStatus[type] = status;
}

//------------------------------------------------------------------------------------------
//!
//!	NSIndicatorEffects::UpdateTransforms
//! Hard coded table of transforms to attach effects to
//!
//------------------------------------------------------------------------------------------
void NSIndicatorEffects::UpdateTransforms()
{
	ntAssert_p( CamMan::Exists(), ("Must have a camera here for this function to work") );
	ntAssert_p( CEntityManager::Exists(), ("Must have an enity manager for this function to work") );
	
	//ntAssert_p( CEntityManager::Get().GetPlayer(), ("Must have a player here for this function to work") );
	if ( !CEntityManager::Get().GetPlayer() )
		return;

	// retrieve the camera matrix first
	CMatrix camWorldMat = CamMan::Get().GetPrimaryView()->GetViewTransform()->GetWorldMatrix();

	for (int i = 0; i < NSFX_COUNT; i++)
	{
		CMatrix playerMat;

		NS_INDICATOR_EFFECT type = (NS_INDICATOR_EFFECT)i;
		
		switch (type)
		{
		default:
		case NSFX_PRESS_DIR_N:
		case NSFX_PRESS_DIR_NE:
		case NSFX_PRESS_DIR_E:
		case NSFX_PRESS_DIR_SE:
		case NSFX_PRESS_DIR_S:
		case NSFX_PRESS_DIR_SW:
		case NSFX_PRESS_DIR_W:
		case NSFX_PRESS_DIR_NW:
			playerMat = CEntityManager::Get().GetPlayer()->GetHierarchy()->GetTransform( "pelvis" )->GetWorldMatrix();
			break;

		case NSFX_COUNTER:
		case NSFX_GRAB:
		case NSFX_ACTION:
		case NSFX_ATTACK:
			playerMat = CEntityManager::Get().GetPlayer()->GetHierarchy()->GetTransform( "root" )->GetWorldMatrix();
			break;
		}

		// our transforms for the directions are based on the camera, rotated about it's Z
		// 

		CMatrix mat = camWorldMat;
		mat.SetTranslation( playerMat.GetTranslation() );
		m_effectTransforms[i].SetLocalMatrix(mat);
	}
}
