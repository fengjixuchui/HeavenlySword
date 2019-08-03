//------------------------------------------------------------------------------------------
//!
//!	\file CamCool_kingBossFight.cpp
//!
//------------------------------------------------------------------------------------------

#include "camera/coolcam_kingbossfight.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "objectdatabase/dataobject.h"
#include "game/entityboss.h"
#include "game/entitymanager.h"
#include "camera/camutils.h"
#include "core/visualdebugger.h"
#include "gfx/graphing.h"
#include "camera/camview.h"

START_CHUNKED_INTERFACE( CoolCam_KingBossFightDef, Mem::MC_CAMERA)
	DEFINE_INTERFACE_INHERITANCE(CoolCam_RootBossDef)
	COPY_INTERFACE_FROM(CoolCam_RootBossDef)

	PUBLISH_VAR_AS( m_fMinCameraHeight, MinimumCameraHeight )
	PUBLISH_VAR_AS( m_fCameraVerticalModifier, CameraVerticalModifier )
	PUBLISH_VAR_AS( m_fFocusOffsetModifier, FocusOffsetModifier )
	PUBLISH_VAR_AS( m_fBossHeightAbovePlayerScaler, BossHeightAbovePlayerScaler )
END_STD_INTERFACE

CoolCam_KingBossFightDef::CoolCam_KingBossFightDef( void )
: CoolCam_RootBossDef(),
  m_fMinCameraHeight(1.0f),
  m_fCameraVerticalModifier(1.0f),
  m_fFocusOffsetModifier(0.1f),
  m_fBossHeightAbovePlayerScaler(1.0f)
{
}

CoolCam_KingBossFight* CoolCam_KingBossFightDef::Create( const CamView& view )
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_KingBossFight( view, this );
}

CoolCam_KingBossFight::CoolCam_KingBossFight( const CamView& view, const CoolCam_KingBossFightDef* pCamDef )
: CoolCam_RootBoss( view, pCamDef ),
  m_pCamDef(pCamDef),
  m_fSeperationDistanceModifier(1.0f),
  m_obFocusToCameraDir( CONSTRUCT_CLEAR ),
  m_obCamVerticalOffset( CONSTRUCT_CLEAR ),
  m_fCameraPullbackMultiplier(1.0f),
  m_obFocusVerticalOffset( CONSTRUCT_CLEAR )
{
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

	CPoint obNewCameraPos = obPlayerPosition - (obPlayerDir * m_pCamDef->GetCamPlayerDistance());

	m_obNewCamPos = obNewCameraPos;
	//m_obVelBasedPos = m_obTransform.GetTranslation();
	m_obVelBasedPos = obNewCameraPos;
	m_obNewCamLookAt = obFocusPoint;

	CCamUtil::CreateFromPoints(m_obTransform, m_obNewCamPos, m_obNewCamLookAt);
}

CoolCam_KingBossFight::~CoolCam_KingBossFight( void )
{
}

CPoint CoolCam_KingBossFight::CalcDesiredCameraPosition( CEntity& obPlayer, CPoint& obBossPosition )
{
	//CPoint obPlayerPosition(obPlayer.GetPosition());

	DebugTestRender();


	CPoint obPlayerPosition( CONSTRUCT_CLEAR );
	if( obPlayer.GetSceneElement() )
	{
		obPlayerPosition = obPlayer.GetSceneElement()->GetPosition();
	}
	else 
	{
		obPlayerPosition = obPlayer.GetPosition();
	}

	CPoint obCameraPosition( m_obNewCamPos );

	CDirection obCamToPlayer( obCameraPosition - obPlayerPosition );
	CDirection obCamToBoss( obCameraPosition - obBossPosition );
	//float fCamToPlayerDist = obCamToPlayer.Length();
	obCamToPlayer.Normalise();
	obCamToBoss.Normalise();

	float fCamToBossDotCamToPlayer = obCamToBoss.Dot( obCamToPlayer );
	float fCamToBossCamToPlayerAngleDeg = acosf( fCamToBossDotCamToPlayer ) * RAD_TO_DEG_VALUE;
	UNUSED(fCamToBossCamToPlayerAngleDeg);

	CDirection obPlayerToBoss(obPlayerPosition - obBossPosition);
	float fPlayerBossSeperation = obPlayerToBoss.Length();
	float fPlayerToBossVerticalDiff = fabsf( obPlayerToBoss.Y() );
	obPlayerToBoss.Normalise();

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 32, DC_WHITE, 0, "Player->Boss Height difference: %f", fPlayerToBossVerticalDiff );
	}
