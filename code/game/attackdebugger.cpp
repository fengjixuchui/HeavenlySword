//------------------------------------------------------------------------------------------
//!
//!	\file attackdebugger.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "attackdebugger.h"
#include "game/attacks.h"
#include "input/inputhardware.h"
#include "input/mouse.h"
#include "game/entity.h"
#include "game/entityboss.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "anim/hierarchy.h"
#include "game/movement.h"
#include "game/movementcontrollerinterface.h"
#include "camera/camman_public.h"
#include "core/visualdebugger.h"
#include "game/awareness.h"
#include "game/hitcounter.h"
#include "game/testmenu.h"
#include "objectdatabase/dataobject.h"
#include "effect/effect_shims.h"
#include "effect/effect_manager.h"
#include "inputcomponent.h"
#include "Physics/advancedcharactercontroller.h"
#include "physics/world.h"
#include "game/shellconfig.h"
#include "game/shelldebug.h"
#include "camera/camutils.h"

#include "physics/shapephantom.h"

#include "superstylesafety.h"

// Grid Settings
#define GRID_COLOUR			( NTCOLOUR_ARGB( 0xff,0xff,0xff,0x77 ) )
#define GRID_ORIGIN_COLOUR	( DC_WHITE )
#define GRID_RADIUS			( 9.0f )
#define GRID_OFFSETY		( 0.1f )

// Protractor settings
#define PROTRACTOR_AXIS_COLOUR		( DC_RED )
#define PROTRACTOR_CIRCLE_COLOUR	( NTCOLOUR_ARGB( 0xff,0xff,0xaa,0x00 ) )
#define PROTRACTOR_OFFSETY			( 0.1f )
#define PROTRACTOR_CIRCLE_COUNT		( 3 )
#define PROTRACTOR_SEGMENTS			( 12 )
static const float PROTRACTOR_RADIUS[PROTRACTOR_CIRCLE_COUNT] = { 2.75f, 5.0f, 8.0f };

// General Output configuration
#define ATTACK_DEBUG_LINE_SPACING ( 12.0f )

// Convert STANCE_TYPE to a String
char* g_apcStanceNameTable[ST_COUNT] = 
{
	"SPEED",
	"POWER",
	"RANGE"
};

// Convert ATTACK_CLASS to a String
char* g_apcClassNameTable[AC_COUNT] = 
{
	"SPEED FAST",
	"SPEED MEDIUM",
	"POWER FAST",
	"POWER MEDIUM",
	"RANGE FAST",
	"RANGE MEDIUM",
	"GRAB GOTO",
	"GRAB HOLD",
	"GRAB STRIKE",
	"EVADE"
};


// Convert int to a String
char* g_apcAttackMoveTable[AM_RANGE_GRAB + 1] = 
{
	"Sf",	
	"Sm",
	"Sg",
	"Pf",	
	"Pm",
	"Pg",
	"Rf",	
	"Rm",
	"Rg",			
};
	

// Convert COMBAT_STATE to a String
char* g_apcCombatStateTable[CS_COUNT] = 
{
	"STANDARD",							
	"ATTACKING",
	"RECOILING",
	"BLOCKING",
	"DEFLECTING",
	"KO",
	"FLOORED", 
	"RISE WAIT",
	"BLOCK STAGGERING",
	"IMPACT STAGGERING",
	"INSTANTRECOVER",
	"HELD",
	"RECOVERING",
	"DYING",
	"DEAD"
};	

char* g_apcRagdollStateTable[(Physics::DEAD)+1] =
{
	"DISABLED",
	"TRANSFORM_TRACKING",
	"TRANSFORM_TRACKING_ANIMATED",
	"ANIMATED",		
	"ANIMATED_ABSOLUTE",
	"DEAD"
};

char* g_apcCharacterControllerTypeTable[(Physics::CharacterController::DUMMY)+1] = 
{
	"FULL_CONTROLLER",
	"RIGID_BODY",
	"RAYCAST_PHANTOM", 
	"DUMMY"
};

// Convert RECOVERY_TYPE to a String
char* g_apcRecoveryTypeTable[RC_COUNT] = 
{
	"STANDARD",
	"RISING",
	"DEFLECT",
	"GRAB"
};

// Convert ATTACK_MOVEMENT_TYPE to a String
char* g_apcMovementTypeTable[AMT_COUNT] = 
{
	"GROUND TO GROUND",
	"GROUND TO AIR",
	"AIR TO AIR",
	"AIR TO GROUND"
};

