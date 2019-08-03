//--------------------------------------------------
//!
//!	\file effect_error.cpp
//!	Simple ntError display class
//!
//--------------------------------------------------

#include "effect_error.h"

#ifndef _GOLD_MASTER

#include "effect/effect_manager.h"
#include "core/timer.h"
#include "core/visualdebugger.h"
#include "game/shelldebug.h"

//--------------------------------------------------
//!
//! this will do untill we have an ntError message system
//!
//--------------------------------------------------
void EffectErrorMSG::AddDebugError( const char* pMSG, float fDuration )
{
	UNUSED(pMSG);
	UNUSED(fDuration);
#ifndef _RELEASE
	EffectManager::Get().AddEffect( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) EffectErrorMSG( pMSG, fDuration ) );	
#endif
}

//--------------------------------------------------
//!
//!	EffectErrorMSG::RenderEffect
//! draw our ntError messages
//!
//--------------------------------------------------
#define iMAX_VIEWABLE 70

void EffectErrorMSG::RenderEffect()
{
	static u_int s_iFrameCount = 0xffffffff;
	static float s_fStartLine = sfErrorTopBorder;

	if (s_iFrameCount != CTimer::Get().GetSystemTicks())
	{
		s_iFrameCount = CTimer::Get().GetSystemTicks();
		s_fStartLine = sfErrorTopBorder;
	}

	int iMessageSize = strlen( m_pMSG );
	int iOffset = 0;

	do
	{	
		float fCol = sinf(sinf((m_fRemaining / m_fAge) * HALF_PI)*HALF_PI);
		uint32_t dwCol = NTCOLOUR_FROM_FLOATS( fCol, 0.0f, 0.0f, fCol );

		char aTemp[iMAX_VIEWABLE+1];
		strncpy( aTemp, m_pMSG+iOffset, iMAX_VIEWABLE );
		aTemp[iMAX_VIEWABLE] = 0;

		if (iOffset == 0)
			g_VisualDebug->Printf2D(sfDebugLeftBorder, s_fStartLine, dwCol, 0, aTemp );
		else
			g_VisualDebug->Printf2D(sfDebugLeftBorder, s_fStartLine, dwCol, 0, "-%s", aTemp );

		s_fStartLine += sfDebugLineSpacing;
		iMessageSize-=iMAX_VIEWABLE;
		iOffset+=iMAX_VIEWABLE;
	}
	while (iMessageSize > 0);
	
	s_fStartLine += sfDebugLineSpacing;
}

#endif

