//------------------------------------------------------------------------------------------
//!
//!	aipatrolpath.cpp
//!
//------------------------------------------------------------------------------------------

#include "aipatrolnode.h"

#include "aipatrolpath.h"


//------------------------------------------------------------------------------------------
//!
//!	AIPatrolPath::~AIPatrolPath
//!
//------------------------------------------------------------------------------------------

AIPatrolPath::~AIPatrolPath()
{
}

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolPath::AddNode
//!
//------------------------------------------------------------------------------------------

void
AIPatrolPath::AddNode( PatrolNode* pobNode )
{
	m_obNodeList.push_back( pobNode );
	pobNode->SetNum( m_iNumNodes++ );
}

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolPath::DebugRender
//!
//------------------------------------------------------------------------------------------

void AIPatrolPath::DebugRender()
{
	PatrolNodeList::iterator itr = m_obNodeList.begin();
	for(;itr!=m_obNodeList.end();++itr)
	{
		(*itr)->PaulsDebugRender();
	}
}

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolPath::GetNode
//!
//------------------------------------------------------------------------------------------

PatrolNode* AIPatrolPath::GetNode( int iNodeNum )
{
	PatrolNodeList::iterator itr = m_obNodeList.begin();
	for(int i = 0;itr!=m_obNodeList.end();++itr,++i)
	{
		if (i == iNodeNum)
		{
			return *itr;
		}
	}

	return NULL;
}
