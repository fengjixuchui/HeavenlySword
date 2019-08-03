//! -------------------------------------------
//! AIPatrolPath.cpp
//!
//! Navigation path for patroling 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------


#ifndef _AIPATROLGRAPH_H
#define _AIPATROLGRAPH_H

#include "ainaviggraph.h"

// Forward declarations

class CAINavigNode;
class CAINavigGraph;
class CEntity;
class CAINavigPath;

#define MAX_PATROLLERS_PER_PATROL_PATH 5

typedef struct _SPatroller
{
	_SPatroller()								: pEntPatroller(NULL), uiCurrentNodeIndex(0), bGoingBackwards(false) {}
	_SPatroller( CEntity* pE, unsigned int ui )	: pEntPatroller(pE), uiCurrentNodeIndex(ui), bGoingBackwards(false) {}
	
	CEntity*		pEntPatroller;
	unsigned int	uiCurrentNodeIndex;
	bool			bGoingBackwards;
} SPatroller;

typedef ntstd::Vector<SPatroller*, Mem::MC_AI> SPatrollerVector;

class CAIPatrolGraph
{
	public:
		
		CAIPatrolGraph( CAINavigGraph* ); 
		~CAIPatrolGraph();

		
		CHashedString	GetName				( void ) { return (m_ksName); }						// returns the name of the Patrol Graph
		unsigned int	size				( void		) { return (m_vNavigNodePath.size()); }		// Returns the number of nodes in the path
		CAINavigNode*	GetCurrentNode		( CEntity*	);											// Returns the current node a patroller is going to
		void			PointToNextNode		( CEntity*	);											// Sets the next node as target (where to go)
		bool			AddPatroller		( CEntity*	);											// Adds a patroller to this path
		bool			RemovePatroller		( CEntity*	);											// Removes a patroller from this path
		CAINavigGraph*	GetPatrollersGraph	( CEntity* pEnt	) { if (pEnt && (GetPatrollerIndex(pEnt)>=0)) return (m_pNavigGraph); else return NULL; }
		
		const CAINavigNode*	GetPatrolLocator( void		) const { return  (m_pNavigGraph->GetPatrolLocator()); }
		CAINavigNode*	GetClosestNode		( CEntity*	);											// Returns the current node a patroller is going to
		void			MakePath			( CEntity*, CEntity*, CAINavigPath* );
	
		// Debug

		void DebugRender ( void );

	private:

		int				GetPatrollerIndex	( CEntity*	);											// Returns the patrolle's index
		int				GetNodeIndex		( CAINavigNode* );


	private:
	
		AINavigNodeVector				m_vNavigNodePath;
		SPatrollerVector				m_vPatrollers;
		CAINavigGraph*					m_pNavigGraph;			// Pointer to the original NavigGrpah
		CHashedString					m_ksName;
		bool							m_bOpenGraph;
		unsigned int					m_uiNumOfPatrollers;
		unsigned int					m_uiGraphSize;
};

typedef ntstd::List<CAIPatrolGraph*, Mem::MC_AI> AIPatrolGraphList;


#endif // _AIPATROLGRAPH_H 


