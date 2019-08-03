/***************************************************************************************************
*
*	FILE			ChatterBoxMan.h
*
*	DESCRIPTION		
*
*	AUTHOR: Dario L. Sancho-Pradel	
*
***************************************************************************************************/


//**************************************************************************************
//	Includes files.
//**************************************************************************************

#include "ChatterBoxMan.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/luaglobal.h"
#include "core/visualdebugger.h"

#define TAG_LAST_STATISTIACAL_TRIGGER (-1024)

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//------------------------------------------------------------------------------------------
// CChatterBoxMan Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CChatterBoxMan)
	LUA_EXPOSED_METHOD(Trigger, Trigger, "Triggers a chatterbox event", "string event, entity source", "The name of the event to trigger|The source entity for this event")
	LUA_EXPOSED_METHOD(IsEnabled, IsEnabled, "Returns if the chatterboxman is enabled", "void", "void")
	LUA_EXPOSED_METHOD(Enable, Enable, "Enables the chatterboxman","void","void")
	LUA_EXPOSED_METHOD(Disable, Disable, "Disables the chatterboxman","void","void")
	//LUA_EXPOSED_METHOD(ActiveChatterbox, ActiveChatterbox, "Bool: Is the selected chatterbox active?", "string name", "The name of the Chatterbox you want to check if is active")
	LUA_EXPOSED_METHOD(Activate, ActivateChatterBox, "Activates a particular chatterbox", "string name", "The name of the chatterbox to be activated")
	LUA_EXPOSED_METHOD(AddParticipant, AddParticipant, "Adds a participant to the active chatterbox", "entity participant", "The participant to be added to the chatterbox")
	LUA_EXPOSED_METHOD(RemoveParticipant, RemoveParticipant, "Removes a participant from te active chatterbox", "entity participant", "The participant to be removed from the chatterbox")
	LUA_EXPOSED_METHOD(AddParticipantToChatterBox, AddParticipantToChatterBox, "Adds a participant to the selected chatterbox", "entity participant, string chatterbox", "The participant to be added to the chatterbox|The name of the chatterbox")
	LUA_EXPOSED_METHOD(RemoveParticipantFromChatterBox, RemoveParticipantFromChatterBox, "Removes a participant from the selected chatterbox", "entity participant, string chatterbox", "The participant to be removed from the chatterbox|The name of the chatterbox")
	LUA_EXPOSED_METHOD(SetBankedResponsesLimit, SetBankedResponsesLimit, "Set the maximum number of responses in a Banked conversation", "unsigned integer number", "The maximum number of responses in a Banked conversation")
	LUA_EXPOSED_METHOD(DbgPrint, DbgPrint, "Debug: Prints the participants list from the current chatterbox", "", "")
	LUA_EXPOSED_METHOD(DebugRender, DebugRender, "Debug: Prints the participants list from the current chatterbox", "", "")
	LUA_EXPOSED_METHOD(ActivateSubChatterBox, ActivateSubChatterBox, "Forces the activation of the selected subchatterbox", "string", "SubChatterBoxName")
	LUA_EXPOSED_METHOD(DisableChecksOnAI, DisableChecksOnAI, "Enables or disables the ChatterBox checks on an AI", "CEntity, bool", "AI, true/false")
	LUA_EXPOSED_METHOD(SetChatGroupID,SetChatGroupID, "","", "")
	
//	LUA_EXPOSED_METHOD(GetCurrentStatistic,GetCurrentStatistic,"Gets a ChatterBox Statistic", "hashedstring, bool*", "")
	
LUA_EXPOSED_END(CChatterBoxMan)

/***************************************************************************************************
*
*	CLASS			CChatterBoxMan
*
*	DESCRIPTION		C++ porting of the LUA ChatterBox Manager found in chatterbox2.lua
*
***************************************************************************************************/
CChatterBoxMan::CChatterBoxMan() : 
	 m_bRender(false), m_ActiveChatterBox(NULL), m_PlayingFromChatterBox(NULL), m_LastPlayingChatterBox(NULL), m_Enabled(false),
	 m_uiCurrentChatGUID(0)
{
	ATTACH_LUA_INTERFACE(CChatterBoxMan);
	CLuaGlobal::Get().State().GetGlobals().Set("CChatterBoxMan", this);
}

CChatterBoxMan::~CChatterBoxMan()
{
	this->LevelUnload();
}

void CChatterBoxMan::LevelUnload ( void )
{
	m_ChatterBoxes.clear();
}
/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::DebugRender
*
*	DESCRIPTION		Debug: Prints on the screen debug information for the chatterbox
*
***************************************************************************************************/
void CChatterBoxMan::DebugRender(void)
{
#ifdef _CHATTERBOX_DEBUG_RENDER

	static const float iYinc = 15.0f;
	static const float iXorigin = 15.0f;
	float	fY = 10.0f;
	if (!this->m_ActiveChatterBox) 
	{
		// There isn't a valid active ChatterBox
		g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"No valid Active Chatter Box!!!");
		return;
	}