#endif


	CPoint obFocusPoint = (obPlayerPosition + obBossPosition) * 0.5f;

	CDirection obPlayerDir = obPlayer.GetMatrix().GetZAxis();

	CPoint obNewCameraPos = obPlayerPosition - (obPlayerDir * m_pCamDef->GetCamPlayerDistance() );

	CDirection obCamFocusDir(obCameraPosition - obFocusPoint);

/*	if( obCamFocusDir.LengthSquared()<m_pCamDef->GetMinCameraFocusDistSqrd() )
	{
		// push camera back away from focus point
		// direction
		// dot product between direction from player to boss and camera to focus point.
		obCamFocusDir.Normalise();
		float fPlayerBossVsCamFocusDot = obCamFocusDir.Dot( obPlayerToBoss );
		if( fPlayerBossVsCamFocusDot<0.15 )
		{
			// relatively perpendicular to centre line
			// push back in same direction
			obNewCameraPos = obNewCameraPos + (obCamFocusDir * m_pCamDef->GetMinCameraFocusDist() );
		}
	}
*/
	// calc camera distance based on ideal seperation angle between boss and player
	float fCameraDistance = fPlayerBossSeperation / 2.0f * tanf( m_pCamDef->GetBossHeroSeperationAngleDeg() * 0.5f * DEG_TO_RAD_VALUE );

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 80, DC_RED, 0, "Raw Camera Distance: %f", fCameraDistance );
	}
#endif

/*
	if( fCamToBossCamToPlayerAngleDeg>(m_pCamDef->GetBossHeroSeperationAngleDeg() + m_pCamDef->GetDistanceToleranceZone() ) )
	{
		m_fSeperationDistanceModifier += m_pCamDef->GetSeperationDistModifierIncrement();
	}
	else if( (fCamToBossCamToPlayerAngleDeg<(m_pCamDef->GetBossHeroSeperationAngleDeg() - m_pCamDef->GetDistanceToleranceZone())) && (m_fSeperationDistanceModifier>1.0f) )
	{
        m_fSeperationDistanceModifier -= m_pCamDef->GetSeperationDistModifierIncrement();
	}
*/
#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 92, DC_RED, 0, "sep distance modifier: %f", m_fSeperationDistanceModifier );
	}
#endif

	//fCameraDistance *= m_fSeperationDistanceModifier;

	m_obFocusToCameraDir = obPlayerToBoss.Cross( CDirection( 0.0f, 1.0f, 0.0f ) );
	CQuat obAngleModify( CDirection( 0.0f, 1.0f, 0.0f ), m_pCamDef->GetCameraAngleModifierDeg() * DEG_TO_RAD_VALUE );
	CMatrix obAngleModifyMat( obAngleModify );
	m_obFocusToCameraDir = m_obFocusToCameraDir * obAngleModifyMat;

	// modify desired distance between camera and focus point based on distance between player and boss.
	float fCamToFocusDistance = m_pCamDef->GetCamFocusDistance();
	//float fCamToFocusDifference = fCurrentCamFocusDir - m_pCamDef->GetCamFocusDistance();

#ifdef BOSS_CAM_DBGREND
	float fCurrentCamFocusDir = obCamFocusDir.Length();
	float fCamToFocusRatio = fCurrentCamFocusDir / m_pCamDef->GetCamFocusDistance();
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 104, DC_YELLOW, 0, "Focus Distance Ratio: %f", fCamToFocusRatio );
	}
