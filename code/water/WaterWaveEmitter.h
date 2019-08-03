//--------------------------------------------------
//!
//!	\file WaveEmitter.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _WATERWAVEEMITTER_H_
#define _WATERWAVEEMITTER_H_

#include "editable/enumlist.h"
#include "tbd/xmlfileinterface.h"


struct WaveDma;
class WaterInstance;	

struct WaveEmitter
{
	HAS_INTERFACE( WaveEmitter )

	WaveEmitter() : m_fTimeSinceLastEmission(0), m_pobWaterInstance(0) {}
	void PostPostConstruct( void );
	bool EditorChangeValue(CallBackParameter, CallBackParameter);

	bool Update( float fTimeStep );

	bool CreateWave( WaveDma* pWave, bool bForceOverwrite = false );
	
	void InstallParentWaterInstance( WaterInstance* pobWaterInstance );
	WaterInstance* GetParentWaterInstance( void ) { return m_pobWaterInstance; }

	//
	//	Transform 
	//
	CPoint			m_obPos;						//!< emitter position. Emitted waves originate from here
	
	//
	// These can vary randomly
	//
	CDirection		m_obAvgWaveDir;					//!< avg wave direction (when relevant of course)			
	float			m_fAvgWaveAmplitude;			//!< average wave amplitude
	float			m_fAvgWavelength;				//!< average wave wavelength
	float			m_fAvgWaveLifespan;				//!< average wave lifespan
	float			m_fAvgWaveSpeed;				//!< average wave horizontal speed magnitude
	float			m_fAvgWaveSharpness;			//!< average wave sharpness
	float			m_fRandomness;					//!< randomness control [0.0f, 1.0f]. Will vary current attribute +/- that percent
	
	//
	// same for every wave emitted
	//
	float			m_fAttLinear;					//!< distance attenuation linear
	float			m_fAttQuadratic;				//!< distance attenuation quadratic
	float			m_fFadeInTime;					//!< waves fade-in time
	float			m_fFadeOutTime;					//!< waves fade-out time. Please observe: ( FadeInTime + FadeOutTime ) < ( AvgMaxAge - Randomness * AvgMaxAge )

	WAVE_TYPE		m_eType;						//!< emitter type
	
	float			m_Attack0_fWidth;				
	float			m_Attack0_fFalloff;		
	float			m_Attack1_fBackTrail;
	float			m_Attack1_fFrontTrail;				
	float			m_Attack2_fRadius;

	float			m_fEmissionRate;				//!< bad name but too late. Emit every X secs
	float			m_fTimeSinceLastEmission;		//!< self-explanatory
	bool			m_bAutoEmit;						//!< ehrrr...
	int				m_iPreferredSlot;

private:
	void ClampValues( void );

	void ClampValues( const WaterInstance* pobWaterInstance );

private:
	friend class WaterInstance;
	WaterInstance*				m_pobWaterInstance;
};



#endif // end of _WATERWAVEEMITTER_H_
