/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef _AI_NAVNODES_H
#define _AI_NAVNODES_H

class CAINavEdge;
class CAINavPath;
class CAINavFollowPoint;
class CKeywords;
class CAINavNodeRegion;

/***************************************************************************************************
*
*	CLASS			CAINavNodeAABB
*
*	DESCRIPTION		An axis aligned bounding box defining a safe zone for AI navigation
*
	NOTES			
*
***************************************************************************************************/
class CAINavNodeAABB
{
public:
	CAINavNodeAABB();
	virtual ~CAINavNodeAABB();

	// member function declarations
	virtual void	PostConstruct( void );

	// public data interface for serialisation
	CPoint	m_obWorldMin;
	CPoint	m_obWorldMax;
	ntstd::String m_obParentNavSet;
	ntstd::String m_obNavSetName;

private:
	CAINavNodeRegion*						m_pobRegion;

};



#include "gfx/triangle.h"

class CAIRegionVertex
{
public:
	CAIRegionVertex()		{};
	virtual ~CAIRegionVertex()		{};

	virtual void	PostConstruct( void )	{ m_bRemoved = false; }

	CPoint	m_obPos;
	bool	m_bRemoved;
	bool	m_bInsideTestTri;

private:

	int m_iIndex;
};
typedef ntstd::List<CAIRegionVertex*,Mem::MC_AI>				RegionVertexList;

class AINavTriInfo
{
	public:
		int		iNumConnected;
		int		aConnected[3];
		CPoint	aConnectPos[3];


	private:
};


#include "ai/ainavdata.h"
typedef ntstd::List<CPoint*,Mem::MC_AI>							AIPointList;


class CAINavNodeRegion
{
public:
	CAINavNodeRegion()		{ m_aVertices = 0; m_aTriangles = 0; };
	virtual ~CAINavNodeRegion();

	// member function declarations
	virtual void	PostConstruct( void );
	void			Preprocess();

	void			PaulsDebugRender( void ) const;

	int				GetNumVertices()	{ if (m_aVertices == NULL) {return 0;} return m_obPoints.size(); }
	int				GetNumTriangles()	{ if (m_aTriangles == NULL) {return 0;} return m_obTriangles.size(); }
	int				GetNumConnections()	{ return m_obConnections.size() + m_obExternalConnections.size(); }
	void			SetBaseValues( int iBaseVertIdx, int iBaseTriIdx)	{ m_iBaseVertIdx = iBaseVertIdx; m_iBaseTriangleIdx = iBaseTriIdx; }
	int				GetTriBaseIdx()		{ return m_iBaseTriangleIdx; }

	void			ReBaseInternal();
	void			ReBaseExternal();

	void			MakeConnections( CAINavNodeRegion*	otherRegion );
	int				ContainingTriangle( CPoint&	pos );

	// data members made public for Welder, they should still only be used
	// through their accessors
	float								m_fWorldMinHeight;
	float								m_fWorldMaxHeight;
	ntstd::String						m_obParentNavSet;
	ntstd::String						m_obNavSetName;
	RegionVertexList					m_obPoints;

	friend class AINavData;	
private:

	struct ExternalConnection
	{
		int					edgeIndices[2];		// indices in this region
		int					triangleIdx;		// triangle in this region
		CAINavNodeRegion*	otherRegion;
		int					otherTriangleIdx;	// refers to triangle in otherRegion		
	};
	typedef ntstd::List<ExternalConnection*,Mem::MC_AI>				ExternalConnectionList;

	void MakeBoundingBox();
	void GetBoundingBox( CPoint& min, CPoint& max );

	CPoint	m_WorldMin;
	CPoint	m_WorldMax;

	AITriangleList								m_obTriangles;				// list of triangles comprising the region
	AIPointList									m_obConnectionPositions;
	AINavDataStructsConnectionList				m_obConnections;
	AIPointList									m_obExternalConnectionPositions;
	ExternalConnectionList						m_obExternalConnections;

	int	m_iBaseVertIdx;
	int	m_iBaseTriangleIdx;

	double	(*m_aVertices)[2];
	int		(*m_aTriangles)[3];
};








#endif // _AI_NAVNODES_H
