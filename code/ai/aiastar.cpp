#include "ai/aiastar.h"
#include "ai/ainavgraphmanager.h"
#include "game/keywords.h"


// cost funtion for AABBnodes we'll define as distance centre to centre
int AABBCostFunc( CAIAStarNode* from, CAIAStarNode* to, int data, void* cb)
{
	UNUSED( data );
	UNUSED( cb );

	ntAssert( from );
	ntAssert( to );

	CPoint obFromCentre;
	CPoint obToCentre;
	
	int penalty = 0;

	obFromCentre = CAINavGraphManager::Get().GetNodePos( from->navNode );
	obToCentre = CAINavGraphManager::Get().GetNodePos( to->navNode );

	return static_cast<int>( (obFromCentre - obToCentre).Length() ) + penalty;
}

int AABBValidFunc( CAIAStarNode* from, CAIAStarNode* to, int data, void* cb)
{
	UNUSED( data );
	UNUSED( cb );
	UNUSED( from );
	UNUSED( to );

	ntAssert( to );

	if (cb)
	{
		CKeywords* nodeKeywords = CAINavGraphManager::Get().GetNodeKeywords( to->navNode );
		if (nodeKeywords)
		{
			//ntPrintf( "ENTITY KEYWORDS:\n" );
			//static_cast<CKeywords*>( cb )->Dump();
			//ntPrintf( "NODE KEYWORDS:\n" );
			//nodeKeywords->Dump();

			return !(static_cast<CKeywords*>( cb )->operator &( *(nodeKeywords) ));
		}
		return 1;
	}

	return 1;
}



CAStar::CAStar( CKeywords* keywords ) 
{
	m_pKeywords = keywords;

	m_pOpen = m_pClosed = NULL;
	m_pStack = NULL;
	m_pBest = NULL;

	udCost = AABBCostFunc;
	udValid = AABBValidFunc;
	udNotifyChild = NULL;
	udNotifyList = NULL;
}

CAStar::~CAStar() 
{
	ClearNodes();
}

void CAStar::ClearNodes() 
{
	CAIAStarNode*	temp = NULL;
	//CAIAStarNode*	temp2 = NULL;

	while (m_pOpen)
	{
		temp = m_pOpen->next;

		NT_DELETE_CHUNK( Mem::MC_AI, m_pOpen );

		m_pOpen = temp;
	}

	while (m_pClosed)
	{
		temp = m_pClosed->next;

		NT_DELETE_CHUNK( Mem::MC_AI, m_pClosed );

		m_pClosed = temp;
	}
}

/////////////////////////////////////////////////
// CAStar::GeneratePath(int, int, int, int)
//
// Main A* algorithm. The step functions are used
// to keep code redundancy to a minimum.
//


bool
CAStar::GeneratePath( const int start, const int end )
{
	StepInitialize( start, end );
	
	int retval = 0;
	while (retval == 0)
	{
		retval = Step();
	};
	
	if (retval == -1 || !m_pBest)
	{
		m_pBest = NULL;
		return false;
	}

	return true;
}

static int
GetManhattanDist( const int start, const int end )
{
	CPoint	obStartCentre( CAINavGraphManager::Get().GetNodePos( start ) );
	CPoint	obEndCentre( CAINavGraphManager::Get().GetNodePos( end ) );

	return static_cast<int>(
			fabs(obEndCentre.X() - obStartCentre.X())
		+	fabs(obEndCentre.Y() - obStartCentre.Y())
		+	fabs(obEndCentre.Z() - obStartCentre.Z())  );
}

void
CAStar::StepInitialize( const int start, const int end )
{
	ClearNodes();
	
	m_obStart = start;
	m_obEnd = end;

	CAIAStarNode *temp = NT_NEW_CHUNK( Mem::MC_AI ) CAIAStarNode( start );

	temp->g = 0;
	// cheap and dirty heuristic... manhattan distance
	temp->h = GetManhattanDist( start, end );
	// XXX: no heuristic, so for now we do an undirected search
	//temp->h = 0;
	temp->f = temp->g + temp->h;
	m_pOpen = temp;

	udFunc(udNotifyList, NULL, m_pOpen, ASNL_STARTOPEN, m_pNCData);
	udFunc(udNotifyChild, NULL, temp, 0, m_pNCData);
}


int CAStar::Step()
{
	m_pBest = GetBest();

	if (!m_pBest)
	{
		return -1;
	}

	if (m_pBest->navNode == m_obEnd) 
	{
		return 1;
	}

	CreateChildren( m_pBest );

	return 0;
}

CAIAStarNode *CAStar::GetBest() 
{
	if (!m_pOpen) return NULL;

	CAIAStarNode *temp = m_pOpen, *temp2 = m_pClosed;
	m_pOpen = temp->next;

	udFunc(udNotifyList, NULL, temp, ASNL_DELETEOPEN, m_pNCData);

	m_pClosed = temp;
	m_pClosed->next = temp2;

	udFunc(udNotifyList, NULL, m_pClosed, ASNL_ADDCLOSED, m_pNCData);

	return temp;
}

void CAStar::CreateChildren(CAIAStarNode *node) 
{
	CAIAStarNode temp( NULL );

	int	childNavNode;


	for (int i = 0; i < CAINavGraphManager::Get().GetNumConnections( node->navNode ); ++i)
	{
		childNavNode = CAINavGraphManager::Get().GetConnected( node->navNode, i );
		temp.navNode = childNavNode;
		if (!udFunc(udValid, node, &temp, 0, m_pKeywords))
		{
			continue;
		}
		LinkChild(node, &temp);
	}
}