// Convert REACTION_APPEARANCE to a String
char* g_apcReactionAppearanceTable[RA_COUNT] = 
{
	"SPEED_HIGH_LEFT",
	"SPEED_HIGH_RIGHT",
	"SPEED_LOW_LEFT",
	"SPEED_LOW_RIGHT",
	"SPEED_UP",
	"SPEED_DOWN",
	"POWER_HIGH_LEFT",
	"POWER_HIGH_RIGHT",
	"POWER_LOW_LEFT",
	"POWER_LOW_RIGHT",
	"POWER_UP",
	"POWER_DOWN",
	"RANGE_HIGH_LEFT",
	"RANGE_HIGH_RIGHT",
	"RANGE_LOW_LEFT",
	"RANGE_LOW_RIGHT",
	"RANGE_UP",
	"RANGE_DOWN"
};

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::AttackDebugger
//! Construction
//!
//------------------------------------------------------------------------------------------
AttackDebugger::AttackDebugger( void ):
	m_bRenderProtractors( false ),
	m_bRenderPlayerOneGrid( false ),
	m_bResetPlayerOneGrid( true ),
	m_bShowCurrentAttackState( true ),
	m_bShowCurrentRecoveryState( false ),
	m_bShowCurrentAttackClass( true ),
	m_bShowCurrentStance( true ),
	m_bShowAttackWindows( false ),
	m_bShowHealth( true ),
	m_bShowIncapacityTime( false ),
	m_bShowStyleLevel( true ),
	m_bShowAttackMovementType( false ),
	m_bRenderBigHeads( true ),
	m_bButtonBasherTest( false ),
	m_bRenderTargeting( false ),
	m_bRenderRoots( false ),
	m_bShowStrikeProximityCheckAngle( false ),
	m_bShowEvade( false ),
	m_bAutoCounter( false ),
	m_bRenderSuperSafeVolumes( false ),
	m_bBattleMode( false ),
	m_pobAttackComponents(),
	m_pobDebugComponentOne( 0 ),
	m_pobDebugComponentTwo( 0 ),
	m_obPlayerOneGridMatrix( CONSTRUCT_CLEAR ),
	m_obAttackList()
{
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::~AttackDebugger
//! Destruction
//!
//------------------------------------------------------------------------------------------
AttackDebugger::~AttackDebugger( void )
{
	// We shouldn't have any components registered at this stage - assert if we do
	ntAssert( m_pobAttackComponents.size() == 0 );

	Reset();
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::Reset
//! Cleans out the lists of characters
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::Reset()
{
	// Clear out the list
	m_pobAttackComponents.clear();

	// Clear out the pointers we are using directly
	m_pobDebugComponentOne = 0;
	m_pobDebugComponentTwo = 0;

	ClearAttackLists();
}

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::Update
//! This updates stuff
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::Update( float fTimeChange )
{
#ifndef _GOLD_MASTER

	UNUSED( fTimeChange );

	// Do any static state changes based on this frames input
	if( CInputHardware::Get().GetContext() == INPUT_CONTEXT_COMBAT )
	{
		// See if we need to change our active debug components
		SetDebugComponent( MOUSE_LEFT, m_pobDebugComponentOne );
		SetDebugComponent( MOUSE_RIGHT, m_pobDebugComponentTwo );

		float fYPosition = sfContextTopBorder;

		// Let the users now that we are in combat context
		if (g_ShellOptions->m_bOneHitKills) 
			g_VisualDebug->Printf2D( 5.0f, 395.0f, DC_RED, 0, "One hit kills are on!" );
		fYPosition += sfDebugLineSpacing;

		if (m_bButtonBasherTest)
			g_VisualDebug->Printf2D( 5.0f, 410.0f, DC_GREEN, 0, "Nina button mashing simulator is on!" );
		fYPosition += sfDebugLineSpacing;

		if (m_bAutoCounter)
			g_VisualDebug->Printf2D( 5.0f, 425.0f, DC_GREEN, 0, "Auto counter is on!" );
		fYPosition += sfDebugLineSpacing;

		// Do all our basic rendering tasks based on the static settings
		RenderProtractors();
		RenderGrid();

		// Render the player specific details, making sure not to overlap with the combat test menu
		RenderPlayerDetails( m_pobDebugComponentOne, sfDebugLeftBorder + TestMenu::GetP()->GetMenuWidth() + 100.0f, fYPosition, sfDebugLineSpacing);
		RenderPlayerDetails( m_pobDebugComponentTwo, sfDebugLeftBorder + TestMenu::GetP()->GetMenuWidth() + 550.0f, fYPosition, sfDebugLineSpacing);

		// Render health bars if requested
		RenderHealthBars();

		// Render big heads if necessary
		RenderBigHeads();

		// Loop through our list of components and do any per component stuff
		for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin();
			obIt != m_pobAttackComponents.end(); ++obIt )
		{
			// Boss render
			if ((*obIt)->m_pobParentEntity->IsBoss())
			{
				Boss* pobBoss = (Boss*)(*obIt)->m_pobParentEntity;
				CPoint obScreenPosition( pobBoss->GetPosition() );
				pobBoss->DebugRender(obScreenPosition, -350.0f, 0.0f );
			}

			// Render the root position of this character
			if ( m_bRenderRoots )
				RenderRoot( *obIt );

			if ( m_bRenderSuperSafeVolumes )
			{
				if ( SuperStyleSafetyManager::Exists() )
				{
					SuperStyleSafetyManager::Get().DebugRender();
				}
			}

			if ((*obIt)->m_pobSyncdTransform)
				g_VisualDebug->RenderAxis((*obIt)->m_pobSyncdTransform->GetWorldMatrix(),2.5f);
		}
	}
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::ToggleSuperSafeVolumeDisplay
//! 
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT AttackDebugger::ToggleSuperSafeVolumeDisplay()
{
	m_bRenderSuperSafeVolumes = !m_bRenderSuperSafeVolumes;
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::ToggleGridDisplay
//! 
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT AttackDebugger::ToggleGridDisplay()
{
	m_bRenderPlayerOneGrid = !m_bRenderPlayerOneGrid;
	m_bResetPlayerOneGrid = true;
	return CR_SUCCESS;
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::ToggleAutoCounter
//! 
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT AttackDebugger::ToggleAutoCounter()
{
	m_bAutoCounter = !m_bAutoCounter;
	ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
	for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
	{
		CAttackComponent* pobAttack = const_cast<CAttackComponent*>((*obIt));
		pobAttack->SetAutoCounter(m_bAutoCounter);
	}	
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::ToggleNinaSimulator
//! 
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT AttackDebugger::ToggleNinaSimulator()
		{
			// No longer big heads
			//m_bRenderBigHeads = !m_bRenderBigHeads;
			m_bButtonBasherTest = !m_bButtonBasherTest;

			ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
			for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
			{
				if (!(*obIt)->m_pobParentEntity->IsAI())
				{
					(*obIt)->m_pobParentEntity->GetInputComponent()->SetRandomOutputEnabled(m_bButtonBasherTest);
				}
			}
	return CR_SUCCESS;
		}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::AttackComponentOneBackward
//! 
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT AttackDebugger::AttackComponentOneBackward()
		{
			if (m_pobDebugComponentOne)
			{
				ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.begin();
				ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.end();
				obIt--;

				while (true)
				{
					CAttackComponent* pobAttack = const_cast<CAttackComponent*>((*obIt));
					if (pobAttack == m_pobDebugComponentOne && obIt != obEnd)
					{
						// Get the next one along backwards
						obIt--;
						m_pobDebugComponentOne = (*obIt);
						break;
					}
					else if (obIt == obEnd)
					{
						m_pobDebugComponentOne = 0;
						break;
					}

					obIt--; // Working backwardly
				}	
			}
			else
			{
				ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
				obEnd--;
				m_pobDebugComponentOne = *(obEnd);
			}
	return CR_SUCCESS;
		}
		

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::AttackComponentOneForward
//! 
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT AttackDebugger::AttackComponentOneForward()
		{
			if (m_pobDebugComponentOne)
			{
				ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
				for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
				{
					CAttackComponent* pobAttack = const_cast<CAttackComponent*>((*obIt));
					if (pobAttack == m_pobDebugComponentOne && obIt != obEnd)
					{
						// Get the next one along
						obIt++;
						m_pobDebugComponentOne = (*obIt);
						break;
					}
					else if (obIt == obEnd)
					{
						m_pobDebugComponentOne = 0;
					}
				}	
			}
			else
			{
				m_pobDebugComponentOne = *(m_pobAttackComponents.begin());
			}		
	return CR_SUCCESS;
		}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::AttackComponentTwoBackward
//! 
//!
//------------------------------------------------------------------------------------------

COMMAND_RESULT AttackDebugger::AttackComponentTwoBackward()
		{
			if (m_pobDebugComponentTwo)
			{
				ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.begin();
				ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.end();
				obIt--;

				while (true)
				{
					CAttackComponent* pobAttack = const_cast<CAttackComponent*>((*obIt));
					if (pobAttack == m_pobDebugComponentTwo && obIt != obEnd)
					{
						// Get the next one along backwards
						obIt--;
						m_pobDebugComponentTwo = (*obIt);
						break;
					}
					else if (obIt == obEnd)
					{
						m_pobDebugComponentTwo = 0;
						break;
					}

					obIt--; // Working backwardly
				}	
			}
			else
			{
				ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
				obEnd--;
				m_pobDebugComponentTwo = *(obEnd);
			}
	return CR_SUCCESS;
}
		
//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::AttackComponentTwoForward
//! 
//!
//------------------------------------------------------------------------------------------

COMMAND_RESULT AttackDebugger::AttackComponentTwoForward()
		{
			if (m_pobDebugComponentTwo)
			{
				ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
				for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
				{
					CAttackComponent* pobAttack = const_cast<CAttackComponent*>((*obIt));
					if (pobAttack == m_pobDebugComponentTwo && obIt != obEnd)
					{
						// Get the next one along
						obIt++;
						m_pobDebugComponentTwo = (*obIt);
						break;
					}
					else if (obIt == obEnd)
					{
						m_pobDebugComponentTwo = 0;
					}
				}	
			}
			else
			{
				m_pobDebugComponentTwo = *(m_pobAttackComponents.begin());
			}		
	return CR_SUCCESS;
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::IncrementStyleLevels
//! Moves through the style levels of all the registered attack components
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT AttackDebugger::IncrementStyleLevels( void )
{
	// Loop through our list of components to see if anyone is staggering
	ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
	for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
	{
		// Get a straight pointer to the component
		CAttackComponent* pobComponent = const_cast< CAttackComponent* >( *obIt );

		// Increment the style level
		if (pobComponent->GetHitCounter() )
		{
			// How many hits do we need to add for next style level?
			HIT_LEVEL eCurrentHitLevel = pobComponent->GetHitCounter()->GetCurrentHitLevel();
			HIT_LEVEL eNewHitLevel = eCurrentHitLevel;
			while (eCurrentHitLevel == eNewHitLevel)
			{
				// Just keep adding successful hits till something changes in the hit level
				pobComponent->GetHitCounter()->AddStylePoints(1);
				pobComponent->GetHitCounter()->Update(0.0f);
				eNewHitLevel = pobComponent->GetHitCounter()->GetCurrentHitLevel();

				if (pobComponent->GetHitCounter()->GetCurrentStyleProgressionLevel() == HL_SPECIAL )
				{
					if (eNewHitLevel == HL_SPECIAL)
						break;
				}
				else
				{	
					if (eNewHitLevel == HL_FOUR)
						break;
				}
			}
		}
		else
		{		
			// Just increment our enum
			int h = pobComponent->m_eHitLevel;
			h++;

			// Check the boundaries
			if ( h > 9 )
				h= 0;

			pobComponent->m_eHitLevel = (HIT_LEVEL)h;
		}
	}
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::DecrementStyleLevels
//! Moves through the style levels of all the registered attack components
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT AttackDebugger::DecrementStyleLevels( void )
{
	// Loop through our list of components to see if anyone is staggering
	ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
	for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
	{
		// Get a straight pointer to the component
		CAttackComponent* pobComponent = const_cast< CAttackComponent* >( *obIt );

		// Increment the style level
		if (pobComponent->GetHitCounter() )
		{
			// How many hits do we need to add for next style level?
			pobComponent->GetHitCounter()->SetStylePoints( 0 );
			pobComponent->GetHitCounter()->Update(0.0f);
		}
		else
		{		
			pobComponent->m_eHitLevel = (HIT_LEVEL)0;
		}
	}
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RenderTargeting
//! Shows the targeting details of a character.  All of it.
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RenderTargeting( const CAttackComponent* pobAttackComponent )
{
#ifndef _GOLD_MASTER

	// Drop out of here if we should not be rendering this
	if ( !m_bRenderTargeting )
		return;

	// Make sure we have a good input - otherwise drop out
	if ( !pobAttackComponent )
		return;

	// Make sure we can get the awareness details for this component too
	const AwarenessComponent* pobAwareness = pobAttackComponent->m_pobParentEntity->GetAwarenessComponent();
	if ( !pobAwareness )
		return;

	// Get access to the targeting data for this character - and check its validity
	const CAttackTargetingData* pobTargeting = pobAttackComponent->m_pobParentEntity->GetAwarenessComponent()->m_pobAttackTargetingData;
	if ( !pobTargeting )
		return;
	
	if (pobAttackComponent->m_eCombatState == CS_KO && pobTargeting)
	{
		CPoint obTOne( pobAttackComponent->m_pobParentEntity->GetPosition() );
		CPoint obTTwo( pobAttackComponent->m_pobParentEntity->GetPosition() + pobAttackComponent->m_obKOTargetVector*5 );
		obTOne[1] += 0.1f;
		obTTwo[1] += 0.1f;
		g_VisualDebug->RenderLine(	obTOne, obTTwo, 0xff00ff00 );
		
		if (pobAttackComponent->m_eStruckReactionZone == RZ_FRONT)
		{
			obTOne = pobAttackComponent->m_pobParentEntity->GetPosition() ;
			obTTwo = pobAttackComponent->m_pobParentEntity->GetPosition() + (pobAttackComponent->m_pobParentEntity->GetMatrix().GetZAxis()*-5) ;
			obTOne[1] += 0.1f;
			obTTwo[1] += 0.1f;
			g_VisualDebug->RenderLine(	obTOne, obTTwo, 0xff0000ff );
		}
		else
		{
			obTOne = pobAttackComponent->m_pobParentEntity->GetPosition() ;
			obTTwo = pobAttackComponent->m_pobParentEntity->GetPosition() + (pobAttackComponent->m_pobParentEntity->GetMatrix().GetZAxis()*5) ;
			obTOne[1] += 0.1f;
			obTTwo[1] += 0.1f;
			g_VisualDebug->RenderLine(	obTOne, obTTwo, 0xff0000ff );
		}
	}

	// See if the character is requesting a high enough input speed for directed searches
	bool bDirectionInputMagnitude = ( pobAwareness->m_fMovementMagnitude > pobAwareness->m_pobAttackTargetingData->m_fDirectionLockonMagnitude );

	// We need a matrix to describe our search direction
	CMatrix obLocalMatrix( CONSTRUCT_IDENTITY );
	
	// ...if we are using the facing direction to we simply use the root matrix
	if ( !bDirectionInputMagnitude )
	{
		obLocalMatrix = pobAttackComponent->m_pobParentEntity->GetMatrix();
	}

	// ...otherwise we construct a matrix from the input direction
	else
	{
		// Take our input direction and remove any Y badness
		CDirection obSearchDirection( pobAwareness->m_obMovementDirection.X(), 0.0f, pobAwareness->m_obMovementDirection.Z() );
		obSearchDirection.Normalise();

		// Get our translation
		CPoint obTranslation = pobAttackComponent->m_pobParentEntity->GetPosition();

		// Build a matrix that represents the character were it facing in the pad input direction
		CDirection obXDir = obSearchDirection.Cross( CVecMath::GetYAxis() );
		obXDir.Normalise();

		obLocalMatrix = CMatrix(	obXDir.X(),				0.0f,					obXDir.Z(),				0.0f,
									0.0f,					1.0f,					0.0f,					0.0f,
									obSearchDirection.X(),	0.0f,					obSearchDirection.Z(),	0.0f,
									obTranslation.X(),		obTranslation.Y(),		obTranslation.Z(),		1.0f );
	}

	// Set up our colour set for rendering
	uint32_t dwInactive	= DC_WHITE;
	uint32_t dwActive	= DC_BLUE;
	uint32_t dwSuccess	= DC_RED;

	// We need to tell the awareness component to render how it finds the best match - the need to 
	// be so intrusive should be removed when the targeting is refactored?
	pobAwareness->SetDebugRender();

	// Try to find ourselves the basic lockon target
	const CEntity* pobTarget = pobAwareness->GetTarget( AT_TYPE_ATTACK, pobAttackComponent->m_pobParentEntity );

	// Find the render colour for the next segment to render
	uint32_t dwRenderColour = dwInactive;
	if ( !bDirectionInputMagnitude && pobTarget ) dwRenderColour = dwSuccess;
	else if ( !bDirectionInputMagnitude ) dwRenderColour = dwActive;

	// The inner undirected version
	RenderTargetSegment( obLocalMatrix,	pobTargeting->m_fUpperHeight, 
										pobTargeting->m_fLowerHeight, 
										0.0f,
										pobTargeting->m_fInnerLockonDistance,
										pobTargeting->m_fInnerLockonAngle * DEG_TO_RAD_VALUE,
										dwRenderColour );

	// Ground attacks
	RenderTargetSegment( obLocalMatrix,	pobTargeting->m_fGroundAttackHeightTop, 
										pobTargeting->m_fGroundAttackHeightBottom, 
										pobTargeting->m_fGroundAttackInnerDistance,
										pobTargeting->m_fGroundAttackOuterDistance,
										pobTargeting->m_fGroundAttackAngle * DEG_TO_RAD_VALUE,
										dwRenderColour );

	// Find the render colour for the next segment to render
	dwRenderColour = dwInactive;
	if ( bDirectionInputMagnitude && pobTarget ) dwRenderColour = dwSuccess;
	else if ( bDirectionInputMagnitude ) dwRenderColour = dwActive;

	// Render our four possible targeting areas - the normal directed version
	RenderTargetSegment( obLocalMatrix,	pobTargeting->m_fUpperHeight, 
										pobTargeting->m_fLowerHeight, 
										0.0f,
										pobTargeting->m_fToLockOnDistance,
										pobTargeting->m_fToLockOnAngle * DEG_TO_RAD_VALUE,
										dwRenderColour );

	// If we have no target at this stage - try for another one - at set the render colours
	if ( !bDirectionInputMagnitude ) 
		dwRenderColour = dwInactive;
	else if ( pobTarget )
		dwRenderColour = dwActive;
	else
	{
		pobTarget = pobAwareness->GetTarget( AT_TYPE_MEDIUM_ATTACK, pobAttackComponent->m_pobParentEntity );

		if ( pobTarget )
			dwRenderColour = dwSuccess;
		else
			dwRenderColour = dwActive;
	}

	// The medium range to lockon
	RenderTargetSegment( obLocalMatrix,	pobTargeting->m_fUpperHeight, 
										pobTargeting->m_fLowerHeight, 
										pobTargeting->m_fToLockOnDistance,
										pobTargeting->m_fMediumLockOnDistance,
										pobTargeting->m_fMediumLockOnAngle * DEG_TO_RAD_VALUE,
										dwRenderColour );

	// If we have no target at this stage - try for another one - at set the render colours
	if ( !bDirectionInputMagnitude ) 
		dwRenderColour = dwInactive;
	else if ( pobTarget )
		dwRenderColour = dwActive;
	else
	{
		pobTarget = pobAwareness->GetTarget( AT_TYPE_LONG_ATTACK, pobAttackComponent->m_pobParentEntity );

		if ( pobTarget )
			dwRenderColour = dwSuccess;
		else
			dwRenderColour = dwActive;
	}

	// The long range to lockon
	RenderTargetSegment( obLocalMatrix,	pobTargeting->m_fUpperHeight, 
										pobTargeting->m_fLowerHeight, 
										pobTargeting->m_fMediumLockOnDistance,
										pobTargeting->m_fLongLockOnDistance,
										pobTargeting->m_fLongLockOnAngle * DEG_TO_RAD_VALUE,
										dwRenderColour );

	// If we have found ourselves a target then render it
	if ( pobTarget )
	{
		g_VisualDebug->RenderSphere( CQuat( pobTarget->GetMatrix() ),
											pobTarget->GetPosition(),
											0.4f, 0x88ff0000 );
	}

	// Vuln zones
	for (ntstd::List<SpecificAttackVulnerabilityZone*, Mem::MC_ENTITY>::const_iterator obIt = pobAttackComponent->m_pobAttackDefinition->m_obSpecificVulnerabilities.begin();
		obIt != pobAttackComponent->m_pobAttackDefinition->m_obSpecificVulnerabilities.end();
		obIt++)
	{
		CMatrix obMatrix = pobAttackComponent->m_pobParentEntity->GetMatrix();
		CMatrix obRotY( CONSTRUCT_IDENTITY );
		CCamUtil::MatrixFromEuler_XYZ(obRotY, 0.0f, (*obIt)->m_fZoneAngle * DEG_TO_RAD_VALUE, 0.0f );
		obMatrix = obRotY * obMatrix;

		RenderTargetSegment( obMatrix,	pobTargeting->m_fLowerHeight, 
										pobTargeting->m_fUpperHeight, 
										(*obIt)->m_fInnerDistance,
										(*obIt)->m_fOuterDistance,
										(*obIt)->m_fZoneSweep * DEG_TO_RAD_VALUE,
										DC_CYAN );
	}

	// Make sure we clear the awareness rendering
	pobAwareness->ClearDebugRender();

#endif
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RenderTargetSegment
//! Renders a single targeting segment about the entity.
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RenderTargetSegment(	const CMatrix&	obLocalMatrix, 
											float			fBottom, 
											float 			fTop, 
											float 			fInnerRadius, 
											float 			fOuterRadius, 
											float 			fSweep,
											uint32_t			dwColour)
{
#ifndef _GOLD_MASTER
	// Strip out the translation of the same matrix so we have the orientation
	CMatrix obOrientation = obLocalMatrix;
	obOrientation.SetTranslation( CPoint( CONSTRUCT_CLEAR ) );

	// Draw Arcs at the top and bottom points for both the inner and out radius
	CMatrix obLowerSurface = obLocalMatrix;
	obLowerSurface.SetTranslation( obLowerSurface.GetTranslation() + ( CPoint( 0.0f, fBottom, 0.0f ) * obOrientation ) );
	g_VisualDebug->RenderArc( obLowerSurface, fOuterRadius, fSweep, dwColour );
	g_VisualDebug->RenderArc( obLowerSurface, fInnerRadius, fSweep, dwColour );

	CMatrix obUpperSurface = obLocalMatrix;
	obUpperSurface.SetTranslation( obUpperSurface.GetTranslation() + ( CPoint( 0.0f, fTop, 0.0f ) * obOrientation ) );
	g_VisualDebug->RenderArc( obUpperSurface, fOuterRadius, fSweep, dwColour );
	g_VisualDebug->RenderArc( obUpperSurface, fInnerRadius, fSweep, dwColour );

	// Now join up all the points with some lines - claculate the x and z values from the angles
	float fUnitX = fsinf( fSweep / 2.0f );
	float fUnitZ = fcosf( fSweep / 2.0f );

	// Build four points ( for a single height ) here
	CPoint obPosInner( fUnitX * fInnerRadius,	0.0f,	fUnitZ * fInnerRadius ); 
	CPoint obNegInner( -fUnitX * fInnerRadius,	0.0f,	fUnitZ * fInnerRadius ); 
	CPoint obPosOuter( fUnitX * fOuterRadius,	0.0f,	fUnitZ * fOuterRadius ); 
	CPoint obNegOuter( -fUnitX * fOuterRadius,	0.0f,	fUnitZ * fOuterRadius ); 

	// Draw our eight lines - between the surfaces
	g_VisualDebug->RenderLine( obPosInner * obUpperSurface, obPosInner * obLowerSurface, dwColour );
	g_VisualDebug->RenderLine( obNegInner * obUpperSurface, obNegInner * obLowerSurface, dwColour );
	g_VisualDebug->RenderLine( obPosOuter * obUpperSurface, obPosOuter * obLowerSurface, dwColour );
	g_VisualDebug->RenderLine( obNegOuter * obUpperSurface, obNegOuter * obLowerSurface, dwColour );

	// ...and between the inner and outer arcs
	g_VisualDebug->RenderLine( obPosInner * obUpperSurface, obPosOuter * obUpperSurface, dwColour );
	g_VisualDebug->RenderLine( obNegInner * obUpperSurface, obNegOuter * obUpperSurface, dwColour );
	g_VisualDebug->RenderLine( obPosInner * obLowerSurface, obPosOuter * obLowerSurface, dwColour );
	g_VisualDebug->RenderLine( obNegInner * obLowerSurface, obNegOuter * obLowerSurface, dwColour );		

	// Draw some midpoints too to make things clearer
	CMatrix obMiddleSurface = obLocalMatrix;
	obMiddleSurface.SetTranslation( obMiddleSurface.GetTranslation() + ( CPoint( 0.0f, ( ( fTop + fBottom ) / 2.0f ), 0.0f ) * obOrientation ) );
	g_VisualDebug->RenderArc( obMiddleSurface, fOuterRadius, fSweep, dwColour );
	g_VisualDebug->RenderArc( obMiddleSurface, fInnerRadius, fSweep, dwColour );

	g_VisualDebug->RenderLine( obPosInner * obMiddleSurface, obPosOuter * obMiddleSurface, dwColour );
	g_VisualDebug->RenderLine( obNegInner * obMiddleSurface, obNegOuter * obMiddleSurface, dwColour );
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RenderBigHeads
//! Check the states of the registered characters and render big debug heads if required.
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RenderBigHeads( void )
{
#ifndef _GOLD_MASTER

	// We only do this if the big heads flag is true
	if ( !m_bRenderBigHeads )
		return;

	// Loop through our list of components and do any per component stuff
	ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
	for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
	{
		// Details for checking relevant character states
		float fNeedBigHead = false;
		uint32_t dwBigHeadColour = (uint32_t)DC_WHITE;
		const CAttackComponent* pobAttack = ( *obIt );

		// If we are staggering - big red head
		if ( ( pobAttack->m_eCombatState == CS_BLOCK_STAGGERING ) || ( pobAttack->m_eCombatState == CS_IMPACT_STAGGERING ) )
		{
			fNeedBigHead = true;
			dwBigHeadColour = 0x88ff0000;
		}

		// If we have made an instant recovery - big green head
		else if ( ( pobAttack->m_eCombatState == CS_KO ) && ( pobAttack->m_bQuickRecover ) )
		{
			fNeedBigHead = true;
			dwBigHeadColour = 0x8800ff00;
		}

		// If someone needs a big head - draw it
		if ( fNeedBigHead )
		{
			// Find the head transform of the character
			int iIdx = pobAttack->m_pobParentEntity->GetHierarchy()->GetTransformIndex( CHashedString( "head" ) );
			if (iIdx > -1)
			{
				const CMatrix pobMatrix = pobAttack->m_pobParentEntity->GetHierarchy()->GetTransform( iIdx )->GetWorldMatrix();

				// Give them a big head of the relevant colour
				g_VisualDebug->RenderSphere( CQuat( pobMatrix ), pobMatrix.GetTranslation(), 0.25f, dwBigHeadColour );
			}
		}
	}
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RenderHealthBars
//! Find the first four registered components in our list and render a health bar for them.
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RenderHealthBars( void )
{
#ifndef _GOLD_MASTER

	// If we are in the right mode
	if ( m_bBattleMode )
	{
		// Dimensions of a health bar
		float fHealthBarHalfLength = 0.25f;
		float fHealthBarHeight = 0.05f;
		float fHealthBarOffset = 1.8f;
		uint32_t dwHealthBarColour = NTCOLOUR_ARGB(0xff,0xff,0x99,0x99);

		// Loop through our list of components and render a health bar for each
		ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
		for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
		{
			// Get a direct reference for clarity
			const CAttackComponent* pobComponent = ( *obIt );

			// Get the current heath percentage of the character
			float fHealthPercentage = pobComponent->m_pobParentEntity->ToCharacter()->GetCurrHealth() 
										/ pobComponent->m_pobParentEntity->ToCharacter()->GetStartHealth();

			// Only render if they have some health
			if ( fHealthPercentage > 0.0f )
			{
				// Get the root position of the character we are drawing for in camera space
				CPoint obCharacterRoot = pobComponent->m_pobParentEntity->GetPosition();

				// Get the x axis of the camera so we can line things up
				CDirection obCameraX = CamMan_Public::GetCurrMatrix().GetXAxis();

				// Generate the corner points of the health bar
				CPoint obTopLeft		= obCharacterRoot + CPoint( 0.0f, fHealthBarOffset + fHealthBarHeight, 0.0f ) + CPoint( obCameraX * fHealthBarHalfLength );
				CPoint obTopRight		= obCharacterRoot + CPoint( 0.0f, fHealthBarOffset + fHealthBarHeight, 0.0f ) + CPoint( obCameraX * -fHealthBarHalfLength );
				CPoint obBottomLeft		= obCharacterRoot + CPoint( 0.0f, fHealthBarOffset, 0.0f ) + CPoint( obCameraX * fHealthBarHalfLength );
				CPoint obBottomRight	= obCharacterRoot + CPoint( 0.0f, fHealthBarOffset, 0.0f ) + CPoint( obCameraX * -fHealthBarHalfLength );

				// Draw the outline of the health bar
				g_VisualDebug->RenderLine( obTopLeft, obTopRight, dwHealthBarColour );	
				g_VisualDebug->RenderLine( obTopRight, obBottomRight, dwHealthBarColour );	
				g_VisualDebug->RenderLine( obBottomRight, obBottomLeft, dwHealthBarColour ); 
				g_VisualDebug->RenderLine( obBottomLeft, obTopLeft, dwHealthBarColour ); 

				// Calculate the point along the top of the health bar that corresponds to this health
				CPoint obTopHealthPoint = obTopLeft + CPoint( obCameraX * -( fHealthBarHalfLength * 2.0f * fHealthPercentage ) );

				// Draw the health value into the health bar
				g_VisualDebug->RenderLine( obTopLeft - CPoint( 0.0f, fHealthBarHeight / 2.0f, 0.0f ), obTopHealthPoint - CPoint( 0.0f, fHealthBarHeight / 2.0f, 0.0f ), dwHealthBarColour );	
				g_VisualDebug->RenderLine( obTopHealthPoint, obTopHealthPoint - CPoint( 0.0f, fHealthBarHeight, 0.0f ), dwHealthBarColour );	
			}
		}
	}
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RenderRoot
//! Render the root position of this character
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RenderRoot( const CAttackComponent* pobAttackComponent )
{
#ifndef _GOLD_MASTER

	// Make sure we have sensible input
	if ( !pobAttackComponent )
		return;

	// Make sure we can get an entity pointer
	if ( !pobAttackComponent->m_pobParentEntity )
		return;

	// Get our matrix
	CMatrix obRoot = pobAttackComponent->m_pobParentEntity->GetMatrix();

	// How big do we want our axis
	static const float fAxisLength = 0.3f;

	// Draw the x axis
	g_VisualDebug->RenderLine(	CPoint( 0.0f, 0.0f, 0.0f ) * obRoot, 
										CPoint( fAxisLength, 0.0f, 0.0f ) * obRoot, 
										0xffff0000 );

	// Draw the Y axis
	g_VisualDebug->RenderLine(	CPoint( 0.0f, 0.0f, 0.0f ) * obRoot, 
										CPoint( 0.0f, fAxisLength, 0.0f ) * obRoot, 
										0xff00ff00 );

	// Draw the Z axis
	g_VisualDebug->RenderLine(	CPoint( 0.0f, 0.0f, 0.0f ) * obRoot, 
										CPoint( 0.0f, 0.0f, fAxisLength ) * obRoot, 
										0xff0000ff );

#endif
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RenderPlayerDetails
//! Render all the currently active details for the selected character
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RenderPlayerDetails( const CAttackComponent* pobAttackComponent, float fXOffset, float fYOffset, float fYSpacing )
{
	// Possibly unused in release
	UNUSED( pobAttackComponent );
	UNUSED( fXOffset );
	UNUSED(fYOffset);
	UNUSED(fYSpacing);
#ifndef _RELEASE

	// If we have no attack component we better get out of here
	if ( !pobAttackComponent )
		return;

	CPoint obOne( pobAttackComponent->m_pobParentEntity->GetPosition() );
	CPoint obTwo( pobAttackComponent->m_pobParentEntity->GetPosition() );
	obTwo[1] = 0.0;
	g_VisualDebug->RenderLine(	obOne, obTwo, 0xff0000ff );

	// Render the targeting details for this character
	RenderTargeting( pobAttackComponent );

	// Print out the current attack state if required
	if ( m_bShowCurrentAttackState )
	{
		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "State: %s", g_apcCombatStateTable[pobAttackComponent->m_eCombatState] );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Time:  %.2f", pobAttackComponent->m_fStateTime );
		fYOffset += fYSpacing;
	}

	// Print out the current recovery state if required
	if ( m_bShowCurrentRecoveryState )
	{
		if ( pobAttackComponent->m_eCombatState == CS_RECOVERING )
			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Recovery: %s", g_apcRecoveryTypeTable[pobAttackComponent->m_eRecoveryType] );
		else
			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Recovery: ");

		// Account for our lines
		fYOffset += fYSpacing;
	}

	// Print out the current attack class if required
	if ( m_bShowCurrentAttackClass )
	{
		if ( pobAttackComponent && pobAttackComponent->m_pobMyCurrentStrike )
		{
			g_VisualDebug->Printf2D(	fXOffset, fYOffset, 0xe0ffffff, 0, "Attack: %s",
				ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( pobAttackComponent->m_pobMyCurrentStrike->GetAttackDataP() )));
			fYOffset += fYSpacing;

			g_VisualDebug->Printf2D(	fXOffset, fYOffset, 0xe0ffffff, 0, "Attack link: %s", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( pobAttackComponent->m_obAttackTracker.GetCurrentAttackLinkP() )) );
			fYOffset += fYSpacing;

			g_VisualDebug->Printf2D(	fXOffset, fYOffset, 0xe0ffffff, 0, "        %f, %s", 
											( pobAttackComponent->m_fStateTime / ( pobAttackComponent->m_pobMyCurrentStrike->GetAttackDataP()->GetAttackTime( pobAttackComponent->m_fAttackScalar ) )),
											g_apcClassNameTable[ pobAttackComponent->m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass ] );

			fYOffset += fYSpacing;
		}
		else
		{
			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Attack: ");
			fYOffset += fYSpacing;

			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Attack Link: ");
			fYOffset += fYSpacing;

			fYOffset += fYSpacing;
		}
	}

	// Print out the current stance if required
	if ( m_bShowCurrentStance )
	{
		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Stance: %s", g_apcStanceNameTable[pobAttackComponent->m_eCurrentStance] );

		// Account for our line
		fYOffset += fYSpacing;
	}

	// Print out the style level if required
	if ( m_bShowStyleLevel )
	{
		if (pobAttackComponent->GetHitCounter() )
		{
			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Style Points: %i", pobAttackComponent->GetHitCounter()->m_iStylePoints );

		// Account for our line
			fYOffset += fYSpacing;

			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Style Level: %i", pobAttackComponent->m_eHitLevel );
		// Account for our line
			fYOffset += fYSpacing;
		}
	}

	// Print out the attack movement type if required
	if ( m_bShowAttackMovementType )
	{
		if ( pobAttackComponent->m_pobMyCurrentStrike )
			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Movement Type: %s", g_apcMovementTypeTable[ pobAttackComponent->m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackMovementType ] );

		// Account for our line
		fYOffset += fYSpacing;
	}

	// Print out the attack windows if required
	if ( m_bShowAttackWindows )
	{
		// Render our data
		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Strike: %s", (!pobAttackComponent->IsInSuperStyleSafetyTransition() && pobAttackComponent->IsInStrikeWindow() ) ? "***" : "" );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Attack Pop Out: %s", ( pobAttackComponent->IsInAttackWindow() ) ? "***" : "" );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Move Pop Out: %s", ( pobAttackComponent->IsInMovementWindow() ) ? "***" : "" );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Block Pop Out: %s", ( pobAttackComponent->IsInBlockWindow() ) ? "***" : "" );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Next Move: %s", ( pobAttackComponent->IsInNextMoveWindow() ) ? "***" : "" );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Invulnerable: %s", ( pobAttackComponent->IsInInvulnerabilityWindow() ) ? "***" : "" );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "No Lock: %s", ( pobAttackComponent->IsInNoLockWindow() ) ? "***" : "" );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "No Colide: %s", ( pobAttackComponent->IsInNoCollideWindow() ) ? "***" : "" );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Uninterruptible: %s", ( pobAttackComponent->IsInUninterruptibleWindow() ) ? "***" : "" );
		fYOffset += fYSpacing;	

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Bad counter detect: %s", ( pobAttackComponent->m_bInBadCounterDetectWindow ) ? "***" : "" );
		fYOffset += fYSpacing;
		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Bad counter punishing: %s", ( pobAttackComponent->m_bBadCounterBeingPunished ) ? "***" : "" );
		fYOffset += fYSpacing;

		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Button held timing: %f", pobAttackComponent->m_fAttackButtonTime );
		fYOffset += fYSpacing;
	}

	if (m_bShowStrikeProximityCheckAngle)
	{		
		if (!pobAttackComponent->IsInSuperStyleSafetyTransition() && pobAttackComponent->IsInStrikeWindow()) 
		{
			// Draw the attack proximity check angle
			CDirection obLook(0.0,0.0,1.0);
			CDirection obUp(0.0,1.0,0.0);

			// Draw white line for direction
			CMatrix obMtx(pobAttackComponent->m_pobParentEntity->GetMatrix());
			CMatrix obRot(obUp,(pobAttackComponent->GetAttackStrikeProximityCheckAngle(1) * DEG_TO_RAD_VALUE));
			obMtx = obMtx * obRot;
			obLook = obLook * obMtx;

			CPoint obOne( 0.0f, 0.1f, 0.0f );
			CPoint obTwo = obOne + (obLook*pobAttackComponent->GetAttackStrikeProximityCheckDistance());			
			g_VisualDebug->RenderLine(	obOne + pobAttackComponent->m_pobParentEntity->GetPosition(), 
										obTwo + pobAttackComponent->m_pobParentEntity->GetPosition(), 
											DC_WHITE );
			
			// Draw blue segment for sweep
			obMtx.SetTranslation(pobAttackComponent->m_pobParentEntity->GetPosition());
			RenderTargetSegment(obMtx, 0.1f, 0.5f, 0.0f, pobAttackComponent->GetAttackStrikeProximityCheckDistance(), pobAttackComponent->GetAttackStrikeProximityCheckSweep(1) * DEG_TO_RAD_VALUE ,DC_BLUE);
			RenderTargetSegment(obMtx, 0.1f, 0.5f, 0.0f, pobAttackComponent->GetAttackStrikeProximityCheckExclusionDistance(), pobAttackComponent->GetAttackStrikeProximityCheckSweep(1) * DEG_TO_RAD_VALUE ,DC_BLUE);

			// Text output too
			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Strike1 Current Angle: %f", ( pobAttackComponent->GetAttackStrikeProximityCheckAngle(1) ) );
			fYOffset += fYSpacing;

			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Strike1 Sweep: %f", ( pobAttackComponent->GetAttackStrikeProximityCheckSweep(1) ) );
			fYOffset += fYSpacing;

			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Distance: %f", ( pobAttackComponent->GetAttackStrikeProximityCheckDistance() ) );
			fYOffset += fYSpacing;
		}
		
		if (!pobAttackComponent->IsInSuperStyleSafetyTransition() && pobAttackComponent->IsInStrike2Window()) 
		{
			// Draw the attack proximity check angle
			CDirection obLook(0.0,0.0,1.0);
			CDirection obUp(0.0,1.0,0.0);

			// Draw blue line for direction
			CMatrix obMtx(pobAttackComponent->m_pobParentEntity->GetMatrix());
			CMatrix obRot(obUp,(pobAttackComponent->GetAttackStrikeProximityCheckAngle(2) * DEG_TO_RAD_VALUE));
			obMtx = obMtx * obRot;
			obLook = obLook * obMtx;

			CPoint obOne( 0.0f, 0.1f, 0.0f );
			CPoint obTwo = obOne + (obLook*pobAttackComponent->GetAttackStrikeProximityCheckDistance());			
			g_VisualDebug->RenderLine(	obOne + pobAttackComponent->m_pobParentEntity->GetPosition(), 
										obTwo + pobAttackComponent->m_pobParentEntity->GetPosition(), 
											0xff0000ff );
			
			// Draw red segment for sweep
			obMtx.SetTranslation(pobAttackComponent->m_pobParentEntity->GetPosition());
			RenderTargetSegment(obMtx, 0.1f, 0.5f, 0.0f, pobAttackComponent->GetAttackStrikeProximityCheckDistance(), pobAttackComponent->GetAttackStrikeProximityCheckSweep(2) * DEG_TO_RAD_VALUE ,DC_RED);


			// Text output too
			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Strike2 Current Angle: %f", ( pobAttackComponent->GetAttackStrikeProximityCheckAngle(2) ) );
			fYOffset += fYSpacing;

			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Strike2 Sweep: %f", ( pobAttackComponent->GetAttackStrikeProximityCheckSweep(2) ) );
			fYOffset += fYSpacing;

			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Distance: %f", ( pobAttackComponent->GetAttackStrikeProximityCheckDistance() ) );
			fYOffset += fYSpacing;
		}
	}

	if ( m_bShowEvade )
	{		
		CDirection obEvade(	pobAttackComponent->m_obEvadeDirection.X(),
							0.0f,
							pobAttackComponent->m_obEvadeDirection.Z());
		obEvade.Normalise();

		// Draw evade direction lines
		CPoint obOne(pobAttackComponent->m_pobParentEntity->GetPosition().X(),
					pobAttackComponent->m_pobParentEntity->GetPosition().Y() + 0.1f,
					pobAttackComponent->m_pobParentEntity->GetPosition().Z());

		CPoint obTwo = obOne + (5*obEvade);		
		g_VisualDebug->RenderLine(	obOne, obTwo, DC_WHITE );

		// Draw evade lock on segment
		const CAttackTargetingData* pobTargeting = pobAttackComponent->m_pobParentEntity->GetAwarenessComponent()->m_pobAttackTargetingData;
		RenderTargetSegment(pobAttackComponent->m_pobParentEntity->GetMatrix(), 0.1f, 0.5f, 0.0f, pobTargeting->m_fEvadeLockOnDistance, pobTargeting->m_fEvadeLockOnAngle * DEG_TO_RAD_VALUE , DC_GREEN);
	}

	// Print out the current health if required
	if ( m_bShowHealth )
	{
		g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Health: %.2f", pobAttackComponent->m_pobParentEntity->ToCharacter()->GetCurrHealth() );

		// Account for our lines
		fYOffset += fYSpacing;
	}

	// Print out the current incapacity time if required
	if ( m_bShowIncapacityTime )
	{
		switch ( pobAttackComponent->m_eCombatState )
		{
		case CS_RECOILING:
		case CS_BLOCKING:
		case CS_DEFLECTING:
		case CS_FLOORED: 
		case CS_BLOCK_STAGGERING:
		case CS_IMPACT_STAGGERING:
		// case CS_HELD:
		//	g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Time Out: %.2f", pobAttackComponent->m_fIncapacityTime - pobAttackComponent->m_fStateTime );
		//	break;
		case CS_KO:
			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Time Out: %.2f", pobAttackComponent->m_fIncapacityTime );
			break;
		default:
			g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xe0ffffff, 0, "Time Out: " );
			break;
		}

		// Account for our lines
		fYOffset += fYSpacing;
	}

	// If we are pausing we need to tell the players about the attacks that they can do
	if ( pobAttackComponent->m_pobMyCurrentStrike && (pobAttackComponent->m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_GOTO || pobAttackComponent->m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_HOLD || pobAttackComponent->m_pobMyCurrentStrike->GetAttackDataP()->m_eAttackClass == AC_GRAB_STRIKE) )
	{
		ClearAttackLists();
		FillAttackLists( pobAttackComponent->m_obAttackTracker.GetCurrentAttackLinkP() );

		// Actually there should only ever be a single possible String now - check
		//assert( m_obAttackList.size() == 1 );
		
		if (m_obAttackList.size() == 1) // Lost the assert to prevent a crash
		{
			// Loop through all the available strings and write them to the screen
			attackList::iterator obEnd = m_obAttackList.end();
			for( attackList::iterator obIt = m_obAttackList.begin(); obIt != obEnd; ++obIt )
			{
				// Make sure we have some attacks in this particular String - we should have
				if ( ( *obIt )->size() > 0 )
				{
					// Create a String to fill out with the attack data
					ntstd::String obThisString;

					// Loop through all the attack types in this String
					moveList::iterator obSubEnd = ( *obIt )->end();
					for( moveList::iterator obSubIt = ( *obIt )->begin(); obSubIt != obSubEnd; ++obSubIt )
					{
						// If the String is not empty then add some formatting stuff
						if ( obThisString.length() != 0 )
							obThisString.append( ", " );

						// Now append the text version of the attack type
						obThisString.append( g_apcAttackMoveTable[ ( *obSubIt ) ] );
					}

					// Write out the String to the screen
					g_VisualDebug->Printf2D( fXOffset, fYOffset, 0xffff0f0f, 0, obThisString.c_str() );

					// Account for our lines
					fYOffset += fYSpacing;
				}
			}
		}
	}


#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::StringIsRelevantToStance
//! Returns true if the first attack in a String maybe executed whilst stood in the current
//! stance.
//!
//!	We are using the ATTACK_CLASS here because we know that it correlates with the order
//! of ATTACK_MOVE_TYPE which is a bitshift based enum.  We are interested in the button
//! required to execute the move though - not the class of attack.  Attacks of a particular
//! class need not necessarily be placed in their default slot (AIs for example), although
//! if would confuse the player no end if some of their attacks were muddled.
//!
//------------------------------------------------------------------------------------------
bool AttackDebugger::StringIsRelevantToStance( const CAttackComponent* pobAttackComponent, ATTACK_MOVE_TYPE eAttackType )
{
	// Otherwise we must check for stance specific attacks
	switch ( pobAttackComponent->m_eCurrentStance )
	{
	case ST_SPEED:
		if ( ( eAttackType == AM_SPEED_FAST ) || ( eAttackType == AM_SPEED_MEDIUM ) || ( eAttackType == AM_SPEED_GRAB ) )
			return true;
		break;

	case ST_POWER:
		if ( ( eAttackType == AM_POWER_FAST ) || ( eAttackType == AM_POWER_MEDIUM ) || ( eAttackType == AM_POWER_GRAB ) )
			return true;
		break;

	case ST_RANGE:
		if ( ( eAttackType == AM_RANGE_FAST ) || ( eAttackType == AM_RANGE_MEDIUM ) || ( eAttackType == AM_RANGE_GRAB ) )
			return true;
		break;

	default:
		ntAssert( 0 );
		break;
	}

	// If we are here then the String isn't valid
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::SetDebugComponent
//! Set up the active debug component
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::SetDebugComponent( MOUSE_BUTTON eButton, CAttackComponent const*& pobDebugComponent )
{
	// Check that we have an active mouse input component
	if ( MouseInput::Exists() )
	{
		// If the required mouse button is currently pressed...
		if ( MouseInput::Get().GetButtonState( eButton ).GetPressed() )
		{
			// Reset the selection
			pobDebugComponent = 0;

			// Find a ray that the mouse position is describing
			CPoint obCameraPosition( CONSTRUCT_CLEAR );
			CDirection obWorldRay( CONSTRUCT_CLEAR );
			MouseInput::Get().GetWorldRayFromMousePos( obWorldRay, obCameraPosition );

			Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;
			// [Mus] - What settings for this cast ?
			obFlag.flags.i_am = Physics::LINE_SIGHT_BIT;
			obFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
											Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
											Physics::RAGDOLL_BIT						);

			// Fire a ray parallel to the camera Z axis to find the first entity it intersects
			const CEntity* pobClickedOnEntity = Physics::CPhysicsWorld::Get().CastRay( obCameraPosition, CPoint( obWorldRay * 1000.0f ), obFlag);
			
			// If the entity has an attack component and it is registered with us set
			// it to the passed in component pointer - peasy
			if ( pobClickedOnEntity && IsRegistered( pobClickedOnEntity->GetAttackComponent() ) )
				pobDebugComponent = pobClickedOnEntity->GetAttackComponent();
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::IsRegistered
//! Checks if an attack component is registered in our list
//!
//------------------------------------------------------------------------------------------
bool AttackDebugger::IsRegistered( const CAttackComponent* pobAttackComponent )
{
	// If we have been passed a null pointer drop out early
	if ( !pobAttackComponent )
		return false;

	// Loop through our list of components
	ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
	for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
	{
		// If we find it return true
		if ( *obIt == pobAttackComponent )
			return true;
	}
	
	// If we have got here then we haven't found the thing
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RegisterComponent
//! Add a component you would like to debug using the attack debugger
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RegisterComponent( const CAttackComponent* pobComponent )
{
	// Push this component on to our list
	m_pobAttackComponents.push_front( pobComponent );

	// Add the first registered component to the player one slot
	if ( !m_pobDebugComponentOne )
		m_pobDebugComponentOne = pobComponent;
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::UnregisterComponent
//! Remove an attack component from the debug list
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::UnregisterComponent( const CAttackComponent* pobComponent )
{
	// Loop through our list of components
	ntstd::List< const CAttackComponent* >::iterator obEnd = m_pobAttackComponents.end();
	for( ntstd::List< const CAttackComponent* >::iterator obIt = m_pobAttackComponents.begin(); obIt != obEnd; ++obIt )
	{
		// If this is the one we are looking for then remove it and drop out so we can check something was found
		if ( *obIt == pobComponent )
		{
			m_pobAttackComponents.erase( obIt );
			return;
		}
	}
	
	// If we have got here then we haven't found the thing
	ntAssert( 0 );
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RenderProtractors
//! Draw the 'protractors' for player one and two if relevant
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RenderProtractors( void )
{
	// Render a protractor for each
	if ( m_bRenderProtractors )
	{
		RenderProtractor( m_pobDebugComponentOne );
		RenderProtractor( m_pobDebugComponentTwo );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RenderProtractor
//! Render a protrator to show movement angles and distances
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RenderProtractor( const CAttackComponent* pobAttackComponent )
{
	// Possibly unused in release
	UNUSED( pobAttackComponent );

#ifndef _RELEASE

	// If we have no component pointer drop out
	if ( !pobAttackComponent )
		return;

	// Find the facing angle of the player about the y axis - relative to z
	float fAngle = MovementControllerUtilities::GetYRotation( pobAttackComponent->m_pobParentEntity->GetRotation() );

	// Find the centre point of the circle
	CPoint obCentre = pobAttackComponent->m_pobParentEntity->GetPosition();
	obCentre.Y() += PROTRACTOR_OFFSETY;
	
	// Create an array of first and last points on the each radius
	CPoint obFirstPoint[ PROTRACTOR_CIRCLE_COUNT ];
	CPoint obLastPoint[ PROTRACTOR_CIRCLE_COUNT ];
	
	// Loop through all the radius circles at once
	for ( int iStep = 0; iStep < PROTRACTOR_SEGMENTS; iStep++ )
	{
		// For each of the radii...
		for ( int iCount = 0; iCount < PROTRACTOR_CIRCLE_COUNT; iCount++ ) 
		{
			// Make sure the circle has a valid radius, otherwise ignore it
			if ( PROTRACTOR_RADIUS[iCount] > 0.0f ) 
			{
				// Generate the points we are drawing between
				CPoint obPoint( obCentre );
				obPoint.X() -= PROTRACTOR_RADIUS[iCount] * fcosf( fAngle );
				obPoint.Z() += PROTRACTOR_RADIUS[iCount] * fsinf( fAngle );

				// The first time through we just make a note of the start point
				if ( iStep == 0 )
					obFirstPoint[iCount] = obPoint;

				// Otherwise we draw from the last point to the new point
				else 
				{
					// Render from the old point to the new point
					g_VisualDebug->RenderLine( obLastPoint[iCount], obPoint, PROTRACTOR_CIRCLE_COLOUR );	

					 // We are on the last step, close the circle
					if ( iStep == ( PROTRACTOR_SEGMENTS - 1 ) )
						g_VisualDebug->RenderLine( obPoint, obFirstPoint[iCount], PROTRACTOR_CIRCLE_COLOUR );
				}

				// Make a note of the point we just generated for the next time around
				obLastPoint[iCount] = obPoint; 

				// Draw segment lines between centre point and outermost circle
				if ( iCount == ( PROTRACTOR_CIRCLE_COUNT - 1 ) ) 
				{
					// If we are on an axis then we'll highlight the lines
					if ( abs( fmod( ( ( TWO_PI / PROTRACTOR_SEGMENTS ) * iStep ), HALF_PI ) ) < EPSILON )
						g_VisualDebug->RenderLine( obCentre, obPoint, PROTRACTOR_AXIS_COLOUR );
					else
						g_VisualDebug->RenderLine( obCentre, obPoint, PROTRACTOR_CIRCLE_COLOUR );
				}
			}
		}

		// Increment angle to next segment
		fAngle += ( TWO_PI / PROTRACTOR_SEGMENTS );
	}

#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::RenderGrid
//! Render a grid to to make movement distances clear
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::RenderGrid( void )
{
#ifndef _RELEASE

	// Don't bother with any of this if we are not to render a grid - or there is no player one
	if ( !m_pobDebugComponentOne || !m_bRenderPlayerOneGrid )
		return;

	// If the grid needs to be reset then recalulate the grid origin
	if ( m_bResetPlayerOneGrid )
	{
		// Set the grid matrix to be the world matrix of the character currently refered to as player one
		m_obPlayerOneGridMatrix = m_pobDebugComponentOne->m_pobParentEntity->GetMatrix();

		// This is what we have just done...
		m_bResetPlayerOneGrid = false;
	}

	// Render our grid - loop through the z axis...
	for ( float fX =- GRID_RADIUS; fX < GRID_RADIUS; fX += 1.0f )
	{
		// ...and loop through the x axis
		for ( float fZ =- GRID_RADIUS; fZ < GRID_RADIUS; fZ += 1.0f )
		{
			// Back to front
			CPoint obP1 = CPoint( -GRID_RADIUS, GRID_OFFSETY, fZ ) * m_obPlayerOneGridMatrix;
			CPoint obP2 = CPoint( GRID_RADIUS, GRID_OFFSETY, fZ ) * m_obPlayerOneGridMatrix;
			g_VisualDebug->RenderLine( obP1, obP2, GRID_COLOUR );

			// Left to right
			CPoint obP3 = CPoint( fX, GRID_OFFSETY, -GRID_RADIUS ) * m_obPlayerOneGridMatrix;
			CPoint obP4 = CPoint( fX, GRID_OFFSETY, GRID_RADIUS ) * m_obPlayerOneGridMatrix;
			g_VisualDebug->RenderLine( obP3, obP4, GRID_COLOUR );
		}
	}

	// Render grid origin
	CPoint obP1 = m_obPlayerOneGridMatrix.GetTranslation();
	obP1.Y() += 1.0f;
	CPoint obP2 = m_obPlayerOneGridMatrix.GetTranslation();
	obP2.Y() -= 1.0f;
	g_VisualDebug->RenderLine( obP1, obP2, GRID_ORIGIN_COLOUR );

#endif // _RELEASE
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::ClearAttackLists
//! Clears out the list describing possible attack strings (List of 
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::ClearAttackLists( void )
{
	// Loop through and destroy all the contained strings - list of pointers to lists
	for(	attackList::iterator obIt = m_obAttackList.begin();
			obIt != m_obAttackList.end(); ++obIt )
	{
		NT_DELETE( ( *obIt ) );
	}

	// Clear the pointers
	m_obAttackList.clear();
}


//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::FillAttackLists
//! Fills out our lists of attack strings that can be carried out from the start point of
//! the given CAttackLink.
//!
//!	We are using the ATTACK_CLASS here because we know that it correlates with the order
//! of ATTACK_MOVE_TYPE which is a bitshift based enum.  We are interested in the button
//! required to execute the move though - not the class of attack.  Attacks of a particular
//! class need not necessarily be placed in their default slot (AIs for example), although
//! if would confuse the player no end if some of their attacks were muddled.
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::FillAttackLists( const CAttackLink* pobAttackLink )
{
	// Make sure that our container is empty
	ntAssert( m_obAttackList.empty() );

	// Make sure that our base point is valid
	ntAssert( pobAttackLink );

	// Get a pointer to the first attack - fake up an array
	const CAttackLink* const* pobBaseAttack = &pobAttackLink->m_pobLinks[AM_SPEED_FAST];

	//GILES Loop through the base level of the attacks
	for( int iAttack = 0; iAttack < AM_ACTION; ++iAttack )
	{
		// If we have an attack in this slot...
		if ( pobBaseAttack[iAttack] )
		{
			// Create a new list for the attacks
			moveList* pobNewList = NT_NEW moveList;

			// Push the first attack onto the list
			pobNewList->push_back( static_cast<ATTACK_MOVE_TYPE>( iAttack ) );

			// See if any further attacks should be added to the String
			AddFurtherAttacks( pobBaseAttack[iAttack], pobNewList );

			// Push the pointer to this list onto the main list
			m_obAttackList.push_back( pobNewList );
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger::AddFurtherAttacks
//! Puts attacks into strings - used recursively
//!
//!	We are using the ATTACK_CLASS here because we know that it correlates with the order
//! of ATTACK_MOVE_TYPE which is a bitshift based enum.  We are interested in the button
//! required to execute the move though - not the class of attack.  Attacks of a particular
//! class need not necessarily be placed in their default slot (AIs for example), although
//! if would confuse the player no end if some of their attacks were muddled.
//!
//------------------------------------------------------------------------------------------
void AttackDebugger::AddFurtherAttacks( const CAttackLink* pobAttackLink, moveList* pobFirstString )
{
	// Make sure that our base point is valid
	ntAssert( pobAttackLink );

	// Get a pointer to the first attack - fake up an array
	const CAttackLink* const* pobBaseAttack = &pobAttackLink->m_pobLinks[AM_SPEED_FAST];

	// Set up a flag to see if we need to create further strings
	bool bUsedPassedString = false;

	// Take a copy of the list we were passed in - we'll need duplicates probably
	moveList obPassedString = *pobFirstString;

	//GILES Loop through the base level of the attacks
	for( int iAttack = 0; iAttack < AC_EVADE; ++iAttack )
	{
		// Check that we are not recursing - could crash
		if ( ( pobBaseAttack[iAttack] ) && ( pobBaseAttack[iAttack] == pobAttackLink ) )
		{
			// ntPrintf( "%s(%d):\tWARNING: Recursive attacks found in the special attack tree.", __FILE__, __LINE__ );
			continue;
		}

		// If we have an attack in this slot...
		if ( pobBaseAttack[iAttack] )
		{
			// Create a new list for the attacks
			moveList* pobNewList = 0;
				
			// If we haven't used the list we were passed yet
			if ( !bUsedPassedString )
			{
				// Point to the existing list
				pobNewList = pobFirstString;

				// Set the flag to say it's been used
				bUsedPassedString = true;
			}

			// Otherwise we need to create an entire new String
			else
			{
				// Create the new String and copy in the list we were passed
				pobNewList = NT_NEW moveList;
				*pobNewList = obPassedString;
			}

			// Push the first attack onto the list
			pobNewList->push_back( static_cast<ATTACK_MOVE_TYPE>( iAttack ) );

			// See if any further attacks should be added to the String
			AddFurtherAttacks( pobBaseAttack[iAttack], pobNewList );

			// If we have created this list push it on to our main list
			if ( pobNewList != pobFirstString )
				m_obAttackList.push_back( pobNewList );
		}
	}

}
