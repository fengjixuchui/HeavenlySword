//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_RootBoss.cpp
//!
//------------------------------------------------------------------------------------------

#include "camera/coolcam_rootboss.h"
#include "camera/sceneelementcomponent.h"
#include "camera/camutils.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "core/visualdebugger.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "objectdatabase/dataobject.h"
#include "game/entitymanager.h"
#include "game/inputcomponent.h"

START_CHUNKED_INTERFACE( CoolCam_RootBossDef, Mem::MC_CAMERA)

	PUBLISH_PTR_AS( m_pBoss, Boss )

	PUBLISH_VAR_AS( m_fCameraMaxSpeed, MaxSpeed )
	PUBLISH_VAR_AS( m_fCameraAcceleration, Acceleration )
	PUBLISH_VAR_AS( m_fCameraDecceleration, Deceleration )

	PUBLISH_VAR_AS( m_fDragThresholdDistance, DragThresholdDistance )
	PUBLISH_VAR_AS( m_fDragStopDistance, DragStopDistance )

	PUBLISH_VAR_AS( m_fFocusPointMaxSpeed, FocusPointMaxSpeed )
	PUBLISH_VAR_AS( m_fFocusPointAcceleration, FocusPointAcceleration )
	PUBLISH_VAR_AS( m_fFocusPointDeceleration, FocusPointDeceleration )

	PUBLISH_VAR_AS( m_fCamToFocusDistance, CameraToFocusDistance )

	PUBLISH_VAR_AS( m_fCamToPlayerDistance, CameraToPlayerDistance )
	PUBLISH_VAR_AS( m_fCamToPlayerMinProximity, CamToPlayerMinProximity )

	PUBLISH_VAR_AS( m_fBossHeroSeperationAngleDeg, BossHeroSeperationAngleDeg )
	PUBLISH_VAR_AS( m_fCameraAngleModifierDeg, CameraAngleModifierDeg )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_bReverseMode, false, ReverseMode )
	PUBLISH_VAR_AS( m_fFOV, FieldofView )
	PUBLISH_VAR_AS( m_fFOVBlendtime, FOVBlendTime )

	PUBLISH_VAR_AS( m_obFocusWorldOffset, FocusWorldOffset )
	PUBLISH_VAR_AS( m_obFocusCameraRelativeOffset, FocusCameraRelativeOffset )

	PUBLISH_VAR_AS( m_fFocusLambda, FocusLambda )
	PUBLISH_VAR_AS( m_fPullbackMult, PullbackMultiplier )
	PUBLISH_VAR_AS( m_fOffsetAngle, CameraOffsetAngle )
	PUBLISH_VAR_AS( m_fCameraPullbackBase, CameraPullbackBase )
	PUBLISH_VAR_AS( m_obProximityOffset, ProximityOffset )
	PUBLISH_VAR_AS( m_obDistanceOffset, DistanceOffset )
	PUBLISH_VAR_AS( m_obReverseAngleOffset, ReverseAngleOffset )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fCameraModeSwitchDistance, 15.0, CameraModeSwitchDistance )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fObstructionCamCutTimeLimit, 2.0f, ObstructionCamCutTimeLimit )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fFocusAvoidanceZoneRadius, 2.0f, FocusAvoidanceZoneRadius )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_fReverseAngleDelayTime, 1.0f, ReverseCameraAngleDelayTime )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_bDebugVolumeRender, false, RenderDebugVolumes )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_bRenderDebug, false, BossCamDebugRender )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_bDebugUpdate, false, DebugUpdate )
END_STD_INTERFACE


CoolCam_RootBossDef::CoolCam_RootBossDef( void )
: m_pBoss(0),
  m_fCameraMaxSpeed(25.0f),
  m_fCameraAcceleration(8.0f),
  m_fCameraDecceleration(12.0f),
  m_fDragThresholdDistance(12.0f),
  m_fDragStopDistance(0.0f),
  m_fFocusPointMaxSpeed(5.0f),
  m_fFocusPointAcceleration(3.0f),
  m_fFocusPointDeceleration(5.0f),
  m_fCamToFocusDistance(7.0f),
  m_fCamToPlayerDistance(4.0f),
  m_fCamToPlayerMinProximity(3.0f),
  m_fBossHeroSeperationAngleDeg(40.0f),
  m_fCameraAngleModifierDeg(90.0f),
  m_bReverseMode(false),
  m_fFOV(35.0f),
  m_fFOVBlendtime(1.0f),
  m_obFocusWorldOffset(CONSTRUCT_CLEAR),
  m_obFocusCameraRelativeOffset(CONSTRUCT_CLEAR),
  m_fFocusLambda(0.6f),
  m_fPullbackMult(2.25f),
  m_fOffsetAngle(40.0f),
  m_fCameraPullbackBase(1.0f),
  m_obProximityOffset(CONSTRUCT_CLEAR),
  m_obDistanceOffset(CONSTRUCT_CLEAR),
  m_obReverseAngleOffset(CONSTRUCT_CLEAR),
  m_fCameraModeSwitchDistance(14.0f),
  m_fObstructionCamCutTimeLimit(2.0f),
  m_fFocusAvoidanceZoneRadius(2.0f),
  m_fReverseAngleDelayTime(1.0f),
  m_bDebugVolumeRender(false),
  m_bRenderDebug(false),
  m_bDebugUpdate(false)
{
}

