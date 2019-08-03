//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_GladiatorGeneral.cpp
//!
//------------------------------------------------------------------------------------------

#include "coolcam_gladiatorgeneral.h"
#include "camera/sceneelementcomponent.h"
#include "camera/camutils.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "core/visualdebugger.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "objectdatabase/dataobject.h"
#include "physics/world.h"
#include "game/entityboss.h"
#include "game/entitymanager.h"
#include "camera/camvolume_bossavoid.h"
#include "core/mem.h"
#include "game/inputcomponent.h"

#ifdef BOSS_CAM_DBGREND
#include "game/randmanager.h"
#endif

//#define BOSS_CAM_DBGREND

//------------------------------------------------------------------------------------------
// Interfaces                                                                               
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE( CoolCam_GladiatorGeneralDef, Mem::MC_CAMERA)
	DEFINE_INTERFACE_INHERITANCE(CoolCam_RootBossDef)
	COPY_INTERFACE_FROM(CoolCam_RootBossDef)

	PUBLISH_VAR_AS( m_fOffsetFromPillarVolume, OffsetFromPillarVolume )
	PUBLISH_VAR_AS( m_fDeflectionAngleFromPillar, DeflectionAngleFromPillar )
	PUBLISH_VAR_AS( m_fDeflectedHeightModifier, DeflectedHeightModifier )
	PUBLISH_VAR_AS( m_fBasePillarCameraHeight, BasePillarCameraHeight )

	PUBLISH_VAR_AS( m_fPlayerMovementFocusBias, PlayerMovementFocusBias )

	PUBLISH_VAR_AS( m_bEliminateVerticalDeflection, EliminateVerticalDeflection )

	DECLARE_CSTRUCTURE_PTR_CONTAINER( m_avoidVolList, AvoidVolumes )

	PUBLISH_VAR_AS( m_bRenderPillarVolumes, RenderPillarVolumes )
	PUBLISH_VAR_AS( m_bRenderCameraForceVolumes, RenderCameraForceVolumes )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_bDoObstructionCheck, true, DoObstructionCheck )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bDoObstructionCameraCut, true, DoObstructionCameraCut )

END_STD_INTERFACE


CoolCam_GladiatorGeneralDef::CoolCam_GladiatorGeneralDef( void )
: m_fOffsetFromPillarVolume(4.0f),
  m_fDeflectionAngleFromPillar(70.0f),
  m_fBasePillarCameraHeight(2.0f),
  m_fPlayerMovementFocusBias(0.25f),
  m_bEliminateVerticalDeflection(true),
  m_bRenderPillarVolumes(false),
  m_bRenderCameraForceVolumes(false),
  m_bDoObstructionCheck(true),
  m_bDoObstructionCameraCut(true)
{
}

//------------------------------------------------------------------------------------------
//!
//!	CoolCam_GladiatorGeneralDef::Create
//!	Creates a camera instance from the definition.
//!
//------------------------------------------------------------------------------------------

CoolCam_GladiatorGeneral* CoolCam_GladiatorGeneralDef::Create( const CamView& view )
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_GladiatorGeneral( view, this );
}

CoolCam_GladiatorGeneral::CoolCam_GladiatorGeneral( const CamView& view, const CoolCam_GladiatorGeneralDef* pCamDef )
: CoolCam_RootBoss(view, pCamDef),	
  m_pCamDef(pCamDef),
  m_pobBoss(pCamDef->GetBossEntity()),
  m_fMinDist(3.0f),
  m_fMaxDist(5.0f),
  m_fMinFOV(30.0f),
  m_fMaxFOV(35.0f),
  m_fCurrDist(5.0f),
  m_fCurrFOV(30.0f),
  m_fFOVToleranceZone(0.1f),
  m_fDistToleranceZone(0.75f),
  m_fMinCamFocusDist(5.0f),
  m_fMinCamFocusDistSqrd(25.0f),
  m_fMinCamBossDistSqrd(9.0f),
  m_obCamVerticalOffset( 0.0f, 1.5f, 0.0f ),
  m_fBossHeroSeperationAngleDeg(40.0f),
  m_fSeperationDistanceModifier(1.0f),
  m_fSepDistModifierIncrement(0.075f),
  m_fCameraAngleModifierDeg(90.0f),
  m_obTotalForce( CONSTRUCT_CLEAR ),
  m_obCamVelocity( CONSTRUCT_CLEAR ),
  m_obCachedPosition( CONSTRUCT_CLEAR ),
  m_pObstructingAvoidVol(0)
#ifdef BOSS_CAM_DBGREND
  ,m_iDebugTextOffset1(90),
  m_iDebugTextOffset2(0)
