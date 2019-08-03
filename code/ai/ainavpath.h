/***************************************************************************************************
*
*	CLASS			CAINavPath
*
*	DESCRIPTION		A path between 2 points, composed of edges between nodes in a navgraph
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _AINAVPATH_H
#define _AINAVPATH_H

class CAINavEdge;
class CAINavNodeAABB;

#include "aidefines.h"

class CAINavPath
{
public:
	CAINavPath( void ) :
		m_obStartNode(0), m_obDestNode(0), m_pobEdgeItr(0),
		m_obPathStart(CONSTRUCT_CLEAR), m_obPathDest(CONSTRUCT_CLEAR), m_bStringPull(true) {};

	~CAINavPath( void );

	void			Update();
	void			DebugRender();

	void			AddEdge( int obEdge )	{ m_obEdgeList.push_front( obEdge ); }
	int				GetNextEdge();
	void			SetNodes( const int obStartNode, const int obDestNode)	{ m_obStartNode = obStartNode; m_obDestNode = obDestNode ; }
	int				GetDestNode()	{ return m_obDestNode; }
	int				GetEdge( int iEdgeNum );	// iterates and counts, not efficient for long paths

	void			CleanLastEdge( CAINavNodeAABB* obDestNode );
	void			KillFirstEdge();

	void			Simplify();
	void			SetStringPulling( bool bStringPull )	{ m_bStringPull = bStringPull; } 

	void			SetStart( const CPoint& obStart )		{ m_obPathStart = obStart; }
	void			SetDest( const CPoint& obDest )		{ m_obPathDest = obDest; }

	typedef			ntstd::List<int, Mem::MC_AI> EdgeList;
private:

	void						CleanUp();

	int							m_obStartNode;
	int							m_obDestNode;
	EdgeList					m_obEdgeList;		// list of edges to traverse
	EdgeList::iterator*			m_pobEdgeItr;		// iterator keeping track of where we are on the path

	CPoint						m_obPathStart;
	CPoint						m_obPathDest;

	bool						m_bStringPull;
};

#endif
