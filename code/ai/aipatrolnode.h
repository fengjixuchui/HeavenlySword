//------------------------------------------------------------------------------------------
//!
//!	aipatrolnode.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIPATROLNODE_H
#define _AIPATROLNODE_H

//------------------------------------------------------------------------------------------
//!
//!	PatrolNode
//!	A point on a patrol path for AI characters
//!
//------------------------------------------------------------------------------------------

class PatrolNode
{
public:

	// This interface is exposed
	HAS_INTERFACE( PatrolNode );
	virtual ~PatrolNode() {};

	PatrolNode();

	virtual void	PostConstruct();

	void PaulsDebugRender();

	// accessors
	CPoint	GetPos() const					{ return m_obPos; }
	void	SetPos( const CPoint& obPos )	{ m_obPos = obPos; }
	int		GetPath() const 				{ return m_iPathNum; }
	void	SetPath( const int iPathNum );
	int		GetNum()						{ return m_iNodeNum; }
	void	SetNum( const int iNodeNum )	{ m_iNodeNum = iNodeNum; }

	// public data for Welder
	CPoint	m_obPos;
	int		m_iPathNum;
	int		m_iNodeNum;

private:
	bool		m_bEnabled;
	ntstd::String	m_strStatus;

};

#endif // _AIPATROLNODE_H
