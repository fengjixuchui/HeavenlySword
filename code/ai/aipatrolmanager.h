//------------------------------------------------------------------------------------------
//!
//!	aipatrolmanager.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIPATROLMANAGER_H
#define _AIPATROLMANAGER_H

// forward declarations
class AIPatrolPath;
class PatrolNode;

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolManager
//!	Looks after our patrol paths
//!
//!	XXX: currently uses a fixed size array to store the paths, but could really do with
//! being a vector (dynamic size, index access). Are we writing our own, or using std:: ?
//------------------------------------------------------------------------------------------

static const int	MAX_PATHS	=	64;

class AIPatrolManager : public Singleton<AIPatrolManager>
{
public:
	AIPatrolManager();
	~AIPatrolManager();

	void DebugRender();

	AIPatrolPath*	GetPath( int iPathNum )		{ return m_aPaths[iPathNum]; }
	int				GetNumPaths()				{ return m_iNumPaths; }
	int				GetLargestPathNum()			{ return m_iLargestPathNum; }
	AIPatrolPath*	CreatePath( int iPathNum );
	PatrolNode*		GetNearestNodeWithinRange( const CPoint &obPos, float fRange );

private:
	
	AIPatrolPath*	m_aPaths[MAX_PATHS];
	int				m_iNumPaths;
	int				m_iLargestPathNum;
};

//------------------------------------------------------------------------------------------
//!
//!	AIAdHocFormationManager
//!	Provides an extra layer of management for formations created by multiple individual
//!	attackers
//!
//------------------------------------------------------------------------------------------

class AIAdHocFormationManager : public Singleton<AIAdHocFormationManager>
{
public:
	AIAdHocFormationManager()	{};
	~AIAdHocFormationManager()	{};

	void DebugRender();

private:
	
};

//------------------------------------------------------------------------------------------
//!
//!	AIAlertManager
//!	When AIs alert one another to the player's presence, we want it to be in a controlled
//!	way, in order to prevent reciprocal alerts. The first AI to see the player will be
//!	designated the alerter, and all alerts submitted by AIs in range of him will be ignored
//------------------------------------------------------------------------------------------

#define MAX_ALERTS	32

class AIAlertManager : public Singleton<AIAlertManager>
{
public:
	AIAlertManager();
	~AIAlertManager()	{};

	void DebugRender();
	void Update( float fTimeChange );
	void LevelUnload();

	void SendAlert( const CPoint& obAIPos, const CPoint& obPlayerPos );
	bool ActiveAlertInRange( const CPoint &obPos, CPoint& obPlayerPos );
	void SetAlertRange( float fRange )			{ m_fAlertRange = fRange; }
	void SetAlertLifespan( float fLifespan )	{ m_fAlertLifespan = fLifespan; }

private:
	typedef struct
	{
        CPoint	obAIPos;
		CPoint	obPlayerPos;
		float	fAge;
		bool	bActive;
	} RaisedAlert;

	RaisedAlert	m_aRaisedAlerts[MAX_ALERTS];
	int			m_iNumAlerts;
	float		m_fAlertRange;
	float		m_fAlertLifespan;
};



#endif // _AIPATROLPATH_H