// ------ Print Entity, Last Chatter, etc...
	char cActive[] = "Y";
	char cChatting[] = "Y";

	#define SAY_YES_NO(x) ( x ? "Y" : "N" )

	if (this->m_ActiveChatterBox->IsActive())
	{  cActive[0] = 'Y';
	} else { cActive[0] = 'N'; }
	if (this->m_ActiveChatterBox->IsChatting())
	{ cChatting[0] = 'Y';
	} else { cChatting[0] = 'N'; }
	
	g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"CHATTERBOX: (%s) - Active[%s] - Chatting[%s] - Scheduled Items (%s) - PLaying AnimEventSound (%s)", 
														m_ActiveChatterBox->GetName(),
														SAY_YES_NO(m_ActiveChatterBox->IsActive()),
														SAY_YES_NO(m_ActiveChatterBox->IsChatting()),
														SAY_YES_NO(m_ActiveChatterBox->HasScheduledChatItems()),
														SAY_YES_NO(m_ActiveChatterBox->IsPlayingAnimEventSound())
														);
	fY+=iYinc;
	g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"-----------"); fY+=iYinc;
	// Statistics
	const CBStatsList* pStatList = m_ActiveChatterBox->GetStatisticsList_const();
	if (pStatList)
	{
		g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"Active Statistics"); fY+=iYinc;
		ntstd::List<SChatterBoxStatisticPair*>::const_iterator obIt		= pStatList->begin();
		ntstd::List<SChatterBoxStatisticPair*>::const_iterator obItEnd	= pStatList->end();
		for ( ; obIt != obItEnd; ++obIt )
		{
			SChatterBoxStatisticPair* pStat = *obIt;
			g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"[%d] - %s",pStat->uiCount, ntStr::GetString(pStat->hsTriggerEvent)); fY+=iYinc;
		}
		g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"-----------"); fY+=iYinc;
	}

	// Generic CBs
	const ntstd::List<CChatterBox*>* plistGenericCB = m_ActiveChatterBox->GetGenericChatterBoxes();
	if ( plistGenericCB )
		g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"Associated Generic ChatterBoxes: (%d)", plistGenericCB->size()); fY+=iYinc;
	if ( m_LastPlayingChatterBox )
		g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"Lastest ChatterBox: %s - Chatting[%s] - Scheduled Items [%s]", 
													m_LastPlayingChatterBox->GetName(),
													SAY_YES_NO(m_LastPlayingChatterBox->IsChatting()),
													SAY_YES_NO(m_LastPlayingChatterBox->HasScheduledChatItems())
); fY+=iYinc;
	
	g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"-----------"); fY+=iYinc;
	if (!this->m_ActiveChatterBox->GetSource())
	{
		g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"Source Entity: NONE");
	} else 
	{
		g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"Source Entity: %s",this->m_ActiveChatterBox->GetSource()->GetName().c_str());
	}
	fY+=iYinc;
	if (!this->m_ActiveChatterBox->GetLastChatter())
	{
		g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"LastChatter: NONE");
	} else 
	{
		g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"LastChatter: %s",this->m_ActiveChatterBox->GetLastChatter()->GetName().c_str());
	}
	fY+=iYinc;
	// Trigger Info
	g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"Trigger Count/Time Info"); fY+=iYinc;
	g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"-----------------------"); fY+=iYinc;
	const ntstd::List<SChatTrigger*>* pTriggerList = m_ActiveChatterBox->GetTriggerList();
	ntstd::List<SChatTrigger*>::const_iterator obItTrigger		= pTriggerList->begin();
	ntstd::List<SChatTrigger*>::const_iterator obItTriggerEnd	= pTriggerList->end();
	for ( ; obItTrigger != obItTriggerEnd; ++obItTrigger )
	{
		SChatTrigger* pSCT = (*obItTrigger);
		g_VisualDebug->Printf2D(iXorigin,fY,DC_YELLOW,0,"%s:",ntStr::GetString(pSCT->m_TriggerID));
		g_VisualDebug->Printf2D(iXorigin+150,fY,DC_WHITE,0,"Count(%d/%d) - Time(%.3f/%.3f)",pSCT->m_uiCurrentCount,pSCT->m_uiCountThreshold,pSCT->m_fCurrentTime,pSCT->m_fTimeThreshold);
		fY+=iYinc;
	}

	// Print List of Participants
	const ntstd::List<CEntity*>* plistRemainingParticipantsList = m_ActiveChatterBox->GetParticipantsList();
	if (!plistRemainingParticipantsList)
	{
		// Null participants list passed
		g_VisualDebug->Printf2D(iXorigin,fY,DC_RED,0,"Null participants list for ActiveChatterBox(%s)",m_ActiveChatterBox->GetName());
		fY+=iYinc;
		return;
	}
	ntstd::List<CEntity*>::const_iterator obItEnd = plistRemainingParticipantsList->end();

	g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"PARTICIPANTS LIST"); fY+=iYinc;
	g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"-----------------"); fY+=iYinc;
	unsigned int uiIdx = 0;
	if (plistRemainingParticipantsList->empty()) 
	{
		g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"NO PARTICIPANTS LEFT!");
		fY+=iYinc;
	} 
	else
	{
		for (ntstd::List<CEntity*>::const_iterator obIt = plistRemainingParticipantsList->begin(); obIt!=obItEnd; ++obIt)
		{
			bool bChattingAI = ( (m_ActiveChatterBox->IsChatting() && m_ActiveChatterBox->GetLastChatter() == (*obIt)) || (m_LastPlayingChatterBox && m_LastPlayingChatterBox->IsChatting() && m_LastPlayingChatterBox->GetLastChatter() == (*obIt)));
			CEntity* pEntAI = (*obIt);
			g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"%d - Participant: (%s) %s %s",	
						++uiIdx,
						pEntAI->GetName().c_str(), 
						pEntAI->ToCharacter()->IsDead() ? " - DEAD!" : "",
						bChattingAI ? "CHATTING" : ""
						);
			if (bChattingAI)
				g_VisualDebug->Printf3D(pEntAI->GetPosition()+CPoint(0,1.9f,0),DC_YELLOW,0,"CHATTING");
			fY+=iYinc;
		}
	}
	//ntPrintf("-----------------\n");
	// ------ Print List of Banks
	g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"BANKED LIST"); fY+=iYinc;
	g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"-----------"); fY+=iYinc;
	
	// Go along the Different events
	ntstd::List<SChatTrigger*>::const_iterator obItEventEnd = pTriggerList->end();
	for (ntstd::List<SChatTrigger*>::const_iterator obItEvent = pTriggerList->begin(); obItEvent!=obItEventEnd; ++obItEvent)
	{
		unsigned int uiIdxBank = 0;
		const ntstd::List<SSubChatItemBank*>* plistBanks = &((*obItEvent)->m_listParticipantsBanks);
		if (!plistBanks)
		{
			// Null List of banks passed
			g_VisualDebug->Printf2D(iXorigin,fY,DC_RED,0,"Null List of banks passed for ActiveChatterBox(%s)",m_ActiveChatterBox->GetName());
			return;
		}
		ntstd::List<SSubChatItemBank*>::const_iterator obItBanksEnd = plistBanks->end();
		for (ntstd::List<SSubChatItemBank*>::const_iterator obIt = plistBanks->begin(); obIt!=obItBanksEnd; ++obIt)
		{
			if (!(*obIt)->pParticipant)
			{
				g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"Event : %s - BANK [%d] - Participant : NO_PARTICIPANT", ntStr::GetString((*obItEvent)->m_TriggerID),uiIdxBank);
			} 
			else 
			{
				g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"Event : %s - BANK [%d] - Participant : %s", ntStr::GetString((*obItEvent)->m_TriggerID),uiIdxBank,((*obIt)->pParticipant->GetName().c_str()));
			}
			fY+=iYinc;
			g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"------------\n"); fY+=iYinc;
			unsigned int uiIdxItem = 0;
			ntstd::List<SChatItem*>::const_iterator obItChatItemEnd = (*obIt)->listBankedChatItems.end();
			for (ntstd::List<SChatItem*>::const_iterator obItChatItem = (*obIt)->listBankedChatItems.begin(); obItChatItem!=obItChatItemEnd; ++obItChatItem)
			{
				SChatItem* pSelChatItem = *(obItChatItem);
				g_VisualDebug->Printf2D(iXorigin,fY,DC_WHITE,0,"Bank[%d] : Position[%d] - ChatItem (%s - %s)",uiIdxBank, ++uiIdxItem, ntStr::GetString(pSelChatItem->m_Bank), ntStr::GetString(pSelChatItem->m_Sfx));
				fY+=iYinc;
			}
			uiIdxBank++;
			g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"------------"); fY+=iYinc;
		}
		g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"------------"); fY+=iYinc;
	}

	// Test - To be removed
		static int iCountTemp = 0;
		bool ok1,ok2;
		if (iCountTemp++ < 150)
		{
			int iEKO = GetCurrentStatistic("EnemyKO", &ok1);
			int ED = GetCurrentStatistic("EnemyDeath", &ok2);
			g_VisualDebug->Printf2D(iXorigin,fY,DC_YELLOW,0,"STAT REPORT: EnemyKO [%d], EnemyDeath [%d]", iEKO, ED); fY+=iYinc;

			g_VisualDebug->Printf2D(iXorigin,fY,DC_PURPLE,0,"------------"); fY+=iYinc;
		}
		if (iCountTemp > 300)
			iCountTemp = 0;

