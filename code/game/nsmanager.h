//------------------------------------------------------------------------------------------
//!
//!	nsmanager.h
//!
//! Ninja Sequence and Cutscene Manager header
//!
//------------------------------------------------------------------------------------------

#ifndef _NSMANAGER_H
#define _NSMANAGER_H

// forward declarations
class CEntity;
class ObjectContainer;
class CoolCam_MayaAnimator;
class CoolCam_Maya;
class NSPackage;
class NSClip;
class NSCondition;
class NSAnim;
class NSBSAnim;
class NSEvent;
class BSAnimContainer;
class GhostGirlController;
class CMatrixTweakerEditor;
class NinjaSequence_Entity;

// includes
#include "core/nt_std.h"
#include "entitymanager.h"
#include "nseffects.h"
#include "camera/camtrans_poirot.h"
#include "hud/hudmanager.h"
#include "hud/hudimage.h"
#include "game/query.h"

//------------------------------------------------------------------------------------------
//!
//!	NinjaSequenceEntityClearanceVolume
//!	Abstract base class for clearance areas in ninja sequences
//!
//------------------------------------------------------------------------------------------
class NinjaSequenceEntityClearanceVolume
{
public:
	virtual void AddToEntityQuery(CEntityQuery& obQuery, const NinjaSequence_Entity* pRelativeTo = 0) = 0;
	virtual ~NinjaSequenceEntityClearanceVolume() {};
};

//------------------------------------------------------------------------------------------
//!
//!	NinjaSequenceEntityClearanceSphere
//!	Concrete implementation of clearance area
//!
//------------------------------------------------------------------------------------------
class NinjaSequenceEntityClearanceSphere : public NinjaSequenceEntityClearanceVolume
{
	HAS_INTERFACE(NinjaSequenceEntityClearanceSphere);
public:
	virtual void AddToEntityQuery(CEntityQuery& obQuery, const NinjaSequence_Entity* pRelativeTo = 0);
	virtual ~NinjaSequenceEntityClearanceSphere() {};

protected:

	// Here is'uth placed a query clause.
	CEQCProximitySphere m_obSphere;

private:
	// If the offset given is in world space, or space relative to another entity
	bool	m_bRelativeToTriggeringEntity;

	// Position
	CPoint	m_obPosition;
	
	// Radius
	float	m_fRadius;
};

//------------------------------------------------------------------------------------------
//!
//!	NinjaSequence
//!	XML interface class for Ninja Sequence container
//!
//------------------------------------------------------------------------------------------
class NinjaSequence_Entity;

typedef NinjaSequence_Entity NSEntityInstance;

class NinjaSequence
{
public:
	HAS_INTERFACE(NinjaSequence)

	// Construction/Destruction
	NinjaSequence( void );
	virtual ~NinjaSequence( void );

	// Callbacks so we can load and build our data
	void ConstructDescribedPackage( NSEntityInstance* pobEntity );

	// Access to the build Ninja Sequence package
	NSPackage* GetNSPackage( void ) { return m_pobNSPackage; }

	// Public serialised members
	float				m_fDuration;
	ntstd::List<NSClip*>	m_ClipList;

	enum NS_MODE {NS_MODE_NORMAL = 0 , NS_MODE_TUTORIAL};


	virtual void OnPostConstruct();
private:

    bool				m_bIsTutorial;
	float				m_fTutorialScalar;
	ntstd::String 		m_sNsFormatVersion;
	ntstd::String 		m_sExporterVersion;

	float m_fFinalCameraOutTransition;

	ntstd::String m_obEndSequenceLevelCamera;

	// The package we describe
	NSPackage*			m_pobNSPackage;
};


class NinjaSequence_Entity : public CEntity
{
	public:

		HAS_INTERFACE(NinjaSequence_Entity)

		NinjaSequence_Entity();

		bool	IsTutorial( void ) const							{	return m_bIsTutorial;		}
		float	TutorialScalar( void ) const						{	return m_fTutorialScalar; 	}

		// scee.sbashow : 
			// Let the mapper specify what the suitable first level camera after NS is
			//	can't just have the object ptr, as may not be 
			//	availavle if in a different area to the one the NS is associated with..?
		ntstd::String	GetEndSequenceLevelCamName( void ) const	{	return m_obEndSequenceLevelCamera; 	}
		// scee.sbashow : or just the camera direct if it is available...
		BasicCameraTemplate*	GetLevelCameraObject( void ) 		{	return m_pobLevelCameraObject; 		}

