/***************************************************************************************************
*
*	DESCRIPTION		
*	NOTES
*
***************************************************************************************************/

// Includes
#include "guilua.h"
#include "gui/guimanager.h"
#include "game/entitymanager.h"
#include "game/playeroptions.h"
#include "game/gamedata.h"
#include "game/languages.h"
#include "gui/guiflow.h"
#include "gui/guiscreen.h"
#include "game/credits.h"
#include "gui/guitext.h"
#include "gui/guisubtitle.h"
#include "game/checkpointmanager.h"
#include "game/gameinfo.h"
#include "objectdatabase/dataobject.h"

#include "game/renderablecomponent.h"
#include "gfx/meshinstance.h"
#include "game/randmanager.h"
#include "hud/hudmanager.h"
#include "hud/failurehud.h"
#include "game/shellmain.h"
#include "game/shelllevel.h"

#define NEWGAME_SKIPFORWARD_PATH_ARGS 4, true, "purgatory", "purgatorymenu", "chapterselect", "newgame" 
#define LATESTCP_SKIPFORWARD_PATH_ARGS 2, true, "chapterselect", "continue" 
#define LOADING_SKIPFORWARD_PATH_ARGS 1, true, "continue"
#define CHAPTERSELECT_SKIPFORWARD_PATH_ARGS 2, true, "purgatorymenu", "chapterselect" 

#ifdef PLATFORM_PS3
	#include <sysutil/sysutil_common.h>
	#include <sysutil/sysutil_sysparam.h>
	#include "game/sysmsgdialog.h"

	static void SaveOptionsDialogCallback( const CellMsgDialogButtonType ButtonPressed );
	static void NewGameProfileOverwriteCallback( const CellMsgDialogButtonType ButtonPressed );
	static void PauseMenuReturnToPurgatoryCallback( const CellMsgDialogButtonType ButtonPressed );
	static void PauseMenuExitGameCallback( const CellMsgDialogButtonType ButtonPressed );
	static void NewGameAreYouSureCallback( const CellMsgDialogButtonType ButtonPressed );
#endif

static void PauseMenuReturnToPurgatoryCallbackYes( void );
static void PauseMenuExitGameCallbackYes( void );

///
//Globals
///
#if !defined(_GOLD_MASTER)
bool g_bUnlockAllGameData = false;
bool g_bClearAllGameData = false;
int g_iChaptersComplete = -1;
int g_iUpToCheckpoint = -1;
bool g_bClearHitFlags = true;
bool g_bSetLastHitFlags = true;
#endif

///
//Statics
///
CGuiLua::ContextStack* CGuiLua::ms_pobContextStack = NULL;
NinjaLua::LuaObject CGuiLua::ms_obGlobalStore;

///
//GUI lua Bindings
///

namespace GUI
{
	int iCallingScreen = 0;	// 0 during startup, 1 in game.
	bool bDoPlayerOptionsSaveLoadBusyCheck = true;
	SubtitleLanguage eSubtitleLanguage = NT_SUBTITLE_NUM_LANGUAGES;
	bool bFirstBoot = false;

	namespace Gameplay
	{
		void NewGameProfileOverwriteCheck( void )
		{
			ntPrintf( "NewGameProfileOverwriteCheck().\n" );

#ifdef PLATFORM_PS3
			// Not sure about best way to detect save data.
			// One method suggested is to detect save data for the first checkpoint.  Presence of this
			// data should signify that the player has data to lose.
			if	( CheckpointManager::Get().GetDataForCheckpoint( 0, 0 ) )
			{
				CSysMsgDialog::CreateDialog( CSysMsgDialog::YESNO_NORMAL_YES, "MAIN_OVERWRITE_PROFILE", NewGameProfileOverwriteCallback );
			}
			else
			{
				// Start game with no save data, are you sure you want to create a new game?
				CSysMsgDialog::CreateDialog( CSysMsgDialog::YESNO_NORMAL_YES, "START_NEW_GAME_NO_SAVE_DATA", NewGameAreYouSureCallback );
			}
#else
			//CGuiManager::Get().MoveOnScreenGroup( "purgatory" );
			CGuiManager::Get().SkipForwardScreen(NEWGAME_SKIPFORWARD_PATH_ARGS);
#endif
		}

		void PauseGameUser(bool bPause)
		{
			// request a user pause of the game
			ShellMain::Get().RequestPauseToggle( ShellMain::PM_USER, bPause );
		}

		void PauseMenuReturnToPurgatory( void )
		{
#ifdef PLATFORM_PS3
			CSysMsgDialog::CreateDialog( CSysMsgDialog::YESNO_NORMAL_YES, "PAUSE_RETURN_DIALOGUE", PauseMenuReturnToPurgatoryCallback );
#else
			// If not PS3, just assume player wants to return to Purgatory.
			PauseMenuReturnToPurgatoryCallbackYes();
#endif
		}

		void PauseMenuExitGame( void )
		{
#ifdef PLATFORM_PS3
			CSysMsgDialog::CreateDialog( CSysMsgDialog::YESNO_NORMAL_YES, "PAUSE_EXITGAME_DIALOGUE", PauseMenuExitGameCallback );
#else
			// If not PS3, just assume player wants to exit the game.
			PauseMenuExitGameCallbackYes();
#endif
		}
		
		bool PauseScreenPurgatoryCheck()
		{
			GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
			ntAssert_p(pobGameInfo, ("GameInfo object not found") );

			//locate the chapter
			const ChapterInfoDef* pobChapterInfo = pobGameInfo->InitialBattleChapter();
			ntAssert_p(pobChapterInfo, ("Failed to find InitialBattle chapter in GameInfo") );

			//lookup the current level
			const ShellLevel* pPlayingLevel = ShellMain::Get().GetCurrRunningLevel();
			return (pPlayingLevel->GetLevelNumber() == pobChapterInfo->ChapterNumber());
		}

		//Various Events. currently only for sound hooks
		void OnEnterFrontend()
		{
			ntPrintf( "gui.Gameplay.OnEnterFrontend().\n" );
			GameAudioManager::Get().OnFrontend();
		}

		void OnPause()
		{
			ntPrintf( "gui.Gameplay.OnPause().\n" );
			GameAudioManager::Get().OnPause(true);
		}

		void OnUnpause()
		{
			ntPrintf( "gui.Gameplay.OnUnpause().\n" );
			GameAudioManager::Get().OnPause(false);
		}

		void OnLevelStart()
		{
			ntPrintf( "gui.Gameplay.OnLevelStart().\n" );
			GameAudioManager::Get().OnLevelStart();
		}

		void OnLevelExit()
		{
			ntPrintf( "gui.Gameplay.OnLevelExit().\n" );
			GameAudioManager::Get().OnLevelExit();
		}
	};

