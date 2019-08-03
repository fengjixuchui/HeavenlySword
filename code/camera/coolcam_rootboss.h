//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_RootBoss.h
//!
//------------------------------------------------------------------------------------------

#ifndef _COOLCAM_ROOT_BOSS_H
#define _COOLCAM_ROOT_BOSS_H

#include "camera/camcool.h"
#include "gfx/graphing.h"

#if defined( _DEBUG ) || defined( _DEVELOPMENT )
#define BOSS_CAM_DBGREND
#endif

class CEntity;

class CoolCam_RootBossDef
{
	public:	
		HAS_INTERFACE( CoolCam_RootBossDef );

		const CEntity* GetBossEntity( void ) const { return m_pBoss; }
		float GetCameraMaxSpeed( void )		const { return m_fCameraMaxSpeed; }
		float GetCameraAcceleration( void ) const { return m_fCameraAcceleration; }
		float GetCameraDeceleration( void ) const { return m_fCameraDecceleration; }
		float GetDragThresholdDistance( void ) const { return m_fDragThresholdDistance; }
		float GetDragStopDistance( void ) const { return m_fDragStopDistance; }
		float GetFocusPointMaxSpeed( void ) const { return m_fFocusPointMaxSpeed; }
		float GetFocusPointAcceleration( void ) const { return m_fFocusPointAcceleration; }
		float GetFocusPointDeceleration( void ) const { return m_fFocusPointDeceleration; }
		float GetCamFocusDistance( void ) const { return m_fCamToFocusDistance; }
		float GetCamPlayerDistance( void ) const { return m_fCamToPlayerDistance; }
		float GetCamToPlayerMinProximity( void ) const { return m_fCamToPlayerMinProximity; }

		float GetBossHeroSeperationAngleDeg( void ) const { return m_fBossHeroSeperationAngleDeg; }
		float GetCameraAngleModifierDeg( void ) const { return m_fCameraAngleModifierDeg; }

		void ActivateReverseAngleMode( void );
		void DeactivateReverseAngleMode( void );

		void ReverseAngleLeadInOn( void ) { m_bReverseLeadIn = true; }
		void ReverseAngleLeadInOff( void ) { m_bReverseLeadIn = false; }
		bool IsReverseAngleLeadInActive( void ) const { return m_bReverseLeadIn; }

		bool IsReverseAngleActive( void ) const { return m_bReverseMode; }
    
		float GetFOV( void ) const { return m_fFOV; }
		void SetFOV( float fFOV );
		float GetFOVBlendTime( void ) const { return m_fFOVBlendtime; }
		void SetFOVBlendTime( float fFOVBlendTime );

		CDirection GetFocusWorldOffset( void ) const { return m_obFocusWorldOffset; }
		CDirection GetFocusCameraRelativeOffset( void ) const { return m_obFocusCameraRelativeOffset; }

		float GetFocusSplitLambda( void ) const { return m_fFocusLambda; }
		float GetCameraPullbackMultiplier( void ) const { return m_fPullbackMult; }
		float GetCameraOffsetAngle( void ) const { return m_fOffsetAngle; }
		float GetCameraPullbackBase( void ) const { return m_fCameraPullbackBase; }
		CDirection GetProximityOffset( void ) const { return m_obProximityOffset; }
		CDirection GetDistanceOffset( void ) const { return m_obDistanceOffset; }
		CDirection GetReverseAngleOffset( void ) const { return m_obReverseAngleOffset; }

		float GetCameraModeSwitchDistance( void ) const { return m_fCameraModeSwitchDistance; }
		float GetObstructionCamCutTimeLimit( void ) const { return m_fObstructionCamCutTimeLimit; }

		float GetFocusAvoidanceZoneRadius( void ) const { return m_fFocusAvoidanceZoneRadius; }

		float GetReverseAngleDelayTime( void ) const { return m_fReverseAngleDelayTime; }

