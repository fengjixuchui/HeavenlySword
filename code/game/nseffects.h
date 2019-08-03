//------------------------------------------------------------------------------------------
//!
//!	nseffects.h
//!
//! Ninja Sequence Effects header
//!
//------------------------------------------------------------------------------------------

#ifndef _NSEFFECTS_H
#define _NSEFFECTS_H

#ifndef _TRANSFORM_H
#include "anim/transform.h"
#endif

#define USE_PARTICLE_INDICATORS

class NSConditionStorage;
typedef ntstd::Vector< NSConditionStorage* > NSCondList;

//------------------------------------------------------------------------------------------
//!
//!	NS_EFFECT_STATUS
//! Set the status of effects
//!
//------------------------------------------------------------------------------------------
enum NS_EFFECT_STATUS
{
	NSFX_INACTIVE,
	NSFX_ACTIVE,
	NSFX_SUCCEEDED,
	NSFX_FAILDED,
};

//------------------------------------------------------------------------------------------
//!
//!	NSIndicatorEffects
//! Class that handles creation and managment of NS indicator effects.
//!
//------------------------------------------------------------------------------------------
class NSIndicatorEffects
{
public:
	NSIndicatorEffects();
	~NSIndicatorEffects();

	void	Reset();
	void	Update( bool bInWindow, const NSCondList& conditionList );
	void	SetStatus( NS_EFFECT_STATUS status, const NSCondList& conditionList );

private:
	enum NS_INDICATOR_EFFECT
	{
		NSFX_PRESS_DIR_N = 0,
		NSFX_PRESS_DIR_NE,
		NSFX_PRESS_DIR_E,
		NSFX_PRESS_DIR_SE,
		NSFX_PRESS_DIR_S,
		NSFX_PRESS_DIR_SW,
		NSFX_PRESS_DIR_W,
		NSFX_PRESS_DIR_NW,

		NSFX_COUNTER,
		NSFX_GRAB,
		NSFX_ACTION,
		NSFX_ATTACK,

		NSFX_COUNT,
	};

	void	SetEffectStatus( NS_INDICATOR_EFFECT type, NS_EFFECT_STATUS status );
	void	UpdateTransforms();

	bool				m_bInWindow;
	NS_EFFECT_STATUS	m_effectStatus[ NSFX_COUNT ];
	u_int				m_effectIDs[ NSFX_COUNT ];
	Transform			m_effectTransforms[ NSFX_COUNT ];

	static bool	g_bResourcesLoaded;
	static void* g_indicatorDefs[NSFX_COUNT];
	static void* g_successDefs[NSFX_COUNT];
	static void* g_failureDefs[NSFX_COUNT];
};

#endif // _NSEFFECTS_H