#endif
{
	ntAssert_p( pCamDef, ("Invalid CoolCam_GladiatorGeneralDef pointer\n") );

	m_pobLinearSpeedSampleSet = m_obLinearSpeed.AddSampleSet( "SPEED", 750, DC_GREEN );
	m_pobFocusSpeedSampleSet = m_obFocusSpeed.AddSampleSet( "SPEED", 750, DC_GREEN );

	m_obLinearSpeed.SetYAxis( 0.0f, m_pCamDef->GetCameraMaxSpeed() , m_pCamDef->GetCameraMaxSpeed() / 100.0f );
	m_obFocusSpeed.SetYAxis( 0.0f, m_pCamDef->GetFocusPointMaxSpeed(), m_pCamDef->GetFocusPointMaxSpeed() / 100.0f );

	CPoint obPlayerPosition( CONSTRUCT_CLEAR );
	CEntity* pPlayer = CEntityManager::Get().GetPlayer();
	if( pPlayer && pPlayer->GetSceneElement() )
	{
		obPlayerPosition = pPlayer->GetSceneElement()->GetPosition();
	}
	else if( pPlayer ) 
	{
		obPlayerPosition = pPlayer->GetPosition();
	}

	CPoint obBossPosition( CONSTRUCT_CLEAR );
	if( m_pCamDef->GetBossEntity() && m_pCamDef->GetBossEntity()->GetSceneElement() )
	{
		obBossPosition = m_pCamDef->GetBossEntity()->GetSceneElement()->GetPosition();
	}
	else if( m_pCamDef->GetBossEntity() )
	{
		obBossPosition = m_pCamDef->GetBossEntity()->GetPosition();
	}
	CPoint obFocusPoint = (obPlayerPosition + obBossPosition) * 0.5f;

	CDirection obPlayerDir = pPlayer->GetMatrix().GetZAxis();

	CPoint obNewCameraPos = obPlayerPosition - (obPlayerDir * m_pCamDef->GetCamPlayerDistance() );

	m_obNewCamPos = obNewCameraPos;
	m_obVelBasedPos = obNewCameraPos;
	m_obLookAt = m_obNewCamLookAt = obFocusPoint;

	CCamUtil::CreateFromPoints(m_obTransform, m_obNewCamPos, m_obNewCamLookAt);
}
//------------------------------------------------------------------------------------------
//!
//!	CoolCam_GladiatorGeneral::~CoolCam_GladiatorGeneral
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CoolCam_GladiatorGeneral::~CoolCam_GladiatorGeneral()

{
}


CPoint CoolCam_GladiatorGeneral::CalcDesiredCameraPosition( CEntity& obPlayer, CPoint& obBossPosition )
{
	if( m_pCamDef->IsReverseAngleActive()==true )
	{
		CPoint obCachedPosition( CONSTRUCT_CLEAR );
		if( m_bReverseModeActivated==false || m_pCamDef->IsReverseAngleLeadInActive() )
		{
			// hold input direction
			//obPlayer.GetInputComponent()->HoldInputDirection();

			// calculate position for caching
			const CEntity* pBoss = m_pCamDef->GetBossEntity();
			ntError_p( pBoss!=0, ("Failed to get boss entity for reverse angle setup") );
			if( pBoss )
			{
				const CMatrix obBossTrans = pBoss->GetMatrix();

				if( pBoss->GetSceneElement() )
				{
					obBossPosition = pBoss->GetSceneElement()->GetPosition();
				}
				else
				{
					obBossPosition = pBoss->GetPosition();
				}

				CDirection obVertical = obBossTrans.GetYAxis();
				CDirection obForward = obBossTrans.GetZAxis();
				CDirection obRight = obBossTrans.GetXAxis();

				CDirection obReverseOffset = m_pCamDef->GetReverseAngleOffset();
				CDirection obOffset = (obRight * obReverseOffset.X()) + (obVertical * obReverseOffset.Y()) + (obForward * obReverseOffset.Z());

				m_obReverseAnglePosition = obCachedPosition = obBossPosition + obOffset;

				if( ObstructionTest( m_obReverseAnglePosition ) )
				{
					ntError_p( m_pObstructingAvoidVol!=0, ("Detected an obstruction but no volume returned") );
					CalcSafePosition( m_pObstructingAvoidVol, m_obLookAt, m_obReverseAnglePosition, obCachedPosition );
					m_obReverseAnglePosition = obCachedPosition;
				}
				m_bReverseModeActivated = true;
			}
		}
		return obCachedPosition;
	}
	else
	{
		return ImprovedCalcPos( obPlayer, obBossPosition );
	}
}


CPoint CoolCam_GladiatorGeneral::CalcDesiredCameraLookatPoint( CEntity& obPlayer, CPoint& obBossPosition )
{
	CPoint obPlayerPosition( CONSTRUCT_CLEAR );
	if( obPlayer.GetSceneElement() )
	{
		obPlayerPosition = obPlayer.GetSceneElement()->GetPosition();
	}
	else 
	{
		obPlayerPosition = obPlayer.GetPosition();
	}

	//CPoint obFocusPoint = (obPlayerPosition + obBossPosition) * 0.5f;
	CPoint obFocusPoint = ((obPlayerPosition * m_pCamDef->GetFocusSplitLambda()) + (obBossPosition * (1.0f - m_pCamDef->GetFocusSplitLambda()))); // * 0.5f;

	obFocusPoint += m_pCamDef->GetFocusWorldOffset();

	CMatrix matCamTrans = GetTransform();

	CDirection obVertical = matCamTrans.GetYAxis();
	CDirection obViewDir = matCamTrans.GetZAxis();
	CDirection obRightDir = matCamTrans.GetXAxis();

	CDirection obCamRelOffset = m_pCamDef->GetFocusCameraRelativeOffset();
	obFocusPoint += (obRightDir * obCamRelOffset.X()) + (obVertical * obCamRelOffset.Y()) + (obViewDir * obCamRelOffset.Z()); 

	return obFocusPoint;
}

