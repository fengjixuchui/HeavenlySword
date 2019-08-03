//! ------------------------------------------------------
//! aiuseobjectqueueman.cpp
//!
//! Queuing Manager for safe usage of objects (ladders...)
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!-------------------------------------------------------

#include "aiuseobjectqueueman.h"
#include "ai/ainavigationsystem/aimovement.h"
#include "game/entityai.h"
#include "game/aicomponent.h"

//------------------------------------------------------------------------------------------
//	Destructors
//------------------------------------------------------------------------------------------
CAIQueueManager::~CAIQueueManager()
{
	QueueFoldersList::const_iterator obIt		= m_QueueFoldersList.begin();
	QueueFoldersList::const_iterator obEndIt	= m_QueueFoldersList.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		CAIQueueOjectFolder* pQF = *obIt;
		pQF->FreeDynamicData();	
		NT_DELETE_CHUNK( Mem::MC_AI, pQF );
	}

	m_QueueFoldersList.clear();
}

CAIQueueOjectFolder::~CAIQueueOjectFolder()
{
	FreeDynamicData();
}

void CAIQueueOjectFolder::FreeDynamicData(void)
{
	SingleQueueList::const_iterator obIt	= m_SingleQueueList.begin();
	SingleQueueList::const_iterator obEndIt	= m_SingleQueueList.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		CAISingleQueue* pSQ = *obIt;
		NT_DELETE_CHUNK( Mem::MC_AI, pSQ );
	}

	m_SingleQueueList.clear();
}

//------------------------------------------------------------------------------------------
//	UpdateQueueIndexes
//------------------------------------------------------------------------------------------
void CAISingleQueue::UpdateQueueIndexes( void )
{
	int i = -1;

	AIList::const_iterator obIt		= m_AIList.begin();
	AIList::const_iterator obEndIt	= m_AIList.end();
	for ( ; obIt != obEndIt; ++obIt )
	{
		AI* pAI = *obIt;
		
		CAIMovement* pMov = pAI->GetAIComponent()->GetCAIMovement();
		pMov->SetQueueIndex(i);
		pMov->SetQueueIndexUpdated(true);
		i ++;
	}
}