		// scee.sbashow : public, serialised value for 
			// transitioning out of sequence to game/lev cam.
		float			GetFinalCameraOutTransition( void ) const	{	return m_fFinalCameraOutTransition; }
		NinjaSequence* GetNS() 			const						{	return m_pobNinjaSequence;	}
		void OnPostConstruct();
        void OnPostPostConstruct();

		CamTrans_POIRotDef& 	GetToCamLevelOutTransitionDef( void )  const 	{	return m_obPOIRDefLevelNSOut;	}
		CamTrans_POIRotDef& 	GetToCamCurrentTransitionDef( void )   const	{	return m_obPOIRDef;				}

		const CHashedString& 	GetCamLevelOutName( void ) 		const		{	return m_obLevelCamDest;		}

		NinjaSequence::NS_MODE		GetNSMode()	const	{	return m_eNSMode;	}

		float 		GetTutorialBaseScalar() const	{	return m_fTutorialScalar;	}

		void ActivateInstance(int iView = 0);

		// A value defining the size of bounding box we use to do a visibility check on entities to clear out
		#define NS_ENTITY_CLEARANCE_BOUNDING_BOX_CHECK_HALF_EXTENT 1.5f
		void GetEntitiesInClearanceVolumes();
		void TryClearEntitiesInClearanceVolumes();
	
		// Accessing the triggering entity
		const CEntity* GetTriggeringEntity(void) const { return m_pTriggeringEntity; }
		void SetTriggeringEntity(const CEntity* pEnt) { m_pTriggeringEntity = pEnt; }

	private:
		// Exposed list of volumes for clearance in the NS
		ntstd::List<NinjaSequenceEntityClearanceVolume*> m_obEntityClearanceVolumes;
		// Unexposed query constructed from the above volumes
		CEntityQuery			m_obEntityClearanceVolumesQuery;

		// published
		BasicCameraTemplate* 	m_pobLevelCameraObject; 	
		float 					m_fFinalCameraOutTransition;
		ntstd::String 			m_obEndSequenceLevelCamera;
		bool					m_bIsTutorial;
		float					m_fTutorialScalar;
		NinjaSequence* 			m_pobNinjaSequence;


		// This is pointer holds the entity that triggered the NS to start. 
		// It is quite possible that'll be null as many NS's don't trigger via
		// a entities trigger volume. 
		// Use this with caution. 
		const CEntity*	m_pTriggeringEntity;


		// non published.
		NinjaSequence::NS_MODE m_eNSMode;
		mutable CamTrans_POIRotDef 	m_obPOIRDef;				// Cam Transition object out to level camera after last clip from cool camera.
		mutable CamTrans_POIRotDef 	m_obPOIRDefLevelNSOut;		// Cam Transition object out to new level camera after NS from level camera just as NS ends.
		CHashedString m_obLevelCamDest;

		// a hangover from the past, or useful ?
		ntstd::String 			m_obStartupScript;
		ntstd::String 			m_obShutdownScript;
};

//------------------------------------------------------------------------------------------
//!
//!	NsClip
//!	XML interface class for Ninja Sequence clips
//!
//------------------------------------------------------------------------------------------


class NSClip
{
public:
	float				m_fDuration;
	float				m_fWindowStartTime;
	float				m_fWindowEndTime;
	ntstd::String		m_sDefaultGotoClip;
	ntstd::List<NSCondition*>	m_ConditionList;
	ntstd::List<NSAnim*>		m_AnimList;
	ntstd::List<NSBSAnim*>		m_BSAnimList;
	ntstd::List<NSEvent*>		m_EventList;
	CMatrixTweakerEditor*       m_pTweaker;
};


//------------------------------------------------------------------------------------------
//!
//!	NsCondition
//!	XML interface class for Ninja Sequence conditions
//!
//------------------------------------------------------------------------------------------


class NSCondition
{
public:
	ntstd::String		m_sTest;
	ntstd::String		m_sIfTrue;
	ntstd::String		m_sIfFalse;
};


//------------------------------------------------------------------------------------------
//!
//!	NsEntity
//!	XML interface class for Ninja Entities
//!
//------------------------------------------------------------------------------------------


class NSEnt
{
public:

	// Construction / Destruction
	NSEnt( void );
	~NSEnt( void );

