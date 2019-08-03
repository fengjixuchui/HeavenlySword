#ifndef _AINAVDATA_H
#define _AINAVDATA_H

// forward declarations
class CAIComponent;
class CAINavNodeRegion;
class AIIntermediateNavData;
class CKeywords;
struct AINavHint;

namespace AINavDataStructs
{
	struct Connection
	{
		int verts[2];	// the verts making up the edge for this connection
		int tris[2];	// the triangles joined by this connection
	};
};
typedef ntstd::List<AINavDataStructs::Connection*,Mem::MC_AI>	AINavDataStructsConnectionList;


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::AINavData
//!                                                                                         
//! Stores the vertex, triangle and connectivity data used by the pathfinder
//!                                                                                         
//------------------------------------------------------------------------------------------

class AINavData
{
public:

	AINavData( AIIntermediateNavData*	pobIntData );
	~AINavData();

	void DebugRender();

	int			GetContainingNode( const CPoint& obPos, AINavHint* pHint = 0, const CKeywords* pobKeys = NULL );
	bool		GetIntersectionEdge( u_int uiTri, const CPoint& obPIn, const CPoint& obPOut, CMatrix* pResult );
	void		AddKeywordToRegion( const char* keyword, const int idx );
	CKeywords*	GetNodeKeywords( int nodeNum ) const;

	friend class CAINavGraphManager;
private:

	CPoint	GetNodeCentre( int nodeNum );
	bool	IsConnectionExternal( int connIdx );
	bool	IsPointInRegionBB( const CPoint& pos, int regionIdx );
	bool	IsPointInTriangle( const CPoint& pos, int triangleIdx );
	int		GetRegionForTriangle( int triangleIdx );
	void	JoinPointsRender( int a, int b, int region, uint32_t colour );

	static const int AI_CONNECTIONS_PER_TRIANGLE = 16;

	struct Region
	{
		CPoint		obWorldMin;
		CPoint		obWorldMax;
		CPoint		obCentre;
		CPoint		obExtents;
		int			minTriIndex;
		int			maxTriIndex;
		int			minVertIndex;
		int			maxVertIndex;
		CKeywords*	keywords;
		CPoint*		debugRenderTriangleList;
	};
	struct Triangle
	{
		int a;
		int b;
		int c;

		int connections[AI_CONNECTIONS_PER_TRIANGLE];
	};

	Region*							m_aRegions;
	CPoint*							m_aVertices;
	Triangle*						m_aTriangles;
	AINavDataStructs::Connection*	m_aConnections;
	int								m_iNumRegions;
	int								m_iNumVerts;
	int								m_iNumTriangles;
	int								m_iNumConnections;
	
};

typedef ntstd::List<CAINavNodeRegion*, Mem::MC_AI> AINavNodeRegionList;

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AINavData::AINavData
//!                                                                                         
//! Stores navdata, as loaded from XML, prior to preprocessing
//!                                                                                         
//------------------------------------------------------------------------------------------

class AIIntermediateNavData
{
public:
	AIIntermediateNavData() : m_bPreprocessed(false)	{}
	~AIIntermediateNavData();
	
	void	AddRegion( CAINavNodeRegion* pobRegion );
	void	Preprocess();

	AINavNodeRegionList					m_obRegionList;
	int							m_iNumVerts;
	int							m_iNumTris;
	int							m_iNumConnections;

	bool						m_bPreprocessed;
};

#endif
