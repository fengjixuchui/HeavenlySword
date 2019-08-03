#include "superstylesafety.h"

#include "core/osddisplay.h" // Debugging
#include "objectdatabase/dataobject.h"

// Expose me:
//DGF - set with default
START_STD_INTERFACE							( SuperStyleSafetyVolumeCollection )
	PUBLISH_PTR_CONTAINER_AS				( m_obStartVolumes, StartVolumes )
	PUBLISH_PTR_CONTAINER_AS				( m_obContinueVolumes, ContinueVolumes )
END_STD_INTERFACE

START_STD_INTERFACE							( SuperStyleSafetySphereVolume )
	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obPosition, CPoint( CONSTRUCT_CLEAR ), Position )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fRadius, 1.0f, Radius )
	PUBLISH_PTR_AS							( m_pobContinueVolume, ContinueVolume )
END_STD_INTERFACE

START_STD_INTERFACE							( SuperStyleSafetyBoxVolume )
	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obMinPoint, CPoint( 0.0f, 0.0f, 0.0f ), MinPoint )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obMaxPoint, CPoint( 1.0f, 1.0f, 1.0f ), MaxPoint )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obOrientation, CQuat( 0.0f, 0.0f, 0.0f, 1.0f ), Orientation )
	PUBLISH_PTR_AS							( m_pobContinueVolume, ContinueVolume )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyManager
//!
//------------------------------------------------------------------------------------------
SuperStyleSafetyManager::SuperStyleSafetyManager()
{
	m_pobCurrentVolumeCollection = 0;
}

SuperStyleSafetyManager::~SuperStyleSafetyManager()
{
}

void SuperStyleSafetyManager::SetSuperStyleSafetyVolumeCollection(SuperStyleSafetyVolumeCollection* pobCollection)
{
	m_pobCurrentVolumeCollection = pobCollection;
}

SuperStyleSafetyVolume* SuperStyleSafetyManager::PointInSuperStyleContinueVolume(const CPoint& obPoint)
{
	if (m_pobCurrentVolumeCollection)
	{
		for (ntstd::List<SuperStyleSafetyVolume*>::iterator obIt = m_pobCurrentVolumeCollection->m_obContinueVolumes.begin();
			obIt != m_pobCurrentVolumeCollection->m_obContinueVolumes.end();
			obIt++)
		{
			if ((*obIt)->IsInside(obPoint))
                return (*obIt);
		}

		return 0;
	}
	else
	{
		//ntPrintf("SuperStyleSafety: Checking for a point in continue volumes, but there are no volumes!\n");
		return 0;
	}
}

SuperStyleSafetyVolume* SuperStyleSafetyManager::PointInSuperStyleStartVolume(const CPoint& obPoint)
{
	if (m_pobCurrentVolumeCollection)
	{
		for (ntstd::List<SuperStyleSafetyVolume*>::iterator obIt = m_pobCurrentVolumeCollection->m_obStartVolumes.begin();
			obIt != m_pobCurrentVolumeCollection->m_obStartVolumes.end();
			obIt++)
		{
			if ((*obIt)->IsInside(obPoint))
                return (*obIt);
		}

		return 0;
	}
	else
	{
		//ntPrintf("SuperStyleSafety: Checking for a point in start volumes, but there are no volumes!\n");
		return 0;
	}
}

