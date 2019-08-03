/***************************************************************************************************
*
*	FILE			ChatterBox.h
*
*	DESCRIPTION	
*
*	AUTHOR: Dario L. Sancho-Pradel		
*
***************************************************************************************************/

#ifndef _CHATTERBOX_H
#define _CHATTERBOX_H

//**************************************************************************************
//	Includes files.
//**************************************************************************************

#include "game/entity.h"
#include "objectdatabase/dataobject.h"
#include "game/entityinfo.h"
#include "game/randmanager.h"
#include "editable/enumlist.h"
#include "game/attacks.h"

//**************************************************************************************
//	Defines
//**************************************************************************************

#define DEBUG_PRINT_CHATTERBOX_ON (g_ShellOptions->m_bChatterBoxPrintDebug)
#define DEBUG_PRINT_CHATTERBOX(condition_msg) if (g_ShellOptions->m_bChatterBoxPrintDebug) { Debug::Printf("CB: "); Debug::Printf condition_msg; }

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************
class CAnimEventSound;

typedef struct _SChatTrigger SChatTrigger;
typedef struct _SChatItem SChatItem;
typedef struct _SCallBackData SCallBackData;
typedef struct _SCallBackBankedData SCallBackBankedData;
typedef struct _SSubChatItemBank SSubChatItemBank;
typedef struct _SScheduledChatItem SScheduledChatItem;
typedef struct _SCallBackAnimEvent SCallBackAnimEvent;
typedef struct _SChatterBoxStatisticPair SChatterBoxStatisticPair;

typedef ntstd::List<SChatterBoxStatisticPair* , Mem::MC_MISC>	CBStatsList;

/***************************************************************************************************
*
*	CLASS			CChatterBox
*
*	DESCRIPTION		C++ porting of the LUA Helper Functions found in chatterbox2.lua
*
***************************************************************************************************/

class CChatterBox
{
	public:
	
		HAS_INTERFACE(CChatterBox)

		//
		//	Ctor, dtor.
		//
		CChatterBox();
		~CChatterBox();

	public:
		//
		//	For debug checking.
		//
		void	PostConstruct	();

	public:
		// 
		//	Accessors
		//
		bool				TriggerAnimEventSound	( CEntity*, CAnimEventSound* );

		bool				StartChatting			(const char*, CEntity* pEntSource);
	//	void				SetStartPhrase			(SChatItem* pSCI) { m_pStartPhrase = pSCI; }
		bool				IsParticipantInList		(const CEntity* pobParticipant) const;
		bool				RemoveParticipant		(const CEntity* pobParticipant);
		bool				AddParticipant			(CEntity* pobParticipant);
		bool				Activate				(bool = true);
		bool				Deactivate				(void);
		const char*			GetName					(void) const { return m_pcName; }
		const CHashedString	GetNameSubChatterBox	(void) const { return m_pcNameSubChatterBox; }
		int					GetTransitionSize		(void) const { return m_iTransitionSize;}
		int					GetParticipantsListSize	(void) const { return m_listParticipants.size(); }
		bool				IsChatting				(void) {return m_Chatting;}
		bool				IsActive				(void) {return m_Active;}
		bool				IsEventSupported		(const char* pcEventName) const;
		bool				IsInitialChatterBox		(void) const { return m_bInitialChatterBox; }
		void				SetChattingStatus		(bool bStatus) { m_Chatting = bStatus; }
		bool				PlayResponse			(SChatItem* pPreviousSentence);
		bool				AssignBankToParticipant (CEntity* pParticipant);
		bool				DoIUseBankedPhrases		(void) const { return m_bUseBankedPhrases; }
		void				LevelUnload				(void) ;
		unsigned int		GetBankedResponsesLimit	(void) { return ( m_uiMaxNumberOfBankedResponses); }
		void				SetBankedResponsesLimit	(unsigned int n) { m_uiMaxNumberOfBankedResponses=n; }
		
		bool				Schedule				( SChatItem*, CEntity* );
		bool				HasScheduledChatItems	( void ) const	{ return m_bHasScheduledChatItems; }
		void				SetHasScheduledChatItems( bool b )		{ m_bHasScheduledChatItems = b; }
		bool				IsPlayingAnimEventSound	( void ) const	{ return m_bPlayingAnimEventSound; }
		void				SetPlayingAnimEventSound( bool b ) 		{ m_bPlayingAnimEventSound = b; }
		void				StopChats				( void );
		void				StopParticipantAudio	( CEntity* );
		void				Update					( float );
		
		bool				IsSafeToUseEntityAsChatter( CEntity* );

		bool				IsGeneric				( void ) const	{ return m_bIsGeneric; }
		void				UpdateChatGroupParticipantsVector ( int );
		void				CopyParticipantsIntoGenericChatterBoxes ( void );
		bool				HasValidChatGUID		( void );
		unsigned int		GetChatGUID				( void ) const { return m_uiChatGUID; }
		