	namespace System
	{
		void UnloadCurrentLevel()
		{
			if (ShellMain::Get().HaveLoadedLevel())
				ShellMain::Get().RequestLevelUnload();
		}

		NinjaLua::LuaObject GetLuaStore()
		{
			return CGuiLua::GetGlobalStore();
		}

		void LoadPurgatoryLevel()
		{
			static const char* pcPurgatoryLevel = "purgatory/purgatory";

			if	(
				( ShellMain::Get().HaveLoadedLevel() ) &&
				( CHashedString( ShellMain::Get().GetCurrRunningLevel()->GetLevelName() ) == CHashedString(pcPurgatoryLevel) )
				)
				return;

			CGuiManager::Get().LoadGameLevel_Name(pcPurgatoryLevel, -1, 0);
		}

		bool InPurgatory()
		{
			static const char* pcPurgatoryLevel = "purgatory/purgatory";

			if	(( ShellMain::Get().HaveLoadedLevel() ) &&
				 ( CHashedString( ShellMain::Get().GetCurrRunningLevel()->GetLevelName() ) == CHashedString(pcPurgatoryLevel) ) )
				return true;
			return false;
		}

		void RestartFromLastCheckpoint()
		{
			if ( CheckpointManager::Exists() )
			{
				CheckpointManager::Get().RestartFromLastCheckpoint();
			}
		}

		ntstd::String GetFailureMessage()
		{
			if ( CHud::Exists() && CHud::Get().GetFailureHud() )
			{
				return CHud::Get().GetFailureHud()->GetFailureStringID();
			}
			else
				return "";
		}

		void LoadFrontendStrings()
		{
			// Update the strings file and font
			CStringManager::Get().LevelChange( "purgatory/purgatory" );
		}

		void LoadSelectedChapter()
		{
			NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();

			NinjaLua::LuaObject obChapter = obStore["selectedChapter"];
			NinjaLua::LuaObject obCheckpoint = obStore["selectedCheckpoint"];

			if (obChapter.IsNumber() && obCheckpoint.IsNumber())
			{
				CGuiManager::Get().LoadGameLevel_Chapter(obChapter.GetInteger(), obCheckpoint.GetInteger());
			}
			else
			{
				ntError_p(!obChapter.IsNumber() && !obCheckpoint.IsNumber(), ("LoadSelectedChapter failed. selectedChapter [int] and selectedCheckpoint [int] need to be set"));
				ntError_p(!obChapter.IsNumber(), ("LoadSelectedChapter failed. selectedChapter [int] needs to be set"));
				ntError_p(!obCheckpoint.IsNumber(), ("LoadSelectedChapter failed. selectedCheckpoint [int] needs to be set"));
			}
		}

		void StartNewGame()
		{
			GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
			ntError_p(pobGameInfo, ("GameInfo object not found") );

			//locate the chapter
			const ChapterInfoDef* pobChapterInfo = pobGameInfo->InitialBattleChapter();
			ntError_p(pobChapterInfo, ("Failed to find InitialBattle chapter in GameInfo") );

			int iChapter = pobChapterInfo->ChapterNumber();
			int iCheckPoint = 0;

			//Save the loadlevel info into lua
			NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();

			obStore.Set("selectedChapter", iChapter);
			obStore.Set("selectedCheckpoint", iCheckPoint);
		}

		bool IsLevelLoading()
		{
			return ShellMain::Get().GetShellState() != ShellMain::SS_RUNNING_LEVEL;
		}

		void GotoLatestCheckpoint()
		{
			int iChapter = 0;
			int iCheckpoint = 0;

			CheckpointManager::Get().GetLastCheckpointNumberReachedByPlayer(iChapter, iCheckpoint);

			//its possible that the returned checkpoint is the 'final' checkpoint. if so, go to the next checkpoint
			GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
			ntAssert_p(pobGameInfo, ("GameInfo object not found") );

			//locate the chapter
			const ChapterInfoDef* pobChapterInfo = pobGameInfo->GetChapter(iChapter);
			ntAssert_p(pobChapterInfo, ("Failed to find chapter %d in GameInfo", iChapter) );

			//and the checkpoint
			const CheckpointInfoDef* pobCPInfo = pobChapterInfo->GetCheckpoint(iCheckpoint);
			ntAssert_p(pobCPInfo, ("Failed to find checkpoint %d for chapter %d in GameInfo", iCheckpoint, iChapter) );

			//check if final. if so then we need to get the next checkpoint
			if (pobCPInfo->Final())
			{
				pobCPInfo = pobCPInfo->NextCheckpoint();
				pobChapterInfo = pobGameInfo->FindChapterContainingCheckpoint(pobCPInfo);

				iChapter = pobChapterInfo->ChapterNumber();
				iCheckpoint = pobCPInfo->CheckpointNumber();
			}

			NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();

			obStore.Set("selectedChapter", iChapter);
			obStore.Set("selectedCheckpoint", iCheckpoint);

			CGuiManager::Get().SkipForwardScreen(LATESTCP_SKIPFORWARD_PATH_ARGS);
		}

		void SkipToChapterSelect()
		{
			CGuiManager::Get().SkipForwardScreen(CHAPTERSELECT_SKIPFORWARD_PATH_ARGS);
		}

		void SkipFromPurgatoryToLoadingScreen()
		{
			CGuiManager::Get().SkipForwardScreen(LOADING_SKIPFORWARD_PATH_ARGS);
		}

		void ExitPurgatory()
		{
			UnloadCurrentLevel();
			CGuiManager::Get().MoveBackScreenGroup( "title" );
		}

		void StandardChapterExit()
		{
			//unload
			UnloadCurrentLevel();

			//check if we want to override the standard destination
			NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();
			NinjaLua::LuaObject obTmp = obStore["standardexitOverride"];
			
			if (!obTmp.IsNil())
			{
				ntPrintf("standardexitOverride: %s\n", obTmp.GetString());
			}

			const char* pcDest = obTmp.IsNil() ? "purgatory" : obTmp.GetString();
			CGuiManager::Get().MoveBackScreenGroup( pcDest );

			obTmp.SetNil();
			obStore.Set("standardexitOverride", obTmp);
		}

		bool CheckpointDataAvailable()
			{
			return CheckpointManager::Get().GetLastCheckpoint() != NULL;
		}
	};

	namespace Options
	{
		void SetAppropriateAudioLanguageForSubtitleLanguage( const SubtitleLanguage eSubtitleLanguage );

		void enableMotionControl(bool bOn)
		{
			ntPrintf("enableMotionControl: %d\n", (int)bOn);

			CPlayerOptions::Get().SetUseMotionControl( bOn );
		}

		void InvertY( bool bInvert )
		{
			CPlayerOptions::Get().SetInvertY( bInvert );
		}

		void setSFXVolume(float fVal)
		{
			//ntPrintf("setSFXVolume: %f\n", fVal);

			CPlayerOptions::Get().SetSFXVol( fVal );
		}

