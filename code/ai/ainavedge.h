/***************************************************************************************************
*
*	CLASS			CAINavEdge
*
*	DESCRIPTION		An edge between 2 nav nodes, with a position representing a point to head for
*					when moving between those nodes
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _AINAVEDGE_H
#define _AINAVEDGE_H

#include "aidefines.h"

class CAINavEdge
{
public:
	CAINavEdge() {}
	CAINavEdge( CPoint &obPos, unsigned uFrom, unsigned uTo ) : m_obPos(obPos), m_uFrom(uFrom), m_uTo(uTo) { }
	~CAINavEdge() {}

	CPoint		GetPos()
	{
		return m_obPos;
	}

	unsigned	GetTo()								{ return m_uTo; }

	void		SetExtents( CPoint&	obExtents )		{ m_obExtents = obExtents; }

private:

	CPoint		m_obPos;
	CPoint		m_obExtents;
	unsigned	m_uFrom;
	unsigned	m_uTo;
};

#endif