#endif // _CHATTERBOX_DEBUG_RENDER
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::DbgPrint
*
*	DESCRIPTION		Debug: Prints the participants list from the current chatterbox
*
***************************************************************************************************/
void CChatterBoxMan::DbgPrint(void)
{
#ifdef _CHATTERBOX_DEBUG_PRINT

	if (!this->m_ActiveChatterBox)
	{
		// There isn't a valid active ChatterBox
		return;
	}
// ------ Print Entity, Last Chatter, etc...
	char cActive[] = "Y";
	char cChatting[] = "Y";

	if (this->m_ActiveChatterBox->IsActive())
	{  cActive[0] = 'Y';
	} else { cActive[0] = 'N'; }
	if (this->m_ActiveChatterBox->IsChatting())
	{ cChatting[0] = 'Y';
	} else { cChatting[0] = 'N'; }

	ntPrintf("\nGENERAL INFO FOR CHATTERBOX: (%s)\n", m_ActiveChatterBox->GetName());
	ntPrintf("----------------------------\n");
	if (!this->m_ActiveChatterBox->GetSource())
	{
		ntPrintf("Source Entity: NONE\n");
	} else 
	{
		ntPrintf("Source Entity: %s\n",this->m_ActiveChatterBox->GetSource()->GetName().c_str());
	}
	if (!this->m_ActiveChatterBox->GetLastChatter())
	{
		ntPrintf("LastChatter: NONE\n");
	} else 
	{
		ntPrintf("LastChatter: %s\n",this->m_ActiveChatterBox->GetLastChatter()->GetName().c_str());
	}
	ntPrintf("Active [%s] - Chatting [%s]\n",cActive,cChatting);
// ------ Print List of Participants
	const ntstd::List<CEntity*>* plistRemainingParticipantsList = m_ActiveChatterBox->GetParticipantsList();
	if (!plistRemainingParticipantsList)
	{
		// Null participants list passed
		ntPrintf("DbgPrint: Null participants list for ActiveChatterBox(%s)\n",m_ActiveChatterBox->GetName());
		return;
	}
	ntstd::List<CEntity*>::const_iterator obItEnd = plistRemainingParticipantsList->end();

	ntPrintf("PARTICIPANTS LIST\n");
	ntPrintf("-----------------\n");
	unsigned int uiIdx = 0;
	for (ntstd::List<CEntity*>::const_iterator obIt = plistRemainingParticipantsList->begin(); obIt!=obItEnd; ++obIt)
	{
		ntPrintf("%d - Participant: (%s)\n",++uiIdx,(*obIt)->GetName().c_str());
	}
	ntPrintf("-----------------\n");
// ------ Print List of Banks
	ntPrintf("BANKED LISTS\n");
	ntPrintf("------------\n");
	
	// Go along the Different events
	const ntstd::List<SChatTrigger*>* pTriggerList = this->m_ActiveChatterBox->GetTriggerList();
	ntstd::List<SChatTrigger*>::const_iterator obItEventEnd = pTriggerList->end();
	for (ntstd::List<SChatTrigger*>::const_iterator obItEvent = pTriggerList->begin(); obItEvent!=obItEventEnd; ++obItEvent)
	{
		unsigned int uiIdxBank = 0;
		const ntstd::List<SSubChatItemBank*>* plistBanks = &((*obItEvent)->m_listParticipantsBanks);
		if (!plistBanks)
		{
			// Null List of banks passed
			ntPrintf("DbgPrint: Null List of banks passed for ActiveChatterBox(%s)\n",m_ActiveChatterBox->GetName());
			return;
		}
		ntstd::List<SSubChatItemBank*>::const_iterator obItBanksEnd = plistBanks->end();
		for (ntstd::List<SSubChatItemBank*>::const_iterator obIt = plistBanks->begin(); obIt!=obItBanksEnd; ++obIt)
		{
			if (!(*obIt)->pParticipant)
			{
				ntPrintf("Event : %s - BANK [%d] - Participant : NO_PARTICIPANT\n",*((*obItEvent)->m_TriggerID),uiIdxBank);
			} 
			else 
			{
				ntPrintf("Event : %s - BANK [%d] - Participant : %s\n",*((*obItEvent)->m_TriggerID),uiIdxBank,((*obIt)->pParticipant->GetName().c_str()));
			}
			ntPrintf("------------\n");
			unsigned int uiIdxItem = 0;
			ntstd::List<SChatItem*>::const_iterator obItChatItemEnd = (*obIt)->listBankedChatItems.end();
			for (ntstd::List<SChatItem*>::const_iterator obItChatItem = (*obIt)->listBankedChatItems.begin(); obItChatItem!=obItChatItemEnd; ++obItChatItem)
			{
				SChatItem* pSelChatItem = *(obItChatItem);
				ntPrintf("Bank[%d] : Position[%d] - ChatItem (%s - %s)\n",uiIdxBank, ++uiIdxItem, *(pSelChatItem->m_Bank), *(pSelChatItem->m_Sfx));
			}
			uiIdxBank++;
			ntPrintf("------------\n");
		}
		ntPrintf("------------\n");
	}

#endif // _CHATTERBOX_DEBUG_PRINT

}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::AddChatterBox
*
*	DESCRIPTION		Adds a ChatterBox to the ChatterBox Manager
*
***************************************************************************************************/
bool CChatterBoxMan::AddChatterBox (CChatterBox* pobNewCB)
{
	//if (!m_Enabled)
	//{
	//	DEBUG_PRINT_CHATTERBOX(("(Warning!) AddChatterBox: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
	//	return false;
	//}

	// NULL poniter checking
	if (!pobNewCB) 
	{
		// Don't change Active ChatterBox
		user_error_p( 0, ( "Big Problem. You are trying to add a ChatterBox that doesn't exist.\n" ) );
		return false;
	}
	// Check that this ChatterBox is not already in list
	if (!(this->GetChatterBox(pobNewCB->GetName(),false)))
	{
		if (pobNewCB->IsGeneric())
		{
			DEBUG_PRINT_CHATTERBOX(("CChatterBoxMan: Adding and ACTIVATING new Generic ChatterBox (%s)\n",pobNewCB->GetName()));
			pobNewCB->Activate(true);
			m_GenericChatterBoxes.push_back(pobNewCB);
		}
		else
		{
			// It is a new ChatterBox. Put it in.
			DEBUG_PRINT_CHATTERBOX(("CChatterBoxMan: Adding new ChatterBox (%s)\n",pobNewCB->GetName()));
			m_ChatterBoxes.push_back(pobNewCB);
			// Activate it if it is the Initial one
			if (pobNewCB->IsInitialChatterBox())
			{
				DEBUG_PRINT_CHATTERBOX(("ChatterBoxMan:AddChatterBox -> ChatterBox (%s) is INITIAL.\n",pobNewCB->GetName()));
				DEBUG_PRINT_CHATTERBOX(("ChatterBoxMan:AddChatterBox -> No auto-activation of Chatterboxes. Waiting for a Activate() LUA command\n"));
				this->ActivateChatterBox(pobNewCB->GetName());
			}
		}
		return true;
	}
	// The ChatterBox was already in the list. Warning time...
	user_warn_p( 0, ( "Warning. You are trying to add a ChatterBox that is already in the list.\n" ) );
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::RemoveChatterBox
*
*	DESCRIPTION		Removes a ChatterBox from the ChatterBox Manager
*
***************************************************************************************************/
bool CChatterBoxMan::RemoveChatterBox (CChatterBox* pobCB)
{
	if (!m_Enabled)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) RemoveChatterBox: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
		return false;
	}

	// NULL poniter checking
	if (!pobCB) 
	{
		// Don't change Active ChatterBox
		ntAssert_p( 0, ( "CChatterBoxMan::RemoveChatterBox -> NULL parameter passed.\n" ) );
		return false;
	}

	ntstd::List<CChatterBox*>::iterator obIt = this->m_ChatterBoxes.begin();
	ntstd::List<CChatterBox*>::iterator obEndIt = this->m_ChatterBoxes.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt) == pobCB )
		{
			NT_DELETE (*obIt);
			this->m_ChatterBoxes.erase(obIt);
			return true;
		}
	}
	// ChatterBox not found. Warning time...
	DEBUG_PRINT_CHATTERBOX(( "CChatterBoxMan::RemoveChatterBox -> Warning. You have treid to remove a ChatterBox [%s] that is not in the list.\n",pobCB->GetName()));
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::GetChatterBox
*
*	DESCRIPTION		Returns a poionter to the selected chatterbox
*
***************************************************************************************************/