void CoolCam_RootBossDef::ActivateReverseAngleMode( void )
{
	m_bReverseMode = true;
}

void CoolCam_RootBossDef::DeactivateReverseAngleMode( void )
{
	m_bReverseMode = false;
}

void CoolCam_RootBossDef::SetFOV( float fFOV )
{ 
	ntError_p( fFOV>0.0f, ("Field of View value needs to be larger than 0.0") );
	ntError_p( fFOV<90.0f, ("Field of View value needs to be less than 90.0") );
	m_fFOV = fFOV;
}

void CoolCam_RootBossDef::SetFOVBlendTime( float fFOVBlendTime )
{
	if( fFOVBlendTime<0.0f )
	{
		fFOVBlendTime = 0.0f;
	}
	m_fFOVBlendtime = fFOVBlendTime;
}

CoolCam_RootBoss::CoolCam_RootBoss( const CamView& view, const CoolCam_RootBossDef* pCamDef )
: CoolCamera( view ),
  m_pCamDef(pCamDef),
  m_obNewCamPos( CONSTRUCT_CLEAR ),
  m_obCamTargetPos( CONSTRUCT_CLEAR ),
  m_obNewCamLookAt( CONSTRUCT_CLEAR ),
  m_obVelBasedPos( CONSTRUCT_CLEAR ),
  m_obCamVelocity( CONSTRUCT_CLEAR ),
  m_obLinearSpeed( GRAPH_TYPE_ROLLING ),
  m_obFocusSpeed( GRAPH_TYPE_ROLLING ),
  m_pobLinearSpeedSampleSet(0),
  m_pobFocusSpeedSampleSet(0),
  m_fCameraSpeed(0.0f),
  m_fFocusPointSpeed(0.0f),
  m_fFocusVerticalOffset(1.2f),
  m_fCameraDistance(3.0f),
  m_bCameraDragActive(false),
  m_bHoldCameraDistance(false),
  m_bFOVChange(false),
  m_fFOVTargetValue(40.0f),
  m_fRemainingFOVBlendTime(0.0f),
  m_bReverseModeActivated(false),
  m_obReverseAnglePosition(CONSTRUCT_CLEAR),
  m_bOcclusionCut(false),
  m_fCameraAngleMod(-1.0f),
  m_fTimeSinceObstructionCameraCut(0.0f),
  m_fReverseCameraAngleDelayTimer(0.0f),
  m_fTimeSlice(0.0f)
{
	m_fFOVTargetValue = m_fFOV;
}

CoolCam_RootBoss::~CoolCam_RootBoss( void )
{
}


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_RootBoss::Update
//!	Update the camera.
//!
//------------------------------------------------------------------------------------------