void CoolCam_GladiatorGeneral::CalcSafePosition( CamVolBossAvoid* pBossAvoidVol, CPoint& obProjPoint,
												 CPoint& obTestPoint, CPoint& obResult )
{
	// direction from vol to focus point
	CDirection obFocusTestDir( obProjPoint - pBossAvoidVol->GetPosition() );
	//float fFocusTestDist = obFocusTestDir.Length();
	obFocusTestDir.Normalise();
	obFocusTestDir.Y() = 0.0f;

	// create vectors deflected each side
	CQuat obRotate( CDirection( 0.0f, 1.0f, 0.0f ), m_pCamDef->GetDeflectionFromPillar() * DEG_TO_RAD_VALUE );
	CMatrix obRotMat( obRotate );

	CDirection obRotatedProjDir1 = obFocusTestDir * obRotMat;

	// need other point - negate rotation angle
	obRotate = CQuat( CDirection( 0.0f, 1.0f, 0.0f ), -m_pCamDef->GetDeflectionFromPillar() * DEG_TO_RAD_VALUE );
	obRotMat = CMatrix( obRotate );

	CDirection obRotatedProjDir2 = obFocusTestDir * obRotMat;

	CPoint obCentrePoint = pBossAvoidVol->GetPosition() + ( pBossAvoidVol->GetRadius() * m_pCamDef->GetOffsetFromPillarVolume() * obFocusTestDir );
	CPoint obDeflected1 = pBossAvoidVol->GetPosition() + ( pBossAvoidVol->GetRadius() * m_pCamDef->GetOffsetFromPillarVolume() * obRotatedProjDir1 );
	CPoint obDeflected2 = pBossAvoidVol->GetPosition() + ( pBossAvoidVol->GetRadius() * m_pCamDef->GetOffsetFromPillarVolume() * obRotatedProjDir2 );

	// determine nearest point to test point and use as result
	float fCentreDist = CDirection( obTestPoint - obCentrePoint ).Length();
	float fDeflected1Dist = CDirection( obTestPoint - obDeflected1 ).Length();
	float fDeflected2Dist = CDirection( obTestPoint - obDeflected2 ).Length();

	obCentrePoint.Y() += m_pCamDef->GetBasePillarCameraHeight();
	obDeflected1.Y() += m_pCamDef->GetBasePillarCameraHeight();
	obDeflected2.Y() += m_pCamDef->GetBasePillarCameraHeight();

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug()==true )
	{
		g_VisualDebug->RenderLine( pBossAvoidVol->GetPosition(), obTestPoint, DC_BLACK, 0 );
		g_VisualDebug->RenderLine( pBossAvoidVol->GetPosition(), pBossAvoidVol->GetPosition() + ( 3.0f * obFocusTestDir ), DC_BLUE, 0 );
		g_VisualDebug->RenderLine( pBossAvoidVol->GetPosition(), pBossAvoidVol->GetPosition() + ( 3.0f * obRotatedProjDir1 ), DC_BLUE, 0 );
		g_VisualDebug->RenderLine( pBossAvoidVol->GetPosition(), pBossAvoidVol->GetPosition() + ( 3.0f * obRotatedProjDir2 ), DC_BLUE, 0 );

		if( m_pCamDef->GetDebugVolumeRender()==true )
		{
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY), obTestPoint, 0.4f, DC_BLACK );
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obProjPoint, 0.25f, DC_CYAN );
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obCentrePoint, 0.35f, DC_GREEN );
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obDeflected1, 0.35f, DC_GREEN );
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obDeflected2, 0.35f, DC_GREEN );
		}
	}
#endif

	if( fCentreDist<fDeflected1Dist && fCentreDist<fDeflected2Dist )
	{
		obResult = obCentrePoint;
		if( fCentreDist<m_pCamDef->GetCamFocusDistance() )
		{
			// too close to central point - pick one of the others
			//obResult.Y() = sqrtf( (m_pCamDef->GetCamFocusDistance() * m_pCamDef->GetCamFocusDistance()) - (fCentreDist * fCentreDist) );
			if( fDeflected1Dist<fDeflected2Dist )
			{
				obResult = obDeflected1;
			}
			else
			{
				obResult = obDeflected2;
			}
		}
	}
	else if( fDeflected1Dist<fCentreDist && fDeflected1Dist<fDeflected2Dist )
	{
		obResult = obDeflected1;
		if( fDeflected1Dist<m_pCamDef->GetCamFocusDistance() )
		{
			obResult.Y() = m_pCamDef->GetDeflectedHeightModifier() * sqrtf( (m_pCamDef->GetCamFocusDistance() * m_pCamDef->GetCamFocusDistance()) - (fDeflected1Dist * fDeflected1Dist) );
		}
	}
	else
	{
		obResult = obDeflected2;
		if( fDeflected2Dist<m_pCamDef->GetCamFocusDistance() )
		{
			obResult.Y() = m_pCamDef->GetDeflectedHeightModifier() * sqrtf( (m_pCamDef->GetCamFocusDistance() * m_pCamDef->GetCamFocusDistance()) - (fDeflected2Dist * fDeflected2Dist) );
		}
	}

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug()==true )
	{
		if( m_pCamDef->GetDebugVolumeRender()==true )
		{
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obResult, 0.5f, DC_RED );
		}
	}
