//--------------------------------------------------
//!
//!	\file game/gameinfo.h
//!	
//!
//--------------------------------------------------

#ifndef	_GAMEINFO_H
#define	_GAMEINFO_H

#include "core/nt_std.h"

//--------------------------------------------------
//!
//! Class CheckpointInfoDef
//! 
//!
//--------------------------------------------------
class CheckpointInfoDef
{
	HAS_INTERFACE(CheckpointInfoDef);
public:
	int CheckpointNumber() const { return m_iCheckpointNumber; }
	const ntstd::String& CheckpointTitleID() const { return m_obCheckpointTitleID; }

	bool Final() const { return m_bFinal; }
	bool EndGame() const { return m_bEndGame; }

	const CheckpointInfoDef* NextCheckpoint() const { return m_pobNextCheckpoint; }

protected:
	bool m_bFinal;
	bool m_bEndGame;
	CheckpointInfoDef* m_pobNextCheckpoint;
	int m_iCheckpointNumber;
	ntstd::String m_obCheckpointTitleID;
};

//--------------------------------------------------
//!
//! Class ChapterInfoDef
//! 
//!
//--------------------------------------------------
class ChapterInfoDef
{
	HAS_INTERFACE(ChapterInfoDef);
public:
	typedef ntstd::List<CheckpointInfoDef*> CheckpointList;

public:
	const ntstd::String& ChapterTitleID() const { return m_obChapterTitleID; }
	const ntstd::String& ChapterPath() const { return m_obChapterPath; }
	const ntstd::String& CompletionCutsceneTrigger() const { return m_obCompletionCutsceneTrigger; }

	int ChapterNumber() const { return m_iChapterNumber; }

	int NumCheckpoints() const { return (int)m_obCheckpointList.size(); }

	const CheckpointList& Checkpoints() const { return m_obCheckpointList; }

	const CheckpointInfoDef* GetCheckpoint(int iNumber) const;
protected:

	int m_iChapterNumber;

	ntstd::String m_obChapterTitleID;
	ntstd::String m_obChapterPath;

	CheckpointList m_obCheckpointList;

	ntstd::String m_obCompletionCutsceneTrigger;
};

//--------------------------------------------------
//!
//! Class GameInfoDef
//! 
//!
//--------------------------------------------------
class GameInfoDef
{
	HAS_INTERFACE(GameInfoDef);
public:
	typedef ntstd::List<ChapterInfoDef*> ChapterList;

public:

	const ChapterInfoDef* GetChapter(int iNumber) const;

	int			NumChapters() const { return (int)m_obChapterList.size(); }

	const ChapterList& Chapters() const { return m_obChapterList; }

	const ChapterInfoDef* InitialBattleChapter() const { return m_pobInitialBattleChapter; }

	const ChapterInfoDef* FindChapterContainingCheckpoint(const CheckpointInfoDef* pobCP) const;

	//void FindNextCheckpoint(int iChapter, int iCheckpoint, int& iNewChapter, int& iNewCheckpoint);

protected:

	ChapterList m_obChapterList;
	ChapterInfoDef* m_pobInitialBattleChapter;
};

#endif // _GAMEINFO_H