CChatterBox* CChatterBoxMan::GetChatterBox(const char* strNameCB, bool bAssertOn) const
{
	//if (!m_Enabled)
	//{
	//	DEBUG_PRINT_CHATTERBOX(("(Warning!) GetChatterBox: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
	//	return NULL;
	//}

	// Checking for NULL pointers
	if ( !strNameCB )
	{
		ntAssert_p( 0, ( "Request to Get a NULL pointer ChatterBox from the list.\n" ) );
		return NULL;
	}
	// Iterate throughout the ChatterBoxes list
	ntstd::List<CChatterBox*>::const_iterator obEndIt = m_ChatterBoxes.end();
	for( ntstd::List<CChatterBox*>::const_iterator obIt = m_ChatterBoxes.begin(); obIt != obEndIt; ++obIt )
	{
		CChatterBox* pCB = (*obIt);
		// If found, return it
		if (pCB && strcmp(pCB->GetName(), strNameCB )==0)
		{
			DEBUG_PRINT_CHATTERBOX(("The Chatterbox: %s - has been found in the list\n",strNameCB));
			return pCB;
		}
	}

	// If I am here means that the ChatterBox was not in the list... 
	if (bAssertOn)
	{
		// warining time...
		DEBUG_PRINT_CHATTERBOX(("(Error!)  GetChatterBox: Request to return a ChatterBox [%s] that has not been loaded.\nCheck XML and Lua data.\nNumber of Loaded ChatterBoxes: %d",strNameCB,m_ChatterBoxes.size() ));
		user_warn_p( 0, ( "(Error!) GetChatterBox: Request to return a ChatterBox [%s] that has not been loaded.\nCheck XML and Lua data.\nNumber of Loaded ChatterBoxes: %d",strNameCB,m_ChatterBoxes.size() ));
	} else
	{
		DEBUG_PRINT_CHATTERBOX(("(AssertOff) The ChatterBox requested [%s] is not in the list. Fine if called from AddChatterBox.\n",strNameCB));
	}
	return NULL;
}


