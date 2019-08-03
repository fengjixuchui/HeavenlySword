//--------------------------------------------------
//!
//!	\file emission_function.h
//!	Defines an arbitary emmitter that can be used
//! by a given effect
//!
//--------------------------------------------------

#ifndef _E_FUNC_H
#define _E_FUNC_H

#include "editable/enumlist.h"
#include "effect_util.h"
#include "functioncurve.h"
#include "effect_resetable.h"
#include "effect/effect_resourceman.h"

class EmitterSimpleDef;

//--------------------------------------------------
//!
//!	EmitterState
//! Corresponds to state of emitter at a given moment
//! Animated emitters work by generating this block 
//! every frame
//!
//--------------------------------------------------
class EmitterState
{
public:
	EmitterState() :
		m_offset( CONSTRUCT_CLEAR ),
		m_dimensions( CONSTRUCT_CLEAR ),
		m_fInheritVelScalar(0.0f),
		m_fAngleMin(0.0f),
		m_fAngleMax(0.0f),
		m_fVelMin(1.0f),
		m_fVelMax(1.0f),
		m_volumeType( EV_CUBE ),
		m_bInheritTransVel(false),
		m_bUseSpawnVolMat(false),
		m_bUseSpawnDirMat(false)
	{}

	void SetSpawnVolMatrix( const CDirection& spawnVolYPR );
	void SetSpawnDirMatrix( const CDirection& spawnDirYPR );
	void DebugRender( const CMatrix& frame, bool bSpawnDirInWorld ) const;

	inline CPoint GetNewPosition() const;
	inline CPoint GetNewPositionNoOffset() const;
	inline CDirection GetNewVelocity() const;

	CPoint		m_offset;
	CDirection	m_dimensions;
	float		m_fInheritVelScalar;
	float		m_fAngleMin;
	float		m_fAngleMax;
	float		m_fVelMin;
	float		m_fVelMax;
	EMISSON_VOLUME_TYPE	m_volumeType;
	bool		m_bInheritTransVel;

private:
	bool		m_bUseSpawnVolMat;
	bool		m_bUseSpawnDirMat;
	CMatrix		m_spawnVolMat;
	CMatrix		m_spawnDirMat;
};

//--------------------------------------------------
//!
//! EmitterState::GetNewPosition
//!	return random point based on our state
//!
//--------------------------------------------------
inline CPoint EmitterState::GetNewPosition() const
{
	CPoint result( CONSTRUCT_CLEAR );

	switch( m_volumeType )
	{
	case EV_CUBE:
		result = EffectUtils::RandomPointInCube() * m_dimensions;
		break;

	case EV_SPHERE:
		result = EffectUtils::RandomPointInSphere() * m_dimensions;
		break;

	case EV_CYLINDER:
		result = EffectUtils::RandomPointInCylinder() * m_dimensions;
		break;
	}

	if (m_bUseSpawnVolMat)
		result = result * m_spawnVolMat;

	result += m_offset;
	return result;
}

//--------------------------------------------------
//!
//! EmitterState::GetNewPositionNoOffset
//!	return random point based on our state
//!
//--------------------------------------------------
inline CPoint EmitterState::GetNewPositionNoOffset() const
{
	CPoint result( CONSTRUCT_CLEAR );

	switch( m_volumeType )
	{
	case EV_CUBE:
		result = EffectUtils::RandomPointInCube() * m_dimensions;
		break;

	case EV_SPHERE:
		result = EffectUtils::RandomPointInSphere() * m_dimensions;
		break;

	case EV_CYLINDER:
		result = EffectUtils::RandomPointInCylinder() * m_dimensions;
		break;
	}

	if (m_bUseSpawnVolMat)
		result = result * m_spawnVolMat;

	return result;
}

//--------------------------------------------------
//!
//! EmitterState::GetNewPosition
//!	return velocity (in meters per second) based on 
//! our state.
//!
//--------------------------------------------------
inline CDirection EmitterState::GetNewVelocity() const
{
	float fVelocity		= erandf( m_fVelMax - m_fVelMin ) + m_fVelMin;
	float fLatitude		= (erandf( m_fAngleMax - m_fAngleMin ) + m_fAngleMin) * DEG_TO_RAD_VALUE;
	float fLongditude	= erandf( TWO_PI );

	float SX, SY, CX, CY;
	CMaths::SinCos( fLatitude, SX, CX );
	CMaths::SinCos( fLongditude, SY, CY );

	CDirection result( CDirection(SX*SY, CX, SX*CY) * fVelocity );

	if (m_bUseSpawnDirMat)
		return result * m_spawnDirMat;
	else
		return result;	
}

//--------------------------------------------------
//!
//!	EmitterDef
//! Base class for simple and complex emitter defs
//!
//--------------------------------------------------
class EmitterDef
{
public:
	EmitterDef() :
		m_iLoopCount( -1 ),
		m_iEmitPerLoop( 10 ),	
		m_fLoopDuration( 1.0f ),
		m_bSpawnDirInWorld( false ),
		m_bWorldSpaceEffect( true )
	{}

	virtual ~EmitterDef(){};