		void setMusicVolume(float fVal)
		{
			//ntPrintf("setMusicVolume: %f\n", fVal);

			CPlayerOptions::Get().SetMusicVol( fVal );
		}

		void setDialogueVolume(float fVal)
		{
			//ntPrintf("setDialogueVolume: %f\n", fVal);

			CPlayerOptions::Get().SetDialogueVol( fVal );
		}

		void ManageDialogueSample( bool bActive )
		{	
			if	( false == bActive )
			{
				CPlayerOptions::Get().StopPlayingDialogueSample();
			}
		}

		void enableSubtitles(bool bOn)
		{
			ntPrintf("enableSubtitles: %d\n", (int)bOn);

			CPlayerOptions::Get().SetShowSubtitles( bOn );
		}

		// Not called during the startup sequence so uses SetAudioLanguage not a live version. 
		void SetAudioLanguage( uint32_t lang )
		{
			ntError_p( lang < NT_AUDIO_NUM_LANGUAGES, ("Invalid audio language selected - reverting to english.") );
			if ( lang >= NT_AUDIO_NUM_LANGUAGES )
			{
				lang = NT_AUDIO_EURO_ENGLISH;
			}

			CPlayerOptions::Get().SetAudioLanguage( (AudioLanguage)lang );
		
			CGuiManager::Get().MoveBackScreen();
		}

		void SetSubtitleLanguage( uint32_t lang )
		{
			ntError_p( lang < NT_SUBTITLE_NUM_LANGUAGES, ("Invalid subtitle language selected - reverting to english.") );
			if ( lang >= NT_SUBTITLE_NUM_LANGUAGES )
			{
				lang = NT_SUBTITLE_EURO_ENGLISH;
			}
		
			if	( 0 == iCallingScreen )
			{
				// Called during the startup sequence

				// Set live options.
				CPlayerOptions::Get().SetLiveSubtitleLanguage( (SubtitleLanguage)lang );

				// Set appropriate audio language for the chosen subtitle language.
				SetAppropriateAudioLanguageForSubtitleLanguage( (SubtitleLanguage)lang );

				// Trigger a save to persists language selection
				CPlayerOptions::Get().Save();
				
				CGuiManager::Get().MoveOnScreen();

				// camdw - Please leave this.
				//
				// Bit messy, but the language selection screen will be calling LanguageSelectionScreenUpdateIdle
				// every update and checking this flag to see when it is safe to move on to the next screen.  When save/load
				// not busy LanguageSelectionScreenUpdateIdle will be responsible for moving on to the next screen.
				//bDoPlayerOptionsSaveLoadBusyCheck = true;
			}
			else
			{
				// Set temp options.  Called from the options screen in game.
				CPlayerOptions::Get().SetSubtitleLanguage( (SubtitleLanguage)lang );

				// Called from options, so go back.
				CGuiManager::Get().MoveBackScreen();
			}
		}

		void CopyToTemp( void )
		{
			CPlayerOptions::Get().CopyToTemp();
		}

		void OptionsBack( void )
		{
			ntPrintf( "OptionsBack().\n" );

			CPlayerOptions::Get().StopPlayingDialogueSample();

#ifdef PLATFORM_PS3
			if	( CPlayerOptions::Get().GetChanged() )
			{
				CSysMsgDialog::CreateDialog( CSysMsgDialog::YESNO_NORMAL_YES, "OPTIONS_OVERWRITE_OPTIONS", SaveOptionsDialogCallback );
			}
			else
			{
				CGuiManager::Get().MoveBackScreen();
			}
#else
			// PC just comit the changes and go back.
			CPlayerOptions::Get().ComitChanges();
			CGuiManager::Get().MoveBackScreen();
#endif
		}

		// Handle the back action on the Language Select screen.
		// Back action is not allowed during the startup sequence.
		void LanguageBack( void )
		{
			if	( 1 == iCallingScreen )
			{
				// Called from options, so go back.
				CGuiManager::Get().MoveBackScreen();
			}
		}

		// Update display to show selected language.
		void UpdateSelectedLanguage( void )
		{
			// Lams string id's.
			static const char *apcLanguageID[ NT_SUBTITLE_NUM_LANGUAGES ] =
			{
				"LANGUAGE_DA",
				"LANGUAGE_NL",
				"LANGUAGE_EN",
				"LANGUAGE_FI",
				"LANGUAGE_FR",
				"LANGUAGE_DE",
				"LANGUAGE_IT",
				"LANGUAGE_JA",
				"LANGUAGE_NN",
				"LANGUAGE_PT",
				"LANGUAGE_ES",
				"LANGUAGE_SV",
				"LANGUAGE_US",
				"LANGUAGE_KO"
			};

			int iLanguage = CPlayerOptions::Get().GetSubtitleLanguage();

			CGuiUnit* pobChild = CGuiLua::GetContext()->FindChildUnit("SELECTED_LANGUAGE", true);
			ntAssert(pobChild);

			if (pobChild)
			{
				pobChild->SetAttribute("titleid", apcLanguageID[ iLanguage ]);
			}
		}

		// Update display to show selected dialogue language.
		void UpdateSelectedDialogueLanguage( void )
		{
			// Lams string id's.
			static const char *apcLanguageID[ NT_AUDIO_NUM_LANGUAGES ] =
			{
				"LANGUAGE_DA",
				"LANGUAGE_NL",
				"LANGUAGE_EN",
				"LANGUAGE_FI",
				"LANGUAGE_FR",
				"LANGUAGE_DE",
				"LANGUAGE_IT",
				"LANGUAGE_JA",
				"LANGUAGE_NN",
				"LANGUAGE_PT",
				"LANGUAGE_ES",
				"LANGUAGE_SV",
			};

			int iLanguage = CPlayerOptions::Get().GetAudioLanguage();

			CGuiUnit* pobChild = CGuiLua::GetContext()->FindChildUnit("DIALOGUE_LANGUAGE", true);
			ntAssert(pobChild);

			if (pobChild)
			{
				pobChild->SetAttribute("titleid", apcLanguageID[ iLanguage ]);
			}
		}

		
		void DialogueLanguageSelectionBeginIdle( void )
		{
			// Select the appropriate menu option to show current audio language.
			NinjaLua::LuaObject obStore = CGuiLua::GetContext()->GetScreenHeader()->GetScreenStore();
			obStore.Set("menuCurrentSelection", (int)CPlayerOptions::Get().GetAudioLanguage());
		}

		void LanguageSelectionBeginIdle( void )
		{
			// Select the appropriate menu option to show current subtitle language.
			NinjaLua::LuaObject obStore = CGuiLua::GetContext()->GetScreenHeader()->GetScreenStore();
			obStore.Set("menuCurrentSelection", (int)CPlayerOptions::Get().GetSubtitleLanguage());
		}