		bool GetDebugVolumeRender( void ) const { return m_bDebugVolumeRender; }
		bool GetRenderDebug( void ) const { return m_bRenderDebug; }
		bool GetDebugUpdate( void ) const { return m_bDebugUpdate; }

	protected:
		explicit CoolCam_RootBossDef( void );

		const CEntity* m_pBoss;

		float m_fCameraMaxSpeed;
		float m_fCameraAcceleration;
		float m_fCameraDecceleration;


		float m_fDragThresholdDistance;
		float m_fDragStopDistance;

		float m_fFocusPointMaxSpeed;
		float m_fFocusPointAcceleration;
		float m_fFocusPointDeceleration;
	
		float m_fCamToFocusDistance;

		float m_fCamToPlayerDistance;
		float m_fCamToPlayerMinProximity;

		float m_fBossHeroSeperationAngleDeg;
		float m_fCameraAngleModifierDeg;

		bool m_bReverseMode;
		bool m_bReverseLeadIn;

		float m_fFOV;
		float m_fFOVBlendtime;

		CDirection m_obFocusWorldOffset;
		CDirection m_obFocusCameraRelativeOffset;

		float m_fFocusLambda;
		float m_fPullbackMult;
		float m_fOffsetAngle;
		float m_fCameraPullbackBase;
		CDirection m_obProximityOffset;
		CDirection m_obDistanceOffset;
		CDirection m_obReverseAngleOffset;

		float m_fCameraModeSwitchDistance;
		float m_fObstructionCamCutTimeLimit;

		float m_fFocusAvoidanceZoneRadius;

		float m_fReverseAngleDelayTime;

		bool m_bDebugVolumeRender;
		bool m_bRenderDebug;
		bool m_bDebugUpdate;
};


class CoolCam_RootBoss : public CoolCamera
{
	public:
		virtual ~CoolCam_RootBoss( void );

		virtual void        Update(float fTimeDelta);
		virtual void        Reset() {}
		virtual bool        HasFinished() const   {return m_bFinished;}
		virtual CHashedString GetCameraName() const {return "*Boss Cam*";}
		virtual CAMTYPE     GetType() const       {return CT_BOSS;}

	#ifndef _RELEASE
		virtual void RenderDebugInfo() {;}
	#endif

	protected:
		explicit CoolCam_RootBoss( const CamView& view, const CoolCam_RootBossDef* pCamDef );
		//explicit CoolCam_RootBoss( const CamView& view );

		virtual CPoint CalcDesiredCameraPosition( CEntity& obPlayer, CPoint& obBossPosition )=0;
		virtual CPoint CalcDesiredCameraLookatPoint( CEntity& obPlayer, CPoint& obBossPosition )=0;

		const CoolCam_RootBossDef* m_pCamDef;

		// camera location and movement parameters
		CPoint m_obNewCamPos;
		CPoint m_obCamTargetPos;
		CPoint m_obNewCamLookAt;
		CPoint m_obVelBasedPos;

		CDirection m_obCamVelocity;

		CGraph m_obLinearSpeed;
		CGraph m_obFocusSpeed;
		CGraphSampleSet* m_pobLinearSpeedSampleSet;
		CGraphSampleSet* m_pobFocusSpeedSampleSet;

		float m_fCameraSpeed;
		float m_fFocusPointSpeed;
		float m_fFocusVerticalOffset;
		float m_fCameraDistance;

		bool m_bCameraDragActive;
		bool m_bHoldCameraDistance;

		bool m_bFOVChange;
		float m_fFOVTargetValue;
		float m_fRemainingFOVBlendTime;

		bool m_bReverseModeActivated;
		CPoint m_obReverseAnglePosition;

		bool m_bOcclusionCut;
		float m_fCameraAngleMod;
		float m_fTimeSinceObstructionCameraCut;

		float m_fReverseCameraAngleDelayTimer;

		float m_fTimeSlice;
};


#endif