	// for housekeeping
	ResetSet<Resetable> m_resetSet;

	virtual void InitState( EmitterState& result ) const = 0;
	virtual void RetriveState( EmitterState& result, float fTimeN ) const = 0;
	virtual const FunctionCurve_Fitted* GetEmissionFunction() const = 0;

	// retrive the maximum possible emittees using this emitter
	u_int CalcMaximumEmitted( float fEmitteeLifetime ) const
	{
		float fEmitRatio = fEmitteeLifetime / m_fLoopDuration;
		u_int iResult = ntstd::Max( m_iEmitPerLoop, (u_int)(m_iEmitPerLoop * fEmitRatio));

		if (m_iLoopCount > 0)
			iResult = ntstd::Min( iResult, m_iLoopCount * m_iEmitPerLoop );

		return iResult;
	}

	int		m_iLoopCount;
	u_int	m_iEmitPerLoop;
	float	m_fLoopDuration;
	bool	m_bSpawnDirInWorld;
	bool	m_bWorldSpaceEffect;
};

//--------------------------------------------------
//!
//!	EmissionFunctionResource
//!
//--------------------------------------------------
class EmissionFunctionResource : public EffectResource
{
public:
	EmissionFunctionResource();
	virtual ~EmissionFunctionResource();

	virtual void GenerateResources();
	virtual bool ResourcesOutOfDate() const;

	void SetParent( const EmitterDef* pParent ) { m_pParent = pParent; }

	FunctionCurve_User*		m_pFunction;
	FunctionCurve_Fitted*	m_pEmissionFunction;

private:
	const EmitterDef*	m_pParent;
};

//--------------------------------------------------
//!
//!	EmitterSimpleDef
//! Simple Emitter type, used as a default for most
//! particle systems
//!
//--------------------------------------------------
class EmitterSimpleDef : public EmitterDef
{
public:
	HAS_INTERFACE(EmitterSimpleDef);
	
	EmitterSimpleDef() :
		m_spawnVolYPR( CONSTRUCT_CLEAR ),
		m_spawnDirYPR( CONSTRUCT_CLEAR )
	{}

	EmissionFunctionResource	m_function;
	
	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter );

	// Interfaces from EmitterDef
	virtual void InitState( EmitterState& result ) const { NT_MEMCPY( &result, &m_defaults, sizeof(EmitterState) ); }
	virtual void RetriveState( EmitterState& result, float ) const
	{
		// for live editing only
		UNUSED(result);
		#ifndef _RELEASE
		InitState( result );
		#endif
	};

	virtual const FunctionCurve_Fitted* GetEmissionFunction() const { return m_function.m_pEmissionFunction; }

private:
	EmitterState m_defaults;
	CDirection	m_spawnVolYPR;
	CDirection	m_spawnDirYPR;
};

//--------------------------------------------------
//!
//!	EmitterComplexDef
//! Compelex Emitter type, has animated emission params
//!
//--------------------------------------------------
class EmitterComplexDef : public EmitterDef
{
public:
	HAS_INTERFACE(EmitterComplexDef);
	EmitterComplexDef();

	EmissionFunctionResource	m_function;
	
	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter );

	// Interfaces from EmitterDef
	virtual void InitState( EmitterState& result ) const { RetriveState( result, 0.0f ); }
	virtual void RetriveState( EmitterState& result, float fTimeN ) const;

	virtual const FunctionCurve_Fitted* GetEmissionFunction() const { return m_function.m_pEmissionFunction; }

private:
	EmitterState m_defaults;
	CDirection	m_spawnVolYPR;
	CDirection	m_spawnDirYPR;

	// m_offset
	FunctionCurve_User*		m_pEditableOffsetX;
	FunctionCurve_User*		m_pEditableOffsetY;
	FunctionCurve_User*		m_pEditableOffsetZ;

	// m_dimensions
	FunctionCurve_User*		m_pEditableDimensionX;
	FunctionCurve_User*		m_pEditableDimensionY;
	FunctionCurve_User*		m_pEditableDimensionZ;

	// m_fInheritVelScalar
	FunctionCurve_User*		m_pEditableVelScalar;

	// m_fAngleMin, m_fAngleMax
	FunctionCurve_User*		m_pEditableAngleMin;
	FunctionCurve_User*		m_pEditableAngleMax;

	// m_fVelMin, m_fVelMax
	FunctionCurve_User*		m_pEditableVelMin;
	FunctionCurve_User*		m_pEditableVelMax;

	// m_spawnVolYPR
	FunctionCurve_User*		m_pEditableVolY;
	FunctionCurve_User*		m_pEditableVolP;
	FunctionCurve_User*		m_pEditableVolR;

	// m_spawnDirYPR
	FunctionCurve_User*		m_pEditableDirY;
	FunctionCurve_User*		m_pEditableDirP;
	FunctionCurve_User*		m_pEditableDirR;
};




