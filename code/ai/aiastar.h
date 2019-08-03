
#ifndef _AIASTARNODE_H_
#define _AIASTARNODE_H_

// forward declarations
class	CAINavNodeAABB;
class	CKeywords;

#include "aidefines.h"
#include "aiastarnode.h"

class CAStar {
	public:
		CAStar( CKeywords* );
		~CAStar();

		_asFunc	 udCost;			// Called when cost value is need
		_asFunc  udValid;			// Called to check validity of a coordinate
		_asFunc  udNotifyChild;		// Called when child is added/checked (LinkChild)
		_asFunc	 udNotifyList;		// Called when node is added to Open/Closed list

		void	*m_pCBData;			// Data passed back to callback functions
		void	*m_pNCData;			// Data passed back to notify child functions

		bool	GeneratePath( const int, const int );
		void	StepInitialize( const int, const int );

		int		Step();
		void	SetRows(int r)		 { m_iRows = r;    }
		void	Reset() { m_pBest = NULL; }

		CAIAStarNode	*GetBestNode() { return m_pBest; }

	protected:
		// path initialisation details for 
		int		m_iRows;			// Used to calculate node->number
		int		m_iSX, m_iSY, m_iDX, m_iDY, m_iDNum;

		int						m_obStart;
		int						m_obEnd;

		CAIAStarNode*	m_pOpen;			// The open list
		CAIAStarNode*	m_pClosed;			// The closed list
		CAIAStarNode*	m_pBest;			// The best node
		AStarStack*		m_pStack;			// Propagation stack

		CKeywords*		m_pKeywords;		// Searches made will add a penalty to nodes matching these keywords

		// Functions.
		void			AddToOpen(CAIAStarNode *);
		void			ClearNodes();
		void			CreateChildren(CAIAStarNode *);
		void			LinkChild(CAIAStarNode *, CAIAStarNode *);
		void			UpdateParents(CAIAStarNode *);

		// Stack Functions.
		void			Push(CAIAStarNode *);
		CAIAStarNode*	Pop();
		
		bool			CheckList( CAIAStarNode*, int, CAIAStarNode*& );
		CAIAStarNode*	GetBest();
		
		// Inline functions.
		inline int		Coord2Num(int x, int y) { return x * m_iRows + y; }
		inline int		udFunc(_asFunc, CAIAStarNode *, CAIAStarNode *, int, void *);
};

#endif
