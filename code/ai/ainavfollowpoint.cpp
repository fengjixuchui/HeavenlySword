#include "ainavgraphmanager.h"
#include "ainavedge.h"
#include "ainavpath.h"

#include "ainavfollowpoint.h"
#include "core/visualdebugger.h"


/***************************************************************************************************
***************************************************************************************************/
int CAINavFollowPoint::m_iNumFollowers = 0;
/***************************************************************************************************
***************************************************************************************************/

bool
CAINavFollowPoint::AtDestination()
{
	// are we in the target node?
	if (m_pobCurrentPath)
	{
		if (CAINavGraphManager::Get().NodeContains( m_pobCurrentPath->GetDestNode(), m_obPos ))
		{
			// is our distance to the target pos < currently set threshold?
			if ((m_obPos - (m_obDest)).LengthSquared() < 0.1f)
			{
				return true;
			}
		}
	}
	return false;
}

void
CAINavFollowPoint::Update( float fTimeChange )
{

	if (m_pobCurrentPath)
	{
		// grab a waypoint off the list if we haven't got one already
		if (!m_pobCurrentEdge)
		{
			m_pobCurrentEdge = m_pobCurrentPath->GetNextEdge();
			m_bCachedPosValid = false;
			if (m_pobCurrentEdge)
			{
				//ntPrintf( "current edge: %X\n", m_pobCurrentEdge );
			}
		}

		// if we have a current edge, head towards its waypoint position, if not then we've been through
		// all the path waypoints and should head for the destination
		if (!m_bCachedPosValid)
		{
			m_obWaypointPos = (m_pobCurrentEdge != -1) ? CAINavGraphManager::Get().GetConnectionPos( m_pobCurrentEdge ) : m_obDest;
			m_bCachedPosValid = true;
		}

		// waypoint - pos, normalise, then multiply by current speed
		float speed = 7.0f;
		float fAheadDistSq = m_fAheadDist * m_fAheadDist; 

		if (m_fDistSquared > fAheadDistSq)
		{
			speed = 0.01f;
		}
		else if (m_fDistSquared < (0.7 * fAheadDistSq))
		{
			speed *= 1.9f;
		}

		speed *= fTimeChange;

		//ntPrintf( "%f\n", speed ); 

		CPoint dir = m_obWaypointPos - m_obPos;
		float length = dir.Length() + 0.01f;

		if (length < speed)
		{
			m_pobCurrentEdge = 0;
			//ntPrintf("going to next edge\n");
		}
		else
		{
			//ntPrintf("speed: %.3f\n", speed);
			dir /= length;
			dir *= speed;
			m_obPos += dir;
		}
	}
}

void
CAINavFollowPoint::DebugRender()
{
#ifndef _GOLD_MASTER
	if (CAINavGraphManager::Get().IsDebugRenderOn())
	{
		CQuat	obOrientation( CONSTRUCT_IDENTITY );
		static const float fRadius = 0.1f;
		
		// setting fYOffset = 1.0f causes a rendering bug (HS-1293)
		//static const float fYOffset = 1.0f;
		static const float fYOffset = 0.8489465153f;
		CPoint renderPos( m_obPos.X(), m_obPos.Y() + fYOffset, m_obPos.Z() );
		g_VisualDebug->RenderSphere( obOrientation, renderPos, fRadius, 0xff0fff0f );

		if (m_pobCurrentPath)
		{
			m_pobCurrentPath->DebugRender();
		}
	}
#endif
}