#endif
}

bool CoolCam_GladiatorGeneral::ObstructionTest( CPoint& obCameraPos )
{
	const CoolCam_GladiatorGeneralDef::AvoidVolList& avoidVols = m_pCamDef->GetVolumeList();

	CoolCam_GladiatorGeneralDef::AvoidVolListIterConst obVolIter = avoidVols.begin();

	bool bPointObstructed = false;

	// Attempts to keep the camera outside obstruction volumes, pushing it back towards its last current good(?) position.
	// calculates the average position over all the volumes the current position intersects.
	//CPoint obCameraPosition = obCameraPos;
	CPoint obNewPosition( CONSTRUCT_CLEAR );
	CamVolBossAvoid* pAvoidVol = 0;
	m_pObstructingAvoidVol = 0;
	//int iIntersectCount = 0;
	//int iOcclusionCount = 0;
#ifdef BOSS_CAM_DBGREND
	m_iDebugTextOffset2 = 400;
#endif

	while( obVolIter!=avoidVols.end() && bPointObstructed==false )
	{
		pAvoidVol = *obVolIter;
		CDirection obVolCamDir( obCameraPos - pAvoidVol->GetPosition() );
		float fVolCamDist = obVolCamDir.Length();
		obVolCamDir.Normalise();

#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			g_VisualDebug->Printf2D( 35, float(m_iDebugTextOffset2), DC_GREY, 0, "Vol: " );
			if( fVolCamDist<pAvoidVol->GetRadius() )
			{
				g_VisualDebug->Printf2D( 90, float(m_iDebugTextOffset2), DC_RED, 0, "Camera too close" );
			}
			CPoint obResult( CONSTRUCT_CLEAR );
			if( PointInProjectedPolygon2( pAvoidVol, m_obLookAt, obCameraPos ) )
			{
				g_VisualDebug->Printf2D( 300, float(m_iDebugTextOffset2), DC_RED, 0, "Camera in shadow poly" );
			}
		}
#endif

		//CPoint obResult( CONSTRUCT_CLEAR );
		if( fVolCamDist<pAvoidVol->GetRadius() ) 
		{
			bPointObstructed = true;
			m_pObstructingAvoidVol = pAvoidVol;
		}

		if( PointInProjectedPolygon2( pAvoidVol, m_obLookAt, obCameraPos ) )
		{
			bPointObstructed = true;
			m_pObstructingAvoidVol = pAvoidVol;
		}

#ifdef BOSS_CAM_DBGREND
		m_iDebugTextOffset2 += 12;
#endif
		obVolIter++;
	}

	return bPointObstructed;
}

bool CoolCam_GladiatorGeneral::PointInProjectedPolygon2( CamVolBossAvoid* pBossAvoidVol, CPoint& obProjPointParam, CPoint& obTestPointParam )
{
	CPoint obProjPoint( obProjPointParam );
	CPoint obTestPoint( obTestPointParam );
	// remove any height values from our testing
	obProjPoint.Y() = 0.0f;
	obTestPoint.Y() = 0.0f;
	// vector from projection point to volume point
	CDirection obProjDir( pBossAvoidVol->GetPosition() - obProjPoint );
	obProjDir.Y() = 0.0f;
	// angle between projection direction and direction towards outer edge of volume
	float fProjAngleRad = atanf( pBossAvoidVol->GetRadius() / obProjDir.Length() );
	float fProjPointVolDist = obProjDir.Length();
	obProjDir.Normalise();
	// rotate projection direction by angle around vertical.
	CQuat obRotate( CDirection( 0.0f, 1.0f, 0.0f ), fProjAngleRad );
	CMatrix obRotMat( obRotate );

	CDirection obRotatedProjDir1 = obProjDir * obRotMat;
	// need other point - negate rotation angle
	obRotate = CQuat( CDirection( 0.0f, 1.0f, 0.0f ), -fProjAngleRad );
	obRotMat = CMatrix( obRotate );

	CDirection obRotatedProjDir2 = obProjDir * obRotMat;
	
	// length of projection back to edge of volume h = sqrt( adj*adj + opp*opp)

	float fProjLength = sqrtf( (pBossAvoidVol->GetRadius() * pBossAvoidVol->GetRadius()) + (fProjPointVolDist * fProjPointVolDist) );

	// Points of polygon
	CPoint aobPoly[4];
	aobPoly[0] = obProjPoint + (fProjLength * obRotatedProjDir1);
	aobPoly[1] = obProjPoint + (fProjLength * obRotatedProjDir2);

	aobPoly[2] = aobPoly[0] + (fProjLength * 3.0f * obRotatedProjDir1);
	aobPoly[3] = aobPoly[1] + (fProjLength * 3.0f * obRotatedProjDir2);


#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->RenderLine( CPoint( aobPoly[0].X(), -17.0f, aobPoly[0].Z() ), CPoint( aobPoly[2].X(), -17.0f, aobPoly[2].Z() ), DC_BLUE );
		g_VisualDebug->RenderLine( CPoint( aobPoly[2].X(), -17.0f, aobPoly[2].Z() ), CPoint( aobPoly[3].X(), -17.0f, aobPoly[3].Z() ), DC_BLUE );
		g_VisualDebug->RenderLine( CPoint( aobPoly[3].X(), -17.0f, aobPoly[3].Z() ), CPoint( aobPoly[1].X(), -17.0f, aobPoly[1].Z() ), DC_BLUE );
		g_VisualDebug->RenderLine( CPoint( aobPoly[1].X(), -17.0f, aobPoly[1].Z() ), CPoint( aobPoly[0].X(), -17.0f, aobPoly[0].Z() ), DC_BLUE );
	}
