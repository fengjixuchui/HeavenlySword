/***************************************************************************************************
*
*	FILE			ChatterBoxMan.h
*
*	DESCRIPTION		
*
*	AUTHOR: Dario L. Sancho-Pradel
*
***************************************************************************************************/

#ifndef _CHATTERBOXMAN_H
#define _CHATTERBOXMAN_H

#ifndef GAME_SHELLCONFIG_H
#include "game/shellconfig.h"
#endif

//	Includes files

#include "game/ChatterBox.h"
#include "lua/ninjalua.h"


#ifndef _RELEASE

#define _CHATTERBOX_DEBUG_RENDER

#endif // _RELEASE

#define IS_AUDIO_ENGINE_ON (g_ShellOptions->m_bAudioEngine)

//	Forward declarations.

class CEntity;
class CAnimEventSound;

/***************************************************************************************************
*
*	CLASS			CChatterBoxMan
*
*	DESCRIPTION		C++ porting of the LUA ChatterBox Manager found in chatterbox2.lua
*
***************************************************************************************************/

class CChatterBoxMan : public Singleton<CChatterBoxMan>
{
	public:
	//static void Register();
	
		//
		//	Ctor, dtor.
		//
		CChatterBoxMan();
		~CChatterBoxMan();

		HAS_LUA_INTERFACE()

	public:
		//
		//	For debug checking.
		//
		void	PostConstruct	();

	public:

		//	Accessors

		bool TriggerAnimEventSound				( CEntity*, CAnimEventSound* );

		bool Trigger							(const char* pcEventName, CEntity* pobEntSource); // Triggers an {Event,source}
		bool IsEnabled							(void) const { return m_Enabled;}				// Is Enabled the CBM
		void Enable								(void)			{ m_Enabled=true && IS_AUDIO_ENGINE_ON; } // Enables CBM
		void Disable							(void)			{ m_Enabled=false; }			// Disables CBM
		bool IsChatterBoxActive					(const char* strNameCB) const {return (this->GetChatterBox(strNameCB)->IsActive()); }
		bool ActivateChatterBox					(const char* strNameCB, bool = true);
		bool AddParticipantToChatterBox			(CEntity* pobParticipant, const char* strNameCB );
		bool RemoveParticipantFromChatterBox	(const CEntity* pobParticipant, const char* strNameCB);  // False if fails
		bool AddParticipant						(CEntity* pobParticipant);
		bool RemoveParticipant					(const CEntity* pobParticipant);
		bool AddChatterBox						(CChatterBox *);
		bool RemoveChatterBox					(CChatterBox *);
		void LevelUnload						(void);
		unsigned int GetBankedResponsesLimit	(void)			 { if (m_ActiveChatterBox) { return (m_ActiveChatterBox->GetBankedResponsesLimit()); } }
		void SetBankedResponsesLimit			(unsigned int n) { if (m_ActiveChatterBox) { m_ActiveChatterBox->SetBankedResponsesLimit(n); } } 

		void StopCurrentChats					(void);
		void StopParticipantAudio				( CEntity* );
		void SetCurrentChatGUID					( unsigned int ui ) { m_uiCurrentChatGUID = ui; }
		unsigned int GetCurrentChatGUID			( void ) const		{ return m_uiCurrentChatGUID; }

		bool ActivateSubChatterBox				( CHashedString );
		CChatterBox* GetActiveChatterBox		( void ) const		{ return m_ActiveChatterBox; }
		unsigned int GetNumberOfStandardChatterBoxes	( void ) const		{ return m_ChatterBoxes.size(); }
		unsigned int GetNumberOfGenericChatterBoxes		( void ) const		{ return m_GenericChatterBoxes.size(); }

		CChatterBox* GetLastPlayingChatterBox	( void ) const		{ return m_LastPlayingChatterBox; }
		void		 SetChatGroupID				( CEntity*, int );	

		// Statistics
		int GetCurrentStatistic					( CHashedString, bool* ) const;

		void DisableChecksOnAI					( CEntity*, bool );
		bool HasChatterBoxChecksDisabled		( CEntity* );

		void DbgPrint							(void); // For Debugging ONLY
		void DebugRender						(void);
		bool DebugIsEntityInCSStandadrd			( CEntity * );
		void Update								(float fTimeChange);
		COMMAND_RESULT TroggleDebugRender					(void) { m_bRender=!m_bRender; return CR_SUCCESS; }


	private:

		CChatterBox*	GetChatterBox			(const char* strNameCB, bool bAssertOn = true) const;
		bool			MoveToSubChatterBox		(CChatterBox*, bool);

		// Statistics
		void UpdateStatistics					( CHashedString );
	//	void CopyCurrentStatistic				( CChatterBox*, CChatterBox* );

	public :

		bool						m_bRender;

	private:

		ntstd::List<CChatterBox*>	m_ChatterBoxes;			// List of ChaterBoxes (CBs)
		ntstd::List<CChatterBox*>	m_GenericChatterBoxes;	// List of Generic ChaterBoxes (CBs)
		CChatterBox*				m_ActiveChatterBox;
		CChatterBox*				m_PlayingFromChatterBox;	// Chatterbox that is currently playing a chat item (i.e. ActiveCB or GeneralCB)
		CChatterBox*				m_LastPlayingChatterBox;
		bool						m_Enabled;				// ChatterBoxMan Enable Status
		unsigned int				m_uiCurrentChatGUID;

};

LV_DECLARE_USERDATA(CChatterBoxMan);


#endif // _CHATTERBOXMAN_H
