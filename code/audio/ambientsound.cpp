//------------------------------------------------------------------------------------------
//!
//!	\file audio/ambientsound.cpp
//!
//------------------------------------------------------------------------------------------

#include "audio/ambientsound.h"
#include "audio/audiosystem.h"

#include "objectdatabase/dataobject.h"

#include "core/timer.h"
#include "core/visualdebugger.h" // Debug rendering

#include "game/randmanager.h"

#include "camera/camutils.h"




#ifndef _RELEASE

#define _AMBIENTSOUND_DEBUG

#endif // _RELEASE




START_STD_INTERFACE							( AmbientSoundDefinition )
	PUBLISH_VAR_AS							( m_obSoundBank,			SoundBank )
	PUBLISH_VAR_AS							( m_obSoundCue,				SoundCue )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_bEnabled,				true,							Enabled )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_bDebugRender,			false,							DebugRender )
	
	PUBLISH_GLOBAL_ENUM_WITH_DEFAULT_AS		( m_eType,					Type,							AMBIENT_SOUND_TYPE,	 HRTF_INTERMITTENT )
	PUBLISH_GLOBAL_ENUM_WITH_DEFAULT_AS		( m_eShape,					Shape,							AMBIENT_SOUND_SHAPE, BOX_AREA )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obTriggerPosition,		CPoint(0, 0, 0),				Position )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obTriggerRotation,		CDirection(0, 0, 0),			TriggerRotation )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obTriggerHalfExtents1,	CDirection(30, 30, 30),			TriggerHalfExtents1 )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obTriggerHalfExtents2,	CDirection(40, 40, 40),			TriggerHalfExtents2 )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fTriggerRadius,			1,								TriggerRadius )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fMinDistance,			0,								MinDistance )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fMaxDistance,			1,								MaxDistance )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obRandomOffset,			CPoint(0, 0, 0),				RandomOffset )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fMinInterval,			2,								MinInterval )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fMaxInterval,			4,								MaxInterval )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_iPlayCount,				0,								PlayCount )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fFadeInDuration,		1,								FadeInDuration )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fFadeOutDuration,		1,								FadeOutDuration )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_bRespawn,				false,							Respawn )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_iRespawnMinInstances,	0,								RespawnMinInstances )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_iRespawnMaxInstances,	0,								RespawnMaxInstances )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fRespawnMinInterval,	0,								RespawnMinInterval )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fRespawnMaxInterval,	0,								RespawnMaxInterval )

	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE


START_STD_INTERFACE							( SlaveAmbientSoundDefinition )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obPosition,				CPoint(CONSTRUCT_CLEAR),		Position )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obRandomOffset,			CDirection(CONSTRUCT_CLEAR),	RandomOffset )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fMinStartOffset,		0.0f,							MinStartOffset )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fMaxStartOffset,		1.0f,							MaxStartOffset )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fProbability,			0.5f,							Probability )

	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE


START_STD_INTERFACE							( MasterAmbientSoundDefinition )
	PUBLISH_PTR_CONTAINER_AS				( m_obSlaveList,			SlaveList )

	PUBLISH_VAR_AS							( m_obSound,				Sound )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_bEnabled,				true,							Enabled )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_bDebugRender,			false,							DebugRender )
	PUBLISH_GLOBAL_ENUM_WITH_DEFAULT_AS		( m_eShape,					Shape,							AMBIENT_SOUND_SHAPE, SPHERE )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obTriggerPosition,		CPoint(CONSTRUCT_CLEAR),		Position )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obTriggerRotation,		CDirection(CONSTRUCT_CLEAR),	TriggerRotation )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obTriggerHalfExtents,	CDirection(10.0f,10.0f,10.0f),	TriggerHalfExtents )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fTriggerRadius,			1.0f,							TriggerRadius )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_obRandomOffset,			CPoint(CONSTRUCT_CLEAR),		RandomOffset )

	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fMinInterval,			0.0f,							MinInterval )
	PUBLISH_VAR_WITH_DEFAULT_AS				( m_fMaxInterval,			10.0f,							MaxInterval )

	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE



// -------------------------- Math functions for calculate attenuation for ambient sounds --------------------------

bool LinePlaneIntersect (const CPoint& obP1,const CPoint& obP2,const CDirection& obNormal,const CPoint& obPointOnPlane,CPoint& obIntersect)
{
	CDirection obD1(obPointOnPlane - obP1);
	CDirection obD2(obP2 - obP1);

	float fKNumerator=obNormal.Dot(obD1);
	float fKDenominator=obNormal.Dot(obD2);

	if (fKDenominator < EPSILON) // The line must be pretty much parallel to the plane
	{
		obIntersect.Clear();
		return false;
	}

	float fK=fKNumerator / fKDenominator;

	if (fK<0.0f || fK>1.0f) // Line does not intersect the plane
	{
		obIntersect.Clear();
		return false;
	}

	CDirection obDiff(obP2 - obP1);

	obDiff *= fK;

	obIntersect = obP1;

	obIntersect += obDiff;

	return true;
}

bool LineAABBIntersect (const CPoint& obTestPoint,const CDirection& obHalfExtents,CPoint& obIntersect)
{
	{ // Test X face
		CDirection obNormal(1.0f,0.0f,0.0f);

		CPoint obPointOnPlane(obHalfExtents.X()*obNormal.X(),obHalfExtents.Y()*obNormal.Y(),obHalfExtents.Z()*obNormal.Z());
		
		if (LinePlaneIntersect(CVecMath::GetZeroPoint(),obTestPoint,obNormal,obPointOnPlane,obIntersect) &&
			fabsf(obIntersect.Y()) <= obHalfExtents.Y() &&
			fabsf(obIntersect.Z()) <= obHalfExtents.Z())
		{
			return true;
		}
	}

	{ // Test -X face
		CDirection obNormal(-1.0f,0.0f,0.0f);

		CPoint obPointOnPlane(obHalfExtents.X()*obNormal.X(),obHalfExtents.Y()*obNormal.Y(),obHalfExtents.Z()*obNormal.Z());

		if (LinePlaneIntersect(CVecMath::GetZeroPoint(),obTestPoint,obNormal,obPointOnPlane,obIntersect) &&
			fabsf(obIntersect.Y()) <= obHalfExtents.Y() &&
			fabsf(obIntersect.Z()) <= obHalfExtents.Z())
		{
			return true;
		}
	}

	{ // Test Y face
		CDirection obNormal(0.0f,1.0f,0.0f);

		CPoint obPointOnPlane(obHalfExtents.X()*obNormal.X(),obHalfExtents.Y()*obNormal.Y(),obHalfExtents.Z()*obNormal.Z());

		if (LinePlaneIntersect(CVecMath::GetZeroPoint(),obTestPoint,obNormal,obPointOnPlane,obIntersect) &&
			fabsf(obIntersect.X()) <= obHalfExtents.X() &&
			fabsf(obIntersect.Z()) <= obHalfExtents.Z())
		{
			return true;
		}
	}

	{ // Test -Y face
		CDirection obNormal(0.0f,-1.0f,0.0f);

		CPoint obPointOnPlane(obHalfExtents.X()*obNormal.X(),obHalfExtents.Y()*obNormal.Y(),obHalfExtents.Z()*obNormal.Z());

		if (LinePlaneIntersect(CVecMath::GetZeroPoint(),obTestPoint,obNormal,obPointOnPlane,obIntersect) &&
			fabsf(obIntersect.X()) <= obHalfExtents.X() &&
			fabsf(obIntersect.Z()) <= obHalfExtents.Z())
		{
			return true;
		}
	}

	{ // Test Z face
		CDirection obNormal(0.0f,0.0f,1.0f);

		CPoint obPointOnPlane(obHalfExtents.X()*obNormal.X(),obHalfExtents.Y()*obNormal.Y(),obHalfExtents.Z()*obNormal.Z());

		if (LinePlaneIntersect(CVecMath::GetZeroPoint(),obTestPoint,obNormal,obPointOnPlane,obIntersect) &&
			fabsf(obIntersect.X()) <= obHalfExtents.X() &&
			fabsf(obIntersect.Y()) <= obHalfExtents.Y())
		{
			return true;
		}
	}

	{ // Test -Z face
		CDirection obNormal(0.0f,0.0f,-1.0f);

		CPoint obPointOnPlane(obHalfExtents.X()*obNormal.X(),obHalfExtents.Y()*obNormal.Y(),obHalfExtents.Z()*obNormal.Z());

		if (LinePlaneIntersect(CVecMath::GetZeroPoint(),obTestPoint,obNormal,obPointOnPlane,obIntersect) &&
			fabsf(obIntersect.X()) <= obHalfExtents.X() &&
			fabsf(obIntersect.Y()) <= obHalfExtents.Y())
		{
			return true;
		}
	}

	return false;
}