	ntstd::String			m_sName;
	ntstd::String			m_sClump;
	ntstd::String			m_sPhysics;
	ntstd::String			m_sBSClump;
	ObjectContainer*			m_pAnimContainer;
	BSAnimContainer*		m_pBSAnimContainer;
	CEntity*				m_pGameEntity;
	CoolCam_MayaAnimator*	m_pCameraAnimator;
	GhostGirlController*	m_pEffectController;
	
	// The current nspackage home for the entity
	NSPackage*				m_pobHome;
	
	// flags used to restore the entity to the same configuration
	// before it was grabbed by the NS
	bool					m_bHadAIEnabled;
	bool					m_bHadMovementComp;
	bool					m_bHadAnimatorComp;
	bool					m_bHadPhysicsComp;
	bool					m_bAnimContainerAdded;	//true if container was added
	bool					m_bBSAnimContainerAdded;
	bool					m_bBlendShapesComponentAdded;
	bool					m_bBSClumpAdded;
	bool					m_bHadAttackComp;
	bool					m_bNSEntity;
	bool					m_bInvisibleAfterClip;	// true if it should be made invisible *after* the clip finishes
	
	bool	HasBlendShapeClump( void ) const { return( m_sBSClump != ""); }
	void	SetHome( NSPackage* pHome ) { ntError( (bool)m_pobHome ^ (bool)pHome  ); m_pobHome = pHome; }
};


//------------------------------------------------------------------------------------------
//!
//!	NSAnim
//!	XML interface class for Ninja Sequence anims
//!
//------------------------------------------------------------------------------------------


class NSAnim
{
public:
	ntstd::String		m_sTargetAnim;
	NSEnt*				m_pTargetEntity;
};


//------------------------------------------------------------------------------------------
//!
//!	NSBSAnim
//!	XML interface class for Ninja Sequence blendshape anims
//!
//------------------------------------------------------------------------------------------


class NSBSAnim
{
public:
	ntstd::String		m_sTargetAnim;
	NSEnt*				m_pTargetEntity;
};


//------------------------------------------------------------------------------------------
//!
//!	NsEvent
//!	XML interface class for Ninja Sequence events
//!
//------------------------------------------------------------------------------------------


class NSEvent
{
public:
	float				m_fTriggerTime;
	ntstd::String		m_sEvent;
	bool				m_bTriggered;

	NSEvent() {	Reset(); };
	~NSEvent(){};
	void Reset( void )	{ m_bTriggered = false;	}
};


//------------------------------------------------------------------------------------------
//!
//!	NsClip
//!	XML interface class for Ninja Sequence clips
//!
//------------------------------------------------------------------------------------------


class NSButtonSpec
{
public:

	HudImageRenderDef*	m_pobHudDefShake;
	HudImageRenderDef*	m_pobHudDefPad;
	HudImageRenderDef*	m_pobHudDefTutorialLeft;
	HudImageRenderDef*	m_pobHudDefTutorialRight;
	HudImageRenderDef*	m_pobHudDefTutorialUp;
	HudImageRenderDef*	m_pobHudDefTutorialDown;
	HudImageRenderDef*	m_pobHudDefTutorialUpDown;

	float				m_fDirection_top_size;
	float				m_fDirection_top_right_size;
	float				m_fDirection_top_left_size;
	float				m_fDirection_bottom_size;
	float				m_fDirection_bottom_right_size;
	float				m_fDirection_bottom_left_size;
	float				m_fDirection_left_size;
	float				m_fDirection_right_size;

	float				m_fButton_triangle_size;
	float				m_fButton_square_size;
	float				m_fButton_cross_size;
	float				m_fButton_circle_size;

	float				m_fDirection_top_alpha;
	float				m_fDirection_top_right_alpha;
	float				m_fDirection_top_left_alpha;
	float				m_fDirection_bottom_alpha;
	float				m_fDirection_bottom_right_alpha;
	float				m_fDirection_bottom_left_alpha;
	float				m_fDirection_right_alpha;
	float				m_fDirection_left_alpha;

	float				m_fButton_triangle_alpha;
	float				m_fButton_circle_alpha;
	float				m_fButton_cross_alpha;
	float				m_fButton_square_alpha;

	float				m_fDirection_top_pos[2];
	float				m_fDirection_top_right_pos[2];
	float				m_fDirection_top_left_pos[2];
	float				m_fDirection_bottom_pos[2];
	float				m_fDirection_bottom_right_pos[2];
	float				m_fDirection_bottom_left_pos[2];
	float				m_fDirection_right_pos[2];
	float				m_fDirection_left_pos[2];

	float				m_fButton_triangle_pos[2];
	float				m_fButton_circle_pos[2];
	float				m_fButton_cross_pos[2];
	float				m_fButton_square_pos[2];

