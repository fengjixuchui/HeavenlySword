/***************************************************************************************************
*
*	FILE			ChatterBox.cpp
*
*	DESCRIPTION		
*
*	AUTHOR: Dario L. Sancho-Pradel	
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************

#include "game/ChatterBox.h"
#include "game/ChatterBoxMan.h"
#include "audio/gameaudiocomponents.h"
#include "game/entity.inl"
#include "anim/animevents.h"
#include "gui/guisubtitle.h"
#include "game/movement.h"


class	CEntityAudioChannel;

//**************************************************************************************
//	Expose the interface.
//**************************************************************************************
START_STD_INTERFACE(SChatterBoxStatisticPair)
	PUBLISH_VAR_WITH_DEFAULT_AS(hsTriggerEvent, "INVALID", Trigger)
	PUBLISH_VAR_WITH_DEFAULT_AS(uiCount, -1, Count)
//	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
END_STD_INTERFACE

START_STD_INTERFACE(CChatterBox)
	PUBLISH_PTR_CONTAINER_AS(m_Triggers,	 Triggers)
	PUBLISH_PTR_CONTAINER_AS(m_listParticipants, Participants)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iTransitionSize, -1, TransitionSize)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bInitialChatterBox, false, InitialChatterBox)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_pcNameSubChatterBox, "", NameSubChatterBox)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUseBankedPhrases, false, UseBankedPhrases)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bIsGeneric, false, IsGeneric)
	PUBLISH_PTR_CONTAINER_AS(m_listGenericChatterBoxes, GenericChatterBoxes)
	PUBLISH_PTR_CONTAINER_AS(m_listStatistics, StatisticsTable)
	

	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
END_STD_INTERFACE

START_STD_INTERFACE(SChatTrigger)
	PUBLISH_VAR_AS( m_TriggerID, TriggerID)
	PUBLISH_PTR_CONTAINER_AS(m_listInitialPhrases, Responses)
	PUBLISH_PTR_CONTAINER_AS(m_listParticipantsBanks, ParticipantsBanks)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_uiCountThreshold, 0, CountThreshold)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeThreshold, 0.0f, TimeThreshold)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_uiPriority, 1, TriggerPriority)

END_STD_INTERFACE

START_STD_INTERFACE(SChatItem)
	PUBLISH_VAR_AS( m_Sfx, Sfx)
	PUBLISH_VAR_AS( m_Bank, Bank)
	PUBLISH_PTR_AS( m_pSpeaker, Speaker)
	//PUBLISH_VAR_WITH_DEFAULT_AS( m_Delay, 0, Delay)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bOverlapsNext, false, OverlapsNext)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fCheeringDelayMin, 0.0f, CheeringDelayMin)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fCheeringDelayMax, 0.0f, CheeringDelayMax)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bHasSubtitles, false, HasSubtitles)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fProbability, 1, Probability)
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iUseChatGroup, 0, UseChatGroup)
	PUBLISH_PTR_CONTAINER_AS(m_listResponses, Responses)
	PUBLISH_VAR_AS( m_hsAnimation, PartialAnim)
END_STD_INTERFACE

START_STD_INTERFACE(SSubChatItemBank)
	PUBLISH_PTR_CONTAINER_AS(listBankedChatItems, BankedPhrases)
END_STD_INTERFACE

//**************************************************************************************
//	Fixup the raw pointers into their correct types.
//**************************************************************************************
void CChatterBox::PostConstruct()
{
	// Get the ChatterBox Name
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	if(pDO)
	{
		// ALEXEY_TODO : mmmm... what?
		strcpy(m_pcName, ntStr::GetString(pDO->GetName()));
	}
	// Define the maximum expected capacity of the ChatGroup Vector
	m_vectorChatGroupParticipants.reserve(m_listParticipants.size());

	// Add this ChatterBox to the List
	DEBUG_PRINT_CHATTERBOX(("CHATTERBOX: Adding [%s] to ChatterBox Manager\n",m_pcName));
	CChatterBoxMan::Get().AddChatterBox( this );
}

//**************************************************************************************
//	Ctor.
//**************************************************************************************
CChatterBox::CChatterBox() : 
	m_pLastChatter(NULL), m_pSource(NULL), m_iTransitionSize(-1),
	m_SectorBits(0), m_DeathLimit(0), m_uiMaxNumberOfBankedResponses(0),
	m_Active(false), m_Chatting(false),	m_bInitialChatterBox(false), m_bUseBankedPhrases(false), m_bHasScheduledChatItems(false),
	m_bPlayingAnimEventSound(false), m_pActiveTrigger(NULL), m_bIsGeneric(false), m_uiChatGUID(0)
{
	m_CallBackData = NT_NEW SCallBackData;
	m_CallBackData->pChatterBox = NULL;
	m_CallBackData->pCurrentSentence = NULL;
	
	m_CallBackBankedData = NT_NEW SCallBackBankedData;
	m_CallBackBankedData->iNrOfResponses = 0;
	m_CallBackBankedData->pChatterBox = NULL;
	m_CallBackBankedData->pTriggerID = NULL;

	m_CallBackAnimEvent = NT_NEW SCallBackAnimEvent;
	m_CallBackAnimEvent->pChatterBox = NULL;
}

//**************************************************************************************
//	Dtor.
//**************************************************************************************
CChatterBox::~CChatterBox()
{
	this->LevelUnload();
}

void CChatterBox::LevelUnload( void ) 
{
	NT_DELETE (m_CallBackData);
	NT_DELETE (m_CallBackBankedData);
	NT_DELETE (m_CallBackAnimEvent);

	m_CallBackData = NULL;
	m_CallBackBankedData = NULL;
	m_CallBackAnimEvent = NULL;

	// Make sure that all the dynamic memory is released
	ntstd::List<SScheduledChatItem*>::iterator obIt		= m_listScheduledChatItems.begin();
	ntstd::List<SScheduledChatItem*>::iterator obItEnd	= m_listScheduledChatItems.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		NT_DELETE_CHUNK( Mem::MC_MISC, *obIt );
	}
	m_listScheduledChatItems.clear();
}


/***************************************************************************************************
*
*	FUNCTION		CChatterBox::IsSafeToUseEntityAsChatter
*
*	DESCRIPTION		Checks that a particular entity is capable of chatting at a particular moment
*
***************************************************************************************************/
bool CChatterBox::IsSafeToUseEntityAsChatter( CEntity* pEnt )
{
	if (HasChatterBoxChecksDisabled(pEnt))
		return true;

	bool bOK = (	pEnt->GetAttackComponent() && 
					pEnt->GetAttackComponent()->AI_Access_IsInCSStandard() && 
					!pEnt->ToCharacter()->IsDead() &&
					!pEnt->ToCharacter()->IsPaused() 
				);
	return bOK;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::AddParticipant
*
*	DESCRIPTION		Adds a Participant to the ChatterBox. if the participant is already in -> false
*
***************************************************************************************************/
bool CChatterBox::AddParticipant(CEntity* pobParticipant)
{
		// Checking for NULL pointers
	if ( !pobParticipant )
	{
		ntAssert_p( 0, ( "Request to add a NULL pointer Participant to the ChatterBox.\n" ) );
		return false;
	}

	// Make sure that we don't already have this child in our list - that would be bad
	ntstd::List<CEntity*>::iterator obEndIt = m_listParticipants.end();
	for( ntstd::List<CEntity*>::iterator obIt = m_listParticipants.begin(); obIt != obEndIt; ++obIt )
	{
		CEntity* pEnt = (*obIt);
		if (!pEnt)
		{
			DEBUG_PRINT_CHATTERBOX(("(ERROR!) Corrupted Participant's list (i.e. NULL Entities) in ChatterBox [%s].\n",this->m_pcName));
			return false;
		}

		// If we find it warn some people
		if ( pEnt == pobParticipant )
		{
			DEBUG_PRINT_CHATTERBOX(("(WARNING!) Request to add a Participant [%s] to the ChatterBox that is already present.\n",ntStr::GetString(pobParticipant->GetName())));
			return false;
		}
	}

	// Add the new child to our list
	DEBUG_PRINT_CHATTERBOX(("Adding Participant: %s to Chatterbox: %s\n",pobParticipant->GetName().c_str(),this->m_pcName));
	m_listParticipants.push_back( pobParticipant );
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::RemoveParticipant
*
*	DESCRIPTION		Removes a Participant from the ChatterBox. if problem -> false
*
***************************************************************************************************/
bool CChatterBox::RemoveParticipant(const CEntity* pobParticipant)
{
	// Checking for NULL pointers
	if ( !pobParticipant )
		return false;

	if ( pobParticipant->ToAI()->IsSpawned() )
	{
		DEBUG_PRINT_CHATTERBOX(("Participant: %s will not be removed from a chatterbox since he was Spawned\n",ntStr::GetString(pobParticipant->GetName())));
		return false;
	}

	// Run through the list of participants and find the selected one
	ntstd::List<CEntity*>::iterator obIt	= m_listParticipants.begin();
	ntstd::List<CEntity*>::iterator obEndIt	= m_listParticipants.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CEntity* pEnt = (*obIt);
		if (!pEnt)
		{
			DEBUG_PRINT_CHATTERBOX(("(ERROR!) Corrupted Participant's list (i.e. NULL Entities) in ChatterBox [%s].\n",this->m_pcName));
			return false;
		}

		// If found, remove it and return true
		if ( pEnt == pobParticipant )
		{
			DEBUG_PRINT_CHATTERBOX(("Removing Participant: %s from Chatterbox: %s\n",ntStr::GetString(pEnt->GetName()),this->m_pcName));
			m_listParticipants.erase( obIt );
			return true;
		}
	}

	// If we are here then something has gone wrong
	DEBUG_PRINT_CHATTERBOX(("(WARNING!) We have tried to remove a participant (%s) that didn't exist in the ChatterBox (%s).\n", ntStr::GetString(pobParticipant->GetName()),this->m_pcName));
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::Activate
*
*	DESCRIPTION		Activates the ChatterBox. if already active -> returns false
*
***************************************************************************************************/
void CChatterBox::CopyParticipantsIntoGenericChatterBoxes( void )
{
	ntstd::List<CChatterBox*>::iterator obItCB		= m_listGenericChatterBoxes.begin();
	ntstd::List<CChatterBox*>::iterator obEndItCB	= m_listGenericChatterBoxes.end();
	for( ; obItCB != obEndItCB; ++obItCB )
	{
		CChatterBox* pCB = *obItCB;
		if (!pCB)
		{
			DEBUG_PRINT_CHATTERBOX(("(Error!) Invalid Generic ChatterBoxes Assigned to ChatterBox: [%s]\n",this->GetName() ));
			return;
		}

		pCB->m_listParticipants.clear();

		ntstd::List<CEntity*>::iterator obIt	= m_listParticipants.begin();
		ntstd::List<CEntity*>::iterator obEndIt	= m_listParticipants.end();
		for( ; obIt != obEndIt; ++obIt )
		{
			CEntity* pEnt = (*obIt);
			pCB->m_listParticipants.push_back(pEnt);
		}
	}
}

/***************************************************************************************************
*	FUNCTION		CChatterBox::ResetStatistics
*	DESCRIPTION		Resets the Statistics of this chatterbox
***************************************************************************************************/
void CChatterBox::ResetStatistics( void )
{
	if (m_listStatistics.empty())
		return;

	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obIt		= m_listStatistics.begin();
	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obItEnd	= m_listStatistics.end();
	for ( ; obIt != obItEnd; ++obIt )
	{
		SChatterBoxStatisticPair* pStat = *obIt;
		if ( pStat )
		{
			pStat->uiCount = 0;
			DEBUG_PRINT_CHATTERBOX(("Statistic Reset: ChatterBox: [%s] - Trigger [%s] - Count: [%d]\n",this->GetName(),ntStr::GetString(pStat->hsTriggerEvent), pStat->uiCount ));
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::Activate
*
*	DESCRIPTION		Activates the ChatterBox. if already active -> returns false
*
***************************************************************************************************/
bool CChatterBox::Activate( bool bResetStatistics )
{
	if (!this)
	{
		DEBUG_PRINT_CHATTERBOX(("(ERROR!) You are trying to activate a NULL chatterbox.\n"));
		return false;
	}

	if (!m_Active)
	{
		// Set Up CallBack Data
		this->m_CallBackData->pChatterBox = this;
		this->m_CallBackBankedData->pChatterBox = this;
		this->m_CallBackAnimEvent->pChatterBox = this;

		// Transfer participants to any associated Generic ChatterBoxes
		CopyParticipantsIntoGenericChatterBoxes();

		// Reset Statistics
		if (bResetStatistics)
			ResetStatistics();

		return (m_Active=true);
	}
	// If here, you are trying to activate an already active CB
	DEBUG_PRINT_CHATTERBOX(("(WARNING!) You are trying to activate an already active ChatterBox (%s).\n",this->GetName()));
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::Deactivate
*
*	DESCRIPTION		Deactivates the ChatterBox. if already not active -> returns false
*
***************************************************************************************************/
bool CChatterBox::Deactivate(void)
{
	if (m_Active)
	{
	//	this->m_CallBackData->pChatterBox = NULL;
		this->m_CallBackBankedData->pChatterBox = NULL;
		this->m_CallBackAnimEvent->pChatterBox = NULL;
		m_Active=false;
		return (true);
	}
	// If here, you are trying to deactivate an already unactive CB
	DEBUG_PRINT_CHATTERBOX(("(WARNING!) You are trying to deactivate an already unactive ChatterBox (%s).\n",this->GetName()));
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::IsParticipantInList
*
*	DESCRIPTION		Returns true if the participant is in the list
*
***************************************************************************************************/
bool CChatterBox::IsParticipantInList(const CEntity* pobParticipant) const
{
	// Checking for NULL pointers
	if ( !pobParticipant )
	{
		DEBUG_PRINT_CHATTERBOX(("(ERROR!) Request to Check a NULL pointer Participant from the ChatterBox (%s).\n", this->GetName()));
		return false;
	}

	// Is the participants' list empty?
	if (m_listParticipants.empty())
	{
		DEBUG_PRINT_CHATTERBOX(("(WARNING!) The chatterbox (%s) participants' list is empty!\n", this->GetName()));
		return false;
	}
	// Run through the list of participants and find the selected one
	ntstd::List<CEntity*>::const_iterator obIt		= m_listParticipants.begin();
	ntstd::List<CEntity*>::const_iterator obEndIt	= m_listParticipants.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CEntity* pEnt = (*obIt);
		// If found, return true
		if ( pEnt == pobParticipant )
		{
			return true;
		}

		if (!pEnt)
		{
			DEBUG_PRINT_CHATTERBOX(("(ERROR!) Corrupted Participant's list (i.e. NULL Entities) in ChatterBox [%s].\n",this->m_pcName));
			return false;
		}
	}
	// Not found
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::IsEventSupported
*
*	DESCRIPTION		Returns true if the event is in the list
*
***************************************************************************************************/
bool CChatterBox::IsEventSupported(const char* pcEventName) const
{
// Checking for NULL pointers
	if ( !pcEventName )
	{
		DEBUG_PRINT_CHATTERBOX(("(ERROR!) Request to Check support for a NULL pointer Event(String) from the ChatterBox (%s).\n", this->GetName()));
		return false;
	}
	// Run through the list of Events and find the selected one
	ntstd::List<SChatTrigger*>::const_iterator obEndIt = m_Triggers.end();
	for( ntstd::List<SChatTrigger*>::const_iterator obIt = m_Triggers.begin(); obIt != obEndIt; ++obIt )
	{
		// If found, return true
		if ( ( *obIt )->m_TriggerID == pcEventName )
		{
			return true;
		}
	}
	// Not found
	return false;
}

/**************************************************************************************************
*	FUNCTION		CChatterBox::PickChatter
***************************************************************************************************/

void CChatterBox::UpdateChatGroupParticipantsVector ( int iChatGroupID )
{
	m_vectorChatGroupParticipants.clear();

	ntstd::List<CEntity*>::const_iterator obIt		= m_listParticipants.begin();
	ntstd::List<CEntity*>::const_iterator obItEnd	= m_listParticipants.end();

	for ( ; obIt != obItEnd; ++obIt )
	{
		CEntity* pEnt = *obIt;
		if (!pEnt)
		{
			DEBUG_PRINT_CHATTERBOX(("(ERROR!) NULL participant found in ChatterBox (%s)\n",this->GetName()));
			continue;
		}
		if ( pEnt->IsAI() && pEnt->ToAI()->GetChatGroupID() == iChatGroupID )
		{
			m_vectorChatGroupParticipants.push_back(pEnt);
		}
#ifndef _RELEASE
		if ( !pEnt->IsAI())
		{
			user_warn_msg(("Entity: [%s] IS NOT AN AI, but has been added as participant in\n ChatterBox: [%s]",ntStr::GetString(pEnt->GetName()), this->GetName() ));
		}
#endif
	}
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::PickChatter
*
*	DESCRIPTION		
*
***************************************************************************************************/
CEntity* CChatterBox::PickChatter (SChatItem* pChatItem, bool* bFindAnotherPhrase)
{
	CEntity* pEntSpeaker	= pChatItem->m_pSpeaker;

	*bFindAnotherPhrase = false;

	// NULL pointer checking done in the functions
	
	// Do we have a selected Speaker?
	if (pEntSpeaker)
	{
		if (IsSafeToUseEntityAsChatter(pEntSpeaker))
			return pEntSpeaker;
		else
		{
			*bFindAnotherPhrase = true;
			return NULL;
		}
	}

	// Is there any participant?
	if (m_listParticipants.empty())
	{
		DEBUG_PRINT_CHATTERBOX(("(WARNING!) No participants in the ChatterBox (%s)\n",this->GetName()));
		return NULL;
	}
	
	UpdateChatGroupParticipantsVector(pChatItem->m_iUseChatGroup);

	// Let's take a random chatter from this group
	unsigned int uiParticipantsListSize = m_vectorChatGroupParticipants.size();
	if (uiParticipantsListSize < 1)
	{
		DEBUG_PRINT_CHATTERBOX(("(WARNING!) PickChatter: No available participants of GROUP [%d] in ChatterBox (%s)\n",pChatItem->m_iUseChatGroup, this->GetName()));
		return NULL;
	}

	unsigned int uiRand = (drand() % uiParticipantsListSize);
	
	DEBUG_PRINT_CHATTERBOX(("Random Participant Index : %d of %d\n",uiRand,uiParticipantsListSize-1));
	if (this->m_pLastChatter)
	{
		DEBUG_PRINT_CHATTERBOX(("Event Source: %s - Last Chatter: %s\n", ntStr::GetString(this->m_pSource->GetName()), ntStr::GetString(this->m_pLastChatter->GetName())));
	}
	// Is there is ANY participant to chat with : NOTE: What if 2 participants, but one is KO ?????
	if (uiParticipantsListSize<2)
	{
		// No participants available...
		DEBUG_PRINT_CHATTERBOX(("No one to chat with... (participants' list size = %d)\n",uiParticipantsListSize));
		this->SetChattingStatus(false);
		return NULL;
	}

	ntAssert_p(uiRand<uiParticipantsListSize, ("PickChatter: Error in the selection of a participant. Index >= vector size!!!!!")); // To be removed!!!

	// Take the selected participant
	CEntity* pChatter = m_vectorChatGroupParticipants[uiRand];

	// Is the selected chatter the same one who chatted last time or the source? 
	if ( (!(pChatter==m_pLastChatter ) &&
		  !(pChatter==m_pSource) ) &&
		 (IsSafeToUseEntityAsChatter(pChatter)))
	{
		// No, he's a different one and can chat. Gooooood.....
		return pChatter;
	}
	else
	{
		// Conflict solving ...
		// Check to the right
		unsigned int iSearchIndex = uiRand;
		while (++iSearchIndex < uiParticipantsListSize)
		{
			pChatter = m_vectorChatGroupParticipants[iSearchIndex];
			if (!(pChatter==m_pLastChatter ) &&
				!(pChatter==m_pSource)  &&
				(IsSafeToUseEntityAsChatter(pChatter)))
			{
				// A suitable chatter found.
				DEBUG_PRINT_CHATTERBOX(("Returning Chatter (++): %s\n",ntStr::GetString(pChatter->GetName())));
				return pChatter;
			}
		}
		// Check to the left
		iSearchIndex = uiRand;
		while (iSearchIndex > 0)
		{
			--iSearchIndex;
			pChatter = m_vectorChatGroupParticipants[iSearchIndex];
			if (!(pChatter==m_pLastChatter ) &&
				!(pChatter==m_pSource)  &&
				(IsSafeToUseEntityAsChatter(pChatter)))
			{
				// A suitable chatter found.
				DEBUG_PRINT_CHATTERBOX(("Returning Chatter (--): %s\n",ntStr::GetString(pChatter->GetName())));
				return pChatter;
			}
		}
		// No suitable chatter found... This means that there is only two chatters and one is KO...
		// Change the chatterbox status
		this->SetChattingStatus(false);
		//ntAssert_p( 0 , ("PickChatter could not find a chatter diff. from Source and  diff. from last chatter \n") );
		DEBUG_PRINT_CHATTERBOX(("PickChatter: No chatter available to further the chat (reply). They are source and the last chatter. Participants: %d\n",uiParticipantsListSize));
		return NULL;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::GetChatTrigger
*
*	DESCRIPTION		
*
***************************************************************************************************/
SChatTrigger* CChatterBox::GetChatTrigger(const char* pcEventName, CChatterBox** pFromCB)
{
	*pFromCB = NULL;

	if (!pcEventName)
		return NULL;

	// Run through the Trigger List to find the selected event
	ntstd::List<SChatTrigger*>::const_iterator obIt		= m_Triggers.begin();
	ntstd::List<SChatTrigger*>::const_iterator obEndIt	= m_Triggers.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( ( *obIt )->m_TriggerID == pcEventName )
		{
			*pFromCB = this;
			return ( *obIt );
		}
	}

	// Not found so move to the associated generic chatterboxes
	ntstd::List<CChatterBox*>::const_iterator obItCB		= m_listGenericChatterBoxes.begin();
	ntstd::List<CChatterBox*>::const_iterator obEndItCB		= m_listGenericChatterBoxes.end();
	for( ; obItCB != obEndItCB; ++obItCB )
	{
		CChatterBox* pCB = *obItCB;
		if (!pCB)
			continue;
		const ntstd::List<SChatTrigger*>* plistSTrigger = pCB->GetTriggerList();
		if (!plistSTrigger)
			continue;

		ntstd::List<SChatTrigger*>::const_iterator obItTrigger		= plistSTrigger->begin();
		ntstd::List<SChatTrigger*>::const_iterator obEndItTrigger	= plistSTrigger->end();
		for( ; obItTrigger != obEndItTrigger; ++obItTrigger )
		{
			SChatTrigger* pST = *obItTrigger;
			if (pST->m_TriggerID == pcEventName)
			{
				DEBUG_PRINT_CHATTERBOX(( "Trigger Event (%s) not supported by ChatterBox (%s) but found in associated Generic Chatterbox (%s).\n",pcEventName,this->GetName(),pCB->GetName() ))
				*pFromCB = pCB;
				return pST;
			}
		}
	}

	DEBUG_PRINT_CHATTERBOX(( "Trigger Event (%s) not supported neither by ChatterBox (%s) nor any of its (%d) associated Generic Chatterboxes.\n",pcEventName,this->GetName(),m_listGenericChatterBoxes.size() ))
	return NULL;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::PickStartPhrase
*
*	DESCRIPTION		
*
***************************************************************************************************/
SChatItem* CChatterBox::PickStartPhrase(const char* pcEventName)
{
	if (!pcEventName)
		return NULL;

	CChatterBox* pCB = NULL;
	SChatTrigger* pobSelectedTrigger = GetChatTrigger(pcEventName, &pCB);

	if (pCB != this)
	{
		DEBUG_PRINT_CHATTERBOX(("PickStartPhrase: Trigger (%s) could not be found in Chatterbox(%s)\n",pcEventName,this->GetName()));
		return NULL;
	}

	// NULL check
	if (!pobSelectedTrigger)
	{
		DEBUG_PRINT_CHATTERBOX(("PickStartPhrase: Trigger (%s) coudl not be found in Chatterbox(%s) or any associated Generic Chatterboxes\n",pcEventName,this->GetName()));
		return NULL;
	}
	
	if (pobSelectedTrigger->m_listInitialPhrases.empty())
	{
		// There is no starting phrases available!! Report this.
		DEBUG_PRINT_CHATTERBOX(("(ERROR!) There is no starting phrases available for event (%s) in ChatterBox (%s)\n",pcEventName, this->GetName()) );
		return NULL;
	}
    
	unsigned int uiInitialPhraseListSize = pobSelectedTrigger->m_listInitialPhrases.size();
	unsigned int uiRand = (drand() % uiInitialPhraseListSize);
	ntstd::List<SChatItem*>::const_iterator obIt = pobSelectedTrigger->m_listInitialPhrases.begin();

	if (uiInitialPhraseListSize == 1)
	{
		// Strange... an event with a single starting phrase
		DEBUG_PRINT_CHATTERBOX(("(WARNING!) Event (%s) has a single starting phrase in ChatterBox (%s)\n",pcEventName, this->GetName()) );
		return (*obIt);
	}

	// Go to the selected phrase
	unsigned int uiIndex = 0;
	while ( uiIndex < uiRand  ) { ++obIt; ++uiIndex; }

	// Is the selected starting phrase like the previous one? 

	if ( (*obIt) != pobSelectedTrigger->m_pPreviousPhrase )
	{
		// It is a different one
		DEBUG_PRINT_CHATTERBOX(("Selected phrase: %d of %d\n",uiIndex, uiInitialPhraseListSize-1));
		pobSelectedTrigger->m_pPreviousPhrase = (*obIt);
		return (*obIt);
	}

	// Solve conflict

	if (++obIt == pobSelectedTrigger->m_listInitialPhrases.end())
	{
		// If it was the last phrase, take the first one
		obIt=pobSelectedTrigger->m_listInitialPhrases.begin();
	}
	
	DEBUG_PRINT_CHATTERBOX(("Selected phrase: %d of %d\n",uiIndex, uiInitialPhraseListSize-1));
	pobSelectedTrigger->m_pPreviousPhrase = (*obIt);
	return (*obIt);
}


//! ----------------------------------------------------------
//!  SChatItem* CChatterBox::PickResponse(SChatItem* pPreviousSentence)
//!
//!  Pick a response
//! ----------------------------------------------------------
SChatItem* CChatterBox::PickResponse(SChatItem* pPreviousSentence)
{
	if (!pPreviousSentence)
		return NULL;

	if (pPreviousSentence->m_listResponses.empty())
	{
		DEBUG_PRINT_CHATTERBOX(("No further response found. End of Chat.\n"));
		return NULL;
	}
    
	// A random number to select a (random) response
	unsigned int uiResponsesListSize = pPreviousSentence->m_listResponses.size();
	unsigned int uiRand = (drand() % uiResponsesListSize);
	ntstd::List<SChatItem*>::const_iterator obIt = pPreviousSentence->m_listResponses.begin();

	if (uiResponsesListSize == 1)
	{
		// There is only one response. Return it
		return (*obIt);
	}

	// Go to the selected response
	unsigned int uiIndex = 0;
	while ( uiIndex < uiRand  ) { ++obIt; ++uiIndex; }

	// Is the selected response like the previous one? 

	if ( (*obIt) != pPreviousSentence->m_pPreviousResponse )
	{
		// It is a different one
		DEBUG_PRINT_CHATTERBOX(("Selected response: %d of %d\n",uiIndex, uiResponsesListSize-1))
		pPreviousSentence->m_pPreviousResponse = (*obIt);
		return (*obIt);
	}

	// Solve conflict

	if (++obIt == pPreviousSentence->m_listResponses.end())
	{
		// If it was the last phrase, take the first one
		obIt=pPreviousSentence->m_listResponses.begin();
	}
	DEBUG_PRINT_CHATTERBOX(("Selected response (+-): %d of %d\n",uiIndex, uiResponsesListSize-1));
	pPreviousSentence->m_pPreviousResponse = (*obIt);
	return (*obIt);
}

/***************************************************************************************************
*	FUNCTION		CChatterBox::HasValidChatGUID
***************************************************************************************************/

bool CChatterBox::HasValidChatGUID ( void )
{
	unsigned int uiValidGUID = CChatterBoxMan::Get().GetCurrentChatGUID();
	return (this->m_uiChatGUID == uiValidGUID);
}

//! ----------------------------------------------------------
//!  void VOFinished(void* pData)
//!
//!  PlayResponse after the previous audio stream is finished
//! ----------------------------------------------------------

static void VOFinished(void* pData)
{
	SCallBackData* pCD = (SCallBackData*)pData;
	if ( pCD && pCD->pChatterBox )
	{
		if (pCD->pChatterBox->HasValidChatGUID())
			pCD->pChatterBox->PlayResponse(pCD->pCurrentSentence);
		else
		{
			pCD->pChatterBox->SetChattingStatus(false);
			DEBUG_PRINT_CHATTERBOX(("Call-back function VOFinished interrupted by a more important conversation\n"));
		}
	}
	else
	{
		DEBUG_PRINT_CHATTERBOX(("(ERROR) Call-back function VOFinished got NULL data\n"));
	}
}



/***************************************************************************************************
*
*	FUNCTION		CChatterBox::PlayResponse
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CChatterBox::PlayResponse (SChatItem* pPreviousSentence)
{
	CEntity* pobNewChatter;
	SChatItem* pSResponse;

	// Initialise chatting status
	this->SetChattingStatus(false);

	if (!pPreviousSentence || IsPlayingAnimEventSound())
		return false;

	// Check if this response this chatterbox is still valid for responding
	// otherwise, when deactivated, the CallBackFunction points to NULL;
	if (!this)
		return false;

	// ==========================================================================================================
	//	Let's find aresponse and a suitbale chatter. Not the source, not the last one, not KO, recoiling, etc...
	// ==========================================================================================================
	int  iWatchDog = 0;
	bool bAnotherPhraseNeeded = false;
	do
	{
		// Pick a random response for this event, from the pSCI responses list
		pSResponse = this->PickResponse(pPreviousSentence);
		if (!pSResponse)
		{
			DEBUG_PRINT_CHATTERBOX(("No more responses are available for this conversation in ChatterBox (%s)\n", this->GetName()));
			return false;
		}
		// Check the probability of being actually played
		float fRandProb = drandf(1.0f);
		if (pSResponse->m_fProbability < fRandProb)
		{
			DEBUG_PRINT_CHATTERBOX(("Response (%s) not played due to low probability [%.2f/%.2f] in ChatterBox (%s)\n", ntStr::GetString(pSResponse->m_Sfx), pSResponse->m_fProbability, fRandProb, this->GetName()));
			return false;
		}

		// Pick a Chatter
		pobNewChatter=this->PickChatter(pSResponse, &bAnotherPhraseNeeded);
		if (!pobNewChatter)
		{
			// No chatter available
				DEBUG_PRINT_CHATTERBOX(("ChatterBox: no chatter available\n"));
				return false;
			}

			// Don't loop for long
			if (iWatchDog++ > 2)
			{
				if (pSResponse->m_pSpeaker)
				{
					DEBUG_PRINT_CHATTERBOX(("(WARNING!) WatchDog: It seems that there is only ONE response and its linked to an AI (%s) that cannot currently chat in ChatterBox (%s). Action: No chatting.\n", ntStr::GetString(pSResponse->m_pSpeaker->GetName()),this->GetName()));
				}
				else
				{
					DEBUG_PRINT_CHATTERBOX(("(ERROR!) WatchDog: I shouldn't be here... NULL speaker?\n", this->GetName()));
				}
			return false;
		}
	} while(bAnotherPhraseNeeded);

	// ========================================================================================================
	//	Ok, we have an valid Chatter and a phrase... Play it again Sam...
	// ========================================================================================================

	// Remember the last chatter and update the CALLBACK data
	this->m_pLastChatter = pobNewChatter;
	
	// Check if is an Overlapping response (cheering, etc.)
	if (pSResponse->m_bOverlapsNext)
	{
		//Schedule the next response
		DEBUG_PRINT_CHATTERBOX(("AI:(%s) cheering response (Bank: %s - Sfx: %s)\n",ntStr::GetString(pobNewChatter->GetName()),ntStr::GetString(pSResponse->m_Bank), ntStr::GetString(pSResponse->m_Sfx)));
		bool bOKSched = Schedule(pSResponse, pobNewChatter);
		if (!bOKSched)
		return false;
	}
	else
	{	
		this->m_CallBackData->pCurrentSentence = pSResponse;
		pobNewChatter->GetEntityAudioChannel()->SetCallback(&VOFinished, (void *)(this->m_CallBackData));
		DEBUG_PRINT_CHATTERBOX(("AI:(%s) response (Bank: %s - Sfx: %s)\n",ntStr::GetString(pobNewChatter->GetName()),ntStr::GetString(m_CallBackData->pCurrentSentence->m_Bank), ntStr::GetString(m_CallBackData->pCurrentSentence->m_Sfx)));
	}
	
	bool bOK = pobNewChatter->GetEntityAudioChannel()->Play(CHANNEL_VOICE_HIGHPRI, pSResponse->m_Bank, pSResponse->m_Sfx);
	if (bOK && !pSResponse->m_hsAnimation.IsNull() )
	{
		// Play Partial Anim
		pobNewChatter->GetMovement()->PlayPartialAnim( pSResponse->m_hsAnimation ); // Attempts to play an anim on the controller.  Will silently return if it can't play it.
	}

	// campf - handle subtitles if there are any
	if (pSResponse->m_bHasSubtitles)
	{
		CSubtitleMan::Get().Play( Util::Upppercase( Util::NoExtension( Util::BaseName(pSResponse->m_Sfx.GetString()) ) ) );
	}

	this->SetChattingStatus(bOK);
	return bOK;
}



/***************************************************************************************************
*
*	FUNCTION		CChatterBox::StartChatting
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CChatterBox::StartChatting ( const char* strEvent, CEntity* pEntSource )
{
	// Initialise chatting status
	this->SetChattingStatus(false);

	if (!strEvent)
		return false;

	if ( !pEntSource )
	{
		DEBUG_PRINT_CHATTERBOX(("(ERROR!) StartChatting() recived a NULL Source Entity (%s)\n", this->GetName()));
		return false;
	}

	CEntity*	pobNewChatter;
	SChatItem*	pStartPhrase = this->PickStartPhrase(strEvent);
	if (!pStartPhrase)
	{
		DEBUG_PRINT_CHATTERBOX(("(WARNING!): No starting phrase available in ChatterBox (%s)\n", this->GetName()));
		return false;
	}

	float fRandProb = drandf(1.0f);
	if (pStartPhrase->m_fProbability < fRandProb)
	{
		DEBUG_PRINT_CHATTERBOX(("Start Phrase (%s) not played due to low probability [%.2f/%.2f] in ChatterBox (%s)\n", ntStr::GetString(pStartPhrase->m_Sfx), pStartPhrase->m_fProbability, fRandProb, this->GetName()));
		return false;
	}

	// ========================================================================================================
	//	Let's find the first phrase and a chatter. Not the source, not the last one, not KO, recoiling, etc...
	// ========================================================================================================
	int  iWatchDog = 0;
	bool bAnotherPhraseNeeded = false;
	do
	{
		// Take the first phrase for this event
		if (!pStartPhrase)
		{
			DEBUG_PRINT_CHATTERBOX(("(WARNING!): No starting phrase available in ChatterBox (%s)\n", this->GetName()));
			return false;
		}

		// Pick a Chatter (The one who will say the sentence)
		pobNewChatter=this->PickChatter(pStartPhrase, &bAnotherPhraseNeeded);

		// Don't loop for long
		if (iWatchDog++ > 2)
		{
			if (pStartPhrase->m_pSpeaker)
			{
				DEBUG_PRINT_CHATTERBOX(("(WARNING!) WatchDog: It seems that there is only ONE starting phrase and its linked to an AI (%s) that cannot currently chat in ChatterBox (%s). Action: No chatting.\n", ntStr::GetString(pStartPhrase->m_pSpeaker->GetName()),this->GetName()));
			}
			else
			{
				DEBUG_PRINT_CHATTERBOX(("(ERROR!) WatchDog: I shouldn't be here... NULL speaker?\n", this->GetName()));
			}
			return false;
		}
		
	} while(bAnotherPhraseNeeded);

	if (!pobNewChatter)
	{
		// No chatter available
		DEBUG_PRINT_CHATTERBOX(("(WARNING!) No chatter available in ChatterBox (%s)\n", this->GetName()));
		return false;
	}

	this->m_pLastChatter = pobNewChatter;	// Record last chatter;
	this->m_pSource = pEntSource;

	// ========================================================================================================
	//	Ok, we have an valid Chatter and a phrase... Play it again Sam...
	// ========================================================================================================

	CChatterBoxMan::Get().StopCurrentChats();
	m_uiChatGUID = CTimer::Get().GetSystemTicks();

	DEBUG_PRINT_CHATTERBOX(("STARTING CONVERSATION TREE\n"));
	DEBUG_PRINT_CHATTERBOX(("--------------------------\n"));
	// Check if is an Overlapping response (cheering, etc.)
	if (pStartPhrase->m_bOverlapsNext)
	{
		//Schedule the next response
		DEBUG_PRINT_CHATTERBOX(("AI:(%s) cheering start phrase (Bank: %s - Sfx: %s)\n",ntStr::GetString(pobNewChatter->GetName()), ntStr::GetString(pStartPhrase->m_Bank), ntStr::GetString(pStartPhrase->m_Sfx)));
		bool bOKSched = Schedule(pStartPhrase, pobNewChatter);
		if (!bOKSched)
		return false;
	}
	else
	{	
		this->m_CallBackData->pCurrentSentence = pStartPhrase;
		pobNewChatter->GetEntityAudioChannel()->SetCallback(&VOFinished, (void *)(this->m_CallBackData));
		DEBUG_PRINT_CHATTERBOX(("AI:(%s) start phrase (Bank: %s - Sfx: %s)\n",pobNewChatter->GetName().c_str(), ntStr::GetString(m_CallBackData->pCurrentSentence->m_Bank), ntStr::GetString(m_CallBackData->pCurrentSentence->m_Sfx)));
		if (!this->m_CallBackData || !this->m_CallBackData->pChatterBox)
			DEBUG_PRINT_CHATTERBOX(("AI:(%s). However, this conversation finishes here since the Playing ChatterBox (%s) does not have properly set the CallBackData. Was this CB activated?\n",pobNewChatter->GetName().c_str(), this->GetName() ));
	}
	
	bool bOK = pobNewChatter->GetEntityAudioChannel()->Play(CHANNEL_VOICE_HIGHPRI, pStartPhrase->m_Bank, pStartPhrase->m_Sfx);
	if (bOK && !pStartPhrase->m_hsAnimation.IsNull() )
	{
		// Play Partial Anim
		pobNewChatter->GetMovement()->PlayPartialAnim( pStartPhrase->m_hsAnimation ); // Attempts to play an anim on the controller.  Will silently return if it can't play it.
	}

	// campf - handle subtitles if there are any
	if (pStartPhrase->m_bHasSubtitles)
	{
		CSubtitleMan::Get().Play( Util::Upppercase( Util::NoExtension( Util::BaseName(pStartPhrase->m_Sfx.GetString()) ) ) );
	}
	this->SetChattingStatus(bOK);
	return bOK;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::AssignBankToParticipant
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CChatterBox::AssignBankToParticipant (CEntity* pParticipant)
{
	bool ret = false;

	// Go along the Different Triggers
	const ntstd::List<SChatTrigger*>* pTriggerList = this->GetTriggerList();
	ntstd::List<SChatTrigger*>::const_iterator obItEvent = pTriggerList->begin();
	ntstd::List<SChatTrigger*>::const_iterator obItEventEnd = pTriggerList->end();
	for ( ; obItEvent!=obItEventEnd; ++obItEvent)
	{
		SChatTrigger* pSCT = (*obItEvent);
		ret = false;
		const ntstd::List<SSubChatItemBank*>* plistBanks = &(pSCT->m_listParticipantsBanks);
		if (!plistBanks)
		{
			// Null List of banks passed
			DEBUG_PRINT_CHATTERBOX(("AssignBankToParticipant: Null List of banks found for Event (%s)\n",ntStr::GetString(pSCT->m_TriggerID)));
			return false;
		}
		ntstd::List<SSubChatItemBank*>::const_iterator obIt = plistBanks->begin();
		ntstd::List<SSubChatItemBank*>::const_iterator obItBanksEnd = plistBanks->end();
		// Go along the Different Banks
		for ( ; obIt!=obItBanksEnd; ++obIt)
		{
			SSubChatItemBank* pSCBItem = (*obIt);
			// Does it have owner?
			if ( ! pSCBItem->pParticipant )
			{
				// No. Assigne it pParticipant
                pSCBItem->pParticipant = pParticipant;
				DEBUG_PRINT_CHATTERBOX(("Event (%s) -> Participant (%s) found a unallocated Bank. OK\n", ntStr::GetString(pSCT->m_TriggerID), ntStr::GetString(pParticipant->GetName())));
				
				//ntstd::List<SChatItem*>::const_iterator obItChatItem	= pSCBItem->listBankedChatItems.begin();
				//ntstd::List<SChatItem*>::const_iterator obItChatItemEnd = pSCBItem->listBankedChatItems.end();
				/*
				// Go along the Different BankedChatItems (for debug)
				ntPrintf("List of Chat Items in this bank:\n");
				for ( ; obItChatItem!=obItChatItemEnd; ++obItChatItem)
				{
					ntPrintf("-> %s\n", *((*obItChatItem)->m_Sfx));
				}
				*/
				ret = true;
				break;
			}
			DEBUG_PRINT_CHATTERBOX(("Event (%s) -> Participant (%s) found BUSY Bank owned by (%s).\n",ntStr::GetString(pSCT->m_TriggerID),  ntStr::GetString(pParticipant->GetName()), ntStr::GetString(pSCBItem->pParticipant->GetName())));
		}
	}
	return ret;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::PickBankedPhrase
*
*	DESCRIPTION		
*
***************************************************************************************************/
SChatItem* CChatterBox::PickBankedPhrase(SSubChatItemBank* pParticipantBank)
{
	if (!pParticipantBank)
		return NULL;

	// Are there any responses' banks?
	if (pParticipantBank->listBankedChatItems.empty())
	{
		// No banked phrases available
		DEBUG_PRINT_CHATTERBOX(("PickBankedPhrase: No banked phrases available for current event and chatter in (%s)\n", this->GetName()));
		return NULL;
	}
	// Random numbers    
	unsigned int uiBankedPhraseListSize = pParticipantBank->listBankedChatItems.size();
	unsigned int uiRand = drand() % uiBankedPhraseListSize;
	ntstd::List<SChatItem*>::const_iterator obIt = pParticipantBank->listBankedChatItems.begin();

	if (uiBankedPhraseListSize == 1)
	{
		// Strange... an event with a single starting phrase
		DEBUG_PRINT_CHATTERBOX(("(Warning!) Participant (%s) has a single phrase!\n",ntStr::GetString(pParticipantBank->pParticipant->GetName())));
		return (*obIt);
	}

	// Go to the selected phrase
	unsigned int uiIndex = 0;
	while ( uiIndex < uiRand  ) { ++obIt; ++uiIndex; }

	// Is the selected starting phrase like the previous one? 

	if ( (*obIt) != pParticipantBank->pLastPhrase )
	{
		// It is a different one
		//ntPrintf("Selected phrase: %d of %d\n",uiIndex, uiInitialPhraseListSize-1);
		pParticipantBank->pLastPhrase = (*obIt);
		return (*obIt);
	}

	// Solve conflict

	if (++obIt == pParticipantBank->listBankedChatItems.end())
	{
		// If it was the last phrase, take the first one
		obIt=pParticipantBank->listBankedChatItems.begin();
	}
	
	DEBUG_PRINT_CHATTERBOX(("Selected phrase: %d of %d\n",uiIndex, uiBankedPhraseListSize-1));
	pParticipantBank->pLastPhrase = (*obIt);
	return (*obIt);
}


//! ----------------------------------------------------------
//!  SChatItem* CChatterBox::GetParticipantBank(const char* pcEventName, CEntity* pParticipant)
//!
//!   (for SubChatterBox)
//! ----------------------------------------------------------
SSubChatItemBank* CChatterBox::GetParticipantBank(const char* pcEventName, CEntity* pParticipant)
{
	// Null pointers checked by caller functions

	CChatterBox* pCB = NULL;
	SChatTrigger* pobSelectedTrigger = GetChatTrigger(pcEventName, &pCB);

	if (pCB != this)
	{
		DEBUG_PRINT_CHATTERBOX(("GetParticipantBank: Trigger (%s) could not be found in Chatterbox(%s)\n",pcEventName,this->GetName()));
		return NULL;
	}
	
	// NULL check
	if (!pobSelectedTrigger)
	{
		DEBUG_PRINT_CHATTERBOX(("GetParticipantBank: Returned NULL Trigger for event (%s)\n",pcEventName));
		return NULL;
	}
	// Is it empty?
	if (pobSelectedTrigger->m_listParticipantsBanks.empty())
	{
		// There is no banked phrases available!! Report this.
		ntAssert_p(0, (" There is no banked phrases available for event (%s)\n",pcEventName));// and participant (%s)\n",pcEventName,pParticipant->GetName().c_str()) );
		return NULL;
	}
	// Find the bank that belongs to the selected participant
	ntstd::List<SSubChatItemBank*>::const_iterator obItEnd = pobSelectedTrigger->m_listParticipantsBanks.end();
	for (ntstd::List<SSubChatItemBank*>::const_iterator obIt = pobSelectedTrigger->m_listParticipantsBanks.begin(); obIt!=obItEnd; ++obIt)
	{
		if ( (*obIt)->pParticipant == pParticipant ) 
		{
			// Found
			return (*obIt);
		}
	}
	// Not found
	DEBUG_PRINT_CHATTERBOX(("GetParticipantBank: No Phrases' Bank found for Participant (%s)\n", pParticipant->GetName().c_str()));
	return NULL;
}

//! ----------------------------------------------------------------------
//!  void VOBankedFinished(void* pData)
//!
//!  PlayBankedPhrase after the previous banked audio stream is finished
//! ----------------------------------------------------------------------
static void VOBankedFinished(void* pData)
{
	SCallBackBankedData* pCD = (SCallBackBankedData*)pData;
	//ntPrintf("VOFinished: Bank:%s - Sfx:%s\n", *(pCD->pCurrentSentence->m_Bank), *(pCD->pCurrentSentence->m_Sfx));
	// Check if it is time to stop chatting
	if (!pCD || !pCD->pChatterBox)
	{
		// For safety during transitions
		return;
	}
	if ((pCD->iNrOfResponses)-- < 1)
	{
		// Stop Chatting
		pCD->pChatterBox->SetChattingStatus(false);
		return;
	}
	// Otherwise chat ... is this chat is still valid (i.e. not interrupted)
	if (pCD->pChatterBox->HasValidChatGUID())
		pCD->pChatterBox->PlayBankedPhrase(pCD->pTriggerID,false);
	else
	{
		pCD->pChatterBox->SetChattingStatus(false);
		DEBUG_PRINT_CHATTERBOX(("Call-back function VOBankedFinished interrupted by a more important conversation\n"));
	}
}

//! ----------------------------------------------------------
//!  SChatItem* CChatterBox::PlayBankedPhrase()
//! ----------------------------------------------------------
bool CChatterBox::PlayBankedPhrase(const char* pcEventName, bool bCheckChatItemProbability)
{
	CEntity* pobNewChatter;
	SChatItem* pPhrase;
	SSubChatItemBank* pParticipantBank;

	// Initialise chat status
	this->SetChattingStatus(false);

	if (IsPlayingAnimEventSound())
		return false;

	// NULL check
	if (!pcEventName)
		return false;

	// Pick a Chatter (The one who will say the sentence)
	bool bAnotherPhraseNeeded = false;
	pobNewChatter=this->PickChatter(NULL,&bAnotherPhraseNeeded);
	if (!pobNewChatter)
		return false; // No chatter available
	
	this->m_pLastChatter = pobNewChatter;	// Record last chatter;

	// Get Participant's Phrases Bank
	pParticipantBank = GetParticipantBank(pcEventName, pobNewChatter);
	if (!pParticipantBank)
	{
		DEBUG_PRINT_CHATTERBOX(("PlayBankedPhrase: A NULL Phrases Bank retrieved from Participant (%s)\n", ntStr::GetString(pobNewChatter->GetName()))); 
		return false;
	}

	pPhrase = PickBankedPhrase(pParticipantBank);
	if (!pPhrase)
		return false;

	// Check probability
	if (bCheckChatItemProbability)
	{
		float fRandProb = drandf(1.0f);
		if (pPhrase->m_fProbability < fRandProb)
    	{
    		DEBUG_PRINT_CHATTERBOX(("Banked Phrase (%s) not played due to low probability [%.2f/%.2f] in ChatterBox (%s)\n", ntStr::GetString(pPhrase->m_Sfx), pPhrase->m_fProbability, fRandProb, this->GetName()));
    		return false;
		}
	}

	// Record last chatter and phrase
	this->m_pLastChatter = pobNewChatter;
	pParticipantBank->pLastPhrase = pPhrase;

	// Play it again Sam...
	
	DEBUG_PRINT_CHATTERBOX(("\n--------------------------------------------------------\n"));
	DEBUG_PRINT_CHATTERBOX(("Entity (%s) plays Phrase (Bank: %s - Sfx: %s)\n",ntStr::GetString(pobNewChatter->GetName()), ntStr::GetString(pPhrase->m_Bank), ntStr::GetString(pPhrase->m_Sfx)));

	pobNewChatter->GetEntityAudioChannel()->SetCallback(&VOBankedFinished, (void *)(this->m_CallBackBankedData));
	bool bOK = pobNewChatter->GetEntityAudioChannel()->Play(CHANNEL_VOICE_HIGHPRI, pPhrase->m_Bank, pPhrase->m_Sfx);
	if (bOK && !pPhrase->m_hsAnimation.IsNull() )
	{
		// Play Partial Anim
		pobNewChatter->GetMovement()->PlayPartialAnim( pPhrase->m_hsAnimation ); // Attempts to play an anim on the controller.  Will silently return if it can't play it.
	}

	// campf - handle subtitles if there are any
	if (pPhrase->m_bHasSubtitles)
	{
		CSubtitleMan::Get().Play( Util::Upppercase( Util::NoExtension( Util::BaseName(pPhrase->m_Sfx.GetString()) ) ) );
	}
	
	this->SetChattingStatus(bOK);
	return bOK;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBox::StartBankedChatting
*
*	DESCRIPTION		
*
***************************************************************************************************/
///*
bool CChatterBox::StartBankedChatting (const char* pcEventName, CEntity* pEntSource)
{
	// NULL pointers checking
    if ( !pcEventName )
	{
		ntAssert_p( 0, ( "Request to trigger a NULL pointer (string) Event to ChatterBox.\n" ) );
		return false;
	}
	if ( !pEntSource )
	{
		ntAssert_p( 0, ( "Request to trigger into a ChatterBox an Event whose Source is NULL.\n" ) );
		return false;
	}
	// Pick a random number of responses
	this->m_CallBackBankedData->iNrOfResponses = (drand() % this->GetBankedResponsesLimit()) + 1;
	this->m_CallBackBankedData->pTriggerID = pcEventName;

	DEBUG_PRINT_CHATTERBOX(("Number of Responses (Random): %d - for event (%s)\n", this->m_CallBackBankedData->iNrOfResponses,pcEventName));

	// Remember the source entity
	this->m_pLastChatter = pEntSource; // To prevent null pointer first time is accessed (ntPrintf())
	this->m_pSource = pEntSource;
	 
	// Start the Banked Chatting

	this->PlayBankedPhrase(pcEventName, true);

	return true;
}

//--------------------------------------------------
// Schedule
//--------------------------------------------------
bool CChatterBox::Schedule( SChatItem* pSCI, CEntity* pSpeaker )
{
	if (!pSCI || !pSpeaker)
		return false;

	// Calculate a random time between Min and Max
	float fDiff = pSCI->m_fCheeringDelayMax - pSCI->m_fCheeringDelayMin;
	float fRand = drandf(fDiff) + pSCI->m_fCheeringDelayMin;
	pSCI->m_Delay = fRand;
	// Create a Chat Item and add it to the ScheduleList
	SScheduledChatItem* pSSchedChatItme = NT_NEW_CHUNK(Mem::MC_MISC) SScheduledChatItem(pSCI,pSpeaker,fRand);

	if (pSSchedChatItme)
	{
		m_listScheduledChatItems.push_back(pSSchedChatItme);
		DEBUG_PRINT_CHATTERBOX(("Creating a Scheduled Chat Item (%s)- AI:(%s) - Delay:(%.3f)\n",ntStr::GetString(pSCI->m_Sfx),  ntStr::GetString(pSpeaker->GetName()), fRand));
		return true;
	}
	else
	{
		ntAssert_p(0,("CChatterBox::Schedule = NT_NEW_CHUNK -> Failure to allocate a new Scehduled Chat Item!!!"));
		return false;
	}
}

//--------------------------------------------------
// Update
//--------------------------------------------------
void CChatterBox::Update( float fTimeChange )
{

	// Update Trigger Time Statistics

	ntstd::List<SChatTrigger*>::iterator obItTrigger	= m_Triggers.begin();
	ntstd::List<SChatTrigger*>::iterator obItTriggerEnd	= m_Triggers.end();
	for ( ; obItTrigger != obItTriggerEnd; obItTrigger++ )
	{
		SChatTrigger* pSCT = (*obItTrigger);
		
		if ( pSCT->m_fTimeThreshold > pSCT->m_fCurrentTime )
		{
			pSCT->m_bTimeThresholdSurpassed = false;
			pSCT->m_fCurrentTime += fTimeChange;
		}
		else
		{
			pSCT->m_bTimeThresholdSurpassed = true;
		}
	}

    // Go through the list of scheduled chat items
	// and update its time counter.

	if (m_listScheduledChatItems.empty())
	{
		SetHasScheduledChatItems(false);
		return;
	}

	SetHasScheduledChatItems(true);

	ntstd::List<SScheduledChatItem*>::iterator obIt		= m_listScheduledChatItems.begin();
	ntstd::List<SScheduledChatItem*>::iterator obItEnd	= m_listScheduledChatItems.end();
	for ( ; obIt != obItEnd;  )
	{
		SScheduledChatItem* pSchedCI = (*obIt);
		SChatItem*			pSCI	 = pSchedCI->m_pSChatItem;
		
		if ( pSchedCI->m_fCurrentTime > pSchedCI->m_Delay )
		{
			// The time is right to play the VO
			DEBUG_PRINT_CHATTERBOX(("AI:(%s) plays scheduled chat item(Bank: %s - Sfx: %s)\n",ntStr::GetString(pSchedCI->m_pSpeaker->GetName()), ntStr::GetString(pSCI->m_Bank), ntStr::GetString(pSCI->m_Sfx)));
			bool bOK = pSchedCI->m_pSpeaker->GetEntityAudioChannel()->Play(CHANNEL_VOICE_HIGHPRI, pSCI->m_Bank, pSCI->m_Sfx);
			if (bOK && !pSCI->m_hsAnimation.IsNull() )
			{
				// Play Partial Anim
				pSchedCI->m_pSpeaker->GetMovement()->PlayPartialAnim( pSCI->m_hsAnimation ); // Attempts to play an anim on the controller.  Will silently return if it can't play it.
			}

			// campf - handle subtitles if there are any
			if (pSCI->m_bHasSubtitles)
			{
				CSubtitleMan::Get().Play( Util::Upppercase( Util::NoExtension( Util::BaseName(pSCI->m_Sfx.GetString()) ) ) );
			}

			// Delete the entry
			obIt = m_listScheduledChatItems.erase(obIt);
			NT_DELETE_CHUNK( Mem::MC_MISC, pSchedCI );

			PlayResponse(pSCI);
		}
		else
		{
			pSchedCI->m_fCurrentTime += fTimeChange;
			++obIt;
		}
	}
}

//--------------------------------------------------
//! StopChats
//--------------------------------------------------
void CChatterBox::StopParticipantAudio( CEntity* pEnt ) 
{ 
	if (pEnt && pEnt->GetEntityAudioChannel() && pEnt->GetEntityAudioChannel()->IsPlaying(CHANNEL_VOICE_HIGHPRI))
		pEnt->GetEntityAudioChannel()->Stop(CHANNEL_VOICE_HIGHPRI);
}

//--------------------------------------------------
//! StopChats
//--------------------------------------------------
void CChatterBox::StopChats( void )
{
	if (!IsActive()) 
		return;

	// Go through the list of participants and stop their chats
	ntstd::List<CEntity*>::iterator obItPar		= m_listParticipants.begin();
	ntstd::List<CEntity*>::iterator obItEndPar	= m_listParticipants.end();
	for ( ; obItPar != obItEndPar; ++obItPar )
	{
		CEntity* pEnt = (*obItPar);
		StopParticipantAudio(pEnt);
	}

	// Make sure that all the dynamic memory is released
	ntstd::List<SScheduledChatItem*>::iterator obIt		= m_listScheduledChatItems.begin();
	ntstd::List<SScheduledChatItem*>::iterator obItEnd	= m_listScheduledChatItems.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		SScheduledChatItem* pSchedCI = (*obIt);
		CEntity* pEnt = pSchedCI->m_pSpeaker;
		StopParticipantAudio(pEnt);

		NT_DELETE_CHUNK( Mem::MC_MISC, *obIt );
	}
	m_listScheduledChatItems.clear();
}

//! ----------------------------------------------------------
//!  VOCallBackAnimEvent(void* pData)
//! ----------------------------------------------------------

static void VOCallBackAnimEvent(void* pData)
{
	SCallBackAnimEvent* pCD = (SCallBackAnimEvent*)pData;
	if ( pCD && pCD->pChatterBox )
		pCD->pChatterBox->SetPlayingAnimEventSound(false);
	else
		DEBUG_PRINT_CHATTERBOX(("(ERROR) Call-back function VOCallBackAnimEvent got NULL data\n"));
}

//--------------------------------------------------
//! TriggerAnimEventChat
//--------------------------------------------------
bool CChatterBox::TriggerAnimEventSound( CEntity* pSpeaker, CAnimEventSound* pAnimEventSound )
{
	if (!IsActive()) 
		return false;
	
	if (!pSpeaker || !pAnimEventSound || !pSpeaker->GetEntityAudioChannel() )
		return false;
	
	// Stop All the Conversations
	StopChats();

	pSpeaker->GetEntityAudioChannel()->SetCallback(&VOCallBackAnimEvent, (void *)(this->m_CallBackAnimEvent));
	bool bOK = pSpeaker->GetEntityAudioChannel()->Play(	pAnimEventSound->m_eChannel,
														pAnimEventSound->m_obSoundBank,
														pAnimEventSound->m_obSoundCue,
														CPoint(0.0f,0.8f,0.0f),
														pAnimEventSound->m_fVolume,
														pAnimEventSound->m_fPitch

														);

	// campf - handle subtitles if there are any
	if (pAnimEventSound->m_bHasSubtitles)
	{
		CSubtitleMan::Get().Play( Util::Upppercase( Util::NoExtension( Util::BaseName(pAnimEventSound->m_obSoundCue.GetString()) ) ) );
	}

	// Set up the playing anim event flag
	m_bPlayingAnimEventSound = bOK;

	DEBUG_PRINT_CHATTERBOX(("ChatterBox: %s - %s the AnimEventSound: %s\n",	this->GetName(),
																			bOK ? "successfully played" : "failed to play",
																			ntStr::GetString(pAnimEventSound->m_obSoundCue)));

	return bOK;
}

//--------------------------------------------------
//! SetStatistic
//--------------------------------------------------
bool  CChatterBox::SetStatistic( CHashedString hsTriggerEvent, int iValue )
{
	if (m_listStatistics.empty())
		return false;
	if (iValue<0)
	{
		DEBUG_PRINT_CHATTERBOX(("SetStatistic: Trying to set a negative statistic value: [%d] - ChatterBox: [%s] - Trigger [%s]\n",iValue, this->GetName(),ntStr::GetString(hsTriggerEvent) ));
		return false;
	}

	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obIt		= m_listStatistics.begin();
	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obItEnd	= m_listStatistics.end();
	for ( ; obIt != obItEnd; ++obIt )
	{
		SChatterBoxStatisticPair* pStat = *obIt;
		if ( pStat )
		{
			pStat->uiCount = iValue;
			DEBUG_PRINT_CHATTERBOX(("Statistic Set: ChatterBox: [%s] - Trigger [%s] - Count: [%d]\n",this->GetName(),ntStr::GetString(pStat->hsTriggerEvent), pStat->uiCount ));
			return true;
		}
	}
	return false;
}

//--------------------------------------------------
//! GetStatistic
//--------------------------------------------------
int  CChatterBox::GetStatistic( CHashedString hsTriggerEvent, bool* bFound ) const
{
	*bFound = false;

	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obIt		= m_listStatistics.begin();
	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obItEnd	= m_listStatistics.end();
	for ( ; obIt != obItEnd; ++obIt )
	{
		SChatterBoxStatisticPair* pStat = *obIt;
		if ( pStat && pStat->hsTriggerEvent == hsTriggerEvent )
		{
	//		DEBUG_PRINT_CHATTERBOX(("Statistic Get: ChatterBox: [%s] - Trigger [%s] - Count: [%d]\n",this->GetName(),ntStr::GetString(pStat->hsTriggerEvent), pStat->uiCount ));
			*bFound = true;
			return pStat->uiCount;
		}
	}
	return -1;
}

//--------------------------------------------------
//! DisableChecksOnAI
//--------------------------------------------------
void CChatterBox::DisableChecksOnAI(CEntity* pEnt, bool bDisabled)
{
	// Checks done by the CChatterBoxMan

	ntstd::List<CEntity*>::iterator obIt		= m_listNoChecksParticipants.begin();
	ntstd::List<CEntity*>::iterator obItEnd	= m_listNoChecksParticipants.end();

	for ( ; obIt != obItEnd; ++obIt )
	{
		CEntity* pEntAI = *obIt;
		if ( pEntAI ==  pEnt )
		{
			if (!bDisabled)
			{			
				m_listNoChecksParticipants.erase( obIt );
				DEBUG_PRINT_CHATTERBOX(("Enabling ChatterBox checkings on AI [%s]. ChatterBox: [%s] \n",ntStr::GetString(pEnt->GetName()), this->GetName() ));
				return;
			}
			else
			{
				DEBUG_PRINT_CHATTERBOX(("(Warning!) Trying to disable ChatterBox checkings on AI [%s] that has them already disabled. ChatterBox: [%s] \n",ntStr::GetString(pEnt->GetName()), this->GetName() ));
				return;
			}
		}
	}

	if (!bDisabled)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) Trying to enable ChatterBox checkings on AI [%s] that has them already enabled. ChatterBox: [%s] \n",ntStr::GetString(pEnt->GetName()), this->GetName() ));
		return;
	}

	// Everything fine for disabling checkings on this AI

	DEBUG_PRINT_CHATTERBOX(("Disabling ChatterBox checkings on AI [%s]. ChatterBox: [%s] \n",ntStr::GetString(pEnt->GetName()), this->GetName() ));
	m_listNoChecksParticipants.push_back(pEnt);
	
}


//--------------------------------------------------
//! HasChatterBoxChecksDisabled
//--------------------------------------------------
bool CChatterBox::HasChatterBoxChecksDisabled(CEntity* pEnt)
{
	if (!pEnt || !pEnt->IsAI() || !this->IsActive())
		return false;

	ntstd::List<CEntity*>::iterator obIt	= m_listNoChecksParticipants.begin();
	ntstd::List<CEntity*>::iterator obItEnd	= m_listNoChecksParticipants.end();

	for ( ; obIt != obItEnd; ++obIt )
	{
		CEntity* pEntAI = *obIt;
		if ( pEntAI ==  pEnt )
			return true;
	}

	return false;	
}