float CalculateAttenuationFromSphere (const CPoint& obListenerPosition,const CPoint& obPosition,float fMinDistance,float fMaxDistance)
{
	CDirection obDiff(obListenerPosition-obPosition);

	float fDistSqrd=obDiff.LengthSquared();

	if (fDistSqrd > (fMaxDistance * fMaxDistance)) // We are outside the max distance range
	{
		return 0.0f;
	}

	if (fDistSqrd <= (fMinDistance * fMinDistance)) // We are inside the min distance range
	{
		return 1.0f;
	}

	// We are between the min and max distance
	float fDist=fsqrtf(fDistSqrd);
	
	return (fMaxDistance-fDist)/(fMaxDistance-fMinDistance);
}

float CalculateAttenuationFromOBB (const CPoint& obListenerPosition,const CMatrix& obWorldMatrix,const CDirection& obInnerHalfExtents,const CDirection& obOuterHalfExtents)
{
	//ntPrintf("CalculateAttenuationFromOBB\n");
	//ntPrintf("ListenerPosition=%f,%f,%f\n",obListenerPosition.X(),obListenerPosition.Y(),obListenerPosition.Z());
	//ntPrintf("OBB Pos=%f,%f,%f\n",obWorldMatrix.GetTranslation().X(),obWorldMatrix.GetTranslation().Y(),obWorldMatrix.GetTranslation().Z());
	//ntPrintf("Inner half extents=%f,%f,%f\n",obInnerHalfExtents.X(),obInnerHalfExtents.Y(),obInnerHalfExtents.Z());
	//ntPrintf("Outer half extents=%f,%f,%f\n",obOuterHalfExtents.X(),obOuterHalfExtents.Y(),obOuterHalfExtents.Z());


 	//CDirection obRelativePosition(obWorldMatrix.GetTranslation() - obListenerPosition);
	CDirection obRelativePosition(obListenerPosition - obWorldMatrix.GetTranslation());
	float fDistSqrd=obRelativePosition.LengthSquared();
	float fOBBRadius=obOuterHalfExtents.LengthSquared();

	if (fDistSqrd>fOBBRadius) // Do a sphere check first to make sure we are near the OBB
	{
		return 0.0f;
	}

	// Transform the listener position so its relative to the OBB
	CMatrix obInverseWorldMatrix(obWorldMatrix.GetAffineInverse());

	CDirection obTransformedPosition=obRelativePosition * obInverseWorldMatrix;

	//ntPrintf("Transformed listener position=%f,%f,%f\n",obTransformedPosition.X(),obTransformedPosition.Y(),obTransformedPosition.Z());

	// Check to see if we are outside the outer box
    if (fabsf(obTransformedPosition.X())>obOuterHalfExtents.X() ||
		fabsf(obTransformedPosition.Y())>obOuterHalfExtents.Y() ||
		fabsf(obTransformedPosition.Z())>obOuterHalfExtents.Z())
	{
		return 0.0f;
	}

	// Check to see if we are inside the inner box
	if (fabsf(obTransformedPosition.X())<obInnerHalfExtents.X() &&
		fabsf(obTransformedPosition.Y())<obInnerHalfExtents.Y() &&
		fabsf(obTransformedPosition.Z())<obInnerHalfExtents.Z())
	{
		return 1.0f;
	}

	// We are inside the outer area but outside the inner area, so calculate attenuation
	
	/*
	CPoint obIntersect1;
	
	CPoint obTestPoint1(obTransformedPosition);
	CPoint obTestPoint2(obTransformedPosition);
	obTestPoint2 *= 2.0f;

	CPoint obInnerIntersect;
	CPoint obOutputIntersect;
	*/

	// Find the intersect on the inner box

	CPoint obIntersect1;

	CPoint obInnerTestPoint(obTransformedPosition);

	if (LineAABBIntersect(obInnerTestPoint,obInnerHalfExtents,obIntersect1))
	{
		//ntPrintf("Listener to innerbox intersection at %f,%f,%f\n",obIntersect1.X(),obIntersect1.Y(),obIntersect1.Z());
	}
	else
	{
		//ntPrintf("No intersection\n");
	}

	CPoint obOuterTestPoint(obTransformedPosition * 2.0f);

	CPoint obIntersect2;

	if (LineAABBIntersect(obOuterTestPoint,obOuterHalfExtents,obIntersect2))
	{
		//ntPrintf("Listener to outerbox intersection at %f,%f,%f\n",obIntersect2.X(),obIntersect2.Y(),obIntersect2.Z());
	}
	else
	{
		//ntPrintf("No intersection\n");
	}

	float fI1=obIntersect1.Length();
	float fI2=obIntersect2.Length();
	float fP=obTransformedPosition.Length();

	float fVolume =  (fP-fI1) / (fI2-fI1);

	if (fVolume < 0.0f)
		fVolume=0.0f;
	else if (fVolume > 1.0f)
		fVolume=1.0f;
	
	fVolume=1.0f-fVolume;
	
	//ntPrintf("Output Volume=%f\n",fVolume);

	return fVolume;
}

float CalculateAttenuationFromOBBArea (const CPoint& obListener,const CMatrix& obOBB,const CDirection& obOBBExtents,float fMinDistance,float fMaxDistance)
{
	// First calculate a bounding sphere for the area
	float fSphereRadiusSquared=obOBB.GetXAxis().LengthSquared() + obOBB.GetYAxis().LengthSquared() + obOBB.GetZAxis().LengthSquared() + (fMaxDistance*fMaxDistance);

	CDirection obDiff(obListener - obOBB.GetTranslation());

	if (obDiff.LengthSquared() > fSphereRadiusSquared) // The listener is outside the bounding sphere radius
		return 0.0f;

	// Calculate the distance between the listener and the closest point on the box
	float fSqrDist = 0.0f;
	float fDelta;
	CDirection obClosest;

	obClosest.X()=obDiff.Dot(obOBB.GetXAxis());
	if (obClosest.X() < -obOBBExtents.X())
	{
		fDelta = obClosest.X() + obOBBExtents.X();
		fSqrDist += fDelta * fDelta;
		obClosest.X() = -obOBBExtents.X();
	}
	else if (obClosest.X() > obOBBExtents.X())
	{
		fDelta = obClosest.X() - obOBBExtents.X();
		fSqrDist += fDelta * fDelta;
		obClosest.X() = obOBBExtents.X();
	}

	obClosest.Y()=obDiff.Dot(obOBB.GetYAxis());
	if (obClosest.Y() < -obOBBExtents.Y())
	{
		fDelta = obClosest.Y() + obOBBExtents.Y();
		fSqrDist += fDelta * fDelta;
		obClosest.Y() = -obOBBExtents.Y();
	}
	else if (obClosest.Y() > obOBBExtents.Y())
	{
		fDelta = obClosest.Y() - obOBBExtents.Y();
		fSqrDist += fDelta * fDelta;
		obClosest.Y() = obOBBExtents.Y();
	}

	obClosest.Z()=obDiff.Dot(obOBB.GetZAxis());
	if (obClosest.Z() < -obOBBExtents.Z())
	{
		fDelta = obClosest.Z() + obOBBExtents.Z();
		fSqrDist += fDelta * fDelta;
		obClosest.Z() = -obOBBExtents.Z();
	}
	else if (obClosest.Z() > obOBBExtents.Z())
	{
		fDelta = obClosest.Z() - obOBBExtents.Z();
		fSqrDist += fDelta * fDelta;
		obClosest.Z() = obOBBExtents.Z();
	}

	if (fSqrDist<=(fMinDistance*fMinDistance)) // Distance between listener and area is inside the min distance
	{
		return 1.0f;
	}
	else if (fSqrDist>=(fMaxDistance*fMaxDistance)) // Distance between listener and area is outside the max distance
	{
		return 0.0f;
	}
	
	// Listener is between the min and max distance
	float fVolume=(fMaxDistance-fsqrtf(fSqrDist))/(fMaxDistance-fMinDistance);	
			
	return fVolume;
}

