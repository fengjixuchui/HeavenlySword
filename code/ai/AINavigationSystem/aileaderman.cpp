//! -------------------------------------------
//! AILeaderMan.cpp
//!
//! AI Leader Manager
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aileaderman.h"
#include "ainavigsystemman.h"	// For SetLeader()

//! -------------------------------------------
//! Ctor, et. al
//! -------------------------------------------

CAILeaderMan::~CAILeaderMan()
{
	// Free Dynamic Data ( Leaders' Folders )

	SLeaderFolderList::const_iterator obIt	= m_listLeadersFolders.begin();
	SLeaderFolderList::const_iterator obEndIt	= m_listLeadersFolders.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		NT_DELETE_CHUNK( Mem::MC_AI, (*obIt) );
	}
}

//! -------------------------------------------
//! IsLeader
//! -------------------------------------------
bool CAILeaderMan::IsLeader ( AI* pEnt )
{
	if (!pEnt) return false;
	
	SLeaderFolderList::const_iterator obIt	= m_listLeadersFolders.begin();
	SLeaderFolderList::const_iterator obEndIt	= m_listLeadersFolders.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt)->pLeader == pEnt )
		{
			return true;
		}
	}
	return false;
}

//! -------------------------------------------
//! GetLeader
//! -------------------------------------------
AI* CAILeaderMan::GetLeader( AI* pEntF )
{
	if (!pEntF) return NULL;
	
	// Loop through the list of leaders
	SLeaderFolderList::const_iterator obItLFolder		= m_listLeadersFolders.begin();
	SLeaderFolderList::const_iterator obItEndLFolder	= m_listLeadersFolders.end();
	for( ; obItLFolder != obItEndLFolder; ++obItLFolder )
	{
		// Loop thorugh the list of followers of each leader
		ntstd::List<AI*>::const_iterator obItF		= (*obItLFolder)->listpFollowers.begin();
		ntstd::List<AI*>::const_iterator obEndItF	= (*obItLFolder)->listpFollowers.end();
		for( ; obItF != obEndItF; ++obItF )
		{
			if ( (*obItF) == pEntF )
			{
				return (*obItLFolder)->pLeader;
			}
		}
	}
	return NULL;
}

//! -------------------------------------------
//! IsInLeadersFolder
//! -------------------------------------------
bool CAILeaderMan::IsInLeadersFolder ( AI* pEntF, SLeaderFolder* pLF )
{
	if ( !pEntF ) return false;

	ntstd::List<AI*>::const_iterator obIt		= pLF->listpFollowers.begin();
	ntstd::List<AI*>::const_iterator obEndIt	= pLF->listpFollowers.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt) == pEntF )
		{
			return true;
		}
	}
	return false;
}

//! -------------------------------------------
//! GetLeadersFolder
//! -------------------------------------------
SLeaderFolder* CAILeaderMan::GetLeadersFolder ( CEntity * pEntL )
{
	if ( !pEntL ) return NULL;

	SLeaderFolderList::const_iterator obIt	= m_listLeadersFolders.begin();
	SLeaderFolderList::const_iterator obEndIt	= m_listLeadersFolders.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt)->pLeader == pEntL )
		{
			return (*obIt);
		}
	}
	return NULL;
}

//! -------------------------------------------
//! DeleteFollowerFromLeadersList
//! -------------------------------------------
bool CAILeaderMan::DeleteFollowerFromLeadersList ( AI* pEntF, AI* pEntL )
{
	if (!pEntF || !pEntL) return false;
	
	SLeaderFolder* pLF = GetLeadersFolder(pEntL);

	ntstd::List<AI*>::iterator obIt	= pLF->listpFollowers.begin();
	ntstd::List<AI*>::iterator obEndIt	= pLF->listpFollowers.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt) == pEntF )
		{
			pLF->listpFollowers.erase(obIt);
			return true;
		}
	}
	// !!! - (Dario) What to do here? The follower does not exist in the Leaders Folder
	return true;
}