#endif
	// max 1.75
//	if( (fCamToFocusRatio>m_pCamDef->GetMaxCamFocusRatio()) && (fCamToFocusDistance<fPlayerBossSeperation) && (fCamToPlayerDist<m_pCamDef->GetCamPlayerDistance() * 2.0f)  )
//	{
//		//fCamToFocusDistance *= fCamToFocusRatio;
//		m_fCameraPullbackMultiplier += m_pCamDef->GetCameraPullbackModifier();
//	}
//	// min 1.5
//	else if( fCamToFocusRatio<m_pCamDef->GetMinCamFocusRatio() )
//	{
//		m_fCameraPullbackMultiplier -= m_pCamDef->GetCameraPullbackModifier();
//	}
//
//#ifdef BOSS_CAM_DBGREND
//	if( m_pCamDef->GetRenderDebug() )
//	{
//		g_VisualDebug->Printf2D( 885, 116, DC_YELLOW, 0, "Pullback multipler: %f", m_fCameraPullbackMultiplier );
//	}
//#endif
//
//	if( m_fCameraPullbackMultiplier<1.0f )
//	{
//		m_fCameraPullbackMultiplier = 1.0f;
//	}

	//fCamToFocusDistance *= m_fCameraPullbackMultiplier;
#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 128, DC_GREEN, 0, "calc'd cam focus dist: %f", fCamToFocusDistance );
	}
#endif


	m_fCameraDistance = fCameraDistance + fCamToFocusDistance;
#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 140, DC_RED, 0, "final camera dist: %f", m_fCameraDistance );
	}
#endif


	// modify camera height based on height of boss - push camera down the higher the boss is.
	if( fPlayerToBossVerticalDiff>1.75f )
	{
		m_obCamVerticalOffset.Y() -= m_pCamDef->GetCameraVerticalModifier();
	}
	else if( (fPlayerToBossVerticalDiff<1.6f) )  //&& (fPlayerToBossVerticalDiff>1.0f) )
	{
		//m_obCamVerticalOffset.Y() += m_pCamDef->GetCameraVerticalModifier();
		m_obCamVerticalOffset.Y() = 0.0f;
	}

	if( m_obCamVerticalOffset.Y()>0.0f )
	{
		m_obCamVerticalOffset.Y() = 0.0f;
	}

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 44, DC_WHITE, 0, "Vertical offset: %f", m_obCamVerticalOffset.Y() );
	}
#endif

	float fCameraDistModifier = 0.0f;
	if( fPlayerBossSeperation<(fPlayerToBossVerticalDiff * 1.5f) )
	{
		if( fPlayerBossSeperation>0.0f )
		{
			fCameraDistModifier = fPlayerBossSeperation * (fPlayerToBossVerticalDiff / fPlayerBossSeperation) * m_pCamDef->GetBossHeightAbovePlayerScaler();
		}
		else
		{
			fCameraDistModifier = fPlayerBossSeperation  * m_pCamDef->GetBossHeightAbovePlayerScaler();
		}
	}
#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 825, 180, DC_CYAN, 0, "fPlayerToBossVerticalDiff / fPlayerBossSeperation: %f", (fPlayerToBossVerticalDiff  / fPlayerBossSeperation) );
		g_VisualDebug->Printf2D( 885, 192, DC_CYAN, 0, "Cam distance modifier: %f", fCameraDistModifier );
	}
#endif



	m_obFocusToCameraDir.Y() = 0.0f;

	CPoint obOtherCamPos( CONSTRUCT_CLEAR );
	if( (fPlayerBossSeperation/2.0f)>m_pCamDef->GetCamFocusDistance() )
	{
		obOtherCamPos = obPlayerPosition + (m_obFocusToCameraDir * (m_pCamDef->GetCamPlayerDistance() + fCameraDistModifier)) + m_obCamVerticalOffset;
	}
	else
	{
		obOtherCamPos = obFocusPoint + (m_obFocusToCameraDir * (m_fCameraDistance + fCameraDistModifier)) + m_obCamVerticalOffset;
	}