float CalculateDistanceSqrdPointOBB (const CPoint& obPoint,const CMatrix& obOBB,const CDirection& obOBBExtents)
{
	CDirection obDiff(obPoint - obOBB.GetTranslation());

	float fSqrDist = 0.0f;
	float fDelta;
	CDirection obClosest;

	obClosest.X()=obDiff.Dot(obOBB.GetXAxis());
	if (obClosest.X() < -obOBBExtents.X())
	{
		fDelta = obClosest.X() + obOBBExtents.X();
		fSqrDist += fDelta * fDelta;
		obClosest.X() = -obOBBExtents.X();
	}
	else if (obClosest.X() > obOBBExtents.X())
	{
		fDelta = obClosest.X() - obOBBExtents.X();
		fSqrDist += fDelta * fDelta;
		obClosest.X() = obOBBExtents.X();
	}

	obClosest.Y()=obDiff.Dot(obOBB.GetYAxis());
	if (obClosest.Y() < -obOBBExtents.Y())
	{
		fDelta = obClosest.Y() + obOBBExtents.Y();
		fSqrDist += fDelta * fDelta;
		obClosest.Y() = -obOBBExtents.Y();
	}
	else if (obClosest.Y() > obOBBExtents.Y())
	{
		fDelta = obClosest.Y() - obOBBExtents.Y();
		fSqrDist += fDelta * fDelta;
		obClosest.Y() = obOBBExtents.Y();
	}

	obClosest.Z()=obDiff.Dot(obOBB.GetZAxis());
	if (obClosest.Z() < -obOBBExtents.Z())
	{
		fDelta = obClosest.Z() + obOBBExtents.Z();
		fSqrDist += fDelta * fDelta;
		obClosest.Z() = -obOBBExtents.Z();
	}
	else if (obClosest.Z() > obOBBExtents.Z())
	{
		fDelta = obClosest.Z() - obOBBExtents.Z();
		fSqrDist += fDelta * fDelta;
		obClosest.Z() = obOBBExtents.Z();
	}

	return fSqrDist;
}

bool PointInsideOBB (const CPoint& obPoint,const CMatrix& obOBB,const CDirection& obHalfExtents)
{
	// This code transforms the test point so that its relative to the OBB, thus treating the test as a point inside an AABB
	CDirection obRelativePosition(obPoint - obOBB.GetTranslation());
	float fDistSqrd=obRelativePosition.LengthSquared();
	float fOBBRadius=obHalfExtents.LengthSquared();

	if (fDistSqrd>fOBBRadius) // Do a sphere check first to make sure we are near the OBB
	{
		return 0.0f;
	}

	// Transform the test position so its relative to the OBB
	CMatrix obInverseWorldMatrix(obOBB.GetAffineInverse());

	CDirection obTransformedPosition=obRelativePosition * obInverseWorldMatrix;

	// Check to see if we are outside the AABB
    if (fabsf(obTransformedPosition.X())>obHalfExtents.X() ||
		fabsf(obTransformedPosition.Y())>obHalfExtents.Y() ||
		fabsf(obTransformedPosition.Z())>obHalfExtents.Z())
	{
		return false;
	}

	// Point is inside all 3 axis
	return true;
}

bool PointInsideSphere (const CPoint& obPoint,const CPoint& obSpherePosition,float fRadius)
{
	CDirection obDiff(obPoint - obSpherePosition);
	float fDistSqrd=obDiff.LengthSquared();

	if (fDistSqrd  > (fRadius*fRadius)) // Point is outside the sphere
	{
		return false;
	}

	return true; // Point is inside the sphere
}

bool PointInsideArea (const CPoint& obPoint,const CMatrix& obOBB,const CDirection& obHalfExtents,float fRadius)
{
	// The area is considered to be the nearest point on the OBB (plus fRadius) relative to the test point
	float fDistSqrd=CalculateDistanceSqrdPointOBB(obPoint,obOBB,obHalfExtents);

	if (fDistSqrd > (fRadius*fRadius)) // Point is outside the OBB area
	{
		return false;
	}

	return true; // Point is inside the OBB area
}






// -------------------------- New ambient sound definition --------------------------

AmbientSoundDefinition::AmbientSoundDefinition () :
	BaseAmbientSoundDefinition(),
	m_bEnabled(true),
	m_bDebugRender(false),
	m_eType(HRTF_INTERMITTENT),
	m_eShape(BOX_AREA),
	m_obTriggerPosition(CONSTRUCT_CLEAR),
	m_obTriggerRotation(CONSTRUCT_CLEAR),
	m_obTriggerHalfExtents1(CONSTRUCT_CLEAR),
	m_obTriggerHalfExtents2(CONSTRUCT_CLEAR),
	m_fTriggerRadius(1.0f),
	m_obRandomOffset(CONSTRUCT_CLEAR),
	m_fMinDistance(0.0f),
	m_fMaxDistance(1.0f),
	m_fMinInterval(2.0f),
	m_fMaxInterval(4.0f),
	m_iPlayCount(0),
	m_fFadeInDuration(1.0f),
	m_fFadeOutDuration(1.0f),
	m_bRespawn(false),
	m_iRespawnMinInstances(0),
	m_iRespawnMaxInstances(0),
	m_fRespawnMinInterval(0.0f),
	m_fRespawnMaxInterval(0.0f),
	m_dTime(0.0),
	m_uiSoundHandle(0),
	m_iTimesPlayed(0),
	m_fVolume(0.0f),
	m_fFadeModifier(0.0f),
	m_fTimeToPlay(0.0f),
	m_obActualPosition(CONSTRUCT_CLEAR),
	m_fNextRespawn(0.0f),
	m_iRespawnCount(0)
{
}

AmbientSoundDefinition::~AmbientSoundDefinition ()
{
	AmbientSoundManager::Get().RemoveAmbientSoundDef(this);
}

void AmbientSoundDefinition::PostConstruct ()
{
	// Grab a name from the serialise interface if possible
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
	if ( pDO )
		m_obName = CHashedString(pDO->GetName());

	AmbientSoundManager::Get().AddAmbientSoundDef(this);

	SetActualPosition();

	m_dTime=CTimer::Get().GetGameTime();

	m_fTimeToPlay=((float)m_dTime)+m_fMinInterval+drandf(m_fMaxInterval-m_fMinInterval);

	m_bActive=m_bEnabled;

	m_iTimesPlayed=0;

	m_iRespawnCount=0;
	m_fNextRespawn=0.0f;

	m_bDebugRender=false; // Ambient sounds always start with their debug rendering disabled

	if (m_fMinDistance>m_fMaxDistance) // Ensure minimum distance is never higher than maximum distance
		m_fMinDistance=m_fMaxDistance;

	if (m_iRespawnMinInstances>m_iRespawnMaxInstances)
		m_iRespawnMinInstances=m_iRespawnMaxInstances;

	//Validate(); // There is no point doing this check here sadly, because the sound resources might not be loaded yet...

	CCamUtil::MatrixFromEuler_XYZ(m_obTriggerWorldMatrix,m_obTriggerRotation.X()*DEG_TO_RAD_VALUE,m_obTriggerRotation.Y()*DEG_TO_RAD_VALUE,m_obTriggerRotation.Z()*DEG_TO_RAD_VALUE);
	m_obTriggerWorldMatrix.SetTranslation(m_obTriggerPosition);
}

