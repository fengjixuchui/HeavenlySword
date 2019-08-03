//! -------------------------------------------
//! aihearing.h
//!
//! AI Hearing
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIHEARING_H
#define _AIHEARING_H

#include "game/entity.inl"
#include "game/entity.h"

// Forward declaration of classes and structures

//class CEntity;


//!--------------------------------------------
//! SHearingParams
//!--------------------------------------------

typedef struct _SHearingParams
{
	_SHearingParams() :	fVolumeThreshold(0), bDeaf(true) {}

	float	fVolumeThreshold;
	bool	bDeaf;
} SHearingParams;


//!--------------------------------------------
//! CAIHearing
//!--------------------------------------------
class CAIHearing
{
	public:

		// Ctor...
		CAIHearing() :	m_LastKnownSoundSource(CONSTRUCT_CLEAR), m_pSoundSourceEntity(NULL) {}

		// Set/Get Parameters

		void			SetDeafen				( bool b )		{ m_Params.bDeaf = b;}
		void			SetVolumeThreshold		( float fValue)	{ m_Params.fVolumeThreshold = fValue; }
		bool			IsDeaf					( void ) const	{ return m_Params.bDeaf; }
		float			GetVolumeThreshold		( void ) const	{ return m_Params.fVolumeThreshold; }
		void			SetSoundSourceLocation	( const CPoint & obPos ) { m_LastKnownSoundSource = obPos; }
		CPoint			GetSoundSourceLocation	( void ) { return m_LastKnownSoundSource; }
		void			SetSoundSourceEntity	( CEntity* pE ) { m_pSoundSourceEntity = pE; m_LastKnownSoundSource = pE ? pE->GetPosition() : CPoint(CONSTRUCT_CLEAR); }
		CEntity*		GetSoundSourceEntity	( void ) { return m_pSoundSourceEntity; }

	private:
		
		SHearingParams	m_Params;
		CPoint			m_LastKnownSoundSource;
		CEntity*		m_pSoundSourceEntity;
};

#endif //_AIHEARING_H




