//------------------------------------------------------------------------------------------
//!
//!	aipatrolmanager.cpp
//!
//------------------------------------------------------------------------------------------

#include "ai/aipatrolmanager.h"

#include "aipatrolpath.h"
#include "aipatrolnode.h"

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolManager::AIPatrolManager
//!
//------------------------------------------------------------------------------------------

AIPatrolManager::AIPatrolManager() : m_iNumPaths( 0 ), m_iLargestPathNum(0)
{
	memset( m_aPaths, 0, sizeof( m_aPaths ) );
}

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolManager::~AIPatrolManager
//!
//------------------------------------------------------------------------------------------

AIPatrolManager::~AIPatrolManager()
{
	for (int i = 0; i < MAX_PATHS; ++i)
	{
		NT_DELETE_CHUNK( Mem::MC_AI, m_aPaths[i] );
		m_aPaths[i] = NULL;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolManager::DebugRender
//!
//------------------------------------------------------------------------------------------

void AIPatrolManager::DebugRender()
{
	for (int i = 0; i < MAX_PATHS; ++i)
	{
		if (m_aPaths[i])
		{
			m_aPaths[i]->DebugRender();
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolManager::CreatePath
//!
//------------------------------------------------------------------------------------------

AIPatrolPath* AIPatrolManager::CreatePath( int iPathNum )
{
	ntAssert( m_aPaths[iPathNum] == NULL );
	m_aPaths[iPathNum] = NT_NEW_CHUNK( Mem::MC_AI ) AIPatrolPath( iPathNum );
	++m_iNumPaths;
	ntPrintf( "%d paths\n", m_iNumPaths );
	if (iPathNum > m_iLargestPathNum)
	{
		m_iLargestPathNum = iPathNum;
	}
	return m_aPaths[iPathNum];
}

//------------------------------------------------------------------------------------------
//!
//!	AIPatrolManager::GetNearestNodeWithinRange
//!
//------------------------------------------------------------------------------------------

PatrolNode* AIPatrolManager::GetNearestNodeWithinRange( const CPoint &obPos, float fRange )
{
	float fRangeSquared = fRange * fRange;
	PatrolNode* pobBestNode = NULL;
	float fBestRangeSquared = FLT_MAX;

	for (int i = 0; i < MAX_PATHS; ++i)
	{
		if (m_aPaths[i])
		{
			int iNumNodes = m_aPaths[i]->GetNumNodes();
			for (int j = 0; j < iNumNodes; ++j)
			{
				CPoint obNodePos = m_aPaths[i]->GetNode( j )->GetPos();
				float fLengthSquared = (obPos - obNodePos).LengthSquared();
				if (fLengthSquared < fBestRangeSquared && fLengthSquared < fRangeSquared)
				{
					fBestRangeSquared = fLengthSquared;
					pobBestNode = m_aPaths[i]->GetNode( j );
				}
			}
		}
	}

	return pobBestNode;
}

//------------------------------------------------------------------------------------------
//!
//!	AIAlertManager::AIAlertManager
//!
//------------------------------------------------------------------------------------------

AIAlertManager::AIAlertManager() :
	m_iNumAlerts(0),
	m_fAlertRange(15.0f),
	m_fAlertLifespan(1.0f)
{
	for (int i = 0; i < MAX_ALERTS; ++i)
	{
		m_aRaisedAlerts[i].bActive = false;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	AIAlertManager::SendAlert
//!
//------------------------------------------------------------------------------------------

void AIAlertManager::SendAlert( const CPoint& obAIPos, const CPoint& obPlayerPos )
{
	// iterate over alerts
	for (int i = 0; i < MAX_ALERTS; ++i)
	{
		// check range of any active alerts
		float fLengthSquared = (obAIPos - m_aRaisedAlerts[i].obAIPos).LengthSquared();
		if (fLengthSquared < (m_fAlertRange*m_fAlertRange))
		{
			// if there's one in range, then early out
			return;
		}
	}

	for (int i = 0; i < MAX_ALERTS; ++i)
	{
		if (!m_aRaisedAlerts[i].bActive)
		{
			m_aRaisedAlerts[i].bActive = true;
			m_aRaisedAlerts[i].fAge = 0.0f;
			m_aRaisedAlerts[i].obAIPos = obAIPos;
			m_aRaisedAlerts[i].obPlayerPos = obPlayerPos;
			++m_iNumAlerts;
			break;
		}
	}    
}

//------------------------------------------------------------------------------------------
//!
//!	AIAlertManager::ActiveAlertInRange
//!
//------------------------------------------------------------------------------------------
bool AIAlertManager::ActiveAlertInRange( const CPoint &obPos, CPoint &obPlayerPos )
{
	// iterate over alerts
	for (int i = 0; i < MAX_ALERTS; ++i)
	{
		if (m_aRaisedAlerts[i].bActive)
		{
			// check range of any active alerts
			float fLengthSquared = (obPos - m_aRaisedAlerts[i].obAIPos).LengthSquared();
			if (fLengthSquared < (m_fAlertRange*m_fAlertRange))
			{
				// if one in range then true
				obPlayerPos = m_aRaisedAlerts[i].obPlayerPos;
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	AIAlertManager::Update
//!
//------------------------------------------------------------------------------------------

void AIAlertManager::Update( float fTimeChange )
{
	for (int i = 0; i < MAX_ALERTS; ++i)
	{
		if (m_aRaisedAlerts[i].bActive)
		{
			m_aRaisedAlerts[i].fAge += fTimeChange;
			if (m_aRaisedAlerts[i].fAge > m_fAlertLifespan)
			{
				m_aRaisedAlerts[i].bActive = false;
				--m_iNumAlerts;
			}
		}
	}    
}

//------------------------------------------------------------------------------------------
//!
//!	AIAlertManager::LevelUnload
//!
//------------------------------------------------------------------------------------------

void AIAlertManager::LevelUnload()
{
	for (int i = 0; i < MAX_ALERTS; ++i)
	{
		m_aRaisedAlerts[i].bActive = false;
	}    
	m_iNumAlerts = 0;
}


