//! -------------------------------------------
//! AINavigPath.h
//!
//! Navigation path returned by CAINavigStar
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AINAVIGPATH_H
#define _AINAVIGPATH_H

#include "ainavignode.h"

// Forward declarations


class CAINavigPath
{
	public:
		
		CAINavigPath() : m_obLastNode(NULL), m_obStartNode(NULL), m_bPathFinished(false),
						m_bPassedStartNodeWasValid(false), m_bPassedEndNodeWasValid(false), m_bStartEndNodesFromDifNavigGraph(false),
						m_bContainsDynamicallyAllocatedNodes(false), m_obPosWithinCurrentNode(CONSTRUCT_CLEAR), m_bFirstFollowPathFrame(true) {}
		~CAINavigPath();

		void			push_back		( CAINavigNode* pN)	{ if (pN) {m_listNavigNodePath.push_back(pN);}  m_bFirstFollowPathFrame = true; }
		void			push_front		( CAINavigNode* pN) { if (pN) {m_listNavigNodePath.push_front(pN);} m_bFirstFollowPathFrame = true; }
		void			clear			( void ); //			{ m_listNavigNodePath.clear(); }
		bool			empty			( void )			{ return (m_listNavigNodePath.empty()); }
		unsigned int	size			( void )			{ return (m_listNavigNodePath.size()); }
		bool			IsFinished		( void )			{ return (m_bPathFinished); }
		bool			IsSingleNodePath( void )			{ return (m_listNavigNodePath.size()==1); }
		CAINavigNode*	GetStartNode	( void )			{ return (empty() ? NULL : (*(m_listNavigNodePath.begin())));}
		CAINavigNode*	GetGoalNode		( void );

		bool			IsStartNode		( void )			{ return (GetCurrentNode() == m_obStartNode); }
		bool			IsLastNode		( void )			{ return (GetCurrentNode() == m_obLastNode); }

		void			PointToFirstNode( void )			{ m_obIt = m_listNavigNodePath.begin(); m_bPathFinished = false; m_bFirstFollowPathFrame = true;}
		void			PathReady		( void );

		bool			PointToNextNode	( void );
		bool			PointToPrevNode	( void );
		CAINavigNode*	PeekPreviousNode( void );
		CAINavigNode*	GetCurrentNode	( void );
		CAINavigNode*	PeekNextNode	( void );
		unsigned int	TrimPath		( bool );
		void			DeletePreviousNode( void );
		void			DeleteCurrentNode	( void );
		void			EnhanceFirstNode	( const CPoint& );

		bool			IsStartNodeValid	( void ) const	{ return m_bPassedStartNodeWasValid; }
		bool			IsEndNodeValid		( void ) const	{ return m_bPassedEndNodeWasValid; }
		void			SetStartNodeValid	( bool b )		{ m_bPassedStartNodeWasValid = b; }
		void			SetEndNodeValid		( bool b )		{ m_bPassedEndNodeWasValid = b; }
		bool			GetDifNavigGraphError ( void )		{ return m_bStartEndNodesFromDifNavigGraph; }
		void			SetDifNavigGraphError ( bool b )	{ m_bStartEndNodesFromDifNavigGraph = b; }

		// Extension for cover points
		CAINavigCoverPoint*	GetAvailableCoverPoint ( CAIMovement * );

		// Extension for dynamically allocated nodes (for GoAroundVolumes)
		void			SetDeallocateManually( bool b ) { m_bContainsDynamicallyAllocatedNodes = b; }
		bool			GetDeallocateManually( void )	{ return m_bContainsDynamicallyAllocatedNodes; }

		// Extension for follow path with WIDTH
		void			SetPointWithinCurrentNode	( const CPoint& obPos ) { m_obPosWithinCurrentNode = obPos; }
		CPoint			GetPointWithinCurrentNode	( void ) const			{ return m_obPosWithinCurrentNode; }
		bool			IsFirstFollowPathFrame		( void ) const			{ return m_bFirstFollowPathFrame; }
		void			SetFirstFollowPathFrame		( bool b )				{ m_bFirstFollowPathFrame = b; }

		// Extension for Creating Paths that contains certain number of nodes ( NOT USED NOR FULLY IMPLEMENTED )
		//void				SetIntermediateNodes	( AINavigNodeList* pNNS) { m_plistIntermediateNodes = pNNS; }
		//AINavigNodeList*	GetIntermediateNodes()	{ return m_plistIntermediateNodes; }

		// !!! - Debug

		void			PrintPath		( void );
		void			RenderPath		( void );

	private:
	
		//AINavigNodeList*			m_plistIntermediateNodes;
		AINavigNodeList				m_listNavigNodePath;
		AINavigNodeList::iterator	m_obIt;
		CAINavigNode*				m_obLastNode;
		CAINavigNode*				m_obStartNode;
		bool						m_bPathFinished;
		bool						m_bPassedStartNodeWasValid; // user to store the result of Makepath in terms of valid/invalid start node
		bool						m_bPassedEndNodeWasValid;	// user to store the result of Makepath in terms of valid/invalid end node
		bool						m_bStartEndNodesFromDifNavigGraph;
		bool						m_bContainsDynamicallyAllocatedNodes;
		CPoint						m_obPosWithinCurrentNode;
		bool						m_bFirstFollowPathFrame;
};

#endif // _AINAVIGFOLLOWNODE_H 