//------------------------------------------------------------------------------------------
//	RemoveAI
//------------------------------------------------------------------------------------------
bool CAISingleQueue::RemoveAI( AI* pAI )
{
	if (!pAI) return false;

	AIList::iterator obIt		= m_AIList.begin();
	AIList::iterator obEndIt	= m_AIList.end();
	AIList::iterator obRes		= find(obIt,obEndIt,pAI);
	
	if (obRes != obEndIt)
	{
		m_AIList.erase(obRes);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------------------
//	GetAIAtHead
//------------------------------------------------------------------------------------------
AI*	CAISingleQueue::GetAIAtHead	( void )
{ 
	if (!m_AIList.empty()) 
		return (*(m_AIList.begin()));
	else 
		return NULL; 
}
//------------------------------------------------------------------------------------------
//	IsQNodeRegistered
//------------------------------------------------------------------------------------------
bool CAIQueueOjectFolder::IsQNodeRegistered( CAINavigNode* pQNode )
{
	if (!pQNode) return false;

	AINavigNodeList::const_iterator obIt	= m_QNodeList.begin();
	AINavigNodeList::const_iterator obEndIt	= m_QNodeList.end();
	AINavigNodeList::const_iterator obRes	= find(obIt,obEndIt,pQNode);
	
	if (obRes != obEndIt)
		return true;
	else
		return false;
}

//------------------------------------------------------------------------------------------
//	IsUserRegistered
//------------------------------------------------------------------------------------------
bool CAIQueueOjectFolder::IsUserRegistered( AI* pAI )
{
	if (!pAI) return false;

	AIList::const_iterator obIt		= m_AIList.begin();
	AIList::const_iterator obEndIt	= m_AIList.end();
	AIList::const_iterator obRes	= find(obIt,obEndIt,pAI);
	
	if (obRes != obEndIt)
		return true;
	else
		return false;
}


//------------------------------------------------------------------------------------------
//	UpdateQueueIndexes
//------------------------------------------------------------------------------------------
void CAIQueueOjectFolder::UpdateQueueIndexes( void )
{
	CAISingleQueue* pSQ = GetMostBusyQueue();
	if (!pSQ)
	{
		// All queues are empty
		return;
	}

	pSQ->UpdateQueueIndexes();
}

////------------------------------------------------------------------------------------------
////	GetQueuePointAndRadius
////------------------------------------------------------------------------------------------
//CPoint CAIQueueOjectFolder::GetQueuePointAndRadius( AI* pAI, float* fRadius, bool* bSuccess )
//{
//	*bSuccess = false;
//	unsigned int i = 0;
//	CAINavigNode* pQN = NULL;
//
//	CAIMovement* pMov = pAI->GetAIComponent()->GetCAIMovement();
//	CAISingleQueue* pSQ = pMov->GetSingleQueue();
//
//	if (!pSQ) return CPoint(CONSTRUCT_CLEAR);
//
//	AIList::const_iterator obIt		= m_AIList.begin();
//	AIList::const_iterator obEndIt	= m_AIList.end();
//	
//	for ( ; obIt != obEndIt; ++obIt )
//	{
//		AI* pAIFromList = *obIt;
//
//		if (pAIFromList == pAI )
//		{
//			// Find the QNode with the same position (index) in the list
//			unsigned int j = 0;
//			AINavigNodeList::const_iterator obQNIt		= m_QNodeList.begin();
//			AINavigNodeList::const_iterator obQNEndIt	= m_QNodeList.end();
//			
//			for ( ; obQNIt != obQNEndIt; ++obQNIt )
//			{
//				if ( j == i )
//				{
//					pQN = *obQNIt;
//					break;
//				}
//				j++;
//			}
//			break;
//		}
//		i++;
//	}
//
//	if (pQN)
//	{
//		*bSuccess = true;
//		*fRadius = pQN->GetRadiusSQR();
//		return (pQN->GetPos());
//	}
//	else
//	{
//		*bSuccess = false;
//		*fRadius = -1.0f;
//		return CPoint(CONSTRUCT_CLEAR);
//	}
//}


//------------------------------------------------------------------------------------------
//	GetMostEmptyQueue
//------------------------------------------------------------------------------------------
CAISingleQueue* CAIQueueOjectFolder::GetMostEmptyQueue ( void )
{
	unsigned int uiQueueMinSz = 9999; // Something biiiiig
	unsigned int uiQueueSz = 0;
	CAISingleQueue* pMostEmptyQueue = NULL;

	SingleQueueList::const_iterator obIt	= m_SingleQueueList.begin();
	SingleQueueList::const_iterator obEndIt	= m_SingleQueueList.end();
	for ( ; obIt != obEndIt; ++obIt )
	{
		CAISingleQueue* pSQ = *obIt;
		uiQueueSz = pSQ->GetCurrentQueueSize();
		if (uiQueueSz < uiQueueMinSz && uiQueueSz < pSQ->GetMaxSize() )
		{
			uiQueueMinSz = uiQueueSz;
			pMostEmptyQueue = pSQ;
		}
	}

	return pMostEmptyQueue;	
}

//------------------------------------------------------------------------------------------
//	GetMostBusyQueue
//------------------------------------------------------------------------------------------
CAISingleQueue* CAIQueueOjectFolder::GetMostBusyQueue ( void )
{
	unsigned int uiQueueMaxSz = 0;
	unsigned int uiQueueSz = 0;
	CAISingleQueue* pMostBusyQueue = NULL;

	SingleQueueList::const_iterator obIt	= m_SingleQueueList.begin();
	SingleQueueList::const_iterator obEndIt	= m_SingleQueueList.end();
	for ( ; obIt != obEndIt; ++obIt )
	{
		CAISingleQueue* pSQ = *obIt;
		uiQueueSz = pSQ->GetCurrentQueueSize();
		if (uiQueueSz > uiQueueMaxSz)
		{
			uiQueueMaxSz = uiQueueSz;
			pMostBusyQueue = pSQ;
		}
	}

	return pMostBusyQueue;	
}

//================================================================================================================
//===========================================   CAIQueueManager      =============================================
//================================================================================================================

//------------------------------------------------------------------------------------------
//	CreatFolder
//------------------------------------------------------------------------------------------
CAIQueueOjectFolder* CAIQueueManager::CreateFolder( CEntity* pObjectToUse )
{
	if (!pObjectToUse) return NULL;

	CAIQueueOjectFolder* pQF = NT_NEW_CHUNK(Mem::MC_AI) CAIQueueOjectFolder(pObjectToUse);

	ntAssert(pQF);

	return (pQF);
}

//------------------------------------------------------------------------------------------
//	GetAIQueueingInNode
//------------------------------------------------------------------------------------------
AI* CAISingleQueue::GetAIQueueingInNode( CAINavigNode* pNode )
{
	if (!pNode) return NULL;

	int iIndex = -1;

	// Find the index of the Qnode
	for ( unsigned int i = 0 ; i < m_QNodeVector.size(); i++ )
	{
		if (m_QNodeVector[i] == pNode)
		{
			iIndex = i;
			break;
		}
	}

	if (iIndex<0) 
		return NULL;

	// Find the AI with the same queuing index
	AIList::const_iterator obIt		= m_AIList.begin();
	AIList::const_iterator obEndIt	= m_AIList.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		AI* pAI = *obIt;
		if (pAI)
		{
			CAIMovement* pMov = pAI->GetAIComponent()->GetCAIMovement();
			if (pMov->GetQueueIndex() == iIndex)
			{
				return pAI;
			}
		}
    }
	return NULL;
}

//------------------------------------------------------------------------------------------
//	GetAIQueueingInNode
//------------------------------------------------------------------------------------------
AI* CAIQueueOjectFolder::GetAIQueueingInNode( CAINavigNode* pNode )
{
	if (!pNode) return NULL;

	SingleQueueList::const_iterator obIt	= m_SingleQueueList.begin();
	SingleQueueList::const_iterator obEndIt	= m_SingleQueueList.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		CAISingleQueue* pSQ = *obIt;
		AI* pAI = pSQ->GetAIQueueingInNode(pNode);
		if (pAI) 
			return pAI;
    }
	return NULL;
}

