//------------------------------------------------------------------------------------------
//!
//!	\file attackdebugger.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_ATTACKDEBUGGER_H
#define	_ATTACKDEBUGGER_H

// Necessary includes
#include "editable/enumlist.h"
#include "input/mouse.h"
#include "effect/screensprite.h"
#include "hitcounter.h"
#include "game/commandresult.h"

// Forward declarations
class CAttackComponent;
class CAttackLink;

//------------------------------------------------------------------------------------------
//!
//!	AttackDebugger
//!	Sorts out all of the combat debugging functionality in a non invasive way.
//!
//------------------------------------------------------------------------------------------
class AttackDebugger : public Singleton< AttackDebugger >
{
public:

//	typedef ntstd::List< const ATTACK_MOVE_TYPE >	moveList;
	typedef ntstd::List< ATTACK_MOVE_TYPE >	moveList;
	typedef ntstd::List< moveList* >		attackList;

	// Construction destruction
	AttackDebugger( void );
	~AttackDebugger( void );

	// This updates the state of stuff
	void Update( float fTimeChange );

	// This cleans up the current data
	void Reset( void );

	// Attack Component can register themselves here
	void RegisterComponent( const CAttackComponent* pobComponent );
	void UnregisterComponent( const CAttackComponent* pobComponent );

	// Attack component manipulation
	// For keyboard/script invocation
	COMMAND_RESULT ToggleAutoCounter();
	COMMAND_RESULT ToggleNinaSimulator();
	COMMAND_RESULT ToggleGridDisplay();
	COMMAND_RESULT AttackComponentOneBackward();
	COMMAND_RESULT AttackComponentOneForward();
	COMMAND_RESULT AttackComponentTwoBackward();
	COMMAND_RESULT AttackComponentTwoForward();
	COMMAND_RESULT IncrementStyleLevels();
	COMMAND_RESULT DecrementStyleLevels();
	COMMAND_RESULT ToggleSuperSafeVolumeDisplay();

	// Stuff we can do
	bool m_bRenderProtractors;
	bool m_bRenderPlayerOneGrid;
	bool m_bResetPlayerOneGrid;

	// Per Player details
	bool m_bShowCurrentAttackState;
	bool m_bShowCurrentRecoveryState;
	bool m_bShowCurrentAttackClass;
	bool m_bShowCurrentStance;
	bool m_bShowAttackWindows;
	bool m_bShowHealth;
	bool m_bShowIncapacityTime;
	bool m_bShowStyleLevel;
	bool m_bShowAttackMovementType;
	bool m_bRenderBigHeads;
	bool m_bButtonBasherTest;
	bool m_bRenderTargeting;
	bool m_bRenderRoots;
	bool m_bShowStrikeProximityCheckAngle;	
	bool m_bShowEvade;
	bool m_bAutoCounter;
	bool m_bRenderSuperSafeVolumes;

	// If we are in battle mode we show battle type details
	bool m_bBattleMode;

private:

	// Helper functions will be going here
	void SetDebugComponent( MOUSE_BUTTON eButton, CAttackComponent const*& pobDebugComponent );
	bool IsRegistered( const CAttackComponent* pobAttackComponent );
	bool StringIsRelevantToStance( const CAttackComponent* pobAttackComponent, ATTACK_MOVE_TYPE eAttackType );

	// For dealing with our lists of possible attacks
	void ClearAttackLists( void );
	void FillAttackLists( const CAttackLink* pobAttackLink );
	void AddFurtherAttacks( const CAttackLink* pobAttackLink, moveList* pobFirstString );

	// Rendering bits and bobs
	void RenderProtractors( void );
	void RenderProtractor( const CAttackComponent* pobAttackComponent );
	void RenderGrid( void );
	void RenderPlayerDetails( const CAttackComponent* pobAttackComponent, float fXOffset, float fYOffset, float fYSpacing );
	void RenderHealthBars( void );
	void RenderBigHeads( void );
	void RenderTargeting( const CAttackComponent* pobAttackComponent );
	void RenderRoot( const CAttackComponent* pobAttackComponent );
	void RenderTargetSegment( const CMatrix& obLocalMatrix, float fBottom, float fTop, float fInnerRadius, float fOuterRadius, float fSweep, uint32_t dwColour );

	// The components that we can debug
	ntstd::List< const CAttackComponent* > m_pobAttackComponents;

	// Our two debug components
	const CAttackComponent* m_pobDebugComponentOne;
	const CAttackComponent* m_pobDebugComponentTwo;

	// Stuff we need to save
	CMatrix m_obPlayerOneGridMatrix;

	// For interogating possible strings of attacks
	attackList	m_obAttackList;
};

#endif // _ATTACKDEBUGGER_H
