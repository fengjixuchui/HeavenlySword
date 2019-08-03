#if !defined( GAME_ENTITYBROWSER_H)
#define GAME_ENTITYBROWSER_H

#include "game/commandresult.h"

/***************************************************************************************************
*
*	CLASS			CEntityBrowser
*
*	DESCRIPTION		In-game debugging tool for entities.
*
***************************************************************************************************/

class CEntityBrowser : public Singleton<CEntityBrowser>
{
public:

	// Construction
	CEntityBrowser( void );

	void DebugRender ();

	// Retrieve the current entity which we are browsing
	const CEntity* GetCurrentEntity( void ) const { return m_pobCurrentEntity; }
	void SetCurrentEntity( CEntity* const pobNewEntity ) { m_pobCurrentEntity = pobNewEntity; }

	// Functions for keyboard/script binding
	COMMAND_RESULT DumpSuperLog();
	COMMAND_RESULT AdvanceLuaDebugState();
	COMMAND_RESULT AdvanceLockonMode();
	COMMAND_RESULT ToggleDebugAnimEvents();
	COMMAND_RESULT RebuildAnimEventLists();
	COMMAND_RESULT ToggleSkinningDebug();
	COMMAND_RESULT SelectPreviousEntity();
	COMMAND_RESULT SelectNextEntity();
	COMMAND_RESULT SelectPreviousAnim();
	COMMAND_RESULT SelectNextAnim();
	COMMAND_RESULT ToggleMovementController();
	COMMAND_RESULT StartSelectedAnim();
	COMMAND_RESULT StartSelectedAnimWithAttackDuration();

	// Toggle the entity browser on and off
	COMMAND_RESULT ToggleEntityBrowser(void);

	bool m_bEnabled;
	bool m_bShowNames;
	bool m_bDisplayAnimOnly;
	bool m_bDebugAxis;
	bool m_bRenderTransforms; // Transform rendering
	bool m_bRenderUsePoints; // Use-point rendering
	bool m_bRenderCollision; // Collision information
	bool m_bHelp; // Information about keys for entity browser
	bool m_bMaterialRayCast;

protected:

	bool m_bMoveInterface;
	float m_fInterfaceX;
	float m_fInterfaceY;

	CEntity* m_pobPlayer;
	CEntity* m_pobCurrentEntity;
	AnimShortCut* m_pobCurrentTemplateAnim;

	int m_iCurrentAnim;
	int m_iCurrentEntity;
	int m_iLuaStateDebugMode; // LUA State Display
	bool m_bGridInit;
	bool m_bGridEnabled;
	CMatrix m_obGridMatrix;
	float m_fLockonTimer;
	int m_iLockonMode;
	CEntity* m_pobLockonEntity;
	bool m_bCircleEnabled; // Player orientation circle
	bool m_bDebugAnimevents; // Show anim events playing on an entity

	void UpdateBrowser ();
	void UpdateAxis ();
	void UpdateLuaStateDisplay ();
	void RenderLockon ();
	void RenderHelp ();
	void DisplayNames();
	void DisplaySpeeds();
};

#endif //GAME_ENTITYBROWSER_H // end game_entitybrowser_h