#endif

	// create edge vectors
	CDirection aobEdges[4];
	aobEdges[0] = CDirection( aobPoly[2] - aobPoly[0] );
	aobEdges[1] = CDirection( aobPoly[3] - aobPoly[2] );
	aobEdges[2] = CDirection( aobPoly[1] - aobPoly[3] );
	aobEdges[3] = CDirection( aobPoly[0] - aobPoly[1] );

	// vectors from test point to edge points
	CDirection aobPointTests[4];
	aobPointTests[0] = CDirection( aobPoly[0] - obTestPoint );
	aobPointTests[1] = CDirection( aobPoly[2] - obTestPoint );
	aobPointTests[2] = CDirection( aobPoly[3] - obTestPoint );
	aobPointTests[3] = CDirection( aobPoly[1] - obTestPoint );

	// do dot products
	float fResult = 0.0f;
	for( int iCount=0; iCount<4; iCount++ )
	{
		aobEdges[iCount].Normalise();
		aobPointTests[iCount].Normalise();
		fResult = aobEdges[iCount].Dot( aobPointTests[iCount] );

#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			g_VisualDebug->Printf2D( 450 + (float(iCount) * 70.0f), float(m_iDebugTextOffset2), DC_CYAN, 0, "%.3f", fResult );
		}
#endif
		if( fResult>0.0f )
		{
			return false;
		}
	}

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->RenderLine( CPoint( aobPoly[0].X(), -17.0f, aobPoly[0].Z() ), CPoint( aobPoly[3].X(), -17.0f, aobPoly[2].Z() ), DC_RED );
		g_VisualDebug->RenderLine( CPoint( aobPoly[2].X(), -17.0f, aobPoly[2].Z() ), CPoint( aobPoly[1].X(), -17.0f, aobPoly[3].Z() ), DC_RED );
		g_VisualDebug->RenderLine( CPoint( aobPoly[3].X(), -17.0f, aobPoly[3].Z() ), CPoint( aobPoly[0].X(), -17.0f, aobPoly[1].Z() ), DC_RED );
		g_VisualDebug->RenderLine( CPoint( aobPoly[1].X(), -17.0f, aobPoly[1].Z() ), CPoint( aobPoly[2].X(), -17.0f, aobPoly[0].Z() ), DC_RED );
	}
#endif
	return true;
}

CPoint CoolCam_GladiatorGeneral::ImprovedCalcPos( CEntity& obPlayer, CPoint& obBossPosition )
{
	//m_bReverseModeActivated = false;
	CMatrix matCamTrans = GetTransform();

	CDirection obVertical = matCamTrans.GetYAxis();
	CDirection obViewDir = matCamTrans.GetZAxis();
	CDirection obRightDir = matCamTrans.GetXAxis();

	CPoint obPlayerPosition = obPlayer.GetPosition();

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 20, DC_GREEN, 0, "Cam vert: %f, %f, %f", obVertical.X(), obVertical.Y(), obVertical.Z() );
		g_VisualDebug->Printf2D( 885, 32, DC_GREEN, 0, "Cam right: %f, %f, %f", obRightDir.X(), obRightDir.Y(), obRightDir.Z() );
		g_VisualDebug->Printf2D( 885, 44, DC_GREEN, 0, "Cam dir: %f, %f, %f", obViewDir.X(), obViewDir.Y(), obViewDir.Z() );
	}
