//------------------------------------------------------------------------------------------
//!
//!	aipatrolpath.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIPATROLPATH_H
#define _AIPATROLPATH_H

// forward declarations
class PatrolNode;

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolPath
//!	List of patrol nodes forming a path
//!
//------------------------------------------------------------------------------------------

class AIPatrolPath
{
public:
	AIPatrolPath( int iPathNum ) : m_iPathNum( iPathNum ), m_iNumNodes( 0 )		{}
	~AIPatrolPath();

	void AddNode( PatrolNode* pobNode );
	void DebugRender();

	int	GetNumNodes()	{ return m_iNumNodes; }
	PatrolNode*	GetNode( int iNodeNum );

	typedef ntstd::List<PatrolNode*, Mem::MC_AI> PatrolNodeList;
private:
	PatrolNodeList				m_obNodeList;
	int							m_iPathNum;
	int							m_iNumNodes;
};




#endif // _AIPATROLPATH_H