		void SetAppropriateAudioLanguageForSubtitleLanguage( const SubtitleLanguage eSubtitleLanguage )
		{
			// Default to english.
			AudioLanguage eAudioLanguage = NT_AUDIO_EURO_ENGLISH;

			// If audio exists for the chosen subtitle language.
			if	( eSubtitleLanguage < (SubtitleLanguage)NT_AUDIO_NUM_LANGUAGES )
			{
				switch( eSubtitleLanguage )
				{
				// Deal with Scandinavian requirements.
				case NT_SUBTITLE_DANISH:
				case NT_SUBTITLE_FINNISH:
				case NT_SUBTITLE_NORWEGIAN:
				case NT_SUBTITLE_SWEDISH:
					eAudioLanguage = NT_AUDIO_EURO_ENGLISH;
					break;

				default:
					eAudioLanguage = (AudioLanguage)eSubtitleLanguage;
					break;
				}
			}

			CPlayerOptions::Get().SetLiveAudioLanguage( eAudioLanguage );
		}
	};

	namespace GameData
	{
		// No Game Data
		void NGD( void )
		{
#ifdef PLATFORM_PS3
			CGameData::Start();
#else
			CGuiManager::Get().MoveOnScreen();
#endif
		}

		// No Game Data Sufficient Space
		void NGDSS( void )
		{
#ifdef PLATFORM_PS3
			if	( CGameData::Start( CGameData::TESTMODE_NOGAMEDATA_HAVESPACE ) )
			{
			CGuiManager::Get().MoveOnScreen();
			}
#else
			CGuiManager::Get().MoveOnScreen();
#endif
		}

		// No Game Data Insuf
		void NGDIS( void )
		{
#ifdef PLATFORM_PS3
			CGameData::Start( CGameData::TESTMODE_NOGAMEDATA_NOSPACE );
#else
			CGuiManager::Get().MoveOnScreen();
#endif
		}

		// Game Data Sufficient Space
		void GDSS( void )
		{
#ifdef PLATFORM_PS3
			if	( CGameData::Start( CGameData::TESTMODE_GAMEDATA_HAVESPACE ) )
			{
				CGuiManager::Get().MoveOnScreen();
			}
#else
			CGuiManager::Get().MoveOnScreen();
#endif
		}

		// Game Data Insufficient Space
		void GDIS( void )
		{
#ifdef PLATFORM_PS3
			CGameData::Start( CGameData::TESTMODE_GAMEDATA_NOSPACE );
#else
			CGuiManager::Get().MoveOnScreen();
#endif
		}

		// CleanUp
		void CleanUp( void )
		{
#ifdef PLATFORM_PS3
			//CGameData::CleanUp();
#endif
		}
	};

	namespace Screen
	{
		NinjaLua::LuaObject GetLuaStore()
		{
			return CGuiLua::GetContext()->GetScreenHeader()->GetScreenStore();
		}

		void AllowRender(bool bAllow)
		{
			CGuiLua::GetContext()->AllowRender(bAllow);
		}
		
		void SetChildAttribute(const char* pcChild, const char* pcItem, const char* pcValue)
		{
			CGuiUnit* pobChild = NULL;

			if (pcChild == NULL)
				pobChild = CGuiLua::GetContext();
			else
                pobChild = CGuiLua::GetContext()->FindChildUnit(pcChild, true);

			ntAssert(pobChild);

			if (pobChild)
			{
				pobChild->SetAttribute(pcItem, pcValue);
			}
		}

		CGuiScreen* Current()
		{
			return CGuiLua::GetContext();
		}
	};

	namespace SpecialFeatures
	{
		bool IsCheckpointComplete( int iChapter, int iCheckpoint );

		void StartCredits()
		{
			NT_NEW CCredits();
			CCredits::Get().Initialise();
			CCredits::Get().Run();

			ntPrintf( "StartCredits" );
		}

		// Called in response to the exitevent in creditsvideo.xml.
		// 	<EVENT beginexit="gui.SpecialFeatures.ExitCredits()"/> and ultimately
		void ExitCredits()
		{
			if ( CCredits::Exists() )
			{
				if	( CCredits::Get().Terminate() )
				{
					CCredits::Kill();

					ntPrintf( "ExitCredits" );
				}
			}
			else
			{
				ntPrintf("WARNING: gui.SpecialFeatures.ExitCredits called when CCredits does not exist\n");
			}
		}

		// Called every update as part of creditsvideo.xml update
		// 	<EVENT updateidle="gui.SpecialFeatures.CheckCreditsFinished()"/>
		void CheckCreditsFinished()
		{
			if ( CCredits::Exists() )
			{
				if	( CCredits::Get().DisplayedAll() )
				{
					// Not needed yet but will need to add extra logic for when
					// credits have been called during end game.
					
					// MoveBackScreen will trigger the exit event in creditsvideo.xml
					// 	<EVENT beginexit="gui.SpecialFeatures.ExitCredits()"/> and ultimately
					// kill the credits singleton.
					CGuiManager::Get().MoveBackScreen();
				}
			}
			else
			{
				ntPrintf("WARNING: gui.SpecialFeatures.CheckCreditsFinished called when CCredits does not exist\n");
			}
		}
	
		// Check player progress and unlock the appropriate assets.
		void UnlockableContent( void )
		{
			ntPrintf( "UnlockableContent.\n" );

			// Get the screen object.
			CGuiUnit* pobScreen = CGuiLua::GetContext();
			ntAssert(pobScreen);
			
			CGuiUnit* pobMenuSelect = NULL;
			if	( pobScreen )
			{
				// Find MENUSELECT object
				pobMenuSelect = pobScreen->FindChildUnit("MENUSELECT", false);
				ntAssert(pobMenuSelect);
			}

			// Get game info.
			GameInfoDef* pobGameInfo = NULL;
			if	( pobMenuSelect )
			{
				pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
				ntAssert_p(pobGameInfo, ("GameInfo object not found") );
			}

			// Find out which checkpoints have been completed and unlock content.
			if	( pobGameInfo )
			{
				int iNumChapters = pobGameInfo->NumChapters();
				ntPrintf( "iNumChapters = %d.\n", iNumChapters );

				for	( int iChapter = 0; iChapter < iNumChapters; iChapter++ )
				{
					const ChapterInfoDef* pobChapterInfo = pobGameInfo->GetChapter( iChapter );

					if	( pobChapterInfo )
					{
						int iNumCheckPoints = pobChapterInfo->NumCheckpoints();
						ntPrintf( "iNumCheckPoints = %d.\n", iNumCheckPoints );

						// NOTE : May need to change this so that it searches for the last checkpoint found
						// and steps back one so I can get the completed checkpoint rather than the most
						// recent checkpoint reached.
						for	( int iCheckPoint = 0; iCheckPoint < iNumCheckPoints; iCheckPoint++ )
						{
							if	( IsCheckpointComplete( iChapter, iCheckPoint ) )
							{
								// Construct ID to search for.
								char acBuff[ 16 ];
								sprintf( acBuff, "CH%dCP%d", iChapter, iCheckPoint );  
								CHashedString obID( acBuff );

								// Process MENUBUTTON objects within MENUSELECT.
								for( ntstd::List< CXMLElement* >::iterator obIt = pobMenuSelect->GetChildren().begin(); obIt != pobMenuSelect->GetChildren().end(); ++obIt)
								{
									CGuiUnit* pobUnit = (CGuiUnit*)(*obIt);

									if	( pobUnit )
									{
										if	( obID == pobUnit->GetUnitID() )
										{
											pobUnit->SetAttribute("selectable", "TRUE");
											
											// Call PostProcessEnd to update selectable appearance.
											pobUnit->PostProcessEnd();
										}
									}
								}
							}
						}
					}
				}
			}
		}