//! -------------------------------------------
//! SetLeader
//! -------------------------------------------
bool CAILeaderMan::SetLeader ( AI* pEntF, AI* pEntL )
{
	if (!pEntF || !pEntL) return false;
	if ( pEntF == pEntL ) return false;

	AI* pCurrentLeader = GetLeader(pEntF);							// Get Follower's current Leader
	pEntF->GetAIComponent()->GetCAIMovement()->SetLeader(pEntL);		// Assign the Leader to the Followers' CAIMovement 

	// If the Follower has a current Leader -> Remove the Followers from his current Leader's Folder
	if ( pCurrentLeader ) DeleteFollowerFromLeadersList	( pEntF, pCurrentLeader );
	
	// Add the Follower to the new Leader's Folder

	if ( ! IsLeader(pEntL) )
	{
		// The Leader is new (i.e. does not have any followers)
		SLeaderFolder* pLF	= NT_NEW_CHUNK( Mem::MC_AI ) SLeaderFolder(pEntL);				// Create a new Leader's Folder
		pLF->listpFollowers.push_back(pEntF);							// Add the Follower to the Leader's List
		m_listLeadersFolders.push_back(pLF);							// Add the new Folder
		return true;
	}
	else
	{
		// The Leader exists -> Add the Follower to the Leader's Followers List
		SLeaderFolder* pLF = GetLeadersFolder(pEntL);
		if ( pLF && !IsInLeadersFolder (pEntF , pLF) )	
		{
			// Just check that the follower it does not already exist in the folder
			pLF->listpFollowers.push_back(pEntF);
			return true;
		}
	}
	return false;
}

//! -------------------------------------------
//! RemoveLeader
//! -------------------------------------------
void CAILeaderMan::RemoveLeader ( AI* pEntL )
{
	if ( !pEntL ) return;

	SLeaderFolder* pLF = GetLeadersFolder(pEntL);

	if (!pLF) return;

	// We need to:
	// (1) - Select a NEW LEADER from his followers list
	if ( !pLF->listpFollowers.empty() )
	{
		// Get the first follower as new leader
		ntstd::List<AI*>::const_iterator obIt		= pLF->listpFollowers.begin();
		ntstd::List<AI*>::const_iterator obEndIt	= pLF->listpFollowers.end();

		// New Leader
		AI* pNewLeader = (*obIt);
		// Take the old leader's intentions
		SDestination* pD = pEntL->GetAIComponent()->GetCAIMovement()->GetDestination();
		pNewLeader->GetAIComponent()->GetCAIMovement()->SetDestination(pD);
		// If new leader must get to a node, make a path to it
		CAINavigationSystemMan::Get().FollowPathTo(pNewLeader,pD->pNode,pNewLeader->GetAIComponent()->GetCAIMovement()->GetMaxSpeed());

		// (2) - Create a new Folder for the new leader
		SLeaderFolder* pNewLF	= NT_NEW_CHUNK( Mem::MC_AI ) SLeaderFolder(pNewLeader);
		// (3) - Add followers to the leader's folder and reasign the new leader to the rest of the followers
		obIt++; // Skip the first follower since it is the new leader.
		for( ; obIt != obEndIt; ++obIt )
		{
			CAIMovement* pMov = (*obIt)->GetAIComponent()->GetCAIMovement();
			pNewLF->listpFollowers.push_back(*obIt);							// Add Followers to folder
			pMov->SetLeader(pNewLeader);										// Reasign the new leader to the follower
		}
	}

	// (4) Remove the old leader's folder from the list
	SLeaderFolderList::iterator obIt		= m_listLeadersFolders.begin();
	SLeaderFolderList::iterator obEndIt	= m_listLeadersFolders.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt) == pLF )
		{
			m_listLeadersFolders.erase(obIt);
			break;
		}
	}

	// (5) Delete the old Leader's Folder
	NT_DELETE_CHUNK( Mem::MC_AI, pLF);
}