#endif

	CPoint rectPointsPlayer[4];
	rectPointsPlayer[0] = obPlayerPosition + (obRightDir * 0.65f); // bottom left
	rectPointsPlayer[1] = obPlayerPosition - (obRightDir * 0.65f); // bottom right

	rectPointsPlayer[2] = obPlayerPosition + (obVertical * 1.7f) + (obRightDir * 0.65f); // top left
	rectPointsPlayer[3] = obPlayerPosition + (obVertical * 1.7f) - (obRightDir * 0.65f); // top right

	CPoint obBossPositionRoot = m_pCamDef->GetBossEntity()->GetPosition();
	CPoint rectPointsBoss[4];
	rectPointsBoss[0] = obBossPositionRoot + (obRightDir * 0.65f); 
	rectPointsBoss[1] = obBossPositionRoot - (obRightDir * 0.65f);
	
	rectPointsBoss[2] = obBossPositionRoot + (obVertical * 2.4f) + (obRightDir * 0.65f);
	rectPointsBoss[3] = obBossPositionRoot + (obVertical * 2.4f) - (obRightDir * 0.65f);

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), rectPointsPlayer[0], 0.25, DC_RED );
		g_VisualDebug->RenderLine( rectPointsPlayer[0], rectPointsPlayer[1], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPointsPlayer[1], rectPointsPlayer[3], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPointsPlayer[3], rectPointsPlayer[2], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPointsPlayer[2], rectPointsPlayer[0], DC_YELLOW );
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), rectPointsPlayer[3], 0.25, DC_GREEN );

		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), rectPointsBoss[0], 0.25, DC_BLUE );
		g_VisualDebug->RenderLine( rectPointsBoss[0], rectPointsBoss[1], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPointsBoss[1], rectPointsBoss[3], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPointsBoss[3], rectPointsBoss[2], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPointsBoss[2], rectPointsBoss[0], DC_YELLOW );
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), rectPointsBoss[3], 0.25, DC_CYAN );
	}
#endif

	CMatrix matProj;
	matProj.Perspective( m_fFOV, GetView().GetAspectRatio(), GetView().GetZNear(), GetView().GetZFar() );

	CMatrix affineInverse = matCamTrans.GetAffineInverse();

	CVector projPointsPlayer[4];
	CVector projPointsBoss[4];
#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 550, DC_GREEN, 0, "Player:" );
	}
#endif
	for( int count=0; count<4; count++ )
	{
		// Get Cam Space
		projPointsPlayer[count] = CVector( rectPointsPlayer[count] * affineInverse );

		// Get Projection Space
		projPointsPlayer[count].W() = 0.f;
		projPointsPlayer[count] = projPointsPlayer[count] * matProj;

		// Deal with points at infinity
		if(projPointsPlayer[count].W() < EPSILON)
		{
			projPointsPlayer[count] = CVector( 0.f,0.f,-1.f,1.f );
		}
		else
		{
			projPointsPlayer[count] /= fabsf( projPointsPlayer[count].W() );
		}
		//projPointsPlayer[count].X() *= -1.0f;
#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			g_VisualDebug->Printf2D( 885.0f, 562.0f + (count * 12.0f), DC_GREEN, 0, "point %d: (%.3f, %.3f, %.3f)", count, projPointsPlayer[count].X(), projPointsPlayer[count].Y(), projPointsPlayer[count].Z() );
		}
#endif
		projPointsPlayer[count] += CVector( 1.0f, 1.0f, 0.0f, 0.0f );
		//projPointsPlayer[count] *= 300.0f;
	}

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 610, DC_GREEN, 0, "Boss:" );
	}
#endif
	for( int count=0; count<4; count++ )
	{
		// Get Cam Space
		projPointsBoss[count] = CVector( rectPointsBoss[count] * affineInverse );

		// Get Projection Space
		projPointsBoss[count].W() = 0.f;
		projPointsBoss[count] = projPointsBoss[count] * matProj;

		// Deal with points at infinity
		if(projPointsBoss[count].W() < EPSILON)
		{
			projPointsBoss[count] = CVector( 0.f,0.f,-1.f,1.f );
		}
		else
		{
			projPointsBoss[count] /= fabsf( projPointsBoss[count].W() );
		}
		//projPointsBoss[count].X() *= -1.0f;
#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			g_VisualDebug->Printf2D( 885.0f, 622.0f + (count * 12.0f), DC_GREEN, 0, "point %d: (%.3f, %.3f, %.3f)", count, projPointsBoss[count].X(), projPointsBoss[count].Y(), projPointsBoss[count].Z() );
		}
