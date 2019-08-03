
#include "gui/guinotify.h"
#include "gui/guilua.h"
#include "gui/guimanager.h"
#include "game/gameinfo.h"
#include "game/checkpointmanager.h"
#include "game/shellmain.h"
#include "game/shelllevel.h"

void ApplyExitFromBattleScene()
{
	GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
	ntAssert_p(pobGameInfo, ("GameInfo object not found") );

	//locate the chapter
	const ChapterInfoDef* pobChapterInfo = pobGameInfo->InitialBattleChapter();
	ntAssert_p(pobChapterInfo, ("Failed to find InitialBattle chapter in GameInfo") );

	int iChapter = pobChapterInfo->ChapterNumber();

	//now, lookup the current level
	const ShellLevel* pPlayingLevel = ShellMain::Get().GetCurrRunningLevel();
	ntAssert_p( pPlayingLevel, ("No active level ?!?") );
	
	//we ahve a battlescene death
	if (pPlayingLevel->GetLevelNumber() == iChapter)
	{
		NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();
		obStore.Set("exitFromBattleScene", true);
	}
}

void ApplyFirstCompletion()
{
	//lookup the current level
	const ShellLevel* pPlayingLevel = ShellMain::Get().GetCurrRunningLevel();
	ntAssert_p(pPlayingLevel, ("No active level ?!?") );
	
	//locate the chapter
	GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
	ntAssert_p(pobGameInfo, ("GameInfo object not found") );
	const ChapterInfoDef* pobChapterInfo = pobGameInfo->GetChapter(pPlayingLevel->GetLevelNumber());
	ntAssert_p(pobChapterInfo, ("Failed to find InitialBattle chapter in GameInfo") );

	//last checkpoint
	const CheckpointInfoDef* pobCPInfo = pobChapterInfo->Checkpoints().back();

	int iChap = pobChapterInfo->ChapterNumber();
	int iCP = pobCPInfo->CheckpointNumber();

	//lookup data on this checkpoint
	CheckpointData* pobData = CheckpointManager::Get().GetDataForCheckpoint(iChap, iCP);
	ntAssert_p(pobData, ("Missing checkpoint data for chapter %d checkpoint %d", iChap, iCP));

	if (pobData->m_GenericData.IsNewlyHitCheckpoint())
	{
		NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();
		obStore.Set("chapterCompletedFirstTime", iChap);

		if (pobCPInfo->EndGame())
		{
			obStore.Set("gameCompletedFirstTime", true);
		}
	}
}

void CGuiNotify::ChapterComplete(int iChapterNumber, bool bUnlockNext)
{
	UNUSED(iChapterNumber);
	UNUSED(bUnlockNext);

	NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();
	obStore.Set("chapterExitReason", "chaptercomplete");

	ApplyFirstCompletion();
	ApplyExitFromBattleScene();

	CGuiManager::Get().MoveOnScreenGroup("chapterexit");
}

void CGuiNotify::PlayerDeath()
{
	NinjaLua::LuaObject obStore = CGuiLua::GetGlobalStore();
	obStore.Set("chapterExitReason", "death");

	//check if its a battlescene death
	ApplyExitFromBattleScene();

	CGuiManager::Get().MoveOnScreenGroup("chapterexit");
}

#ifdef _REGISTER_GUINOTIFY_KEYS

#include "game/keybinder.h"

COMMAND_RESULT CGuiNotifyKeys::ChapterComplete()
{
	//fake completion

	//lookup the current level
	const ShellLevel* pPlayingLevel = ShellMain::Get().GetCurrRunningLevel();
	ntAssert_p(pPlayingLevel, ("No active level ?!?") );
	
	GameInfoDef* pobGameInfo = ObjectDatabase::Get().GetPointerFromName<GameInfoDef*>(CHashedString("GameInfo"));
	ntAssert_p(pobGameInfo, ("GameInfo object not found") );
	const ChapterInfoDef* pobChapterInfo = pobGameInfo->GetChapter(pPlayingLevel->GetLevelNumber());
	ntAssert_p(pobChapterInfo, ("Failed to find InitialBattle chapter in GameInfo") );

	const ChapterInfoDef::CheckpointList& obCheckpoints = pobChapterInfo->Checkpoints();
	for (ChapterInfoDef::CheckpointList::const_iterator obCPIt = obCheckpoints.begin(); obCPIt != obCheckpoints.end(); ++obCPIt)
	{
		int iCPNum = (*obCPIt)->CheckpointNumber();

		if (NULL == CheckpointManager::Get().GetDataForCheckpoint(pobChapterInfo->ChapterNumber(), iCPNum))
			CheckpointManager::Get().CreateDataForCheckpoint(pobChapterInfo->ChapterNumber(), iCPNum);
	}

	CGuiNotify::ChapterComplete(0, false);
	return CR_SUCCESS;
}

COMMAND_RESULT CGuiNotifyKeys::PlayerDeath()
{
	CGuiNotify::PlayerDeath();
	return CR_SUCCESS;
}

void CGuiNotifyKeys::Register()
{
	CommandBaseNoInput* pobCmd;
		
	pobCmd = CommandManager::Get().CreateCommandNoInput("NotifyGui_ChapterComplete", this, &CGuiNotifyKeys::ChapterComplete, "NotifyGui_ChapterComplete");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCmd, "NotifyGui_ChapterComplete", KEYS_PRESSED, KEYC_C, KEYM_CTRL|KEYM_SHIFT);

	pobCmd = CommandManager::Get().CreateCommandNoInput("NotifyGui_PlayerDeath", this, &CGuiNotifyKeys::PlayerDeath, "NotifyGui_PlayerDeath");
	KeyBindManager::Get().RegisterKeyNoInput("game", pobCmd, "NotifyGui_PlayerDeath", KEYS_PRESSED, KEYC_D, KEYM_CTRL|KEYM_SHIFT);
}

#endif