	ntstd::String		m_sButton_triangle_texture;
	ntstd::String		m_sButton_square_texture;
	ntstd::String		m_sButton_cross_texture;
	ntstd::String		m_sButton_circle_texture;
	ntstd::String		m_sButton_triangle_texture_w;
	ntstd::String		m_sButton_square_texture_w;
	ntstd::String		m_sButton_cross_texture_w;
	ntstd::String		m_sButton_circle_texture_w;
	ntstd::String		m_sButton_triangle_mash_texture;
	ntstd::String		m_sButton_square_mash_texture;
	ntstd::String		m_sButton_cross_mash_texture;
	ntstd::String		m_sButton_circle_mash_texture;

	ntstd::String		m_sDirection_top_texture;
	ntstd::String		m_sDirection_top_right_texture;
	ntstd::String		m_sDirection_top_left_texture;
	ntstd::String		m_sDirection_bottom_texture;
	ntstd::String		m_sDirection_bottom_right_texture;
	ntstd::String		m_sDirection_bottom_left_texture;
	ntstd::String		m_sDirection_left_texture;
	ntstd::String		m_sDirection_right_texture;
	ntstd::String		m_sDirection_top_texture_w;
	ntstd::String		m_sDirection_top_right_texture_w;
	ntstd::String		m_sDirection_top_left_texture_w;
	ntstd::String		m_sDirection_bottom_texture_w;
	ntstd::String		m_sDirection_bottom_right_texture_w;
	ntstd::String		m_sDirection_bottom_left_texture_w;
	ntstd::String		m_sDirection_left_texture_w;
	ntstd::String		m_sDirection_right_texture_w;

	float				m_fCorrect_expand_rate;	// how many seconds to double in size
	float				m_fCorrect_fade_rate;	// how many seconds to fade completely
	float				m_fWrong_fade_rate;		// how many seconds to fade completely
	float				m_fDrop_fade_rate;		// how many seconds to fade completely
	float				m_fDrop_accel_rate;		// accel rate (m/s^2)
	float				m_fWobbleXRate;			// full oscillations per second on X
	float				m_fWobbleYRate;			// full oscillations per second on Y
	float				m_fWobbleXAmplitude;	// oscillation amplitude on X
	float				m_fWobbleYAmplitude;	// oscillation amplitude on Y
	float				m_fHDRBase;				// HDR base colour
};



class NSCommand
{
public:

	enum CONTROLLER_INPUT_TYPE {	NS_COUNTER = 0, NS_GRAB, NS_ACTION, NS_ATTACK, 
									NS_UPLEFT, NS_UPRIGHT, NS_DOWNLEFT, NS_DOWNRIGHT, 
									NS_MOVEUP, NS_MOVELEFT, NS_MOVERIGHT, NS_MOVEDOWN,
									NS_LASTINPUT_TYPE };

	struct RenderSet
	{
		RenderSet()
		{
			m_obRenderInstance = 
			m_obRenderHelperPad = 
			m_obRenderHelperArrowMotion = 0;
		}
		HudImageRenderer*			m_obRenderInstance;
		HudImageRenderer*			m_obRenderHelperPad;
		HudImageRenderer*			m_obRenderHelperArrowMotion;
		HudImageOverlaySlap   		m_obRenderDefOverlay;					// scee.sbashow - hud overlay command object to apply.
	
	};


	NSCommand(CONTROLLER_INPUT_TYPE eInputID, 
			  bool bTutorialMode, bool bIsTestInput);
	

	enum INPUT_RENDER_STATE { INPUT_STATE_NONE = 0, INPUT_STATE_LISTEN, INPUT_STATE_CORRECT, INPUT_STATE_WRONG, INPUT_STATE_NUM };

    INPUT_RENDER_STATE			GetCurrState( void ) const			{	return m_eCurrInputState;	  }
    void						SetState(INPUT_RENDER_STATE eState);
	void 						UpdateEffects(float fDT, float fEffectPeriod, float fCurrentEffectTime);
	const HudImageOverlaySlap 	GetTutorialBaseAppliedOverlay( void ) const;
	void 						GetMotionDirs( float& fDX, float& fDY) const;
	
	const ntstd::String 		GetTutorialString( void ) const;

	bool	operator==( const NSCommand& obOther)  const  {	return m_eInputID==obOther.m_eInputID;	}

	operator	int() const
	{
		return m_eInputID;
	}

private:

	CONTROLLER_INPUT_TYPE	m_eInputID;
	INPUT_RENDER_STATE		m_eCurrInputState;				// scee.sbashow - better to just flag this and use the hud for rendering effects. 
	RenderSet	*			m_pobRenderSet;
	bool					m_bTutorialMode;
};

//------------------------------------------------------------------------------------------
//!
//!	NSConditionPreprocess
//!	Stores parsed condition per clip
//!
//------------------------------------------------------------------------------------------

class NSConditionStorage
{
public:

	// scee.sbashow: we really need to get rid of this command distinction
	//				there should be one set of movement commands
	//				and the attached image is now through the hud, not a sprite overlay spec.
    enum CONDITION_COMMAND { UNKNOWN = 0, COM_TEST_INPUT, COM_IN_WINDOW };

	enum CONDITION_SUBCOMMAND { SUB_UNKNOWN = 0, SUB_SHOW_SPRITE };

	NSConditionStorage() : 
		m_iGeneralInt1( 0 ),
		m_fGeneralFloat1( 0.0f ),
		m_bInputBlocked( false ),
		m_bCorrect( false ),
		m_fTargetPressGap( 0.0f ),
		m_iNumTruePresses( 0 ),
		m_fCurrentHitRateMeasureFunction( 0.0f ),
		m_fCurrentHitRateMeasureCount(0.0f),
		m_bLastMotionDirPositive(true),
		m_fWindowPenaltyTime( 0.0f ),
		m_bConditionTrueExecuted( false ),
		m_pLinkedStorage( NULL ),
		m_fLastOVerlayDisplay( 99999.0f ),
		m_bAllowOverlaySnap( true ),
		m_eCommand( UNKNOWN ),
		m_eSubCommand( SUB_UNKNOWN ) {};
	
	~NSConditionStorage()
	{
		// free allocated strings
		for ( ntstd::Vector<char*>::const_iterator it = m_stringdatalist1.begin(); it != m_stringdatalist1.end(); ++it )
		{
			char *pcString = *it;
			ntAssert( pcString );
			NT_DELETE_ARRAY( pcString );
		}
		for ( ntstd::Vector<char*>::const_iterator it = m_stringdatalist2.begin(); it != m_stringdatalist2.end(); ++it )
		{
			char *pcString = *it;
			ntAssert( pcString );
			NT_DELETE_ARRAY( pcString );
		}
		
		// clear lists
		m_obCommandInputList.clear();
		m_stringdatalist1.clear();
		m_stringdatalist2.clear();
	};

	void SetCommandState( NSCommand::INPUT_RENDER_STATE eState );


	CVector					m_oldOverlayPos;

	ntstd::Vector<NSCommand>		m_obCommandInputList;
	ntstd::Vector<char*>			m_stringdatalist1;

	// scee.sbashow : note, this is not being used. What was it originally for?
	ntstd::Vector<char*>			m_stringdatalist2;

	int						m_iGeneralInt1;
	float					m_fGeneralFloat1;
	
	bool					m_bInputBlocked;		// true if input to this condition is currently blocked
	bool					m_bCorrect;				// true if condition is satisfied
	float					m_fTargetPressGap;		// target average press gap
	int						m_iNumTruePresses;		// number of correct presses for this condition
	float					m_fCurrentHitRateMeasureFunction; // scee.sbashow: a measure of the frequency of hits - is sensitive throughout window period, to allow for better consistency.
	float					m_fCurrentHitRateMeasureCount;	 //  scee.sbashow: a this is used as a counter to know when to start decaying m_fCurrentHitRateMeasureFunction exponentially if enough time
																		//		has passed since the last correct hit during the button-mash input testing regime.

	bool					m_bLastMotionDirPositive;		// scee.sbashow - for motion controller replacement of button mash, needs to pick up going in opposite direction to consider a 'press'

	NSClip*					m_pTargetClipTrue;
	NSClip*					m_pTargetClipFalse;
	float					m_fWindowPenaltyTime;
	bool					m_bConditionTrueExecuted;// flags if the command to execute if the param is correct has been run
	NSConditionStorage*		m_pLinkedStorage;		// link to another storage class (e.g. for show_sprites to test_inputs)
	float					m_fLastOVerlayDisplay;	// time since last overlay display for flashing
	bool					m_bAllowOverlaySnap;	// true if overlay can snap to new position (clip start)
	
	CONDITION_COMMAND		m_eCommand;
	CONDITION_SUBCOMMAND	m_eSubCommand;
};