CPoint SuperStyleSafetyManager::GetSuperStyleSafeContinuePoint(const CPoint& obPoint, SuperStyleSafetyVolume* pobSSV)
{
	if (m_pobCurrentVolumeCollection)
	{
		// Make sure we're not already in a continue volume
		if (PointInSuperStyleContinueVolume( obPoint ) )
		{
			return obPoint;
		}

		SuperStyleSafetyVolume* pobStartVolume = 0;
		if (pobSSV && pobSSV->GetContinueVolume()) // Only use the one we're given if it has a continue volume, otherwise search
			pobStartVolume = pobSSV;

		if (!pobStartVolume)
		{
			// Find the appropriate start volume
			for (ntstd::List<SuperStyleSafetyVolume*>::iterator obIt = m_pobCurrentVolumeCollection->m_obStartVolumes.begin();
				obIt != m_pobCurrentVolumeCollection->m_obStartVolumes.end();
				obIt++)
			{
				if ((*obIt)->IsInside(obPoint))
					pobStartVolume = (*obIt);
			}
		}

		// Must be in a start volume, should've checked before calling this function
		ntError( pobStartVolume );

		// Get the link to it's continue volume
		SuperStyleSafetyVolume* pobContinueVolume = pobStartVolume->GetContinueVolume();
		// To save time, I'm assuming mappers will always link a continue volume
		ntError( pobContinueVolume );
		
		// Chained SCVs
		ntstd::List<SuperStyleSafetyVolume*> obVisitedVolumes;
		obVisitedVolumes.push_back(pobContinueVolume);
		m_obLastHitPoint = pobContinueVolume->GetClosestPointInVolumeTo(obPoint);
		float fDistance = CDirection( m_obLastHitPoint - obPoint ).Length();
		pobContinueVolume = pobContinueVolume->GetContinueVolume();
		while (pobContinueVolume)
		{
			// Have I been through the chain of volumes?
			bool bDone = false;
			for (ntstd::List<SuperStyleSafetyVolume*>::iterator obIt = obVisitedVolumes.begin();
				obIt != obVisitedVolumes.end();
				obIt++)
			{
				if (pobContinueVolume == (*obIt))
					bDone = true;
			}

			// Yes I have, bail now
			if (bDone)
			{
				break;
			}
			else
			{
				// I haven't, make note of it and it's closest point (if it closer that our last one)
				obVisitedVolumes.push_back(pobContinueVolume);
				CPoint obNewPoint = pobContinueVolume->GetClosestPointInVolumeTo(obPoint);
				float fNewDistance = CDirection( obNewPoint - obPoint ).Length();
				if (fNewDistance < fDistance)
				{
					ntPrintf("New shortest distance: %f. %f, %f, %f. %s.\n", fNewDistance, obNewPoint.X(), obNewPoint.Y(), obNewPoint.Z(), ObjectDatabase::Get().GetNameFromPointer(pobContinueVolume).GetDebugString() );
					m_obLastHitPoint = obNewPoint;
				}

				pobContinueVolume = pobContinueVolume->GetContinueVolume();
			}
		}			

		// Point should be on same level as heroine position
		m_obLastHitPoint.Y() = obPoint.Y();

		return m_obLastHitPoint;
	}
	else
	{
		//ntPrintf("SuperStyleSafety: getting a continue point, but there are no volumes!\n");
		return obPoint;
	}
}

void SuperStyleSafetyManager::DebugRender()
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderPoint(m_obLastStartPoint,10.0f,DC_GREEN);
	g_VisualDebug->RenderPoint(m_obLastHitPoint,10.0f,DC_RED);

	if (m_pobCurrentVolumeCollection)
	{
		ntstd::List<SuperStyleSafetyVolume*>::iterator obIt;
		for (obIt = m_pobCurrentVolumeCollection->m_obStartVolumes.begin();
			obIt != m_pobCurrentVolumeCollection->m_obStartVolumes.end();
			obIt++)
		{
			(*obIt)->DebugRender(NTCOLOUR_ARGB(255,0,255,0),NTCOLOUR_ARGB(255,0,255,0));
		}

		for (obIt = m_pobCurrentVolumeCollection->m_obContinueVolumes.begin();
			obIt != m_pobCurrentVolumeCollection->m_obContinueVolumes.end();
			obIt++)
		{
			(*obIt)->DebugRender(NTCOLOUR_ARGB(255,255,255,0),NTCOLOUR_ARGB(255,255,255,0));
		}
	}
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyVolume
//!
//------------------------------------------------------------------------------------------
SuperStyleSafetyVolume::SuperStyleSafetyVolume()
{
	m_pobContinueVolume = 0;
	m_obMatrix = CMatrix( CONSTRUCT_IDENTITY );
}

SuperStyleSafetyVolume::~SuperStyleSafetyVolume ()
{
}

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyBoxVolume
//!
//------------------------------------------------------------------------------------------
SuperStyleSafetyBoxVolume::SuperStyleSafetyBoxVolume() 
:	SuperStyleSafetyVolume(),
	m_obMinPoint( CONSTRUCT_CLEAR ),
	m_obMaxPoint( CONSTRUCT_CLEAR ),
	m_obOrientation( CONSTRUCT_IDENTITY ),
	m_obHalfExtents( CONSTRUCT_CLEAR )
{
}

SuperStyleSafetyBoxVolume::~SuperStyleSafetyBoxVolume ()
{
}

