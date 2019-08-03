#ifndef _AIASTARNODE_H
#define _AIASTARNODE_H

#define ASNL_ADDOPEN		0
#define ASNL_STARTOPEN		1
#define ASNL_DELETEOPEN		2
#define ASNL_ADDCLOSED		3

#define ASNC_INITIALADD		0
#define ASNC_OPENADD_UP		1
#define ASNC_OPENADD		2
#define ASNC_CLOSEDADD_UP	3
#define ASNC_CLOSEDADD		4
#define ASNC_NEWADD			5

#define MAX_CHILDREN		32

#include "aidefines.h"

class CAIAStarNode
{
	public:
		CAIAStarNode( const int nav )
			:	navNode(nav),
				numchildren(0)
		{
			parent = 0;
			next = 0;
			dataptr = 0;
			memset(children, 0, sizeof(children));
		}

		int				f,g,h;			// Fitness, goal, heuristic.
		int	navNode;					// Nav Node represented by this astar node
		int				numchildren;
		CAIAStarNode*	parent;
		CAIAStarNode*	next;			// For Open and Closed lists
		CAIAStarNode*	children[MAX_CHILDREN];	// XXX: needs replacing with something fast and dynamic
		void*			dataptr;		// Associated data
};

// Stack for propagation.
struct AStarStack
{
	CAIAStarNode*	data;
	AStarStack*		next;
};

typedef int(*_asFunc)( CAIAStarNode*, CAIAStarNode*, int, void* );

#endif