		bool IsCheckpointComplete( int iChapter, int iCheckpoint )
		{
			// Get game info.
			GameInfoDef* pobGameInfo = NULL;
			pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
			ntAssert_p(pobGameInfo, ("GameInfo object not found") );

			// Find out which checkpoints have been completed and unlock content.
			if	( pobGameInfo )
			{
				const ChapterInfoDef* pobChapterInfo = pobGameInfo->GetChapter( iChapter );

				if	( pobChapterInfo )
				{
					if	( CheckpointManager::Get().GetDataForCheckpoint( iChapter, iCheckpoint ) )
					{
						return true;
					}
				}
			}

			return false;
		}
	};

	namespace Startup
	{
		bool PlayerOptionsSaveLoadBusy( void );

		void CheckSysLanguageSupport( void )
		{
			eSubtitleLanguage = NT_SUBTITLE_NUM_LANGUAGES;

#ifdef PLATFORM_PS3
			// Only allow this check during the startup sequence and after first boot.
			//
			// iCallingScreen is set from options.xml and controllercheck.xml and will be 0 during the startup sequence.
			if	( (0 == iCallingScreen)  && ( false == bFirstBoot ) )
			{
				// Get the system langauge.
				int iIDSystemLang = CELL_SYSUTIL_LANG_ENGLISH;
				cellSysutilGetSystemParamInt( CELL_SYSUTIL_SYSTEMPARAM_ID_LANG, &iIDSystemLang );
				
				// Game language order differs from System Language order so remap.
				eSubtitleLanguage = Language::GetSubtitleLanguageFromSysLanguage( iIDSystemLang );

				ntPrintf( "SystemLanguageID = %d, PlayerOptionsSubtitleLanguageID = %d.\n", eSubtitleLanguage, CPlayerOptions::Get().GetLiveSubtitleLanguage() );

				// Skip the language selection screen if System Language is supported and is the same as Options Language.
				if	( (eSubtitleLanguage != NT_SUBTITLE_NUM_LANGUAGES) && (CPlayerOptions::Get().GetLiveSubtitleLanguage() == eSubtitleLanguage) )
				{
					Screen::AllowRender( false );
				}
				else
				{
					// Mismatch between System Language and Options Language so show the language select menu.
					eSubtitleLanguage = NT_SUBTITLE_NUM_LANGUAGES;
				}
			}
#endif
		}

		// eSubtitleLanguage will only ever get a valid value when CheckSysLanguageSupport has been called
		// during the startup sequence.  A valid eSubtitleLanguage will result in the code below moving onto
		// the next screen.
		void ProcessSysLanguageSupport( void )
		{
			if	( eSubtitleLanguage != NT_SUBTITLE_NUM_LANGUAGES )
			{
				// Game supports the System Language.  Set language and skip the language selection screen.
				CPlayerOptions::Get().SetLiveSubtitleLanguage( eSubtitleLanguage );

				// Skip the language selection screen.
				CGuiManager::Get().MoveOnScreen();
			}
			else
			{
				// Language Selection screen will be displayed so show the current subtitle language
				// selected.

				NinjaLua::LuaObject obStore = CGuiLua::GetContext()->GetScreenHeader()->GetScreenStore();
				obStore.Set("menuCurrentSelection", (int)CPlayerOptions::Get().GetLiveSubtitleLanguage());
			}
		}

		void CheckGameDataTestMode( void )
		{
#ifdef PLATFORM_PS3
			if	( CGameData::m_bGameDataTestMode )
			{
				Screen::AllowRender( true );
			}
			else
			{
				CGameData::Start();
				CGuiManager::Get().MoveOnScreen();
			}
#else
			Screen::AllowRender( false );
			CGuiManager::Get().MoveOnScreen();
#endif
		}

		// First Screen
		//---------------------------------------------------------------
		void FirstScreenEnterIdle( void )
		{
			bFirstBoot = false;
			bDoPlayerOptionsSaveLoadBusyCheck = true;
		}

		void FirstScreenUpdateIdle( void )
		{
			// Wait for player options to load.
			if	( false == PlayerOptionsSaveLoadBusy() )
			{
				// Player options version may have changed which would result in a failed load.
				// If there was a version error set the first boot flag to force a language selection
				// and ultimately a player options save.
				if	( CPlayerOptions::Get().GetLiveFirstBoot() || CPlayerOptions::Get().GetVersionError() )
				{
					CPlayerOptions::Get().ClearVersionError();

					bFirstBoot = true;

					CPlayerOptions::Get().SetLiveFirstBoot( false );

					// No need to save first boot setting here.  The language selection screen is always shown on first boot and
					// saves player options when calling SetSubtitleLanguage so the first boot setting will also be saved.
				}

				CSubtitleMan::Get().Enable( CPlayerOptions::Get().GetLiveShowSubtitles() );

				CGuiManager::Get().MoveOnScreen();
			}
		}
		//---------------------------------------------------------------

		// Language selection screen
		//---------------------------------------------------------------
		void LanguageSelectionScreenEnterIdle( void )
		{
			// Will be 0 if called during the startup sequence.
			if	( 0 == iCallingScreen )
			{
				bDoPlayerOptionsSaveLoadBusyCheck = false;
			}
		}

		void LanguageSelectionScreenUpdateIdle( void )
		{
			// Will be 0 if called during the startup sequence.
			if	( 0 == iCallingScreen )
			{
				if	( false == PlayerOptionsSaveLoadBusy() )
				{
					CGuiManager::Get().MoveOnScreen();
				}
			}
		}
		//---------------------------------------------------------------
	
