//! --------------------------------------------------------
//! AINavigGraphManager.h
//!
//! New AIs Navigation Graph Manager
//!
//! It handles AIs navigation system for the whole level 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//! Date	04/05/06 - Creation
//!--------------------------------------------

#ifndef _AINAVIGGRAPHMANAGER_H
#define _AINAVIGGRAPHMANAGER_H

#include "ainavigastar.h"
#include "ainaviggraph.h"
#include "aipatrolgraph.h"

// Forward declarations


class CAINavigGraphManager
{

	public:

		// Ctor et al.

		CAINavigGraphManager();
		CAINavigGraphManager::~CAINavigGraphManager();

		// Methods
		// Note: Enable means [DOOR -> Open, LADDER -> In place]
		CAINavigNode*						GetNodeWithName		(const CHashedString &);
		CPoint								GetNodePosWithName	(const CHashedString &, bool*, float *);
	//	CAINavigNode*					GetNode				( CEntity*, CAINavigGraph* );	// !!! - Needed?
		bool								IsDoorOpen			( CEntity* );
		bool								IsLadderAvailable	( CEntity* );
		bool								SetEnableDoor		( CEntity*, bool );
		bool								SetEnableLadder		( CEntity*, bool );
//		bool								DeleteNode			( CEntity* );
//		bool								IsDoor				( CEntity* pEnt) { return ( GetNodeType(pEnt) == NDT_DOOR ); }
//		bool								IsLadder			( CEntity* pEnt) { return ( GetNodeType(pEnt) == NDT_LADDER ); }
		CAINavigNode*						GetClosestNode		( const CPoint&,  float*  );
		const AINavigArrowList*				GetTgtArrows		( CAINavigNode* ) const;
		CAINavigGraph*						GetNavigGraph		( CHashedString hsNGName );
		void								Add					( CAINavigGraph* pNG);

		void								MakePath			( const CPoint&, const CPoint&, CAINavigPath*);
		void								MakePath			( const CPoint&, CAINavigNode*, CAINavigPath*);
		void								MakePath			( const CPoint&, const CHashedString &, CAINavigPath*);
		void								MakePath			( CAINavigNode*, CAINavigNode*, CAINavigPath*);

		// Patrol Graphs
		CAIPatrolGraph*						GetPatrolGraph		( const char* psName );
		void								AddPatroller		( AI* , const char* );
		void								RemovePatroller		( CEntity* , const char* );
//		CAINavigNode*						GetCurrentNode		( CEntity* , const char* );		// Returns the current node a patroller is going to
//		void								PointToNextNode		( CEntity* , const char* );		// Sets the next node as target (where to go)
		CAINavigGraph*						GetPatrollersGraph	( CEntity* );

		// Cover Point

		CAINavigCoverPoint*					GetClosestCoverPointInMinMaxRange	( AI* , const CEntity*, CAINavigGraph* = NULL  );
		CAINavigNode*						GetANodeLinkedToCoverPoint	( CAINavigCoverPoint*, CAINavigGraph* = NULL ) const;

		CAINavigNode*						GetFirstVisibleNode		( const CPoint &, CAINavigGraph* = NULL  );
		CAINavigNode*						GetAbsoluteClosestNode	( const CPoint &obPoint, CAINavigGraph* pNG = NULL );

		bool								IsNavigGraphActiveLUA	( CHashedString );
		void								SetNavigGraphActiveLUA	( CHashedString, bool );

		// Extension For Set of Intermediate Nodes
		//AINavigNodeList*					GetIntermediateNodeList		( CHashedString, CAINavigGraph* = NULL );
	
		// For Debugging

		void DebugRenderNavigGraph	( void );
		void DebugRenderPathGraph	( void );

	private:

		CAINavigArrow* GetLadder	( CEntity* pEnt );
		CAINavigArrow* GetDoor		( CEntity* pEnt );

	private:
		
		AINavigGraphList				m_listNavigGraphs;
		AIPatrolGraphList				m_listPatrolGraphs;
		AINavigGraphList				m_listQueueGraphs;
		CNavigAStar						m_obAStar;
		//CAINavigPath					m_pMyPath;	// For DebugRender
		int								m_iNumberOfRegionsInLevel;
		int								m_iNumberOfPatrolGraphsInLevel;
};

#endif // _AINAVIGGRAPHMANAGER_H