bool AmbientSoundDefinition::EditorChangeValue (CallBackParameter/*pcItem*/, CallBackParameter/*pcValue*/)
{
#ifndef _RELEASE

	SetActualPosition();

	m_dTime=CTimer::Get().GetGameTime();

	m_fTimeToPlay=((float)m_dTime)+m_fMinInterval+drandf(m_fMaxInterval-m_fMinInterval);

	m_bActive=m_bEnabled;

	m_iTimesPlayed=0;

	m_iRespawnCount=0;
	m_fNextRespawn=0.0f;

	if (m_fMinDistance>m_fMaxDistance) // Ensure minimum distance is never higher than maximum distance
		m_fMinDistance=m_fMaxDistance;

	if (m_iRespawnMinInstances>m_iRespawnMaxInstances)
		m_iRespawnMinInstances=m_iRespawnMaxInstances;

	Validate();

	AudioSystem::Get().Sound_Stop(m_uiSoundHandle);
	m_uiSoundHandle=0;

	CCamUtil::MatrixFromEuler_XYZ(m_obTriggerWorldMatrix,m_obTriggerRotation.X()*DEG_TO_RAD_VALUE,m_obTriggerRotation.Y()*DEG_TO_RAD_VALUE,m_obTriggerRotation.Z()*DEG_TO_RAD_VALUE);
	m_obTriggerWorldMatrix.SetTranslation(m_obTriggerPosition);

#endif // _RELEASE

	return true;
}

void AmbientSoundDefinition::SetActive (bool bActive)
{
	m_bActive=bActive;

	if (m_bActive)
	{
		m_dTime=CTimer::Get().GetGameTime();

		m_fTimeToPlay=((float)CTimer::Get().GetGameTime())+m_fMinInterval+drandf(m_fMaxInterval-m_fMinInterval);

		SetActualPosition();
	}
}

