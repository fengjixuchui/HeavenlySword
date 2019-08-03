//------------------------------------------------------------------------------------------
//!
//!	nsmanager.cpp
//!
//------------------------------------------------------------------------------------------


#include "game/entity.h"
#include "game/entity.inl"
#include "game/ghostgirl.h"
#include "game/shellconfig.h"

#include "nsmanager.h"
#include "inputcomponent.h"

#include "anim/animator.h"
#include "anim/hierarchy.h"
#include "camera/camman.h"
#include "camera/camman_public.h"
#include "camera/camview.h"
#include "camera/coolcam_maya.h"
#include "effect/effect_shims.h"
#include "effect/effect_manager.h"
#include "game/aicomponent.h"
#include "game/attacks.h"
#include "game/awareness.h"
#include "game/entitybindings.h"
#include "game/entityinfo.h"
#include "game/messagehandler.h"
#include "game/movement.h"
#include "game/renderablecomponent.h"
#include "game/relativetransitions.h"
#include "input/inputhardware.h"
#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectcontainer.h"
#include "physics/advancedcharactercontroller.h"
#include "physics/system.h"
#include "area/areasystem.h"
#include "simpletransition.h"
#include "core/profiling.h"
#include "tbd/functor.h"
#include "game/combatstyle.h"

#include "audio/audiohelper.h"

#include "luaglobal.h"
#include "luahelp.h"
#include "luaattrtable.h"

#ifndef _RELEASE
#include "core/OSDDisplay.h"
#endif

#include "gfx/texturemanager.h"
#include "gui/guimanager.h"

#include "blendshapes/blendshapes_managers.h"
#include "blendshapes/BlendShapes.h"
#include "blendshapes/BlendShapesComponent.h"
#include "blendshapes/anim/BSAnimator.h"
#include "blendshapes/anim/blendshapes_anim.h"
#include "blendshapes/anim/BSAnimContainer.h"
#include "blendshapes/xpushapeblending.h"


#include "camera/basiccamera.h"

#include "core/user.h"


// debug switches
//#define _NSDEBUG_ALWAYSTRUE					// always success without inputs
//#define DEBUG_PLAYSINGLECLIP ("clip15")		// repeat a specific clip
//#define _DISPLAY_DEBUG_INFO					// display debug info.
//#define _USE_TICK								// use tick or cross code

#define _ALWAYS_CENTRAL_OVERLAYS				// always places overlays in the bottom centre of the screen
#define MIN_FLASH_HERTZ				(4)			// minimum hertz at which button mash icon can flash (doesn't affect logical hertz)
#define TICK_SHOW_PERIOD			(0.5f)		// length of time to show tick or cross
#define MIN_WINDOW_PENALTY_PERIOD	(0.01f)		// minimum length of penalty period for pressing the wrong button
#define WINDOW_GRACE_PERIOD			(0.2f)		// period after start of window where button mashing isn't tested (gives player time to read icon)
#define GAME_CAMERA_THRESHOLD		(0.5f)		// if clip camera anim is more than half a second shorter than clip duration change to game camera at end

// for convenience
#define BUTTONSPEC (NSManager::Get().GetButtonSpec())

// are we using HDR or LDR screen hints
//#define NS_HDR

// used to make the TGS demo work until we get the spec'ed NS behaviour
#define TGS_NS_HACKS

// interfaces
START_STD_INTERFACE(NinjaSequence)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_sNsFormatVersion, "1.0", m_sNsFormatVersion )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_sExporterVersion, "unknown", m_sExporterVersion )
	PUBLISH_VAR_AS( m_fDuration, m_fDuration )
	PUBLISH_PTR_CONTAINER_AS( m_ClipList, m_ClipList )
	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
END_STD_INTERFACE

START_STD_INTERFACE(NinjaSequence_Entity)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	COPY_INTERFACE_FROM(CEntity)

	OVERRIDE_DEFAULT(Position, "0.0,1.0,0.0")

    PUBLISH_PTR_AS(m_pobLevelCameraObject, 							EndSequenceLevelCamera )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fFinalCameraOutTransition, 0.0f, FinalCameraOutTransitionTime)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obEndSequenceLevelCamera, "", 	EndSequenceLevelCameraName )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bIsTutorial, false, 				IsTutorial )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fTutorialScalar, 1.0,			TutorialScalar )
	PUBLISH_PTR_AS(	m_pobNinjaSequence, 							NinjaSequenceName )


	// a hangover from the past, or useful?
	PUBLISH_VAR_AS(m_obStartupScript, 								StartupScript)
	PUBLISH_VAR_AS(m_obShutdownScript, 								ShutdownScript)

	PUBLISH_PTR_CONTAINER_AS(m_obEntityClearanceVolumes,			EntityClearanceVolumes)

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)

END_STD_INTERFACE

START_STD_INTERFACE(NSClip)
	PUBLISH_VAR_AS( m_fDuration, m_fDuration )
	PUBLISH_VAR_AS( m_fWindowStartTime, m_fWindowStartTime )
	PUBLISH_VAR_AS( m_fWindowEndTime, m_fWindowEndTime )
	PUBLISH_VAR_AS( m_sDefaultGotoClip, m_sDefaultGotoClip )
	PUBLISH_PTR_CONTAINER_AS( m_ConditionList, m_ConditionList )
	PUBLISH_PTR_CONTAINER_AS( m_AnimList, m_AnimList )
	PUBLISH_PTR_CONTAINER_AS( m_BSAnimList, m_BSAnimList )
	PUBLISH_PTR_CONTAINER_AS( m_EventList, m_EventList )
	PUBLISH_PTR_AS(m_pTweaker, POV_Tweaker)
END_STD_INTERFACE

START_STD_INTERFACE(NSCondition)
	PUBLISH_VAR_AS( m_sTest, m_sTest )
	PUBLISH_VAR_AS( m_sIfTrue, m_sIfTrue )
	PUBLISH_VAR_AS( m_sIfFalse, m_sIfFalse )
END_STD_INTERFACE

START_STD_INTERFACE(NSEnt)
	PUBLISH_VAR_AS( m_sName, m_sName )
	PUBLISH_VAR_AS( m_sClump, m_sClump )
	PUBLISH_VAR_AS( m_sPhysics, m_sPhysics )
	PUBLISH_VAR_AS( m_sBSClump, m_sBSClump )
	PUBLISH_PTR_AS( m_pAnimContainer, m_pAnimContainer )
	PUBLISH_PTR_AS( m_pBSAnimContainer, m_pBSAnimContainer )
END_STD_INTERFACE

START_STD_INTERFACE(NSAnim)
	PUBLISH_VAR_AS( m_sTargetAnim, m_sTargetAnim )
	PUBLISH_PTR_AS( m_pTargetEntity, m_pTargetEntity )
END_STD_INTERFACE

START_STD_INTERFACE(NSBSAnim)
	PUBLISH_VAR_AS( m_sTargetAnim, m_sTargetAnim )
	PUBLISH_PTR_AS( m_pTargetEntity, m_pTargetEntity )
END_STD_INTERFACE

START_STD_INTERFACE(NSEvent)
	PUBLISH_VAR_AS( m_fTriggerTime, m_fTriggerTime )
	PUBLISH_VAR_AS( m_sEvent, m_sEvent )
END_STD_INTERFACE


// button specification interface - lots of tweakable params here...
START_STD_INTERFACE(NSButtonSpec)

	PUBLISH_PTR_AS(	m_pobHudDefPad, 		 	huddef_nstut_pad )
	PUBLISH_PTR_AS(	m_pobHudDefTutorialLeft, 	huddef_nstut_promptleft )
	PUBLISH_PTR_AS(	m_pobHudDefTutorialRight, 	huddef_nstut_promptright )
	PUBLISH_PTR_AS(	m_pobHudDefTutorialUp, 		huddef_nstut_promptup )
	PUBLISH_PTR_AS(	m_pobHudDefTutorialDown,	huddef_nstut_promptdown )
	PUBLISH_PTR_AS(	m_pobHudDefTutorialUpDown,	huddef_nstut_promptupdown )

	PUBLISH_VAR_AS( m_fDirection_top_size, direction_top_size )
	PUBLISH_VAR_AS( m_fDirection_top_right_size, direction_top_right_size )
	PUBLISH_VAR_AS( m_fDirection_top_left_size, direction_top_left_size )
	PUBLISH_VAR_AS( m_fDirection_bottom_size, direction_bottom_size )
	PUBLISH_VAR_AS( m_fDirection_bottom_right_size, direction_bottom_right_size )
	PUBLISH_VAR_AS( m_fDirection_bottom_left_size, direction_bottom_left_size )
	PUBLISH_VAR_AS( m_fDirection_left_size, direction_left_size )
	PUBLISH_VAR_AS( m_fDirection_right_size, direction_right_size )
	PUBLISH_VAR_AS( m_fButton_triangle_size, button_triangle_size )
	PUBLISH_VAR_AS( m_fButton_square_size, button_square_size )
	PUBLISH_VAR_AS( m_fButton_cross_size, button_cross_size )
	PUBLISH_VAR_AS( m_fButton_circle_size, button_circle_size )

	PUBLISH_VAR_AS( m_fDirection_top_alpha, direction_top_alpha )
	PUBLISH_VAR_AS( m_fDirection_top_right_alpha, direction_top_right_alpha )
	PUBLISH_VAR_AS( m_fDirection_top_left_alpha, direction_top_left_alpha )
	PUBLISH_VAR_AS( m_fDirection_bottom_alpha, direction_bottom_alpha )
	PUBLISH_VAR_AS( m_fDirection_bottom_right_alpha, direction_bottom_right_alpha )
	PUBLISH_VAR_AS( m_fDirection_bottom_left_alpha, direction_bottom_left_alpha )
	PUBLISH_VAR_AS( m_fDirection_left_alpha, direction_left_alpha )
	PUBLISH_VAR_AS( m_fDirection_right_alpha, direction_right_alpha )
	PUBLISH_VAR_AS( m_fButton_triangle_alpha, button_triangle_alpha )
	PUBLISH_VAR_AS( m_fButton_square_alpha, button_square_alpha )
	PUBLISH_VAR_AS( m_fButton_cross_alpha, button_cross_alpha )
	PUBLISH_VAR_AS( m_fButton_circle_alpha, button_circle_alpha )

	PUBLISH_VAR_AS( m_fDirection_top_pos[0], direction_top_position_X )
	PUBLISH_VAR_AS( m_fDirection_top_pos[1], direction_top_position_Y )
	PUBLISH_VAR_AS( m_fDirection_top_right_pos[0], direction_top_right_position_X )
	PUBLISH_VAR_AS( m_fDirection_top_right_pos[1], direction_top_right_position_Y )
	PUBLISH_VAR_AS( m_fDirection_top_left_pos[0], direction_top_left_position_X )
	PUBLISH_VAR_AS( m_fDirection_top_left_pos[1], direction_top_left_position_Y )
	PUBLISH_VAR_AS( m_fDirection_bottom_pos[0], direction_bottom_position_X )
	PUBLISH_VAR_AS( m_fDirection_bottom_pos[1], direction_bottom_position_Y )
	PUBLISH_VAR_AS( m_fDirection_bottom_right_pos[0], direction_bottom_right_position_X )
	PUBLISH_VAR_AS( m_fDirection_bottom_right_pos[1], direction_bottom_right_position_Y )
	PUBLISH_VAR_AS( m_fDirection_bottom_left_pos[0], direction_bottom_left_position_X )
	PUBLISH_VAR_AS( m_fDirection_bottom_left_pos[1], direction_bottom_left_position_Y )
	PUBLISH_VAR_AS( m_fDirection_left_pos[0], direction_left_position_X )
	PUBLISH_VAR_AS( m_fDirection_left_pos[1], direction_left_position_Y )
	PUBLISH_VAR_AS( m_fDirection_right_pos[0], direction_right_position_X )
	PUBLISH_VAR_AS( m_fDirection_right_pos[1], direction_right_position_Y )
	PUBLISH_VAR_AS( m_fButton_triangle_pos[0], button_triangle_position_X )
	PUBLISH_VAR_AS( m_fButton_triangle_pos[1], button_triangle_position_Y )
	PUBLISH_VAR_AS( m_fButton_square_pos[0], button_square_position_X )
	PUBLISH_VAR_AS( m_fButton_square_pos[1], button_square_position_Y )
	PUBLISH_VAR_AS( m_fButton_cross_pos[0], button_cross_position_X )
	PUBLISH_VAR_AS( m_fButton_cross_pos[1], button_cross_position_Y )
	PUBLISH_VAR_AS( m_fButton_circle_pos[0], button_circle_position_X )
	PUBLISH_VAR_AS( m_fButton_circle_pos[1], button_circle_position_Y )

	PUBLISH_VAR_AS( m_sButton_triangle_texture, button_triangle_texture )
	PUBLISH_VAR_AS( m_sButton_square_texture, button_square_texture )
	PUBLISH_VAR_AS( m_sButton_cross_texture, button_cross_texture )
	PUBLISH_VAR_AS( m_sButton_circle_texture, button_circle_texture )
	PUBLISH_VAR_AS( m_sButton_triangle_texture_w, button_triangle_texture_wrong )
	PUBLISH_VAR_AS( m_sButton_square_texture_w, button_square_texture_wrong )
	PUBLISH_VAR_AS( m_sButton_cross_texture_w, button_cross_texture_wrong )
	PUBLISH_VAR_AS( m_sButton_circle_texture_w, button_circle_texture_wrong )
	PUBLISH_VAR_AS( m_sButton_triangle_mash_texture, button_triangle_mash_texture )
	PUBLISH_VAR_AS( m_sButton_square_mash_texture, button_square_mash_texture )
	PUBLISH_VAR_AS( m_sButton_cross_mash_texture, button_cross_mash_texture )
	PUBLISH_VAR_AS( m_sButton_circle_mash_texture, button_circle_mash_texture )

	PUBLISH_VAR_AS( m_sDirection_top_texture, direction_top_texture )
	PUBLISH_VAR_AS( m_sDirection_top_right_texture, direction_top_right_texture )
	PUBLISH_VAR_AS( m_sDirection_top_left_texture, direction_top_left_texture )
	PUBLISH_VAR_AS( m_sDirection_bottom_texture, direction_bottom_texture )
	PUBLISH_VAR_AS( m_sDirection_bottom_right_texture, direction_bottom_right_texture )
	PUBLISH_VAR_AS( m_sDirection_bottom_left_texture, direction_bottom_left_texture )
	PUBLISH_VAR_AS( m_sDirection_left_texture, direction_left_texture )
	PUBLISH_VAR_AS( m_sDirection_right_texture, direction_right_texture )
	PUBLISH_VAR_AS( m_sDirection_top_texture_w, direction_top_texture_wrong )
	PUBLISH_VAR_AS( m_sDirection_top_right_texture_w, direction_top_right_texture_wrong )
	PUBLISH_VAR_AS( m_sDirection_top_left_texture_w, direction_top_left_texture_wrong )
	PUBLISH_VAR_AS( m_sDirection_bottom_texture_w, direction_bottom_texture_wrong )
	PUBLISH_VAR_AS( m_sDirection_bottom_right_texture_w, direction_bottom_right_texture_wrong )
	PUBLISH_VAR_AS( m_sDirection_bottom_left_texture_w, direction_bottom_left_texture_wrong )
	PUBLISH_VAR_AS( m_sDirection_left_texture_w, direction_left_texture_wrong )
	PUBLISH_VAR_AS( m_sDirection_right_texture_w, direction_right_texture_wrong )

	PUBLISH_VAR_AS( m_fCorrect_expand_rate, correct_expand_rate );
	PUBLISH_VAR_AS( m_fCorrect_fade_rate, correct_fade_rate );
	PUBLISH_VAR_AS( m_fWrong_fade_rate, wrong_fade_rate );
	PUBLISH_VAR_AS( m_fDrop_fade_rate, drop_fade_rate );
	PUBLISH_VAR_AS( m_fDrop_accel_rate, drop_accel_rate );
	PUBLISH_VAR_AS( m_fWobbleXRate, wobble_x_rate );
	PUBLISH_VAR_AS( m_fWobbleYRate, wobble_y_rate );
	PUBLISH_VAR_AS( m_fWobbleXAmplitude, wobble_x_amplitude );
	PUBLISH_VAR_AS( m_fWobbleYAmplitude, wobble_y_amplitude );
	PUBLISH_VAR_AS( m_fHDRBase, HDR_base );

END_STD_INTERFACE


// statics
const char* NSManager::m_aConditionStrings[] = { "TEST_INPUT", "IN_WINDOW", "*" };
const char* NSPackage::m_pcEndClipString = "END";
const char* NSPackage::m_pcDieClipString = "DIE";
const char* NSPackage::m_pcFailureClipString = "FAIL";

// match these with order of NSCommand::CONTROLLER_INPUT_TYPE enum
const char* NSManager::m_aInputStrings[] = {	"gKey0", "gKey1", "gKey2","gKey3", 
												"gUpLeft", "gUpRight", "gDownLeft", "gDownRight",
												"gUp", "gLeft", "gRight", "gDown", 
												"*" };

const char* NSManager::m_aEventStrings[] = { "SET_VISIBLE:", "SET_INVISIBLE:", "KILL_ENTITY:", "*" };

//------------------------------------------------------------------------------------------
//!
//!	NSEnt::NSEnt
//!	Construction
//!
//------------------------------------------------------------------------------------------
NSEnt::NSEnt( void )
:	m_sName(),
	m_sClump(),
	m_sPhysics(),
	m_sBSClump(),
	m_pAnimContainer( 0 ),
	m_pBSAnimContainer( 0 ),
	m_pGameEntity( 0 ),
	m_pCameraAnimator(0),
	m_pEffectController(0),
	m_pobHome(0),
	m_bHadAIEnabled( false ),
	m_bHadMovementComp( true ),
	m_bHadAnimatorComp( true ),
	m_bHadPhysicsComp( false ),
	m_bAnimContainerAdded( false ),	
	m_bBSAnimContainerAdded( false ),
	m_bBlendShapesComponentAdded( false ),
	m_bBSClumpAdded( false ),
	m_bHadAttackComp( false ),
	m_bNSEntity( false ),
	m_bInvisibleAfterClip( false )	
{
}


//------------------------------------------------------------------------------------------
//!
//!	NSEnt::NSEnt
//!	Destruction
//!
//------------------------------------------------------------------------------------------
NSEnt::~NSEnt( void )
{
	// If held within a nspackage - remove the NS entity
	if( m_pobHome )
	{
		m_pobHome->RemoveEntity( m_pGameEntity, this );
		m_pobHome = 0;
	}

	if(m_pCameraAnimator)
		NT_DELETE(m_pCameraAnimator);
}

//------------------------------------------------------------------------------------------
//!
//!	NinjaSequence::NinjaSequence
//!	Construction
//!
//------------------------------------------------------------------------------------------
NinjaSequence::NinjaSequence( void )
:	m_fDuration( 0.0f ),
	m_ClipList(),
	m_pobNSPackage( 0 )
{
}

//------------------------------------------------------------------------------------------
//!
//!	NinjaSequence::~NinjaSequence
//!	Destruction
//!
//------------------------------------------------------------------------------------------
NinjaSequence::~NinjaSequence( void )
{
	// Clear up the package that we describe
	NT_DELETE( m_pobNSPackage );
}


//------------------------------------------------------------------------------------------
//!
//!	NinjaSequence::ConstructDescribedPackage
//!
//------------------------------------------------------------------------------------------
void NinjaSequence::ConstructDescribedPackage( NSEntityInstance* pobEntity )
{
	user_error_p(!m_pobNSPackage, ("Must not have a NS package around at the time of construction!!!"));

	// Build the package
	m_pobNSPackage = NT_NEW NSPackage( pobEntity );
}

//------------------------------------------------------------------------------------------
//!
//!	NinjaSequence::OnPostConstruct  
//! 
//!	DESCRIPTION		 Use postconstruct callback for checking
//!					 the format version with (hard coded) 
//!					 known latestversion, and printing the export version
//!
//------------------------------------------------------------------------------------------
void NinjaSequence::OnPostConstruct()
{
	const float fFormatVersion = 
		(float)atof(m_sNsFormatVersion.c_str());
	const float fMinVersion = 1.0f;
	const float fCurrVersion = 1.1f;

	UNUSED(fFormatVersion);
	UNUSED(fMinVersion);
	UNUSED(fCurrVersion);

	user_error_p(fFormatVersion>=fMinVersion, ("*** Note : NS format version is %f, less than earliest version (%f)!!!",fFormatVersion,fMinVersion));
	user_warn_p(fFormatVersion>=fCurrVersion, ("*** Note : NS format version is %f, less than latest version (%f)!!!",fFormatVersion,fCurrVersion));
	if (fFormatVersion>fCurrVersion)
	{
		ntPrintf("*** Note : NS format version is %f, this is greater than hard coded version (%f)- update code to reflect latest!!!\n",fFormatVersion,fCurrVersion); 	
	}
	ntPrintf("*** Note : NS export version is %s!!!\n",	m_sExporterVersion.c_str());
}


