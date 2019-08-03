//------------------------------------------------------------------------------------------
//!
//!	\file CamCool_kingBossFightv2.h
//!
//------------------------------------------------------------------------------------------

#ifndef _COOLCAM_KING_BOSS_FIGHT_V2_H
#define _COOLCAM_KING_BOSS_FIGHT_V2_H

#include "camera/coolcam_rootboss.h"

class CEntity;
class CamView;
class CoolCam_KingBossFightv2;

class CoolCam_KingBossFightv2Def : public CoolCam_RootBossDef
{
	public:
		HAS_INTERFACE( CoolCam_KingBossFightv2Def );

		explicit CoolCam_KingBossFightv2Def( void );

		CoolCam_KingBossFightv2* Create( const CamView& view );

		float GetBossHeightThreshold( void ) const { return m_fBossHeightThreshold; }
		float GetBossHeightCamDistanceModifier( void ) const { return m_fBossHeightCamDistanceModifier; }

	protected:
		float m_fBossHeightThreshold;
		float m_fBossHeightCamDistanceModifier;
};

class CoolCam_KingBossFightv2 : public CoolCam_RootBoss
{
	public:
		explicit CoolCam_KingBossFightv2( const CamView& view, const CoolCam_KingBossFightv2Def* pCamDef );
		virtual ~CoolCam_KingBossFightv2( void );

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

		//void DebugTestRender ( void ) const;

		const CoolCam_KingBossFightv2Def* m_pCamDef;

		float m_fSeperationDistanceModifier;

		CDirection m_obFocusToCameraDir;
		CDirection m_obCamVerticalOffset;

		float m_fCameraPullbackMultiplier;

		CDirection m_obFocusVerticalOffset;

		bool m_bReverseModeActivated;
		CPoint m_obCachedPosition;
};

#endif
