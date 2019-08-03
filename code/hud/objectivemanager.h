#ifndef _OBJECTIVEMANAGER_H
#define _OBJECTIVEMANAGER_H

// Necessary includes
#include "hud/hudunit.h"
#include "hud/hudtext.h"
#include "hud/hudtimer.h"
#include "hud/hudimage.h"

#include "effect/screensprite.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "effect/renderstate_block.h"
#include "editable/enumlist.h"

// FIX ME need to use proper text not debug text
#include "core/visualdebugger.h"

// Forward declarations

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerDef
//!	Defines for ObjectiveManager
//!
//------------------------------------------------------------------------------------------
class ObjectiveManagerDef
{
	HAS_INTERFACE( ObjectiveManagerDef );
public:
	ObjectiveManagerDef();
	~ObjectiveManagerDef();

	float m_fMessageBoxX;
	float m_fMessageBoxY;
	float m_fMessageBoxMinWidth;
	float m_fMessageBoxMinHeight;
	float m_fMessageBoxMaxWidth;
	float m_fMessageBoxMaxHeight;
};

//------------------------------------------------------------------------------------------
//!
//!	Objective
//!	Wrapper class for ObjectiveManager to use
//!	FIX ME - This could be virtualised very well rather than internaly typed
//!
//------------------------------------------------------------------------------------------
class Objective
{
public:
	Objective(ntstd::String obText, int iPriority, int iID);
	~Objective( void );

	enum OBJECTIVE_TYPE
	{
		OT_SIMPLE,
		OT_TIMER,
		OT_STATUS,
		OT_IMAGE
	};


	enum TIMER_OBJECTIVE_STATE
	{
		TOS_INITIAL,
		TOS_FIRST_WARNING,
		TOS_SECOND_WARNING,
		TOS_OVER
	};

	int GetID() { return m_iID; }
	bool IsDone() { return m_bDone; }
	bool IsNew() { return m_bNew; }
	const char* GetText() { return ntStr::GetString(m_obObjectiveText); }
	bool Initialise (ntstd::String obFont);
	bool InitialiseTimer (TimerRenderDef* pobTimerRenderDef);
	bool InitialiseImage ( CHashedString obImageDefName);

	void Passed( void );
	void Failed( void );
private:
	ntstd::String m_obObjectiveText;
	bool m_bDone;
	bool m_bNew;
	int m_iPriority;
	int m_iID;
	float m_fTimer;
	float m_fRenderTime;
	CEntity* m_pobCallbackEnt;

	HudTextRenderer* m_pobTextRenderer;
	HudTextRenderDef m_obTextRenderDef;
	OBJECTIVE_TYPE m_eType;
	
	TIMER_OBJECTIVE_STATE m_eTimerState;
	TimerRenderer* m_pobTimerRenderer;
	HudImageRenderer* m_pobImageRenderer;
	
	friend class ObjectiveManager;
	friend class ObjectiveManagerRenderer;
};

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManager
//!	Manages (at the moment) just some strings for HUD display from script
//!
//------------------------------------------------------------------------------------------
class ObjectiveManager
{
public:
	ObjectiveManager();
	~ObjectiveManager();
	
	int AddObjective(const char* pcText, int iPriority = 0);
	int AddTimedObjective(const char* pcText, float fTime, CEntity* pobCallbackEnt = 0, int iPriority = 0);
	int AddStatusObjective(const char* pcText, int iPriority = 0);
	int AddImageObjective(const char* pcText, int iPriority = 0);
	void RemoveObjective( int iObjectiveID);
	void FailedObjective( int iObjectiveID);
	void PassedObjective( int iObjectiveID);
	Objective* GetObjective(int iID);

	void RegisterDef ( ObjectiveManagerDef* pobDef );

private:
	ntstd::List<Objective*> m_obObjectiveList;

	int m_iObjectivesSoFar;

	ObjectiveManagerDef* m_pobDef;

	bool m_bNSActive;	// Keep track of NS so we don't remove NS tutorial message boxes

	friend class ObjectiveManagerRenderer;
};


//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerRenderDef
//!	Defines times and sizes for ObjectiveManagerRenderer to use
//!
//------------------------------------------------------------------------------------------
class ObjectiveManagerRenderDef : public CHudUnitDef
{
	HAS_INTERFACE( ObjectiveManagerRenderDef );

	virtual CHudUnit* CreateInstance( void ) const;

public:
	ObjectiveManagerRenderDef();
	~ObjectiveManagerRenderDef();

	void PostConstruct( void );

	ntstd::String m_obFont;
	
	float m_fTimeToRenderNew;
	float m_fTimeToRenderWhenDone;
	float m_fTopLeftX;
	float m_fTopLeftY;
	float m_fTimerTopLeftX;
	float m_fTimerTopLeftY;
	float m_fStatusTopLeftX;
	float m_fStatusTopLeftY;
	float m_fTimerLineSpace;

	float m_fOverallScale;

	float m_fFadeSpeed;	

	TimerRenderDef* m_pobTimerRenderDef;

	float m_fTimerBackgroundTopLeftX;
	float m_fTimerBackgroundTopLeftY;

	float m_fTimerBackgroundWidth;
	float m_fTimerBackgroundHeight;

	float m_fTimerBackgroundGlowHeight;
	float m_fTimerBackgroundGlowWidth;

	CKeyString m_obBackgroundImage;
	CKeyString m_obBackgroundGlow;

	ntstd::String m_obLabelTextID;
	ntstd::String m_obLabelFont;

	float m_fTextPosX;
	float m_fTextPosY;

	float m_fTimeForGlow;
	float m_fAlphaForGlowHigh;
	float m_fAlphaForGlowLow;
	float m_fTimeForSecondGlow;
	float m_fAlphaForSecondGlowHigh;
	float m_fAlphaForSecondGlowLow;

	float m_fScalarForGlowHigh;
	float m_fScalarForGlowLow;

	float m_fGlowDelta;

	EFFECT_BLENDMODE m_eBlendMode;
};

//------------------------------------------------------------------------------------------
//!
//!	ObjectiveManagerRenderer
//!	Renders the contents of an ObjectiveManager
//!
//------------------------------------------------------------------------------------------
class ObjectiveManagerRenderer : public CHudUnit
{
public:
	ObjectiveManagerRenderer(ObjectiveManagerRenderDef* pobDef)
	:	m_pobRenderDef( pobDef )
	,	m_fTimeRenderingNew( 0.0f )
	,	m_pobTextRenderer ( 0 )
	{};
	~ObjectiveManagerRenderer();

	virtual bool Render( void );
	virtual bool Update( float fTimeChange );
	virtual bool Initialise( void );

private:
	ObjectiveManagerRenderDef* m_pobRenderDef;
	ObjectiveManager* m_pobObjectiveManager;

	void TimerBackgroundRender ( Objective* pobObjective, float fTimeStep );

	float m_fScale;
	float m_fTimeRenderingNew;
	float m_fTimeChange;

	float m_fGlowAlpha;
	float m_fGlowScalar;

	ScreenSprite m_obTimerBackground;
	ScreenSprite m_obTimerGlow;

	HudTextRenderer* m_pobTextRenderer;
	HudTextRenderDef m_obTextRenderDef;
};

#endif // _OBJECTIVEMANAGER_H