//------------------------------------------------------------------------------------------
//!
//!	NSManager::NSManager
//!
//------------------------------------------------------------------------------------------

NSManager::NSManager() : m_pNSButtonSpec( NULL ),  
						 m_bRestart( false ),
						 m_bSkippingSequence( false ),
						 m_iRenderRequest(-1)
						 
{
	m_playbackClipName = ntstd::String("");
}

//------------------------------------------------------------------------------------------
//!
//!	NSManager::~NSManager
//!
//------------------------------------------------------------------------------------------

NSManager::~NSManager()
{
}

//------------------------------------------------------------------------------------------
//!
//!	NSManager::DebugRender
//!
//------------------------------------------------------------------------------------------

void NSManager::DebugRender()
{
}

void NSManager::BuildInputIconDef( int iID, 
							const CKeyString& obCorrect,
							const CKeyString& obIncorrect,
							const CVector& obCol,
							const CVector& obPosScaleInfo)
{
	m_aobRenderSetForInputs[iID].m_iNumTexturesPerCorrectInput = 0;

    m_aobRenderSetForInputs[iID].m_obRenderDef.m_bLooping = false;
    m_aobRenderSetForInputs[iID].m_obRenderDef.m_aobFrameList.push_back ( obCorrect );

	// so far, up to all the frames for texture animating correct input.
	m_aobRenderSetForInputs[iID].m_iNumTexturesPerCorrectInput++;

    m_aobRenderSetForInputs[iID].m_obRenderDef.m_aobFrameList.push_back (  obIncorrect );

	m_aobRenderSetForInputs[iID].m_obRenderDef.m_iFramesPerSecond = 0;

	HudImageOverlaySlap obOverlay = 
			m_aobRenderSetForInputs[iID].m_obRenderDef.GetBaseOverlay();

	obOverlay.m_fTopLeftX = 			0.5f+obPosScaleInfo.X()*0.5f;
	obOverlay.m_fTopLeftY = 			0.5f+obPosScaleInfo.Y()*0.5f;
	obOverlay.m_obColour = 				obCol;
	obOverlay.m_fScale = 				obPosScaleInfo.W();

	m_aobRenderSetForInputs[iID].m_obRenderDef.SetBaseOverlay(obOverlay);

    m_aobRenderSetForInputs[iID].m_obRenderDef.m_eBlendMode  = 	EBM_LERP;


	// load to cache textures, and extract info
	TextureManager::Get().LoadTexture_Neutral( obIncorrect.GetString() );
	
	const Texture* const pobTextureCorrect = 
		TextureManager::Get().LoadTexture_Neutral( obCorrect.GetString() ).Get();

	// Scale our heights and positions accordingly
	const float fBBHeight = CGuiManager::Get().BBHeight();
	const float fBBWidth = CGuiManager::Get().BBWidth();

	m_aobRenderSetForInputs[iID].m_obRenderDef.m_fWidth = 
		((float)pobTextureCorrect->GetWidth())/fBBWidth;

    m_aobRenderSetForInputs[iID].m_obRenderDef.m_fHeight = 
		((float)pobTextureCorrect->GetHeight())/fBBHeight;

	m_aobRenderSetForInputs[iID].m_bIsValid = true;
}


void NSManager::HudRenderDefinition::CacheDefTextures()
{	
	for ( FrameIter obIt = m_obRenderDef.m_aobFrameList.begin(); obIt != m_obRenderDef.m_aobFrameList.end(); obIt++)
	{
		TextureManager::Get().LoadTexture_Neutral( obIt->GetString() );
	}

	m_bIsValid = true;
}

//------------------------------------------------------------------------------------------
//!
//!	NSManager::Update
//! NS Manager update
//!
//------------------------------------------------------------------------------------------

