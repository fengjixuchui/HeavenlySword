//------------------------------------------------------------------------------------------
//!
//!	aipatrolnode.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AICOVERPOINT_H
#define _AICOVERPOINT_H

class FirePoint;

//------------------------------------------------------------------------------------------
//!
//!	AILocator
//!
//!	A serialisable locator class for various AI systems to derive from
//!
//------------------------------------------------------------------------------------------

class AILocator
{
public:
	AILocator();
	virtual ~AILocator() {};

	virtual void	PostConstruct();

	void PaulsDebugRender();

	// accessors
	CPoint	GetPos() const					{ return m_obPos; }
	void	SetPos( const CPoint& obPos )	{ m_obPos = obPos; }
	int		GetNum()						{ return m_iNodeNum; }
	void	SetNum( const int iNodeNum )	{ m_iNodeNum = iNodeNum; }

	virtual void SetAvailable( const bool bAvailable )	{ m_bAvailable = bAvailable; }
	virtual bool IsAvailable()							{ return m_bAvailable; }


	// public data for Welder
	CPoint	m_obPos;
	int		m_iNodeNum;

protected:
	bool			m_bAvailable;

private:
	bool			m_bEnabled;
	ntstd::String	m_strStatus;

};

//------------------------------------------------------------------------------------------
//!
//!	CoverPoint
//!
//!	A point on the level where the AI can take cover from the player
//!
//------------------------------------------------------------------------------------------
class CoverPoint : public AILocator
{
public:
	// This interface is exposed
	HAS_INTERFACE( CoverPoint );
	virtual ~CoverPoint() {};
	
	CoverPoint();

	void		SetFirePoint( FirePoint* firePoint );
	FirePoint*	GetFirePoint() const;

private:
	FirePoint* m_pFirePoint; 

};

//------------------------------------------------------------------------------------------
//!
//!	FirePoint
//!
//!	Paired with a cover point, a firepoint is the position an AI will move to when firing
//! on the player
//!
//------------------------------------------------------------------------------------------

class FirePoint : public AILocator
{
public:

	// This interface is exposed
	HAS_INTERFACE( FirePoint );
	virtual ~FirePoint() {};

	FirePoint();
};

//------------------------------------------------------------------------------------------
//!
//!	RallyPoint
//!
//!	When falling back to maintain distance from the player, AIs will use the rally point
//! as a final point of fallback when no suitable cover points remain.
//!
//------------------------------------------------------------------------------------------

class RallyPoint : public AILocator
{
public:

	// This interface is exposed
	HAS_INTERFACE( RallyPoint );
	virtual ~RallyPoint() {};

	RallyPoint();
};

//------------------------------------------------------------------------------------------
//!
//!	AvoidPoint
//!
//------------------------------------------------------------------------------------------

class AvoidPoint : public AILocator
{
public:

	// This interface is exposed
	HAS_INTERFACE( AvoidPoint );
	virtual ~AvoidPoint() {};

	AvoidPoint();
};

//------------------------------------------------------------------------------------------
//!
//!	AICoverManager
//!
//!	A class to hold all the cover points and the pairings between cover and firepoints
//!
//!	XXX: currently uses a fixed size array to store the cover points, but could really do
//! with being a vector (dynamic size, index access). Are we writing our own or using std:: ?
//!
//------------------------------------------------------------------------------------------

static const int	MAX_COVERPOINTS	=	128;

class AICoverManager : public Singleton<AICoverManager>
{
public:
	AICoverManager();
	~AICoverManager();

	void DebugRender();
	void LevelUnload();

	int				AddCoverPoint( CoverPoint* point );
	int				AddFirePoint( FirePoint* point );
	int				AddRallyPoint( RallyPoint* point );
	CoverPoint*		GetCoverPoint( int iPointNum )					{ return m_aCoverPoints[iPointNum]; }
	FirePoint*		GetFirePointForCover( CoverPoint* point );
	//FirePoint*	GetFirePointForCover( int iPointNum )			{ return m_aFirePoints[iPointNum]; }
	int				GetNumCoverPoints()								{ return m_iNumCoverPoints; }
	
	//int				GetLargestPathNum()				{ return m_iLargestPathNum; }
	CoverPoint*		GetNearestCoverPointWithinRange( const CPoint &obPos, float fRangeMin, float fRangeMax );
	RallyPoint*		GetNearestRallyPointWithinRange( const CPoint &obPos, float fRangeMin, float fRangeMax );

private:
	AILocator*		GetNearestPointWithinRange( AILocator** locators, const int numLocators, const CPoint &obPos, float fRangeMin, float fRangeMax );
	void Clear();
	void BuildLinks();
	
	CoverPoint*		m_aCoverPoints[MAX_COVERPOINTS];
	FirePoint*		m_aFirePoints[MAX_COVERPOINTS];
	RallyPoint*		m_aRallyPoints[MAX_COVERPOINTS];
	int				m_iNumCoverPoints;
	int				m_iNumFirePoints;
	int				m_iNumRallyPoints;
	bool			m_bLinksBuilt;
};





#endif // _AICOVERPOINT_H