void SuperStyleSafetyBoxVolume::PostConstruct()
{
	// Get world centre
	CPoint obCentre((m_obMinPoint + m_obMaxPoint) * 0.5f);
	
	m_obMatrix = CMatrix( m_obOrientation, obCentre );

	m_obHalfExtents = CDirection( m_obMinPoint - m_obMaxPoint );
	m_obHalfExtents *= 0.5f;
	m_obHalfExtents.X() = abs(m_obHalfExtents.X());
	m_obHalfExtents.Y() = abs(m_obHalfExtents.Y());
	m_obHalfExtents.Z() = abs(m_obHalfExtents.Z());
}

bool SuperStyleSafetyBoxVolume::IsInside(const CPoint& obPoint)
{
	CPoint obLocalPoint( obPoint * m_obMatrix.GetFullInverse() );
	
	if ( fabs(obLocalPoint.X()) < m_obHalfExtents.X() &&
		 fabs(obLocalPoint.Y()) < m_obHalfExtents.Y() &&
		 fabs(obLocalPoint.Z()) < m_obHalfExtents.Z() )
	{
		return true;
	}

	return false;
}

void SuperStyleSafetyBoxVolume::DebugRender(u_long ulFrameColour,u_long ulBodyColour)
{
#ifndef _GOLD_MASTER
	UNUSED( ulBodyColour );

	g_VisualDebug->RenderOBB(m_obMatrix,m_obHalfExtents,ulFrameColour,DPF_WIREFRAME);
#endif
}

CPoint SuperStyleSafetyBoxVolume::GetClosestPointInVolumeTo(const CPoint& obPoint)
{
	CPoint obLocalPoint( obPoint * m_obMatrix.GetFullInverse() );
		
	CPoint obLocalRet( CONSTRUCT_CLEAR );
	if (fabs(obLocalPoint.X()) < m_obHalfExtents.X())
	{
		obLocalRet.X() =  obLocalPoint.X();
	}
	else
	{
		obLocalPoint.X() < 0.0f ? obLocalRet.X() = -m_obHalfExtents.X() : obLocalRet.X() = m_obHalfExtents.X();
	}

	if (fabs(obLocalPoint.Y()) < m_obHalfExtents.Y())
	{
		obLocalRet.Y() = obLocalPoint.Y();
	}
	else
	{
		obLocalPoint.Y() < 0.0f ? obLocalRet.Y() = -m_obHalfExtents.Y() : obLocalRet.Y() = m_obHalfExtents.Y();
	}

	if (fabs(obLocalPoint.Z()) < m_obHalfExtents.Z())
	{
		obLocalRet.Z() = obLocalPoint.Z();
	}
	else
	{
		obLocalPoint.Z() < 0.0f ? obLocalRet.Z() = -m_obHalfExtents.Z() : obLocalRet.Z() = m_obHalfExtents.Z();
	}
	
	return obLocalRet * m_obMatrix;
}

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetySphereVolume
//!
//------------------------------------------------------------------------------------------
SuperStyleSafetySphereVolume::SuperStyleSafetySphereVolume() 
:	SuperStyleSafetyVolume(),
	m_obPosition( CONSTRUCT_CLEAR ),
	m_fRadius( 0.0f )
{
}

SuperStyleSafetySphereVolume::~SuperStyleSafetySphereVolume()
{
}

bool SuperStyleSafetySphereVolume::IsInside (const CPoint& obPoint)
{
	CDirection obVector( obPoint - m_obMatrix.GetTranslation());
	float fDistance = obVector.Length();
	
	if ( fDistance < m_fRadius )
		return true;
	else
		return false;
}

void SuperStyleSafetySphereVolume::PostConstruct ()
{
	m_obMatrix.SetIdentity();
	m_obMatrix.SetTranslation(m_obPosition);
}

void SuperStyleSafetySphereVolume::DebugRender(u_long ulFrameColour, u_long ulBodyColour)
{
#ifndef _GOLD_MASTER
	UNUSED( ulBodyColour );

	g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obMatrix.GetTranslation(),m_fRadius,ulFrameColour,DPF_WIREFRAME);
#endif
}

CPoint SuperStyleSafetySphereVolume::GetClosestPointInVolumeTo(const CPoint& obPoint)
{
	CDirection obVector( obPoint - m_obMatrix.GetTranslation());
	float fDistance = obVector.Length();
	
	// Something weird if the point is within the volume
	if ( fDistance < m_fRadius )
	{
		//ntPrintf("SuperStyleSafety: Getting closestpoint but point is in volume!\n");
		return obPoint;
	}

	obVector.Normalise();
	obVector *= m_fRadius * 0.99f; // Bring it slightly inside the volume

	return m_obMatrix.GetTranslation() + obVector;
}
