//! --------------------------------------------------------
//! AINavigGraphMan.cpp
//!
//! New AIs Navigation Grpah Manager
//!
//! It handles AIs navigation system for the whole level 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ainaviggraphman.h"
#include "game/luaglobal.h" // !!! - Temporary
#include "core/visualdebugger.h"

//------------------------------------------------------------------------------------------
// CChatterBoxMan Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CAINavigGraphMan)
	LUA_EXPOSED_METHOD(MakePath, MakePath, "Make A* Path", "{float, float, float, float, float, float}", "X1|Y1|Z1|X2|Y2|Z2")
	//LUA_EXPOSED_METHOD(PrintPath, PrintPath, "PrintPath A* Path", "", "")
LUA_EXPOSED_END(CAINavigGraphMan)

//! ------------------------------------------------------------------------------------------
//! CAINavigGraphMan::CAINavigGraphMan()
//!
//! Constructor
//! ------------------------------------------------------------------------------------------

CAINavigGraphMan::CAINavigGraphMan() :
		m_bRender(false),
		m_iNumberOfRegionsInLevel(0)
{
	ATTACH_LUA_INTERFACE(CAINavigGraphMan);

	// Lua Registering
	CLuaGlobal::Get().State().GetGlobals().Set("CAINavigGraphMan", this);
}

CAINavigGraphMan::~CAINavigGraphMan()
{
	this->LevelUnload();
}

void CAINavigGraphMan::LevelUnload ( void )
{
	;
}

//! ------------------------------------------------------------------------------------------------
//! void CAINavigGraphMan::Add( CAINavigGraph* pNG)
//!
//! 
//! ------------------------------------------------------------------------------------------------