void AmbientSoundDefinition::Update ()
{
	if (!m_bValidSoundResource) // This ambient sound does not have a valid audio resource, therefore we can't do anything...
	{
		Validate(); // Re-validate...
		return;
	}

	if (!IsPaused()) // Only update time if this sound isn't paused
		m_dTime+=CTimer::Get().GetGameTimeChange();

	const float fTime=(float)m_dTime;

	bool bIsPlaying=AudioSystem::Get().Sound_IsPlaying(m_uiSoundHandle);
	CPoint obListenerPosition(AudioSystem::Get().GetListenerPosition());

	//ntPrintf("Listener position=%f, %f, %f  |  Source position=%f, %f, %f\n",
	//	obListenerPosition.X(),obListenerPosition.Y(),obListenerPosition.Z(),m_obActualPosition.X(),m_obActualPosition.Y(),m_obActualPosition.Z());

	switch(m_eType)
	{
		case HRTF_LOOPING:
		{
			bool bInRange;

			if (m_eShape==BOX)
				bInRange=PointInsideOBB(obListenerPosition,m_obTriggerWorldMatrix,m_obTriggerHalfExtents1);
			else if (m_eShape==BOX_AREA)
				bInRange=PointInsideArea(obListenerPosition,m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,m_fTriggerRadius);
			else
				bInRange=PointInsideSphere(obListenerPosition,m_obTriggerPosition,m_fTriggerRadius);

			if (!m_bActive || !bInRange) // We are outside the max range or we are inactive
			{
				FadeOut();
			}
			else
			{
				FadeIn();
			}

			if (m_fFadeModifier>0.0f)
			{
				if (bIsPlaying==false)
				{
					if (AudioSystem::Get().Sound_Prepare(m_uiSoundHandle,ntStr::GetString(m_obSoundBank),ntStr::GetString(m_obSoundCue)))
					{
						AudioSystem::Get().Sound_SetPosition(m_uiSoundHandle,m_obActualPosition);
						AudioSystem::Get().Sound_SetVolume(m_uiSoundHandle,m_fFadeModifier);
						AudioSystem::Get().Sound_Play(m_uiSoundHandle);
					}
				}
				else
				{
					AudioSystem::Get().Sound_SetVolume(m_uiSoundHandle,m_fFadeModifier);
				}
			}
			else
			{
				AudioSystem::Get().Sound_Stop(m_uiSoundHandle);
				m_uiSoundHandle=0;
			}

			break;
		}

		case HRTF_INTERMITTENT:
		{
			if (m_bActive)
			{
				/*
				CPoint obDiff(obListenerPosition); // Calculate distance to listener
				obDiff-=m_obTriggerPosition;
				bool bInRange=(obDiff.LengthSquared() < (m_fTriggerRadius*m_fTriggerRadius) ? true : false);
				*/

				bool bInRange;

				if (m_eShape==BOX)
					bInRange=PointInsideOBB(obListenerPosition,m_obTriggerWorldMatrix,m_obTriggerHalfExtents1);
				else if (m_eShape==BOX_AREA)
					bInRange=PointInsideArea(obListenerPosition,m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,m_fTriggerRadius);
				else
					bInRange=PointInsideSphere(obListenerPosition,m_obTriggerPosition,m_fTriggerRadius);


				if (fTime>m_fTimeToPlay && (m_iPlayCount==0 || m_iTimesPlayed<m_iPlayCount)) // Time to play another sound
				{
					m_fTimeToPlay=fTime+m_fMinInterval+drandf(m_fMaxInterval-m_fMinInterval);
					
					if (bInRange)
					{
						if (AudioSystem::Get().Sound_Prepare(m_uiSoundHandle,ntStr::GetString(m_obSoundBank),ntStr::GetString(m_obSoundCue)))
						{
							AudioSystem::Get().Sound_SetPosition(m_uiSoundHandle,m_obActualPosition);
							AudioSystem::Get().Sound_Play(m_uiSoundHandle);

							SetActualPosition(); // Pick a new position

							++m_iTimesPlayed;
						}

						if (m_bRespawn) // && m_iRespawnCount==0)
						{
							if (m_iRespawnMinInstances==m_iRespawnMaxInstances)
								m_iRespawnCount=m_iRespawnMinInstances;
							else
								m_iRespawnCount=(drand()%(m_iRespawnMaxInstances-m_iRespawnMinInstances))+m_iRespawnMinInstances;

							if (m_iRespawnCount>0)
							{
								m_fNextRespawn=fTime + drandf(m_fRespawnMaxInterval-m_fRespawnMinInterval) + m_fRespawnMinInterval;
							}
						}
					}
				}

				if (bInRange && m_bRespawn && m_iRespawnCount>0 && fTime>m_fNextRespawn)
				{
					if (AudioSystem::Get().Sound_Prepare(m_uiSoundHandle,ntStr::GetString(m_obSoundBank),ntStr::GetString(m_obSoundCue)))
					{
						AudioSystem::Get().Sound_SetPosition(m_uiSoundHandle,m_obActualPosition);
						AudioSystem::Get().Sound_Play(m_uiSoundHandle);

						SetActualPosition(); // Pick a new position
					}

					--m_iRespawnCount;

					m_fNextRespawn=fTime + drandf(m_fRespawnMaxInterval-m_fRespawnMinInterval) + m_fRespawnMinInterval;
				}

				//ntPrintf("%s -> Active=%d  InRange=%d  Times played=%d   Time=%f   TimeToPlay=%f  NextRespawn=%f\n",*GetName(),m_bActive,bInRange,m_iTimesPlayed,fTime,m_fTimeToPlay,m_fNextRespawn);
			}

			break;
		}

		case LINEAR_LOOPING:
		{
			/*
			CPoint obDiff(obListenerPosition); // Calculate distance to listener
			obDiff-=m_obActualPosition;
			float fDistanceFromSoundSource=obDiff.Length();
			float fVolume=0.0f;

			if (!m_bActive) // Fading is only performed depending on the active flag
			{
				FadeOut();
			}
			else
			{
				FadeIn();
			}

			if (fDistanceFromSoundSource<=m_fMaxDistance)// We are within the max range
			{
				if (fDistanceFromSoundSource<m_fMinDistance) // We are below the min distance therefore the base volume is 1.0
				{
					fVolume=m_fFadeModifier;
				}
				else
				{
					// Attenuate the sound linearly between the minimum and maximum distance
					fVolume=(m_fMaxDistance-fDistanceFromSoundSource)/(m_fMaxDistance-m_fMinDistance);
					fVolume*=m_fFadeModifier;
				}
			}
			*/

			if (!m_bActive) // We are outside the max range or we are inactive
			{
				FadeOut();
			}
			else
			{
				FadeIn();
			}

			float fVolume=CalculateVolumeFromListener() * m_fFadeModifier;

			if (fVolume>0.0f)
			{
				if (bIsPlaying==false)
				{
					if (AudioSystem::Get().Sound_Prepare(m_uiSoundHandle,ntStr::GetString(m_obSoundBank),ntStr::GetString(m_obSoundCue)))
					{
						AudioSystem::Get().Sound_SetVolume(m_uiSoundHandle,fVolume);
						AudioSystem::Get().Sound_Play(m_uiSoundHandle);
					}
				}
				else
				{
					AudioSystem::Get().Sound_SetVolume(m_uiSoundHandle,fVolume);
				}
			}
			else
			{
				AudioSystem::Get().Sound_Stop(m_uiSoundHandle);
				m_uiSoundHandle=0;
			}

			//ntPrintf("%s -> Active=%d  Volume=%f  Fade modifier=%f  Dist from source=%f  IsPlaying=%d\n",*GetName(),m_bActive,fVolume,m_fFadeModifier,fDistanceFromSoundSource,bIsPlaying);

			break;
		}

		case LINEAR_INTERMITTENT:
		{
			if (m_bActive)
			{
				/*
				CPoint obDiff(obListenerPosition); // Calculate distance to listener
				obDiff-=m_obTriggerPosition;
				bool bInRange=(obDiff.LengthSquared() < (m_fTriggerRadius*m_fTriggerRadius) ? true : false);
				*/

				bool bInRange;

				if (m_eShape==BOX)
					bInRange=PointInsideOBB(obListenerPosition,m_obTriggerWorldMatrix,m_obTriggerHalfExtents1);
				else if (m_eShape==BOX_AREA)
					bInRange=PointInsideArea(obListenerPosition,m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,m_fTriggerRadius);
				else
					bInRange=PointInsideSphere(obListenerPosition,m_obTriggerPosition,m_fTriggerRadius);


				if (fTime>m_fTimeToPlay && (m_iPlayCount==0 || m_iTimesPlayed<m_iPlayCount)) // Time to play another sound
				{
					m_fTimeToPlay=fTime+m_fMinInterval+drandf(m_fMaxInterval-m_fMinInterval);
					
					if (bInRange)
					{
						if (AudioSystem::Get().Sound_Prepare(m_uiSoundHandle,ntStr::GetString(m_obSoundBank),ntStr::GetString(m_obSoundCue)))
						{
							AudioSystem::Get().Sound_SetPosition(m_uiSoundHandle,m_obActualPosition);
							AudioSystem::Get().Sound_Play(m_uiSoundHandle);

							SetActualPosition();

							++m_iTimesPlayed;
						}

						if (m_bRespawn) // && m_iRespawnCount==0)
						{
							if (m_iRespawnMinInstances==m_iRespawnMaxInstances)
								m_iRespawnCount=m_iRespawnMinInstances;
							else
								m_iRespawnCount=(drand()%(m_iRespawnMaxInstances-m_iRespawnMinInstances))+m_iRespawnMinInstances;

							if (m_iRespawnCount>0)
							{
								m_fNextRespawn=fTime + drandf(m_fRespawnMaxInterval-m_fRespawnMinInterval) + m_fRespawnMinInterval;
							}
						}
					}
				}

				if (bInRange && m_bRespawn && m_iRespawnCount>0 && fTime>m_fNextRespawn)
				{
					unsigned int id;

					if (AudioSystem::Get().Sound_Prepare(id,ntStr::GetString(m_obSoundBank),ntStr::GetString(m_obSoundCue)))
					{
						AudioSystem::Get().Sound_SetPosition(id,m_obActualPosition);
						AudioSystem::Get().Sound_Play(id);

						SetActualPosition();
					}

					--m_iRespawnCount;

					m_fNextRespawn=fTime + drandf(m_fRespawnMaxInterval-m_fRespawnMinInterval) + m_fRespawnMinInterval;
				}
			}

			break;
		}

		case GLOBAL_LOOPING:
		{
			if (m_bActive)
			{
				FadeIn();

				if (!bIsPlaying) // If the sound aint playing, then fire it off
				{
					if (AudioSystem::Get().Sound_Prepare(m_uiSoundHandle,ntStr::GetString(m_obSoundBank),ntStr::GetString(m_obSoundCue)))
					{
						AudioSystem::Get().Sound_SetVolume(m_uiSoundHandle,m_fFadeModifier);
						AudioSystem::Get().Sound_Play(m_uiSoundHandle);
					}
				}
				else
				{
					AudioSystem::Get().Sound_SetVolume(m_uiSoundHandle,m_fFadeModifier);
				}
			}
			else
			{
				FadeOut();

				if (m_fFadeModifier==0.0f) // Sound should no longer be audible, so stop it
				{
					AudioSystem::Get().Sound_Stop(m_uiSoundHandle);
					m_uiSoundHandle=0;
				}
				else
				{
					AudioSystem::Get().Sound_SetVolume(m_uiSoundHandle,m_fFadeModifier);
				}
			}

			break;
		}

		case GLOBAL_INTERMITTENT:
		{
			if (m_bActive)
			{
				if (fTime>m_fTimeToPlay && (m_iPlayCount==0 || m_iTimesPlayed<m_iPlayCount)) // Time to play another sound
				{
					m_fTimeToPlay=fTime+m_fMinInterval+drandf(m_fMaxInterval-m_fMinInterval);

					if (AudioSystem::Get().Sound_Prepare(m_uiSoundHandle,ntStr::GetString(m_obSoundBank),ntStr::GetString(m_obSoundCue)))
					{
						AudioSystem::Get().Sound_Play(m_uiSoundHandle);

						++m_iTimesPlayed;
					}

					if (m_bRespawn) // && m_iRespawnCount==0)
					{
						if (m_iRespawnMinInstances==m_iRespawnMaxInstances)
							m_iRespawnCount=m_iRespawnMinInstances;
						else
							m_iRespawnCount=(drand()%(m_iRespawnMaxInstances-m_iRespawnMinInstances))+m_iRespawnMinInstances;

						if (m_iRespawnCount>0)
						{
							m_fNextRespawn=fTime + drandf(m_fRespawnMaxInterval-m_fRespawnMinInterval) + m_fRespawnMinInterval;
						}
					}
				}

				if (m_bRespawn && m_iRespawnCount>0 && fTime>m_fNextRespawn)
				{
					unsigned int id;

					if (AudioSystem::Get().Sound_Prepare(id,ntStr::GetString(m_obSoundBank),ntStr::GetString(m_obSoundCue)))
					{
						AudioSystem::Get().Sound_Play(id);
					}

					--m_iRespawnCount;

					m_fNextRespawn=fTime + drandf(m_fRespawnMaxInterval-m_fRespawnMinInterval) + m_fRespawnMinInterval;
				}
			}

			break;
		}
	}
}

float AmbientSoundDefinition::CalculateVolumeFromListener ()
{
	CPoint obListenerPosition(AudioSystem::Get().GetListenerPosition());

	switch(m_eShape)
	{
		case BOX:
		{
			return CalculateAttenuationFromOBB(obListenerPosition,m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,m_obTriggerHalfExtents2);

			break;
		}

		case BOX_AREA:
		{
			return CalculateAttenuationFromOBBArea(obListenerPosition,m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,m_fMinDistance,m_fMaxDistance);
			
			break;
		}

		default: // By default we assume its a sphere shape
		{
			return CalculateAttenuationFromSphere(obListenerPosition,m_obTriggerPosition,m_fMinDistance,m_fMaxDistance);

			break;
		}
	}

	//return 0.0f; // Shouldn't ever get here!
}

void AmbientSoundDefinition::FadeIn ()
{
	if (m_fFadeModifier<1.0f)
	{
		if (m_fFadeInDuration>0.0f)
		{
			if (!IsPaused())
				m_fFadeModifier+=CTimer::Get().GetGameTimeChange()/m_fFadeInDuration;

			if (m_fFadeModifier>1.0f)
				m_fFadeModifier=1.0f;
		}
		else
		{
			m_fFadeModifier=1.0f;
		}
	}
}