void CAStar::LinkChild(CAIAStarNode *node, CAIAStarNode *temp) 
{
	int	navNode = temp->navNode;
	int g = node->g + udFunc(udCost, node, temp, 0, m_pCBData);
	int num = navNode;

	CAIAStarNode* check = NULL;

	if (CheckList( m_pOpen, num, check ))
	{
		node->children[node->numchildren++] = check;

		// A better route found, so update
		// the node and variables accordingly.
		if (g < check->g)
		{
			check->parent = node;
			check->g = g;
			check->f = g + check->h;
			udFunc(udNotifyChild, node, check, 1, m_pNCData);
		}
		else
		{
			udFunc(udNotifyChild, node, check, 2, m_pNCData);
		}
	}
	else if (CheckList( m_pClosed, num, check ))
	{
		node->children[node->numchildren++] = check;

		if (g < check->g)
		{
			check->parent = node;
			check->g = g;
			check->f = g + check->h;

			udFunc(udNotifyChild, node, check, 3, m_pNCData);

			// The fun part...
			UpdateParents(check);
		}
		else
		{
			udFunc(udNotifyChild, node, check, 4, m_pNCData);
		}
	}
	else
	{
		ntAssert( navNode != -1 );
		CAIAStarNode *newnode = NT_NEW_CHUNK( Mem::MC_AI ) CAIAStarNode(navNode);
		newnode->parent = node;
		newnode->g = g;
		// XXX: nasty manhattan heuristic
		ntAssert( m_obEnd != -1 );
		newnode->h = GetManhattanDist( navNode, m_obEnd );
		newnode->f = newnode->g + newnode->h;

		AddToOpen(newnode);

		node->children[node->numchildren++] = newnode;

		udFunc(udNotifyChild, node, newnode, 5, m_pNCData);
	}

	ntAssert( node->numchildren <= MAX_CHILDREN );
}

bool
CAStar::CheckList(CAIAStarNode *list, int num, CAIAStarNode* &returnNode) 
{
	while (list)
	{
		if (list->navNode == num)
		{
			returnNode = list;
			return true;
		}

		list = list->next;
	}

	return false;
}

void CAStar::AddToOpen(CAIAStarNode *addnode) 
{
	CAIAStarNode *node = m_pOpen;
	CAIAStarNode *prev = NULL;

	if (!m_pOpen)
	{
		m_pOpen = addnode;
		m_pOpen->next = NULL;

		udFunc(udNotifyList, NULL, addnode, ASNL_STARTOPEN, m_pNCData);

		return;
	}

	while(node) 
	{
		if (addnode->f > node->f) 
		{
			prev = node;
			node = node->next;
		}
		else
		{
			if (prev)
			{
				prev->next = addnode;
				addnode->next = node;
				udFunc(udNotifyList, prev, addnode, ASNL_ADDOPEN, m_pNCData);
			}
			else
			{
				CAIAStarNode *temp = m_pOpen;

				m_pOpen = addnode;
				m_pOpen->next = temp;
				udFunc(udNotifyList, temp, addnode, ASNL_STARTOPEN, m_pNCData);
			}

			return;
		}
	}

	prev->next = addnode;
	udFunc(udNotifyList, prev, addnode, ASNL_ADDOPEN, m_pNCData);
}

void CAStar::UpdateParents(CAIAStarNode *node) 
{
	int		g = node->g;
    int		c = node->numchildren;

	CAIAStarNode *kid = NULL;
	for (int i=0; i < c; ++i)
	{
		kid = node->children[i];
		if (g+1 < kid->g)
		{
			kid->g = g+1;
			kid->f = kid->g + kid->h;
			kid->parent = node;
			
			Push(kid);
		}
	}

	CAIAStarNode *parent;
	while (m_pStack)
	{
		parent = Pop();
		c = parent->numchildren;
		for (int i=0;i<c;i++)
		{
			kid = parent->children[i];
			
			if (parent->g+1 < kid->g)
			{
				kid->g = parent->g + udFunc(udCost, parent, kid, 0, m_pCBData);
				kid->f = kid->g + kid->h;
				kid->parent = parent;

				Push(kid);
			}
		}
	}
}

void CAStar::Push(CAIAStarNode *node)
{

	if (!m_pStack)
	{
		m_pStack = NT_NEW_CHUNK( Mem::MC_AI ) AStarStack;
		m_pStack->data = node;
		m_pStack->next = NULL;
	}
	else
	{
		AStarStack *temp =  NT_NEW_CHUNK( Mem::MC_AI )  AStarStack;

		temp->data = node;
		temp->next = m_pStack;
		m_pStack = temp;
	}
}

CAIAStarNode *CAStar::Pop() 
{
	CAIAStarNode *data = m_pStack->data;
	AStarStack *temp = m_pStack;

	m_pStack = temp->next;
	
	NT_DELETE_CHUNK( Mem::MC_AI, temp );

	return data;
}

int CAStar::udFunc(_asFunc func, CAIAStarNode *param1, CAIAStarNode *param2, int data, void *cb)
{
	if (func) return func(param1, param2, data, cb);

	return 1;
}



