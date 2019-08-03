/***************************************************************************************************
*
*	DESCRIPTION		The main manager of the flow of the game
*
*	NOTES			Looks after front end screens and the management of levels
*
***************************************************************************************************/

#ifndef	_HUDMANAGER_H
#define	_HUDMANAGER_H

#include "hud/hudunit.h"
#include "hud/hudtext.h"
#include "hud/hudoverlay.h"
#include "editable/enumlist.h"
#include "effect/screensprite.h"
#include "game/attackdebugger.h"
#include "game/attacks.h"
#include "core/visualdebugger.h"

class HitCounter;
class HitCounterRenderDef;
class HitCounterRenderer;

class HealthBarRenderDef;
class HealthBarRenderer;

class ButtonHinter;
class ButtonHinterDef;
class ButtonHinterRenderer;
class ButtonHinterRenderDef;

class ObjectiveManager;
class ObjectiveManagerRenderDef;
class ObjectiveManagerRenderer;
class ObjectiveManagerDef;

class LifeClock;
class LifeClockRenderDef;
class LifeClockRenderer;

class CMessageBoxDef;
class CMessageBox;
class CMessageBoxRenderDef;
class CMessageBoxRenderer;

class MessageDataManager;
class HudSoundManager;

class FailureHud;

class BodyCount;

//------------------------------------------------------------------------------------------
//!
//!	CombatHUDElements
//!	Wrapper struct for all the things our HUD is gonna render
//!
//------------------------------------------------------------------------------------------
struct CombatHUDElements
{
	CombatHUDElements() 
	: 	m_pobButtonHinter( 0 )
	,	m_pobObjectiveManager( 0 )
	,	m_pobFailureHud( 0 )
	,	m_pobBodyCount ( 0 )
	{};

	ButtonHinter*		m_pobButtonHinter;
	ObjectiveManager*	m_pobObjectiveManager;
	FailureHud*			m_pobFailureHud;
	BodyCount*			m_pobBodyCount;
};

// Misc parameters for the hud, parameters for which it is unclear where they ought to belong
class HUDParams
{
	HAS_INTERFACE( HUDParams );

	HUDParams()
	:	m_pobObjectiveManDef ( 0 )
	{};

public:
	ntstd::String m_obNewCheckpointFont;
	float m_fNewCheckpointPosX;
	float m_fNewCheckpointPosY;

	ObjectiveManagerDef* m_pobObjectiveManDef;
};

class CHudConfig
{
	HAS_INTERFACE( CHudConfig );

public:
	CHudConfig ()
	{};

	void PostConstruct( void );

	ntstd::List<CHudUnitDef*> m_aobHudRenderDefList;

	CMessageBoxRenderDef*	m_pobMessageBoxRenderDef;
	CMessageBoxDef*			m_pobMessageBoxDef;

	HUDParams*				m_pobHudParams;
};

class CHud : public Singleton<CHud>
{
public:

	CHud ();
	~CHud ();

	void Reset();

	void SetConfig (CHudConfig* pobHudConfig);
	void Initialise ( void );
	void Render(float fTimeChange);

	CombatHUDElements* GetCombatHUDElements() 	{ return m_pobCombatHUDElements; 						}

	ButtonHinter* GetButtonHinter() 			{ if (!m_pobCombatHUDElements) return 0; return m_pobCombatHUDElements->m_pobButtonHinter; 	}
	ObjectiveManager* GetObjectiveManager() 	{ if (!m_pobCombatHUDElements) return 0; return m_pobCombatHUDElements->m_pobObjectiveManager; }
	MessageDataManager* GetMessageDataManager() { return m_pobMessageDataManager; };
	FailureHud*	GetFailureHud()					{ if (!m_pobCombatHUDElements) return 0; return m_pobCombatHUDElements->m_pobFailureHud; 	}
	HudSoundManager* GetHudSoundManager()		{ return m_pobHudSoundManager; 	}
	BodyCount* GetBodycount()					{ if (!m_pobCombatHUDElements) return 0; return m_pobCombatHUDElements->m_pobBodyCount; 	}

	CHudUnit* 	 CreateHudElement( const CHudUnitDef* pobRenderDef );
	bool		 RemoveHudElement( CHudUnit* pobUnit );

	CHudUnit* 	 CreateHudElement( CHashedString pobRenderDefName );
	bool		 RemoveHudElement( CHashedString pobRenderDefName );
	bool		 ExitHudElement( CHashedString pobRenderDefName );

	void CreateMessageBox( ntstd::String obStringId, float fPosX, float fPosY, 
									float fMinWidth = 0.0f, float fMinHeight = 0.0f, 
									float fMaxWidth = 0.0f, float fMaxHeight = 0.0f );

	void CreateMessageBox( ntstd::Vector<ntstd::String>& obStringList, float fPosX, float fPosY, 
									float fMinWidth = 0.0f, float fMinHeight = 0.0f, 
									float fMaxWidth = 0.0f, float fMaxHeight = 0.0f );

	void RemoveMessageBox( void );

	void RemoveMessageBox( float fDelayTime );

	void CreateMessage( ntstd::String obStringId, float fPosX, float fPosY );
	void RemoveMessage( void );
	void RemoveMessage( float fDelayTime );

	void CreateDelayedMessage( ntstd::String obStringId, float fDelayTime);

    COverlayResourceManager<HudImageOverlayIncremental>& 
		IncrementalOverlayManager( void ) {	return m_obIncOverlayResourceManager;	};

	void SetGrabHint(bool bHint) { m_bGrabHint = bHint; };

protected:
	ScreenSprite 				m_obDebugCameraSprite;
    CHudConfig* 				m_pobHudConfig;
    CombatHUDElements* 			m_pobCombatHUDElements;
    CombatEventLog* 			m_pobCombatEventLog;
	bool 						m_bCombatLogRegistered;

	COverlayResourceManager<HudImageOverlayIncremental>
								m_obIncOverlayResourceManager;

	ntstd::List<CHudUnit*>	m_aobHudRenderList;

	CMessageBoxRenderer*		m_pobMessageBoxRenderer;

	MessageDataManager*			m_pobMessageDataManager;

	HudSoundManager*			m_pobHudSoundManager;

	HudTextRenderDef			m_obMessageDef;
	HudTextRenderer*			m_pobMessageRenderer;

	float						m_fMessageBoxRemoveTime;
	float						m_fMessageRemoveTime;

	ntstd::String				m_obDelayedMessageID;
	float						m_fDelayedMessageTime;

	bool m_bGrabHint;

	// Map of def to reneder so that I can manage HUD elements created from the 
	// lua environment.  Which at the moment is just the paralysis prisoner count.
	ntstd::Map<const CHudUnitDef*, CHudUnit*> m_obElementMap;
};

typedef ntstd::List<CHudUnitDef*>::iterator HudDefIter;
typedef ntstd::List<CHudUnit*>::iterator  HudRenderIter;


#endif // _HUDMANAGER_H