void AmbientSoundDefinition::FadeOut ()
{
	if (m_fFadeModifier>0.0f)
	{
		if (m_fFadeOutDuration>0.0f)
		{
			if (!IsPaused())
				m_fFadeModifier-=CTimer::Get().GetGameTimeChange()/m_fFadeOutDuration;

			if (m_fFadeModifier<0.0f)
				m_fFadeModifier=0.0f;
		}
		else
		{
			m_fFadeModifier=0.0f;
		}
	}
}

void AmbientSoundDefinition::SetActualPosition ()
{
	m_obActualPosition=m_obTriggerPosition;

	if (m_obRandomOffset.X()!=0.0f)
		m_obActualPosition.X()+=drandf(m_obRandomOffset.X()*2.0f)-m_obRandomOffset.X();

	if (m_obRandomOffset.Y()!=0.0f)
		m_obActualPosition.Y()+=drandf(m_obRandomOffset.Y()*2.0f)-m_obRandomOffset.Y();

	if (m_obRandomOffset.Z()!=0.0f)
		m_obActualPosition.Z()+=drandf(m_obRandomOffset.Z()*2.0f)-m_obRandomOffset.Z();
}

bool AmbientSoundDefinition::IsPaused ()
{
	return AudioSystem::Get().Sound_IsPaused(m_uiSoundHandle);
}

void AmbientSoundDefinition::Validate ()
{
	// This function checks to see if the sound assigned to it is a valid one.

	m_bValidSoundResource=true;
}

void AmbientSoundDefinition::DebugRender ()
{
#ifndef _RELEASE

	if (!m_bDebugRender)
		return;

	#ifdef PLATFORM_PC
	const unsigned int VOLUME_COLOUR=(m_bActive ? 0x77ffff00 : 0x77ff7700);
	#else
	const unsigned int VOLUME_COLOUR=(m_bActive ? 0x00ffff77 : 0x0077ff77);
	#endif

	switch(m_eType)
	{
		case HRTF_LOOPING:
		{
			g_VisualDebug->Printf3D( m_obTriggerPosition, 0xffffffff, DTF_ALIGN_HCENTRE, ntStr::GetString(m_obName) );

			if (m_eShape==BOX || m_eShape==BOX_AREA)
			{
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,DPF_WIREFRAME);
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,0);
			}
			else // SPHERE
			{
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obActualPosition,m_fTriggerRadius,0xffffffff,DPF_WIREFRAME);
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obActualPosition,m_fTriggerRadius,VOLUME_COLOUR,0);
			}

			break;
		}

		case HRTF_INTERMITTENT:
		{
			g_VisualDebug->Printf3D( m_obTriggerPosition, 0xffffffff, DTF_ALIGN_HCENTRE, ntStr::GetString(m_obName) );

			if (m_eShape==BOX || m_eShape==BOX_AREA)
			{
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,DPF_WIREFRAME);
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,0);
			}
			else // SPHERE
			{
                g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obTriggerPosition,m_fTriggerRadius,0xffffffff,DPF_WIREFRAME);
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obTriggerPosition,m_fTriggerRadius,VOLUME_COLOUR,0);
			}

			CMatrix obWorldMatrix(CONSTRUCT_IDENTITY);
			obWorldMatrix.SetTranslation(m_obTriggerPosition);

			CDirection obHalfExtents(m_obRandomOffset);

			if (obHalfExtents.LengthSquared()>EPSILON)
				g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,0xffffffff,DPF_WIREFRAME);

			break;
		}

		case LINEAR_LOOPING:
		{
			g_VisualDebug->Printf3D( m_obActualPosition, 0xffffffff, DTF_ALIGN_HCENTRE, ntStr::GetString(m_obName) );

			if (m_eShape==BOX)
			{
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,DPF_WIREFRAME);
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,0);

				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents2,VOLUME_COLOUR,DPF_WIREFRAME);
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents2,VOLUME_COLOUR,0);
			}
			else if (m_eShape==BOX_AREA)
			{
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,DPF_WIREFRAME);
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,0);
			}
			else // SPHERE
			{
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obActualPosition,m_fMinDistance,0xffffffff,DPF_WIREFRAME);
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obActualPosition,m_fMinDistance,VOLUME_COLOUR,0);

				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obActualPosition,m_fMaxDistance,0xffffffff,DPF_WIREFRAME);
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obActualPosition,m_fMaxDistance,VOLUME_COLOUR,0);
			}

			break;
		}

		case LINEAR_INTERMITTENT:
		{
			g_VisualDebug->Printf3D( m_obTriggerPosition, 0xffffffff, DTF_ALIGN_HCENTRE, ntStr::GetString(m_obName) );

			if (m_eShape==BOX || m_eShape==BOX_AREA)
			{
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,DPF_WIREFRAME);
				g_VisualDebug->RenderOBB(m_obTriggerWorldMatrix,m_obTriggerHalfExtents1,VOLUME_COLOUR,0);
			}
			else // SPHERE
			{
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obActualPosition,m_fTriggerRadius,0xffffffff,DPF_WIREFRAME);
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obActualPosition,m_fTriggerRadius,VOLUME_COLOUR,0);
			}

			CMatrix obWorldMatrix(CONSTRUCT_IDENTITY);
			obWorldMatrix.SetTranslation(m_obTriggerPosition);

			CDirection obHalfExtents(m_obRandomOffset);

			if (obHalfExtents.LengthSquared()>EPSILON)
				g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,0xffffffff,DPF_WIREFRAME);

			break;
		}

		case GLOBAL_LOOPING:
		{
			break;
		}

		case GLOBAL_INTERMITTENT:
		{
			break;
		}

		default:
			break;
	}

#endif // _RELEASE
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------

SlaveAmbientSoundDefinition::SlaveAmbientSoundDefinition () :
	m_obPosition(CONSTRUCT_CLEAR),
	m_obRandomOffset(CONSTRUCT_CLEAR),
	m_fMinStartOffset(0.0f),
	m_fMaxStartOffset(0.0f),
	m_fProbability(0.5f),
	m_fPlayTimer(0.0f)
{
}

SlaveAmbientSoundDefinition::~SlaveAmbientSoundDefinition ()
{
}

void SlaveAmbientSoundDefinition::PostConstruct ()
{
#ifndef _RELEASE

	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );

	if ( pDO )
		m_obName = CHashedString(pDO->GetName());

#endif // _RELEASE
}

bool SlaveAmbientSoundDefinition::EditorChangeValue(CallBackParameter /*pcItem*/, CallBackParameter /*pcValue*/)
{
#ifndef _RELEASE
	
	// Ensure the values are valid
	if (m_fMinStartOffset<0.0f)
		m_fMinStartOffset=0.0f;

	if (m_fMaxStartOffset<0.0f)
		m_fMaxStartOffset=0.0f;

	if (m_fMinStartOffset>m_fMaxStartOffset)
		m_fMinStartOffset=m_fMaxStartOffset;

	if (m_fProbability<0.0f)
		m_fProbability=0.0f;

	if (m_fProbability>1.0f)
		m_fProbability=1.0f;

#endif // _RELEASE

	return true;
}

void SlaveAmbientSoundDefinition::Trigger ()
{
	if (m_fProbability==0.0f || grandf(1.0f)>m_fProbability)
		return;

	if (m_fMinStartOffset==0.0f && m_fMaxStartOffset==0.0f)
	{
		m_fPlayTimer=0.001f;	}
	else
	{
		m_fPlayTimer=m_fMinStartOffset+grandf(m_fMaxStartOffset-m_fMinStartOffset);
	}
}