//--------------------------------------------------
//!
//!	Emitter
//! class that calls the method T::EmitParticle inside
//! its templated Update() function when it is time to
//! emit a particle according to its definition.
//! T::EmitParticle should take a float in the
//! range 0 -> 1, denoting how far we are between
//! m_fAge and m_fOldAge
//!
//--------------------------------------------------
class Emitter
{
public:
	Emitter( const EmitterDef& def ) :
		m_pDef(&def),
		m_fAge(0.0f),
		m_fOldAge(0.0f),
		m_fAccumulator(0.0f),
		m_iNumLoops(0),
		m_iEmittedThisLoop(0)
	{
		m_pDef->InitState( m_currState );
	}

	const EmitterDef&	GetDef()		const { return *m_pDef; }
	const EmitterState& GetCurrState()	const { return m_currState; }

	float GetAge()		const { return m_fAge; }
	float GetOldAge()	const { return m_fOldAge; }

	template<class T> bool Update( T* particleSys, bool bEmitNoMore, float fTimeDelta, float fEmitMultiplier );

	void DebugRender();

private:
	const EmitterDef*	m_pDef;
	EmitterState		m_currState;
	float				m_fAge;
	float				m_fOldAge;
	float				m_fAccumulator;
	int					m_iNumLoops;
	u_int				m_iEmittedThisLoop;
};

//--------------------------------------------------
//!
//!	Emitter::Update
//!
//--------------------------------------------------
template<class T> inline bool Emitter::Update( T* particleSys, bool bEmitNoMore, float fTimeDelta, float fEmitMultiplier )
{
	ntAssert( fTimeDelta > 0.0f );
	fEmitMultiplier = ntstd::Min( fEmitMultiplier, 1.0f ); // this must never be larger than 1
	
	// update our age, state and loop counters
	//-------------------------------------------------------------------------------------

	m_fOldAge = m_fAge;
	m_fAge += fTimeDelta;

	float fNumLoops = (m_fAge / m_pDef->m_fLoopDuration);
	int iNumLoops = (int)floor(fNumLoops);
	m_pDef->RetriveState( m_currState, fNumLoops - _R(iNumLoops) );

	if (iNumLoops != m_iNumLoops)
	{
		// we just crossed over to the next loop
		m_iNumLoops = iNumLoops;
		m_iEmittedThisLoop = 0;
	}

	bool bFinished =	((m_pDef->m_iLoopCount > 0) && // non-zero means we have a finite loop count, then auto-destruct
						(m_iNumLoops >= m_pDef->m_iLoopCount)) // we are about to expire		
						? true : bEmitNoMore;

	if ((bFinished) || (m_iEmittedThisLoop >= m_pDef->m_iEmitPerLoop))
	{
		// we're full for the moment
		return bFinished;
	}

	// use our emmision function to generate the number of particles required this frame.
	//-------------------------------------------------------------------------------------

	int iNumIntegrals = 10;
	float fIntegralStep = fTimeDelta / iNumIntegrals;
	float fRCPDelta = 1.0f / fTimeDelta;
	const FunctionCurve_Fitted* pEmitFunc =  m_pDef->GetEmissionFunction();

	for (int i = 0; i < iNumIntegrals; i++)
	{
		float fStartTime	= m_fOldAge + (i*fIntegralStep);
		float fEndTime		= m_fOldAge + ((i+1)*fIntegralStep);

		float fNumParticles = 0.0f;

		if ( pEmitFunc )
		{
			// get time in terms of m_fDuration multiples
			float fU = (fStartTime / m_pDef->m_fLoopDuration);
			float fV = (fEndTime / m_pDef->m_fLoopDuration);

			float fFunctionTimeRange = fV - fU;

			// remove floor to get normalised time to index function with
			fU -= floorf( fU );
			fV -= floorf( fV );

			// trapezium rule to get discrete integral for this function segment
			float fDiscreteIntegral = ( pEmitFunc->Evaluate( fU ) + pEmitFunc->Evaluate( fV ) ) * 0.5f * fFunctionTimeRange;
			fDiscreteIntegral = ntstd::Max( fDiscreteIntegral, 0.0f );
			
			// get this integral as a proportion of the total area under the curve
			float fFraction = fDiscreteIntegral / pEmitFunc->GetFunctionIntegral();

			// get total particles to emit
			fNumParticles = m_pDef->m_iEmitPerLoop * fFraction;
		}
		else
		{
			// simple constant rate emission
			fNumParticles = (fIntegralStep / m_pDef->m_fLoopDuration) * m_pDef->m_iEmitPerLoop;
		}

		m_fAccumulator += fNumParticles * fEmitMultiplier;

		float fAverageInterval = fIntegralStep / m_fAccumulator;
		while ( m_fAccumulator >= 1.0f )
		{
			if (m_iEmittedThisLoop < m_pDef->m_iEmitPerLoop)
			{
				float fNormalisedT = (fStartTime - m_fOldAge) * fRCPDelta;
				particleSys->EmitParticle( fNormalisedT );
				m_iEmittedThisLoop++;
				fStartTime += fAverageInterval;
				m_fAccumulator -= 1.0f;
			}
			else
			{
				m_fAccumulator = 0.0f;
				return false;
			}
		}
	}
	return false;
}

#endif // _E_FUNC_H
