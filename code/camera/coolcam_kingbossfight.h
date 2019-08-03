//------------------------------------------------------------------------------------------
//!
//!	\file CamCool_kingBossFight.h
//!
//------------------------------------------------------------------------------------------

#ifndef _COOLCAM_KING_BOSS_FIGHT_H
#define _COOLCAM_KING_BOSS_FIGHT_H

#include "camera/coolcam_rootboss.h"

class CEntity;
class CamView;
class CoolCam_KingBossFight;

class CoolCam_KingBossFightDef : public CoolCam_RootBossDef
{
	public:
		HAS_INTERFACE( CoolCam_KingBossFightDef );

		explicit CoolCam_KingBossFightDef( void );

		CoolCam_KingBossFight* Create( const CamView& view );

		float GetMinCameraHeight( void ) const { return m_fMinCameraHeight; }
		float GetCameraVerticalModifier( void ) const { return m_fCameraVerticalModifier; }
		float GetFocusOffsetModifier( void ) const { return m_fFocusOffsetModifier; }
		float GetBossHeightAbovePlayerScaler( void ) const { return m_fBossHeightAbovePlayerScaler; }

	protected:
		float m_fMinCameraHeight;
		float m_fCameraVerticalModifier;
		float m_fFocusOffsetModifier;
		float m_fBossHeightAbovePlayerScaler;
};

class CoolCam_KingBossFight : public CoolCam_RootBoss
{
	public:
		explicit CoolCam_KingBossFight( const CamView& view, const CoolCam_KingBossFightDef* pCamDef );
		virtual ~CoolCam_KingBossFight( void );

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

		void DebugTestRender ( void ) const;

		const CoolCam_KingBossFightDef* m_pCamDef;

		float m_fSeperationDistanceModifier;

		CDirection m_obFocusToCameraDir;
		CDirection m_obCamVerticalOffset;

		float m_fCameraPullbackMultiplier;

		CDirection m_obFocusVerticalOffset;
};

#endif
