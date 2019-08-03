//--------------------------------------------------
//!
//!	\file game/gameinfo.cpp
//!	
//!
//--------------------------------------------------

#include "gameinfo.h"
#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE(CheckpointInfoDef, Mem::MC_MISC)
	PUBLISH_VAR_AS( m_iCheckpointNumber, CheckpointNumber )
	PUBLISH_VAR_AS( m_obCheckpointTitleID, CheckpointTitleID )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bFinal, false, Final )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bEndGame, false, EndGame )
	PUBLISH_PTR_AS(m_pobNextCheckpoint, NextCheckpoint)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(ChapterInfoDef, Mem::MC_MISC)
	PUBLISH_VAR_AS( m_iChapterNumber, ChapterNumber )
	PUBLISH_VAR_AS( m_obChapterTitleID, ChapterTitleID )
	PUBLISH_VAR_AS( m_obChapterPath, ChapterPath )
	PUBLISH_VAR_AS( m_obCompletionCutsceneTrigger, CompletionCutsceneTrigger )
	DECLARE_CSTRUCTURE_PTR_CONTAINER(m_obCheckpointList, CheckpointList)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(GameInfoDef, Mem::MC_MISC)
	DECLARE_CSTRUCTURE_PTR_CONTAINER(m_obChapterList, ChapterList)
	PUBLISH_PTR_AS(m_pobInitialBattleChapter, InitialBattleChapter)
END_STD_INTERFACE

const CheckpointInfoDef* ChapterInfoDef::GetCheckpoint(int iNumber) const
{
	for (CheckpointList::const_iterator obIt = m_obCheckpointList.begin(); obIt != m_obCheckpointList.end(); ++obIt)
	{
		if ((*obIt)->CheckpointNumber() == iNumber)
			return (*obIt);
	}
	return NULL;
}

const ChapterInfoDef* GameInfoDef::GetChapter(int iNumber) const
{
	for (ChapterList::const_iterator obIt = m_obChapterList.begin(); obIt != m_obChapterList.end(); ++obIt)
	{
		if ((*obIt)->ChapterNumber() == iNumber)
			return (*obIt);
	}
	return NULL;
}
/*
void GameInfoDef::FindNextCheckpoint(int iChapter, int iCheckpoint, int& iNewChapter, int& iNewCheckpoint)
{
	//locate the chapter
	for (ChapterList::const_iterator obChapIt = m_obChapterList.begin(); obChapIt != m_obChapterList.end(); ++obChapIt)
	{
		if ((*obChapIt)->ChapterNumber() == iChapter)
		{
			//we have found our chapter, search for the checkpoint
			const CheckpointList& obList = (*obChapIt)->Checkpoints();
			for (CheckpointList::const_iterator obCPIt = obList.begin(); obCPIt != obList.end(); ++obCPIt)
			{
				if ((*obCPIt)->CheckpointNumber() == iCheckpoint)
				{
					//got him. do we have another checkpoint?
					CheckpointList::const_iterator obTmpCP = obCPIt;
					++obTmpCP;
					if (obTmpCP != obList.end())
					{
						iNewChapter = iChapter;
						iNewCheckpoint = (*obTmpCP)->CheckpointNumber();
					}
					else
					{
						//we dont have a next, so get the next chapter's first.
						ChapterList::const_iterator obTmpChap = obChapIt;
						++obTmpChap;
						if (obTmpChap != m_obChapterList.end())
						{
							iNewChapter = (*obTmpChap)->ChapterNumber();
							iNewCheckpoint = (*obTmpChap)->Checkpoints()->front()->CheckpointNumber();
						}
						else
						{
							//endgame situation
						}
					}
				}
			}
		}
	}
	//and the checkpoint
//	const CheckpointInfoDef* pobCPInfo = pobChapterInfo->GetCheckpoint(iCheckpoint);
//	ntAssert_p(pobCPInfo, ("Failed to find checkpoint %d for chapter %d in GameInfo", iCheckpoint, iChapter) );

}
*/

const ChapterInfoDef* GameInfoDef::FindChapterContainingCheckpoint(const CheckpointInfoDef* pobCP) const
{
	//locate the chapter
	for (ChapterList::const_iterator obChapIt = m_obChapterList.begin(); obChapIt != m_obChapterList.end(); ++obChapIt)
	{
		const CheckpointInfoDef* pobTmp = (*obChapIt)->GetCheckpoint(pobCP->CheckpointNumber());

		if (pobTmp == pobCP)
			return (*obChapIt);
	}
	ntAssert_p(false, ("Requested checkpoint is not within a chapter -.-. pc textid = %s", ntStr::GetString(pobCP->CheckpointTitleID())));
	return NULL;
}