//------------------------------------------------------------------------------------------
//	GetFolder
//------------------------------------------------------------------------------------------
CAIQueueOjectFolder* CAIQueueManager::GetFolder( CEntity* pObjectToUse )
{
	if (!pObjectToUse) return NULL;

	QueueFoldersList::const_iterator obIt		= m_QueueFoldersList.begin();
	QueueFoldersList::const_iterator obEndIt	= m_QueueFoldersList.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		CAIQueueOjectFolder* pQF = *obIt;
		
		if (pQF->GetObject() == pObjectToUse)
			return pQF;
    }

	return NULL;
}

////------------------------------------------------------------------------------------------
////	GetFolder
////------------------------------------------------------------------------------------------
//CAIQueueOjectFolder* CAIQueueManager::GetFolder( AI* pAI )
//{
//	if (!pAI) return NULL;
//
//	QueueFoldersList::const_iterator obIt		= m_QueueFoldersList.begin();
//	QueueFoldersList::const_iterator obEndIt	= m_QueueFoldersList.end();
//	
//	for ( ; obIt != obEndIt; ++obIt )
//	{
//		CAIQueueOjectFolder* pQF = *obIt;
//		
//		if (pQF->IsUserRegistered(pAI))
//			return pQF;
//    }
//
//	return NULL;
//}