void CAINavigGraphMan::Add ( CAINavigGraph* pNG)
{
	if (pNG) 
	{ 
		m_iNumberOfRegionsInLevel++; 
		m_listNavigGraphs.push_back(pNG);
	}
	else
	{
		ntAssert_p(0, ("CAINavigGraphMan::Add -> NULL Nav Region passed\n"));
	}
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphMan::GetNode( const char* psNodeName, const char* psAreaName ) 
//!
//! Returns the node in the area psAreaName that matches the name: psNodeName, or NULL if not found.
//! ------------------------------------------------------------------------------------------------
CAINavigGraph* CAINavigGraphMan::GetNavigGraphArea( const char* psAreaName )
{
	if (!psAreaName) 
	{
		ntPrintf("CAINavigGraphMan::GetNavigGraphArea -> NULL parameter received (psAreaName)\n");
		return NULL;
	}
	// Go through the list of areas and get the one with the psAreaName
	CHashedString ksAreaName(psAreaName);

	AINavigGraphList::const_iterator obItArea = this->m_listNavigGraphs.begin();
	AINavigGraphList::const_iterator obEndItArea = this->m_listNavigGraphs.end();
	for( ; obItArea != obEndItArea; ++obItArea )
	{
		if ( (*obItArea)->GetKName() == &ksAreaName )
		{
			return (*obItArea);
		}
	}
	// Area Not found
	ntPrintf("CAINavigGraphMan::GetNavigGraphArea -> Area ($s) not found. Number of areas: [%d]\n",psAreaName, this->m_listNavigGraphs.size());
	return NULL;
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphMan::GetNode( CEntity* pEnt ) 
//!
//! Returns the node that matches pEnt, or NULL if not found.
//! ------------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraphMan::GetNode( CEntity* pEnt, CAINavigGraph* pNG)
{
	AINavigGraphList::iterator obIt	= m_listNavigGraphs.begin();
	AINavigGraphList::iterator obItEnd = m_listNavigGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		CAINavigNode* pNode = (*obIt)->GetNode(pEnt);
		if (pNode) 
		{
			pNG = (*obIt);
			return pNode;
		}
	}
	// Not found
	return NULL;
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphMan::GetDoor( CEntity* pEnt ) 
//!
//! Returns the door that matches pEnt, or NULL if not found.
//! ------------------------------------------------------------------------------------------------
CAINavigArrow* CAINavigGraphMan::GetDoor( CEntity* pEnt )
{
	AINavigGraphList::iterator obIt	= m_listNavigGraphs.begin();
	AINavigGraphList::iterator obItEnd = m_listNavigGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		CAINavigArrow* pArrow = (*obIt)->GetDoor(pEnt);
		if (pArrow) return pArrow;
	}
	// Not found
	return NULL;
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphMan::GetLadder( CEntity* pEnt ) 
//!
//! Returns the ladder that matches pEnt, or NULL if not found.
//! ------------------------------------------------------------------------------------------------
CAINavigArrow* CAINavigGraphMan::GetLadder( CEntity* pEnt )
{
	AINavigGraphList::iterator obIt	= m_listNavigGraphs.begin();
	AINavigGraphList::iterator obItEnd = m_listNavigGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		CAINavigArrow* pArrow = (*obIt)->GetLadder(pEnt);
		if (pArrow) return pArrow;
	}
	// Not found
	return NULL;
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphMan::IsDoorOpen( CEntity* pEnt ) 
//! ------------------------------------------------------------------------------------------------
bool CAINavigGraphMan::IsDoorOpen( CEntity* pEnt )
{
	return (this->GetDoor(pEnt)->IsEnabled());
}

//! ------------------------------------------------------------------------------------------------
//! CAINavigNode* CAINavigGraphMan::IsLadderAvailable( CEntity* pEnt ) 
//! ------------------------------------------------------------------------------------------------
bool CAINavigGraphMan::IsLadderAvailable( CEntity* pEnt )
{
	return ( (this->GetDoor(pEnt)->IsEnabled()) && !(this->GetDoor(pEnt)->IsBusy()) );
}
//! ------------------------------------------------------------------------------------------
//! void CAINavigNode::GetClosestNode( CPoint * pobPoint )
//!
//! Returns the closest node to the given coordinates.
//! ------------------------------------------------------------------------------------------
CAINavigNode* CAINavigGraphMan::GetClosestNode( CPoint * pobPoint, float* fDist  )  
{
	*fDist = FLT_MAX;
	CAINavigNode* pClosestNode = NULL;

	AINavigGraphList::iterator obIt	= m_listNavigGraphs.begin();
	AINavigGraphList::iterator obItEnd = m_listNavigGraphs.end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		float fDistanceMnh = FLT_MAX;
		CAINavigNode* pNode = (*obIt)->GetClosestNode(pobPoint, &fDistanceMnh);
		if (fDistanceMnh < *fDist ) 
		{
			*fDist = fDistanceMnh;
			pClosestNode = pNode;
		}
	}
	return (pClosestNode);
}

//! ------------------------------------------------------------------------------------------
//! bool CAINavigGraphMan::DeleteNode( CEntity* pEnt ) 
//!
//! Deletes a node (useful if a ladder is broken or a door is permanently locked
//! ------------------------------------------------------------------------------------------
bool CAINavigGraphMan::DeleteNode( CEntity* pEnt )
{
	CAINavigGraph*	pNG		= NULL;
	this->GetNode(pEnt, pNG);
	
	return (pNG->DeleteNode(pEnt));
}

//! ------------------------------------------------------------------------------------------
//! ntstd::List<CAINavigNode*>*	CAINavigGraphMan::GetTargetNodes ( CEntity* pEnt, CAINavigGraph* pNG) 
//!
//! 
//! ------------------------------------------------------------------------------------------
const ntstd::List<CAINavigArrow*>*	CAINavigGraphMan::GetTgtArrows ( CAINavigNode* pobNode ) const
{
	if(!pobNode) { return NULL; }
	return (pobNode->GetTgtArrows());
}


//------------------------------------------------------------------------------------------
//!
//!	CAINavigGraphMan::DebugRender
//!
//------------------------------------------------------------------------------------------

void CAINavigGraphMan::DebugRender()
{
	g_VisualDebug->Printf2D(10,10,DC_PURPLE,0,"Total Navigation Regions in Level: [%d] (Dario)", m_iNumberOfRegionsInLevel);
	if (!m_listNavigGraphs.empty())
	{
		(*(m_listNavigGraphs.begin()))->DebugRenderNG();
	}

	/*
	CPoint pt3D = m_obPos;
		pt3D.Y() += 0.5f;
	g_VisualDebug->Printf3D( pt3D, 0xffffffff, 0, "Dario" );
	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	float	fRadius = 0.1f;

	g_VisualDebug->RenderSphere( obOrientation, m_obPos,	fRadius, 0xff0ffff0 );

*/
}


void  CAINavigGraphMan::Update(float fTimeChange)
{ 
	UNUSED(fTimeChange); 
	if (m_bRender)
	{
		DebugRender();
	}
	return; 
}

void CAINavigGraphMan::MakePath( float x1, float y1, float z1, float x2, float y2, float z2 )
{
	CPoint obPs = CPoint(x1,y1,z1);
	CPoint obPe = CPoint(x2,y2,z2);
	CPoint* pobPs = &obPs;
	CPoint* pobPe = &obPe;

	AINavigNodeList* plist = m_obAStar.MakePath( pobPs, pobPe );

	ntPrintf("\n");

	if (plist->empty())
	{
		ntPrintf("No path FOUND!!!\n");
		return;
	}
	// Print list
	int idx = 0;
	AINavigNodeList::iterator obIt	= plist->begin();
	AINavigNodeList::iterator obItEnd = plist->end();
	for ( ; obIt != obItEnd; ++obIt)
	{
		ntPrintf("Step [%d] = %d\n",idx++,(*obIt)->GetIndex());
	}
}

