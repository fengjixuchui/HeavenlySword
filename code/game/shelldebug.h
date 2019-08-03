//------------------------------------------------------------------------------------------
//!
//!	\file shelldebug.h
//! Defines shell debug functionality only
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLDEBUG_H
#define GAME_SHELLDEBUG_H

#ifndef _COMMAND_RESULT_H
#include "commandresult.h"
#endif

#ifndef GAME_SHELLCONFIG_H
#include "game/shellconfig.h"
#endif

#ifndef _GOLD_MASTER

//------------------------------------------------------------------------------------------
//!
//!	ShellDebug
//! Wrapper for debug shell functionality
//!
//------------------------------------------------------------------------------------------
class ShellDebug
{
public:
	ShellDebug();

	friend class ShellMain;
	friend class ShellGlobal;
	friend class CreateLevelUpdater;

	// --- Commands ---
	COMMAND_RESULT CommandToggleBoolMember(bool *pobBool);
	COMMAND_RESULT CommandIncrementInt(int* pInt);
	COMMAND_RESULT CommandDecrementInt(int* pInt);
	COMMAND_RESULT CommandSetGameTimeMultiplier(const float& fMultiplier);
	COMMAND_RESULT CommandAdvanceContext(const int& iVal);
	COMMAND_RESULT CommandDebugQuitGame();
	COMMAND_RESULT ToggleCodePause();
	COMMAND_RESULT ToggleUserPause();
	COMMAND_RESULT CommandSwitchDebugCameraMode();
	COMMAND_RESULT CommandToggleDebugRender();
	COMMAND_RESULT CommandDumpGatso();
	COMMAND_RESULT JumpToLevel( const CHashedString& sLevelName) {return JumpToLevel(sLevelName,0);}
	COMMAND_RESULT JumpToLevel( const CHashedString& sLevelName, int nCheckpointID);
	COMMAND_RESULT CommandCallScriptFunc(const CHashedString& sFunc);
	COMMAND_RESULT CommandPauseUnpause();

	// These commands are for remote particle system control
	COMMAND_RESULT CommandRebuildCurve(const CHashedString& sCurveGuid);
	COMMAND_RESULT CommandKillParticleSystem(const CHashedString& sPSystemGuid);
	COMMAND_RESULT CommandSpawnFromTrigger(const CHashedString& sTriggeruid);

	// These commands are for remote anim event control
	COMMAND_RESULT Command_AnimEvent_SetEntity (const CHashedString& obEntityName);
	COMMAND_RESULT Command_AnimEvent_SetAnimation (const CHashedString& obAnimationName);
	COMMAND_RESULT Command_AnimEvent_GetAnimationDuration (const CHashedString& obAnimationLength);
	COMMAND_RESULT Command_AnimEvent_SetAnimationTime (const float& fTime);
	COMMAND_RESULT Command_AnimEvent_SetAnimationSpeed (const float& fSpeed);
	COMMAND_RESULT Command_AnimEvent_SetAnimationLoop (const bool& bLoop);
	COMMAND_RESULT Command_AnimEvent_DebugDisable (const CHashedString& obAnimEventName);
	COMMAND_RESULT Command_AnimEvent_GenerateStrike (const CHashedString& obStrikeName);
	COMMAND_RESULT Command_AnimEvent_SetAttackSimType (const int& iAttackSimType);
	COMMAND_RESULT Command_AnimEvent_RestoreControl ();

	// These command are for remote control of AudioMixerProfiles and AudioMixerTargets
	COMMAND_RESULT Command_AudioMixer_SetProfile (const CHashedString& obProfileName);
	COMMAND_RESULT Command_AudioMixer_SetProfileChangesEnabled (const bool& bEnabled);
	COMMAND_RESULT Command_AudioMixer_SetCategoryToManipulate (const CHashedString& obCategoryName);
	COMMAND_RESULT Command_AudioMixer_SetCategoryMute (const bool& bMute);
	COMMAND_RESULT Command_AudioMixer_SetCategoryPause (const bool& bPause);

	// These commands are for remote NinjaSequence control
	COMMAND_RESULT CommandSetNinjaSequencePlaybackClip (const CHashedString& obClipName);
	COMMAND_RESULT CommandTriggerNinjaSequence (const CHashedString& obEntityName);

	// Command for capturing cube-map image for a given axis
	COMMAND_RESULT Command_CaptureCubeMapAxis (const CHashedString& obAxisName);

	// This is used to toggle the rendering of the safe debug lines in the game.
	COMMAND_RESULT CommandToggleSafeDebugLines();

	//! [0] = Major, [1] = Minor
	uint32_t m_AppVersionNumber[2];

	//! last loaded level number
	uint32_t m_LevelVersionNumber;

	bool	AllowSingleStep() const { return m_bSingleStep; }

private:
	void	SetupLogging();
	void	ReleaseLogs();

	void	RegisterDebugKeys();
	void	RegisterDebugKeysForLevel();
	void    ForceMaterials( RENDER_QUALITY_MODE eRenderMode );

	void	UpdateGlobal();
	void	UpdateLevel();
	void	Render();

	//! file object for the user specific log 
	LogFile* m_pUserLogFile;

	//! resource logs
	LogFile* m_pTextureLogFile;
	LogFile* m_pClumpLogFile;
	LogFile* m_pAnimLogFile;
	LogFile* m_pResourceLogFile;
	LogFile* m_pAreaLoadLogFile;

	//! game logs
	LogFile* m_pGfxLogFile;
	LogFile* m_pGameLogFile;	
	LogFile* m_pLuaLogFile;
	LogFile* m_pExecLogFile;
	LogFile* m_pCoreLogFile;
	LogFile* m_pAILogFile;
	LogFile* m_pAICBLogFile;
	LogFile* m_pAIBehaveLogFile;

	// Enum for anim event attack commands
	enum AnimEventAttackSimType
	{
		FORCE_ATTACK = 0,
		BUTTON_SIM_COMBO = 1
	};

	// These strings are used with the anim event commands
	CHashedString m_obAnimEventTargetEntity;
	CHashedString m_obAnimEventTargetAnim;
	AnimEventAttackSimType m_eAnimEventAttackSimType;

	// This string is used with the audio mixer commands
	CHashedString m_ob_AudioMixerCategoryToManipulate;

	//! debug render flags
	bool	m_bSingleStep;

	//! debug render flags
	bool	m_bDebugRenderShell;
	bool	m_bRenderDebugSafeAreaLines;
};

#else // _GOLD_MASTER

class ShellDebug
{
private:
	friend class ShellMain;
	friend class ShellGlobal;
	friend class CreateLevelUpdater;

	void	UpdateGlobal() {};
	void	UpdateLevel() {};
	void	Render() {};

	void	SetupLogging() {}
	void	ReleaseLogs() {}
	void	RegisterDebugKeys() {}
	void	RegisterDebugKeysForLevel() {}
};

#endif

//------------------------------------------------------------------------------------------
//!
//!	Global floats for debug rendering
//!
//------------------------------------------------------------------------------------------
static const float sfDebugLineSpacing = 12.0f;
static const float sfDebugLeftBorder = 20.0f;
static const float sfDebugTopBorder = 20.0f;
static const float sfContextTopBorder = sfDebugTopBorder + sfDebugLineSpacing*5.0f;
static const float sfOSDTopBorder = sfDebugTopBorder + sfDebugLineSpacing*30.0f;
static const float sfErrorTopBorder = sfDebugTopBorder + sfDebugLineSpacing*50.0f;

#endif // GAME_SHELLDEBUG_H
