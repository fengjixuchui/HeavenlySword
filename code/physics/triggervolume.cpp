//------------------------------------------------------------------------------------------
//!
//!	\file triggervolume.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "triggervolume.h"
//#include "Physics/havokincludes.h"
#include "Physics/world.h"
#include "game/entity.h"
#include "game/entity.inl"

#include "game/entityinfo.h"
#include "camera/camutils.h"
//#include "timer.h"
//#include "enumlist.h"
#include "game/messagehandler.h"
#include "objectdatabase/dataobject.h"
#include "game/inputcomponent.h" // For OnAction event
#include "editable/enumlist.h" // For OnAction event

#include "game/attackdebugger.h"

// Debug includes
#include "camera/camman_public.h"
#include "core/osddisplay.h"
#include "core/visualdebugger.h"

#include "core/gatso.h"


START_STD_INTERFACE( CTriggerVolume )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bActive, true, Active )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bDebugRender, false, DebugRender )
	PUBLISH_VAR_AS( m_obActivatorKeywords,	ActivatorKeywords )
	PUBLISH_PTR_AS( m_pobShapeDef,			ShapeDef )
	PUBLISH_PTR_AS( m_pobGameEventList,		Events )

	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK	( DebugRender )
END_STD_INTERFACE

#include <hkdynamics/phantom/hkShapePhantom.h>
#include <hkcollide/shape/cylinder/hkCylinderShape.h>
#include <hkcollide/shape/box/hkBoxShape.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include "physics/collisionbitfield.h"
#include "physics/world.h"
#include "physics/maths_tools.h"

//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolume::CTriggerVolume
//!	Construction
//!
//------------------------------------------------------------------------------------------
CTriggerVolume::CTriggerVolume( void ) 
:	m_bActive( true ),
	m_obActivatorKeywords(),
	m_pobShapeDef( 0 ),
	m_pobGameEventList( 0 ),
	m_bDebugRender( false ),
	m_obPrevIntersecting(),
	m_obName(),
	m_bIsActive( true ),
	m_bExpired( false ),
	m_pobMessageHandler( 0 )
{
	m_bNeedsTriggerButtonRender = false;
}


//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolume::~CTriggerVolume
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CTriggerVolume::~CTriggerVolume( void )
{
	// Remove ourselves from the trigger manager
	CTriggerVolumeManager::Get().RemoveTriggerVolume( this );

	// If we have a message handler we need to free it
	if ( m_pobMessageHandler )
		NT_DELETE( m_pobMessageHandler );
}


