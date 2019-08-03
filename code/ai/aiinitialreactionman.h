//! ---------------------------------------------
//! aiinitialreactionman.h
//!
//! AI Initial Reactions (on player seen) Manager
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!----------------------------------------------

#ifndef _AIINITIALREACTIONMAN_H
#define _AIINITIALREACTIONMAN_H

#include "lua/ninjalua.h"

class CEntity;
class AI;

struct SAIReactionInfo
{
	SAIReactionInfo () : pAI(NULL), bFirstReporter(false), bClosestToSound(false), bFinishedSoundAnim(false), bFinishedSpotAnim(false) {}
	SAIReactionInfo (AI* pAIEnt ) : pAI(pAIEnt), bFirstReporter(false), bClosestToSound(false), bFinishedSoundAnim(false), bFinishedSpotAnim(false) {}

	AI*		pAI;
	bool	bFirstReporter;
	bool	bClosestToSound;
	bool	bFinishedSoundAnim;
	bool	bFinishedSpotAnim;
};

//!--------------------------------------------
//! CAIHearingMan
//!--------------------------------------------
class CAIInitialReactionMan : public Singleton<CAIInitialReactionMan>
{
	public:
		
		CAIInitialReactionMan();
		~CAIInitialReactionMan();

		HAS_LUA_INTERFACE();

		void		AddAI			( CEntity* );
		void		RemoveAI		( CEntity* );
		void		Report			( AI* pAI );
		void		ClearReporter	( void )			{ m_pAI_FirstReporter = NULL; }
		AI*			GetFirstReporter( void ) const		{ return m_pAI_FirstReporter; }
		bool		IsParticipant	( AI* );
		void		SetAnims		( CHashedString hsSound, CHashedString hsReport, CHashedString hsResponse ) { m_hsSoundAlerted = hsSound; m_hsReportEnemySeenAnim = hsReport;  m_hsResponseEnemySeenAnim = hsResponse; }
		
		void		SetReporterAnimFinished ( bool b )			{ m_bReporterAnimFinished = b; }
		bool		GetReporterAnimFinished ( void )	const	{ return m_bReporterAnimFinished; }

		CHashedString GetReportEnemyAnim	( void ) const	{ return m_hsReportEnemySeenAnim; }
		CHashedString GetResponseEnemyAnim	( void ) const	{ return m_hsResponseEnemySeenAnim; }
		CHashedString GetSoundAlertedAnim	( void ) const	{ return m_hsSoundAlerted; }

		CEntity*	UpdateClosestAIToSound	( const CPoint & obPos );
		CEntity*	GetClosestAIToSound		( void ) const	{ return m_pClosestEntToSound; }
		void		ResetClosestAIToSound	( void ) { m_pClosestEntToSound = NULL; }
		void		PropagateAlertSound		( CEntity* );

		//bool		IsAIFirstReporter		( AI* pAI ) const	{ return (pAI && m_pSAIFirstReporter && m_pSAIFirstReporter->pAI == pAI ); }
		bool		IsFirstReporterAlive	( void )	const;

		// Update ( for DebugRender Only )
		void DebugRender			( void );
		void Update					( float f ) { UNUSED(f); DebugRender(); }		

	public:
	
		bool m_bInitialReactionRender;

	private:

		ntstd::Vector<SAIReactionInfo, Mem::MC_AI>	m_vectorAIReactionInfo; 
		SAIReactionInfo*				m_pSAIFirstReporter;
		CEntity*						m_pClosestEntToSound;
		AI*								m_pAI_FirstReporter;
		CHashedString					m_hsReportEnemySeenAnim;
		CHashedString					m_hsResponseEnemySeenAnim;
		CHashedString					m_hsSoundAlerted;
		bool							m_bReporterAnimFinished;
};

LV_DECLARE_USERDATA(CAIInitialReactionMan);

#endif //_AIINITIALREACTIONMAN_H




