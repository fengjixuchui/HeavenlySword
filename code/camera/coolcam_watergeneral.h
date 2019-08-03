//------------------------------------------------------------------------------------------
//!
//!	\file CamCool_WaterGeneral.h
//!
//------------------------------------------------------------------------------------------

#ifndef _COOLCAM_WATER_GENERAL_H
#define _COOLCAM_WATER_GENERAL_H

#include "camera/coolcam_rootboss.h"

class CEntity;
class CamView;
class CoolCam_WaterGeneral;

class CoolCam_WaterGeneralDef : public CoolCam_RootBossDef
{
	public:
		HAS_INTERFACE( CoolCam_WaterGeneralDef );

		explicit CoolCam_WaterGeneralDef( void );

		CoolCam_WaterGeneral* Create( const CamView& view );

	protected:
};

class CoolCam_WaterGeneral : public CoolCam_RootBoss
{
	public:
		explicit CoolCam_WaterGeneral( const CamView& view, const CoolCam_WaterGeneralDef* pCamDef );
		virtual ~CoolCam_WaterGeneral( void );

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

		const CoolCam_WaterGeneralDef* m_pCamDef;

		float m_fSeperationDistanceModifier;

		CDirection m_obFocusToCameraDir;
		CDirection m_obCamVerticalOffset;

};

#endif