		bool PlayerOptionsSaveLoadBusy( void )
		{
#ifdef PLATFORM_PS3
			if	( bDoPlayerOptionsSaveLoadBusyCheck )
			{
				// Stop the screen moving on until player options save/load is complete.
				if	( false == CPlayerOptions::Get().GetSaveDataBusy() )
				{
					// Stop the check.
					bDoPlayerOptionsSaveLoadBusyCheck = false;
					
					return false;
				}
				else
				{
					//ntPrintf( "PlayerOptionsSaveLoadBusy() - Waiting for playeroptions save/load to complete.\n" );
				}
			}

			return true;
#else
			bDoPlayerOptionsSaveLoadBusyCheck = false;
			
			return false;
#endif
		}
	};

	float AwaitingDesignTimeout()
	{
		return 1.0f;
	}

	int MoveOnScreen(NinjaLua::LuaState* pobState)
	{
		NinjaLua::LuaStack obArgs( pobState );

		if (obArgs[1].IsNone())
		{
			CGuiManager::Get().MoveOnScreen();
		}
		else if (obArgs[1].IsString())
		{
			CGuiManager::Get().MoveOnScreenGroup(obArgs[1].GetString());
		}
		else
		{
			ntAssert_p(false, ("invalid Arg to MoveOnScreen"));
			return 0;
		}

		return 1;
	}

	int MoveBackScreen(NinjaLua::LuaState* pobState)
	{
		NinjaLua::LuaStack obArgs( pobState );

		if (obArgs[1].IsNone())
		{
			CGuiManager::Get().MoveBackScreen();
		}
		else if (obArgs[1].IsString())
		{
			CGuiManager::Get().MoveBackScreenGroup(obArgs[1].GetString());
		}
		else
		{
			ntAssert_p(false, ("invalid Arg to MoveBackScreen"));
			return 0;
		}

		return 1;
	}

	namespace Purgatory
	{
		int Debug_SetupFakeInfo(NinjaLua::LuaState* pobState)
		{
			UNUSED(pobState);

#ifndef _GOLD_MASTER

			//dump gamedata
			if (g_bClearAllGameData)
			{
				ntPrintf("Clearing checkpoint data\n");
				CheckpointManager::Get().Debug_ClearCheckpointData();
			}

			GameInfoDef* pobInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));

			const GameInfoDef::ChapterList& obChapters = pobInfo->Chapters();
			for (GameInfoDef::ChapterList::const_iterator obChapterIt = obChapters.begin(); obChapterIt != obChapters.end(); ++obChapterIt)
			{
				int iChapterNum = (*obChapterIt)->ChapterNumber();
				if (!g_bUnlockAllGameData && iChapterNum > g_iChaptersComplete)
					continue;

				const ChapterInfoDef::CheckpointList& obCheckpoints = (*obChapterIt)->Checkpoints();
				for (ChapterInfoDef::CheckpointList::const_iterator obCPIt = obCheckpoints.begin(); obCPIt != obCheckpoints.end(); ++obCPIt)
				{
					int iCPNum = (*obCPIt)->CheckpointNumber();
					if (!g_bUnlockAllGameData && (iChapterNum == g_iChaptersComplete && iCPNum > g_iUpToCheckpoint))
						continue;
                    
					if (CheckpointManager::Get().GetDataForCheckpoint(iChapterNum, iCPNum))
					{
						ntPrintf("Data already exists for Chapter %d, Checkpoint %d\n", iChapterNum, iCPNum);
						continue;
					}

					CheckpointData* pobData = CheckpointManager::Get().CreateDataForCheckpoint(iChapterNum, iCPNum);

					double dLifeClockDelta = 0;
					dLifeClockDelta += (float)grandf(59.99f);		//seconds
					dLifeClockDelta += ((int)grandf(59.99f)) * 60.0;	//minutes
					dLifeClockDelta += ((int)grandf(23.99f)) * 3600.0;	//hours
					dLifeClockDelta += ((int)grandf(5.0f)) * 86400.0;	//days

					pobData->m_GlobalData.SetLifeClockDelta(dLifeClockDelta);

					ntPrintf("Added data for Chapter %d, Checkpoint %d, Delta: %f\n", iChapterNum, iCPNum, dLifeClockDelta);
					}
					}

			if (g_bClearHitFlags)
				CheckpointManager::Get().ClearAllCheckpointHitFlags();

			if (g_bSetLastHitFlags && CheckpointManager::Get().GetLastCheckpoint())
				CheckpointManager::Get().GetLastCheckpoint()->m_GenericData.SetNewlyHitCheckpointFlag();
#endif
			return 1;
		}
	}

	void SetCallingScreen( int iScreen )
	{
		iCallingScreen = iScreen;
	}

};

///
//Registration logic
///

void AddGameplayBindings(NinjaLua::LuaObject obGUI)
{
	//create table
	NinjaLua::LuaObject obGameplay = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());

	//populate
	obGameplay.Register("PauseGameUser",				GUI::Gameplay::PauseGameUser);
	obGameplay.Register("NewGameProfileOverwriteCheck",	GUI::Gameplay::NewGameProfileOverwriteCheck);
	obGameplay.Register("PauseMenuReturnToPurgatory",	GUI::Gameplay::PauseMenuReturnToPurgatory);
	obGameplay.Register("PauseMenuExitGame",			GUI::Gameplay::PauseMenuExitGame);
	obGameplay.Register("PauseScreenPurgatoryCheck",	GUI::Gameplay::PauseScreenPurgatoryCheck);

	obGameplay.Register("OnEnterFrontend",				GUI::Gameplay::OnEnterFrontend);
	obGameplay.Register("OnPause",						GUI::Gameplay::OnPause);
	obGameplay.Register("OnUnpause",					GUI::Gameplay::OnUnpause);
	obGameplay.Register("OnLevelStart",					GUI::Gameplay::OnLevelStart);
	obGameplay.Register("OnLevelExit",					GUI::Gameplay::OnLevelExit);

	//add to gui table
	obGUI.Set("Gameplay", obGameplay);
}

void AddDebugBindings(NinjaLua::LuaObject obGUI)
{
	//create table
	NinjaLua::LuaObject obDebug = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());

	//populate

	//add to gui table
	obGUI.Set("Debug", obDebug);
}

void AddSystemBindings(NinjaLua::LuaObject obGUI)
{
	//create table
	NinjaLua::LuaObject obSystem = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());

	//populate
	obSystem.Register("UnloadCurrentLevel",		GUI::System::UnloadCurrentLevel);
	obSystem.Register("GetLuaStore",			GUI::System::GetLuaStore);
	obSystem.Register("LoadFrontendStrings",	GUI::System::LoadFrontendStrings);
	obSystem.Register( "LoadPurgatoryLevel",	GUI::System::LoadPurgatoryLevel );
	obSystem.Register( "InPurgatory",			GUI::System::InPurgatory );
	obSystem.Register( "RestartFromLastCheckpoint",GUI::System::RestartFromLastCheckpoint );
	obSystem.Register("GetFailureMessage",		GUI::System::GetFailureMessage);
	obSystem.Register("LoadSelectedChapter",	GUI::System::LoadSelectedChapter);
	obSystem.Register("StartNewGame",			GUI::System::StartNewGame);
	obSystem.Register("IsLevelLoading",			GUI::System::IsLevelLoading);
	obSystem.Register("GotoLatestCheckpoint",	GUI::System::GotoLatestCheckpoint);
	obSystem.Register("SkipFromPurgatoryToLoadingScreen",	GUI::System::SkipFromPurgatoryToLoadingScreen);
	obSystem.Register("SkipToChapterSelect",	GUI::System::SkipToChapterSelect);
	obSystem.Register("ExitPurgatory",			GUI::System::ExitPurgatory);
	obSystem.Register("StandardChapterExit",	GUI::System::StandardChapterExit);
	obSystem.Register("CheckpointDataAvailable",GUI::System::CheckpointDataAvailable);

	//add to gui table
	obGUI.Set("System", obSystem);
}

