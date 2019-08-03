#include "core/vecmath.h"

/***************************************************************************************************
*
*	CLASS			CAINavFollowPoint
*
*	DESCRIPTION		Update our current state
*
***************************************************************************************************/

class CAINavFollowPoint
{
public:
	CAINavFollowPoint() :
		m_obPos(CONSTRUCT_CLEAR),
		m_obOffset(CONSTRUCT_CLEAR),
		m_obOffsetPos(CONSTRUCT_CLEAR),
		m_pobCurrentPath(NULL),
		m_pobCurrentEdge(NULL),
		m_bCachedPosValid(false),
		m_bPosSet(false)					{ m_iPathNum = m_iNumFollowers++; };

	~CAINavFollowPoint()	{ }

	void		Update( float fTimeChange );
	void		DebugRender();

	const CPoint&			GetPos()	const								{ m_obOffsetPos = m_obPos + m_obOffset; return m_obOffsetPos; }
	void					SetPos( const CPoint& obPos )					{ m_obPos = obPos; m_bPosSet = true; /*ntPrintf( "setting follow point\n" );*/ }
	void					SetDest( const CPoint& obDest )					{ m_obDest = obDest; }
	CPoint					GetDest()										{ return m_obDest; }
	void					SetDistSquared( const float fDistSq )			{ m_fDistSquared = fDistSq; }
	bool					PosSet()										{ return m_bPosSet; }
	void					SetCurrentPath( CAINavPath* pobCurrentPath )	{ m_pobCurrentPath = pobCurrentPath; m_pobCurrentEdge = NULL; }
	CAINavPath* 			GetCurrentPath( )								{ return m_pobCurrentPath; }
	void					SetOffset( const CPoint& obOffset )				{ m_obOffset = obOffset; }
	void					SetAheadDist( const float fAheadDist )			{ m_fAheadDist = fAheadDist; }

private:
	static int		m_iNumFollowers;
	int				m_iPathNum;

	bool			AtDestination();

	CPoint			m_obPos;
	CPoint			m_obOffset;
	mutable CPoint	m_obOffsetPos;
	CPoint			m_obDest;
	CAINavPath*		m_pobCurrentPath;
	int				m_pobCurrentEdge;
	float			m_fDistSquared;
	float			m_fAheadDist;
	bool			m_bCachedPosValid;
	CPoint			m_obWaypointPos;
	bool			m_bPosSet;
};


