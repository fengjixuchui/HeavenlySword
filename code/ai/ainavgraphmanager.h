/***************************************************************************************************
*
*	CLASS			CAINavGraphManager
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _AINAVGRAPHMANAGER_H
#define _AINAVGRAPHMANAGER_H

class AIObjectAvoidance;
class CAINavNodeAABB;
class CAINavNodeRegion;
class CAINavPath;
class AIGroupNavBrain;
class AISafeZone;
class AITestManager;
class CEntity;
class AIIntermediateNavData;
class AINavData;
class CKeywords;

#include "ai/aidefines.h"
#include "ai/ainavdata.h"
#include "ai/aidebugwall.h"

enum	AINAVNODE_AABB_CREATE_STATE
{
	AINAVNODE_AABB_CREATE_STATE_START = 0,
	AINAVNODE_AABB_CREATE_STATE_END,
};

enum	INGRAPH_RETURN
{
	INGRAPH_SAME_TRI,
	INGRAPH_DIFFERNT_TRI,
	INGRAPH_START_OUT,
	INGRAPH_END_OUT,
	INGRAPH_BOTH_OUT,
	INGRAPH_ERROR,
};

/***************************************************************************************************
*
*	STRUCT			AINavHint
*
*	DESCRIPTION		
*
***************************************************************************************************/

struct AINavHint
{
	AINavHint() : m_Region(-1), m_Tri(-1) {}
	short	m_Region;
	short	m_Tri;
};

/***************************************************************************************************
*
*	struct			AINavCache
*
*	DESCRIPTION		
*
***************************************************************************************************/

struct AINavCache
{
	AINavHint	m_aobHints[5];
};


/***************************************************************************************************
*
*	CLASS
*
*	DESCRIPTION		
*
***************************************************************************************************/
class CAINavGraphManager : public Singleton<CAINavGraphManager>
{
public:
	CAINavGraphManager( void );
	~CAINavGraphManager( void );

	void			LevelUnload();
	void			Update( float fTimeChange );
	void			DebugRender();
	bool			IsDebugRenderOn() const		{ return m_bRender; };
	bool			m_bRender;

	bool			HasNavData() const;

	CPoint	GetRandomPosInGraph() const;
	CPoint	GetNearestPosInGraph( const CPoint &pos ) const;
	int		GetNearestNode( const CPoint &obPos ) const;

	CPoint	GetRandomReachablePosInGraph( const CPoint &obPos ) const;

	// replacements for all the member functions we used to have on AABB nodes
	bool	NodeContains( int nodeNum, const CPoint& obPos ) const;
	int		GetConnection( int nodeA, int nodeB ) const;
	CPoint	GetConnectionPos( int connectionNum ) const;
	int		GetNumConnections( int nodeNum ) const;
	int		GetConnected( int nodeNum, int connectionNum ) const;
	CPoint	GetNodePos( int nodeNum ) const;
	CPoint	GetRandomPosInNode( int nodeNum ) const;
	CKeywords*	GetNodeKeywords( int nodeNum ) const;
	bool	IsPosLegal( const CPoint& pos, CKeywords* keywords ) const;

	void			AddRegion( CAINavNodeRegion* const );

	CAINavPath*		MakePath( const CPoint& obFrom, const CPoint& obTo, CKeywords* );
	bool			InGraph( const CPoint& obPos, const CKeywords* pobKeys = 0, AINavHint* pHint = 0 ) const;
	bool			InGraph( const CPoint& obPos, const float radius, const CKeywords* pobKeys = 0, AINavCache* pCache = 0 ) const; 
	INGRAPH_RETURN	InGraph( const CPoint& obP1, const CPoint& obP2, CMatrix* pIntersectResult, const CKeywords* pobKeys = 0 ) const; 
	bool			InSafeZone( const CPoint& obPos, const float radius = 0.0f ) const;

	int				GetNumNodes() const;

	AIObjectAvoidance*	GetRepulsion() const	{ return m_pobRepulsion; }

	static bool		HasLineOfSight( const CPoint& obFrom, const CPoint& obTo, const CEntity* pobIgnoreEntity, const float );
	static bool		HasLineOfSightWithWidth( const CPoint& obFrom, const CPoint& obTo, const CEntity* pobIgnoreEntity, const float, const float  );
	static void		CAINavGraphManager::ClampPointToFloor( CPoint& obPos );
	bool			LineContainedByGraph( CPoint& from, CPoint& to, int& nodeHint );

	void			AddWall( AIDebugWall* const pobWall )		{ m_obDebugWalls.push_back( pobWall ); }
	void			AddSafeZone( AISafeZone* const pobZone )	{ m_safeZone = pobZone; }
	AISafeZone*		GetSafeZone( void) const					{ return m_safeZone; }

	void			SetPathfindTestTarget( const CPoint& obTarget )				{ m_obPathfindTestTarget = obTarget; m_bPathfindTestTargetChanged = true; }
	CPoint			GetPathfindTestTarget()	const								{ return m_obPathfindTestTarget; }
	void			SetPathfindTestRun( bool bRun )								{ m_bPathfindTestRun = bRun; m_bPathfindTestTargetChanged = true; }
	bool			GetPathfindTestRun() const									{ return m_bPathfindTestRun; }
	bool			HasPathfindTestChanged()									{ return m_bPathfindTestTargetChanged; }
	void			AcknowledgePathfindTestChange()								{ m_bPathfindTestTargetChanged = false; }
	void			SetPathfindTestObstaclePos( const CPoint& obObstaclePos )	{ m_obPathfindTestObstaclePos = obObstaclePos; }

	// Functions to be invoked by keyboard/script
	void			CreateNewAABB();

private:

	void			FirstFrameInit( void );

	typedef ntstd::List<CAINavNodeAABB*, Mem::MC_AI> AINavNodeAABBList;
	AINavNodeAABBList				m_obNavNodes;		// list of nodes in the level
	CAINavNodeAABB*					m_pobCurrentNode;	// the most recently created node
	AINAVNODE_AABB_CREATE_STATE		m_eCreateState;		// will the next "create" button press begin or end an AABB

	AINavNodeRegionList				m_obNavRegions;		// list of region nodes in the level
	
	// structures for loading nav data
	AIIntermediateNavData*			m_pobIntermediateData;
	AINavData*						m_pobNavData;

	CPoint							m_obTestStartPoint;
	CPoint							m_obTestEndPoint;

	CPoint							m_obPathfindTestTarget;
	CPoint							m_obPathfindTestObstaclePos;
	CEntity*						m_obPathfindTestObstacle;
	bool							m_bPathfindTestRun;
	bool							m_bPathfindTestTargetChanged;
	bool							m_bDrawPathfindTestTarget;

	unsigned						m_uNumNodes;

	AIObjectAvoidance*					m_pobRepulsion;

	int								m_iLastNodeAdded;
	bool							m_bFirstFrame;

	AIGroupNavBrain*				m_pobGroupNav;

	AIDebugWallList					m_obDebugWalls;

	CAINavPath*						m_pobTestPath;

	AISafeZone*						m_safeZone;
};

/***************************************************************************************************
*
*	CLASS
*
*	DESCRIPTION		
*
***************************************************************************************************/
class AITest
{
public:
	AITest();
	virtual ~AITest();

	virtual void	Update() = 0;

private:

};

/***************************************************************************************************
*
*	CLASS
*
*	DESCRIPTION		
*
***************************************************************************************************/
class AITestWander : public AITest
{
public:
	AITestWander();
	virtual ~AITestWander();

	virtual void	Update();

private:

};

#endif