#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->Printf2D( 885, 56, DC_WHITE, 0, "Camera Pos: (%f,%f,%f)", obOtherCamPos.X(), obOtherCamPos.Y(), obOtherCamPos.Z() );
	}
#endif
/*
	if( (fCamToBossCamToPlayerAngleDeg)>(m_fFOV*2.0f) )
	{
		if( fCamToBossCamToPlayerAngleDeg>(m_pCamDef->GetMaxFOV() * 2.0f) )
		{
			fCamToBossCamToPlayerAngleDeg = (m_pCamDef->GetMaxFOV() * 2.0f);
		}
		// hold distance and increase FOV
		m_bHoldCameraDistance = true;
//		m_fCameraDistance = fCameraDistance + m_pCamDef->GetFocusToCamIdealDist();
		m_fFOV = fCamToBossCamToPlayerAngleDeg * 0.5f;

	}
	else
	{
		m_bHoldCameraDistance = false;
	}

	if( m_fFOV<m_pCamDef->GetMinFOV() )
	{
		m_fFOV = m_pCamDef->GetMinFOV();
	}
*/

	if( m_bHoldCameraDistance==true )
	{
		obOtherCamPos = obFocusPoint + (m_obFocusToCameraDir * m_fCameraDistance) + m_obCamVerticalOffset;
	}

	//obOtherCamPos = DetermineObstructedPosition( obOtherCamPos );

	if( obOtherCamPos.Y()<(obPlayerPosition.Y() - m_pCamDef->GetMinCameraHeight() ) )
	{
		obOtherCamPos.Y() = (obPlayerPosition.Y() - m_pCamDef->GetMinCameraHeight() );
	}


#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		if( m_pCamDef->GetDebugVolumeRender() )
		{
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obOtherCamPos, 1.0f, 0x00ff0000 );
		}
		g_VisualDebug->RenderLine( obOtherCamPos, obFocusPoint, 0x00ff0000 );

		CDirection obFrustumCentre( obFocusPoint - obOtherCamPos );
		
		CQuat obQuatRot( CDirection( 0.0f, 1.0f, 0.0f ), m_fFOV * DEG_TO_RAD_VALUE );
		CMatrix obRot( obQuatRot );
		CDirection obFrustumRight = obFrustumCentre * obRot;
		g_VisualDebug->RenderLine( obOtherCamPos, obOtherCamPos + obFrustumRight, 0x00ff0000 );

		obQuatRot = CQuat( CDirection( 0.0f, 1.0f, 0.0f ), -m_fFOV * DEG_TO_RAD_VALUE );
		obRot = CMatrix( obQuatRot );
		CDirection obFrustumLeft = obFrustumCentre * obRot;
		g_VisualDebug->RenderLine( obOtherCamPos, obOtherCamPos + obFrustumLeft, 0x00ff0000 );

		g_VisualDebug->Printf2D( 35, 55, DC_GREEN, 0, "FOV: %f\n", m_fFOV );
		g_VisualDebug->Printf2D( 35, 67, DC_PURPLE, 0, "Player->Boss Dist: %f\n", fPlayerBossSeperation );
		g_VisualDebug->Printf2D( 35, 79, DC_CYAN, 0, "Focus->Cam Dist: %f (Total: %f)\n", fCameraDistance, m_fCameraDistance );
		g_VisualDebug->Printf2D( 35, 91, DC_RED, 0, "Angle at Camera: %f (Desired: %f)\n", fCamToBossCamToPlayerAngleDeg, m_pCamDef->GetBossHeroSeperationAngleDeg() );
		
		if( m_bHoldCameraDistance )
		{
			g_VisualDebug->Printf2D( 35, 103, DC_WHITE, 0, "Holding camera distance: %f\n", m_fCameraDistance );
		}

		g_VisualDebug->Printf2D( 885, 68, DC_GREEN, 0, "Final Camera Pos: (%f,%f,%f)", obOtherCamPos.X(), obOtherCamPos.Y(), obOtherCamPos.Z() );
	}