//------------------------------------------------------------------------------------------
//!
//!	NSPackage
//!	Contains data for each NS instance
//!
//------------------------------------------------------------------------------------------

class NSPackage
{
public:
	enum NS_STAGE { NS_TRIGGER = 0, NS_LOADING, NS_PLAYING, NS_TRANSINGOUT, NS_DISCARD };

	static const char* m_pcEndClipString;
	static const char* m_pcDieClipString;
	static const char* m_pcFailureClipString;

	// Construction/destruction
	NSPackage( NSEntityInstance* pNSEntInstancer);
	~NSPackage();

	// To be called when added to the NS Manager
	void ResetPackage();

	// Public interface
	NS_STAGE		GetNSStage( void ) const { return m_eStage; }
	void			SetNSStage( NS_STAGE eStage ) { m_eStage = eStage; }
	void			Update( float fDt );
	void			CloseClip( void );
	void			RemoveEntity( CEntity* pEntity, NSEnt* pNSEnt = 0 );
	ntstd::String	GetName() const {return "NinjaSequence";}


	const 	NSEntityInstance*  GetActiveInstance() const  {	return m_pobActiveInstance;		}
	void 	SetActiveInstance( CEntity* pobNSEntity );
	const 	NSEntityInstance* HasInstance( CEntity* pobNSEntity ) const;
	void 	AddInstance( NSEntityInstance* pobNSEntity );

	uint32_t				MappedAreaInfo() const 				{ 	return m_uiMappedAreaInfo; 	}
	const ntstd::String		GetNSName( void ) const 	{	return  m_obNSName; 		}

    float				GetCurrentTutorialScalar( void )const	{	return m_fTutorialScalarCurr; }

	void                SetView(int iView) {m_iCamView = iView;}

private:

	// For loading and cleaning up
	void		BuildPackage( void );
	void		CleanPackage( void );
	
	//	part of loading and constructing
	void 		BuildEnts();

	// Internal helper functions
	void		ResetClipTimers(void ) { m_fClipTime = 0.0f;}
	void		UpdateTime( float fDt ) { m_fClipTime += fDt; m_fTotalTime += fDt; }
	void 		UpdateTutorialScalars( float fDT );

	bool		ClipEnd( void ) const { return( m_fClipTime >= m_pClip->m_fDuration ); }
	bool		PlayClip( float fDt );
	void		StartClip( void );
	CEntity*	CreateNSEntity( NSEnt* pEnt );
	void		SetNSEntityForClip( NSEnt* pEnt );
	void		ShowEntity( CEntity* pEntity, bool bShow );
	void		ShowTransform( CEntity* pEntity, const char* pcTransformName, bool bShow );
	void		DisplayPadInput( CEntity* pEntity, NSConditionStorage* pStore, float fDt );
	void		RestoreEntities( void );
	void		SetUprightCharacterRotation( Character* pCharacter );
	int			GetNumberOfClips( void ) const { return m_pNSContainer->m_ClipList.size(); }
	void		RemoveNSAnimContainers( void );
	bool		InWindow( void ) { return( (m_fClipTime >= m_pClip->m_fWindowStartTime) && (m_fClipTime < m_pClip->m_fWindowEndTime) ); }
	bool		PostWindow( void ) { return( m_fClipTime >= m_pClip->m_fWindowEndTime ); }
	void		TestConditions( float fDt );

static 	const char* ParseNSEntName(const char* pcEvent, char* acWorkString, int iWorkspaceLen);
		void		ParseEvents( void );

	bool		ContainsMajorEntities( void );
	void		ModifyCamera( void );
	void		DisplayTickOrCross( float fDt );
	void		PreprocessConditions( void );
	NSClip*		FindClipObject( const char* pcName );
	void		ParseKeyString( NSConditionStorage* pStore, const char* pcCommand );
	float 		GetHitRateMeasurePassDuration( void ) const;
	void		PostProcessConditions( void );
	void		ProcessCorrectConditions( void );
	void		LinkOverlays( void );
//	void		CheckCameraInSync( void );
	void		StorePendingCamera(CoolCam_MayaAnimator* pCameraAnimator, const char* pcCameraAnimName);

	void		UpdateCommandFeedback( float fDt );
	void 		UpdateDisplay( float fDt );

	void		PlayFeedbackSound( bool bCorrect );
	CEntity*	GetGameEntityPtr( const NSEnt* pEnt ) { return( CEntityManager::Get().FindEntity( pEnt->m_sName.c_str() ) ); }
	NSEnt*		FindNSEntFromName( const char* pcName );

