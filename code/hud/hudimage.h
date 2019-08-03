#ifndef _HUDIMAGE_H
#define _HUDIMAGE_H

// Necessary includes
#include "hud/hudoverlay.h"

#include "hud/hudunit.h"
#include "effect/screensprite.h"

#include "editable/enumlist.h"

// Forward Declarations

#define MAX_FRAMES (10)
//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderDef
//!	Defines sprites and sizes for HudImageRenderer to use
//!
//------------------------------------------------------------------------------------------
class HudImageRenderDef : public CHudUnitDef
{
	HAS_INTERFACE( HudImageRenderDef );

public:
	HudImageRenderDef();
	~HudImageRenderDef();

	virtual CHudUnit* CreateInstance( void ) const;
	
	float m_fWidth;
	float m_fHeight;

	//ntstd::List<CKeyString> 
	ntstd::List<CKeyString, Mem::MC_MISC> m_aobFrameList;

	//CKeyString  m_aobFrameList[MAX_FRAMES];
	//u_int m_iTotalFrames;

	bool m_bLooping;
	u_int m_iFramesPerSecond;

	float m_fBlendTime;

	EFFECT_BLENDMODE m_eBlendMode;
	
	friend class HudImageRenderer;

	void SetBaseOverlay(const HudImageOverlaySlap& obNewOverlay )	{ m_obBaseOverlay = obNewOverlay; }
	const HudImageOverlaySlap& GetBaseOverlay( void ) const	{	return m_obBaseOverlay;	}
private:
	HudImageOverlaySlap m_obBaseOverlay;

	void PostConstruct( void );
};

typedef ntstd::List<CKeyString, Mem::MC_MISC>::iterator FrameIter;

//------------------------------------------------------------------------------------------
//!
//!	HudImageRenderer
//!	Renders the Hud Text
//!
//------------------------------------------------------------------------------------------
class HudImageRenderer : public CHudUnit
{
public:
	HudImageRenderer(HudImageRenderDef*  pobRenderDef);
	~HudImageRenderer();

	virtual bool Render( void );
	virtual bool Initialise( void );

	enum OverlayApplication
	{
		OA_IMM =0,
		OA_TRANS
	};
	
	void ApplyOverlay( const HudImageOverlaySlap& obApplyTo,
					   int	iOverlayFilter = HudImageOverlay::OF_ALL,
					   OverlayApplication eApp = OA_IMM, 
					   float fTransTime = 0.0f);

	void DampenIncrementals(float fDampenFactor);
	void ApplyIncrementalOverlay(const HudImageOverlayIncremental& obApply);
	void RemoveAnyIncrementalOverlays( void );
	bool HasIncrementalOverlays( void )	const {	return m_obAppliedIncrementalOverlays.size()>0;	}

	bool InOverlayTransition( void ) const	{	return m_fOverlayParam>0.0f;	}

	const HudImageRenderDef* 	 	GetRenderDef( void ) const		{ 	return	m_pobRenderDef; 	}
	const HudImageOverlaySlap& 		 GetCurrentOverlay( void ) const	{	return m_obAppliedOverlay;	}

	void  Reset(float fTimeToReset);

private:
	void Reset();

    HudImageRenderDef* 	  			m_pobRenderDef;
	int								m_iOverlayFilter;
	float							m_fOverlayParam;
	float							m_fOverlayParamInvLength;
	
	bool							m_bPendingOverlay;
	
	float 							m_fFrameTime;
	float 							m_fCurrentTime;
	//u_int 							m_iCurrentFrame;

	FrameIter		pobCurrentFrame;
	
	// scee.sbashow - for the static overlays
	HudImageOverlaySlap 							m_obTargOverlay;
	HudImageOverlaySlap 							m_obSourceOverlay;
	HudImageOverlaySlap 							m_obAppliedOverlay;
	
	ntstd::List<HudImageOverlayIncremental*>		m_obAppliedIncrementalOverlays;


	ScreenSprite 					m_obImageSprite;
	
	
	bool 			UpdateOverlays( float fTimeChange );
	void 			RenderWithOverlay(const HudImageOverlaySlap& obSlapper );

	virtual bool 	UpdateImage( float fTimeChange);

	virtual void	UpdateEnter( float fTimestep );
	virtual void	UpdateInactive( float fTimestep );
	virtual void	UpdateActive( float fTimestep );
	virtual void	UpdateExit( float fTimestep );

	virtual void	SetStateEnter( void );
	virtual void	SetStateExit( void );



};

#endif // _HUDIMAGE_H