#endif
		projPointsBoss[count] += CVector( 1.0f, 1.0f, 0.0f, 0.0f );
		//projPointsBoss[count] *= 300.0f;
	}

	bool bHorizTest1 = ( (projPointsPlayer[0].X() < projPointsBoss[0].X()) && (projPointsPlayer[0].X() > projPointsBoss[3].X()) );
	bool bHorizTest2 = ( (projPointsPlayer[3].X() < projPointsBoss[0].X()) && (projPointsPlayer[3].X() > projPointsBoss[3].X()) );

	bool bVertTest1 = ( (projPointsPlayer[0].Y() < projPointsBoss[0].Y()) && (projPointsPlayer[0].Y() > projPointsBoss[3].Y()) );
	bool bVertTest2 = ( (projPointsPlayer[3].Y() < projPointsBoss[0].Y()) && (projPointsPlayer[3].Y() > projPointsBoss[3].Y()) );


	bool bHorizTest3 = ( (projPointsBoss[0].X() < projPointsPlayer[0].X()) && (projPointsBoss[0].X() > projPointsPlayer[3].X()) );
	bool bHorizTest4 = ( (projPointsBoss[3].X() < projPointsPlayer[0].X()) && (projPointsBoss[3].X() > projPointsPlayer[3].X()) );

	bool bVertTest3 = ( (projPointsBoss[0].Y() < projPointsPlayer[0].Y()) && (projPointsBoss[0].Y() > projPointsPlayer[3].Y()) );
	bool bVertTest4 = ( (projPointsBoss[3].Y() < projPointsPlayer[0].Y()) && (projPointsBoss[3].Y() > projPointsPlayer[3].Y()) );


	bool bOverlap = (bHorizTest1 && bVertTest1) || (bHorizTest1 && bVertTest2) || (bHorizTest2 && bVertTest2) || (bHorizTest2 && bVertTest1) ||
					(bHorizTest3 && bVertTest3) || (bHorizTest3 && bVertTest4) || (bHorizTest4 && bVertTest4) || (bHorizTest4 && bVertTest3);
	if( bOverlap )
	{
#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			g_VisualDebug->Printf2D( 550.0f, 380.0f, DC_RED, 0, "OVERLAP OVERLAP OVERLAP OVERLAP OVERLAP" );
		}
#endif
		m_bCameraDragActive = true;
	}

	CPoint obPlayerPositionProper( CONSTRUCT_CLEAR );
	if( obPlayer.GetSceneElement() )
	{
		obPlayerPositionProper = obPlayer.GetSceneElement()->GetPosition();
	}
	else 
	{
		obPlayerPositionProper = obPlayer.GetPosition();
	}

	CDirection obPlayerToBoss(obPlayerPosition - obBossPosition);
	float fPlayerBossSeperation = obPlayerToBoss.Length();
	obPlayerToBoss.Normalise();

	CPoint newCamPos( CONSTRUCT_CLEAR );
	CPoint obFocusPoint = ((obPlayerPositionProper * m_pCamDef->GetFocusSplitLambda()) + (obBossPosition * (1.0f - m_pCamDef->GetFocusSplitLambda()))); // * 0.5f;

	if( fPlayerBossSeperation<m_pCamDef->GetCameraModeSwitchDistance() )
	{

		CPoint obCameraPosition( m_obNewCamPos );
		//CDirection obCamToPlayer( obCameraPosition - obPlayerPosition );
		CDirection obFocusToPlayer( obPlayerPositionProper - obFocusPoint );
		float fPlayerFocusDist = obFocusToPlayer.Length();
		obFocusToPlayer.Normalise();
		obFocusToPlayer.Y() = 0.0f;

		// rotate camera to player direction by offset angle to get camera position
		CQuat offsetRot( CDirection( 0.0f, 1.0f, 0.0f),  m_fCameraAngleMod * m_pCamDef->GetCameraOffsetAngle() * DEG_TO_RAD_VALUE );
		CMatrix offsetMat( offsetRot );

		CDirection camPosDir = obFocusToPlayer * offsetMat;

#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obFocusPoint, 0.4f, DC_GREEN );
			g_VisualDebug->RenderLine( obFocusPoint, obFocusPoint + (obFocusToPlayer * 4.0f), DC_CYAN );			
			g_VisualDebug->RenderLine( obFocusPoint, obFocusPoint + (camPosDir * 4.0f), DC_BLACK );			
		}
#endif
		//CDirection obVerticalOffset( 0.0f, m_pCamDef->GetVerticalOffset(), 0.0f );
		CDirection obProxOffset = m_pCamDef->GetProximityOffset();
		CDirection obOffset = CDirection( 0.0f, obProxOffset.Y(), 0.0f ) + (obRightDir * obProxOffset.X()) + (obViewDir * obProxOffset.Z());
		newCamPos = obFocusPoint + (camPosDir * (m_pCamDef->GetCameraPullbackBase() + (fPlayerFocusDist * m_pCamDef->GetCameraPullbackMultiplier()))) + obOffset;
	}
	else
	{
		m_obFocusToCameraDir = obPlayerToBoss.Cross( CDirection( 0.0f, 1.0f, 0.0f ) );
		CQuat obAngleModify( CDirection( 0.0f, 1.0f, 0.0f ), m_pCamDef->GetCameraAngleModifierDeg() * DEG_TO_RAD_VALUE );
		CMatrix obAngleModifyMat( obAngleModify );
		m_obFocusToCameraDir = m_obFocusToCameraDir * obAngleModifyMat;

		m_obFocusToCameraDir.Y() = 0.0f;
		CDirection obDistOffset = m_pCamDef->GetDistanceOffset();
		CDirection obOffset = CDirection( 0.0f, obDistOffset.Y(), 0.0f ) + (obRightDir * obDistOffset.X()) + (obViewDir * obDistOffset.Z());
		newCamPos = obPlayerPositionProper + (m_obFocusToCameraDir * m_pCamDef->GetCamPlayerDistance()) + obOffset;
	}

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		if( m_pCamDef->GetDebugVolumeRender() )
		{
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obTransform.GetTranslation(), 1.0f, 0x00ff0000 );
		}
		g_VisualDebug->RenderLine( m_obTransform.GetTranslation(), obFocusPoint, 0x00ff0000 );

		CDirection obFrustumCentre( obFocusPoint - m_obTransform.GetTranslation() );
		
		CQuat obQuatRot( CDirection( 0.0f, 1.0f, 0.0f ), m_fFOV * DEG_TO_RAD_VALUE );
		CMatrix obRot( obQuatRot );
		CDirection obFrustumRight = obFrustumCentre * obRot;
		g_VisualDebug->RenderLine( m_obTransform.GetTranslation(), m_obTransform.GetTranslation() + obFrustumRight, 0x00ff0000 );

		obQuatRot = CQuat( CDirection( 0.0f, 1.0f, 0.0f ), -m_fFOV * DEG_TO_RAD_VALUE );
		obRot = CMatrix( obQuatRot );
		CDirection obFrustumLeft = obFrustumCentre * obRot;
		g_VisualDebug->RenderLine( m_obTransform.GetTranslation(), m_obTransform.GetTranslation() + obFrustumLeft, 0x00ff0000 );

		g_VisualDebug->Printf2D( 35, 55, DC_GREEN, 0, "FOV: %f\n", m_fFOV );
		g_VisualDebug->Printf2D( 35, 67, DC_PURPLE, 0, "Player->Boss Dist: %f\n", fPlayerBossSeperation );
	}