	void		RemoveNSBlendShapesComponents( void );
	void		RemoveNSBSAnimContainers( void );
	void		RemoveNSBSClumps( void );
	void		RemoveAllNSBlendShapesRelatedStuff( void );

	bool		TestIfEndClip( const char* pcClipName ) const { return( strstr( pcClipName, m_pcEndClipString ) ); }
	bool		TestIfDieClip( const char* pcClipName ) const { return( strstr( pcClipName, m_pcDieClipString ) ); }
	bool		TestIfFailureClip( const char* pcClipName ) const { return( strstr( pcClipName, m_pcFailureClipString ) ); }

	bool		SetupTransingOutCam( int iCurrCoolCamID );

	ntstd::Vector< NSEnt* > m_EntityList;			// entities created during package
	ntstd::Vector< NSConditionStorage* > m_PCList;	// stores preprocessed conditions for a clip
//	ntstd::Vector< ButtonStorage* > m_ButtonList;	// buttons to show effects (rushed - will sort out properly after the big visit)

	ntstd::Vector< NSEntityInstance* >	m_obInstanceList;
	const NSEntityInstance*				m_pobActiveInstance;
	uint32_t							m_uiMappedAreaInfo;
	ntstd::String	 					m_obNSName;
	float								m_fTutorialScalarCurr;

	// CEntity*			m_pNSEntity;					// mapped Ninja sequence entity

	bool			m_bResident;					// true if resource are all resident in memory
	NS_STAGE		m_eStage;
	bool			m_bClipStarted;					// true if clip anims have been started
	float			m_fClipTime;					// current time in clip

	float 			m_fTimeSinceLastSignificantMotion; // scee.sbashow - to know when last motion was made of significance - to prevent motion overlap due to player lag.
	bool 			m_bAllowingForMotionGrace;		
	
	bool			m_bTestInputOverLayBeingRequested;		// scee.sbashow:flags whether on screen command for testing input is being display-requested, or not.
	bool			m_bMeasureAverageNumberPressesMet;		// scee.sbashow:flags whether the number of presses during the testing regime for mashing/shaking is sufficient
															// scee.sbashow:This is only in the 'simple average sense' - the additional conditions for 'spread' consistency 
															//	over the mash/shake test period will be required on top of this.
	float			m_fProximityToFrequencyPassing;			// scee.sbashow:notes how close the more accurate measure of mash is to passing (1.0f==pass)

	u_int			m_uiPadSpriteID;				// effect ID for pad input sprite
	NinjaSequence*	m_pNSContainer;					// container class for NS
	NSClip*			m_pClip;						// current clip ptr
	NSClip*			m_pTargetClip;					// set when transferring to a new clip
	bool			m_bFirstClip;					// true if we need to play the first clip
	bool			m_bInputTriggered;				// true if input has been pressed during this clip
	float			m_fTickTimer;					// timer for displaying tick
	bool			m_bTick;						// whether to display a tick or a cross
	bool			m_bPlayerKilled;				// flags if player's been killed during clip
	const char*		m_pcTargetClipName;				// name of target clip
	bool			m_bPlayerDisabled;				// true if player has been put into a ninja sequence state
	bool			m_bPostProcessedConditions;		// flags if conditions have been processed after the clip window (for frequency checks)
	float			m_fTotalTime;					// total current duration of sequence
	bool			m_bFailed;						// true if failed ninja sequence
	CoolCam_MayaAnimator*	m_pCameraAnimator;				// the cool animator for the camera
	const char*		m_pcCameraAnimName;				// anim name of pending camera
	const char*		m_pcClipName;					// name of current clip
	float			m_fTickPeriod;					// incrementing time counter of how long 'tick' has been displayed

	int				m_iLastModifiedCoolCamID;		// scee.sbashow :  note the cool cam's ID last used - for transitioning out
	int				m_iTransingOutLastCoolCamID;	// notes the coolcam id to last be used for transitioning out of.
	int     	    m_iCamView;						// Camera view to play NS Camera on

													//					The NSPackage will just control the hud requests.

//	NSIndicatorEffects		m_indicatorEffects;		// handle indicator effects

	// cache these, just incase we need finer control over 
	// the effects rather than simple on / off events
	GhostGirlController*	m_ggWhite;	// Ghost girl White
	GhostGirlController*	m_ggBlue;	// Ghost girl Blue
	GhostGirlController*	m_ggRed;	// Ghost girl Red