void AddOptionsBindings(NinjaLua::LuaObject obGUI)
{
	//create table
	NinjaLua::LuaObject obOptions = NinjaLua::LuaObject::CreateTable( CLuaGlobal::Get().State() );

	//populate
	obOptions.Register( "enableMotionControl", GUI::Options::enableMotionControl );
	obOptions.Register( "InvertY", GUI::Options::InvertY );
	obOptions.Register( "setSFXVolume", GUI::Options::setSFXVolume );
	obOptions.Register( "setMusicVolume", GUI::Options::setMusicVolume );
	obOptions.Register( "setDialogueVolume", GUI::Options::setDialogueVolume );
	obOptions.Register( "enableSubtitles", GUI::Options::enableSubtitles );
	obOptions.Register( "SetAudioLanguage", GUI::Options::SetAudioLanguage );
	obOptions.Register( "SetSubtitleLanguage", GUI::Options::SetSubtitleLanguage );
	obOptions.Register( "CopyToTemp", GUI::Options::CopyToTemp );
	obOptions.Register( "OptionsBack", GUI::Options::OptionsBack );
	obOptions.Register( "LanguageBack", GUI::Options::LanguageBack );
	obOptions.Register( "UpdateSelectedLanguage", GUI::Options::UpdateSelectedLanguage );
	obOptions.Register( "UpdateSelectedDialogueLanguage", GUI::Options::UpdateSelectedDialogueLanguage );
	obOptions.Register( "DialogueLanguageSelectionBeginIdle", GUI::Options::DialogueLanguageSelectionBeginIdle );
	obOptions.Register( "LanguageSelectionBeginIdle", GUI::Options::LanguageSelectionBeginIdle );
	obOptions.Register( "ManageDialogueSample", GUI::Options::ManageDialogueSample );

	//add to gui table
	obGUI.Set( "playerOptions", obOptions );
}

void AddSpecialFeaturesBindings(NinjaLua::LuaObject obGUI)
{
	//create table
	NinjaLua::LuaObject obSpecialFeatures = NinjaLua::LuaObject::CreateTable( CLuaGlobal::Get().State() );

	//populate
	obSpecialFeatures.Register( "StartCredits",	GUI::SpecialFeatures::StartCredits );
	obSpecialFeatures.Register( "ExitCredits",	GUI::SpecialFeatures::ExitCredits );
	obSpecialFeatures.Register( "CheckCreditsFinished",	GUI::SpecialFeatures::CheckCreditsFinished );
	obSpecialFeatures.Register( "UnlockableContent",	GUI::SpecialFeatures::UnlockableContent );
	obSpecialFeatures.Register( "IsCheckpointComplete",	GUI::SpecialFeatures::IsCheckpointComplete );

	//add to gui table
	obGUI.Set( "SpecialFeatures", obSpecialFeatures );
}

void AddGameDataBindings(NinjaLua::LuaObject obGUI)
{
	//create table
	NinjaLua::LuaObject obGameData = NinjaLua::LuaObject::CreateTable( CLuaGlobal::Get().State() );

	//populate
	obGameData.Register( "NGD", GUI::GameData::NGD );
	obGameData.Register( "NGDSS", GUI::GameData::NGDSS );
	obGameData.Register( "NGDIS", GUI::GameData::NGDIS );
	obGameData.Register( "GDSS", GUI::GameData::GDSS );
	obGameData.Register( "GDIS", GUI::GameData::GDIS );
	obGameData.Register( "CleanUp", GUI::GameData::CleanUp );

	//add to gui table
	obGUI.Set( "GameData", obGameData );
}

void AddScreenBindings(NinjaLua::LuaObject obGUI)
{
	//create table
	NinjaLua::LuaObject obScreen = NinjaLua::LuaObject::CreateTable( CLuaGlobal::Get().State() );

	//populate
	obScreen.Register( "GetLuaStore", GUI::Screen::GetLuaStore);
	obScreen.Register( "AllowRender", GUI::Screen::AllowRender );
	obScreen.Register( "SetChildAttribute", GUI::Screen::SetChildAttribute );
	obScreen.Register( "Current", GUI::Screen::Current );

	//add to gui table
	obGUI.Set( "Screen", obScreen );
}

void AddStartupBindings(NinjaLua::LuaObject obGUI)
{
	//create table
	NinjaLua::LuaObject obStartup = NinjaLua::LuaObject::CreateTable( CLuaGlobal::Get().State() );

	//populate
	obStartup.Register( "CheckSysLanguageSupport", GUI::Startup::CheckSysLanguageSupport );
	obStartup.Register( "ProcessSysLanguageSupport", GUI::Startup::ProcessSysLanguageSupport );
	obStartup.Register( "CheckGameDataTestMode", GUI::Startup::CheckGameDataTestMode );
	obStartup.Register( "FirstScreenEnterIdle", GUI::Startup::FirstScreenEnterIdle );
	obStartup.Register( "FirstScreenUpdateIdle", GUI::Startup::FirstScreenUpdateIdle );
	obStartup.Register( "LanguageSelectionScreenEnterIdle", GUI::Startup::LanguageSelectionScreenEnterIdle );
	obStartup.Register( "LanguageSelectionScreenUpdateIdle", GUI::Startup::LanguageSelectionScreenUpdateIdle );

	//add to gui table
	obGUI.Set( "Startup", obStartup );
}