#endif

	return obOtherCamPos;
}

CPoint CoolCam_KingBossFight::CalcDesiredCameraLookatPoint( CEntity& obPlayer, CPoint& obBossPosition )
{
	//CPoint obPlayerPosition(obPlayer.GetPosition());
	CPoint obPlayerPosition( CONSTRUCT_CLEAR );
	if( obPlayer.GetSceneElement() )
	{
		obPlayerPosition = obPlayer.GetSceneElement()->GetPosition();
	}
	else
	{
		obPlayerPosition = obPlayer.GetPosition();
	}

	CDirection obPlayerToBoss(obPlayerPosition - obBossPosition);
	float fPlayerBossSeperation = obPlayerToBoss.Length();
	float fPlayerToBossVerticalDiff = obPlayerToBoss.Y();
	//obPlayerToBoss.Normalise();

	if( fPlayerBossSeperation<fabsf(fPlayerToBossVerticalDiff * 1.5f) )
	{
		m_obFocusVerticalOffset.Y() = fPlayerToBossVerticalDiff * m_pCamDef->GetFocusOffsetModifier();		
#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			if( m_pCamDef->GetDebugVolumeRender() )
			{
				g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obBossPosition, 1.0f, DC_WHITE );
			}
			g_VisualDebug->Printf2D( 825, 220, DC_WHITE, 0, "Boss player distance: %f", fPlayerBossSeperation );
			g_VisualDebug->Printf2D( 825, 232, DC_WHITE, 0, "fPlayerToBossVerticalDiff / fPlayerBossSeperation: %f", (fPlayerToBossVerticalDiff  / fPlayerBossSeperation) );
			g_VisualDebug->Printf2D( 885, 244, DC_WHITE, 0, "Vertical offset: %f", m_obFocusVerticalOffset.Y() );
		}
#endif	
	}
	else
	{
		m_obFocusVerticalOffset.Y() = fPlayerToBossVerticalDiff / 3.0f;
	}
	CPoint obFocusPoint = m_obFocusVerticalOffset + ((obPlayerPosition + obBossPosition) * 0.5f);

	return obFocusPoint;
}

void CoolCam_KingBossFight::DebugTestRender( void ) const
{
#ifndef _GOLD_MASTER

	CMatrix matCamTrans = GetTransform();

	CDirection obVertical = matCamTrans.GetYAxis();
	CDirection obViewDir = matCamTrans.GetZAxis();
	CDirection obRightDir = matCamTrans.GetXAxis();

	CPoint obPlayerPosition( CONSTRUCT_CLEAR );
	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
	obPlayerPosition = pobPlayer->GetPosition();

	CPoint rectPoints[4];
	rectPoints[0] = obPlayerPosition - (obRightDir * 0.8f); // bottom left
	rectPoints[1] = obPlayerPosition + (obRightDir * 0.8f); // bottom right

	rectPoints[2] = obPlayerPosition + (obVertical * 1.7f) - (obRightDir * 0.8f); // top left
	rectPoints[3] = obPlayerPosition + (obVertical * 1.7f) + (obRightDir * 0.8f); // top right

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), rectPoints[0], 0.25, DC_RED );
		g_VisualDebug->RenderLine( rectPoints[0], rectPoints[1], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPoints[1], rectPoints[3], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPoints[3], rectPoints[2], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPoints[2], rectPoints[0], DC_YELLOW );
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), rectPoints[3], 0.25, DC_GREEN );
	}