/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::AddParticipant
*
*	DESCRIPTION		Adds a Participant to a selected ChatterBox
*
***************************************************************************************************/
bool CChatterBoxMan::AddParticipantToChatterBox( CEntity* pobParticipant, const char* strNameCB )
{
	if (!strNameCB || !pobParticipant)
		return false;

	// We can't do anything with an empty pointer
	if ( !pobParticipant )
	{
		DEBUG_PRINT_CHATTERBOX(( "Request to add a NULL pointer Entity to the Active ChatterBox participants' list.\n" ) );
		return false;
	}

	if (!m_Enabled)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) Participant [%s] was not added to ChatterBox [%s]. ChatterBoxMan and/or Audio Engine is Disabled!!!\n", ntStr::GetString(pobParticipant->GetName()), strNameCB  ));
		return false;
	}

	// Get the select ChatterBox from the list
	CChatterBox* pobChosenCB = this->GetChatterBox(strNameCB,false);
	if ( ! pobChosenCB )
	{
		// The ChatterBox Does not exists in the List
		DEBUG_PRINT_CHATTERBOX(("(Warning!) AddParticipantToChatterBox: the selected chatterbox [%s] has not been loaded!!!!\n",strNameCB));
		return false;
	}

	// Add the new child to our list
	return (pobChosenCB->AddParticipant(pobParticipant));
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::AddParticipant
*
*	DESCRIPTION		Adds a Participant to the Active ChatterBox
*
***************************************************************************************************/
bool CChatterBoxMan::AddParticipant( CEntity* pobParticipant )
{
	if (!m_Enabled)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) AddParticipant: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
		return false;
	}

	// We can't do anything with an empty pointer
	if ( !pobParticipant )
	{
		ntAssert_p( 0, ( "Request to add a NULL pointer to the Active ChatterBox participants' list.\n" ) );
		return false;
	}

	// Get the select ChatterBox from the list
	if ( ! this->m_ActiveChatterBox )
	{
		// The ChatterBox Does not exists in the List
		// Assert in the GetChatterBox function
		return false;
	}

	// Add the new child to our list
	return (m_ActiveChatterBox->AddParticipant(pobParticipant));
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::RemoveParticipantFromACB
*
*	DESCRIPTION		Removes a Participant from the selected ChatterBox
*
***************************************************************************************************/
bool CChatterBoxMan::RemoveParticipantFromChatterBox(const CEntity* pobParticipant, const char* strNameCB)
{
	if (!m_Enabled)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) RemoveParticipantFromChatterBox: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
		return false;
	}

	// Checking for NULL pointers
	if ( !pobParticipant )
	{
		ntAssert_p( 0, ( "Request to remove a NULL pointer Participant from a ChatterBox participants' list.\n" ) );
		return false;
	}
	
	// Get the select ChatterBox from the list
	CChatterBox* pobChosenCB = this->GetChatterBox(strNameCB);
	if ( ! pobChosenCB )
	{
		// The ChatterBox Does not exists in the List
		// Assert in the GetChatterBox function
		return false;
	}

	// Remove Participant
	return (pobChosenCB->RemoveParticipant(pobParticipant));
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::MoveToSubChatterBox
*
*	DESCRIPTION		Removes a Participant from the Active ChatterBox
*
***************************************************************************************************/
bool CChatterBoxMan::MoveToSubChatterBox(CChatterBox* pobSubCB, bool bLUAForced)
{
	if (!m_Enabled)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) MoveToSubChatterBox: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
		return false;
	}

	// NULL Pointer checked by the caller

	if (bLUAForced || (m_ActiveChatterBox->GetParticipantsListSize() <= pobSubCB->GetTransitionSize()))
	{
		// The number of participants is small enough to move to the SubChatterBox (or LUA-Forced)
		
		const ntstd::List<CEntity*>* plistRemainingParticipantsList = m_ActiveChatterBox->GetParticipantsList();
		if (!plistRemainingParticipantsList)
		{
			// Null participants list passed
			ntAssert_p(0,("CChatterBoxMan::RemoveParticipant: Null participants list passed when trying to move to the SubChatterBox\n"));
			return false;
		}
		ntstd::List<CEntity*>::const_iterator obItEnd = plistRemainingParticipantsList->end();

		DEBUG_PRINT_CHATTERBOX(("Transferring Participants from (%s) to (%s)\n", m_ActiveChatterBox->GetName(), pobSubCB->GetName()));
		DEBUG_PRINT_CHATTERBOX(("---------------------------------------------------------\n"));
		
		for (ntstd::List<CEntity*>::const_iterator obIt = plistRemainingParticipantsList->begin(); obIt!=obItEnd; ++obIt)
		{
			CEntity* pEnt = (*obIt);
			// 0 - Skip dead or paused entities

			if (pEnt->ToCharacter()->IsDead() || pEnt->IsPaused())
			{
				DEBUG_PRINT_CHATTERBOX(("Skipping AI: [%s] for being -%s-\n", ntStr::GetString(pEnt->GetName()), pEnt->ToCharacter()->IsDead() ? "Dead" : "Paused"  ));
				continue;
			}
			// 1 - Transfer remaining participants to the SubChatterBox
			pobSubCB->AddParticipant(pEnt);

			// 2 - Relate Each participant with a Phrases Bank
			pobSubCB->AssignBankToParticipant(pEnt);

			DEBUG_PRINT_CHATTERBOX(("Participant transferred: (%s)\n",ntStr::GetString((pEnt)->GetName())));
		}

		// 3 - Copy Current Statistics
//		CopyCurrentStatistic(ActivateChatterBox,pobSubCB);

		// 4 - Activate the SubChatterBox
		this->ActivateChatterBox(pobSubCB->GetName(), false);

	
		
		return true;
	}
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::ActivateSubChatterBox
*
*	DESCRIPTION		LUA command to force the activation of a subchatterbox
*
***************************************************************************************************/
bool CChatterBoxMan::ActivateSubChatterBox(CHashedString hsName)
{
	const CHashedString pobSubChatterBoxName = m_ActiveChatterBox->GetNameSubChatterBox();
	
	// Is there a SubChatterBox?
	if (pobSubChatterBoxName.IsNull())
	{	
		// Nop...
		DEBUG_PRINT_CHATTERBOX(("(Error!) LUA command ActivateSubChatterBoxMoved try to activate (%s) which is NOT a SUB-CHATTERBOX of (%s)\n", ntStr::GetString(hsName), m_ActiveChatterBox->GetName()));
		return false;
	}

	CChatterBox* pobSubCB = this->GetChatterBox(ntStr::GetString(pobSubChatterBoxName));
	if ( ! pobSubCB )
	{
		// The ChatterBox Does not exists in the List of chatterboxes
		DEBUG_PRINT_CHATTERBOX(("(Error!) LUA command ActivateSubChatterBoxMoved try to activate (%s) which is NULL!!. Active CHATTERBOX: (%s)\n", ntStr::GetString(hsName), m_ActiveChatterBox->GetName()));
		return false;
	}

	if (this->MoveToSubChatterBox(pobSubCB, true))
	{
		DEBUG_PRINT_CHATTERBOX(("Participants are transfered to SUB-CHATTERBOX (%s). Activation Completed.\n", pobSubCB->GetName()));
	}
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::RemoveParticipant
*
*	DESCRIPTION		Removes a Participant from the Active ChatterBox
*
***************************************************************************************************/
bool CChatterBoxMan::RemoveParticipant(const CEntity* pobParticipant)
{
	if (!m_Enabled)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) RemoveParticipant: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
		return false;
	}

	// Checking for NULL pointers
	if ( !pobParticipant )
	{
		ntAssert_p( 0, ( "Request to remove a NULL pointer Participant from a ChatterBox participants' list.\n" ) );
		return false;
	}
	
	// Get the select ChatterBox from the list
	if ( ! m_ActiveChatterBox )
	{
		// There is no active ChatterBox.
		//ntAssert_p( 0, ( "CChatterBoxMan::RemoveParticipant: There is no active ChatterBox.\n" ) );
		return false;
	}

	// Stop participant audio and remove him from this chatterbox and associated subchatterboxes
	bool ret = m_ActiveChatterBox->RemoveParticipant(pobParticipant);
	m_ActiveChatterBox->StopParticipantAudio((CEntity*)pobParticipant);

	const ntstd::List<CChatterBox*>* pListGenericCBs = m_ActiveChatterBox->GetGenericChatterBoxes();
	ntstd::List<CChatterBox*>::const_iterator obIt		= pListGenericCBs->begin();
	ntstd::List<CChatterBox*>::const_iterator obItEnd	= pListGenericCBs->end();
	for ( ; obIt != obItEnd; ++obIt )
	{
		CChatterBox* pCB = *obIt;
		pCB->RemoveParticipant(pobParticipant);
	}

	// Is there a SubChatterBox?
	const CHashedString pobSubChatterBoxName = m_ActiveChatterBox->GetNameSubChatterBox();
	if (pobSubChatterBoxName.IsNull())
	{	
		// Nop...
		return (ret);
	}
	// Check the size of the participants' list if required move to the SubChatterBox
	CChatterBox* pobSubCB = this->GetChatterBox(ntStr::GetString(pobSubChatterBoxName));
	if ( ! pobSubCB )
	{
		// The ChatterBox Does not exists in the List of chatterboxes
		// Asserted in the GetChatterBox function
		return (ret);
	}
	if (this->MoveToSubChatterBox(pobSubCB, false))
	{
		DEBUG_PRINT_CHATTERBOX(("Moved to SUB-CHATTERBOX (%s)\n", pobSubCB->GetName()));
	}
	return (ret);

}