//------------------------------------------------------------------------------------------
//	RegisterQueueGraph
//------------------------------------------------------------------------------------------
void CAIQueueManager::RegisterQueueGraph( CEntity* pObjectToUse, AINavigNodeList* pNNL )
{
	if (!pNNL || !pObjectToUse) return;
	CAIQueueOjectFolder* pQF = GetFolder(pObjectToUse);
	if ( pQF )
	{
		// This should not happen
		ntPrintf("RegisterQueueGraph:: folder already exists of object: %s!!!", ntStr::GetString(pObjectToUse->GetName()));
		return;
	}
	
	// Create a new folder in the queue manager for this object
	pQF = CreateFolder(pObjectToUse);
	if ( !pQF ){
		// This should not happen
		ntPrintf("RegisterQueueGraph:: folder could not be created for object: %s!!!", ntStr::GetString(pObjectToUse->GetName()));
		return;
	}

	// Generate the list of single queues
    AINavigNodeList::const_iterator obIt	= pNNL->begin();
	AINavigNodeList::const_iterator obEndIt = pNNL->end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigNode* pNode = (*obIt);
		const AINavigArrowList* pSA = pNode->GetSourceArrows();
		if (pSA->empty())
		{
			// This is a tail node, generate a single queue
			AINavigNodeList obNodeList;

			const AINavigArrowList* pTAL = pNode->GetTgtArrows();
			while (!pTAL->empty())
			{	
				obNodeList.push_front(pNode);
				CAINavigArrow* pTA = *(pTAL->begin());
				pNode = pTA->GetTgtNode();
				pTAL = pNode->GetTgtArrows();				
			}
			// Head node found
			obNodeList.push_front(pNode);

			// Populate the SingleQueue vector
			CAISingleQueue* pSingleQueue = NT_NEW_CHUNK( Mem::MC_AI ) CAISingleQueue(pQF, pObjectToUse, obNodeList.size() );
			AINavigNodeList::const_iterator obNLIt		= obNodeList.begin();
			AINavigNodeList::const_iterator obNLEndIt	= obNodeList.end();
            
			for( ; obNLIt != obNLEndIt; ++obNLIt )
			{
				CAINavigNode* pN = *obNLIt;
				pSingleQueue->push_back(pN);
			}

			// Creatate a name for this queue										
			pSingleQueue->SetName(pSingleQueue->GetNodeAtHead()->GetName());
			ntPrintf("Added SingleQueue (%s) size: [%d]\n"	,ntStr::GetString(pSingleQueue->GetName())
															,pSingleQueue->GetMaxSize());
			// Add the Single queue to its folder
			pQF->AddSingleQueue(pSingleQueue);
		}
	}

	m_QueueFoldersList.push_back(pQF);
}

////------------------------------------------------------------------------------------------
////	RegisterQueueNode
////------------------------------------------------------------------------------------------
//void CAIQueueManager::RegisterQueueNode( CEntity* pObjectToUse, CAINavigNode* pQNode )
//{
//	if (!pObjectToUse || !pQNode) return;
//
//	CAIQueueOjectFolder* pQF = GetFolder(pObjectToUse);
//
//	if ( pQF )
//	{
//		// Folder exists (i.e. object is already registered)
//		pQF->AddQNode(pQNode);
//	}
//	else
//	{
//		// Create a new folder in the queue manager for this object
//		CAIQueueOjectFolder* pQF = CreateFolder(pObjectToUse);
//		if (pQF)
//		{
//			// Register the QNode
//			pQF->AddQNode(pQNode);
//		}	
//	}
//}