#endif

	CMatrix matProj;
	matProj.Perspective( m_fFOV, GetView().GetAspectRatio(), GetView().GetZNear(), GetView().GetZFar() );

	CMatrix affineInverse = matCamTrans.GetAffineInverse();

	CVector projPointsPlayer[4];
	g_VisualDebug->Printf2D( 885, 550, DC_GREEN, 0, "Player:" );
	for( int count=0; count<4; count++ )
	{
		// Get Cam Space
		projPointsPlayer[count] = CVector( rectPoints[count] * affineInverse );

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
		projPointsPlayer[count].X() *= -1.0f;
#ifdef BOSS_CAM_DBGREND
		g_VisualDebug->Printf2D( 885.0f, 562.0f + (count * 12.0f), DC_GREEN, 0, "point %d: (%.3f, %.3f, %.3f)", count, projPointsPlayer[count].X(), projPointsPlayer[count].Y(), projPointsPlayer[count].Z() );
#endif
		projPointsPlayer[count] += CVector( 1.0f, 1.0f, 0.0f, 0.0f );
		//projPointsPlayer[count] *= 300.0f;
	}

	// draw rectangles in screen space representation
	/*
#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->RenderLine( CPoint(projPointsPlayer[0]), CPoint(projPointsPlayer[1]), DC_RED, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint(projPointsPlayer[1]), CPoint(projPointsPlayer[3]), DC_RED, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint(projPointsPlayer[3]), CPoint(projPointsPlayer[2]), DC_RED, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint(projPointsPlayer[2]), CPoint(projPointsPlayer[0]), DC_RED, DPF_DISPLAYSPACE );

		float screenWidth = g_VisualDebug->GetDebugDisplayWidth();
		float screenHeight = g_VisualDebug->GetDebugDisplayHeight();

		g_VisualDebug->Printf2D( 885, 450, DC_CYAN, 0, "display size: %f, %f ", screenWidth, screenHeight );


		CPoint start = CPoint( 0.1f, 0.1f, 0.0f );
		CPoint end = CPoint(0.65f, 0.1f, 0.0f );
		g_VisualDebug->RenderLine( start, end, DC_GREEN, DPF_DISPLAYSPACE );
		start = CPoint( 0.1f, 0.1f, 0.0f );
		end = CPoint(0.1f, 0.65f, 0.0f );
		g_VisualDebug->RenderLine( start, end, DC_GREEN, DPF_DISPLAYSPACE );
		start = CPoint( 0.65f, 0.1f, 0.0f );
		end = CPoint(0.65f, 0.65f, 0.0f );
		g_VisualDebug->RenderLine( start, end, DC_GREEN, DPF_DISPLAYSPACE );
		start = CPoint( 0.1f, 0.65f, 0.0f );
		end = CPoint(0.65f, 0.65f, 0.0f );
		g_VisualDebug->RenderLine( start, end, DC_GREEN, DPF_DISPLAYSPACE );

		start = CPoint( 0.0f, 0.0f, 1.0f );
		end = CPoint(1.0f, 0.0f, 1.0f );
		g_VisualDebug->RenderLine( start, end, DC_GREEN, DPF_DISPLAYSPACE );
		start = CPoint( 0.0f, 0.0f, 1.0f );
		end = CPoint(0.0f, 1.0f, 1.0f );
		g_VisualDebug->RenderLine( start, end, DC_GREEN, DPF_VIEWPORTSPACE );
		start = CPoint( 1.0f, 0.0f, 1.0f );
		end = CPoint(1.0f, 1.0f, 1.0f );
		g_VisualDebug->RenderLine( start, end, DC_GREEN, DPF_DISPLAYSPACE );
		start = CPoint( 0.0f, 1.0f, 1.0f );
		end = CPoint(1.0f, 1.0f, 1.0f );
		g_VisualDebug->RenderLine( start, end, DC_GREEN, DPF_VIEWPORTSPACE );


		start = CPoint( 50.0f, 50.0f, 0.0f );
		end = CPoint(100.0f, 50.0f, 0.0f );
		g_VisualDebug->RenderLine( start, end, DC_BLUE, DPF_DISPLAYSPACE );
		start = CPoint( 50.0f, 50.0f, 0.0f );
		end = CPoint(50.0f, 100.0f, 0.0f );
		g_VisualDebug->RenderLine( start, end, DC_BLUE, DPF_VIEWPORTSPACE );
		start = CPoint( 100.0f, 50.0f, 0.0f );
		end = CPoint(100.0f, 100.0f, 0.0f );
		g_VisualDebug->RenderLine( start, end, DC_BLUE, DPF_DISPLAYSPACE );
		start = CPoint( 50.0f, 100.0f, 0.0f );
		end = CPoint(100.0f, 100.0f, 0.0f );
		g_VisualDebug->RenderLine( start, end, DC_BLUE, DPF_VIEWPORTSPACE );

		g_VisualDebug->RenderLine( CPoint( 0.0f, 0.0f, 0.0f ), CPoint(10.0f, 0.0f, 0.0f ), DC_GREEN, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint( 0.0f, 0.0f, 0.0f ), CPoint(0.0f, 10.0f, 0.0f ), DC_GREEN, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint( 10.0f, 0.0f, 0.0f ), CPoint(10.0f, 10.0f, 0.0f ), DC_GREEN, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint( 0.0f, 10.0f, 0.0f ), CPoint(10.0f, 10.0f, 0.0f ), DC_GREEN, DPF_DISPLAYSPACE );

		g_VisualDebug->RenderLine( CPoint( 0.0f, 0.0f, 0.0f ), CPoint(100.0f, 0.0f, 0.0f ), DC_GREEN, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint( 0.0f, 0.0f, 0.0f ), CPoint(0.0f, 100.0f, 0.0f ), DC_GREEN, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint( 100.0f, 0.0f, 0.0f ), CPoint(100.0f, 100.0f, 0.0f ), DC_GREEN, DPF_DISPLAYSPACE );
		g_VisualDebug->RenderLine( CPoint( 0.0f, 100.0f, 0.0f ), CPoint(100.0f, 100.0f, 0.0f ), DC_GREEN, DPF_DISPLAYSPACE );
	}
#endif
	*/

	CPoint obBossPosition( CONSTRUCT_CLEAR );
	obBossPosition = m_pCamDef->GetBossEntity()->GetPosition();

	rectPoints[0] = obBossPosition - (obRightDir * 0.8f); 
	rectPoints[1] = obBossPosition + (obRightDir * 0.8f);
	
	rectPoints[2] = obBossPosition + (obVertical * 2.4f) - (obRightDir * 0.8f);
	rectPoints[3] = obBossPosition + (obVertical * 2.4f) + (obRightDir * 0.8f);

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), rectPoints[0], 0.25, DC_BLUE );
		g_VisualDebug->RenderLine( rectPoints[0], rectPoints[1], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPoints[1], rectPoints[3], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPoints[3], rectPoints[2], DC_YELLOW );
		g_VisualDebug->RenderLine( rectPoints[2], rectPoints[0], DC_YELLOW );
		g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), rectPoints[3], 0.25, DC_CYAN );
	}
