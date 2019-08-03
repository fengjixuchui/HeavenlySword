//------------------------------------------------------------------------------------------
//!
//!	aicoverpoint.cpp
//!
//------------------------------------------------------------------------------------------

#include "ai/aicoverpoint.h"
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"

#include "core/visualdebugger.h"


//------------------------------------------------------------------------------------------
//!
//!	AILocator::AILocator
//! Constructor
//!
//------------------------------------------------------------------------------------------

AILocator::AILocator()
{
	m_iNodeNum = -1;
}

//------------------------------------------------------------------------------------------
//!
//!	AILocator::PostConstruct
//! 
//!
//------------------------------------------------------------------------------------------

void
AILocator::PostConstruct()
{
}

//------------------------------------------------------------------------------------------
//!
//!	AILocator::PaulsDebugRender
//! 
//!
//------------------------------------------------------------------------------------------

void AILocator::PaulsDebugRender()
{
#ifndef _GOLD_MASTER
	CQuat	obOrientation( CONSTRUCT_IDENTITY );
	float	fRadius = 0.1f;

	g_VisualDebug->RenderSphere( obOrientation, m_obPos,	fRadius, 0xff0ffff0 );
	// if point out of graph, disable it, and set a debug message on it
	if (CAINavGraphManager::Get().InGraph( m_obPos ))
	{
		m_bEnabled = true;
	}
	else
	{
		m_bEnabled = false;
		m_strStatus = "OUT OF NAVGRAPH";
	}

	if (!m_bEnabled)
	{
		// print the status message, if there is one
		CPoint pt3D = m_obPos;
		pt3D.Y() += 0.5f;

		g_VisualDebug->Printf3D( pt3D, 0xffffffff, 0, m_strStatus.c_str() );
	}
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	CoverPoint::CoverPoint
//! Constructor
//!
//------------------------------------------------------------------------------------------
CoverPoint::CoverPoint() : m_pFirePoint(NULL)
{
	AICoverManager::Get().AddCoverPoint( this );
	m_bAvailable = true;
}

//------------------------------------------------------------------------------------------
//!
//!	CoverPoint::SetFirePoint
//! Constructor
//!
//------------------------------------------------------------------------------------------

void CoverPoint::SetFirePoint( FirePoint* firePoint )
{
	m_pFirePoint = firePoint;
}

//------------------------------------------------------------------------------------------
//!
//!	CoverPoint::SetFirePoint
//! Constructor
//!
//------------------------------------------------------------------------------------------

FirePoint* CoverPoint::GetFirePoint() const
{
	return m_pFirePoint;
}

//------------------------------------------------------------------------------------------
//!
//!	FirePoint::FirePoint
//! Constructor
//!
//------------------------------------------------------------------------------------------

FirePoint::FirePoint()
{
	AICoverManager::Get().AddFirePoint( this );
}

//------------------------------------------------------------------------------------------
//!
//!	RallyPoint::RallyPoint
//! Constructor
//!
//------------------------------------------------------------------------------------------

RallyPoint::RallyPoint()
{
	AICoverManager::Get().AddRallyPoint( this );
	m_bAvailable = true;
}

//------------------------------------------------------------------------------------------
//!
//!	AvoidPoint::AvoidPoint
//! Constructor
//!
//------------------------------------------------------------------------------------------

AvoidPoint::AvoidPoint()
{
	// add to repulsion manager
	CAINavGraphManager::Get().GetRepulsion()->AddAvoidPoint( this );
}

//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::AICoverManager
//!
//------------------------------------------------------------------------------------------

AICoverManager::AICoverManager()
{
	Clear();
}

//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::~AICoverManager
//!
//------------------------------------------------------------------------------------------

AICoverManager::~AICoverManager()
{
}

//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::Clear
//!
//------------------------------------------------------------------------------------------

void AICoverManager::Clear()
{
	memset( m_aCoverPoints, 0, sizeof( m_aCoverPoints ) );
	memset( m_aFirePoints, 0, sizeof( m_aFirePoints ) );
	memset( m_aRallyPoints, 0, sizeof( m_aRallyPoints ) );
	m_iNumCoverPoints = 0;
	m_iNumFirePoints = 0;
	m_iNumRallyPoints = 0;
	m_bLinksBuilt = false;
}

//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::LevelUnload
//!
//------------------------------------------------------------------------------------------

void AICoverManager::LevelUnload()
{
	Clear();
}

//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::AddPoint
//!
//------------------------------------------------------------------------------------------

int AICoverManager::AddCoverPoint( CoverPoint* point )
{
	ntAssert( m_aCoverPoints[m_iNumCoverPoints] == NULL );
	ntAssert( m_iNumCoverPoints < (MAX_COVERPOINTS-1) );

	m_aCoverPoints[m_iNumCoverPoints] = point;
	point->SetNum( m_iNumCoverPoints );
	return m_iNumCoverPoints++;
}


//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::AddPoint
//!
//------------------------------------------------------------------------------------------

int AICoverManager::AddFirePoint( FirePoint* point )
{
	ntAssert( m_aFirePoints[m_iNumFirePoints] == NULL );
	ntAssert( m_iNumFirePoints < (MAX_COVERPOINTS-1) );

	m_aFirePoints[m_iNumFirePoints] = point;
	point->SetNum( m_iNumFirePoints );
	return m_iNumFirePoints++;
}


//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::AddPoint
//!
//------------------------------------------------------------------------------------------

int AICoverManager::AddRallyPoint( RallyPoint* point )
{
	ntAssert( m_aRallyPoints[m_iNumRallyPoints] == NULL );
	ntAssert( m_iNumRallyPoints < (MAX_COVERPOINTS-1) );

	m_aRallyPoints[m_iNumRallyPoints] = point;
	point->SetNum( m_iNumRallyPoints );
	return m_iNumRallyPoints++;
}


//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::DebugRender
//!
//------------------------------------------------------------------------------------------

void AICoverManager::DebugRender()
{
	return;

	for (int i = 0; i < m_iNumCoverPoints; ++i)
	{
		m_aCoverPoints[i]->PaulsDebugRender();
	}
	for (int i = 0; i < m_iNumFirePoints; ++i)
	{
		m_aFirePoints[i]->PaulsDebugRender();
	}
	for (int i = 0; i < m_iNumRallyPoints; ++i)
	{
		m_aRallyPoints[i]->PaulsDebugRender();
	}
}


//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::GetNearestPointWithinRange
//!
//------------------------------------------------------------------------------------------

AILocator* AICoverManager::GetNearestPointWithinRange( AILocator** locators, const int numLocators, const CPoint &obPos, float fRangeMin, float fRangeMax )
{
	// build connections before getting points 
	BuildLinks();

	float fRangeMinSquared = fRangeMin * fRangeMin;
	float fRangeMaxSquared = fRangeMax * fRangeMax;

	AILocator* pobBestPoint = NULL;
	float fBestRangeSquared = FLT_MAX;

	for (int i = 0; i < numLocators; ++i)
	{
		ntAssert(locators[i]);
		if (locators[i]->IsAvailable())
		{
			CPoint obPointPos = locators[i]->GetPos();
			float fLengthSquared = (obPos - obPointPos).LengthSquared();
			if ( fLengthSquared < fBestRangeSquared && fLengthSquared < fRangeMaxSquared && fLengthSquared > fRangeMinSquared)
			{
				fBestRangeSquared = fLengthSquared;
				pobBestPoint = locators[i];
			}
		}
	}

	return pobBestPoint;
}

//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::GetNearestCoverPointWithinRange
//!
//------------------------------------------------------------------------------------------

CoverPoint* AICoverManager::GetNearestCoverPointWithinRange( const CPoint &obPos, float fRangeMin, float fRangeMax )
{
	// stupid C++ can't cope with treating an array of pointers to a derived type as an array of pointers to the base type, so we're in reinterpret_cast hell
	// if anyone can think of a better way of doing this, let me know. PMN
	return reinterpret_cast<CoverPoint*>( GetNearestPointWithinRange( reinterpret_cast<AILocator**>( &(m_aCoverPoints[0]) ), m_iNumCoverPoints, obPos, fRangeMin, fRangeMax ) );
}

//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::GetNearestCoverPointWithinRange
//!
//------------------------------------------------------------------------------------------

RallyPoint* AICoverManager::GetNearestRallyPointWithinRange( const CPoint &obPos, float fRangeMin, float fRangeMax )
{
	// see comment in function above
	return reinterpret_cast<RallyPoint*>( GetNearestPointWithinRange( reinterpret_cast<AILocator**>( &(m_aRallyPoints[0]) ), m_iNumRallyPoints, obPos, fRangeMin, fRangeMax ) );
}

//------------------------------------------------------------------------------------------
//!
//!	AICoverManager::GetNearestNodeWithinRange
//!
//------------------------------------------------------------------------------------------

void AICoverManager::BuildLinks()
{
	if (!m_bLinksBuilt)
	{
		static float fRangeMin = 0.0f;
		static float fRangeMax = 10.0f;

		float fRangeMinSquared = fRangeMin * fRangeMin;
		float fRangeMaxSquared = fRangeMax * fRangeMax;

		for (int i = 0; i < m_iNumCoverPoints; ++i)
		{
			FirePoint* pobBestPoint = NULL;
			float fBestRangeSquared = FLT_MAX;

			ntAssert(m_aCoverPoints[i]);
			m_aCoverPoints[i]->GetPos();

			for (int j = 0; j < m_iNumFirePoints; ++j)
			{
				float fLengthSquared = (m_aFirePoints[j]->GetPos() - m_aCoverPoints[i]->GetPos()).LengthSquared();
				if (fLengthSquared < fBestRangeSquared && fLengthSquared < fRangeMaxSquared && fLengthSquared > fRangeMinSquared)
				{
					fBestRangeSquared = fLengthSquared;
					pobBestPoint = m_aFirePoints[j];
				}
			}
			m_aCoverPoints[i]->SetFirePoint( pobBestPoint );
		}
       
		m_bLinksBuilt = true;
	}
}