void AddPurgatoryBindings(NinjaLua::LuaObject obGUI)
{
	//create table
	NinjaLua::LuaObject obPurgatory = NinjaLua::LuaObject::CreateTable( CLuaGlobal::Get().State() );

	//populate
	obPurgatory.RegisterRaw( "Debug_SetupFakeInfo",		GUI::Purgatory::Debug_SetupFakeInfo);

	//add to gui table
	obGUI.Set( "Purgatory", obPurgatory );
}
/***************************************************************************************************
*
*	FUNCTION		CGuiLua::Initialise
*
*	DESCRIPTION		Registers all gui lua bindings. It should only be called once.
*
***************************************************************************************************/
void CGuiLua::Initialise()
{
	//create GUI table
	NinjaLua::LuaObject obGUI = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());

	//populate
	AddGameplayBindings(obGUI);
	AddDebugBindings(obGUI);
	AddSystemBindings(obGUI);
	AddOptionsBindings( obGUI );
	AddGameDataBindings( obGUI );
	AddScreenBindings( obGUI );
	AddSpecialFeaturesBindings( obGUI );
	AddPurgatoryBindings( obGUI );
	AddStartupBindings(obGUI);

	obGUI.Register("AwaitingDesignTimeout",		GUI::AwaitingDesignTimeout);
	obGUI.Register( "SetCallingScreen", GUI::SetCallingScreen );

	obGUI.RegisterRaw("MoveOnScreen",		GUI::MoveOnScreen);
	obGUI.RegisterRaw("MoveBackScreen",		GUI::MoveBackScreen);

	//push into lua system
	CLuaGlobal::Get().State().GetGlobals().Set("gui", obGUI);

	//create global store
	ntAssert(ms_obGlobalStore.IsNil());
	ms_obGlobalStore = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());

	//create context stack
	ms_pobContextStack = NT_NEW ContextStack;
}

void CGuiLua::Uninitialise()
{
	CLuaGlobal::Get().State().GetGlobals()["gui"].SetNil();

	ms_obGlobalStore.SetNil();

	NT_DELETE(ms_pobContextStack);
}

void CGuiLua::PushContext(CGuiScreen* pobScreen)
{
	ntAssert(ms_pobContextStack);

	ms_pobContextStack->push_front(pobScreen);
}

void CGuiLua::PopContext()
{
	ntAssert(ms_pobContextStack);
	ntAssert(ms_pobContextStack->size() >= 1);

	ms_pobContextStack->pop_front();
}

CGuiScreen* CGuiLua::GetContext()
{
	ntAssert(ms_pobContextStack);
	ntAssert(ms_pobContextStack->size() >= 1);

	return ms_pobContextStack->front();
}

NinjaLua::LuaObject CGuiLua::GetGlobalStore()
{
	ntAssert(!ms_obGlobalStore.IsNil());
	return ms_obGlobalStore;
}

///
//Utils
///

//------------------------------------------------------------------------------
//!
//!	CGuiLua::GetLuaFunction
//!
//!	pcFunction should not include ().
//!
//!	FunctionName() is bad.
//! FunctionName is good.
//!
//------------------------------------------------------------------------------
NinjaLua::LuaFunction CGuiLua::GetLuaFunction(const char* pcFunction)
{
	char cBuf[32];
	char* pcBuf = &cBuf[0];
	NinjaLua::LuaObject obCurrentObj = CLuaGlobal::Get().State().GetGlobals();

	while (true)
	{
		if (*pcFunction != '.' && *pcFunction != 0)
		{
			*pcBuf++ = *pcFunction;
		}
		else
		{
			*pcBuf = 0;
			obCurrentObj = obCurrentObj[ cBuf ];
			pcBuf = &cBuf[0];

			if (obCurrentObj.IsNil())
				break;
		}

		if (*pcFunction++ == 0)
			break;
	}

	if (!obCurrentObj.IsNil() && obCurrentObj.IsFunction())
	{
		NinjaLua::LuaFunction luaFunc = NinjaLua::LuaFunction(obCurrentObj);
        return luaFunc;
	}

	return NinjaLua::LuaFunction();
}

#ifdef PLATFORM_PS3

static void SaveOptionsDialogCallback( const CellMsgDialogButtonType ButtonPressed )
{
	if	( CSysMsgDialog::YES == ButtonPressed )
	{
		CPlayerOptions::Get().ComitChanges();
		CPlayerOptions::Get().Save();
		CGuiManager::Get().MoveBackScreen();
	}
	else if ( CSysMsgDialog::NO == ButtonPressed )
	{
		// Player may have changed language whilst in the options menu which would have
		// caused language font to change.  Now the player has said no to options changes
		// so must reset the language system to the current live language.
		CStringManager::Get().SetLanguage( CPlayerOptions::Get().GetLiveSubtitleLanguage() );

		// Overwrite changes with live version.
		CPlayerOptions::Get().CopyToTemp();

		CGuiManager::Get().MoveBackScreen();
	}
}

static void NewGameProfileOverwriteCallback( const CellMsgDialogButtonType ButtonPressed )
{
	if	( CSysMsgDialog::YES == ButtonPressed )
	{
		//CGuiManager::Get().MoveOnScreenGroup( "purgatory" );
		CGuiManager::Get().SkipForwardScreen(NEWGAME_SKIPFORWARD_PATH_ARGS);
	}
}

static void PauseMenuReturnToPurgatoryCallback( const CellMsgDialogButtonType ButtonPressed )
{
	if	( CSysMsgDialog::YES == ButtonPressed )
	{
		PauseMenuReturnToPurgatoryCallbackYes();
	}
}

static void PauseMenuExitGameCallback( const CellMsgDialogButtonType ButtonPressed )
{
	if	( CSysMsgDialog::YES == ButtonPressed )
	{
		PauseMenuExitGameCallbackYes();
	}
}

static void NewGameAreYouSureCallback( const CellMsgDialogButtonType ButtonPressed )
{
	if	( CSysMsgDialog::YES == ButtonPressed )
	{
		CGuiManager::Get().SkipForwardScreen(NEWGAME_SKIPFORWARD_PATH_ARGS);
	}
}

#endif

//--------------------------------------------------------------------------------------
//!
//!	PauseMenuReturnToPurgatoryCallbackYes
//!
//! Perform processing required when the player wishes to exit the game and
//! return to Purgatory.
//!
//--------------------------------------------------------------------------------------
static void PauseMenuReturnToPurgatoryCallbackYes( void )
{
	//unload
	//GUI::System::UnloadCurrentLevel();

	//go back
//	CGuiManager::Get().MoveBackScreenGroup("purgatory");

	CGuiManager::Get().MoveOnScreenGroup("standardexit");

	//let go of userpause
	ShellMain::Get().RequestPauseToggle( ShellMain::PM_USER, false );
}

//--------------------------------------------------------------------------------------
//!
//!	PauseMenuExitGameCallbackYes
//!
//! Returns from the Pause menu to the Title screen
//!
//--------------------------------------------------------------------------------------
static void PauseMenuExitGameCallbackYes( void )
{
	//we need to tell the standard exit that we are actually going back to the titlescreen
	NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();
	obStore.Set("standardexitOverride", "title");

	CGuiManager::Get().MoveOnScreenGroup("standardexit");

	//as we are going to the title, make sure the frontend strings are loaded.
	GUI::System::LoadFrontendStrings();

	//let go of userpause
	ShellMain::Get().RequestPauseToggle( ShellMain::PM_USER, false );
}