	bool					m_bHasInteractiveClip;	// scee.sbashow: flags whether curr clip is interactive (ie has input tests)

};


//------------------------------------------------------------------------------------------
//!
//!	NSManager
//!	Handles Ninja Sequences and Cutscenes (single clip ninja sequences)
//!
//------------------------------------------------------------------------------------------

class NSManager : public Singleton<NSManager>
{
public:
	static const char* m_aConditionStrings[];
	static const char* m_aInputStrings[];
	static const char* m_aEventStrings[];

	NSManager();
	~NSManager();

	void				DebugRender( void );
	void				Update( float fDt );
	void				Reset( void );
	bool				IsRestarting( void ) const { return m_bRestart; }

	void				AddNSP( NSPackage* pNSPackage,  NSEntityInstance* pNSEntity  );
	const char*			GetConditionString( int iCommand ) const { return m_aConditionStrings[ iCommand ]; }
	const char*			GetControlInputStrings( int iInput ) const { return m_aInputStrings[ iInput ]; }
	const char*			GetEventString( int iEvent ) const { return m_aEventStrings[ iEvent ]; }
	bool				IsNinjaSequencePlaying( void ) const { return( !( m_NSPList.empty() ) ); } 
	const NSButtonSpec*	GetButtonSpec( void ) const { return( m_pNSButtonSpec ); } 
	void				RemoveEntityFromAllSequences( CEntity* pEntity );
	bool				IsSkippingSequence() const { return m_bSkippingSequence; }

	ntstd::String		GetPlaybackClipName() const { return m_playbackClipName; };
	void				SetPlaybackClipName( const char* playbackClipName) { m_playbackClipName = ntstd::String(playbackClipName); };

	
	
	// scee.sbashow: render icon definitions, and resources, managers, etc.
	struct	HudRenderDefinition
	{
		HudRenderDefinition()
		{
			m_bIsValid = false;
		}

		enum TEXTURE_SET { TS_RIGHT = 0, TS_WRONG = 1 };
		int  m_iNumTexturesPerCorrectInput;
		// its definition
		bool							m_bIsValid;
		HudImageRenderDef 				m_obRenderDef;
		void	Set(HudImageRenderDef& obDef) {	m_obRenderDef = obDef; m_bIsValid = false;	}
		void CacheDefTextures();
	};

    const HudRenderDefinition& GetPadIconDefinition( )  const												{	return m_obRenderForTutorialPad;				}
    const HudRenderDefinition& GetTutorialIconDefinition( NSCommand::CONTROLLER_INPUT_TYPE eType )  const	{	return m_aobRenderSetForTutorialHelpers[eType];	}
    const HudRenderDefinition& GetCommandIconDefinition( NSCommand::CONTROLLER_INPUT_TYPE eType )   const	{	return m_aobRenderSetForInputs[eType];			}

	NSCommand::RenderSet*		RequestCommandRenderSet( void )
	{
		ntAssert(m_iRenderRequest>=0);
		m_iRenderRequest%=m_iMaxRenderInstances;

		ntAssert(m_aobRenderSet[m_iRenderRequest].m_obRenderInstance==0);
		ntAssert(m_aobRenderSet[m_iRenderRequest].m_obRenderHelperPad==0);
		ntAssert(m_aobRenderSet[m_iRenderRequest].m_obRenderHelperArrowMotion==0);
		return &m_aobRenderSet[m_iRenderRequest++];
	}

private:


	ntstd::Vector< NSPackage* > m_NSPList;
	NSButtonSpec*				m_pNSButtonSpec;
	bool						m_bRestart;				// set if level restart
	bool						m_bSkippingSequence;	// set if skipping sequence

	ntstd::String				m_playbackClipName;


	int									m_iRenderRequest;
	static const	int 				m_iMaxRenderInstances = 16;
	NSCommand::RenderSet				m_aobRenderSet[m_iMaxRenderInstances];

	HudRenderDefinition			 	m_aobRenderSetForInputs[NSCommand::NS_LASTINPUT_TYPE];
	HudRenderDefinition				m_aobRenderSetForTutorialHelpers[NSCommand::NS_LASTINPUT_TYPE];
	HudRenderDefinition				m_obRenderForTutorialPad;

	void BuildInputIconDef( int iID, 
							const CKeyString& obCorrect,
							const CKeyString& obIncorrect,
							const CVector& obCol,
							const CVector& obPosScaleInfo);

};


#endif // _NSMANAGER_H