/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::ActivateChatterBox
*
*	DESCRIPTION		Returns a pointer to the Active ChatterBox
*
***************************************************************************************************/
bool CChatterBoxMan::ActivateChatterBox (const char* strNameCB, bool bResetStats)
{
	//if (!m_Enabled)
	//{
	//	DEBUG_PRINT_CHATTERBOX(("(Warning!) ActivateChatterBox: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
	//	return false;
	//}

	if (!strNameCB)
	{
		DEBUG_PRINT_CHATTERBOX(("(ERROR!) ActivateChatterBox: NULL Chatterbox name passed!!!\n"));
		return false;
	}

	CChatterBox* pobNewActiveCB = this->GetChatterBox(strNameCB); // Hold the new CB
 
	if (!pobNewActiveCB) 
	{
		DEBUG_PRINT_CHATTERBOX(("ActivateChatterBox:: Chatterbox [%s] could not be found. Check XML data.\n",strNameCB));
		// Don't change Active ChatterBox
		return false;
	}

	DEBUG_PRINT_CHATTERBOX(("Activating Chatterbox: %s\n",strNameCB));
	if (m_ActiveChatterBox) 
	{
		m_ActiveChatterBox->Deactivate();		// Deactivate Current ChatterBox
	}
	m_ActiveChatterBox = pobNewActiveCB;	// Point to the new Active ChatterBox
	return(m_ActiveChatterBox->Activate(bResetStats)); // Update and return Activation flag
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CChatterBoxMan::Trigger
*
*	DESCRIPTION		Returns a pointer to the Active ChatterBox
*
***************************************************************************************************/

bool CChatterBoxMan::Trigger (const char* strEvent, CEntity* pobEntSource)
{
	if (!m_Enabled)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) Trigger: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
		return false;
	}

	// Checking for NULL pointers

	if (!this->m_ActiveChatterBox)
		return false;
	
	if ( !pobEntSource )
	{
		DEBUG_PRINT_CHATTERBOX(("(ERROR!) Request to Check a NULL pointer Participant from the ChatterBox (%s). Check LUA scripts or XML file.\n", this->m_ActiveChatterBox->GetName()));
		return false;
	}

	// Cheking if the Event's Source is a Participant from the Active ChatterBox or the Player
	if (!(this->m_ActiveChatterBox->IsParticipantInList(pobEntSource)) && 
		!pobEntSource->IsPlayer())
	{
		DEBUG_PRINT_CHATTERBOX(("ChatterBoxMan.Trigger: The source (%s) of the event (%s) is not a participant.Ignoring event.\n",ntStr::GetString(pobEntSource->GetName()),strEvent));
		return false;
	}

	// Check if event is supported
	m_PlayingFromChatterBox = NULL;
	SChatTrigger* pTrigger = m_ActiveChatterBox->GetChatTrigger(strEvent, &m_PlayingFromChatterBox);
	if (! pTrigger || !m_PlayingFromChatterBox)
	{
		return false;
	}
	
	// Update the Trigger Statistics
	UpdateStatistics(CHashedString(strEvent));

	// ===============================================================================
	// Handling the (valid) trigger with the Active ChatterBox
	// ===============================================================================
	
	SChatTrigger* pActiveChatTrigger = m_LastPlayingChatterBox ? m_LastPlayingChatterBox->GetActiveChatTrigger() : m_ActiveChatterBox->GetActiveChatTrigger();
	
	bool bInterruptChat = (	!m_ActiveChatterBox->IsPlayingAnimEventSound() && 
							(!pActiveChatTrigger || 
							( pActiveChatTrigger && pActiveChatTrigger->m_uiPriority < pTrigger->m_uiPriority  ) ) ) ? true : false;

	// Is the active chatterbox available or interruptable ?
	if (	(!m_LastPlayingChatterBox) ||										// There was no previous chat
			( (!m_LastPlayingChatterBox->IsChatting() || bInterruptChat ) )		// Not Chatting or can be interrupted
		)					
	{	

		m_ActiveChatterBox->SetActiveChatTrigger(NULL);							// Reset Book keeping
		if ( m_LastPlayingChatterBox )
			m_LastPlayingChatterBox->SetActiveChatTrigger(NULL);
		m_PlayingFromChatterBox->SetActiveChatTrigger(NULL);

		// ===============================
		// Check trigger thresholds
		// ===============================

		if ( !pTrigger->m_bTimeThresholdSurpassed )
		{
			pTrigger->m_uiCurrentCount = 0;
			DEBUG_PRINT_CHATTERBOX(("Trigger (%s) was not executed in ChatterBox (%s) due to TimeThreshold(%.3f/%.3f)\n",strEvent,	m_ActiveChatterBox->GetName(),pTrigger->m_fCurrentTime,pTrigger->m_fTimeThreshold));
			return false;
		}

		pTrigger->m_uiCurrentCount++;

		if ( pTrigger->m_uiCountThreshold > pTrigger->m_uiCurrentCount )
		{
			DEBUG_PRINT_CHATTERBOX(("Trigger (%s) was not executed in ChatterBox (%s) due to CountThreshold(%d/%d)\n",strEvent,	m_ActiveChatterBox->GetName(),pTrigger->m_uiCurrentCount,pTrigger->m_uiCountThreshold));
			return false;
		}
		else
		{
			pTrigger->m_uiCurrentCount = 0;
		}

		// ===============================
		// Start a new Conversation
		// ===============================
		
		bool bOk = false;
		if (m_PlayingFromChatterBox->DoIUseBankedPhrases())
		{
			// Banked phrases conversation
			bOk =  m_PlayingFromChatterBox->StartBankedChatting(strEvent,pobEntSource);
		}
		else
		{
			// Normal tree-like conversation
			bOk =  m_PlayingFromChatterBox->StartChatting(strEvent,pobEntSource);
		}
		if (bOk)
		{
			SetCurrentChatGUID(m_PlayingFromChatterBox->GetChatGUID());
			m_LastPlayingChatterBox = m_PlayingFromChatterBox;
			m_PlayingFromChatterBox->SetActiveChatTrigger(pTrigger);// Book keeping
			pTrigger->m_fCurrentTime = 0.0f;						// Reset trigger's timer
		}
		else
		{
			DEBUG_PRINT_CHATTERBOX(("The sound system didn't play the selected ChatItem. Check the previous lines for detailed information.\n"));
		}
		return bOk;
	}

	// Well, the ChatterBox was already in Chatting mode. Ignore the event
	DEBUG_PRINT_CHATTERBOX(("ChatterBox: %s is already engaged in a chatting.\n",m_ActiveChatterBox->GetName()));
	return (false);
}