#endif

	CVector projPointsBoss[4];
	g_VisualDebug->Printf2D( 885, 610, DC_GREEN, 0, "Boss:" );
	for( int count=0; count<4; count++ )
	{
		// Get Cam Space
		projPointsBoss[count] = CVector( rectPoints[count] * affineInverse );

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
		projPointsBoss[count].X() *= -1.0f;
#ifdef BOSS_CAM_DBGREND
		g_VisualDebug->Printf2D( 885.0f, 622.0f + (count * 12.0f), DC_GREEN, 0, "point %d: (%.3f, %.3f, %.3f)", count, projPointsBoss[count].X(), projPointsBoss[count].Y(), projPointsBoss[count].Z() );
#endif
		projPointsBoss[count] += CVector( 1.0f, 1.0f, 0.0f, 0.0f );
		//projPointsBoss[count] *= 300.0f;
	}

	// draw rectangles in screen space representation
#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		g_VisualDebug->RenderLine( CPoint(projPointsBoss[0]), CPoint(projPointsBoss[1]), DC_RED, DPF_VIEWPORTSPACE );
		g_VisualDebug->RenderLine( CPoint(projPointsBoss[1]), CPoint(projPointsBoss[3]), DC_RED, DPF_VIEWPORTSPACE );
		g_VisualDebug->RenderLine( CPoint(projPointsBoss[3]), CPoint(projPointsBoss[2]), DC_RED, DPF_VIEWPORTSPACE );
		g_VisualDebug->RenderLine( CPoint(projPointsBoss[2]), CPoint(projPointsBoss[0]), DC_RED, DPF_VIEWPORTSPACE );
	}