void NSManager::Update( float fDt )
{
	// get ptr to button specs container
	if( !m_pNSButtonSpec )
	{
		// for the render requests to happen later with the commands (for hud display)
		m_iRenderRequest = 0;

		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( "ninjasequence_buttonspec" );
		m_pNSButtonSpec = (NSButtonSpec*)( pDO->GetBasePtr() );

		m_obRenderForTutorialPad.Set(*m_pNSButtonSpec->m_pobHudDefPad);
		m_obRenderForTutorialPad.CacheDefTextures();

        m_aobRenderSetForTutorialHelpers[NSCommand::NS_MOVEDOWN].Set(*m_pNSButtonSpec->m_pobHudDefTutorialDown);
		m_aobRenderSetForTutorialHelpers[NSCommand::NS_MOVEDOWN].CacheDefTextures();
        
		m_aobRenderSetForTutorialHelpers[NSCommand::NS_MOVERIGHT].Set(*m_pNSButtonSpec->m_pobHudDefTutorialRight);
		m_aobRenderSetForTutorialHelpers[NSCommand::NS_MOVERIGHT].CacheDefTextures();

        m_aobRenderSetForTutorialHelpers[NSCommand::NS_MOVELEFT].Set(*m_pNSButtonSpec->m_pobHudDefTutorialLeft);
		m_aobRenderSetForTutorialHelpers[NSCommand::NS_MOVELEFT].CacheDefTextures();

        m_aobRenderSetForTutorialHelpers[NSCommand::NS_MOVEUP].Set(*m_pNSButtonSpec->m_pobHudDefTutorialUp);
		m_aobRenderSetForTutorialHelpers[NSCommand::NS_MOVEUP].CacheDefTextures();

        m_aobRenderSetForTutorialHelpers[NSCommand::NS_ACTION].Set(*m_pNSButtonSpec->m_pobHudDefTutorialUpDown);
		m_aobRenderSetForTutorialHelpers[NSCommand::NS_ACTION].CacheDefTextures();
		
		//
		// scee.sbashow and define the graphics:
		//m_aobRenderSetForInputs[NSCommand::NS_ACTION].m_obRenderDef = 
		//	*m_pNSButtonSpec->m_pobHudDefShake;
		
		BuildInputIconDef(NSCommand::NS_ACTION,
				  CKeyString(m_pNSButtonSpec->m_sButton_cross_texture),
				  CKeyString(m_pNSButtonSpec->m_sButton_cross_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fButton_cross_alpha),
				  CVector(m_pNSButtonSpec->m_fButton_cross_pos[0],
						  m_pNSButtonSpec->m_fButton_cross_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fButton_cross_size));
		
		BuildInputIconDef(NSCommand::NS_ATTACK,
				  CKeyString(m_pNSButtonSpec->m_sButton_square_texture),
				  CKeyString(m_pNSButtonSpec->m_sButton_square_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fButton_square_alpha),
				  CVector(m_pNSButtonSpec->m_fButton_square_pos[0],
						  m_pNSButtonSpec->m_fButton_square_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fButton_square_size));

		BuildInputIconDef(NSCommand::NS_COUNTER,
				  CKeyString(m_pNSButtonSpec->m_sButton_triangle_texture),
				  CKeyString(m_pNSButtonSpec->m_sButton_triangle_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fButton_triangle_alpha),
				  CVector(m_pNSButtonSpec->m_fButton_triangle_pos[0],
						  m_pNSButtonSpec->m_fButton_triangle_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fButton_triangle_size));

		BuildInputIconDef(NSCommand::NS_GRAB,
				  CKeyString(m_pNSButtonSpec->m_sButton_circle_texture),
				  CKeyString(m_pNSButtonSpec->m_sButton_circle_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fButton_circle_alpha),
				  CVector(m_pNSButtonSpec->m_fButton_circle_pos[0],
						  m_pNSButtonSpec->m_fButton_circle_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fButton_circle_size));

		BuildInputIconDef(NSCommand::NS_MOVEUP,
				  CKeyString(m_pNSButtonSpec->m_sDirection_top_texture),
				  CKeyString(m_pNSButtonSpec->m_sDirection_top_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fDirection_top_alpha),
				  CVector(m_pNSButtonSpec->m_fDirection_top_pos[0],
						  m_pNSButtonSpec->m_fDirection_top_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fDirection_top_size));

		BuildInputIconDef(NSCommand::NS_MOVEDOWN,
				  CKeyString(m_pNSButtonSpec->m_sDirection_bottom_texture),
				  CKeyString(m_pNSButtonSpec->m_sDirection_bottom_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fDirection_bottom_alpha),
				  CVector(m_pNSButtonSpec->m_fDirection_bottom_pos[0],
						  m_pNSButtonSpec->m_fDirection_bottom_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fDirection_bottom_size));

		BuildInputIconDef(NSCommand::NS_MOVELEFT,
				  CKeyString(m_pNSButtonSpec->m_sDirection_left_texture),
				  CKeyString(m_pNSButtonSpec->m_sDirection_left_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fDirection_left_alpha),
				  CVector(m_pNSButtonSpec->m_fDirection_left_pos[0],
						  m_pNSButtonSpec->m_fDirection_left_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fDirection_left_size));

		BuildInputIconDef(NSCommand::NS_MOVERIGHT,
				  CKeyString(m_pNSButtonSpec->m_sDirection_right_texture),
				  CKeyString(m_pNSButtonSpec->m_sDirection_right_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fDirection_right_alpha),
				  CVector(m_pNSButtonSpec->m_fDirection_right_pos[0],
						  m_pNSButtonSpec->m_fDirection_right_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fDirection_right_size));

		BuildInputIconDef(NSCommand::NS_UPLEFT,
				  CKeyString(m_pNSButtonSpec->m_sDirection_top_left_texture),
				  CKeyString(m_pNSButtonSpec->m_sDirection_top_left_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fDirection_top_left_alpha),
				  CVector(m_pNSButtonSpec->m_fDirection_top_left_pos[0],
						  m_pNSButtonSpec->m_fDirection_top_left_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fDirection_top_left_size));

		BuildInputIconDef(NSCommand::NS_UPRIGHT,
				  CKeyString(m_pNSButtonSpec->m_sDirection_top_right_texture),
				  CKeyString(m_pNSButtonSpec->m_sDirection_top_right_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fDirection_top_right_alpha),
				  CVector(m_pNSButtonSpec->m_fDirection_top_right_pos[0],
						  m_pNSButtonSpec->m_fDirection_top_right_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fDirection_top_right_size));

		BuildInputIconDef(NSCommand::NS_DOWNLEFT,
				  CKeyString(m_pNSButtonSpec->m_sDirection_bottom_left_texture),
				  CKeyString(m_pNSButtonSpec->m_sDirection_bottom_left_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fDirection_bottom_left_alpha),
				  CVector(m_pNSButtonSpec->m_fDirection_bottom_left_pos[0],
						  m_pNSButtonSpec->m_fDirection_bottom_left_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fDirection_bottom_left_size));

		BuildInputIconDef(NSCommand::NS_DOWNRIGHT,
				  CKeyString(m_pNSButtonSpec->m_sDirection_bottom_right_texture),
				  CKeyString(m_pNSButtonSpec->m_sDirection_bottom_right_texture_w),
				  CVector(1.0f,1.0f,1.0f,m_pNSButtonSpec->m_fDirection_bottom_right_alpha),
				  CVector(m_pNSButtonSpec->m_fDirection_bottom_right_pos[0],
						  m_pNSButtonSpec->m_fDirection_bottom_right_pos[1],
						  0.0f,
						  m_pNSButtonSpec->m_fDirection_bottom_size));

	}

#ifndef _RELEASE
	// debug fast forward the sequence by pressing joypad L1
	if( !m_bSkippingSequence )
	{
		if( IsNinjaSequencePlaying() && (CInputHardware::Get().GetPad( PAD_0 ).GetPressed() & PAD_LEFT_THUMB) )
		{
			m_bSkippingSequence = true;
			CTimer::Get().SetDebugTimeScalar(5.0f);
		}
	}
	else
	{
		if( !IsNinjaSequencePlaying() )
		{
			m_bSkippingSequence = false;
			CTimer::Get().SetDebugTimeScalar(1.0f);
		}
	}
#endif

	// update packages
	for( ntstd::Vector< NSPackage* >::iterator nsp_it = m_NSPList.begin(); nsp_it != m_NSPList.end(); )
	{
		NSPackage* pNSP = *nsp_it;

		if( pNSP->GetNSStage() == NSPackage::NS_DISCARD )
		{
			// one more call to clean everything up
			pNSP->Update( fDt );

			pNSP->SetActiveInstance( 0 );

			// free completed package
			nsp_it = m_NSPList.erase( nsp_it );
		}
		else
		{
			pNSP->Update( fDt );
			++nsp_it;
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSManager::Reset
//!
//! Resets the manager, dumping any current packages
//!
//------------------------------------------------------------------------------------------

void NSManager::Reset()
{
	// set restart flag, this modifies some behaviour when cleaning
	// up the ninja sequences for level restarts
	m_bRestart = true;

	for( ntstd::Vector< NSPackage* >::iterator nsp_it = m_NSPList.begin(); nsp_it != m_NSPList.end(); )
	{
		NSPackage* pNSP = *nsp_it;

		pNSP->SetNSStage( NSPackage::NS_DISCARD );

		// clean up any currently playing clip
		pNSP->CloseClip();

		// one more call to clean every else up (timestep unused)
		pNSP->Update( 0.0f );

		// free package
		nsp_it = m_NSPList.erase( nsp_it );
	}

	m_bRestart = false;
}


//------------------------------------------------------------------------------------------
//!
//!	NSManager::RemoveEntityFromAllSequences
//!
//! Removes the entity from all packages
//!
//! \param	pEntity		Entity to remove
//!
//------------------------------------------------------------------------------------------


void NSManager::RemoveEntityFromAllSequences( CEntity* pEntity )
{
	ntAssert( pEntity );

	for( ntstd::Vector< NSPackage* >::iterator nsp_it = m_NSPList.begin(); nsp_it != m_NSPList.end(); ++nsp_it )
	{
		NSPackage* pNSP = *nsp_it;

		pNSP->RemoveEntity( pEntity );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	NSManager::AddNSP
//! Adds a package to play on the NSManager
//!
//------------------------------------------------------------------------------------------
void NSManager::AddNSP( NSPackage* pNSPackage, NSEntityInstance* pNSEntity )
{
	// Loop through all the existing packages and make sure this one isn't already playing
	ntstd::Vector<NSPackage*>::iterator obEnd = m_NSPList.end();
	for( ntstd::Vector<NSPackage*>::iterator obit = m_NSPList.begin(); obit != obEnd; ++obit )
	{
		// Get a pointer to the package
		NSPackage* pNSP = *obit;

		if ( pNSP->HasInstance( pNSEntity ) )
		{
			ntPrintf( "Attempt to play a Ninja Sequence that is already playing.\n" );
			return;
		}
	}

	// If all is well add package to start playing
	pNSPackage->ResetPackage();
	
	m_NSPList.push_back( pNSPackage );
}
	
NSCommand::NSCommand(CONTROLLER_INPUT_TYPE eInputID, bool bTutorialMode, 
          bool bIsTestInput):
	m_eInputID(eInputID),
	m_eCurrInputState(INPUT_STATE_NONE),
	m_bTutorialMode(bTutorialMode)
{
	if (bIsTestInput)
	{
		m_pobRenderSet = 
			NSManager::Get().RequestCommandRenderSet();
	}
	else
	{
		m_pobRenderSet = 0;
	}
}

const ntstd::String NSCommand::GetTutorialString( void ) const
{
	switch(m_eInputID)
	{
			case NS_MOVELEFT: 
			{	
				return ntstd::String("ids_fort_nstutorial_left");
			}
            break;
			case NS_MOVERIGHT: 
			{	
				return ntstd::String("ids_fort_nstutorial_right");
			}
			break;
			case NS_ATTACK: 
			{	
				return ntstd::String("ids_fort_nstutorial_square");
			}
			break;
            case NS_ACTION: 
			{	
				return ntstd::String("ids_fort_nstutorial_shake");
			}
			break;
		break;
		default:
			return ntstd::String("Err ...");
				break;
	 }
}

void NSCommand::GetMotionDirs( float& fDX, float& fDY) const
{
	const float fMag = 1.0f;

	switch(m_eInputID)
	{
		case NS_MOVELEFT: 
		{	
			fDX = -fMag;
			fDY = 0.0f;
		}
		break;
		case NS_MOVERIGHT: 
		{	
			fDX = fMag;
			fDY = 0.0f;
		}
		break;
		case NS_DOWNRIGHT: 
		{	
			fDX = fMag;
			fDY = -fMag;
		}
		break;
		case NS_DOWNLEFT: 
		{	
			fDX = -fMag;
			fDY = -fMag;
		}
		break;
		case NS_UPRIGHT: 
		{	
			fDX = fMag;
			fDY = fMag;
		}
		break;
		case NS_UPLEFT: 
		{	
			fDX = -fMag;
			fDY = fMag;
		}
		break;
		case NS_MOVEUP: 
		{	
			fDX = 0.0f;
			fDY = fMag;
		}
		break;
		case NS_MOVEDOWN: 
		{	
			fDX = 0.0f;
			fDY = -fMag;
		}
		break;

		case NS_ACTION: 
		{	
			fDX = 0.0f;
			fDY = fMag*5.0f;
		}
		break;
	break;
	default:
		fDX=fDY=0.0f;
			break;
	}
}

//------------------------------------------------------------------------------------------------
//!
//!	The standard overlay for a tutorial icon - moves it in the motion direction along the screen.
//!	NSCommand::GetTutorialBaseAppliedOverlay
//!
//------------------------------------------------------------------------------------------------

const HudImageOverlaySlap NSCommand::GetTutorialBaseAppliedOverlay( void ) const
{
	ntError(NSManager::Get().GetTutorialIconDefinition(m_eInputID).m_bIsValid);
	
	const HudImageRenderDef& obBaseDef = 
		NSManager::Get().GetTutorialIconDefinition(m_eInputID).m_obRenderDef;

	HudImageOverlaySlap obRenderDefHelperOverlay = 
		obBaseDef.GetBaseOverlay();

	obRenderDefHelperOverlay.m_fScale = 0.5f;
	obRenderDefHelperOverlay.m_obColour.W() = 0.4f;

	float fDX,fDY;
	GetMotionDirs(fDX,fDY);

	if (fDX!=0.0f || fDY!=0.0f)
	{
		const float fMag = 0.2f;
		
		fDX*=fMag;
		fDY*=fMag;

		obRenderDefHelperOverlay.m_fTopLeftX+=fDX;
		obRenderDefHelperOverlay.m_fTopLeftY+=fDY;
	}

	return obRenderDefHelperOverlay;
}


//------------------------------------------------------------------------------------------
//!
//!	NSCommand::SetState
//!
//------------------------------------------------------------------------------------------
void NSCommand::SetState(INPUT_RENDER_STATE eState)
{
	if (eState!=m_eCurrInputState)
	{
		if (eState != INPUT_STATE_NONE)
		{
			if (m_pobRenderSet)
			{
				if (m_bTutorialMode)
				{
					if (!m_pobRenderSet->m_obRenderHelperPad)
					{
						if (eState==INPUT_STATE_LISTEN)
						{
							const NSManager::HudRenderDefinition& obDefArrow = 
								NSManager::Get().GetTutorialIconDefinition(m_eInputID);
							
							// do we have a valid helper icon for the listening phase?
							// the action command, for instance, has no helper icon.
							if (obDefArrow.m_bIsValid)
							{
								const HudImageRenderDef& obBaseDefArrow = 
										obDefArrow.m_obRenderDef;
	
								m_pobRenderSet->m_obRenderHelperArrowMotion =  
									static_cast<HudImageRenderer*>(CHud::Get().CreateHudElement(&obBaseDefArrow));
		
								m_pobRenderSet->m_obRenderHelperPad = 
									static_cast<HudImageRenderer*>(CHud::Get().CreateHudElement(&NSManager::Get().GetPadIconDefinition().m_obRenderDef));
		
								const HudImageOverlaySlap obRenderDefHelperOverlay = 
								GetTutorialBaseAppliedOverlay();
							
								// apply initial scale before entering hud active.
								m_pobRenderSet->m_obRenderHelperPad->ApplyOverlay(obRenderDefHelperOverlay,
																					HudImageOverlay::OF_SCALE);
								m_pobRenderSet->m_obRenderHelperPad->BeginEnter();
							
								if (m_eInputID!=NS_ACTION)
								{
									// now apply movement overlay for the direction of motion over 5 secs.
									m_pobRenderSet->m_obRenderHelperPad->ApplyOverlay(obRenderDefHelperOverlay,
																							HudImageOverlay::OF_POS 	|
																							HudImageOverlay::OF_COLOUR,
																					HudImageRenderer::OA_TRANS,
																					0.5f);
								}
	
								m_pobRenderSet->m_obRenderHelperArrowMotion->BeginEnter();
	
							}
	
							CHud::Get().CreateMessageBox(GetTutorialString(), 0.5f, 0.5f);
						}
						else
						{
							// scee.sbashow : this is the case when a correct or incorrect action has been taken when there was no helper icon
							// 						for the listening phase. In which case, we only have the message box to remove
                            CHud::Get().RemoveMessageBox();
						}
					}
					else
					{
						if (eState!=INPUT_STATE_LISTEN)
						{
							// scee.sbashow :this is the case when a correct or incorrect action has been taken when there *was* a helper icon
							// 					for the listening phase. In which case, we have the message box to remove as well as the intimating icon.
							
							HudImageOverlaySlap obRenderDefHelperOverlay;

							// make sure it picks up whatever the current overlay colour is...
							HudImageOverlaySlap::ApplyImmOverlay(obRenderDefHelperOverlay,
																	   m_pobRenderSet->m_obRenderHelperPad->GetCurrentOverlay(),
																	   HudImageOverlay::OF_COLOUR);

							// scee.sbashow : fade out tutorial icon if not listening for command anymore.
							obRenderDefHelperOverlay.m_bRendering = false;
							obRenderDefHelperOverlay.m_obColour.W() = 0.0f;
							obRenderDefHelperOverlay.m_fScale = 1.0f;
							m_pobRenderSet->m_obRenderHelperPad->ApplyOverlay(obRenderDefHelperOverlay, 
																				   HudImageOverlay::OF_RENDFLAG | 
																				   HudImageOverlay::OF_COLOUR 	| 
																				   HudImageOverlay::OF_SCALE, 
																				   HudImageRenderer::OA_TRANS,
																				   0.2f );

                            CHud::Get().RemoveMessageBox();
						}
					}
				}

				if (!m_bTutorialMode || 
					!m_pobRenderSet->m_obRenderHelperPad)
				{
					// scee.sbashow : for the current placeholders- 
					// 					to be deprecated by the GhostGirl
					if (!m_pobRenderSet->m_obRenderInstance)
					{
						const HudImageRenderDef& obBaseDef = 
							NSManager::Get().GetCommandIconDefinition(m_eInputID).m_obRenderDef;

						m_pobRenderSet->m_obRenderInstance = 
							static_cast<HudImageRenderer*>(CHud::Get().CreateHudElement(&obBaseDef));

						m_pobRenderSet->m_obRenderDefOverlay = obBaseDef.GetBaseOverlay();
						m_pobRenderSet->m_obRenderInstance->BeginEnter();
					}
				}


			}
		}
		else
		{
			if (m_pobRenderSet)
			{
				// scee.sbashow : for the current placeholders- 
				// 					to be deprecated by the GhostGirl
				if (m_pobRenderSet->m_obRenderInstance)
				{
					m_pobRenderSet->m_obRenderInstance->BeginExit();

					CHud::Get().RemoveHudElement(	m_pobRenderSet->m_obRenderInstance	);
					m_pobRenderSet->m_obRenderInstance = 0;
				}

				if (m_bTutorialMode && m_pobRenderSet->m_obRenderHelperPad)
				{
					m_pobRenderSet->m_obRenderHelperPad->BeginExit();
					m_pobRenderSet->m_obRenderHelperArrowMotion->BeginExit();
		
					CHud::Get().RemoveHudElement(m_pobRenderSet->m_obRenderHelperPad);
					m_pobRenderSet->m_obRenderHelperPad = 0;
					CHud::Get().RemoveHudElement(m_pobRenderSet->m_obRenderHelperArrowMotion);
					m_pobRenderSet->m_obRenderHelperArrowMotion = 0;
				}
			}
		}

		m_eCurrInputState = eState;
	}

}

//------------------------------------------------------------------------------------------
//!
//!	NSCommand::UpdateEffects
//!
//! scee.sbashow - currently, applying immediate overlay effect for correct/wrong - 
//!			but these are for icons that will be deprecated
//!
//------------------------------------------------------------------------------------------
void NSCommand::UpdateEffects(float fDt, float fEffectPeriod, float fCurrentEffectTime)
{

	#ifdef NS_HDR
		const float fHDR = NSManager::Get().GetButtonSpec()->m_fHDRBase;
	#else
		const float fHDR = 1.0f;
	#endif
		const CVector obBaseColour( fHDR, fHDR, fHDR, 1.0f );
		const CVector obRedColour( fHDR, 0.0f, 0.0f, 1.0f );

	HudImageOverlaySlap& obSlap = m_pobRenderSet->m_obRenderDefOverlay;

	switch(this->GetCurrState())
	{
		case INPUT_STATE_CORRECT:
		{
			// scee.sbashow : for the current placeholders- 
			// 					to be deprecated by the GhostGirl
			if (m_pobRenderSet->m_obRenderInstance)
			{
				// reset in case any overlays had been applied during listening phase.
				m_pobRenderSet->m_obRenderInstance->RemoveAnyIncrementalOverlays();				

				// alpha modifier specified by seconds to fade completely
				const float fAlphaCorrect = 
					1.0f - (fCurrentEffectTime * (1.0f / NSManager::Get().GetButtonSpec()->m_fCorrect_fade_rate));
				obSlap.m_obColour = obBaseColour;
				obSlap.m_obColour.W() = fAlphaCorrect > 0.0f? fAlphaCorrect:0.0f;
				obSlap.m_fScale*=
					(1.0f + fDt * 0.5f * NSManager::Get().GetButtonSpec()->m_fCorrect_expand_rate);
				m_pobRenderSet->m_obRenderInstance->ApplyOverlay( obSlap);
			}
		}
		break;
		case INPUT_STATE_WRONG:
			{
				// scee.sbashow : for the current placeholders- 
				// 					to be deprecated by the GhostGirl
				if (m_pobRenderSet->m_obRenderInstance)
				{
					// reset in case any overlays had been applied during listening phase.
					m_pobRenderSet->m_obRenderInstance->RemoveAnyIncrementalOverlays();				
	
					// alpha modifier specified by seconds to fade completely
					const float fAlphaWrong = 
						1.0f - (fCurrentEffectTime * (1.0f / NSManager::Get().GetButtonSpec()->m_fWrong_fade_rate));;
					obSlap.m_obColour = obRedColour;
					obSlap.m_obColour.W() = fAlphaWrong > 0.0f? fAlphaWrong:0.0f;
					obSlap.m_fTopLeftX+= BUTTONSPEC->m_fWobbleXAmplitude * sinf( fCurrentEffectTime * (BUTTONSPEC->m_fWobbleXRate * TWO_PI) )*0.5f;
					obSlap.m_fTopLeftY+= BUTTONSPEC->m_fWobbleYAmplitude * sinf( fCurrentEffectTime * (BUTTONSPEC->m_fWobbleYRate * TWO_PI) )*0.5f;
					m_pobRenderSet->m_obRenderInstance->ApplyOverlay( obSlap);
				}
			}
		break;
		case INPUT_STATE_LISTEN:
			{
				if (m_bTutorialMode && 
					m_pobRenderSet->m_obRenderHelperPad)
				{
					// has the element stopped the last base transition (in this case moving it in the direction of motion
					// not that it needs to be - the incremental overlays overlay on top of the base overlay
					// but want this condition here as want to know when base overlay motion ends (intimating which direction to move),
					// to then apply a small, friendly, shake of anxiety ;_)
					if (m_pobRenderSet->m_obRenderHelperPad->IsActive())
					{
						if (!m_pobRenderSet->m_obRenderHelperPad->HasIncrementalOverlays())
						{
							if (!m_pobRenderSet->m_obRenderHelperPad->InOverlayTransition())
							{

								// scee.sbashow - s'ppose I could scope these class 
								//			names rather than having large names ;-)

								// for now use the base overlay to get the motion dir.
								const HudImageOverlaySlap obRenderDefHelperOverlay = 
									GetTutorialBaseAppliedOverlay();

								// applying a shake once the first applied overlay has complete.
								// will apply another base overlay to make it opaque, see comments and code just below. 

								// oscillate with magnitude 0.1f in X direction, 
								// every 0.2th of a second
								HudImageOverlayIncremental obIncremental(HudImageOverlay::OM_OSCILLATE,0.1f);

								obIncremental.m_iFilter = HudImageOverlay::OF_POS;

								GetMotionDirs(obIncremental.m_fDX,obIncremental.m_fDY);

								obIncremental.m_fDX *= 0.02f;
								obIncremental.m_fDY *= 0.02f;

								HudImageOverlaySlap obOverlayCurr(m_pobRenderSet->m_obRenderHelperPad->GetCurrentOverlay());
								obOverlayCurr.m_obColour.W()=1.0f;

								// after the motion has intimated in what direction to move (the first applied overlay from GetTutorialBaseAppliedOverlay(),
								//	 see above and below for its application) which faded the icon a little as well as moved it), we make it opaque again
								m_pobRenderSet->m_obRenderHelperPad->ApplyOverlay(obOverlayCurr,
																					   HudImageOverlay::OF_COLOUR,
																						HudImageRenderer::OA_TRANS,
																						fEffectPeriod*0.1f);

								m_pobRenderSet->m_obRenderHelperPad->ApplyIncrementalOverlay(obIncremental);
							}
						}
						else
						{
							if (m_eInputID!=NS_ACTION)
							{

								// if it has incremental overlays (ie the shake), but is not in an overlay transition, it must have opaqued to normal.
								if (!m_pobRenderSet->m_obRenderHelperPad->InOverlayTransition())
								{
									// reset to the original motion/fade overlay once the opaque transition has complete.
										m_pobRenderSet->m_obRenderHelperPad->Reset(0.0f);
	
									const HudImageOverlaySlap obRenderDefHelperOverlay = 
										GetTutorialBaseAppliedOverlay();
								
										m_pobRenderSet->m_obRenderHelperPad->ApplyOverlay(obRenderDefHelperOverlay,HudImageOverlay::OF_SCALE);
	
										m_pobRenderSet->m_obRenderHelperPad->ApplyOverlay(obRenderDefHelperOverlay,
																							HudImageOverlay::OF_POS|HudImageOverlay::OF_COLOUR|HudImageOverlay::OF_SCALE,
																							HudImageRenderer::OA_TRANS,
																							0.5f);
								}
								else
								{
									// dampen the shake for the period that the overlay becomes opaque again
									m_pobRenderSet->m_obRenderHelperPad->DampenIncrementals(0.95f);
								}
							}
						}
					}

				}

				if (m_eInputID==NS_ACTION)
				{
					// scee.sbashow : for the current placeholders- 
					// 					to be deprecated by the GhostGirl
					if (m_pobRenderSet->m_obRenderInstance)
					{
						if (m_pobRenderSet->m_obRenderInstance->IsActive())
						{
							if (!m_pobRenderSet->m_obRenderInstance->HasIncrementalOverlays())
							{
								HudImageOverlayIncremental obIncremental(HudImageOverlay::OM_OSCILLATE,fEffectPeriod*0.1f);
	
								//
								obIncremental.m_iFilter = HudImageOverlay::OF_COLOUR;
								obIncremental.m_obDColour = CVector(1.0f,1.0f,1.0f,0.0f);
	
								ntPrintf("Applying colour overlay to ns action icon!\n");
								m_pobRenderSet->m_obRenderInstance->ApplyIncrementalOverlay(obIncremental);
	
							}
						}
					}
				}

				return;
			}
		break;
		default:
			return;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSConditionStorage::SetCommandState
//!
//------------------------------------------------------------------------------------------

void NSConditionStorage::SetCommandState( NSCommand::INPUT_RENDER_STATE eState )
{
	for( ntstd::Vector<NSCommand>::iterator it = m_obCommandInputList.begin(); 
		 it != m_obCommandInputList.end(); ++it )
	{
		NSCommand& obCommand = *it;
		obCommand.SetState(eState);
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::AddInstance
//!  since NSPackages depend on mapped ents, and there can be multiples of these sharing the bacis NS package, we make
//!		a NSPackageInstance which currently just wraps the entity which was used as the NS instance.
//!
//------------------------------------------------------------------------------------------
void NSPackage::AddInstance( NSEntityInstance* pobNSEntity )
{
	m_obInstanceList.push_back( pobNSEntity );
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::HasInstance
//!				Determines if this mapped ns entity was mapped as such 
//!				referring to the NS corresponding to this NS package.
//------------------------------------------------------------------------------------------
const NSEntityInstance* NSPackage::HasInstance( CEntity* pobNSEntity ) const
{

	ntstd::Vector<NSEntityInstance*>::const_iterator nsPI_it =
		ntstd::find(m_obInstanceList.begin(), 
					m_obInstanceList.end(), 
					pobNSEntity);
	
	if (nsPI_it!=m_obInstanceList.end())
	{
	   return static_cast<const NSEntityInstance*>(pobNSEntity);
	}

	return 0;
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::SetActiveInstance
//!				Makes this instance of the NSPackage - active. The Rotational info from the mapped entity will be used
//!				To determine how to orientate the NS from local to world space.
//------------------------------------------------------------------------------------------
void NSPackage::SetActiveInstance( CEntity* pobNSEntity )
{
	if (!pobNSEntity)
	{
		m_pobActiveInstance = 0;
		return;
	}
	else
	if (const NSEntityInstance* const pobEInst = HasInstance(pobNSEntity))
	{
		m_pobActiveInstance = pobEInst;
		return;
	}

	user_error_p(0, ("Cannot set NSPackage active instance as this entity seems not to be mapped as an instance of the NSPackage it should refer to!!!"));
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::RemoveNSAnimContainers
//! Removes animations from entity anim tables that were added by the NS
//!
//------------------------------------------------------------------------------------------
void NSPackage::RemoveNSAnimContainers()
{
	for( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); ++nse_it )
	{
		if( (*nse_it)->m_bAnimContainerAdded )
		{
			DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( (*nse_it)->m_pAnimContainer );
			if ( pDO )
			{
					CEntity* pEntity = GetGameEntityPtr( *nse_it );
				if( pEntity )
					pEntity->UninstallNSAnims( CHashedString(pDO->GetName()) );
			}
		}
	}
}


//----------------------------------------------------------------------------------------
//!
//!	NSPackage::RemoveNSBSAnimContainers
//! Removes blendshape animations from entity anim tables that were added by the NS
//!
//------------------------------------------------------------------------------------------
void NSPackage::RemoveNSBSAnimContainers()
{
	for( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); ++nse_it )
	{
		if ( (*nse_it)->m_bBSAnimContainerAdded )
		{
			DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( (*nse_it)->m_pBSAnimContainer );
			if ( pDO )
			{
				CEntity* pEntity = GetGameEntityPtr( *nse_it );
				if( pEntity && pEntity->GetBlendShapesComponent() )
				{
					pEntity->GetBlendShapesComponent()->RemoveBSAnimsFromContainer( CHashedString(pDO->GetName()) );
					user_code_start(Ozz)
						ntPrintf("NS - %s: bsanim containers removed by ninja sequence\n", pEntity->GetName().c_str() );
					user_code_end()
				}
			}
		}
		(*nse_it)->m_bBSAnimContainerAdded  = false;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::RemoveNSBlendShapesComponents
//! Removes blendshapes components that were added by this NS
//!
//------------------------------------------------------------------------------------------
void NSPackage::RemoveNSBlendShapesComponents()
{
	for( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); ++nse_it )
	{
		if( (*nse_it)->m_bBlendShapesComponentAdded )
		{
			CEntity* pEntity = GetGameEntityPtr( *nse_it );
			if( pEntity && pEntity->GetBlendShapesComponent()  )
			{
				pEntity->UninstallBlendShapesComponent();
				user_code_start(Ozz)
					ntPrintf("NS - %s: blendshapes component removed by ninja sequence\n", pEntity->GetName().c_str() );
				user_code_end()
			}
		}
		(*nse_it)->m_bBlendShapesComponentAdded = false;
	}
}

void NSPackage::RemoveNSBSClumps( void )
{
	for( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); ++nse_it )
	{
		if( (*nse_it)->m_bBSClumpAdded )
		{
			CEntity* pEntity = GetGameEntityPtr( *nse_it );
			if( pEntity )
			{
				ntError( pEntity->GetBlendShapesComponent() );
				pEntity->GetBlendShapesComponent()->PopBSSet();
				user_code_start(Ozz)
					ntPrintf("NS - %s: bsclump %s removed by ninja sequence\n", pEntity->GetName().c_str() , (*nse_it)->m_sBSClump.c_str() );
				user_code_end()
			}
		}
		(*nse_it)->m_bBSClumpAdded = false;
	}
}

void NSPackage::RemoveAllNSBlendShapesRelatedStuff( void )
{
	RemoveNSBSAnimContainers();
	RemoveNSBSClumps();
	RemoveNSBlendShapesComponents();
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::FindNSEntFromName
//! Scans through a package's NSEnt list look for a matching name and returns the NSEnt
//! Returns NULL if not found
//! 
//------------------------------------------------------------------------------------------

NSEnt* NSPackage::FindNSEntFromName( const char* pcName )
{
	for( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); ++nse_it )
	{
		NSEnt* pEnt = *nse_it;
		if( !strcmp( pEnt->m_sName.c_str(), pcName ) )
		{
			return pEnt;
		}
	}

	return NULL;
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::RestoreEntities
//! Restore pre-existing entities to their former state
//!
//------------------------------------------------------------------------------------------
void NSPackage::RestoreEntities()
{
	RemoveAllNSBlendShapesRelatedStuff();

	for( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); ++nse_it )
	{
		NSEnt* pEnt = *nse_it;
		CEntity* pEntity = GetGameEntityPtr( pEnt );
		if( pEntity )
		{
			bool bRemovedAnims = false;

			// clear NS movement controllers and unblock			
			if( pEntity->GetMovement() )
			{
				pEntity->GetMovement()->ClearControllers();
				pEntity->GetMovement()->BlockNewControllers( false );

				if( pEnt->m_pGameEntity->GetAnimator() )
				{
					bRemovedAnims = true;
					pEnt->m_pGameEntity->GetAnimator()->RemoveAllAnimations();
				}
			}

			// Make characters upright
			if ( pEntity->IsCharacter() )
			{
				SetUprightCharacterRotation( pEntity->ToCharacter() );
			}

			// if restarting the level then destroy all ninja sequence created entities,
			// otherwise only destroy those which have been set as invisible at the end of
			// the NS
			if( (NSManager::Get().IsRestarting() && pEnt->m_bNSEntity) || pEntity->GetRenderableComponent()->DisabledByGame() )
			{
				// as object destroying isn't working properly yet...
				if( pEnt->m_bInvisibleAfterClip )
				{
					ShowEntity( pEnt->m_pGameEntity, false );
				}
			}
			else
			{
				// restore persistant entities to former state
				if( pEnt->m_bHadAIEnabled )
				{
					ntError( pEntity->IsAI() );
					((AI*)pEntity)->GetAIComponent()->SetDisabled( false );
				}
				if( !pEnt->m_bHadMovementComp )
				{
					// remove movement component?
				}
				if( !pEnt->m_bHadAnimatorComp )
				{
					// remove animation component?
				}
				if( pEnt->m_pGameEntity->GetAnimator() )
				{
					bRemovedAnims = true;
					pEnt->m_pGameEntity->GetAnimator()->RemoveAllAnimations();
				}
				
				// clear the ninja sequence flag on the entity
				pEntity->SetInNinjaSequence( false );
			
				// renable attack component if it had one
				if( pEnt->m_bHadAttackComp )
				{
					pEntity->GetAttackComponent()->SetDisabled( false );


#if defined( TGS_NS_HACKS )
					// TGS HACK TGSHACK!
					if( m_pcClipName && pEntity->IsPlayer() && strstr( "NS_Catapult1_failure3", m_pcClipName ) )
					{
						pEntity->GetAttackComponent()->StartFlooredState();
					}
#endif
				}
					
				// If the entity has a message handler inform them that external control is over
				if ( pEntity->GetMessageHandler() )
				{
					if( NS_DISCARD == GetNSStage() )
					{
						if( m_bPlayerKilled )
						{
							CMessageSender::SendEmptyMessage( CHashedString(HASH_STRING_MSG_NINJA_SEQUENCE_DIE), pEntity->GetMessageHandler() );
						}
						else if( m_bFailed )
						{
							CMessageSender::SendEmptyMessage( CHashedString(HASH_STRING_MSG_NINJA_SEQUENCE_FAIL), pEntity->GetMessageHandler() );
						}
					}

					CMessageSender::SendEmptyMessage( CHashedString(HASH_STRING_MSG_EXTERNAL_CONTROL_END), pEntity->GetMessageHandler() );
				}
					
				if( pEnt->m_bInvisibleAfterClip )
				{
					ShowEntity( pEnt->m_pGameEntity, false );
				}
			}
		}
        
		// Finally de-activate any special effects we may need (like ghost girl)
		if ((pEnt->m_pEffectController) && (pEnt->m_pEffectController->Active()))
			pEnt->m_pEffectController->Deactivate();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::SetUprightCharacterRotation
//! Sets the an in-game character to an upright position
//!
//------------------------------------------------------------------------------------------
void NSPackage::SetUprightCharacterRotation( Character* pCharacter )
{
	ntAssert( pCharacter );

	CQuat obRotQuat = pCharacter->GetRotation();

	CMatrix obNewMatrix( obRotQuat );

	// In the new matrix, Y is up.
	CDirection obNewYAxis( 0.0f, 1.0f, 0.0f );

	// Get the Z axis and remove any Y component from it
	CDirection obNewZAxis = obNewMatrix.GetZAxis();
	obNewZAxis.Y() = 0.0f;
	obNewZAxis.Normalise();

	// Put new axes into the matrix
	obNewMatrix.SetYAxis( obNewYAxis );
	obNewMatrix.SetZAxis( obNewZAxis );

	// Regenerate X axis
	obNewMatrix.BuildXAxis();

	// Create the new rotation quat
	CQuat obNewRotQuat( obNewMatrix );

	// Set the new rotation on the player
	pCharacter->SetRotation( obNewRotQuat );
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::CreateNSEntity
//! Creates an entity during the sequence
//! Also name matches to use entities currently existing in the level
//!
//| \return Ptr to the newly created entity
//! \param pcName	Name of the entity
//!
//------------------------------------------------------------------------------------------

CEntity* NSPackage::CreateNSEntity( NSEnt* pEnt )
{
	// CMicroTimer obTimer;
	// obTimer.Start();

	// If we're a camera than create a cameraanimator instead...
	if (strcmp(pEnt->m_sName.c_str(), "thecamera") == 0)
	{
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(pEnt->m_pAnimContainer);
		CoolCam_MayaAnimator* pCameraAnimator = NT_NEW CoolCam_MayaAnimator( ntStr::GetString(pEnt->m_sClump), ntStr::GetString(pDO->GetName()));
		pCameraAnimator->InstallGetName(NT_NEW SpecificFunctor<NSPackage, ntstd::String, true>(this, &NSPackage::GetName));
		pEnt->m_pCameraAnimator = pCameraAnimator;
		return 0;
	}

	// first see if the entity already exists nearby
	const char* pcEntName = pEnt->m_sName.c_str();
	CEntity* pEntity = CEntityManager::Get().FindEntity( pcEntName );
	if( !pEntity )
	{
		// entity doesn't already exist so create it from a lua table
		NinjaLua::LuaObject table;
		table.AssignNewTable(CLuaGlobal::Get().State());
		table.Set("ForceLuaTable", false);

		table.Set("Name", pcEntName );
		table.Set("Clump", pEnt->m_sClump.c_str() );
		table.Set("ConstructionScript", "Animated" );
		table.Set("DefaultDynamics", "Animated" );
		table.Set("AnimToPlay", "" );
		table.Set("AnimationContainer", "" );
		table.Set("SectorBits", MappedAreaInfo());


		// sooo... what to do with these?
		//table.Set("BSClump", "" );
		//table.Set("BSAnimContainer", "" );

		LuaAttributeTable temp(table); 
		CreateEntityFromLuaAttributeTable(&temp);
		pEntity = CEntityManager::Get().FindEntity( pcEntName );
		user_error_p( pEntity, ("NS: Couldn't create entity %s\n", pcEntName ) );

		// flag that it was created by the ninja sequence (in case we want to quit it half way through
		// on level restart)
		pEnt->m_bNSEntity = true;

		//scee.sbashow: shouldn't appear until 
		// 	NS clip kicks off and SetVisible is called.
		ShowEntity(pEntity, false);
	}
	else 
	{
		pEnt->m_bNSEntity = false;
	}
/*
	// now see if we're one of the specially named entities that
	// indicates we're a ghost girl effect
	if (strcmp(pEnt->m_sName.c_str(), "ghostgirl_white") == 0)
	{
		user_error_p( m_ggWhite == 0, ("NS created by ent %s already has a white ghost girl\n", GetNSName().c_str()) );

		pEnt->m_pEffectController = NT_NEW GhostGirlController( GhostGirlController::GG_WHITE, pEntity, pEnt );
		m_ggWhite = pEnt->m_pEffectController;

		// set visibility of clump
		if (pEntity->GetRenderableComponent())
			pEntity->GetRenderableComponent()->AddRemoveAll_Game( false );
	}
	else if (strcmp(pEnt->m_sName.c_str(), "ghostgirl_blue") == 0)
	{
		user_error_p( m_ggBlue == 0, ("NS created by ent %s already has a blue ghost girl\n", GetNSName().c_str()) );

		pEnt->m_pEffectController = NT_NEW GhostGirlController( GhostGirlController::GG_BLUE, pEntity, pEnt );
		m_ggBlue = pEnt->m_pEffectController;

		// set visibility of clump
		if (pEntity->GetRenderableComponent())
			pEntity->GetRenderableComponent()->AddRemoveAll_Game( false );
	}
	else if (strcmp(pEnt->m_sName.c_str(), "ghostgirl_red") == 0)
	{
		user_error_p( m_ggRed == 0, ("NS created by ent %s already has a red ghost girl\n", GetNSName().c_str()) );

		pEnt->m_pEffectController = NT_NEW GhostGirlController( GhostGirlController::GG_RED, pEntity, pEnt );
		m_ggRed = pEnt->m_pEffectController;
	}
*/
	// check it has an animator
	if( !pEntity->GetAnimator() )
	{
		pEntity->InstallAnimator(CHashedString());
		pEnt->m_bHadAnimatorComp = false;
	}

	// check it has a movement component
	if( !pEntity->GetMovement() )
	{
		CMovement* pobMovement = NT_NEW CMovement( pEntity, pEntity->GetAnimator(), pEntity->GetPhysicsSystem() );
		pEntity->SetMovement( pobMovement );
		pEnt->m_bHadMovementComp = false;
	}

	// default
	pEnt->m_bInvisibleAfterClip = false;

	// Add its animation containers if they aren't already there
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(  pEnt->m_pAnimContainer );
	if( pDO && !pEntity->AreNSAnimsAdded( CHashedString(pDO->GetName()) ) )
	{
		pEntity->InstallNSAnims( CHashedString(pDO->GetName()) );
		pEnt->m_bAnimContainerAdded = true;
	}

	//if( XPUShapeBlending::Get().IsEnabled() )
	//{
	//	// entity needs new bsclump. We need to standarise this!
	//	if ( !pEnt->m_sBSClump.empty() && (pEnt->m_sBSClump != "") && (pEnt->m_sBSClump != "NULL" ) )
	//	{
	//		// if entity does not have a bscomponent, install a new one an flag it as being owned by this NS
	//		if ( !pEntity->GetBlendShapesComponent() )
	//		{
	//			pEntity->InstallBlendShapesComponent();	
	//			pEnt->m_bBlendShapesComponentAdded = true;
	//			
	//			user_code_start(Ozz)
	//				ntPrintf( "NS - %s: blendshapes component added by ninja sequence\n", pEntity->GetName().c_str() );
	//			user_code_end()
	//		}
	//		// push the new bsclump as the current set
	//		pEnt->m_bBSClumpAdded = pEntity->GetBlendShapesComponent()->PushBSSet( pEnt->m_sBSClump.c_str() );
	//		user_code_start(Ozz)
	//			if ( pEnt->m_bBSClumpAdded )
	//				ntPrintf("NS - %s: bsclump %s added by ninja sequence\n", pEntity->GetName().c_str() , pEnt->m_sBSClump.c_str() );
	//		user_code_end()
	//	}

	//	// now add all those pretty blendshape animations from the NSBSAnimContainer if not already there
	//	if ( pEnt->m_pBSAnimContainer )
	//	{
	//		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pEnt->m_pBSAnimContainer );
	//		BlendShapesComponent* pBSComp = pEntity->GetBlendShapesComponent();
	//		//ntAssert_p( pBSComp, ("Cannot add bsanims from container. %s doesn't have a valid BlendShapesComponent\n", pEntity->GetName().c_str() ) );
	//		
	//		if ( pDO && pBSComp && !pBSComp->IsBSAnimContainerAdded(ntStr::GetString(pDO->GetName())) )
	//		{
	//			pBSComp->AddBSAnimsFromContainer( ntStr::GetString(pDO->GetName(), true) );
	//			pEnt->m_bBSAnimContainerAdded = true;
	//			user_code_start(Ozz)
	//				ntPrintf("NS - %s: bsanim container added by ninja sequence\n", pEntity->GetName().c_str() );
	//			user_code_end()
	//		}
	//	}
	//}

	// keep a ptr to the entity in NSEnt - have to be careful as this could dangle
	// so it's *only* used for removing from lists, not to access the class members
	pEnt->m_pGameEntity = pEntity;

	// obTimer.Stop();
	// ntPrintf( "******* Just spent %f setting up %s for a ninja sequence. *******\n", obTimer.GetFramePercent(), pcEntName );

	return pEntity;
}



//------------------------------------------------------------------------------------------
//!
//!	NSPackage::SetNSEntityForClip
//! This takes control of an entity for a nija sequence
//!
//------------------------------------------------------------------------------------------
void NSPackage::SetNSEntityForClip( NSEnt* pEnt )
{
	CEntity* pEntity = pEnt->m_pGameEntity;

	// Make sure we have a valid game entity
	user_error_p( pEntity, ( "This Ninja Sequence Entity %s has no valid game entity pointer.\n", pEnt->m_sName.c_str() ) );
	if ( !pEntity )
		return;


	if( XPUShapeBlending::Get().IsEnabled() )
	{
		// entity needs new bsclump. We need to standarise this!
		if ( !pEnt->m_sBSClump.empty() && (pEnt->m_sBSClump != "") && (pEnt->m_sBSClump != "NULL" ) )
		{
			// if entity does not have a bscomponent, install a new one an flag it as being owned by this NS
			if ( !pEntity->GetBlendShapesComponent() )
			{
				pEntity->InstallBlendShapesComponent();	
				pEnt->m_bBlendShapesComponentAdded = true;
				
				user_code_start(Ozz)
					ntPrintf( "NS - %s: blendshapes component added by ninja sequence\n", pEntity->GetName().c_str() );
				user_code_end()
			}
			// push the new bsclump as the current set
			pEnt->m_bBSClumpAdded = pEntity->GetBlendShapesComponent()->PushBSSet( pEnt->m_sBSClump.c_str() );
			user_code_start(Ozz)
				if ( pEnt->m_bBSClumpAdded )
					ntPrintf("NS - %s: bsclump %s added by ninja sequence\n", pEntity->GetName().c_str() , pEnt->m_sBSClump.c_str() );
			user_code_end()
		}

		// now add all those pretty blendshape animations from the NSBSAnimContainer if not already there
		if ( pEnt->m_pBSAnimContainer )
		{
			DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pEnt->m_pBSAnimContainer );
			BlendShapesComponent* pBSComp = pEntity->GetBlendShapesComponent();
			
			if ( pDO && pBSComp && !pBSComp->IsBSAnimContainerAdded(ntStr::GetString(pDO->GetName())) )
			{
				pBSComp->AddBSAnimsFromContainer( ntStr::GetString(pDO->GetName()), true );
				pEnt->m_bBSAnimContainerAdded = true;
				user_code_start(Ozz)
					ntPrintf("NS - %s: bsanim container added by ninja sequence\n", pEntity->GetName().c_str() );
				user_code_end()
			}
		}
	}

	// We only need to do most things if we are taking over an existing entity
	if ( !pEnt->m_bNSEntity )
	{
		// Disable the state system if the message control handles it
		if ( pEntity->GetMessageHandler() )
			CMessageSender::SendEmptyMessage( "msg_external_control_start", pEntity->GetMessageHandler() );

		// if entity is player then disable player input and put entity into ninja sequence state
		if( pEntity->IsPlayer() )
		{
			// scee.sbashow : using virtualised controller, so not disabling anymore
			// CInputComponent* pInput = pEntity->GetInputComponent();
			// pInput->SetDisabled( true );
			m_bPlayerDisabled = true;
		}

		// disable attack components as ProcessStrike can place new controllers onto the entity
		// after the ninja sequence has started
		if( pEntity->GetAttackComponent() )
		{
			pEntity->GetAttackComponent()->SetDisabled( true );
			pEnt->m_bHadAttackComp = true;
		}

		if( pEntity->GetMovement() ) pEntity->GetMovement()->ClearControllers();

		// If this character is in the ragdoll state then make sure they are not

		if ( pEntity->GetPhysicsSystem() )
		{
			Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) pEntity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );

			if( pobCharacterState )
			{
				if( ( pobCharacterState->IsRagdollActive() ) && ( pobCharacterState->GetRagdollState() != Physics::ANIMATED ) )
				{
					if (!pobCharacterState->IsCharacterControllerActive())
						pobCharacterState->ActivateCharacterController();
					
					pobCharacterState->SetCharacterControllerCollidable(true);
					pobCharacterState->SetRagdollDisabled();
				}
			}
		}

		// disable its AI if it has any
		if( pEntity->IsAI() )
		{
			((AI*)pEntity)->GetAIComponent()->SetDisabled( true );
			pEnt->m_bHadAIEnabled = true;
		}
	}

	// Set a flag to say that we are now in control of the entity
	pEntity->SetInNinjaSequence( true );
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::Update
//! Update of a NS package
//!
//------------------------------------------------------------------------------------------

void NSPackage::Update( float fDt )
{
	switch( GetNSStage() ) {

		case NS_TRIGGER: 
		{
			SetNSStage( NS_LOADING );
		}
		// intentional fall through
		
		case NS_LOADING:
		{
			// wait here until resources are ready...
			SetNSStage( NS_PLAYING );

			// scee.sbashow: is this always the hero at this stage?
			Player* const pPlayer = CEntityManager::Get().GetPlayer();

			// Check that the player is the hero
			if ( pPlayer != NULL && pPlayer->IsHero() )
			{
				// scee.sbashow : I hope so...
				Hero* const pHero = pPlayer->ToHero();

				// scee.sbashow : Put swords away.
				pHero->LeftSword_Away();
				pHero->RightSword_Away();
			}
		}
		// intentional fall through
		
		case NS_PLAYING:
		{
			// play the current clip
			if( !PlayClip( fDt ) )
			{
				// clean up the clip
				CloseClip();

				if( !m_pTargetClip )
				{
					// is it a player killed or failure sequence?
					if( m_bPlayerKilled )
					{
						// temp. just return to clip 0 and loop, display message
						//m_bPlayerKilled = false;
						//m_bFirstClip = true;
						//m_bClipStarted = false;
#ifndef _RELEASE
						OSD::Add( OSD::DEBUG_CHAN, 0xffff0000, "NS: PLAYER HAS BEEN KILLED." );
#endif
					}
					else if( m_bFailed )
					{
						// player's failed
#ifndef _RELEASE
						OSD::Add( OSD::DEBUG_CHAN, 0xffff0000, "NS: PLAYER HAS FAILED." );
#endif
					}
					else
					{
#ifndef _RELEASE
						OSD::Add( OSD::DEBUG_CHAN, 0xffff0000, "NS: PLAYER HAS SUCCEEDED." );
#endif
					}

					if (m_iLastModifiedCoolCamID>-1 && 
						SetupTransingOutCam(m_iLastModifiedCoolCamID))
					{
						m_iLastModifiedCoolCamID = -1;

                        // end of NS - trans out smoothly first
						SetNSStage( NS_TRANSINGOUT );
					}
					else
					{
						// the transition was made mid sequence
						if (m_iTransingOutLastCoolCamID>-1 &&  
							m_iLastModifiedCoolCamID == -1 )
						{
							// end of NS - trans out smoothly first
							SetNSStage( NS_TRANSINGOUT );
						}
						else
						{
							// end of NS
							SetNSStage( NS_DISCARD );
						}
					}
				}
				else
				{
					// trigger start of next clip
					m_bClipStarted = false;
				}
			}
			
			// display tick/cross if necessary
			//DisplayTickOrCross( fDt );
		}
		break;
        
		case NS_TRANSINGOUT:
		{
			// scee.sbashow - check - remove this if I check it in, as should have fixed the issue
			// 					that required this printf in the first place!
			ntPrintf("Current num cool cams = %d\n",CamMan::Get().GetView(m_iCamView)->GetNumCoolCams());
			ntError(m_iTransingOutLastCoolCamID>-1);
			
			CoolCamera* const pobCam = 
				CamMan::Get().GetView(m_iCamView)->GetCoolCam(m_iTransingOutLastCoolCamID);

            // scee.sbashow: if this is the case, the transition has been set off 
					// See camview.cpp's update on its list of CoolCam's
			if (!pobCam || pobCam->RemovingFromView())
			{
				// scee.sbashow: useful input to halt NS to test transitioning to game cam works ok.
					//CInputComponent* const pobICPlyr = CEntityManager::Get().GetPlayer()->GetInputComponent();
					//if (pobICPlyr->GetVPressed() & ( 1 << AB_ACTION ))
				{
					// end of NS
					SetNSStage( NS_DISCARD );
				}
			}
		}
		break;

		case NS_DISCARD:
		{
			// restore entities to their former state (and destroy any obsolete ones)
			RestoreEntities();

			// tell player NS over
			if( m_bPlayerDisabled )
			{
				CEntity* pPlayer = CEntityManager::Get().GetPlayer();
				if( pPlayer )
				{
					// scee.sbashow: since not disabled anymore, this is not required, but keeping anyway...
					// turn player input back on
					CInputComponent* pInput = pPlayer->GetInputComponent();
					pInput->SetDisabled( false );
				}
			}

			// scee.sbashow:  the last cool camera will transition out and finish by itself - no need
			//					to "RemoveCoolCamera" now.
			//					Only do so in cases where no transition time was set. 
			//					The NS_TRANSINGOUT state will never be passed through in these cases, so m_iLastModifiedCoolCamID>-1
			//					if a cool camera exists in the last clip.
			if (m_iLastModifiedCoolCamID>-1)
			{
				// 					return the camera back to normal
					CamMan::Get().GetView(m_iCamView)->RemoveCoolCamera(m_iLastModifiedCoolCamID);
					m_iLastModifiedCoolCamID = -1;
			}
			
			m_pCameraAnimator = 0;

			const NSEntityInstance* const pobNEEntInstance = this->GetActiveInstance();

			// send a message to the NS Entity so it can trigger a shutdown script
			if( m_bPlayerKilled )
			{
				CMessageSender::SendEmptyMessage( "msg_ninja_sequence_die", pobNEEntInstance->GetMessageHandler() );
			}
			else if( m_bFailed )
			{
				CMessageSender::SendEmptyMessage( "msg_ninja_sequence_fail", pobNEEntInstance->GetMessageHandler() );
				if ( StyleManager::Exists() && m_bHasInteractiveClip)
				{
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTFAILNS) /*, optional override default style score */);
				}
			}
			else
			{
				CMessageSender::SendEmptyMessage( "msg_ninja_sequence_success", pobNEEntInstance->GetMessageHandler() );
				if ( StyleManager::Exists() && m_bHasInteractiveClip)
				{
					StyleManager::Get().RegisterStyleEvent( CHashedString(HASH_STRING_EVENTSUCCESSNS) /*, optional override default style score */);
				}
			}
		}
		break;
	}
}


void NSPackage::UpdateTutorialScalars( float fDT )
{
	const float fCatchRate = fDT*3.0f;
	if (m_bTestInputOverLayBeingRequested)
	{
		if (!m_bMeasureAverageNumberPressesMet)
		{
			m_fTutorialScalarCurr+=
				(GetActiveInstance()->GetTutorialBaseScalar() -m_fTutorialScalarCurr)*fCatchRate;
		}
		else
		{
			// ok, but keep only as slow as the proper proximity is not being met...
			//	m_fProximityToFrequencyPassing will verge on 1.0f if it is being met.
			m_fTutorialScalarCurr += 
				(m_fProximityToFrequencyPassing - m_fTutorialScalarCurr)*fCatchRate;
		}
	}
	else
	{
		m_fTutorialScalarCurr+=(1.0f - m_fTutorialScalarCurr)*fCatchRate;
	}

	m_fTutorialScalarCurr = max(0.0f,min(1.0f,m_fTutorialScalarCurr));
}


//------------------------------------------------------------------------------------------
//!
//!	NSManager::PlayClip
//! Updates a clip
//!
//! \return false if clip has finished
//! \param pobNS	Pointer to the NS entity
//! \param fDt		Time step
//!
//------------------------------------------------------------------------------------------

bool NSPackage::PlayClip( float fDt )
{
	// update package clip duration
	UpdateTime( fDt );

	if( !m_bClipStarted )
	{
		StartClip();

		if (!m_pcCameraAnimName)
		{
			if (m_iLastModifiedCoolCamID>-1)
			{
				if (!SetupTransingOutCam(m_iLastModifiedCoolCamID))
				{
					// if we couldn't transition out, then remove cam
					CamMan::Get().GetView(m_iCamView)->RemoveCoolCamera(m_iLastModifiedCoolCamID);
				}
				m_iLastModifiedCoolCamID = -1;
			}
		}
	}
	else
	{
		// start any pending cool cam off a frame later.
		if (m_pcCameraAnimName)
		{
			ModifyCamera();
		}
	}
	
#ifdef _DISPLAY_DEBUG_INFO

	const NSEntityInstance* const pobNEEntInstance = this->GetActiveInstance();

	g_VisualDebug->Printf2D(50.0f, 320.0f, DC_WHITE, 0, "Sequence: %s", pobNEEntInstance->GetName().c_str() );
	
	if( m_pcClipName ) g_VisualDebug->Printf2D(50.0f, 340.0f, DC_WHITE, 0, "Clip: %s Clip timer: %.2f Clip duration: %.2f Total time: %.2f", 
							m_pcClipName, m_fClipTime, m_pClip->m_fDuration, m_fTotalTime );

	float debugy = 360.0f;
	for ( ntstd::List< void* >::const_iterator it = m_pClip->m_EventList.begin(); it != m_pClip->m_EventList.end(); ++it )
	{
		NSEvent* pEvent = (NSEvent*)( *it );
		g_VisualDebug->Printf2D(50.0f, debugy, DC_WHITE, 0, "Event: %s, Trigger: %.2f", pEvent->m_sEvent.c_str(), pEvent->m_fTriggerTime );
		debugy += 20.0f;
	}
#endif

	// DEBUG: check the camera is in sync
	// CheckCameraInSync();

	//DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( m_pClip );
	//const char* pName = pDO->GetName().c_str();
	//ntPrintf( "NS: Clip %s : Default Target %s : Cond. target %s\n", pName, m_pClip->m_sDefaultGotoClip.c_str(), m_pcTargetClipName );


	// default value - will be determined in TestConditions()
	m_bTestInputOverLayBeingRequested = false;

	// Check conditions if in window
	if( InWindow() )
	{
		TestConditions( fDt );

		if (GetActiveInstance()->GetNSMode() == NinjaSequence::NS_MODE_TUTORIAL)
		{
			UpdateTutorialScalars( fDt );

			// scee.sbashow:
			//			SetSpecialTimeScalar is used by the fighting - 
			//					but NS's do not conflict with those, so should be fine.
			CTimer::Get().SetSpecialTimeScalar( GetCurrentTutorialScalar() );
		}

	}
	else 
	{
		if (GetActiveInstance()->GetNSMode() == NinjaSequence::NS_MODE_TUTORIAL)
		{
			 CTimer::Get().SetSpecialTimeScalar( 1.0f );
		}

		if( PostWindow() )
		{
			PostProcessConditions();
		}
	}
	

	UpdateCommandFeedback(fDt);

	// update any indicator effects we may be using
	//	m_indicatorEffects.Update( InWindow(), m_PCList );

	UpdateDisplay( fDt );

	// process new correct conditions
	ProcessCorrectConditions();

	// check events
	ParseEvents();

	if( m_pTargetClip )
	{
		// for non-final clips just test time as it's quicker
		return( !ClipEnd() );
	}
	else
	{
		// otherwise make sure the animations have finished on all (major only if any present) entities 
		// (i.e. last frame plays so root joint is at the correct orientation when NS hands back control)
		bool bAnimsCompleted( true );
		bool bHasMajorEntities = ContainsMajorEntities();
		for( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); ++nse_it )
		{
			const NSEnt* const pobNSEnt = *nse_it;
			const CEntity* pEntity = GetGameEntityPtr( pobNSEnt );
			
			// This might be a camera animator rather than an entity...
			if(!pEntity)
			{
				// is it an animating cam?
				if (pobNSEnt->m_pCameraAnimator && 
					pobNSEnt->m_pCameraAnimator->GetAnimator())
				{
					// and if there's only one NSEnt...
					if (m_EntityList.size()==1)
					{
						// then, in which case, consider as a valid anim
						// but only because it's the only NSEnt around. This is special behaviour, 
						// as it is assumed that if the nsent cam is around only, then
						// its anim defines the clip.
						bAnimsCompleted = 
							!pobNSEnt->m_pCameraAnimator->GetAnimator()->IsPlayingAnimation();
						break;
					}
				}

				continue;
			}

			user_error_p( pEntity->GetMovement(), ("NS Entity %s must have movement component!!!", pEntity->GetName().c_str()) );
			if( !(pEntity->GetMovement()->HasMovementCompleted()) && (!bHasMajorEntities || pEntity->GetAttackComponent()) )
			{
				bAnimsCompleted = false;
				break;
			}
		}
		
		// this should never happen, but it's here just in case
		if( m_fClipTime > (m_pClip->m_fDuration + 0.5f ) )
		{
			ntPrintf( "NS ERROR! Anim not completing.\n" );
			bAnimsCompleted = true;
		}
		 
		// return true if anims are still ongoing...
		// ... or if there is a camera pending... - JML
		return( !bAnimsCompleted || m_pcCameraAnimName );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::ContainsMajorEntities
//! Checks whethe this NS contains entities with an AttackComponent (i.e. any pEntity->GetAttackComponent() != 0 )
//!
//------------------------------------------------------------------------------------------
bool NSPackage::ContainsMajorEntities()
{
	for( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); ++nse_it )
	{
		const CEntity* pEntity = GetGameEntityPtr( *nse_it );

		// This might be a camera animator rather than an entity...
		if(!pEntity) continue;

		ntAssert( pEntity->GetMovement() );
		if( pEntity->GetAttackComponent() )
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::ParseEvents
//! Parses the name corresponding to an event in acWorkString
//! Return value is pcEvent incremented to next token in event command.
//---------------------------------------------------------------------------
const char* NSPackage::ParseNSEntName(const char* pcEvent, 
									  char* acWorkString, 
									  int iWorkspaceLen)
{
	// get entity name
	int iIdx = 0;
	while( *pcEvent != ',' )
	{
		acWorkString[ iIdx++ ] = *pcEvent++;
		ntError( iIdx < iWorkspaceLen );
	};
	acWorkString[ iIdx ] = 0;
	pcEvent++;	// skip comma to point to transform

	return pcEvent;
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::ParseEvents
//! Parses the events
//!
//------------------------------------------------------------------------------------------

void NSPackage::ParseEvents()
{
	const int iWorkspaceLen = 64;
	char aWorkString[iWorkspaceLen];	// handy little workspace

	enum COMMAND_IDS
	{
		CID_BEGIN = 0,
		CID_VISIBLE = CID_BEGIN,
		CID_INVISIBLE,
		CID_KILL
	};

	for ( ntstd::List< NSEvent* >::const_iterator it = m_pClip->m_EventList.begin(); it != m_pClip->m_EventList.end(); ++it )
	{
		NSEvent* pEvent = ( *it );

		if (pEvent->m_bTriggered)
		{
		}
		else
		if (pEvent->m_fTriggerTime > m_pClip->m_fDuration )
		{
			const char* pcTestEventString = NSManager::Get().GetEventString( CID_INVISIBLE );
			user_error_p( *pcTestEventString != '*', ("NS: Unknown event String") );
			const char* pcEvent = strstr( pEvent->m_sEvent.c_str() , pcTestEventString );
			if( pcEvent )
			{
				pcEvent += strlen( pcTestEventString );

				pcEvent = ParseNSEntName( pcEvent, aWorkString, iWorkspaceLen );
				NSEnt* const pobNSEnt = FindNSEntFromName( aWorkString );
				user_error_p( pobNSEnt, ("Entity name %s incorrect for NS (%s), (current target clip is %s), for set_visible event command!!", aWorkString, GetNSName().c_str(), m_pcTargetClipName ));
				pobNSEnt->m_bInvisibleAfterClip = true;
			}

			pEvent->m_bTriggered = true;
		}
		else
        // check time or if already triggered
		if( (pEvent->m_fTriggerTime <= m_fClipTime))
		{
			const char* pcEventName = pEvent->m_sEvent.c_str();
			//ntPrintf( "NS: Event %s, %.2f\n", pcEventName, pEvent->m_fTriggerTime );

			// scan for command
			int iEvent = CID_BEGIN;
			do
			{
				const char* pcTestEventString = NSManager::Get().GetEventString( iEvent );
				user_error_p( *pcTestEventString != '*', ("NS: Unknown event String") );

				const char* pcEvent = strstr( pcEventName, pcTestEventString );
				if( pcEvent )
				{
					pcEvent += strlen( pcTestEventString );
					pEvent->m_bTriggered = true;

					// execute it
					switch( iEvent )
					{
						// ---------------------------- SET_VISIBLE command

						case CID_VISIBLE: {
							
							pcEvent = ParseNSEntName( pcEvent, aWorkString, iWorkspaceLen );

							// find NS ent first, see if it's really an effect entity
							NSEnt* const pobNSEnt = FindNSEntFromName( aWorkString );
							user_error_p( pobNSEnt, ("Entity name %s incorrect for NS (%s), (current target clip is %s), for set_visible event command!!", aWorkString, GetNSName().c_str(), m_pcTargetClipName ));

							if (pobNSEnt && pobNSEnt->m_pEffectController)
							{
								pobNSEnt->m_pEffectController->Activate();
							}
							else
							{
								// find entity
								CEntity* const pEntity = 
									CEntityManager::Get().FindEntity( aWorkString );
								if (pEntity)
								{
									if( !strcmp( pcEvent, "null" ) )
									{
										// show the whole entity
										ShowEntity( pEntity, true );
									}
									else
									{
										ShowTransform( pEntity, pcEvent, true );
									}
								}
							}
						}
						break;

						// ---------------------------- SET_INVISIBLE command

						case CID_INVISIBLE: {
							
                            pcEvent = ParseNSEntName( pcEvent, aWorkString, iWorkspaceLen );

							NSEnt* const pobNSEnt = FindNSEntFromName( aWorkString );
							user_error_p( pobNSEnt, ("Entity name %s incorrect for NS (%s), (current target clip is %s), for set_invisible event command!!", aWorkString, GetNSName().c_str(), m_pcTargetClipName ));

							// see if NS Ent's really an effect entity
							if (pobNSEnt && pobNSEnt->m_pEffectController)
							{
								pobNSEnt->m_pEffectController->Deactivate();
							}
							else
							{
								// find entity
								CEntity* const pEntity = 
									CEntityManager::Get().FindEntity( aWorkString );
								if (pEntity)
								{
									if( !strcmp( pcEvent, "null" ) )
									{
										// show the whole entity
										ShowEntity( pEntity, false );
									}
									else
									{
										ShowTransform( pEntity, pcEvent, false );
									}
								}
							}
#if defined( TGS_NS_HACKS )
							// TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS
							// TGS WARNING UBER HACKS AHEAD!  PLEASE PROCEED WITH EXTREME CAUTION! TGS
							// TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS TGS
							if(!strcmp(this->m_obNSName.c_str(), "Main_NinjaSequence_Entity_1") &&
							   !strcmp(aWorkString, "bazooka"))
							{
								// Bring in the real player now!
								// Get bazooka
								CEntity* pobBazooka = CEntityManager::Get().FindEntity("Main_Object_Ranged_Weapon_1");
								ntAssert( pobBazooka );

								// Get Hero
								CEntity* pobHero = CEntityManager::Get().GetPlayer();
								ntAssert( pobHero );

								// Activate them
								pobHero->GetPhysicsSystem()->Activate();
								pobBazooka->GetPhysicsSystem()->Activate();

								// Show them
								pobHero->Show();
								pobBazooka->Show();

								// Turn the Hero's input component back on
								pobHero->GetInputComponent()->SetDisabled(false);
								CamMan::Get().GetView(m_iCamView)->RemoveAllCoolCameras(true);
							}
#endif
						}
						break;

						// ---------------------------- KILL_ENTITY command

						case CID_KILL: {
							
							// find entity
							DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( pcEvent );
							user_error_p( pDO, ("NS: KILL_ENTITY event trying to kill non-existant %s", pcEvent ) );

							CEntity* pEntity =(CEntity*)( pDO->GetBasePtr() );
							user_error_p( pEntity, ("Entity referred as %s in kill command not present (NS is %s, current target clip is %s))", pcEvent, GetNSName().c_str(), m_pcTargetClipName));

							// make sure it's not the player as that would be nuts
							user_error_p( pEntity->IsPlayer(), ("NS: Event trying to kill player. No! (NS is %s, current target clip is %s))", GetNSName().c_str(), m_pcTargetClipName) );

							// remove it from all ninja sequences
							NSManager::Get().RemoveEntityFromAllSequences( pEntity );

							// destroy it
							ObjectDatabase::Get().DestroyObject( pDO );
						}
						break;

						// unkown event
						default: ntError( 0 );
					}
					
					// event triggered - move onto next one
					break;
				}
				else ++iEvent;
			}
			while( 1 );
		}
	}	
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::ParseKeyString
//! Parses the button input String
//!
//! \param pStore		Storage instance to put information in
//! \param pcCommand	String to parse
//------------------------------------------------------------------------------------------

void NSPackage::ParseKeyString( NSConditionStorage* pStore, 
								const char* pcCommand )
{
	int iInput = 0;
	while( 1 )
	{
		const char* pcSrcPtr = pcCommand;
		const char* pcPtr = NSManager::Get().GetControlInputStrings(iInput);
		
		user_error_p( *pcPtr != '*', ("NS: Input command '%s' doesn't exist", pcSrcPtr ) );
		
		bool bFound( true );
		while( *pcPtr )
		{
			if( *pcSrcPtr++ != *pcPtr++ )
			{
				bFound = false;
				break;
			}
		}
		
		if( bFound )
		{
			if( !(*pcSrcPtr) || *pcSrcPtr == '@' ) 
			{
				// last command - push back and quit loop
				pStore->m_obCommandInputList.push_back( NSCommand((NSCommand::CONTROLLER_INPUT_TYPE)iInput,
																  GetActiveInstance()->GetNSMode()==NinjaSequence::NS_MODE_TUTORIAL,
																	pStore->m_eCommand == NSConditionStorage::COM_TEST_INPUT) );
				break;
			}
			else if( *pcSrcPtr == '+' )
			{
				// another command after this, push this back and keep looping
				pStore->m_obCommandInputList.push_back( NSCommand((NSCommand::CONTROLLER_INPUT_TYPE)iInput,
																  GetActiveInstance()->GetNSMode()==NinjaSequence::NS_MODE_TUTORIAL,
																	pStore->m_eCommand == NSConditionStorage::COM_TEST_INPUT) );
				++pcSrcPtr;	//skip comma
				pcCommand = pcSrcPtr;
				iInput = -1;
			}
		}

		// not this key - try another
		++iInput;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::LinkOverlays
//! Scans through conditions finding any matching button patterns to link to
//! so they can disappear if the condition is satisfied
//!
//! Should be done once per clip after clip preprocessing done
//! Will throw up a user error if more than one condition matches (not on my patch giirlfriend)
//------------------------------------------------------------------------------------------


void NSPackage::LinkOverlays()
{
	for ( ntstd::Vector<NSConditionStorage*>::const_iterator it = m_PCList.begin(); it != m_PCList.end(); ++it )
	{
		NSConditionStorage* pStore = *it;

		switch( pStore->m_eCommand ) 
		{
			case NSConditionStorage::COM_IN_WINDOW: 
			{
				// scan preprocessed condition list for TEST_INPUT conditions
				for ( ntstd::Vector<NSConditionStorage*>::const_iterator sub_it = m_PCList.begin(); sub_it != m_PCList.end(); ++sub_it )
				{
					NSConditionStorage* pSubStore = *sub_it;
					if( pSubStore->m_eCommand == NSConditionStorage::COM_TEST_INPUT )
					{
						// does it have a matching button pattern?
						if( pStore->m_obCommandInputList.size() == pSubStore->m_obCommandInputList.size() )
						{
							unsigned int iMatch = 0;
							for( unsigned int i = 0; i < pStore->m_obCommandInputList.size(); ++i )
							{
								for( unsigned int j = 0; j < pSubStore->m_obCommandInputList.size(); ++j )
								{
									// scee.sbashow: check is this a bug?
									// 			should it not be pSubStore->m_obCommandInputList.at( j ) == pStore->m_obCommandInputList.at( i )
									//			?? Or even just  pSubStore->m_obCommandInputList.at( i ) == pStore->m_obCommandInputList.at( i ) without the j for-loop?
									if( pStore->m_obCommandInputList.at( j ) == pStore->m_obCommandInputList.at( i ) )
									{
										++iMatch;
										break;
									}
								}
							}
							if( iMatch == pSubStore->m_obCommandInputList.size() )
							{
								// match - link them together
								user_error_p( !pSubStore->m_pLinkedStorage, ("NS: Duplicate button patterns in clip" ) );

								pSubStore->m_pLinkedStorage = pStore;
								pStore->m_pLinkedStorage = pSubStore;
							}
						}
					}
				}
			}
			break;

			default: break;
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::PreprocessConditions
//! Parses the conditions and stores in a list of NSConditionStorage classes
//!
//------------------------------------------------------------------------------------------


void NSPackage::PreprocessConditions()
{
	const int iWorkspaceLen = 64;
	char aWorkString[iWorkspaceLen];	// handy little workspace

	ntError( m_PCList.empty() );	// should be empty here

	for ( ntstd::List< NSCondition* >::const_iterator it = m_pClip->m_ConditionList.begin(); it != m_pClip->m_ConditionList.end(); ++it )
	{
		// create condition class and push onto clip list
		NSConditionStorage* pStore = NT_NEW NSConditionStorage;
		ntError( pStore );
		m_PCList.push_back( pStore );

		NSCondition* pCondition = ( *it );
		const char* pcTest = pCondition->m_sTest.c_str();
		//ntPrintf( "NS: Condition %s, %s, %s\n", pcTest, pCondition->m_sIfTrue.c_str(), pCondition->m_sIfFalse.c_str() );

		// scan for command
		int iCommand = 0;
		while( strcmp( NSManager::Get().GetConditionString( iCommand ), "*" ) )
		{
			const char* pcCommand = strstr( pcTest, NSManager::Get().GetConditionString( iCommand ) );
			if( pcCommand )
			{
				// ---------------------------- TEST_INPUT command
				switch( iCommand )
				{
					case 0: { 
						pStore->m_eCommand = NSConditionStorage::COM_TEST_INPUT;

						// parse input buttons (format: but1+but2+etc...@x hertz,)
						// x = 0 for single press
						pcCommand += strlen( "TEST_INPUT:" );
						ParseKeyString( pStore, pcCommand );

						const char* pcPtr = strstr( pcCommand, "@" );	// skip '@'
						if( pcPtr )							// support legacy files with no '@'
						{
							++pcPtr;
							
                            if( *pcPtr != '0' ) 
							{
								// store target gap
									pStore->m_fTargetPressGap =
												1.0f / (float)( atoi( pcPtr ) );

								// intialise the measure function that'll be used for checking button press consistency.
								pStore->m_fCurrentHitRateMeasureFunction = 
									(m_pClip->m_fWindowEndTime - m_pClip->m_fWindowStartTime) - WINDOW_GRACE_PERIOD;
								ntPrintf( "NS: Setting hit rate measure for button mash initially to window period = %f\n", pStore->m_fCurrentHitRateMeasureFunction);
							}
						}

						// test for SET_GOTOCLIP
						const char* pcParam = strstr( pCondition->m_sIfTrue.c_str(),"SET_GOTOCLIP:" );
						if( pcParam )
						{
							// goto another clip
							pcParam += strlen( "SET_GOTOCLIP:" );

							// store target clip name in storage String list
							char* pcTargetClipName = NT_NEW char[ strlen( pcParam ) ];
							strcpy( pcTargetClipName, pcParam );
							pStore->m_stringdatalist1.push_back( pcTargetClipName );
						}
						
						pcParam = strstr( pCondition->m_sIfFalse.c_str(),"SET_GOTOCLIP:" );
						user_error_p( !pcParam, ("If False SET_GOTOCLIP is not currently supported") );
						//if( pcParam )
						//{
						//	// goto another clip
						//	pcParam += strlen( "SET_GOTOCLIP:" );

						//	// store target clip name in storage String list
						//	char* pcTargetClipName = NT_NEW char[ strlen( pcParam ) ];
						//	strcpy( pcTargetClipName, pcParam );
						//	pStore->m_stringdatalist2.push_back( pcTargetClipName );
						//}
						 
						// If the command list for any clip has a test input, then note that the package has an interactive slant to it.
						m_bHasInteractiveClip = true;
					}
					break;

					// --------------------------- IN_WINDOW command

					case 1: {
						pStore->m_eCommand = NSConditionStorage::COM_IN_WINDOW;

						// test for subcommand 'SHOW_SPRITE'
						const char* pcParam = strstr( pCondition->m_sIfTrue.c_str(),"SHOW_SPRITE:" );
						if( pcParam )
						{
							pStore->m_eSubCommand = NSConditionStorage::SUB_SHOW_SPRITE;

							pcParam += strlen( "SHOW_SPRITE:" );

							// display an input overlay
							// params: overlay id, entity name, transform name
							
							// get overlay name into null-terminated work buffer
							int iLen = 0;
							while( *pcParam != ',' )
							{
								aWorkString[ iLen++ ] = *pcParam++;
								ntError( iLen < iWorkspaceLen );
							}
							aWorkString[ iLen ] = 0;

							// match up with overlay enum
							ParseKeyString( pStore, aWorkString );

							// get target entity name
							iLen = 0;
							++pcParam;	// skip comma
							while( *pcParam != ',' )
							{
								aWorkString[ iLen++ ] = *pcParam++;
								ntError( iLen < iWorkspaceLen );
							}
							aWorkString[ iLen ] = 0;

							++iLen; // make room for null-terminator
							char* pcName = NT_NEW char[ iLen ];
							ntError( pcName );
							NT_MEMCPY( pcName, aWorkString, iLen * sizeof( char ) );

							// get transform node name
							++pcParam;	// skip comma
							char* pcTransformName = NT_NEW char[ strlen( pcParam ) ];
							strcpy( pcTransformName, pcParam );

							// push onto String list
							pStore->m_stringdatalist1.push_back( pcName );
							pStore->m_stringdatalist1.push_back( pcTransformName );
						}
					}
					break;

					default:;
				}
				
				break;	// out of command check while loop
			}
			else ++iCommand;
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::GetHitRateMeasurePassDuration
//! Used with m_fCurrentHitRateMeasureFunction in the store condition for mashing/shaking
//------------------------------------------------------------------------------------------
float NSPackage::GetHitRateMeasurePassDuration( void ) const
{
	// duration of window
	const float fWindowDuration = 
		(m_pClip->m_fWindowEndTime - m_pClip->m_fWindowStartTime) - WINDOW_GRACE_PERIOD;

	return (fWindowDuration*2.0f);
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::PostProcessConditions
//! Loops through the conditions post-window doing any necessary final processing
//!
//! only needs to be called once per clip (frame after the window closes)
//------------------------------------------------------------------------------------------

void NSPackage::PostProcessConditions()
{
	// only need to do this once after the window closes
	if( !m_bPostProcessedConditions )
	{
		m_bPostProcessedConditions = true;

		for ( ntstd::Vector<NSConditionStorage*>::const_iterator it = m_PCList.begin(); it != m_PCList.end(); ++it )
		{
			NSConditionStorage* pStore = *it;
			
			switch( pStore->m_eCommand ) 
			{
				case NSConditionStorage::COM_TEST_INPUT: 
				{
					if( pStore->m_fTargetPressGap > 0.0f )
					{
						// this is a frequency condition - was it satisfied?
						if( m_bMeasureAverageNumberPressesMet 
							&&
							// scee.sbashow: check for pressing consistency throughout - more accurate than a simple average
							(m_fProximityToFrequencyPassing > 0.9f ))
						{
							// okay passed
							pStore->m_bCorrect = true;

							// show tick
							// scee.sbashow - note, these three lines are surely not necessary here 
							//							as ProcessCorrectConditions() will set them? 
							//							as it does for the single presses being correct. 
							m_bTick = true;
							m_fTickTimer = TICK_SHOW_PERIOD;
							PlayFeedbackSound( true );
						}
						else
						{
							// failed
							pStore->m_bCorrect = false;

							// show cross for penalty period (will be overridden by future ticks)
							m_bTick = false;
							m_fTickTimer = TICK_SHOW_PERIOD;
							PlayFeedbackSound( false );
						}
					}
					else if( !pStore->m_bCorrect )
					{
						// show cross if nothing pressed in single press window
						m_bTick = false;
						m_fTickTimer = TICK_SHOW_PERIOD;
						//PlayFeedbackSound( false );
					}

				}
				break;

				default: break;
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::ProcessCorrectConditions
//! Loops through to execute correct conditions
//!
//------------------------------------------------------------------------------------------

void NSPackage::ProcessCorrectConditions()
{
	for ( ntstd::Vector<NSConditionStorage*>::const_iterator it = m_PCList.begin(); it != m_PCList.end(); ++it )
	{
		NSConditionStorage* pStore = *it;

		if( !pStore->m_bConditionTrueExecuted && pStore->m_bCorrect )
		{
			pStore->m_bConditionTrueExecuted = true;

			switch( pStore->m_eCommand ) 
			{
				case NSConditionStorage::COM_TEST_INPUT: 
				{
					if( !(pStore->m_stringdatalist1.empty() ) )
					{
						// currently only SET_GOTOCLIP supported (target clip name in stringdatalist1)

						// make sure player isn't killed
						m_bPlayerKilled = false;

						// set the target clip from the condition object
						const char* pcTargetClipName = pStore->m_stringdatalist1.front();
						m_pTargetClip = FindClipObject( pcTargetClipName );
						m_pcTargetClipName = pcTargetClipName;

						// Tick!
						m_bTick = true;
						m_fTickTimer = TICK_SHOW_PERIOD;
						PlayFeedbackSound( true );
					}
				}
				break;

				default:
				break;
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::TestConditions
//! Parses the conditions (only during window)
//!
//------------------------------------------------------------------------------------------

void NSPackage::TestConditions( float fDt )
{
#define MIN_STICK_PUSH 0.3f
#define THIRD_PI (PI/3.0f)
#define FIFTH_PI (PI/5.0f)

#define USING_CAM_LOCAL_SPACE_NOT
#define NS_DEBUG_INPUT_TEST

	struct PLAYER_INFO
	{
		PLAYER_INFO()
		{
			pobInputComp=0;
			bDoingMotionController=false;
			obRelativeYAxis=obRelativeXAxis=CDirection(CONSTRUCT_CLEAR);
		}
		const CInputComponent* pobInputComp;
		CDirection		obRelativeYAxis,obRelativeXAxis;
		bool bDoingMotionController;
		float fMotionControllerSignificantSqr;
		float fMotionGrav;
		float fSignificantMotion;
		float fLengthMotionGracePeriod;
		struct MotionInfo
		{
			MotionInfo()
			{
				bMovingPlusX = bMovingNegX = bMovingPlusYZ = bMovingNegYZ = false;
				bMotionisSignificant= false;
			}

			CVector obMotionAccelVector;
			CVector obYZFilter;
			CVector obXFilter;
			CVector obDirSignXFilter;
			CVector obDirSignYZFilter;

			bool 	bMovingPlusX:1;
			bool 	bMovingNegX:1;
			bool	bMovingPlusYZ:1;
			bool	bMovingNegYZ:1;

			//scee.sbashow - could just union this with those above...
			bool bMotionisSignificant;

			void DetermineFilters()
			{
				obYZFilter = 			CVector(0.0f,obMotionAccelVector.Y(),obMotionAccelVector.Z(),0.0f);
				obXFilter = 			CVector(obMotionAccelVector.X(),0.0f,0.0f,0.0f);
				obDirSignYZFilter = 	CVector(0.0f,1.0f,1.0f,0.0f);
				obDirSignXFilter = 	CVector(1.0f,0.0f,0.0f,0.0f);
			}

			void DetermineDirs(float fSignificantMotionValSQR)
			{
				if (obMotionAccelVector.Dot(obMotionAccelVector)>fSignificantMotionValSQR)
				{
					if (obMotionAccelVector.Dot(obXFilter)>fSignificantMotionValSQR)
					{
						const float fXVal = obMotionAccelVector.Dot(obDirSignXFilter);
						bMovingPlusX = fXVal<0.0f;
						bMovingNegX = !bMovingPlusX;
						bMotionisSignificant = bMovingPlusX || bMovingNegX;
					}

					if (obMotionAccelVector.Dot(obYZFilter)>fSignificantMotionValSQR)
					{
						const float fYZVal = obMotionAccelVector.Dot(obDirSignYZFilter);
						bMovingPlusYZ = fYZVal>0.0f;
						bMovingNegYZ = !bMovingPlusYZ;
						bMotionisSignificant = bMotionisSignificant || bMovingPlusYZ || bMovingNegYZ;
					}
				}
			}

		} obMotionFiltered, obMotionRaw;

	} obPlayerInfo;


	const bool bInTutorialMode(GetActiveInstance()->GetNSMode() == NinjaSequence::NS_MODE_TUTORIAL);
   
	Player* const pobPlayer = CEntityManager::Get().GetPlayer();

	if (pobPlayer)
	{
		obPlayerInfo.pobInputComp = pobPlayer->GetInputComponent();

		#ifdef NS_DEBUG_INPUT_TEST
			obPlayerInfo.bDoingMotionController = g_ShellOptions->m_bUsingMotionControllerOnNSs;

			if (obPlayerInfo.bDoingMotionController)
			{
				obPlayerInfo.fMotionGrav = -0.2f; //scee.sbashow - roughly.
				obPlayerInfo.fMotionControllerSignificantSqr = 0.4f*0.4f;

				obPlayerInfo.obMotionFiltered.obMotionAccelVector = CVector( obPlayerInfo.pobInputComp->GetSensorFilteredMag( PAD_SENSOR_ACCEL_X,PAD_SENSOR_FILTER_AVERAGE_5  ),
																			 obPlayerInfo.pobInputComp->GetSensorFilteredMag( PAD_SENSOR_ACCEL_Y,PAD_SENSOR_FILTER_AVERAGE_5  ),
																			 obPlayerInfo.pobInputComp->GetSensorFilteredMag( PAD_SENSOR_ACCEL_Z,PAD_SENSOR_FILTER_AVERAGE_5  ),
																			 obPlayerInfo.pobInputComp->GetSensorFilteredMag( PAD_SENSOR_GYRO_Y, PAD_SENSOR_FILTER_AVERAGE_5  ));

				obPlayerInfo.obMotionFiltered.DetermineFilters();

				obPlayerInfo.obMotionFiltered.DetermineDirs(obPlayerInfo.fMotionControllerSignificantSqr);


				obPlayerInfo.obMotionRaw.obMotionAccelVector = CVector( obPlayerInfo.pobInputComp->GetSensorRawMag( PAD_SENSOR_ACCEL_X ),
																		obPlayerInfo.pobInputComp->GetSensorRawMag( PAD_SENSOR_ACCEL_Y ),
																		obPlayerInfo.pobInputComp->GetSensorRawMag( PAD_SENSOR_ACCEL_Z ),
																		obPlayerInfo.pobInputComp->GetSensorRawMag( PAD_SENSOR_GYRO_Y ));
				obPlayerInfo.obMotionRaw.DetermineFilters();

				obPlayerInfo.fLengthMotionGracePeriod = (m_pClip->m_fDuration)*0.2f;

				//ntPrintf("Window motion grace period=%f\n",obPlayerInfo.fLengthMotionGracePeriod);
			}

		#endif
		
		#ifdef USING_CAM_LOCAL_SPACE

			obPlayerInfo.obRelativeXAxis = CDirection( 1.0f, 0.0f, 0.0f );
			obPlayerInfo.obRelativeYAxis = CDirection( 0.0f, 0.0f, 1.0f );

		#else

			if (!CamMan::Get().GetView(m_iCamView))
			{
				return;
			}
			
			obPlayerInfo.obRelativeYAxis = CamMan::Get().GetView(m_iCamView)->GetCurrMatrix().GetZAxis();
			obPlayerInfo.obRelativeXAxis = CamMan::Get().GetView(m_iCamView)->GetCurrMatrix().GetXAxis();
		#endif
	}

	if (!obPlayerInfo.pobInputComp)
	{
		return;
	}

	if (obPlayerInfo.bDoingMotionController)
	{
		m_fTimeSinceLastSignificantMotion += fDt;

		if (!m_bAllowingForMotionGrace)
		{
			if (obPlayerInfo.obMotionFiltered.bMotionisSignificant)
			{
				m_fTimeSinceLastSignificantMotion = 0.0f;
			}
		}
	}

	#ifdef USING_CAM_LOCAL_SPACE
	
	const CMatrix obCamAffineInverse = 
		CamMan::Get().GetView(m_iCamView)->GetCurrMatrix().GetAffineInverse();
	const CDirection obStdWorldInputDirection = obPlayerInfo.pobInputComp->GetInputDir();
	const CDirection obStdPadDirection = obStdWorldInputDirection * obCamAffineInverse;

	#else
	
	const CDirection obStdPadDirection = obPlayerInfo.pobInputComp->GetInputDir();

	#endif

	for ( ntstd::Vector<NSConditionStorage*>::const_iterator it = m_PCList.begin(); it != m_PCList.end(); ++it )
	{
		NSConditionStorage* pStore = *it;
		
		switch( pStore->m_eCommand ) 
		{
			case NSConditionStorage::COM_TEST_INPUT: 
			{
				// allow grace period for button mashing windows
				bool bInGracePeriod( false );
				bool bInMotionGracePeriod( false );
				if( pStore->m_fTargetPressGap > 0.0f )
				{
					if( (m_fClipTime - m_pClip->m_fWindowStartTime) < WINDOW_GRACE_PERIOD )
					{
						bInGracePeriod = true;
					}

					m_bAllowingForMotionGrace = false;
				}
				else
				{
					// scee.sbashow: allow a grace for next motion 
					// 				if time since last significant motion of the last clip is not within the grace period
					if (obPlayerInfo.bDoingMotionController)
					{
						if (m_bAllowingForMotionGrace)
						{
							if (m_fTimeSinceLastSignificantMotion < obPlayerInfo.fLengthMotionGracePeriod )
							{
								bInMotionGracePeriod = true;
							}
							else
							{
								m_bAllowingForMotionGrace = false;
							}
						}
					}
				}

				if( !pStore->m_bInputBlocked && !bInGracePeriod )
				{
					bool bExpectingDiag( false), bExpectingButton( false );

					// scee.sbashow: use virtualised results.
                    bool bHasSufficientDir	   = obPlayerInfo.bDoingMotionController? 	
						obPlayerInfo.obMotionFiltered.bMotionisSignificant
													: 
						obPlayerInfo.pobInputComp->IsDirectionHeld();
					
					// scee.sbashow - check - remove this if I check it in, as should have fixed the issue
					// 					that required this printf in the first place!
					//ntPrintf( "NS: Current (virtual) player input during input testing regime is %d\n", obPlayerInfo.pobInputComp->GetVPressed() );

                    int iCorrect = 0;
//					ntPrintf( "Angle: %.2f %.2f\n", pPad->GetAnalogLAngle(),pPad->GetAnalogLMag() );
					for( ntstd::Vector<NSCommand>::iterator it = pStore->m_obCommandInputList.begin(); it != pStore->m_obCommandInputList.end(); ++it )
					{
						switch( *it )
						{
							// scee.sbashow: was "gKey0 - pad triangle"
							case NSCommand::NS_COUNTER:	bExpectingButton = true; if( obPlayerInfo.pobInputComp->GetVPressed() & ( 1 << AB_ATTACK_FAST )) 			++iCorrect; break;

							// scee.sbashow: was "gKey1 - pad circle"
							case NSCommand::NS_GRAB:		bExpectingButton = true; if( obPlayerInfo.pobInputComp->GetVPressed() & ( 1 << AB_GRAB )) 					++iCorrect; break;

							// scee.sbashow: was "gKey2 - pad cross"
							case NSCommand::NS_ACTION:		
							{
								if (obPlayerInfo.bDoingMotionController)
								{
									bExpectingDiag = true; 

									ntPrintf( "NS:testing motion shake for Cross!...\n");

									if (obPlayerInfo.obMotionRaw.obMotionAccelVector.Dot(obPlayerInfo.obMotionRaw.obYZFilter)>(obPlayerInfo.fMotionControllerSignificantSqr))
									{
										ntPrintf( "NS:motion shake magnitude passed!...\n");
										
										const bool bInPosDirection = 
											obPlayerInfo.obMotionRaw.obMotionAccelVector.Dot(obPlayerInfo.obMotionRaw.obDirSignYZFilter)>0.0f;
										if (!(pStore->m_fTargetPressGap>0.0f) || 
											(pStore->m_bLastMotionDirPositive == bInPosDirection))
										{
											ntPrintf( "NS: shake dir CORRECT!...\n");
											++iCorrect; 
										}
										else
										{
											ntPrintf( "NS: but shake dir (%s) incorrect!...\n", pStore->m_bLastMotionDirPositive? "positive":"negative");
										}
									}
								}
								else
								{
									bExpectingButton = true; 

									if( obPlayerInfo.pobInputComp->GetVPressed() & ( 1 << AB_ACTION )) 					
									{
										++iCorrect; 
									}
								}
									

							}
							break;

							// scee.sbashow: was "gKey3 - pad square"
							case NSCommand::NS_ATTACK:		bExpectingButton = true; if( obPlayerInfo.pobInputComp->GetVPressed() & ( 1 << AB_ATTACK_MEDIUM )) 			++iCorrect; break;

							// directions
							case NSCommand::NS_MOVEUP:	
							{		
								
								// scee.sbashow - added
								ntPrintf( "NS: Current NS expects upwards movement\n");
								
								bExpectingDiag = true; 


								if (obPlayerInfo.bDoingMotionController)
								{
									if (obPlayerInfo.obMotionFiltered.bMovingPlusYZ)
									{
										++iCorrect; 
									}
									else
									{
										if (!obPlayerInfo.obMotionFiltered.bMovingNegYZ)
										{
											// scee.sbashow --> if sufficient dir because of horiz motion, then ignore.
											bHasSufficientDir = false;
										}

									}
								}
								else
								{

									if( bHasSufficientDir &&  
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeYAxis)>0.0f)
									{
										++iCorrect; 
									} 
									else
									{
										ntPrintf( "Failed!...\n");
									}
								}

							} break;
							case NSCommand::NS_MOVEDOWN: 
							{		
								// scee.sbashow - added
								ntPrintf( "NS: Current NS expects downwards movement...\n");
								
								bExpectingDiag = true;

								if (obPlayerInfo.bDoingMotionController)
								{
									if (obPlayerInfo.obMotionFiltered.bMovingNegYZ)
									{
										++iCorrect; 
									}
									else
									{
										if (!obPlayerInfo.obMotionFiltered.bMovingPlusYZ)
										{
											// scee.sbashow --> if sufficient dir because of horiz motion, then ignore.
											bHasSufficientDir = false;
										}
									}
								}
								else
								{
									if( bHasSufficientDir &&  
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeYAxis)<0.0f)
									{
										++iCorrect; 
									} 
									else
									{
										ntPrintf( "Failed!...\n");
									}
								}
							} break;
							case NSCommand::NS_MOVELEFT: 
							{		
								bExpectingDiag = true;

								if (obPlayerInfo.bDoingMotionController)
								{
									if (obPlayerInfo.obMotionFiltered.bMovingNegX)
									{
										++iCorrect; 
									}
									else
									{
										if (!obPlayerInfo.obMotionFiltered.bMovingPlusX)
										{
											// scee.sbashow --> if sufficient dir because of vertical motion, then ignore.
											bHasSufficientDir = false;
										}
									}
								}
								else
								{
									if( bHasSufficientDir &&  
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeXAxis)>0.0f)
									{
										++iCorrect; 
									} 
								}
							} break;
							case NSCommand::NS_MOVERIGHT: 
							{	
								bExpectingDiag = true;

								if (obPlayerInfo.bDoingMotionController)
								{
									if (obPlayerInfo.obMotionFiltered.bMovingPlusX)
									{
										++iCorrect; 
									}
									else
									{
										if (!obPlayerInfo.obMotionFiltered.bMovingNegX)
										{
											// scee.sbashow --> if sufficient dir because of vertical motion, then ignore.
											bHasSufficientDir = false;
										}
									}
								}
								else
								{
									if( bHasSufficientDir &&  
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeXAxis)<0.0f)
									{
										++iCorrect; 
									} 
								}

							} break;
							case NSCommand::NS_UPLEFT:	
							{		
								// scee.sbashow - added
								ntPrintf( "NS: Current NS expects upleft movement...\n");
								
								bExpectingDiag = true;

								if (obPlayerInfo.bDoingMotionController)
								{
									if (obPlayerInfo.obMotionFiltered.bMovingNegX && 
										obPlayerInfo.obMotionFiltered.bMovingPlusYZ)
									{
										++iCorrect; 
									}
								}
								else
								{
									if( bHasSufficientDir &&  
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeYAxis)>0.0f && 
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeXAxis)>0.0f)
									{
										++iCorrect; 
									} 
									else
									{
										ntPrintf( "Failed!...\n");
									}
								}

							} break;
							case NSCommand::NS_UPRIGHT: 
							{		
								// scee.sbashow - added
								ntPrintf( "NS: Current NS expects upright movement\n");
								
								bExpectingDiag = true;
								if (obPlayerInfo.bDoingMotionController)
								{
									if (obPlayerInfo.obMotionFiltered.bMovingPlusX && 
										obPlayerInfo.obMotionFiltered.bMovingPlusYZ)
									{
										++iCorrect; 
									}
								}
								else
								{
									if( bHasSufficientDir &&  
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeYAxis)>0.0f && 
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeXAxis)<0.0f)
									{
										++iCorrect; 
									} 
									else
									{
										ntPrintf( "Failed!...\n");
									}
								}

							} break;
							case NSCommand::NS_DOWNLEFT: 
							{		
								// scee.sbashow - added
								ntPrintf( "NS: Current NS expects downleft movement\n");
								bExpectingDiag = true;

								if (obPlayerInfo.bDoingMotionController)
								{
									if (obPlayerInfo.obMotionFiltered.bMovingNegX && 
										obPlayerInfo.obMotionFiltered.bMovingNegYZ)
									{
										++iCorrect; 
									}
								}
								else
								{
	
									if( bHasSufficientDir &&  
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeYAxis)<0.0f && 
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeXAxis)>0.0f)
									{
										++iCorrect; 
									} 
									else
									{
										ntPrintf( "Failed!...\n");
									}
								}
							} break;
							case NSCommand::NS_DOWNRIGHT: 
							{	
								// scee.sbashow - added
								ntPrintf( "NS: Current NS expects downright movement\n");
								
								bExpectingDiag = true;

								if (obPlayerInfo.bDoingMotionController)
								{
									if (obPlayerInfo.obMotionFiltered.bMovingPlusX && 
										obPlayerInfo.obMotionFiltered.bMovingNegYZ)
									{
										++iCorrect; 
									}
								}
								else
								{
									if( bHasSufficientDir &&  
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeYAxis)<0.0f && 
										 obStdPadDirection.Dot(obPlayerInfo.obRelativeXAxis)<0.0f)
									{
										++iCorrect; 
									} 
									else
									{
										ntPrintf( "Failed!...\n");
									}
								}
							} break;
						}
					}

					bool bCorrectPresses( iCorrect == (int)( pStore->m_obCommandInputList.size() ) );

// debug option to always get correct presses...
//#ifdef _NSDEBUG_ALWAYSTRUE							
					if( NSManager::Get().IsSkippingSequence() )
					{
						bCorrectPresses = true;
						if(	pStore->m_fTargetPressGap > 0.0f )
						{
							 pStore->m_iNumTruePresses++;
							 pStore->m_fCurrentHitRateMeasureFunction+=pStore->m_fTargetPressGap;
						}
					}
//#endif
// ...debug option to always get correct presses

					if( pStore->m_fTargetPressGap > 0.0f )
					{
						// frequency input required in window
						if( bCorrectPresses )
						{
							// increase correct button counter
							pStore->m_iNumTruePresses++;

							//const float fExpf1_0f = 2.71828f;

							// scee.sbashow : add target press gap period for a good hit
							pStore->m_fCurrentHitRateMeasureFunction+=pStore->m_fTargetPressGap*2.0f;
							pStore->m_fCurrentHitRateMeasureCount = pStore->m_fTargetPressGap;

							// scee.sbashow : make sure to clamp it though, to avoid button mash spikes blowing things ups.
							//pStore->m_fCurrentHitRateMeasureFunction = 
							//	min(pStore->m_fTargetPressGap*fExpf1_0f,pStore->m_fCurrentHitRateMeasureFunction);

							// scee.sbashow : switch motion controller dir.
							pStore->m_bLastMotionDirPositive = !pStore->m_bLastMotionDirPositive;

							if (!m_bMeasureAverageNumberPressesMet)
							{
								// duration of window
								const float fWindowDuration = 
									(m_pClip->m_fWindowEndTime - m_pClip->m_fWindowStartTime) - WINDOW_GRACE_PERIOD;

								// is it less than the max. allowed to pass?
								m_bMeasureAverageNumberPressesMet = 
									(fWindowDuration < (pStore->m_fTargetPressGap * (float)pStore->m_iNumTruePresses) );
							}
							else
							{
								m_fProximityToFrequencyPassing = 
									min(1.0f,pStore->m_fCurrentHitRateMeasureFunction/GetHitRateMeasurePassDuration());
							}

						}
						else
						{
							// scee.sbashow :
							// if a correct press has happened within pStore->m_fTargetPressGap; 
							// of the current time, then do not start to decay the measure function exponentialy
							// let it drop linearly, instead.
							if (pStore->m_fCurrentHitRateMeasureCount>0.0f)
							{
								pStore->m_fCurrentHitRateMeasureFunction-=fDt;
                                pStore->m_fCurrentHitRateMeasureCount-=fDt;
							}
							else
							{
								// scee.sbashow : exponentially reduce - drops to e(-1) of current value 
									// over DT == pStore->m_fTargetPressGap, if not pressing frequently enough.
								pStore->m_fCurrentHitRateMeasureFunction-=
									pStore->m_fCurrentHitRateMeasureFunction*(fDt/pStore->m_fTargetPressGap);
							}
						}
					}
					else
					{
						// single press required
						if( bCorrectPresses )
						{
							// correct - show tick and block any more inputs
							pStore->m_bInputBlocked = true;
							//InputCorrect();
							pStore->m_bCorrect = true;
						}
						else
						{
							
							bool bAnyPressed( false );//

							// if expecting a button only, don't penalise for pushing a diagonal
							if( bExpectingButton && !bExpectingDiag &&
								(obPlayerInfo.pobInputComp->GetVHeld() & 
								 (( 1 << AB_ATTACK_FAST ) | 
								  ( 1 << AB_ATTACK_MEDIUM ) | 
								  ( 1 << AB_ACTION ) | 
								  ( 1 << AB_GRAB ) ))) bAnyPressed = true;

							// likewise if expecting a diagonal only, don't penalise for pushing a button
							if( bExpectingDiag && !bExpectingButton && bHasSufficientDir )
							{
								bAnyPressed = true;
							}

							if( bAnyPressed )
							{
								if (bInMotionGracePeriod)
									// scee.sbashow -- ignore motion incorrectness if in motion grace period.
								{
									const float fGraceTimeLeft = 
										obPlayerInfo.fLengthMotionGracePeriod - m_fTimeSinceLastSignificantMotion;
									ntPrintf("Wrong move, but ok as in motion grace period (time left = %f)!!\n",fGraceTimeLeft );
									ntAssert(!bInTutorialMode);
									pStore->m_fWindowPenaltyTime = 
										fGraceTimeLeft*0.8f;

									pStore->m_bInputBlocked = true;
								}
								else
								{
									const float fwindowTimeRemaning = 
										m_pClip->m_fWindowEndTime - m_fClipTime;
									const float fErrorWindowFraction = 0.33333f;
									
									// scee.sbashow: ignore incorrect 
										//				if in tutorial mode also...
									if (bInTutorialMode)
									{
										pStore->m_fWindowPenaltyTime = 
											fwindowTimeRemaning * fErrorWindowFraction * GetActiveInstance()->GetTutorialBaseScalar();

										pStore->m_bInputBlocked = true;
									}
									else
									{
										pStore->m_fWindowPenaltyTime = fwindowTimeRemaning * fErrorWindowFraction;
										// enough time in the window left for another chance?
										if( pStore->m_fWindowPenaltyTime > MIN_WINDOW_PENALTY_PERIOD )
										{
											// yes - block inputs for a while
											pStore->m_bInputBlocked = true;

											// show cross for penalty period (will be overridden by future ticks)
											m_bTick = false;
											m_fTickTimer = pStore->m_fWindowPenaltyTime;
										}
										else
										{
											// nope, no penalty time and block inputs
											pStore->m_bInputBlocked = true;
											pStore->m_fWindowPenaltyTime = 0.0f;

											// show cross
											m_bTick = false;
											m_fTickTimer = TICK_SHOW_PERIOD;
											PlayFeedbackSound( false );
										}
									}

								}
							}
						}
					}
				}
				else
				{
					// is this TEST_INPUT condition in a penalty period?
					if( pStore->m_fWindowPenaltyTime > 0.0f )
					{
						// yes - reduce time remaining
						pStore->m_fWindowPenaltyTime -= fDt;
						if( pStore->m_fWindowPenaltyTime < 0.0f )
						{
							// penalty period over - allow inputs again
							pStore->m_fWindowPenaltyTime = 0.0f;
							pStore->m_bInputBlocked = false;
						}
					}
				}
			}
			break;

			// --------------------------- IN_WINDOW command (contains subcommands)
			
			case NSConditionStorage::COM_IN_WINDOW: {
				
				switch( pStore->m_eSubCommand ) 
				{
					case NSConditionStorage::SUB_SHOW_SPRITE:
					{
						// first check if there is a connected TEST_INPUT condition
						bool bHide( false );
						if( pStore->m_pLinkedStorage )
						{
							// has it been mmmm... satisfied
							if( pStore->m_pLinkedStorage->m_bCorrect )//|| pStore->m_pLinkedStorage->m_bInputBlocked )
							{
								// oh yeah
								bHide = true;
							}
							else
							{
								// is it frequency flashing?
								/*if( pStore->m_pLinkedStorage->m_fTargetPressGap > 0.0f )
								{
									// flash the overlay at the required minimum frequency
									pStore->m_fLastOVerlayDisplay += fDt;
									const float fFlashGap = min( pStore->m_pLinkedStorage->m_fTargetPressGap, (1.0f / MIN_FLASH_HERTZ) );
									if( pStore->m_fLastOVerlayDisplay > fFlashGap )
									{
										pStore->m_fLastOVerlayDisplay = 0.0f;
									}
									else if( pStore->m_fLastOVerlayDisplay > (fFlashGap * 0.5f) )
									{
										// only display for half the gap period
										bHide = true;
									}
								}*/
							}
						}

						if( !bHide )
						{
							// scee.sbashow: check, is this entity overlay actually used anymore?
							const char* pcEntityName = pStore->m_stringdatalist1.front();
							CEntity* pEntity( NULL );
							if( strcmp( pcEntityName, "null" ) )
							{
								pEntity = CEntityManager::Get().FindEntity( pStore->m_stringdatalist1.front() );
								user_error_p( pEntity, ("NS Entity %s unavailable to display overlay", pcEntityName ) );
							}

							m_bTestInputOverLayBeingRequested = true;
							pStore->m_bAllowOverlaySnap = false;
						}
					}
					break;

					default:
					break;
				}
			}
			break;

			// unkown command
			default: ntError( 0 );
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::NSPackage
//! Construction
//!
//------------------------------------------------------------------------------------------
NSPackage::NSPackage( NSEntityInstance* pobEntity ) 
:   m_pobActiveInstance(0), 


	// scee.sbashow : these two should be mapped to the NS def
	//					as the name for obvious reasons
	//					and the mapped area info because
	//					the ns game ent's are (currently, at least)
	//					mapped at construction in BuildPackage()
	m_uiMappedAreaInfo(pobEntity->GetMappedAreaInfo()),
	m_obNSName(pobEntity->GetName()),

	m_bResident( false ),				
	m_eStage( NS_TRIGGER ),
	m_bClipStarted( false ),			
	m_fClipTime( 0.0f ),				
	m_fTimeSinceLastSignificantMotion( 0.0f),
	m_bAllowingForMotionGrace(false),
	m_uiPadSpriteID( 0 ),				
	m_pNSContainer(pobEntity->GetNS()),		
	m_pClip( 0 ),						
	m_pTargetClip( 0 ),					
	m_bFirstClip( true ),				
	m_bInputTriggered( false ),			
	m_fTickTimer( 0.0f ),				
	m_bTick( false ),					
	m_bPlayerKilled( false ),		
	m_pcTargetClipName( 0 ),			
	m_bPlayerDisabled( false ),			
	m_bPostProcessedConditions( false ),						
	m_fTotalTime( 0.0f ),				
	m_bFailed( false ),					
	m_pcCameraAnimName( 0 ),			
	m_pcClipName( 0 ),					
	m_fTickPeriod( 0.0f ),
	m_iLastModifiedCoolCamID(-1),
	m_iTransingOutLastCoolCamID(-1),
	m_iCamView(0),
	m_ggWhite(0),
	m_ggBlue(0),
	m_ggRed(0),
	m_bHasInteractiveClip(false)
{
	BuildPackage();
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::~NSPackage
//! Destruction
//!
//------------------------------------------------------------------------------------------
NSPackage::~NSPackage( void )
{
	CleanPackage();
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::BuildPackage
//! 
//!
//------------------------------------------------------------------------------------------
void NSPackage::BuildPackage( void )
{
	BuildEnts();
	// and..?
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::BuildEnts
//! Constructs the much needed game ents.
//!
//------------------------------------------------------------------------------------------
void NSPackage::BuildEnts()
{
	// Build an internal list of our actual entities - loop through all the clips
	ntstd::List<NSClip*>::iterator obClipEndIt = m_pNSContainer->m_ClipList.end();
	for ( ntstd::List<NSClip*>::iterator obClipIt = m_pNSContainer->m_ClipList.begin(); obClipIt != obClipEndIt; ++obClipIt )
	{
		// Loop through all the animations
		ntstd::List<NSAnim*>::iterator obAnimEndIt = ( *obClipIt )->m_AnimList.end();
		for ( ntstd::List<NSAnim*>::iterator obAnimIt = ( *obClipIt )->m_AnimList.begin(); obAnimIt != obAnimEndIt; ++obAnimIt )
		{
			// If the NSEnt for this animation is not in our list - add it
			bool bNSEntInList = false;
			ntstd::Vector<NSEnt*>::iterator obEntEndIt = m_EntityList.end();
			for ( ntstd::Vector<NSEnt*>::iterator obEntIt = m_EntityList.begin(); obEntIt != obEntEndIt; ++obEntIt )
			{
				if ( ( *obEntIt ) == ( *obAnimIt )->m_pTargetEntity )
				{
					bNSEntInList = true;
					break;
				}
			}
			
			if ( !bNSEntInList )
			{
				NSEnt* pEnt = ( *obAnimIt )->m_pTargetEntity;

				// Set the home for the entity
				pEnt->SetHome( this );

				m_EntityList.push_back( pEnt );
			}
		}
	}

	// Loop through all our NSEnts and build all the data we would possibly need
	ntstd::Vector<NSEnt*>::iterator obEntEndIt = m_EntityList.end();
	for ( ntstd::Vector<NSEnt*>::iterator obEntIt = m_EntityList.begin(); obEntIt != obEntEndIt; ++obEntIt )
	{
		CreateNSEntity( *obEntIt );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::CleanPackage
//! Clears out all the data in a package
//!
//------------------------------------------------------------------------------------------
void NSPackage::CleanPackage( void )
{
	// Take all the NS animations from the entities
	RemoveNSAnimContainers();

	RemoveAllNSBlendShapesRelatedStuff();

	// Loop through the entity list and destroy the entities created for the sequence
	for ( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); ++nse_it )
	{
		NSEnt* pEnt = *nse_it;

		// This entity is no longer part of this nspackage
		pEnt->SetHome( 0 );

		if ( pEnt->m_pEffectController )
		{
			NT_DELETE( pEnt->m_pEffectController );
			pEnt->m_pEffectController = 0;
		}

		CEntity* pEntity = GetGameEntityPtr( pEnt );
		if ( pEntity && pEnt->m_bNSEntity )
		{
			// destroy non-persistant entities
			DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pEntity );
			ntError( pDO );
			ObjectDatabase::Get().DestroyObject( pDO );
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::ResetPackage
//! 
//!
//------------------------------------------------------------------------------------------
void NSPackage::ResetPackage( )
{			
	m_bResident = false;				
	m_eStage = NS_TRIGGER;
	m_bClipStarted = false;			
	m_fClipTime = 0.0f;				
	m_uiPadSpriteID = 0;				
	m_pClip = 0;							// Points to serialised data				
	m_pTargetClip = 0;						// Points to serialised data			
	m_bFirstClip = true;				
	m_bInputTriggered = false;			
	m_fTickTimer = 0.0f;				
	m_bTick = false;					
	m_bPlayerKilled = false;		
	m_pcTargetClipName = 0;					// Points to serialised data			
	m_bPlayerDisabled = false;			
	m_bPostProcessedConditions = false;						
	m_fTotalTime = 0.0f;				
	m_bFailed = false;					
	m_pcCameraAnimName = 0;					// Points to serialised data		
	m_pcClipName = 0;						// Points to serialised data			
	m_fTickPeriod = 0.0f;	

	m_bHasInteractiveClip = false;

	m_iLastModifiedCoolCamID = m_iTransingOutLastCoolCamID = -1;
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::CloseClip
//! Clean up at the end of the clip
//!
//------------------------------------------------------------------------------------------

void NSPackage::CloseClip()
{
	// clear the condition list
	for ( ntstd::Vector<NSConditionStorage*>::const_iterator it = m_PCList.begin(); it != m_PCList.end(); ++it )
	{
		NSConditionStorage* pStore = *it;
		if( pStore->m_eCommand == NSConditionStorage::COM_TEST_INPUT )
		{
			pStore->SetCommandState(NSCommand::INPUT_STATE_NONE);
		}
		NT_DELETE( pStore );
	}
	m_PCList.clear();
}

bool NSPackage::SetupTransingOutCam(int iCurrentCoolCamID)
{
	//scee.sbashow : set cool camera out transition to game camera....
		// scee.sbashow: note, current code assumes transitioning out
		//					will happen on the last clip finishing
		//					but if the last cool cam happens on some mid clip
		//					then there will be no proper transition.
		//					This may need to be looked at.
		//					Also, am not happy with this SetScratchTransition. At all.
		//					Also, if not happy with that, have you thought of just forcing through
		//					the mapped level cam to be the current level cam before the last cool cam?? need to try.
	
    if (GetActiveInstance()->GetToCamCurrentTransitionDef().GetTotalTime()>0.0f)
	{
#if 1
		ntError(m_iTransingOutLastCoolCamID==-1);
		CoolCamera* pobCam = 
			CamMan::Get().GetView(m_iCamView)->GetCoolCam(iCurrentCoolCamID);

		//scee.sbashow : setup the current level cam to have a scratch (oe one time only) transition
		//				reference to whatever the GetActiveInstance()->GetToCamLevelOutTransitionDef() camera refers to
		//				This GetActiveInstance()->GetToCamLevelOutTransitionDef() definition is specified through the mapping for the NS as a whole.
		const BasicCamera* const pobCamCurrLevel = 
			static_cast<const BasicCamera*>(CamMan::Get().GetView(m_iCamView)->GetLevelCamera());

		ntAssert (pobCamCurrLevel)
		{
			if (!GetActiveInstance()->GetToCamLevelOutTransitionDef().GetDestination())
			{
				DataObject* const pobDO = 
					ObjectDatabase::Get().GetDataObjectFromName( GetActiveInstance()->GetCamLevelOutName() );

				if (pobDO)
				{
					GetActiveInstance()->GetToCamLevelOutTransitionDef().SetDestCamera(reinterpret_cast<BasicCameraTemplate*>(pobDO->GetBasePtr()));
				}
				else
				{
					return false;
				}
			}

			// GetActiveInstance()->GetToCamLevelOutTransitionDef() has its lifetime bound up with the NSPackage,
			//				whose lifetime is itself bound up with the NinjaSequence,
			//				which is constructed/destructed by the object database
			//				So this should be safe.
			pobCamCurrLevel->SetScratchTransition( GetActiveInstance()->GetToCamLevelOutTransitionDef() );
		}

		pobCam->SetEndingTransition( &GetActiveInstance()->GetToCamCurrentTransitionDef() );

		// scee.sbashow : call EndCamera here, so that it 
		//					will play the POIR EndingTransition
		//					see camview.cpp's update on its list of CoolCam's
		pobCam->EndCamera();
		m_iTransingOutLastCoolCamID = iCurrentCoolCamID;
		return true;
#endif
	}
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::StartClip
//! Starts a clip
//!
//! \param pobNS	Pointer to the NS entity
//!
//------------------------------------------------------------------------------------------
void NSPackage::StartClip()
{
	m_bClipStarted = true;

	// update the package's current clip ptr.
	if( m_bFirstClip )
	{
		// If we're being limited to play back only a given named clip, then find 
		//	the named clip and set it as current.
		// [nb - used by (eg) AnimEvent Editor to preview only a given named clip]
		const ntstd::String playbackClipName = NSManager::Get().GetPlaybackClipName();
		if( playbackClipName.length() )
		{
			m_pClip = FindClipObject( playbackClipName.c_str() );
			user_error_p( m_pClip, ("NS: No playback clip - user-specified name was invalid") );
		}
		else
		{
		// first clip in the sequence
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( m_pNSContainer->m_ClipList.front() );
		user_error_p( pDO, ("NS: No first clip - check exported XML file") );
		m_pClip = (NSClip*)( pDO->GetBasePtr() );
		}
		
		m_bFirstClip = false;
	
		m_bAllowingForMotionGrace = false;
	
	}
	else
	{
		user_error_p( m_pTargetClip, ("NS: No target clip - check exported XML file") );
		m_pClip = m_pTargetClip;

		// scee.sbashow: only consider a motion grace when NOT in tutorial mode
		//					this is because wrong moves are considered ok in tutorial mode anyway...
		m_bAllowingForMotionGrace = (GetActiveInstance()->GetNSMode()!=NinjaSequence::NS_MODE_TUTORIAL);
	}

	// scee.sbashow: flags when the simple average shake/mash is sufficient
	m_bMeasureAverageNumberPressesMet = false;

	//	scee.sbashow: note how close the actual measure is to passing
	m_fProximityToFrequencyPassing  = 0.0f;

	//	scee.sbashow: used during tutorials to work out how fast the game should run. 
	//					(0->1 ... still->normal speed)
	m_fTutorialScalarCurr = 1.0f;

#ifdef DEBUG_PLAYSINGLECLIP
	m_pClip = FindClipObject( DEBUG_PLAYSINGLECLIP );
#endif

	// set target clip name to default target clip
	m_pcTargetClipName = m_pClip->m_sDefaultGotoClip.c_str();
	
	// player killed if fails a clip with target clip name of 'DIE'
	m_bPlayerKilled = TestIfDieClip( m_pcTargetClipName );

	// player killed if fails a clip with target clip name of 'DIE'
	m_bFailed = TestIfFailureClip( m_pcTargetClipName );

	// if default target clip is DIE or END, or we're only playing a single named clip, then set ptr to NULL
	if( !TestIfEndClip( m_pcTargetClipName ) && !m_bPlayerKilled && !m_bFailed && !NSManager::Get().GetPlaybackClipName().length() )
	{
		m_pTargetClip = FindClipObject( m_pcTargetClipName );
	}
	else
	{
		m_pTargetClip = NULL;
		NSManager::Get().SetPlaybackClipName("");
	}

	// preprocess the conditions so I don't have to parse strings every frame
	PreprocessConditions();

	// link up overlay button patterns with potential TEST_INPUT condition patterns
	LinkOverlays();

	// go through anims creating the necessary target entities (or grabbing existing ones), or modifying the camera anim
	for ( ntstd::List<NSAnim*>::const_iterator it = m_pClip->m_AnimList.begin(); it != m_pClip->m_AnimList.end(); ++it )
	{
		NSAnim* pAnim = ( *it );
		
		// is it the camera's anim?
		if( strstr( pAnim->m_pTargetEntity->m_sName.c_str(), "camera" ) )
		{
			// get anim container name for camera anims
			user_error_p(pAnim->m_pTargetEntity->m_pCameraAnimator, ("NS: camera anim container isn't resident"));

			// store a new Maya camera (will be triggered at the start of the next frame to sync with entity anims)
			// No! Start it now, paused, then set it going next frame...
			StorePendingCamera(pAnim->m_pTargetEntity->m_pCameraAnimator, pAnim->m_sTargetAnim.c_str());
		}
		else
		{
			// Loop through all our clips and make sure that all the data is built up now
			SetNSEntityForClip( pAnim->m_pTargetEntity );
				
			// start its clip animation
			SimpleRelativeTransitionDef obDef;
			obDef.m_pobAnimation = pAnim->m_pTargetEntity->m_pGameEntity->GetAnimator()->CreateAnimation(pAnim->m_sTargetAnim.c_str());
			obDef.m_fMovementDuration = m_pClip->m_fDuration;

			const NSEntityInstance* const pobNEEntInstance = this->GetActiveInstance();

			// apply offset from mapped ninja sequence entity
			CMatrix Mat;
			Mat.SetFromQuat( pobNEEntInstance->GetRotation() );
			Mat.SetTranslation( pobNEEntInstance->GetPosition() );

			//scee.sbashow : note to self: was worried about this peice of code when I saw it -
			//					but this new will eventually be freed because obDef.m_bOwnsTransform 
			//					is set to true (below) meaning that the controller will delete this def member
			obDef.m_pobRelativeTransform = NT_NEW Transform();

			obDef.m_pobRelativeTransform->SetLocalMatrix( Mat );
			CHierarchy::GetWorld()->GetRootTransform()->AddChild( obDef.m_pobRelativeTransform );
			obDef.m_bOwnsTransform = true;
			pAnim->m_pTargetEntity->m_pGameEntity->GetMovement()->BlockNewControllers( false );
			pAnim->m_pTargetEntity->m_pGameEntity->GetMovement()->BringInNewController( obDef, CMovement::DMM_HARD_RELATIVE, 0.0f );
			pAnim->m_pTargetEntity->m_pGameEntity->GetMovement()->BlockNewControllers( true );
		}
	}

	// scee.sbashow: reset clip triggered setting to zero, 
	// 			so that if this clip is replaying, the various events will be tested again.
	for ( ntstd::List< NSEvent* >::const_iterator it = m_pClip->m_EventList.begin(); it != m_pClip->m_EventList.end(); ++it )
	{
		NSEvent* const pobEvent = *it;
		pobEvent->Reset();
	}

	// start blendshape anims
	if ( XPUShapeBlending::Get().IsEnabled() ) 
	{
		for ( ntstd::List<NSBSAnim*>::const_iterator it = m_pClip->m_BSAnimList.begin(); it != m_pClip->m_BSAnimList.end(); ++it )
		{
			CEntity* pTargetEntiy = (*it)->m_pTargetEntity->m_pGameEntity;
			ntError( pTargetEntiy );
			user_error_p( pTargetEntiy->IsBlendShapesCapable(), ("NS BSANIMATION - %s: Trying to play a bsanim clip on %s. Entity is not blendshapes-capable\n", m_pcTargetClipName, pTargetEntiy->GetName().c_str()) )
			user_error_p( pTargetEntiy->GetBlendShapesComponent(), ("NS BSANIMATION - %s: Cannot play bsanim on %s. Entity does not have a BlendshapesComponent\n", m_pcTargetClipName, pTargetEntiy->GetName().c_str() ) )
			user_error_p( pTargetEntiy->GetBlendShapesComponent()->GetBSAnimator(), ("NS BSANIMATION - %s: %s's BlendShapesComponent doesn't have a valid BSAnimator installed\n", m_pcTargetClipName, pTargetEntiy->GetName().c_str() ) );
			pTargetEntiy->GetBlendShapesComponent()->GetBSAnimator()->Play( (*it)->m_sTargetAnim.c_str() );
		}
	}

	// reset timer to zero
	ResetClipTimers();

	// reset vars.
	m_bPostProcessedConditions = false;
	m_fTickTimer = 0.0f;
	m_fTickPeriod = 0.0f;

}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::ModifyCamera
//! Plays the clip camera anim
//!
//! \param pEntity	Pointer to the NS entity
//! \param bShow	Toggles the rendering (& entity processing)
//!
//------------------------------------------------------------------------------------------
void NSPackage::ModifyCamera()
{
	ntError( m_pcCameraAnimName );

	if (m_iLastModifiedCoolCamID>-1)
	{
		// MUST remove cool cams before creating any new ones, as we share our animator between cool cams.
		// removing any previous cool cams after we create the next one will reset our animator, killing the
		// new animation. sigh.
		CamMan::Get().GetView(m_iCamView)->RemoveCoolCamera(m_iLastModifiedCoolCamID);
	}

	CHashedString cam_anim_name( m_pcCameraAnimName );
	CoolCam_MayaDef CoolCamDef;
	CoolCamDef.pCoolAnimator = m_pCameraAnimator;
	CoolCamDef.sAnim = cam_anim_name;
	if(m_pClip->m_pTweaker)
	{
		CoolCamDef.pTweaker = m_pClip->m_pTweaker;
	}
	CoolCam_Maya* const pCoolCam = 
		NT_NEW CoolCam_Maya(*CamMan::Get().GetView(m_iCamView), CoolCamDef);

	const NSEntityInstance* const pobNEEntInstance = GetActiveInstance();

	// apply mapped offset from ninja sequence entity
	CMatrix Mat;
	Mat.SetFromQuat( pobNEEntInstance->GetRotation() );
	Mat.SetTranslation( pobNEEntInstance->GetPosition() );
	pCoolCam->SetMatrix( Mat );


	// if animation is shorter than clip then let camera go back to game cam
	if( pCoolCam->GetAnimDuration() > (m_pClip->m_fDuration - GAME_CAMERA_THRESHOLD) )
	{
		pCoolCam->SetAutoFinish( false );
	}
	else
	{
		//OSD::Add( OSD::DEBUG_CHAN, 0xffff0000, "RELEASE CAMERA EARLY" );
	}

	CamMan::Get().GetView(m_iCamView)->AddCoolCamera(pCoolCam);
	m_iLastModifiedCoolCamID = pCoolCam->GetID();

	m_pcCameraAnimName = 0;
}   	


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::StorePendingCamera
//! Stores the information to start a new camera
//! This is held for a frame as the entity anim controllers take a frame to get going
//! whilst the camera gets going immediately
//!
//! \param pcCameraAnimName				Anim name
//! \param pcCameraAnimContainerName	Container name
//!
//------------------------------------------------------------------------------------------

void NSPackage::StorePendingCamera(CoolCam_MayaAnimator* pCameraAnimator, const char* pcCameraAnimName)
{
	ntError(pcCameraAnimName);

	m_pCameraAnimator = pCameraAnimator;
	m_pcCameraAnimName = pcCameraAnimName;
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::ShowEntity
//! Shows or hides an entity
//!
//! \param pEntity	Pointer to the NS entity
//! \param bShow	Toggles the rendering (& entity processing)
//!
//------------------------------------------------------------------------------------------

void NSPackage::ShowEntity( CEntity* pEntity, bool bShow )
{
	if(bShow)
	{
		pEntity->Show();
	}
	else
	{
		pEntity->Hide();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::ShowTransform
//! Shows or hides all meshes under a transform
//!
//! \param pEntity			Pointer to the NS entity
//! \param pcTransformName	Transform name
//! \param bShow			Toggles the rendering (& entity processing)
//!
//------------------------------------------------------------------------------------------

void NSPackage::ShowTransform( CEntity* pEntity, const char* pcTransformName, bool bShow )
{
	CRenderableComponent* pobRenderable = pEntity->GetRenderableComponent();
	ntError( pobRenderable );

	CHierarchy* pobH = pEntity->GetHierarchy();
	ntError( pobH );

	const int iIdx = pobH->GetTransformIndex( CHashedString( pcTransformName ) );
	ntError_p( iIdx !=-1, ( "transform '%s' not found on hierarchy for entity %s!", pcTransformName, ntStr::GetString(pEntity->GetName()) ) );
	Transform* pobTransform = pobH->GetTransform( iIdx );


	pobRenderable->EnableAllByTransform( pobTransform, bShow, true );
}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::FindClipObject
//! Finds a clip container by name and returns a ptr to the object
//!
//! \param	pcName	Name of clip object to find
//! \return	Ptr to clip object
//------------------------------------------------------------------------------------------

NSClip* NSPackage::FindClipObject( const char* pcName )
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( pcName );
	user_error_p(pDO, ("Could not find %s for NinjaSequence\n", pcName));

	NSClip* pClip = (NSClip*)( pDO->GetBasePtr() );
	m_pcClipName = ntStr::GetString(pDO->GetName());

	return pClip;
}




//------------------------------------------------------------------------------------------
//!
//!	NSPackage::UpdateCommandFeedback
//!
//! \param	fDt		Frame timestep
//------------------------------------------------------------------------------------------

void NSPackage::UpdateCommandFeedback( float fDt )
{
	// this will be greater than zero if a right or wrong input has been given
	if( m_fTickTimer > 0.0f )
	{
		//ntAssert(m_bTestInputOverLayBeingRequested);
		m_fTickTimer -= fDt;
		m_fTickPeriod += fDt;
		if( m_fTickTimer < 0.0f ) m_fTickTimer = 0.0f;

		// find the condition so I can display the correct button
		for ( ntstd::Vector<NSConditionStorage*>::const_iterator it = m_PCList.begin(); it != m_PCList.end(); ++it )
		{
			NSConditionStorage* pStore = *it;
			if( pStore->m_eCommand == NSConditionStorage::COM_TEST_INPUT )
			{
				if( m_bTick ) 
				{
					pStore->SetCommandState(NSCommand::INPUT_STATE_CORRECT);
//					m_indicatorEffects.SetStatus( NSFX_SUCCEEDED, m_PCList );
				}
				else
				{
					pStore->SetCommandState(NSCommand::INPUT_STATE_WRONG);
//					m_indicatorEffects.SetStatus( NSFX_FAILDED, m_PCList );
				}
				
				break;
			}
		}
	}
	else
	{
		if (m_bTestInputOverLayBeingRequested)
		{
			for ( ntstd::Vector<NSConditionStorage*>::const_iterator it = m_PCList.begin(); it != m_PCList.end(); ++it )
			{
				NSConditionStorage* pStore = *it;
				if( pStore->m_eCommand == NSConditionStorage::COM_TEST_INPUT )
				{
					pStore->SetCommandState(NSCommand::INPUT_STATE_LISTEN);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::UpdateDisplay
//!		Here is where the commands have there icon effects applied to the standard overlays for motion
//!			during the tutorial period. It is a general method, not just for this, though currently only does this
//!
//------------------------------------------------------------------------------------------
void NSPackage::UpdateDisplay( float fDt )
{
	const float fListenTestWindow =  
		(m_pClip->m_fWindowEndTime - m_pClip->m_fWindowStartTime) - WINDOW_GRACE_PERIOD;
	
	for ( ntstd::Vector<NSConditionStorage*>::const_iterator it = m_PCList.begin(); it != m_PCList.end(); ++it )
	{
		NSConditionStorage* pStore = *it;
		if( pStore->m_eCommand == NSConditionStorage::COM_TEST_INPUT )
		{
			for( ntstd::Vector<NSCommand>::iterator it = pStore->m_obCommandInputList.begin(); 
				 it != pStore->m_obCommandInputList.end(); ++it )
			{
				NSCommand& obCommand = *it;
				if (obCommand.GetCurrState()!= NSCommand::INPUT_STATE_NONE)
				{
					if (obCommand.GetCurrState() == NSCommand::INPUT_STATE_LISTEN)
					{
						obCommand.UpdateEffects(fDt, fListenTestWindow, m_fClipTime - m_pClip->m_fWindowStartTime );
					}
					else
					{
						obCommand.UpdateEffects(fDt, TICK_SHOW_PERIOD, m_fTickPeriod);
					}
				}
			}
		}
	}

}


//------------------------------------------------------------------------------------------
//!
//!	NSPackage::PlayFeedbackSound
//! Plays a right/wrong sound
//!
//! \param	bCorrect	True if 'right' feedback sound required
//!
//------------------------------------------------------------------------------------------

void NSPackage::PlayFeedbackSound( bool bCorrect )
{
	//const char* pcSoundBank = "environment_sb";
	//const char* pcCue;
	//if( bCorrect ) 
	//{
	//	pcCue = "button_positive";
	//}
	//else
	//{
	//	pcCue = "button_negative";
	//}

	//CAudioObjectID obID;
	//CAudioManager::Get().Create( obID, pcSoundBank, pcCue, SPATIALIZATION_NONE );
	//CAudioManager::Get().Play( obID );
	if( !bCorrect ) 
	{
		AudioHelper::PlaySound("env_sb","button_negative");
	}
}

//------------------------------------------------------------------------------------------
//!
//!	NSPackage::RemoveEntity
//! Removes an entity from the package's entity list - does NOT destroy the entity
//! and doesn't actually use the entity class so it's safe to use the stored
//! entity ptr.
//!
//! \param	pEntity		Entity to remove
//!
//------------------------------------------------------------------------------------------

void NSPackage::RemoveEntity( CEntity* pEntity, NSEnt* pNSEnt )
{
	for( ntstd::Vector< NSEnt* >::iterator nse_it = m_EntityList.begin(); nse_it != m_EntityList.end(); )
	{
		NSEnt* pEnt = *nse_it;
		CEntity* pGameEntity = pEnt->m_pGameEntity;
		if( pEntity == pGameEntity || pNSEnt == pEnt )
		{
			// remove from the package's entity list
			nse_it = m_EntityList.erase( nse_it );
			
			// No longer apart of the ns package
			pEnt->SetHome( 0 );
			
			// no need to keep checking
			break;
		}
		else ++nse_it;
	}
}

#include "fsm.h"

//--------------------------------------------------
//!
//! NSEntity Instance State Machine - was the ninjasequence.lua
//!		NinjaSequence_Entity is typedef'd to NSEntityInstance
//!
//--------------------------------------------------
STATEMACHINE(NSEI_FSM, NinjaSequence_Entity)

//-------------- Setup --------------//
	NSEI_FSM(const CHashedString& obInitState)
	{
		// TODO: Move this to an enumeration
		if			( obInitState == CHashedString("Default") )										SET_INITIAL_STATE( NSEI_DEFAULTSTATE );
		else		user_error_p(0, ("Unrecognised playerstate %s in NSEI_DEFAULTSTATE\n", 			ntStr::GetString(obInitState) ) );
	}

//-------------- Global msg --------------//
//	BEGIN_EVENTS
//		EVENT()
//		END_EVENT(true)
//	END_EVENTS

//-------------- NSEI states --------------//
	STATE(NSEI_DEFAULTSTATE)
		BEGIN_EVENTS
			ON_ENTER
			END_EVENT(true)

			EVENT(Trigger)
				ME->ActivateInstance(msg.IsInt(2) ? msg.GetInt(2) : 0);
				
				SET_STATE( NSEI_CHECKING );
			END_EVENT(true)
			END_EVENTS
		END_STATE // NSEI_DEFAULTSTATE

	STATE(NSEI_CHECKING)
		BEGIN_EVENTS
			ON_UPDATE
			END_EVENT(true)

			EVENT(msg_ninja_sequence_fail)
			if (ME->GetMessageHandler()) ME->GetMessageHandler()->ProcessEvent("OnFail");
			SET_STATE( NSEI_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_ninja_sequence_success)
			if (ME->GetMessageHandler()) ME->GetMessageHandler()->ProcessEvent("OnComplete");
			SET_STATE( NSEI_DEFAULTSTATE );
			END_EVENT(true)

			EVENT(msg_ninja_sequence_die)
			if (ME->GetMessageHandler()) ME->GetMessageHandler()->ProcessEvent("OnDie");
			SET_STATE( NSEI_DEFAULTSTATE );
			END_EVENT(true)

		END_EVENTS
	END_STATE // NSEI_CHECKING

END_STATEMACHINE //NSEI_FSM

NinjaSequence_Entity::NinjaSequence_Entity() :
	m_obPOIRDef(1.0f),
	m_obPOIRDefLevelNSOut(1.0f)
{
}

//------------------------------------------------------------------------------------------
//!
//!	NinjaSequenceEntityClearanceSphere::AddToEntityQuery
//! Addd spherical query information to the supplied query
//!
//------------------------------------------------------------------------------------------

void NinjaSequenceEntityClearanceSphere::AddToEntityQuery(CEntityQuery& obQuery, const NinjaSequence_Entity* pRelativeTo )
{
	// If required - apply the position relative to another entity.
	if( pRelativeTo && m_bRelativeToTriggeringEntity && pRelativeTo->GetTriggeringEntity() )
	{
		m_obSphere.Set( m_obPosition * pRelativeTo->GetTriggeringEntity()->GetMatrix(), m_fRadius );
	}
	else
	{
		m_obSphere.Set( m_obPosition, m_fRadius );
	}

	// Add the clause for the sphere
	obQuery.AddClause(m_obSphere);
}

//------------------------------------------------------------------------------------------
//!
//!	NSEntityInstance::ActivateInstance :
//!		NinjaSequence_Entity is typedef'd to NSEntityInstance
//!
//------------------------------------------------------------------------------------------

void NinjaSequence_Entity::ActivateInstance(int iView)
{
	NinjaSequence* const pNSContainer = GetNS();

	user_error_p(pNSContainer, ("%s can't find ninjasequence.\n", ntStr::GetString(GetName())));

	// Add what we have found to the NS Manager
	NSPackage* pPackage = pNSContainer->GetNSPackage();
	pPackage->SetView(iView);
	NSManager::Get().AddNSP( pPackage, this );

	// scee.sbashow:
	//		 set this specific mapped NS entity as the active instance of the package
	pNSContainer->GetNSPackage()->SetActiveInstance( this );
}

//------------------------------------------------------------------------------------------
//!
//!	NinjaSequence_Entity::OnPostConstruct :
//! 	NinjaSequence_Entity is typedef'd to NSEntityInstance
//!
//------------------------------------------------------------------------------------------
void NinjaSequence_Entity::OnPostConstruct()
{
	// scee.sbashow: backwards compat: make sure this is null, as not using anymore
	//	otherwise CEntity::OnPostPostConstruct() will try to call it!
	//	Indeed, will set this to null before CEntity::OnPostConstruct(), 
	//	just in case the CEntity logic for this is moved to the earlier 
	//	OnPostConstruct step
	m_ConstructionScript = CHashedString::nullString;
	
	CEntity::OnPostConstruct();

	InstallMessageHandler();

	m_eNSMode = IsTutorial() ? NinjaSequence::NS_MODE_TUTORIAL: NinjaSequence::NS_MODE_NORMAL;

	m_obPOIRDef.SetControlTransitionTime(GetFinalCameraOutTransition());
	m_obPOIRDefLevelNSOut.SetControlTransitionTime(GetFinalCameraOutTransition());		
	
	m_obLevelCamDest = 
		CHashedString(GetEndSequenceLevelCamName());

	// Create and attach the statemachine
	NSEI_FSM* pobFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) NSEI_FSM(CHashedString("Default"));
	ATTACH_FSM(pobFSM);

}

//------------------------------------------------------------------------------------------
//!
//!	NSEntityInstance::OnPostPostConstruct: 
//!		NinjaSequence_Entity is typedef'd to NSEntityInstance
//!
//------------------------------------------------------------------------------------------
void NinjaSequence_Entity::OnPostPostConstruct()
{
	LOAD_PROFILE( NinjaSequence_Entity_OnPostPostConstruct )

	CEntity::OnPostPostConstruct();

	m_obPOIRDefLevelNSOut.SetDestCamera(GetLevelCameraObject());		

	NinjaSequence* const pNSContainer = this->GetNS();

	if (pNSContainer)
	{
		if (!pNSContainer->GetNSPackage())
		{
			// Build the package, if it hasn't already been.
			pNSContainer->ConstructDescribedPackage(this);
		}

		// Add this instance to it
		pNSContainer->GetNSPackage()->AddInstance(this);
		
	}
}