#endif

	//* OBSTRUCTION TEST CODE
	if( (m_pCamDef->GetObstructionCheck()==true) && (m_fTimeSinceObstructionCameraCut>m_pCamDef->GetObstructionCamCutTimeLimit()) 
		&& (ObstructionTest( newCamPos )==true) )
	{
		m_fCameraAngleMod *= -1.0f;
		
		if( m_pCamDef->GetObstructionCameraCut()==true )
		{
			m_bOcclusionCut = true;
		}
		
		if( fPlayerBossSeperation<m_pCamDef->GetCameraModeSwitchDistance() )
		{

			CPoint obCameraPosition( m_obNewCamPos );
			//CDirection obCamToPlayer( obCameraPosition - obPlayerPosition );
			CDirection obFocusToPlayer( obPlayerPositionProper - obFocusPoint );
			float fPlayerFocusDist = obFocusToPlayer.Length();
			obFocusToPlayer.Normalise();
			obFocusToPlayer.Y() = 0.0f;

			// rotate camera to player direction by offset angle to get camera position
			CQuat offsetRot( CDirection( 0.0f, 1.0f, 0.0f),  m_fCameraAngleMod * m_pCamDef->GetCameraOffsetAngle() * DEG_TO_RAD_VALUE );
			CMatrix offsetMat( offsetRot );

			CDirection camPosDir = obFocusToPlayer * offsetMat;

#ifdef BOSS_CAM_DBGREND
			if( m_pCamDef->GetRenderDebug() )
			{
				g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obFocusPoint, 0.4f, DC_GREEN );
				g_VisualDebug->RenderLine( obFocusPoint, obFocusPoint + (obFocusToPlayer * 4.0f), DC_CYAN );			
				g_VisualDebug->RenderLine( obFocusPoint, obFocusPoint + (camPosDir * 4.0f), DC_BLACK );			
			}
#endif
			//CDirection obVerticalOffset( 0.0f, m_pCamDef->GetVerticalOffset(), 0.0f );
			CDirection obProxOffset = m_pCamDef->GetProximityOffset();
			CDirection obOffset = CDirection( 0.0f, obProxOffset.Y(), 0.0f ) + (obRightDir * obProxOffset.X()) + (obViewDir * obProxOffset.Z());
			newCamPos = obFocusPoint + (camPosDir * (m_pCamDef->GetCameraPullbackBase() + (fPlayerFocusDist * m_pCamDef->GetCameraPullbackMultiplier()))) + obOffset;
		}
		else
		{
			m_obFocusToCameraDir = obPlayerToBoss.Cross( CDirection( 0.0f, 1.0f, 0.0f ) );
			CQuat obAngleModify( CDirection( 0.0f, 1.0f, 0.0f ), m_pCamDef->GetCameraAngleModifierDeg() * DEG_TO_RAD_VALUE );
			CMatrix obAngleModifyMat( obAngleModify );
			m_obFocusToCameraDir = m_obFocusToCameraDir * obAngleModifyMat;

			m_obFocusToCameraDir.Y() = 0.0f;
			CDirection obDistOffset = m_pCamDef->GetDistanceOffset();
			CDirection obOffset = CDirection( 0.0f, obDistOffset.Y(), 0.0f ) + (obRightDir * obDistOffset.X()) + (obViewDir * obDistOffset.Z());
			newCamPos = obPlayerPositionProper + (m_obFocusToCameraDir * m_pCamDef->GetCamPlayerDistance()) + obOffset;
		}
		
	}
	//* END OBSTRUCTION TEST CODE

	return newCamPos;

}