void SlaveAmbientSoundDefinition::Update (float fTimeDelta)
{
	if (m_fPlayTimer==0.0f)
		return;

	m_fPlayTimer-=fTimeDelta;

	if (m_fPlayTimer<=0.0f)
	{
		m_fPlayTimer=0.0f;

		//ntPrintf("Triggering slave %s\n",m_obName.GetDebugString());

		unsigned int id;
		
		if (AudioSystem::Get().Sound_Prepare(id,m_obSound.GetString()))
		{
			CPoint obPosition(
				m_obPosition.X() - m_obRandomOffset.X() + grandf(m_obRandomOffset.X() * 2.0f),
				m_obPosition.Y() - m_obRandomOffset.Y() + grandf(m_obRandomOffset.Y() * 2.0f),
				m_obPosition.Z() - m_obRandomOffset.Z() + grandf(m_obRandomOffset.Z() * 2.0f));

			AudioSystem::Get().Sound_SetPosition(id,obPosition);
			AudioSystem::Get().Sound_Play(id);
		}
	}
}

void SlaveAmbientSoundDefinition::DebugRender ()
{
#ifndef _RELEASE

	// Render random offset area
	g_VisualDebug->RenderLine(CPoint(m_obPosition.X()-0.5f,m_obPosition.Y(),m_obPosition.Z()),CPoint(m_obPosition.X()+0.5f,m_obPosition.Y(),m_obPosition.Z()),0xffffffff);
	g_VisualDebug->RenderLine(CPoint(m_obPosition.X(),m_obPosition.Y()-0.5f,m_obPosition.Z()),CPoint(m_obPosition.X(),m_obPosition.Y()+0.5f,m_obPosition.Z()),0xffffffff);
	g_VisualDebug->RenderLine(CPoint(m_obPosition.X(),m_obPosition.Y(),m_obPosition.Z()-0.5f),CPoint(m_obPosition.X(),m_obPosition.Y(),m_obPosition.Z()+0.5f),0xffffffff);
	g_VisualDebug->RenderAABB(CPoint(m_obPosition - m_obRandomOffset),CPoint(m_obPosition + m_obRandomOffset),0x33ffffff, DPF_WIREFRAME);

	// Render some debug info text
	float fY=-12.0f * 4.0f - 4.0f;
	g_VisualDebug->Printf3D(m_obPosition,0.0f,fY,0xffffffff,DTF_ALIGN_HCENTRE,"Slave:%s",m_obName.GetDebugString()); fY+=12.0f;
	g_VisualDebug->Printf3D(m_obPosition,0.0f,fY,0xffffffff,DTF_ALIGN_HCENTRE,"Sound:%s",m_obSound.GetString()); fY+=12.0f;
	g_VisualDebug->Printf3D(m_obPosition,0.0f,fY,0xffffffff,DTF_ALIGN_HCENTRE,"Prob:%.1f%%",m_fProbability*100.0f); fY+=12.0f;
	g_VisualDebug->Printf3D(m_obPosition,0.0f,fY,0xffffffff,DTF_ALIGN_HCENTRE,"Start Time:%.2f - %.2f",m_fMinStartOffset,m_fMaxStartOffset); fY+=12.0f;

#endif // _RELEASE
}



//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

MasterAmbientSoundDefinition::MasterAmbientSoundDefinition () :
	BaseAmbientSoundDefinition(),
	m_bEnabled(true),
	m_bDebugRender(false),
	m_eShape(SPHERE),
	m_obTriggerPosition(CONSTRUCT_CLEAR),
	m_obTriggerRotation(CONSTRUCT_CLEAR),
	m_obTriggerHalfExtents(CONSTRUCT_CLEAR),
	m_fTriggerRadius(1.0f),
	m_obRandomOffset(CONSTRUCT_CLEAR),
	m_fMinInterval(0.0f),
	m_fMaxInterval(10.0f),
	m_obTriggerMatrix(CONSTRUCT_IDENTITY),
	m_fPlayTimer(0.0f)
{
}

MasterAmbientSoundDefinition::~MasterAmbientSoundDefinition ()
{
	if (AmbientSoundManager::Exists())
		AmbientSoundManager::Get().RemoveAmbientSoundDef(this);
}

void MasterAmbientSoundDefinition::PostConstruct ()
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
	if ( pDO )
		m_obName = CHashedString(pDO->GetName());

	AmbientSoundManager::Get().AddAmbientSoundDef(this);

	m_bDebugRender=false;

	m_fPlayTimer=grandf(m_fMaxInterval-m_fMinInterval) + m_fMinInterval;

	m_bActive=m_bEnabled;

	CCamUtil::MatrixFromEuler_XYZ(m_obTriggerMatrix,m_obTriggerRotation.X()*DEG_TO_RAD_VALUE,m_obTriggerRotation.Y()*DEG_TO_RAD_VALUE,m_obTriggerRotation.Z()*DEG_TO_RAD_VALUE);
	m_obTriggerMatrix.SetTranslation(m_obTriggerPosition);
}

bool MasterAmbientSoundDefinition::EditorChangeValue(CallBackParameter /*pcItem*/,CallBackParameter /*pcValue*/)
{
#ifndef _RELEASE

	CCamUtil::MatrixFromEuler_XYZ(m_obTriggerMatrix,m_obTriggerRotation.X()*DEG_TO_RAD_VALUE,m_obTriggerRotation.Y()*DEG_TO_RAD_VALUE,m_obTriggerRotation.Z()*DEG_TO_RAD_VALUE);
	m_obTriggerMatrix.SetTranslation(m_obTriggerPosition);

#endif // _RELEASE

	return true;
}

void MasterAmbientSoundDefinition::Update ()
{
	if (!m_bActive)
		return;

	CPoint obListenerPosition(AudioSystem::Get().GetListenerPosition());

	// Check to see if the listener is inside the trigger area

	if ((m_eShape==SPHERE && PointInsideSphere(obListenerPosition,m_obTriggerPosition,m_fTriggerRadius)) || // The trigger area is a sphere and we are inside it
		((m_eShape==BOX || m_eShape==BOX_AREA) && PointInsideOBB(obListenerPosition,m_obTriggerMatrix,m_obTriggerHalfExtents))) // The trigger area is a box and we are inside it
	{
		const float fTimeDelta=CTimer::Get().GetGameTimeChange();

		m_fPlayTimer-=fTimeDelta; // Decrement the play timer

		if (m_fPlayTimer<=0.0f) // We are ready to play
		{
			m_fPlayTimer=grandf(m_fMaxInterval-m_fMinInterval) + m_fMinInterval; // Reset the timer

			//ntPrintf("Triggering master %s\n",m_obName.GetDebugString());

			// Trigger our master sound
			unsigned int id;
			
			if (!m_obSound.IsNull() && AudioSystem::Get().Sound_Prepare(id,m_obSound.GetString()))
			{
				CPoint obPosition(
					m_obTriggerPosition.X() - m_obRandomOffset.X() + grandf(m_obRandomOffset.X() * 2.0f),
					m_obTriggerPosition.Y() - m_obRandomOffset.Y() + grandf(m_obRandomOffset.Y() * 2.0f),
					m_obTriggerPosition.Z() - m_obRandomOffset.Z() + grandf(m_obRandomOffset.Z() * 2.0f));

				AudioSystem::Get().Sound_SetPosition(id,obPosition);
				AudioSystem::Get().Sound_Play(id);
			}

			// Trigger each of the slaves
			for(ntstd::List<SlaveAmbientSoundDefinition*>::iterator obIt=m_obSlaveList.begin(); obIt!=m_obSlaveList.end(); ++obIt)
			{
				(*obIt)->Trigger();
			}
		}

		// Make sure slaves work each update
		for(ntstd::List<SlaveAmbientSoundDefinition*>::iterator obIt=m_obSlaveList.begin(); obIt!=m_obSlaveList.end(); ++obIt)
		{
			(*obIt)->Update(fTimeDelta);
		}
	}
}

