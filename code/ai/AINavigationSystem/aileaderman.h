//! -------------------------------------------
//! AILeaderMan.h
//!
//! AI Leader Manager
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AILEADERMAN_H
#define _AILEADERMAN_H

// Forward declaration of classes and structures

class CEntity;
class AI;
typedef struct _SLeaderType SLeaderFolder;
typedef ntstd::List<_SLeaderType*, Mem::MC_AI > SLeaderFolderList;

//! -------------------------------------------
//! CAILeaderMan
//! -------------------------------------------

class CAILeaderMan
{
	public:
		
		~CAILeaderMan();
		
		void		RemoveLeader	( AI*				);
		bool		SetLeader		( AI*, AI*			);
		bool		IsLeader		( AI*				);
		AI*			GetLeader		( AI*				);

	private:
		
		SLeaderFolder*	GetLeadersFolder				( CEntity *				);
		//bool			AddFollowerToLeadersList		( CEntity *, CEntity *  );
		bool			DeleteFollowerFromLeadersList	( AI*, AI*  );
		bool			IsInLeadersFolder				( AI*, SLeaderFolder*	);
	
	private:

		SLeaderFolderList	m_listLeadersFolders;

};

typedef struct _SLeaderType
{
	_SLeaderType()				: pLeader(NULL)  {}
	_SLeaderType( AI* pEntL )	: pLeader(pEntL) {}

	
	AI*					pLeader;
	ntstd::List<AI*>	listpFollowers;

} SLeaderFolder;

#endif // _AILEADERMAN_H


