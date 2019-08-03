//! ------------------------------------------------------
//! aiuseobjectqueueman.h
//!
//! Queuing Manager for safe usage of objects (ladders...)
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!-------------------------------------------------------

#ifndef _AIQUEUINGMAN_H
#define _AIQUEUINGMAN_H

#include "ai/ainavigationsystem/ainavignode.h"

class CEntity;
class AI;
class CAINavigNode;
class CAIQueueOjectFolder;

typedef ntstd::List<AI*		, Mem::MC_AI>	AIList;

//!--------------------------------------------
//! CAIQueue
//!--------------------------------------------

class CAISingleQueue
{
	public:

		CAISingleQueue() : m_pObjectToUse(NULL), m_fTimer(0.0f), m_hsName("NO_NAME"), m_pFolder(NULL) {}
		CAISingleQueue( CAIQueueOjectFolder* pF, CEntity* pObj, unsigned int sz ) : 
				m_pObjectToUse(pObj), m_fTimer(0.0f), m_hsName("NO_NAME"), m_pFolder(pF) 
				{ if (sz>0) {m_QNodeVector.reserve(sz); } }

		CEntity*	GetObject			( void				) const { return m_pObjectToUse; }
	
		void		UpdateQueueIndexes	( void );	
		void		UpdateTimer			( float fTimeChange ) { if (!m_AIList.empty()) m_fTimer+=fTimeChange; }

		void			push_back				( CAINavigNode* pNN ) { if (pNN) { m_QNodeVector.push_back(pNN); } }
		bool			empty					( void ) const	{ return (m_AIList.empty()); }
		unsigned int	GetMaxSize				( void ) const	{ return (m_QNodeVector.size()); }
		unsigned int	GetCurrentQueueSize		( void ) const	{ return (m_AIList.size()); }
		CPoint			GetQueuePointAndRadius	( int , float*, bool* );
		void			AddUser					( AI* pAI )		{ m_AIList.push_back(pAI); }
		bool			RemoveAI				( AI* pAI			);
		AI*				GetAIAtHead				( void );
		CAINavigNode*	GetNodeAtHead			( void ) { if (!m_QNodeVector.empty()) return m_QNodeVector[0]; else return NULL; }
		CHashedString	GetName					( void ) const { return m_hsName; }
		void			SetName					( const CHashedString phs ) { m_hsName = phs; } 
		
		CAIQueueOjectFolder*	GetFolder		( void ) { return m_pFolder; }
		void					SetFolder		( CAIQueueOjectFolder* pF ) { if (pF) {m_pFolder = pF;} }
		AI*						GetAIQueueingInNode ( CAINavigNode* );
			
	private:

		AINavigNodeVector		m_QNodeVector;
		AIList					m_AIList;
		CEntity*				m_pObjectToUse;
		float					m_fTimer;
		CHashedString			m_hsName;
		CAIQueueOjectFolder*	m_pFolder;

};
typedef ntstd::List<CAISingleQueue*, Mem::MC_AI> SingleQueueList;

//!--------------------------------------------
//! CAIHearingMan
//!--------------------------------------------

class CAIQueueOjectFolder
{
	public:

		friend class CAIQueueManager;

		CAIQueueOjectFolder() : m_pObjectToUse(NULL), m_fTimer(0.0f) {}
		CAIQueueOjectFolder( CEntity* pObj ) : m_pObjectToUse(pObj), m_fTimer(0.0f) {}
		~CAIQueueOjectFolder();
		void FreeDynamicData	(void);

		CEntity*	GetObject			( void				) const { return m_pObjectToUse; }

		void		AddQNode			( CAINavigNode* pQN	)		{ if (!IsQNodeRegistered(pQN))	{m_QNodeList.push_back(pQN);} }
//		void		AddUser				( AI* pAI			)		{ if (!IsUserRegistered(pAI))	{m_AIList.push_back(pAI);} }
//		bool		RemoveAI			( AI* pAI			);
		
		void		UpdateQueueIndexes	( void );	

		void		UpdateTimer			( float fTimeChange ) { if (!m_AIList.empty()) m_fTimer+=fTimeChange; }

		bool		IsQNodeRegistered	( CAINavigNode* );
		bool		IsUserRegistered	( AI* );

		CPoint			GetQueuePointAndRadius	( AI*, float*, bool* );
		void			AddSingleQueue		( CAISingleQueue* pSQ ) { if (pSQ) { m_SingleQueueList.push_back(pSQ); } }
		CAISingleQueue*	GetMostEmptyQueue	( void ); 
		CAISingleQueue*	GetMostBusyQueue	( void );

		AI*				GetAIQueueingInNode	( CAINavigNode* );

		
	private:

		SingleQueueList	m_SingleQueueList;
		AINavigNodeList m_QNodeList;
		AIList			m_AIList;
		CEntity*		m_pObjectToUse;
		float			m_fTimer;

};

typedef ntstd::List<CAIQueueOjectFolder*, Mem::MC_AI> QueueFoldersList;

//!--------------------------------------------
//! CAIHearingMan
//!--------------------------------------------
class CAIQueueManager : public Singleton<CAIQueueManager>
{
	public:
		
		~CAIQueueManager();

		void		RegisterQueueNode			( CEntity*, CAINavigNode* );
		void		RegisterQueueGraph			( CEntity*, AINavigNodeList* );

		bool		RegisterUser				( AI*, CEntity*);
		void		ReportObjectUsed			( AI* );
		void		ReportObjectAvailable		( AI* );
		void		ReportNewQueuePointReached	( AI* );

//		CPoint		GetAIQueuingPoint			( AI* );
//		float		GetAIQueuingPointRadius		( AI* );
//		int			GetAIQueueIndex				( AI* );
		
		bool		IsQueueIndexUpdated			( AI* );
		bool		IsQueuingNeeded				( AI* );
		bool		IsQueuing					( AI* );

		AI*			GetFirstOnTheQueue			( CEntity* );

		CPoint		GetQueuePointAndRadius	( AI*, float*, bool* );
		
		CAIQueueOjectFolder* GetFolder		( CEntity* );

	
		// Update ( for DebugRender Only )
//		void DebugRender			( void );
		void Update					( float );	

		bool	m_bDebugRender;
	
	private:

		CAIQueueOjectFolder* CreateFolder	( CEntity* );
//		CAIQueueOjectFolder* GetFolder		( AI* );


	private:
		ntstd::List<CAIQueueOjectFolder*, Mem::MC_AI>	m_QueueFoldersList;
		AIList											m_AIGlobalList;
};


#endif //_AIQUEUINGMAN_H