//------------------------------------------------------------------------------------------
//	RegisterUser
//------------------------------------------------------------------------------------------
bool CAIQueueManager::RegisterUser( AI* pAI, CEntity* pObjectToUse )
{
	CAIQueueOjectFolder* pQF = GetFolder(pObjectToUse);

	if (!pObjectToUse || !pAI || !pQF) return false;
	
	CAISingleQueue* pSQ = pQF->GetMostEmptyQueue();
	if (!pSQ)
	{
		ntPrintf("CAIQueueManager::RegisterUser -> Queue is FULL!!!!\n");
		return false;
	}
	CAISingleQueue* pSQBusy = pQF->GetMostBusyQueue();

	CAIMovement* pMov = pAI->GetAIComponent()->GetCAIMovement();

	if (pSQBusy)
	{
		// Is the only AI in queue using the object?
		AI* pAIAtHead = pSQ->GetAIAtHead();
		if (!pAIAtHead)
		{
			// Assigne the the first index (0) to the new AI
			pMov->SetQueueIndex(0);
		}
		else
		{
			int iIndex		= pAIAtHead->GetAIComponent()->GetCAIMovement()->GetQueueIndex();
			int iQueueSize	= pSQ->GetCurrentQueueSize();
			int iQueuePos	= (iIndex >= 0)			? iQueueSize : 
							  ((iQueueSize == 1)	? 0 : iQueueSize - 1);
			pMov->SetQueueIndex(iQueuePos);
		}
	}
	pMov->SetObjectToQueue(pObjectToUse);
	pMov->SetSingleQueue(pSQ);
	pSQ->AddUser(pAI);
	ntPrintf("CAIQueueManager::RegisterUser -> Added [%s] (idx: %d), Queue [%s], Object [%s]\n",	
																	ntStr::GetString(pAI->GetName()),
																	pMov->GetQueueIndex(),
																	ntStr::GetString(pSQ->GetName()),
																	ntStr::GetString(pQF->GetObject()->GetName())
																	);
																			
	return true;
	//// Object is not registered...
	//ntAssert_p(0,("CAIQueueManager::RegisterUser -> error registering user (object is not registered)"));
}

//------------------------------------------------------------------------------------------
//	ReportObjectUsed
//------------------------------------------------------------------------------------------
void CAIQueueManager::ReportObjectUsed( AI* pAI )
{
	CAIMovement* pMov	= pAI->GetAIComponent()->GetCAIMovement();
	CAISingleQueue* pSQ = pMov->GetSingleQueue();

	if (!pSQ)
		return;
	pMov->SetQueueIndex(-1);
	pMov->SetObjectToQueue(NULL);
	pMov->SetObjectToQueue(NULL);
	pMov->SetQueueIndexUpdated(true);
	pMov->SetSingleQueue(NULL);

	pSQ->RemoveAI(pAI);

	CAIQueueOjectFolder* pQF = this->GetFolder(pSQ->GetObject());
	CAISingleQueue* pSQBusy = pQF->GetMostBusyQueue();

	if (pSQBusy)
		pSQBusy->UpdateQueueIndexes();
}

//------------------------------------------------------------------------------------------
//	GetQueuePointAndRadius
//------------------------------------------------------------------------------------------
CPoint CAIQueueManager::GetQueuePointAndRadius( AI* pAI, float* fRadius, bool* bSuccess )
{
	*bSuccess = false;

	// Get the SingleQueue where the AI is
	CAIMovement* pMov = pAI->GetAIComponent()->GetCAIMovement();
	CAISingleQueue* pSQ = pMov->GetSingleQueue();
	if (!pSQ) return CPoint(CONSTRUCT_CLEAR);

	// Find its position
	int iIndex = pMov->GetQueueIndex();

	// return radius and position

	return (pSQ->GetQueuePointAndRadius(iIndex,fRadius,bSuccess));	
}


//------------------------------------------------------------------------------------------
//	Update
//------------------------------------------------------------------------------------------
void CAIQueueManager::Update( float fTimeChange )
{
	QueueFoldersList::const_iterator obIt		= m_QueueFoldersList.begin();
	QueueFoldersList::const_iterator obEndIt	= m_QueueFoldersList.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		CAIQueueOjectFolder* pQF = *obIt;
		
		pQF->UpdateTimer(fTimeChange);
	}
}

//------------------------------------------------------------------------------------------
//	GetQueuePointAndRadius
//------------------------------------------------------------------------------------------
CPoint CAISingleQueue::GetQueuePointAndRadius ( int iIndex, float* fRadius, bool* bSuccess )
{
	CPoint obPos(CONSTRUCT_CLEAR);
	*bSuccess = false;

	if (iIndex>=0)
	{
		CAINavigNode* pNN = m_QNodeVector[iIndex];
		if (pNN)
		{
			obPos		= pNN->GetPos();
			*fRadius	= pNN->GetRadius();
			*bSuccess	= true;
		}
	}
	return obPos;
}





