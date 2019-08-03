//--------------------------------------------------
//!
//!	\file game/entitypurgatorymanager.h
//!	Definition of the purgatory manager
//!
//--------------------------------------------------

#ifndef	_ENTITY_PURGATORYMANAGER_H
#define	_ENTITY_PURGATORYMANAGER_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"

#include "game/gameinfo.h"

class CMeshInstance;
class BasicCameraTemplate;
class CGuiScreen;

class SwordCheckpointInfoDef
{
public:
	enum CHECKPOINT_MODE
	{
		SELECTED,
		AVAILABLE,
		LOCKED,
		MAX_MODES
	};

public:
	SwordCheckpointInfoDef(const CheckpointInfoDef* pobDef, CMeshInstance* pobSelected, CMeshInstance* pobAvailable, CMeshInstance* pobLocked);
	~SwordCheckpointInfoDef();

	void SetMode(CHECKPOINT_MODE eMode);
	CHECKPOINT_MODE Mode() const { return m_eMode; }
	int CheckpointNumber() const { return m_iCheckpointNumber; }

private:
	void ShowMesh(CMeshInstance* pobMesh);
	void HideMesh(CMeshInstance* pobMesh);

	CHECKPOINT_MODE m_eMode;
    int m_iCheckpointNumber;

	CMeshInstance* m_apobMeshes[MAX_MODES];
};

class SwordInfoDef
{
	// Declare dataobject interface
	HAS_INTERFACE(SwordInfoDef);
public:
	enum SWORD_MODE
	{
		LOCKED,			// Sword will stay underground
		UNLOCKED,		// Sword will animate this time, then go AVAILABLE
		AVAILABLE		// Sword was previous unlocked. us static mesh
	};

	enum SELECT_CHECKPOINT
	{
		NEXT_CHECKPOINT,
		PREVIOUS_CHECKPOINT,
		NEWEST_CHECKPOINT,
		FIRST_CHECKPOINT,
	};

public:
	SwordInfoDef();
	~SwordInfoDef();

	int ChapterNumber() const { return m_iChapterNumber; }
	char SwordChar() const { return m_obSwordChar[0]; }
	int CheckpointIndexOffset() const { return m_iCheckpointIndexOffset; }
	BasicCameraTemplate* Camera() const { return m_pobCamera; }

	SwordInfoDef* NextSword() { return m_pobNextSword; }
	SwordInfoDef* PreviousSword() { return m_pobPreviousSword; }

	void SetMode(SWORD_MODE eMode, bool bUpdateVisibility = true);
	SWORD_MODE Mode() const { return m_eMode; }

	CEntity* Sword() { ntAssert(m_pobSword); return m_pobSword; }

	void SetupCheckpoints(const ChapterInfoDef* pobChap);

	bool SelectCheckpoint(SELECT_CHECKPOINT eCheckpoint);
	int SelectedCheckpoint();
private:

	SwordCheckpointInfoDef* SetupCheckpointInfo(const CheckpointInfoDef* pobCP);

	//exposed variables
    int m_iChapterNumber;
    ntstd::String m_obSwordChar;			/// sorry :( really didnt want this
    int m_iCheckpointIndexOffset;			/// sorry :( really didnt want this

	BasicCameraTemplate* m_pobCamera;

	CEntity* m_pobSword;

	SwordInfoDef* m_pobNextSword;
	SwordInfoDef* m_pobPreviousSword;

	//internal
	SWORD_MODE m_eMode;

	typedef ntstd::List<SwordCheckpointInfoDef*, Mem::MC_ENTITY> CheckpointList;
	CheckpointList m_obCheckpoints;
	CheckpointList::iterator m_obCurrentCheckpoint;
};

//--------------------------------------------------
//!
//! Class Object_PurgatoryManager.
//! Manages the purgatory level
//!
//--------------------------------------------------
class Object_PurgatoryManager : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_PurgatoryManager)

public:

	enum CAMERA
	{
		INVALID_CAMERA,
		MAINMENU_CAMERA,
		OPTIONS_CAMERA,
		SPECIALFEATURES_CAMERA,
		CHAPTERSELECT_CAMERA
	};

	enum SWITCH_CHAPTER
	{
		NEXT_CHAPTER,
		PREVIOUS_CHAPTER
	};

	enum SWITCH_CHECKPOINT
	{
		NEXT_CHECKPOINT,
		PREVIOUS_CHECKPOINT
	};

public:
	// Constructor
	Object_PurgatoryManager();

	// Destructor
	~Object_PurgatoryManager();

	// Post Construct
	void OnPostConstruct();

	void SetupInitialState();

	//void SetCameraStyle(CHashedString obStyle);

	//Camera switching
	void SwitchToCamera(CAMERA eCamera);

	// return of false means it cannot be done at this time
	bool SwitchChapter(SWITCH_CHAPTER eChapter);

	// return of false means it cannot be done at this time
	bool SwitchCheckpoint(SWITCH_CHECKPOINT eCheckpoint);

	// Cutscene control
	bool UpdateCutscene() { return false; }

	// Sword control
	bool UpdateSwordRaise() { return false; }

	//get the id's of the currently selected chapter and checkpoint
	int CurrentSelectedChapter() { ntAssert(m_pobCurrentSword); return m_pobCurrentSword->ChapterNumber(); }
	int CurrentSelectedCheckpoint() { ntAssert(m_pobCurrentSword); return m_pobCurrentSword->SelectedCheckpoint(); }

	bool UpdateTransitions();

	CAMERA TargetLocation() { return m_eTargetLocation; }

	void NotifyScreenEnter(CGuiScreen* pobScreen, CHashedString obStyle);

	void TriggerCutscene(int iChapterCompleted);
	bool TriggerSwordAnim(int iChapterUnlocked);
	bool SwordAnimComplete();
	void OnCutsceneComplete();

	void ChangeCameraStyle();

	bool DelayedSwordAnim() {	return m_bDelayedSwordAnim;	}
    void TriggerDelayedSwordAnim(int iChapterUnlocked);

	void SetupPurgatoryMenu(CGuiScreen* pobScreen);

protected:

	void BlockGuiInput(bool bBlock);

    void MoveToInitialScreen();

	CAMERA StringToCameraStyle(CHashedString obStyle);
	void RequestCameraStyleChange(CAMERA eCamera);

	SwordInfoDef* LocateSword(int iChapter);

private:
	BasicCameraTemplate* m_pobMenuCamera;
	BasicCameraTemplate* m_pobOptionsCamera;
	BasicCameraTemplate* m_pobSpecialFeaturesCamera;

	typedef ntstd::List<SwordInfoDef*, Mem::MC_ENTITY> SwordList;
	SwordList m_obSwords;
	SwordInfoDef* m_pobCurrentSword;
	SwordInfoDef* m_pobCutsceneSword;

	GameInfoDef* m_pobGameInfo;

	CAMERA m_eTargetLocation;

	CGuiScreen* m_pobTargetScreen;

	bool m_bTransitionActive;

	bool m_bDelayedSwordAnim;
	bool m_bSetupMenuRequired;
	int	m_iChapterSelectTransitionGracePeriod;
};

LV_DECLARE_USERDATA(Object_PurgatoryManager);

#endif // _ENTITY_PURGATORYMANAGER_H