//--------------------------------------------------
//! UpdateStatistics
//--------------------------------------------------
void CChatterBoxMan::UpdateStatistics(CHashedString hsCurrentTriggerEvent)
{
	if (!m_ActiveChatterBox)
		return;

	CBStatsList* pStatList = m_ActiveChatterBox->GetStatisticsList();
	if (!pStatList)
		return;

	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obIt		= pStatList->begin();
	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obItEnd	= pStatList->end();
	for ( ; obIt != obItEnd; ++obIt )
	{
		SChatterBoxStatisticPair* pStat = *obIt;
		if (!pStat)
		{
			DEBUG_PRINT_CHATTERBOX(("(Error!) Statistic Update: ChatterBox: [%s] loaded an invalid XML Statistics definition.\n",m_ActiveChatterBox->GetName() ));
		}

		if ( pStat->hsTriggerEvent == hsCurrentTriggerEvent )
		{
			pStat->uiCount++;
			DEBUG_PRINT_CHATTERBOX(("Statistic Update: ChatterBox: [%s] - Trigger [%s] - Count: [%d]\n",m_ActiveChatterBox->GetName(),ntStr::GetString(hsCurrentTriggerEvent), pStat->uiCount ));
			return;
		}
	}
}

//--------------------------------------------------
//! GetCurrentStatistic
//--------------------------------------------------
int  CChatterBoxMan::GetCurrentStatistic( CHashedString hsTriggerEvent, bool* bFound ) const
{
	*bFound = false;

	if (!m_ActiveChatterBox)
		return -1;

	int ret = m_ActiveChatterBox->GetStatistic(hsTriggerEvent,bFound);

	return ret;
}