void CoolCam_RootBoss::Update(float fTimeDelta)
{
	m_fTimeSlice = fTimeDelta;
	if( m_pCamDef!=0 )
	{
		ntError_p ( m_pCamDef->GetBossEntity(), ("Boss cam %s without valid boss entity\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( m_pCamDef ))) );

		CDirection obTotalAccel( CONSTRUCT_CLEAR );

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

#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			g_VisualDebug->Printf2D( 35, 32, DC_WHITE, 0, "Player pos: (%f,%f,%f)", obPlayerPosition.X(), obPlayerPosition.Y(), obPlayerPosition.Z() );
		}
#endif

		CPoint obBossPosition;
		if( m_pCamDef->GetBossEntity() && m_pCamDef->GetBossEntity()->GetSceneElement() )
		{
			obBossPosition = m_pCamDef->GetBossEntity()->GetSceneElement()->GetPosition();
		}
		else if( m_pCamDef->GetBossEntity() )
		{
			obBossPosition = m_pCamDef->GetBossEntity()->GetPosition();
		}
		CPoint obCameraPosition( m_obTransform.GetTranslation() );

		m_obCamTargetPos = CalcDesiredCameraPosition( *pPlayer, obBossPosition );
		
		CPoint obNewCameraPos( m_obCamTargetPos );

		CPoint obFocusPoint = CalcDesiredCameraLookatPoint( *pPlayer, obBossPosition );

		CDirection obFocusDir( obFocusPoint - obNewCameraPos );
		obFocusDir.Normalise();

		//CDirection obPlayerCamDir( obPlayerPosition - m_obTransform.GetTranslation() );
		//float fPlayerCamDist = obPlayerCamDir.Length();

		CDirection obCamToPlayer( obCameraPosition - obPlayerPosition );
		CDirection obCamToBoss( obCameraPosition - obBossPosition );
		float fPlayerCamDist = obCamToPlayer.Length();
		obCamToPlayer.Normalise();
		obCamToBoss.Normalise();

		float fCamToBossDotCamToPlayer = obCamToBoss.Dot( obCamToPlayer );
		float fCamToBossCamToPlayerAngleDeg = acosf( fCamToBossDotCamToPlayer ) * RAD_TO_DEG_VALUE;


#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			g_VisualDebug->Printf2D( 35, 312, DC_YELLOW, 0, "Player cam distance: %f", fPlayerCamDist );
			g_VisualDebug->Printf2D( 500, 620, DC_GREEN, 0, "Obstruct Cam Cut Time: %f", m_fTimeSinceObstructionCameraCut );
		}
#endif

		// update time since last obstruction triggered camera cut
		m_fTimeSinceObstructionCameraCut += fTimeDelta;
		if( m_fTimeSinceObstructionCameraCut >= (FLT_MAX - (EPSILON * 10.0f )) )
		{
			m_fTimeSinceObstructionCameraCut = m_pCamDef->GetObstructionCamCutTimeLimit();
		}
		//-----------------------------
		// Translational movement
		//-----------------------------

		// work out distance from current camera position and desired camera pos
		CDirection obCamTargetDir( obNewCameraPos - obCameraPosition );
		float fCamTargetDist = obCamTargetDir.Length();
		obCamTargetDir.Normalise();


		if( (fCamTargetDist>m_pCamDef->GetDragThresholdDistance()) || (fCamToBossCamToPlayerAngleDeg>m_pCamDef->GetBossHeroSeperationAngleDeg()) )
		{
			m_bCameraDragActive = true;
		}
		else if( fCamTargetDist<=(m_pCamDef->GetDragStopDistance() + (EPSILON * 2.0f * m_pCamDef->GetCameraMaxSpeed())) )
		{
			m_bCameraDragActive = false;
		}

		if( fPlayerCamDist<m_pCamDef->GetCamToPlayerMinProximity() )
		{
			m_bCameraDragActive = true;
		}

		if( m_pCamDef->GetDebugUpdate() )
		{
			m_bCameraDragActive = true;
		}

		// check that the camera isn't going to travel too close to the focus
		// point and so gimble lock. If it is, simply disable the camera movement for the moment.
		if( m_bCameraDragActive==true )
		{
			// transform camera position and target position into focus space,
			// i.e. focus at 0,0,0 in the world.
			CPoint obCameraPosFocusSpace = obCameraPosition - m_obNewCamLookAt;
			CPoint obCameraTargetFocusSpace = m_obCamTargetPos - m_obNewCamLookAt;
#ifdef BOSS_CAM_DBGREND
			if( m_pCamDef->GetRenderDebug() )
			{
				g_VisualDebug->RenderLine( obCameraPosition, m_obCamTargetPos, 0x00ffaa00 );
			}
#endif
			// We ignore the vertical component, we don't want to approach the focus point
			// at all, altitude is irrelevant.
			obCameraPosFocusSpace.Y() = 0.0f;
			obCameraTargetFocusSpace.Y() = 0.0f;

			// calc difference vector from current pos to target pos
			CDirection obCamDir( obCameraTargetFocusSpace - obCameraPosFocusSpace );
			float fDistSqrd = ( obCamDir.X() * obCamDir.X() ) + ( obCamDir.Z() * obCamDir.Z() );
			// Cross prod of co-ords
			float fCrossProd = (obCameraPosFocusSpace.X() * obCameraTargetFocusSpace.Z()) - (obCameraTargetFocusSpace.X() * obCameraPosFocusSpace.Z());

			// calc discriminant to determine if there is an intersection.
			float fAvoidRadiusSqrd = m_pCamDef->GetFocusAvoidanceZoneRadius() * m_pCamDef->GetFocusAvoidanceZoneRadius();
			float fDiscrim = (fAvoidRadiusSqrd) * fDistSqrd - (fCrossProd * fCrossProd);

#ifdef BOSS_CAM_DBGREND
			if( m_pCamDef->GetRenderDebug() )
			{
				g_VisualDebug->Printf2D( 500, 632, DC_RED, 0, "Discriminator: %f", fDiscrim );
				if( m_pCamDef->GetDebugVolumeRender() )
				{
					g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obNewCamLookAt, m_pCamDef->GetFocusAvoidanceZoneRadius(), DC_PURPLE );
				}
			}
#endif
			// if the discriminant is greater than 0, then the line intersects the circle
			// otherwise at 0 it forms a tangent and <0 there is no intersection.
			if( fDiscrim>0.0f && fCamTargetDist>0.1f )
			{
				// turn off camera drag until a more suitable target position is chosen.
				//m_bCameraDragActive = false;
				m_bOcclusionCut = true;
			}
		}

		if( m_bReverseModeActivated==true )
		{
			m_fReverseCameraAngleDelayTimer += fTimeDelta;
			if( m_fReverseCameraAngleDelayTimer>m_pCamDef->GetReverseAngleDelayTime() )
			{
				m_obVelBasedPos = m_obReverseAnglePosition;
				m_fReverseCameraAngleDelayTimer = 0.0f;
				if( m_bReverseModeActivated==true && m_pCamDef->IsReverseAngleActive()==false )
				{
					m_bReverseModeActivated = false;
					m_obVelBasedPos = m_obCamTargetPos;

					pPlayer->GetInputComponent()->HoldInputDirection( GetTransform().GetZAxis(), this );
				}
			}
		}
		else if( m_bOcclusionCut==true && m_bCameraDragActive==true )
		{
			m_obVelBasedPos = m_obCamTargetPos;
			m_fTimeSinceObstructionCameraCut = 0.0f;
			m_bOcclusionCut = false;
			pPlayer->GetInputComponent()->HoldInputDirection( GetTransform().GetZAxis(), this );
		}
		else if( m_bCameraDragActive==true )
		{
			m_bCameraDragActive = true;
			// time to reach destination = distance/speed
			float fTravelTime = fCamTargetDist / m_fCameraSpeed;

			// distance possible this frame = time * speed
			float fPotentialDistance = fTimeDelta * m_fCameraSpeed;

			// deceleration time, i.e. time to reach 0 speed from current speed
			float fDecelTime = m_fCameraSpeed / m_pCamDef->GetCameraDeceleration(); 

			// calc braking distance v^2 = v0^2 + 2a(x - x0)
			// v - final velocity
			// v0 - initial velocity
			// a - acceleration
			// x - starting distance
			// x0 - final distance
			float fDecelDistance = (m_fCameraSpeed* m_fCameraSpeed) / (2.0f * m_pCamDef->GetCameraDeceleration());

#ifdef BOSS_CAM_DBGREND
			float fAccelDist = ((m_pCamDef->GetCameraMaxSpeed() * m_pCamDef->GetCameraMaxSpeed()) - (m_fCameraSpeed * m_fCameraSpeed)) / 2.0f * m_pCamDef->GetCameraAcceleration() ;
#endif

			// distance to decelerate is greater than distance to travel - start decelerating now
			if( fDecelDistance + fPotentialDistance > fCamTargetDist )	
			{
#ifdef BOSS_CAM_DBGREND
				float fOldSpeed = m_fCameraSpeed;
#endif
				m_fCameraSpeed -= (fTimeDelta * m_pCamDef->GetCameraDeceleration());
#ifdef BOSS_CAM_DBGREND
				if( m_pCamDef->GetRenderDebug() )
				{
					g_VisualDebug->Printf2D( 35, 220, DC_RED, 0, "Applying breaks - speed: %f -> %f", fOldSpeed, m_fCameraSpeed );
				}
#endif
			}
			else if( ((fDecelDistance + fPotentialDistance)<fCamTargetDist) && (m_fCameraSpeed<m_pCamDef->GetCameraMaxSpeed()) ) // enough space to accelerate and then decelerate
			{
				m_fCameraSpeed += (fTimeDelta *  m_pCamDef->GetCameraAcceleration());
#ifdef BOSS_CAM_DBGREND
				if( m_pCamDef->GetRenderDebug() )
				{
					g_VisualDebug->Printf2D( 35, 232, DC_RED, 0, "Accelerating (target distance)" );
				}
#endif
			}

			if( ((fTravelTime<=fTimeDelta) && (fDecelTime>=fTimeDelta)) || (fCamTargetDist<fPotentialDistance) )
			{
				obTotalAccel.Clear();
				m_obCamVelocity.Clear();

				m_fCameraSpeed = 0.0f;
				m_bCameraDragActive = false;
#ifdef BOSS_CAM_DBGREND
				if( m_pCamDef->GetRenderDebug() )
				{
					g_VisualDebug->Printf2D( 35, 244, DC_RED, 0, "Stopping camera motion - distance" );
				}
#endif
			}

#ifdef BOSS_CAM_DBGREND
			if( m_pCamDef->GetRenderDebug() )
			{
				g_VisualDebug->Printf2D( 35, 130 , DC_GREY, 0, "distance - cam pos to desired pos: %f\n", fCamTargetDist );
				g_VisualDebug->Printf2D( 35, 142 , DC_GREY, 0, "travel time at current speed: %f\n", fTravelTime );
				g_VisualDebug->Printf2D( 35, 154 , DC_GREY, 0, "max distance this frame: %f\n", fPotentialDistance );
				g_VisualDebug->Printf2D( 35, 166 , DC_GREY, 0, "decel/breaking time: %f\n", fDecelTime );
				g_VisualDebug->Printf2D( 35, 178 , DC_GREY, 0, "decel distance: %f\n", fDecelDistance );
				g_VisualDebug->Printf2D( 35, 190 , DC_GREY, 0, "distance to accel to max speed: %f\n", fAccelDist );
				g_VisualDebug->Printf2D( 35, 202 , DC_GREY, 0, "current speed: %f\n", m_fCameraSpeed );
			}
#endif

			if( m_fCameraSpeed<EPSILON )
			{
				m_fCameraSpeed = 0.0f;
			}

			if( m_fCameraSpeed>m_pCamDef->GetCameraMaxSpeed() )
			{
				m_fCameraSpeed = m_pCamDef->GetCameraMaxSpeed();
			}

			if( fCamTargetDist<(EPSILON * 2.0f * m_pCamDef->GetCameraMaxSpeed()) )
			{
				m_fCameraSpeed = 0.0f;
			}

			m_pobLinearSpeedSampleSet->AddSample( m_fCameraSpeed );

			m_obNewCamPos = obCameraPosition + (obCamTargetDir * (m_fCameraSpeed * fTimeDelta));

			if( obTotalAccel.Length()>0.0f )
			{
				m_obCamVelocity += obTotalAccel * fTimeDelta;
			}
			else
			{
				m_obCamVelocity.Clear();
			}
			
#ifdef BOSS_CAM_DBGREND
			if( m_pCamDef->GetRenderDebug() )
			{
				g_VisualDebug->Printf2D( 35, 270, DC_GREEN, 0, "Accel scalar: %f", obTotalAccel.Length() );
				g_VisualDebug->Printf2D( 35, 282, DC_GREEN, 0, "speed: %f", m_obCamVelocity.Length() );
				g_VisualDebug->Printf2D( 35, 294, DC_GREEN, 0, "previous speed: %f", m_fCameraSpeed );
			}
#endif
			m_obVelBasedPos = m_obNewCamPos + (fTimeDelta * m_obCamVelocity);
		}


		//--------------------------
		// End translational movement
		//--------------------------

		//----------------------------------
		// Focus point movement
		//----------------------------------
		{
			CDirection obFocusTargetDir( obFocusPoint - m_obNewCamLookAt );
			float fFocusTargetDist = obFocusTargetDir.Length();
			obFocusTargetDir.Normalise();

			// distance possible this frame = time * speed
			float fPotentialDistance = fTimeDelta * m_fFocusPointSpeed;

			// calc braking distance v^2 = v0^2 + 2a(x - x0)
			// v - final velocity
			// v0 - initial velocity
			// a - acceleration
			// x - starting distance
			// x0 - final distance
			float fDecelDistance = (m_fFocusPointSpeed* m_fFocusPointSpeed) / (2.0f * m_pCamDef->GetFocusPointDeceleration());

			// distance to decelerate is greater than distance to travel - start decelerating now
			if( fDecelDistance + fPotentialDistance > fFocusTargetDist )	
			{
				m_fFocusPointSpeed -= (fTimeDelta * m_pCamDef->GetFocusPointDeceleration());
			}
			else if( ((fDecelDistance)<fFocusTargetDist) && (m_fFocusPointSpeed<m_pCamDef->GetFocusPointMaxSpeed()) ) // enough space to accelerate and then decelerate
			{
				m_fFocusPointSpeed += (fTimeDelta * m_pCamDef->GetFocusPointAcceleration());
			}

			if( m_fFocusPointSpeed>m_pCamDef->GetFocusPointMaxSpeed() )
			{
				m_fFocusPointSpeed = m_pCamDef->GetFocusPointMaxSpeed();
			}

			if( fFocusTargetDist<(fPotentialDistance + EPSILON) )
			{
				m_obNewCamLookAt = obFocusPoint;
				m_fFocusPointSpeed = 0.0f;
			}

			m_pobFocusSpeedSampleSet->AddSample( m_fFocusPointSpeed );

			m_obNewCamLookAt += (obFocusTargetDir * (m_fFocusPointSpeed * fTimeDelta));

#ifdef BOSS_CAM_DBGREND
		if( m_pCamDef->GetRenderDebug() )
		{
			CDirection obCurrentCamDir = m_obTransform.GetZAxis();
			g_VisualDebug->RenderLine( obCameraPosition, obCameraPosition + (3.0f * obCurrentCamDir), DC_BLACK );
			g_VisualDebug->RenderLine( obCameraPosition, obCameraPosition + (3.0f * obFocusDir), DC_BLUE );
			g_VisualDebug->RenderLine( obCameraPosition, m_obNewCamLookAt, DC_PURPLE );

			g_VisualDebug->RenderLine( m_obNewCamLookAt, obFocusPoint, DC_RED );
			g_VisualDebug->RenderLine( m_obNewCamLookAt, m_obNewCamLookAt + (3.0f * obFocusTargetDir), DC_WHITE );


			g_VisualDebug->RenderLine( m_obVelBasedPos, m_obVelBasedPos + obTotalAccel, DC_RED );
			g_VisualDebug->RenderLine( m_obVelBasedPos, m_obVelBasedPos + m_obCamVelocity, DC_GREEN );

		}
#endif

	}

	m_obLookAt = m_obNewCamLookAt;
//	m_obLookAt.Y() += m_fFocusVerticalOffset;

#ifdef BOSS_CAM_DBGREND
	if( m_pCamDef->GetRenderDebug() )
	{
		if( m_pCamDef->GetDebugVolumeRender() )
		{
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obFocusPoint, 1.0f, DC_YELLOW );
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), obNewCameraPos, 1.0f, DC_RED );
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obLookAt, 1.0f, DC_CYAN );
			g_VisualDebug->RenderSphere( CQuat( CONSTRUCT_IDENTITY ), m_obVelBasedPos, 1.0f, DC_PURPLE );
		}
		g_VisualDebug->RenderGraph( &m_obLinearSpeed, CPoint( 35.0f, 500.0f, 0.0f ), CPoint( 235.0f, 700.0f, 0.0f ), 0 );
		g_VisualDebug->RenderGraph( &m_obFocusSpeed, CPoint( 255.0f, 500.0f, 0.0f ), CPoint( 455.0f, 700.0f, 0.0f ), 0 );
	}
#endif

	// Field of View update
	//m_bFOVChange = false;
	m_fFOVTargetValue = m_pCamDef->GetFOV();
	float fFOVDiff = m_fFOVTargetValue - m_fFOV;
	if( m_bFOVChange==false && (fabsf(fFOVDiff)>EPSILON) )
	{
		m_bFOVChange = true;
		m_fRemainingFOVBlendTime = m_pCamDef->GetFOVBlendTime();
	}

	if( m_bFOVChange==true )
	{
		float fBlendFrac = fFOVDiff/m_fRemainingFOVBlendTime;
		fBlendFrac = fFOVDiff * fTimeDelta;
		m_fFOV += fBlendFrac;
		m_fRemainingFOVBlendTime -= fTimeDelta;

		if( m_fRemainingFOVBlendTime<=0.0f )
		{
			m_bFOVChange = false;
			m_fFOV = m_fFOVTargetValue;
			//SetFOV
		}
	}

	CCamUtil::CreateFromPoints(m_obTransform, m_obVelBasedPos, m_obLookAt);	}
}
