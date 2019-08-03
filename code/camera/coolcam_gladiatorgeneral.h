//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Boss.h
//!
//------------------------------------------------------------------------------------------


#ifndef _COOLCAM_GLADIATOR_GENERAL_H
#define _COOLCAM_GLADIATOR_GENERAL_H

#include "camera/coolcam_rootboss.h"

#if defined( _DEBUG ) || defined( _DEVELOPMENT )
#define BOSS_CAM_DBGREND
#endif

class CEntity;
class CoolCam_GladiatorGeneral;
class CamVolBossAvoid;

class CoolCam_GladiatorGeneralDef : public CoolCam_RootBossDef
{
	public:
		explicit CoolCam_GladiatorGeneralDef( void );

		CoolCam_GladiatorGeneral* Create( const CamView& view );

		float GetOffsetFromPillarVolume( void ) const { return m_fOffsetFromPillarVolume; }
		float GetDeflectionFromPillar( void ) const { return m_fDeflectionAngleFromPillar; }
		float GetDeflectedHeightModifier( void ) const { return m_fDeflectedHeightModifier; }
		float GetBasePillarCameraHeight( void ) const { return m_fBasePillarCameraHeight; }
        
		float GetPlayerMovementFocusBias( void ) const { return m_fPlayerMovementFocusBias; }
		
		bool GetEliminateVerticalDeflection( void ) const { return m_bEliminateVerticalDeflection; }

		bool GetRenderPillarVolumes( void ) const { return m_bRenderPillarVolumes; }
		bool GetRenderCameraForceVolumes( void ) const { return m_bRenderCameraForceVolumes; }
		
		typedef ntstd::List<CamVolBossAvoid*, Mem::MC_CAMERA> AvoidVolList;
		typedef ntstd::List<CamVolBossAvoid*, Mem::MC_CAMERA>::iterator AvoidVolListIter;
		typedef ntstd::List<CamVolBossAvoid*, Mem::MC_CAMERA>::const_iterator AvoidVolListIterConst;
		const AvoidVolList& GetVolumeList( void ) const { return m_avoidVolList; }

		bool GetObstructionCheck( void ) const { return m_bDoObstructionCheck; }
		bool GetObstructionCameraCut( void ) const { return m_bDoObstructionCameraCut; }

	protected:
		// default constructor will go here eventually

	private:

		float m_fOffsetFromPillarVolume;
		float m_fDeflectionAngleFromPillar;
		float m_fDeflectedHeightModifier;
		float m_fBasePillarCameraHeight;

		float m_fPlayerMovementFocusBias;

		AvoidVolList			m_avoidVolList;

		bool m_bEliminateVerticalDeflection;

		bool m_bRenderPillarVolumes;
		bool m_bRenderCameraForceVolumes;

		bool m_bDoObstructionCheck;
		bool m_bDoObstructionCameraCut;

		friend class CoolCam_GladiatorGeneralDefInterface;
};


//------------------------------------------------------------------------------------------
//! 
//!	CoolCam_Boss
//! 
//------------------------------------------------------------------------------------------
class CoolCam_GladiatorGeneral : public CoolCam_RootBoss
{
	public:
		explicit CoolCam_GladiatorGeneral( const CamView& view, const CoolCam_GladiatorGeneralDef* pCamDef );
		virtual ~CoolCam_GladiatorGeneral( void );

//		virtual void        Update(float fTimeDelta);
		virtual void        Reset() {}
		virtual bool        HasFinished() const   {return m_bFinished;}
		virtual CHashedString GetCameraName() const {return "*Boss Cam*";}
		virtual CAMTYPE     GetType() const       {return CT_BOSS;}

	#ifndef _RELEASE
		virtual void RenderDebugInfo() {;}
	#endif

	// Members
protected:
		explicit CoolCam_GladiatorGeneral( void );

		const CoolCam_GladiatorGeneralDef* m_pCamDef;

		CPoint CalcDesiredCameraPosition( CEntity& obPlayer, CPoint& obBossPosition );
		CPoint CalcDesiredCameraLookatPoint( CEntity& obPlayer, CPoint& obBossPosition );

		void CalcSafePosition( CamVolBossAvoid* pBossAvoidVol, CPoint& obProjPoint,
							   CPoint& obTestPoint, CPoint& obResult );

		bool ObstructionTest( CPoint& obCameraPos );
		bool PointInProjectedPolygon2( CamVolBossAvoid* pBossAvoidVol, CPoint& obProjPointParam, CPoint& obTestPointParam );

		CPoint ImprovedCalcPos( CEntity& obPlayer, CPoint& obBossPosition );

		const CEntity* m_pobBoss;

		float m_fMinDist;
		float m_fMaxDist;
		float m_fMinFOV;
		float m_fMaxFOV;
		float m_fCurrDist;
		float m_fCurrFOV;

		float m_fFOVToleranceZone;		//< percentage value of min/max
		float m_fDistToleranceZone;		//< percentage value of min/max

		float m_fMinCamFocusDist;
		float m_fMinCamFocusDistSqrd;
		float m_fMinCamBossDistSqrd;

		CDirection m_obFocusToCameraDir;

		CDirection m_obCamVerticalOffset;

		float m_fBossHeroSeperationAngleDeg;

		float m_fSeperationDistanceModifier;
		float m_fSepDistModifierIncrement;

		float m_fCameraAngleModifierDeg;

		CVector m_obTotalForce;

		CDirection m_obCamVelocity;

//		bool m_bReverseModeActivated;
		CPoint m_obCachedPosition;

		CamVolBossAvoid* m_pObstructingAvoidVol;

#ifdef BOSS_CAM_DBGREND
		int m_iDebugTextOffset1;
		int m_iDebugTextOffset2;
#endif

	private:
		friend class CoolCam_GladiatorGeneralInterface;

};

#endif // _COOLCAM_BOSS_H
