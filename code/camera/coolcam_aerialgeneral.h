//------------------------------------------------------------------------------------------
//!
//!	\file CamCool_AerialGeneral.h
//!
//------------------------------------------------------------------------------------------

#ifndef _COOLCAM_AERIAL_GENERAL_H
#define _COOLCAM_AERIAL_GENERAL_H

#include "camera/coolcam_rootboss.h"

class CEntity;
class CamView;
class CoolCam_AerialGeneral;

class CoolCam_AerialGeneralDef : public CoolCam_RootBossDef
{
	public:
		HAS_INTERFACE( CoolCam_AerialGeneralDef );

		explicit CoolCam_AerialGeneralDef( void );

		CoolCam_AerialGeneral* Create( const CamView& view );

		CPoint GetOrbitCentrePoint( void ) const { return m_obCentrePoint; }
		float GetOrbitRadius( void ) const { return m_fRadius; }
		float GetOrbitInitialSpeed( void ) const { return m_fInitialSpeed; }
		float GetOrbitAcceleration( void ) const { return m_fAcceleration; }
		float GetOrbitMaxSpeed( void ) const { return m_fMaxSpeed; }

		bool IsOrbitActive( void ) const { return m_bOrbitalCameraActive; }
		bool OrbitCutOnActivation( void ) const { return m_bDoCutOnActivation; }
		bool ApplyAcceleration( void ) const { return m_bApplyAcceleration; }

		void ActivateOrbitMode( void ) { m_bOrbitalCameraActive = true; }
		void DeactivateOrbitMode( void ) { m_bOrbitalCameraActive = false; }
		void StartAccelerating( void ) { m_bApplyAcceleration = true; }
		void StopAccelerating( void ) { m_bApplyAcceleration = false; }

		void SetAngularAcceleration( float fAngularAcceleration ) { m_fAcceleration = fAngularAcceleration; }
		void SetMaxAngularSpeed( float fMaxSpeed ) { m_fMaxSpeed = fMaxSpeed; }

	protected:
		CPoint m_obCentrePoint;
		float m_fRadius;
		float m_fInitialSpeed;
		float m_fAcceleration;
		float m_fMaxSpeed;

		bool m_bOrbitalCameraActive;
		bool m_bDoCutOnActivation;
		bool m_bApplyAcceleration;
};

class CoolCam_AerialGeneral : public CoolCam_RootBoss
{
	public:
		explicit CoolCam_AerialGeneral( const CamView& view, const CoolCam_AerialGeneralDef* pCamDef );
		virtual ~CoolCam_AerialGeneral( void );

		//virtual void        Update(float fTimeDelta);
		virtual void        Reset() {}
		virtual bool        HasFinished() const   {return m_bFinished;}
		virtual CHashedString GetCameraName() const {return "*Boss Cam*";}
		virtual CAMTYPE     GetType() const       {return CT_BOSS;}

	#ifndef _RELEASE
		virtual void RenderDebugInfo() {;}
	#endif

	protected:
		CPoint CalcDesiredCameraPosition( CEntity& obPlayer, CPoint& obBossPosition );
		CPoint CalcDesiredCameraLookatPoint( CEntity& obPlayer, CPoint& obBossPosition );

		const CoolCam_AerialGeneralDef* m_pCamDef;

		float m_fSeperationDistanceModifier;

		CDirection m_obFocusToCameraDir;
		CDirection m_obCamVerticalOffset;

		bool m_bOrbitCameraActive;
		float m_fOrbitCamSpeed;
		CPoint m_obOrbitStartPoint;
		float m_fOrbitAngle;
};


#endif