//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolume::PostConstruct
//!	To be called after serialisation is complete
//!
//------------------------------------------------------------------------------------------
void CTriggerVolume::PostConstruct( void )
{
	// Grab a name from the serialise interface if possible
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
	if ( pDO )
		m_obName = ntStr::GetString(pDO->GetName());

	// Add this trigger volume to the trigger volume manager
	CTriggerVolumeManager::Get().AddTriggerVolume( this );

	// If we have a game event list we need to create a message handler
	if ( m_pobGameEventList )
		m_pobMessageHandler = NT_NEW CMessageHandler( 0, m_pobGameEventList );

	// Reset our data
	Reset();

#if 0

#if 0
	// Create a cylinder
	hkVector4 vertexA(0.0f,  0.0f, 0.0f);
	hkVector4 vertexB(0.0f, 10.0f, 0.0f);
	hkCylinderShape(vertexA, vertexB, 2.0f);
	hkShape* shape1 = HK_NEW hkCylinderShape(vertexA, vertexB, 2.0f);
#endif

#if 1
	// Create a box
//	hkVector4 halfExtent(4.0f, 4.0f, 4.0f);
//	hkShape* shape2 = HK_NEW hkBoxShape(halfExtent);

	// put it in the world
	hkVector4 pos(0.0f, 0.0f, 0.0f, 0.0f);
	hkTransform obLocalTransform( hkQuaternion::getIdentity(), pos );
#endif	
	
	// Collision bits
	hkUint32 iCollide = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
							Physics::CHARACTER_CONTROLLER_ENEMY_BIT		|
							Physics::RAGDOLL_BIT						|
							Physics::SMALL_INTERACTABLE_BIT				|
							Physics::LARGE_INTERACTABLE_BIT				);



	CShapePhantom* pobDef = (CShapePhantomOBB*)m_pobShapeDef;

	if (pobDef->GetType() == CShapePhantom::SHAPE_SPHERE)
	{
		ntPrintf("debug stub: remove this");
	}
	// Call post construct to make sure that the shape is initialised
	m_pobShapeDef->PostConstruct();

	hkQuaternion obOrient;
	hkVector4 obTrans;
	Physics::MathsTools::CQuatTohkQuaternion(CQuat(pobDef->GetLocalMatrix()), obOrient);
	Physics::MathsTools::CPointTohkVector(pobDef->GetPosition(), obTrans);
	hkTransform obTransform(obOrient, obTrans);

	hkSimpleShapePhantom* pobPhantom = HK_NEW hkSimpleShapePhantom(pobDef->GetShape(), obTransform, iCollide);

	// Add to world
	Physics::CPhysicsWorld::Get().GetHavokWorldP()->addPhantom(pobPhantom);
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolume::DebugRender
//!
//------------------------------------------------------------------------------------------
void CTriggerVolume::DebugRender( void )
{
#ifndef _RELEASE

	// Nothing to draw if we don't have a shape
	if ( m_pobShapeDef == 0 )
		return;

	// Set up some colour values
	u_long ulTextColour;
	u_long ulVolumeColour;
	u_long ulFrameColour;

	// Blue for expired
	if ( m_bExpired ) 
	{
		ulTextColour = 0xff7777ff;
		ulVolumeColour = 0x557777ff; 
		ulFrameColour = 0xff7777ff;
	}

	// Red for inactive
	else if (!m_bIsActive)
	{
		ulTextColour = 0xffff7777;
		ulVolumeColour = 0x55ff7777; 
		ulFrameColour = 0xffff7777;
	}

	// Green for active
	else  
	{
		ulTextColour = 0xff77ff77;
		ulVolumeColour = 0x5577ff77; 
		ulFrameColour = 0xff77ff77;
	}

	// Render the shape phantom
	m_pobShapeDef->Render( ulFrameColour, ulVolumeColour ); 

	// Get the location of the shapes centre in screen space
	CPoint obPosition( CONSTRUCT_CLEAR );
	m_pobShapeDef->GetCentre( obPosition );

	g_VisualDebug->Printf3D( obPosition, ulTextColour, DTF_ALIGN_HCENTRE, m_obName.c_str() );

#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolume::EditorChangeValue
//!
//------------------------------------------------------------------------------------------
bool CTriggerVolume::EditorChangeValue( CallBackParameter/*pcItem*/, CallBackParameter /*pcValue*/ )
{
	// Always reset the trigger
	Reset();

	return true;
}

//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolume::SetActive
//!
//------------------------------------------------------------------------------------------
void CTriggerVolume::SetActive( bool bActive )
{
	// NOTE: We really need to define exactly what activating/deactivating a trigger volume 
	// does and how it affects the state of a trigger volume and what variables it resets.
	if ( bActive != m_bIsActive && !m_bExpired )
	{
		m_obPrevIntersecting.clear();
		m_bIsActive = bActive;

		if( !m_bIsActive )
		{
			m_bNeedsTriggerButtonRender = false;
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolume::Reset
//!
//------------------------------------------------------------------------------------------
void CTriggerVolume::Reset ()
{
	// Reset our event handler if we have one
	if ( m_pobMessageHandler )
		m_pobMessageHandler->ResetEventProcessor();

	// Clear our intersections
	m_obPrevIntersecting.clear();

	// Set the active state back to serialised
	m_bIsActive = m_bActive;

	// Set up our keyword value from the serialised string
	m_obKeywords.Set( ntStr::GetString(m_obActivatorKeywords) );

	// Set the expired flag from the message handler
	m_bExpired = !( ( m_pobMessageHandler ) && ( m_pobMessageHandler->ProcessingEventWillFireMessages() ) );
}


//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolume::Update
//!	Check for our OnEnter and OnExit events
//!
//------------------------------------------------------------------------------------------
void CTriggerVolume::Update( float fTimeDelta )
{
	// Oops, we don't have a shapedef for some bizarre reason
	if ( m_pobShapeDef == 0 )
		return;

	// If we don't have a message handler then we are pretty useless
	if ( !m_pobMessageHandler )
		return;


	// Update the message handler
	m_pobMessageHandler->Update( fTimeDelta );


	// If we are active and we are not expired
	if ( ( m_bIsActive ) && !m_bExpired )
	{
		// Figure out what's intersecting this trigger
		ntstd::Set< CEntity* > obNowIntersecting;
		ntstd::List<CEntity*> obIntersecting;

		// Get all the things that are currently intersecting our shape
		m_pobShapeDef->GetIntersecting( obIntersecting );

		// Run through all the things that are currently intersecting
		for ( ntstd::List<CEntity*>::iterator obIntersectIt = obIntersecting.begin(); obIntersectIt != obIntersecting.end(); ++obIntersectIt )
		{
			// Get a direct pointer
			CEntity* pobCollidee = ( *obIntersectIt );

			// After checking for duplicates - add the currently intersecting items to our set
			if ( obNowIntersecting.find( pobCollidee ) == obNowIntersecting.end() )
			{
				obNowIntersecting.insert( pobCollidee );

				// ON ACTION event
				if (pobCollidee->GetInputComponent() && (pobCollidee->GetInputComponent()->GetVPressed() & ( 1 << AB_ACTION )))
				{
					m_pobMessageHandler->ProcessEventWithDefaultReceiver( "OnAction", pobCollidee );
					// Stop rendering cos we've pressed our button
					m_bNeedsTriggerButtonRender = false;
				}
			}
		}


		// Process ON ENTER events
		for( ntstd::Set< CEntity* >::iterator obIt = obNowIntersecting.begin(); obIt != obNowIntersecting.end(); ++obIt ) 
		{
			// Get a direct pointer
			CEntity* pobEntity = ( *obIt );

			// Is there an item in our new set that wasn't in it previously?
			if ( m_obPrevIntersecting.find( pobEntity ) == m_obPrevIntersecting.end() )
			{
				// Check the entity description against in our keywords before processing the event
				if ( pobEntity->GetKeywords().ContainsAny(m_obKeywords) )
				{
					// Process the event - a string because we hope to generalise in the future
					m_pobMessageHandler->ProcessEventWithDefaultReceiver( "OnEnter", pobEntity );
	
					// If I also handle an OnAction, and this entity is the player, display an action sprite
					if ( this->m_pobGameEventList->WillHandleEvent( "OnAction" ) && pobEntity->IsPlayer() )
					{
						// Start rendering button
						m_bNeedsTriggerButtonRender = true;
					}

#ifndef _RELEASE
					// Some debug output if the trigger manager requires it
					if ( CTriggerVolumeManager::Get().DebugRenderEnabled() )
					{
						char acTemp [128];
						sprintf( acTemp,"%s enters %s", pobEntity->GetName().c_str(), m_obName.c_str() );
						OSD::Add( OSD::OBJECTS, 0xff77ff77, acTemp );
					}
#endif // _RELEASE
				}
			}

			// If we did find the item remove it from our list so we will be left with a list of who has left the intersection
			else
			{
				m_obPrevIntersecting.erase( pobEntity );
			}
		}

		// Process ON LEAVE events
		for( ntstd::Set< CEntity* >::iterator obIt = m_obPrevIntersecting.begin(); obIt != m_obPrevIntersecting.end(); ++obIt )
		{
			// Our 'previous' list is now down to only those who have left - check if this is an entity type we care about
			if ( ( *obIt)->GetKeywords() & m_obKeywords )
			{
				// Process the event - a string because we hope to generalise in the future
				m_pobMessageHandler->ProcessEventWithDefaultReceiver( "OnExit", ( *obIt ) );

				//DGF
				// If I also handle an OnAction, and this entity is the player, display an action sprite
				if (this->m_pobGameEventList->WillHandleEvent( "OnAction" ) && (*obIt)->IsPlayer())
				{
					// Stop rendering button
					m_bNeedsTriggerButtonRender = false;
				}

#ifndef _RELEASE
				// Some debug output if the trigger manager requires it
				if ( CTriggerVolumeManager::Get().DebugRenderEnabled() )
				{
					char acTemp [128];
					sprintf( acTemp,"%s leaves %s", ( *obIt)->GetName().c_str(), m_obName.c_str() );
					OSD::Add( OSD::OBJECTS, 0xff77ff77, acTemp );
				}
#endif // _RELEASE
			}
		}

		// Update our previous intersecting list
		m_obPrevIntersecting = obNowIntersecting;

		// Check to see if all the events on this trigger have expired
		m_bExpired = !m_pobMessageHandler->ProcessingEventWillFireMessages();
	}
}







//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager::CTriggerVolumeManager
//!
//------------------------------------------------------------------------------------------

CTriggerVolumeManager::CTriggerVolumeManager ()
{
	m_bDebugRender=false;
}

//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager::~CTriggerVolumeManager
//!
//------------------------------------------------------------------------------------------

CTriggerVolumeManager::~CTriggerVolumeManager ()
{
	Reset();
}

//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager::Reset
//!
//------------------------------------------------------------------------------------------
void CTriggerVolumeManager::Reset ()
{
	m_obTriggerVolumeList.clear();
}


//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager::Update
//!
//------------------------------------------------------------------------------------------
void CTriggerVolumeManager::Update ( float fTimeDelta )
{
	bool bNeedsTriggerButtonRender = false;
	for(ntstd::List<CTriggerVolume*>::iterator obIt=m_obTriggerVolumeList.begin(); obIt!=m_obTriggerVolumeList.end(); ++obIt)
	{
		(*obIt)->Update( fTimeDelta );

		bNeedsTriggerButtonRender |= (*obIt)->m_bNeedsTriggerButtonRender;
	}

	m_bNeedsTriggerButtonRender = bNeedsTriggerButtonRender;

#ifndef _RELEASE

	// Enable or disable OSD

	if (m_bDebugRender)
	{
		// Turn on the OSD if not already enabled
		if (!OSD::IsChannelEnabled(OSD::OBJECTS))
		{
			OSD::EnableChannel(OSD::OBJECTS);
			OSD::Add(OSD::OBJECTS, 0xff77ff77, "Trigger Volume Debugging Enabled");
		}
	}
	else
	{
		// Turn off the OSD if not already disabled
		if (OSD::IsChannelEnabled(OSD::OBJECTS)) 
			OSD::DisableChannel(OSD::OBJECTS);
	}

	// Debug render
	if (m_bDebugRender)
	{
		for(ntstd::List<CTriggerVolume*>::iterator obIt=m_obTriggerVolumeList.begin();
			obIt!=m_obTriggerVolumeList.end();
			++obIt)
		{
			(*obIt)->DebugRender();
		}
	}
	else
	{
		for(ntstd::List<CTriggerVolume*>::iterator obIt=m_obTriggerVolumeList.begin();
			obIt!=m_obTriggerVolumeList.end();
			++obIt)
		{
			if ((*obIt)->m_bDebugRender)
				(*obIt)->DebugRender();
		}
	}

#endif // _RELEASE

}
	
//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager::ActivateTrigger
//!
//------------------------------------------------------------------------------------------

bool CTriggerVolumeManager::ActivateTrigger (const char* pcTriggerName)
{
	CTriggerVolume* pobTriggerVolume=FindTriggerVolume(pcTriggerName);

	if (pobTriggerVolume)
	{
		pobTriggerVolume->SetActive(true);

		return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager::DeactivateTrigger
//!
//------------------------------------------------------------------------------------------

bool CTriggerVolumeManager::DeactivateTrigger (const char* pcTriggerName)
{
	CTriggerVolume* pobTriggerVolume=FindTriggerVolume(pcTriggerName);

	if (pobTriggerVolume)
	{
		pobTriggerVolume->SetActive(false);
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager::AddTriggerVolume
//!
//------------------------------------------------------------------------------------------

void CTriggerVolumeManager::AddTriggerVolume (CTriggerVolume* pobTriggerVolume)
{
	m_obTriggerVolumeList.push_back(pobTriggerVolume);
}

//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager::RemoveTriggerVolume
//!
//------------------------------------------------------------------------------------------

void CTriggerVolumeManager::RemoveTriggerVolume (CTriggerVolume* pobTriggerVolume)
{
	for(ntstd::List<CTriggerVolume*>::iterator obIt=m_obTriggerVolumeList.begin(); obIt!=m_obTriggerVolumeList.end(); ++obIt)
	{
		if ((*obIt)==pobTriggerVolume)
		{
			m_obTriggerVolumeList.erase(obIt);
			return;
		}
	}
}
//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager::FindTriggerVolume
//!
//------------------------------------------------------------------------------------------

CTriggerVolume* CTriggerVolumeManager::FindTriggerVolume (const char* pcTriggerName)
{
	for(ntstd::List<CTriggerVolume*>::iterator obIt=m_obTriggerVolumeList.begin(); obIt!=m_obTriggerVolumeList.end(); ++obIt)
	{
		if ( strcmp((*obIt)->GetName(),pcTriggerName) == 0 )
		{
			return (*obIt);
		}
	}

	return 0;
}