//--------------------------------------------------
//! CopyCurrentStatistic
//--------------------------------------------------
//void  CChatterBoxMan::CopyCurrentStatistic( CChatterBox* pFromCB, CChatterBox* pToCB )
//{
//	if (!pFromCB || !pToCB)
//		return;
//
//	const CBStatsList* pStatList = pFromCB->GetStatisticsList_const();
//	if (!pStatList)
//		return;
//
//	pToCB->ResetStatistics();
//
//	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obIt		= pFromCB->begin();
//	ntstd::List<SChatterBoxStatisticPair*>::const_iterator obItEnd	= pFromCB->end();
//	for ( ; obIt != obItEnd; ++obIt )
//	{
//		SChatterBoxStatisticPair* pStat = *obIt;
//		pToCB->SetStatistic( pStat->hsTriggerEvent, pStat->uiCount);
//	}
//	return;
//}

//--------------------------------------------------
//! StopCurrentChats
//--------------------------------------------------
void  CChatterBoxMan::StopCurrentChats( void )
{ 
	if (m_ActiveChatterBox)			m_ActiveChatterBox->StopChats();
	if (m_PlayingFromChatterBox)	m_PlayingFromChatterBox->StopChats();
	if ( m_LastPlayingChatterBox )	m_LastPlayingChatterBox->StopChats();
}

//--------------------------------------------------
//! StopParticipantAudio
//--------------------------------------------------
void  CChatterBoxMan::StopParticipantAudio( CEntity* pEnt )
{ 
	if (!pEnt)
		return;

	if (m_ActiveChatterBox)			m_ActiveChatterBox->StopParticipantAudio( pEnt );
	if (m_PlayingFromChatterBox)	m_PlayingFromChatterBox->StopParticipantAudio( pEnt );
	if ( m_LastPlayingChatterBox )	m_LastPlayingChatterBox->StopParticipantAudio( pEnt );
}


//--------------------------------------------------
//! Update
//--------------------------------------------------
void  CChatterBoxMan::Update(float fTimeChange)
{ 
	if (!m_Enabled)
	{
		//DEBUG_PRINT_CHATTERBOX(("(Update!) Trigger: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
		return;
	}

	// Debug Render
	if (m_bRender)
		DebugRender();

	// Update the active chatterbox
	ntstd::List<CChatterBox*>::const_iterator obIt		= m_ChatterBoxes.begin();
	ntstd::List<CChatterBox*>::const_iterator obItEnd	= m_ChatterBoxes.end();
	for ( ; obIt != obItEnd; ++obIt )
	{
		CChatterBox* pCB = (*obIt);
		if (pCB->IsActive())
			pCB->Update(fTimeChange);
	}
}

//--------------------------------------------------
//! TriggerAnimEventChat
//--------------------------------------------------
bool CChatterBoxMan::TriggerAnimEventSound( CEntity* pSpeaker, CAnimEventSound* pAnimEventSound)
{
	if (!m_ActiveChatterBox)
	{
		DEBUG_PRINT_CHATTERBOX(("(Error!) TriggerAnimEventSound cannot play since there isn't an active ChatterBox!" ));
		return false;
	}

	DEBUG_PRINT_CHATTERBOX(("ChatterBox: %s received a AnimEventSound\n",m_ActiveChatterBox->GetName()));

	if (!m_Enabled)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) TriggerAnimEventSound: ChatterBoxMan and/or Audio Engine is Disabled!!!\n"));
		return false;
	}

	if (!pSpeaker || !pAnimEventSound)
		return false;
	
	// Am I enabled and there is a valid active chatterbox?
	if (!m_Enabled || !this->m_ActiveChatterBox) 
		return false;
	
	bool bOK = m_ActiveChatterBox->TriggerAnimEventSound(pSpeaker, pAnimEventSound);
	return bOK;
}

//--------------------------------------------------
//! DisableChecksOnAI
//--------------------------------------------------
void CChatterBoxMan::DisableChecksOnAI(CEntity* pEnt, bool bDisabled)
{
	if (!pEnt)
		return;

	if (!pEnt->IsAI())
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) DisableChecksOnAI: Entity [%s] is not an AI!!!\n",ntStr::GetString(pEnt->GetName())));
		return;
	}

	if (!this->m_ActiveChatterBox)
	{
		DEBUG_PRINT_CHATTERBOX(("(Warning!) DisableChecksOnAI(...) didn't execute becouse there isn't an Active ChatterBox - AI [%s] !!!\n",ntStr::GetString(pEnt->GetName())  ));
		return;
	}
	
	m_ActiveChatterBox->DisableChecksOnAI(pEnt,bDisabled);
}

//--------------------------------------------------
//! HasChatterBoxChecksDisabled
//--------------------------------------------------
bool CChatterBoxMan::HasChatterBoxChecksDisabled(CEntity* pEnt)
{
	if (!pEnt || !pEnt->IsAI() || !this->m_ActiveChatterBox)
		return false;

	return (m_ActiveChatterBox->HasChatterBoxChecksDisabled(pEnt));
}

//--------------------------------------------------
//! DebugIsEntityInCSStandadrd
//--------------------------------------------------
bool CChatterBoxMan::DebugIsEntityInCSStandadrd	( CEntity * pEnt )
{
	return ( pEnt && pEnt->GetAttackComponent() && pEnt->GetAttackComponent()->AI_Access_IsInCSStandard() );
}

//--------------------------------------------------
//! DebugIsEntityInCSStandadrd
//--------------------------------------------------
void CChatterBoxMan::SetChatGroupID	( CEntity* pEnt, int iNewGroupID )
{ 
	if (!pEnt)
	{
		DEBUG_PRINT_CHATTERBOX(("(Error!) SetChatGroupID - NULL Entity\n"));
		return;
	}
	else if ( !pEnt->IsAI() ) 
	{
		DEBUG_PRINT_CHATTERBOX(("(Error!) SetChatGroupID - Entity [%s] is not an AI !!!\n",ntStr::GetString(pEnt->GetName())  ));
		return;
	}
	else
	{ 
		DEBUG_PRINT_CHATTERBOX(("Entity [%s] changed its ChatGroup from [%d] to [%d]\n",ntStr::GetString(pEnt->GetName()),pEnt->ToAI()->GetChatGroupID(), iNewGroupID  ));
		pEnt->ToAI()->SetChatGroupID(iNewGroupID); 
	} 
}