		const ntstd::List<CChatterBox*>* GetGenericChatterBoxes ( void ) const { return &m_listGenericChatterBoxes; }
		unsigned int		GetNumberOfGenericChatterBoxes ( void ) const { return m_listGenericChatterBoxes.size(); }


		const ntstd::List<CEntity*>*		GetParticipantsList		(void) const	{ return &m_listParticipants; }
		const ntstd::List<SChatTrigger*>*	GetTriggerList			(void) const	{ return &m_Triggers; }
		SChatTrigger*						GetChatTrigger			(const char* pcEventName, CChatterBox** );
		SChatTrigger*						GetActiveChatTrigger	(void)			{ return m_pActiveTrigger; }
		void								SetActiveChatTrigger	( SChatTrigger* pSCT ) { m_pActiveTrigger = pSCT; }

		// Statistics
		int									GetStatistic			(CHashedString, bool*) const;
		bool								SetStatistic			(CHashedString, int);
		const CBStatsList*					GetStatisticsList_const	(void) const	{ return &m_listStatistics; }
		CBStatsList*						GetStatisticsList		(void)			{ return &m_listStatistics; }
		void								ResetStatistics			(void);

		//
		void								DisableChecksOnAI			( CEntity*, bool );
		bool								HasChatterBoxChecksDisabled	( CEntity* );

		
		// For DEBUG ONLY 
		const CEntity*						GetSource			(void) const { return m_pSource; }
		const CEntity*						GetLastChatter		(void) const { return m_pLastChatter; }

	private:

		SChatItem*		PickStartPhrase					(const char* pcEventName);
		CEntity*		PickChatter						(SChatItem* , bool* );
		SChatItem*		PickResponse					(SChatItem* pPreviousSentence);

		SChatItem*		GetAnimEventChatItem			(CHashedString ksSfx);
	
		// For Banked Phrases - SubChatterBox -

	public:

		bool PlayBankedPhrase		(const char*, bool);
		bool StartBankedChatting	(const char*, CEntity*);

	private:
		
		static const int MAX_NUMBER_OF_BANKED_RESPONSES = 5;

		bool				DeleteParticipantBank	(CEntity* pParticipant);
		SSubChatItemBank*	GetParticipantBank		(const char* pcEventName, CEntity* pParticipant);
		SChatItem*			PickBankedPhrase		(SSubChatItemBank* pParticipantBank);		
		
	private:

		ntstd::List<SChatTrigger*>			m_Triggers;
		ntstd::List<CChatterBox*>			m_listGenericChatterBoxes;
		ntstd::List<CEntity*>				m_listParticipants;
		ntstd::List<CEntity*>				m_listNoChecksParticipants;
		ntstd::Vector<CEntity*>				m_vectorChatGroupParticipants;
		ntstd::List<SChatItem*>				m_listAnimEventChatItems;
		ntstd::List<SScheduledChatItem*>	m_listScheduledChatItems;
		CBStatsList							m_listStatistics;
		CEntity*							m_pLastChatter;
		CEntity*							m_pSource;
		char								m_pcName[128];
		CHashedString						m_pcNameSubChatterBox;
		int									m_iTransitionSize;
		CHashedString						m_ConstructionScript;
		CHashedString						m_NextChatterBox;
		int									m_SectorBits;
		int									m_DeathLimit;
		unsigned int						m_uiMaxNumberOfBankedResponses;
		bool								m_Active;
		bool								m_Chatting;
		bool								m_bInitialChatterBox;
		bool								m_bUseBankedPhrases;
		bool								m_bHasScheduledChatItems;
		bool								m_bPlayingAnimEventSound;
		SChatTrigger*						m_pActiveTrigger;
		bool								m_bIsGeneric;
		unsigned int						m_uiChatGUID;
				
	private:

		SCallBackData*				m_CallBackData;
		SCallBackBankedData*		m_CallBackBankedData;
		SCallBackAnimEvent*			m_CallBackAnimEvent;
};

/***************************************************************************************************
*
*	STRUCTRUE			SChatTrigger
*
*	DESCRIPTION		
*
***************************************************************************************************/