void MasterAmbientSoundDefinition::DebugRender ()
{
#ifndef _RELEASE

	if (!m_bDebugRender)
		return;

	// Render the trigger volume
	g_VisualDebug->RenderLine(CPoint(m_obTriggerPosition.X()-0.5f,m_obTriggerPosition.Y(),m_obTriggerPosition.Z()),CPoint(m_obTriggerPosition.X()+0.5f,m_obTriggerPosition.Y(),m_obTriggerPosition.Z()),0xffffffff);
	g_VisualDebug->RenderLine(CPoint(m_obTriggerPosition.X(),m_obTriggerPosition.Y()-0.5f,m_obTriggerPosition.Z()),CPoint(m_obTriggerPosition.X(),m_obTriggerPosition.Y()+0.5f,m_obTriggerPosition.Z()),0xffffffff);
	g_VisualDebug->RenderLine(CPoint(m_obTriggerPosition.X(),m_obTriggerPosition.Y(),m_obTriggerPosition.Z()-0.5f),CPoint(m_obTriggerPosition.X(),m_obTriggerPosition.Y(),m_obTriggerPosition.Z()+0.5f),0xffffffff);
	
	if (m_eShape==SPHERE)
	{
        g_VisualDebug->RenderSphere( CQuat(CONSTRUCT_IDENTITY), m_obTriggerPosition, m_fTriggerRadius, 0x33ffffff, 0);
	}
	else
	{
		g_VisualDebug->RenderOBB(m_obTriggerMatrix,m_obTriggerHalfExtents,0x33ffffff);
	}

	
	// Render the random offset area
	g_VisualDebug->RenderAABB(CPoint(m_obTriggerPosition - m_obRandomOffset),CPoint(m_obTriggerPosition + m_obRandomOffset),0xaaffffff, DPF_WIREFRAME);

	// Render some information text
	float fY=-12.0f * 4.0f - 4.0f;

	g_VisualDebug->Printf3D(m_obTriggerPosition,0.0f,fY,0xffffffff,DTF_ALIGN_HCENTRE,"Master:%s",m_obName.GetDebugString()); fY+=12.0f;
	g_VisualDebug->Printf3D(m_obTriggerPosition,0.0f,fY,0xffffffff,DTF_ALIGN_HCENTRE,"Sound:%s",m_obSound.GetString()); fY+=12.0f;
	g_VisualDebug->Printf3D(m_obTriggerPosition,0.0f,fY,0xffffffff,DTF_ALIGN_HCENTRE,"Interval:%.2f - %.2f ",m_fMinInterval,m_fMaxInterval); fY+=12.0f;
	
	if (m_bActive)
		g_VisualDebug->Printf3D(m_obTriggerPosition,0.0f,fY,0xffffffff,DTF_ALIGN_HCENTRE,"Active");
	else
		g_VisualDebug->Printf3D(m_obTriggerPosition,0.0f,fY,0xffffffff,DTF_ALIGN_HCENTRE,"Inactive");
	
	fY+=12.0f;

	// Render the slave stuff
	for(ntstd::List<SlaveAmbientSoundDefinition*>::iterator obIt=m_obSlaveList.begin(); obIt!=m_obSlaveList.end(); ++obIt)
	{
		(*obIt)->DebugRender();
	}

#endif // _RELEASE
}

void MasterAmbientSoundDefinition::SetActive (bool bActive)
{
	m_bActive=bActive;

	m_fPlayTimer=grandf(m_fMaxInterval-m_fMinInterval) + m_fMinInterval;
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------



AmbientSoundManager::AmbientSoundManager () :
	m_bActive(false)
{
	/*
	//CPoint obListenerPosition(8.5f,0.0f,8.5f);
	CPoint obListenerPosition(5.1f,0.0f,0.0f);
	//CMatrix obOBB(CDirection(0.0f,1.0f,0.0f),45.0f * DEG_TO_RAD_VALUE);
	CMatrix obOBB(CONSTRUCT_IDENTITY);
	CDirection obInnerHalfExtents(5.0f,5.0f,5.0f);
	CDirection obOuterHalfExtetns(10.0f,10.0f,10.0f);

	CalculateAttenuationFromOBB(obListenerPosition,obOBB,obInnerHalfExtents,obOuterHalfExtetns);

	ntPrintf("Distance from box=%f\n",fsqrtf(CalculateDistancePointOBB(obListenerPosition,obOBB,obInnerHalfExtents)));
	*/
}

AmbientSoundManager::~AmbientSoundManager ()
{
	m_obAmbientSoundDefList.clear();
}

void AmbientSoundManager::Update ()
{
	if (m_bActive)
	{
		for(ntstd::List<BaseAmbientSoundDefinition*>::iterator obIt=m_obAmbientSoundDefList.begin(); obIt!=m_obAmbientSoundDefList.end(); ++obIt)
		{
			(*obIt)->Update();
		}
	}
}

void AmbientSoundManager::SetActive (bool bActive)
{
	if (bActive!=m_bActive)
	{
		if (bActive)
		{
			// Reset any existing ambient sounds
			for(ntstd::List<BaseAmbientSoundDefinition*>::iterator obIt=m_obAmbientSoundDefList.begin(); obIt!=m_obAmbientSoundDefList.end(); ++obIt)
			{
				(*obIt)->PostConstruct();
			}
		}

		m_bActive=bActive;
	}

}

void AmbientSoundManager::AddAmbientSoundDef (BaseAmbientSoundDefinition* pobAmbientSoundDef)
{
	// Prevent duplicate definitions from being added
	for(ntstd::List<BaseAmbientSoundDefinition*>::iterator obIt=m_obAmbientSoundDefList.begin(); obIt!=m_obAmbientSoundDefList.end(); ++obIt)
	{
		if (pobAmbientSoundDef==(*obIt)) // This ambient sound def appears to already be in the list...
			return;
	}

	m_obAmbientSoundDefList.push_back(pobAmbientSoundDef); // Okay it's safe to add to the list
}

void AmbientSoundManager::RemoveAmbientSoundDef (BaseAmbientSoundDefinition* pobAmbientSoundDef)
{
	m_obAmbientSoundDefList.remove(pobAmbientSoundDef);
}

void AmbientSoundManager::Activate (const CHashedString& obAmbientSound)
{
	for(ntstd::List<BaseAmbientSoundDefinition*>::iterator obAmbientSoundIt=m_obAmbientSoundDefList.begin(); obAmbientSoundIt!=m_obAmbientSoundDefList.end(); ++obAmbientSoundIt)
	{
		if( (*obAmbientSoundIt)->GetName()==obAmbientSound )
		{
			#ifndef _RELEASE
			ntPrintf("AmbientSoundManager: Activating %s\n",ntStr::GetString(obAmbientSound));
			#endif // _RELEASE
			(*obAmbientSoundIt)->SetActive(true);
			return;
		}
	}

	#ifdef _AMBIENTSOUND_DEBUG
	ntPrintf("AmbientSoundManager: Can't activate %s, doesn't exist\n", ntStr::GetString(obAmbientSound));
	#endif // _AMBIENTSOUND_DEBUG
}

void AmbientSoundManager::Deactivate (const CHashedString& obAmbientSound)
{
	for(ntstd::List<BaseAmbientSoundDefinition*>::iterator obAmbientSoundIt=m_obAmbientSoundDefList.begin(); obAmbientSoundIt!=m_obAmbientSoundDefList.end(); ++obAmbientSoundIt)
	{
		if( (*obAmbientSoundIt)->GetName()==obAmbientSound )
		{
			#ifndef _RELEASE
			ntPrintf("AmbientSoundManager: Deactivating %s\n", ntStr::GetString(obAmbientSound));
			#endif // _RELEASE
			(*obAmbientSoundIt)->SetActive(false);
			return;
		}
	}

	#ifdef _AMBIENTSOUND_DEBUG
	ntPrintf("AmbientSoundManager: Can't deactivate %s, doesn't exist\n",ntStr::GetString(obAmbientSound));
	#endif // _AMBIENTSOUND_DEBUG
}

bool AmbientSoundManager::IsActive (const CHashedString& obAmbientSound)
{
	for(ntstd::List<BaseAmbientSoundDefinition*>::iterator obAmbientSoundIt=m_obAmbientSoundDefList.begin(); obAmbientSoundIt!=m_obAmbientSoundDefList.end(); ++obAmbientSoundIt)
	{
		if( (*obAmbientSoundIt)->GetName()==obAmbientSound )
		{
			return (*obAmbientSoundIt)->IsActive();
		}
	}

	return false; // Unable to find the ambient sound
}