#endif	

	bool bHorizTest1 = ( (projPointsPlayer[0].X() > projPointsBoss[0].X()) && (projPointsPlayer[0].X() < projPointsBoss[3].X()) );
	bool bHorizTest2 = ( (projPointsPlayer[3].X() > projPointsBoss[0].X()) && (projPointsPlayer[3].X() < projPointsBoss[3].X()) );

	bool bVertTest1 = ( (projPointsPlayer[0].Y() > projPointsBoss[0].Y()) && (projPointsPlayer[0].Y() < projPointsBoss[3].Y()) );
	bool bVertTest2 = ( (projPointsPlayer[3].Y() > projPointsBoss[0].Y()) && (projPointsPlayer[3].Y() < projPointsBoss[3].Y()) );


	bool bHorizTest3 = ( (projPointsBoss[0].X() > projPointsPlayer[0].X()) && (projPointsBoss[0].X() < projPointsPlayer[3].X()) );
	bool bHorizTest4 = ( (projPointsBoss[3].X() > projPointsPlayer[0].X()) && (projPointsBoss[3].X() < projPointsPlayer[3].X()) );

	bool bVertTest3 = ( (projPointsBoss[0].Y() > projPointsPlayer[0].Y()) && (projPointsBoss[0].Y() < projPointsPlayer[3].Y()) );
	bool bVertTest4 = ( (projPointsBoss[3].Y() > projPointsPlayer[0].Y()) && (projPointsBoss[3].Y() < projPointsPlayer[3].Y()) );


	bool bOverlap = (bHorizTest1 && bVertTest1) || (bHorizTest1 && bVertTest2) || (bHorizTest2 && bVertTest2) || (bHorizTest2 && bVertTest1) ||
					(bHorizTest3 && bVertTest3) || (bHorizTest3 && bVertTest4) || (bHorizTest4 && bVertTest4) || (bHorizTest4 && bVertTest3);
	if( bOverlap )
	{
		g_VisualDebug->Printf2D( 550.0f, 380.0f, DC_RED, 0, "OVERLAP" );
	}


//	// screen limit values
//	rectPoints[0] = obBossPosition - (obRightDir * 0.8f); 
//	rectPoints[1] = obBossPosition + (obRightDir * 0.8f);
//	
//	rectPoints[2] = obBossPosition + (obVertical * 2.4f) - (obRightDir * 0.8f);
//	rectPoints[3] = obBossPosition + (obVertical * 2.4f) + (obRightDir * 0.8f);
//
//	if( m_pCamDef->GetRenderDebug() )
//	{
//		g_VisualDebug->RenderLine( rectPoints[0], rectPoints[1], DC_YELLOW );
//		g_VisualDebug->RenderLine( rectPoints[1], rectPoints[3], DC_YELLOW );
//		g_VisualDebug->RenderLine( rectPoints[3], rectPoints[2], DC_YELLOW );
//		g_VisualDebug->RenderLine( rectPoints[2], rectPoints[0], DC_YELLOW );
//	}

#endif
}