typedef struct _SChatTrigger
{
	_SChatTrigger() :	m_pPreviousPhrase(NULL), m_uiCountThreshold(0), m_uiCurrentCount(0), m_fTimeThreshold(0.0f), m_fCurrentTime(0.0f),
						m_bTimeThresholdSurpassed(true), m_uiPriority(1) {}

	CKeyString						m_TriggerID;			// EventID 
	ntstd::List<SChatItem*>			m_listInitialPhrases;	// List of Initial Phrases
	SChatItem*						m_pPreviousPhrase;		// Previously used initial phrase
	ntstd::List<SSubChatItemBank*>	m_listParticipantsBanks;// List of Different Bank Phrases
	unsigned int					m_uiCountThreshold;		// Number of times a trigger needs to be called before being executed
	unsigned int					m_uiCurrentCount;		// 
	float							m_fTimeThreshold;		// Minimum time between trigger executions
	float							m_fCurrentTime;			// 
	bool							m_bTimeThresholdSurpassed;
	unsigned int					m_uiPriority;			// 1 - Max (No interruption), 2 - can be interrupted by 1, etc...


} SChatTrigger;

/***************************************************************************************************
*
*	STRUCTRUE		SChatItem
*
*	DESCRIPTION		
*
***************************************************************************************************/

typedef struct _SChatItem
{
	_SChatItem() : m_Bank("aivo_sb"), m_pSpeaker(0), m_Delay(0.0f), m_bOverlapsNext(false), m_fCheeringDelayMin(0.0f), m_fCheeringDelayMax(0.0f),
				   m_fCurrentTime(0.0f), m_bHasSubtitles(false), m_fProbability(1.0f), m_iUseChatGroup(0) {}

	CKeyString					m_Sfx;			// Sound Effect 
	CKeyString					m_Bank;			// Sound Bank
	ntstd::List<_SChatItem*>	m_listResponses;	// List of nested replys
	_SChatItem*					m_pPreviousResponse; // Previously chosen response
	CEntity*					m_pSpeaker;		// Entity selected to talk
	float						m_Delay;		// Delay until the sound is played (this should not be exposed in XML)
	//int							m_Crowd;		// ???
	
	// Cheering and Jeering
	bool						m_bOverlapsNext;
	float						m_fCheeringDelayMin;
	float						m_fCheeringDelayMax;	
	float						m_fCurrentTime;
	// Subtitles
	bool						m_bHasSubtitles;
	// Probability
	float						m_fProbability;
	// Chat Group
	int							m_iUseChatGroup;
	// Animation
	CHashedString				m_hsAnimation;
} SChatItem;

/***************************************************************************************************
*
*	STRUCTRUE		SScheduledChatItem
*
*	DESCRIPTION		
*
***************************************************************************************************/

typedef struct _SScheduledChatItem
{
	_SScheduledChatItem() : m_pSChatItem(NULL), m_pSpeaker(NULL), m_Delay(0.f), m_fCurrentTime(0.0f) {}

	_SScheduledChatItem (SChatItem* pSCI, CEntity* pSpk, float fDelay) :	m_pSChatItem(pSCI), m_pSpeaker(pSpk), m_Delay(fDelay), 
																			m_fCurrentTime(0.0f) {}

	SChatItem*		m_pSChatItem;
	CEntity*		m_pSpeaker;
	float			m_Delay;	
	float			m_fCurrentTime;
} SScheduledChatItem;

/***************************************************************************************************
*
*	STRUCTRUE			SCallBackData
*
*	DESCRIPTION		
*
***************************************************************************************************/
typedef struct _SCallBackData
{
	CChatterBox*	pChatterBox;		// This Class
	SChatItem*		pCurrentSentence;	// Current sentence
} SCallBackData;

/***************************************************************************************************
*
*	STRUCTRUE			SCallBackBankedData
*
*	DESCRIPTION		
*
***************************************************************************************************/
typedef struct _SCallBackBankedData
{
	CChatterBox*	pChatterBox;	// This Class
	const char*		pTriggerID;		// Event
	int				iNrOfResponses; // Random Number of Responses
} SCallBackBankedData;

/***************************************************************************************************
*
*	STRUCTRUE			SCallBackBankedData
*
*	DESCRIPTION		
*
***************************************************************************************************/
typedef struct _SCallBackAnimEvent
{
	CChatterBox*	pChatterBox;	// This Class
} SCallBackAnimEvent;

/***************************************************************************************************
*
*	STRUCTRUE			SSubChatItemBank
*
*	DESCRIPTION		
*
***************************************************************************************************/
typedef struct _SSubChatItemBank
{
	_SSubChatItemBank() : pLastPhrase(NULL), pParticipant(NULL) {}
	ntstd::List<SChatItem*>	listBankedChatItems;	// List of SChatItem for this bank
	SChatItem*				pLastPhrase;			// Last Response from this bank
	CEntity*				pParticipant;		// Name of the participant who owns the bank
	//int						index;					// Bank index
} SSubChatItemBank;

/***************************************************************************************************
*
*	STRUCTRUE			SChatterBoxStatisticPair
*
*	DESCRIPTION		
*
***************************************************************************************************/
typedef struct _SChatterBoxStatisticPair
{
	CHashedString hsTriggerEvent;
	int uiCount;
} SChatterBoxStatisticPair;

#endif // _CHATTERBOX_H
