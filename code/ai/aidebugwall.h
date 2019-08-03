#ifndef _AI_DEBUGWALL_H
#define _AI_DEBUGWALL_H

class CAINavEdge;
class CAINavPath;
class CAINavFollowPoint;

/***************************************************************************************************
*
*	CLASS			CAIDebugWall
*
*	DESCRIPTION		
*
	NOTES			
*
***************************************************************************************************/
class AIDebugWall
{
public:
	AIDebugWall();
	AIDebugWall( const CPoint &worldMin, const CPoint &worldMax );
	AIDebugWall( const bool );
	virtual ~AIDebugWall();

	// member function declarations
	void			CommonConstructor( void );
	virtual void	PostConstruct( void );

	void			PaulsDebugRender( void ) const;

	void			MakeMinMax( float[3], float[3] ) const;

	void			SetMax( const CPoint &worldMax )		{ m_obWorldMax = worldMax; }
	void			SetMinMax( float afMin[3], float afMax[3] )
	{
		m_obWorldMin.X() = afMin[0]; m_obWorldMin.Y() = afMin[1]; m_obWorldMin.Z() = afMin[2]; 
		m_obWorldMax.X() = afMax[0]; m_obWorldMax.Y() = afMax[1]; m_obWorldMax.Z() = afMax[2]; 
	}

	void			GetCentre( CPoint &centre ) const		{ centre = 0.5f * ( m_obWorldMax + m_obWorldMin ); }

	void			GetExtents( CPoint &extents ) const		{ extents = 0.5f * ( m_obWorldMax - m_obWorldMin ); }

	bool		Contains( CPoint const &pos ) const
	{
		CPoint	thisExtents;
		this->GetExtents( thisExtents );

		CPoint	thisCentre;
		CPoint	otherCentre( pos );
		this->GetCentre( thisCentre );

		const CPoint diff = (otherCentre - thisCentre).Abs();

		return 
			diff.X() <= fabs(thisExtents.X())
			&&
			diff.Y() <= fabs(thisExtents.Y())
			&&
			diff.Z() <= fabs(thisExtents.Z());
	}

	// data members made public for Welder, they should still only be used
	// through their accessors
	CPoint	m_obWorldMin;
	CPoint	m_obWorldMax;

private:
	
	int	m_iAvoidID;

};
typedef ntstd::List<AIDebugWall*, Mem::MC_AI> AIDebugWallList;



#endif // _AI_DEBUGWALL_H
