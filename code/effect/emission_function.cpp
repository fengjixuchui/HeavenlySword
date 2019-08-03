//--------------------------------------------------
//!
//!	\file emission_function.cpp
//!	Defines an arbitary emmitter that can be used
//! by a given effect
//!
//--------------------------------------------------

#include "emission_function.h"
#include "objectdatabase/dataobject.h"
#include "camera/camutils.h"
#include "functiongraph.h"
#include "effect_error.h"
#include "core/visualdebugger.h"

START_STD_INTERFACE( EmitterSimpleDef )

	I2INT		( m_iLoopCount,					LoopCount )							
	I2INT		( m_iEmitPerLoop,				EmitPerLoop )						
	I2FLOAT		( m_fLoopDuration,				LoopDuration )						
	I2REFERENCE	( m_function.m_pFunction,		EmissionFunction )					

	// spawning position parameters
	I2ENUM		( m_defaults.m_volumeType,		VolumeType,	EMISSON_VOLUME_TYPE )	
	I2POINT		( m_defaults.m_offset,			Offset )							
	I2VECTOR	( m_defaults.m_dimensions,		Scale )								
	I2VECTOR	( m_spawnVolYPR,				VolumeYPR(deg) )					
	
	// spawning velocity parameters
	I2FLOAT		( m_defaults.m_fAngleMin,		AngleMin(deg) )						
	I2FLOAT		( m_defaults.m_fAngleMax,		AngleMax(deg) )						
	I2FLOAT		( m_defaults.m_fVelMin,			VelocityMin(m/s) )					
	I2FLOAT		( m_defaults.m_fVelMax,			VelocityMax(m/s) )					

	I2VECTOR	( m_spawnDirYPR,				DirectionYPR(deg) )					
	I2BOOL		( m_bSpawnDirInWorld,			DirectionInWorld )					
	I2BOOL		( m_bWorldSpaceEffect,			WorldSpaceEffect )					

	I2FLOAT		( m_defaults.m_fInheritVelScalar,	InheritParentVelScalar )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

START_STD_INTERFACE( EmitterComplexDef )

	I2INT		( m_iLoopCount,					LoopCount )							
	I2INT		( m_iEmitPerLoop,				EmitPerLoop )						
	I2FLOAT		( m_fLoopDuration,				LoopDuration )						
	I2REFERENCE	( m_function.m_pFunction,		EmissionFunction )					

	// spawning position parameters
	I2ENUM		( m_defaults.m_volumeType,		VolumeType,	EMISSON_VOLUME_TYPE )	
	
	I2POINT		( m_defaults.m_offset,			DefaultOffset )
	I2REFERENCE	( m_pEditableOffsetX,			AnimOffsetX )					
	I2REFERENCE	( m_pEditableOffsetY,			AnimOffsetY )					
	I2REFERENCE	( m_pEditableOffsetZ,			AnimOffsetZ )					

	I2VECTOR	( m_defaults.m_dimensions,		Scale )								
	I2REFERENCE	( m_pEditableDimensionX,		AnimScaleX )					
	I2REFERENCE	( m_pEditableDimensionY,		AnimScaleY )					
	I2REFERENCE	( m_pEditableDimensionZ,		AnimScaleZ )					

	I2VECTOR	( m_spawnVolYPR,				VolumeYPR(deg) )					
	I2REFERENCE	( m_pEditableVolY,				AnimVolumeY )					
	I2REFERENCE	( m_pEditableVolP,				AnimVolumeP )					
	I2REFERENCE	( m_pEditableVolR,				AnimVolumeR )					

	// spawning velocity parameters
	I2FLOAT		( m_defaults.m_fAngleMin,		AngleMin(deg) )						
	I2FLOAT		( m_defaults.m_fAngleMax,		AngleMax(deg) )
	I2REFERENCE	( m_pEditableAngleMin,			AnimAngleMin )					
	I2REFERENCE	( m_pEditableAngleMax,			AnimAngleMax )					

	I2FLOAT		( m_defaults.m_fVelMin,			VelocityMin(m/s) )					
	I2FLOAT		( m_defaults.m_fVelMax,			VelocityMax(m/s) )					
	I2REFERENCE	( m_pEditableVelMin,			AnimVelMin )					
	I2REFERENCE	( m_pEditableVelMax,			AnimVelMax )					

	I2VECTOR	( m_spawnDirYPR,				DirectionYPR(deg) )					
	I2REFERENCE	( m_pEditableDirY,				AnimDirectionY )					
	I2REFERENCE	( m_pEditableDirP,				AnimDirectionP )					
	I2REFERENCE	( m_pEditableDirR,				AnimDirectionR )					

	I2BOOL		( m_bSpawnDirInWorld,			DirectionInWorld )					
	I2BOOL		( m_bWorldSpaceEffect,			WorldSpaceEffect )					

	I2FLOAT		( m_defaults.m_fInheritVelScalar,	InheritParentVelScalar )		
	I2REFERENCE	( m_pEditableVelScalar,				AnimParentVelScalar )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

//--------------------------------------------------
//!
//!	EmitterState::SetSpawnVolMatrix
//! decide to use matrix and set it
//!
//--------------------------------------------------
void EmitterState::SetSpawnVolMatrix( const CDirection& spawnVolYPR )
{
	m_bUseSpawnVolMat = false;

	if (spawnVolYPR.LengthSquared() > 1.0f)
	{
		m_bUseSpawnVolMat = true;
		m_spawnVolMat = CMatrix( CCamUtil::QuatFromYawPitchRoll(spawnVolYPR.X() * DEG_TO_RAD_VALUE,
																spawnVolYPR.Y() * DEG_TO_RAD_VALUE,
																spawnVolYPR.Z() * DEG_TO_RAD_VALUE ), CPoint(0.0f, 0.0f, 0.0f) );
	}
}

//--------------------------------------------------
//!
//!	EmitterState::SetSpawnDirMatrix
//! decide to use matrix and set it
//!
//--------------------------------------------------
void EmitterState::SetSpawnDirMatrix( const CDirection& spawnDirYPR )
{
	m_bUseSpawnDirMat = false;

	if (spawnDirYPR.LengthSquared() > 1.0f)
	{
		m_bUseSpawnDirMat = true;
		m_spawnDirMat = CMatrix( CCamUtil::QuatFromYawPitchRoll(spawnDirYPR.X() * DEG_TO_RAD_VALUE,
																spawnDirYPR.Y() * DEG_TO_RAD_VALUE,
																spawnDirYPR.Z() * DEG_TO_RAD_VALUE ), CPoint(0.0f, 0.0f, 0.0f) );
	}
}

//--------------------------------------------------
//!
//!	EmitterState::DebugRender
//!
//--------------------------------------------------
void EmitterState::DebugRender( const CMatrix& frame, bool bSpawnDirInWorld ) const
{
#ifndef _GOLD_MASTER
	// render our emission volume
	CMatrix emitterVolume( CONSTRUCT_IDENTITY );
	emitterVolume[0][0] = m_dimensions.X();
	emitterVolume[1][1] = m_dimensions.Y();
	emitterVolume[2][2] = m_dimensions.Z();

	if (m_bUseSpawnVolMat)
		emitterVolume = emitterVolume * m_spawnVolMat;

	emitterVolume.SetTranslation( m_offset );
	emitterVolume = emitterVolume * frame;

	switch ( m_volumeType )
	{
	case EV_CUBE:
		g_VisualDebug->RenderCube( emitterVolume, NTCOLOUR_RGBA(255,0,0,128), 0 );
		break;

	case EV_SPHERE:
		g_VisualDebug->RenderSphere( emitterVolume, NTCOLOUR_RGBA(255,0,0,128), 0 );
		break;

	case EV_CYLINDER:
		g_VisualDebug->RenderCube( emitterVolume, NTCOLOUR_RGBA(255,0,0,128), 0 );
		break;
	}

	// render our emission direction
	CMatrix orient(CONSTRUCT_IDENTITY);
		
	if (m_bUseSpawnDirMat)
		orient = m_spawnDirMat;

	orient.SetTranslation( m_offset );
	
	if (bSpawnDirInWorld)
		 EffectUtils::DebugRenderFrame( orient );
	else
		 EffectUtils::DebugRenderFrame( orient * frame );
#endif
}





//--------------------------------------------------
//!
//!	EmissionFunctionResource::ctor
//!
//--------------------------------------------------
EmissionFunctionResource::EmissionFunctionResource() :
	m_pFunction(0),
	m_pEmissionFunction(0)
{
	EffectResourceMan::Get().RegisterResource( *this );
}

//--------------------------------------------------
//!
//! EmissionFunctionResource::dtor
//!
//--------------------------------------------------
EmissionFunctionResource::~EmissionFunctionResource()
{
	if (m_pEmissionFunction)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pEmissionFunction );
		m_pEmissionFunction = 0;
	}

	EffectResourceMan::Get().ReleaseResource( *this );
}

//--------------------------------------------------
//!
//!	EmissionFunctionResource::GenerateResources
//!
//--------------------------------------------------
void EmissionFunctionResource::GenerateResources()
{
	if (m_pFunction)
	{
		if (!m_pEmissionFunction)
			m_pEmissionFunction = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) FunctionCurve_Fitted( m_pFunction->m_fTolerance );

		float fScale = m_pFunction->m_fScale;
		float fOffset = m_pFunction->m_fOffset;

#ifndef _RELEASE
		// check our function for validity
		// ( the problem here is having a negative integral under portions of the curve )
		// ( this is hard to detect without extra code, GetFunctionMin and GetFunctionMax )
		// ( are debug functions that walk the curve, intended for debug rendering )
		float fMin = (m_pFunction->GetFittedCurve()->GetFunctionMin() * fScale)+fOffset;
		float fMax = (m_pFunction->GetFittedCurve()->GetFunctionMax() * fScale)+fOffset;

		fMin = ntstd::Min(fMin, fMax);
		if ( fMin < -0.01f )
		{
			const char* pName = EffectUtils::GetInterfaceName( (void*)m_pParent );
			char ntError[512];
			sprintf( ntError, "Emitter definition: %s has an invalid emission function: it goes negative!", pName );
			
			EffectErrorMSG::AddDebugError( ntError );

			if (m_pEmissionFunction)
			{
				NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pEmissionFunction );
				m_pEmissionFunction = 0; 
			}

			m_bRequireRefresh = false;
			return;
		}
#endif // end _RELEASE

		// we have to copy the emission function as we need to scale and offset its CVs,
		// and scale its tangents, so we can get an integral of the curve
		
		// [scee_st] can't chunk this easily as CCubicTimeHermite is shared with Camera code.
		// We could ignore it
		const CCubicTimeHermite* pOldHermite = &m_pFunction->GetFittedCurve()->GetCurve();

		CVector* pVerts = NT_NEW CVector[ pOldHermite->GetNumCV() ];
		for (u_int i = 0; i < pOldHermite->GetNumCV(); i++ )
		{
			pVerts[i].Clear();
			pVerts[i].X() = (pOldHermite->GetCV(i).X() * fScale) + fOffset;
		}

		CVector* pStarts = NT_NEW CVector[ pOldHermite->GetNumTan() ];
		CVector* pEnds = NT_NEW CVector[ pOldHermite->GetNumTan() ];

		for (u_int i = 0; i < pOldHermite->GetNumTan(); i++ )
		{
			pStarts[i].Clear();
			pStarts[i].X() = pOldHermite->GetStartTan(i).X() * fScale;

			pEnds[i].Clear();
			pEnds[i].X() = pOldHermite->GetEndTan(i).X() * fScale;
		}

		float* pTimes = NT_NEW float[ pOldHermite->GetTimeModule().GetNumTimes() ];
		for (u_int i = 0; i < pOldHermite->GetTimeModule().GetNumTimes(); i++ )
		{
			pTimes[i] = pOldHermite->GetTimeModule().GetTime(i);
		}

		CCubicTimeHermite* pNewHermite = NT_NEW_CHUNK( Mem::MC_EFFECTS ) CCubicTimeHermite( pTimes, true, true, pOldHermite->GetNumCV(), pVerts, pStarts, pEnds, true, true );
		m_pEmissionFunction->Reset();
		m_pEmissionFunction->Finalise( pNewHermite );
		m_pEmissionFunction->CalcFunctionIntegral();
	}
	else if (m_pEmissionFunction)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pEmissionFunction );
		m_pEmissionFunction = 0;
	}

	ResourcesOutOfDate();
	m_bRequireRefresh = false;
}

//--------------------------------------------------
//!
//!	EmissionFunctionResource::ResourcesOutOfDate
//! per frame callback to see if we need regenerating
//!
//--------------------------------------------------
bool EmissionFunctionResource::ResourcesOutOfDate() const
{
	if	(
		(m_pFunction) &&
		(m_pFunction->DetectCurveChanged())
		)
	{
		m_bRequireRefresh = true;
	}
	return m_bRequireRefresh;
}




//--------------------------------------------------
//!
//!	EmitterSimpleDef::PostConstruct
//! Validate parameters
//!
//--------------------------------------------------
void EmitterSimpleDef::PostConstruct()
{
	m_function.SetParent( this );

	m_iEmitPerLoop = ntstd::Max( m_iEmitPerLoop, 1u );
	m_fLoopDuration = ntstd::Max( m_fLoopDuration, EPSILON );

	m_defaults.SetSpawnVolMatrix( m_spawnVolYPR );
	m_defaults.SetSpawnDirMatrix( m_spawnDirYPR );

	if (fabsf(m_defaults.m_fInheritVelScalar) > 0.001f)
		m_defaults.m_bInheritTransVel = true;
	else
		m_defaults.m_bInheritTransVel = false;
}

//--------------------------------------------------
//!
//!	EmitterSimpleDef::Mark for Refresh
//!
//--------------------------------------------------
bool EmitterSimpleDef::EditorChangeValue( CallBackParameter param, CallBackParameter )
{
	CHashedString pName(param);

	PostConstruct();

	if (HASH_STRING_EMISSIONFUNCTION == pName)
	{
		m_function.MarkForRefresh();
	}

	if (HASH_STRING_LOOPCOUNT == pName || HASH_STRING_EMITPERLOOP == pName || HASH_STRING_LOOPDURATION == pName)
	{
		m_resetSet.ResetThings();
	}

	return true;
}




//--------------------------------------------------
//!
//!	EmitterComplexResource::ctor
//!
//--------------------------------------------------
EmitterComplexDef::EmitterComplexDef() :
	m_spawnVolYPR( CONSTRUCT_CLEAR ),
	m_spawnDirYPR( CONSTRUCT_CLEAR ),
	m_pEditableOffsetX(0),
	m_pEditableOffsetY(0),
	m_pEditableOffsetZ(0),
	m_pEditableDimensionX(0),
	m_pEditableDimensionY(0),
	m_pEditableDimensionZ(0),
	m_pEditableVelScalar(0),
	m_pEditableAngleMin(0),
	m_pEditableAngleMax(0),
	m_pEditableVelMin(0),
	m_pEditableVelMax(0),
	m_pEditableVolY(0),
	m_pEditableVolP(0),
	m_pEditableVolR(0),
	m_pEditableDirY(0),
	m_pEditableDirP(0),
	m_pEditableDirR(0)
{}

//--------------------------------------------------
//!
//!	EmitterComplexDef::PostConstruct
//! Validate parameters
//!
//--------------------------------------------------
void EmitterComplexDef::PostConstruct()
{
	m_function.SetParent( this );

	m_iEmitPerLoop = ntstd::Max( m_iEmitPerLoop, 1u );
	m_fLoopDuration = ntstd::Max( m_fLoopDuration, EPSILON );
}

//--------------------------------------------------
//!
//!	EmitterComplexDef::Mark for Refresh
//!
//--------------------------------------------------
bool EmitterComplexDef::EditorChangeValue( CallBackParameter param, CallBackParameter )
{
	CHashedString pName(param);

	PostConstruct();

	if (HASH_STRING_EMISSIONFUNCTION == pName)
	{
		m_function.MarkForRefresh();
	}

	if (HASH_STRING_LOOPCOUNT == pName || HASH_STRING_EMITPERLOOP == pName || HASH_STRING_LOOPDURATION == pName)
	{
		m_resetSet.ResetThings();
	}

	return true;
}

//--------------------------------------------------
//!
//!	EmitterComplexDef::RetriveState
//!
//--------------------------------------------------
void EmitterComplexDef::RetriveState( EmitterState& result, float fTimeN ) const
{
	ntAssert( fTimeN >= 0.0f );
	ntAssert( fTimeN <= 1.0f );

	NT_MEMCPY( &result, &m_defaults, sizeof(EmitterState) );

	if (m_pEditableOffsetX) result.m_offset.X() = m_pEditableOffsetX->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableOffsetY) result.m_offset.Y() = m_pEditableOffsetY->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableOffsetZ) result.m_offset.Z() = m_pEditableOffsetZ->EvaluateScaledAndOffset( fTimeN );
	
	if (m_pEditableDimensionX) result.m_dimensions.X() = m_pEditableDimensionX->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableDimensionY) result.m_dimensions.Y() = m_pEditableDimensionY->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableDimensionZ) result.m_dimensions.Z() = m_pEditableDimensionZ->EvaluateScaledAndOffset( fTimeN );

	if (m_pEditableVelScalar) result.m_fInheritVelScalar = m_pEditableVelScalar->EvaluateScaledAndOffset( fTimeN );	
	result.m_bInheritTransVel = (fabsf(result.m_fInheritVelScalar) > 0.001f) ? true : false;

	if (m_pEditableAngleMin) result.m_fAngleMin = m_pEditableAngleMin->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableAngleMax) result.m_fAngleMax = m_pEditableAngleMax->EvaluateScaledAndOffset( fTimeN );

	if (m_pEditableVelMin) result.m_fVelMin = m_pEditableVelMin->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableVelMax) result.m_fVelMax = m_pEditableVelMax->EvaluateScaledAndOffset( fTimeN );

	CDirection spawnVolYPR(m_spawnVolYPR);
	CDirection spawnDirYPR(m_spawnDirYPR);

	if (m_pEditableVolY) spawnVolYPR.X() = m_pEditableVolY->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableVolP) spawnVolYPR.Y() = m_pEditableVolP->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableVolR) spawnVolYPR.Z() = m_pEditableVolR->EvaluateScaledAndOffset( fTimeN );

	if (m_pEditableDirY) spawnDirYPR.X() = m_pEditableDirY->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableDirP) spawnDirYPR.Y() = m_pEditableDirP->EvaluateScaledAndOffset( fTimeN );
	if (m_pEditableDirR) spawnDirYPR.Z() = m_pEditableDirR->EvaluateScaledAndOffset( fTimeN );

	result.SetSpawnVolMatrix( spawnVolYPR );
	result.SetSpawnDirMatrix( spawnDirYPR );
};








//--------------------------------------------------
//!
//!	DebugRender
//!
//--------------------------------------------------
void Emitter::DebugRender()
{
#ifndef _GOLD_MASTER
	FunctionGraph graph( 0.11f, 0.49f, 0.11f, 0.49f );
	graph.DrawBounds(0x80ffffff);

	float fCurr = (m_fAge / m_pDef->m_fLoopDuration);
	fCurr -= floorf( fCurr );
	float fCurrentTop = 1.0f;

	if (m_pDef->GetEmissionFunction())
	{
		// find bounds of graph for debug drawing
		float fMin = 0.0f;
		float fMax = ntstd::Max( (m_pDef->GetEmissionFunction()->GetFunctionMax()), 1.0f );
		float fRange = fMax - fMin;

		static int iNumSamples = 100;
		for (int i = 0; i < iNumSamples; i++)
		{
			float fU = _R(i)	/ (iNumSamples+1);
			float fV = _R(i+1)	/ (iNumSamples+1);
			
			CPoint curr( fU, ntstd::Max( (m_pDef->GetEmissionFunction()->Evaluate(fU) - fMin) / fRange, fMin ), 0.0f );
			CPoint next( fV, ntstd::Max( (m_pDef->GetEmissionFunction()->Evaluate(fV) - fMin) / fRange, fMin ), 0.0f );

			graph.DrawLine( curr, next, 0xffffffff );
		}

		fCurrentTop = ntstd::Max( (m_pDef->GetEmissionFunction()->Evaluate(fCurr) - fMin) / fRange, fMin );
	}

	CPoint next( fCurr, fCurrentTop, 0.0f );
	graph.DrawLine( CPoint( fCurr, 0.0f, 0.0f ), next, 0x80ff0000 );

	char aLine[128];
	sprintf( aLine, "%d", m_iNumLoops );
	graph.DrawText( aLine, CPoint( 0.0f, 0.0f, 0.0f ), 0x80ffffff, FunctionGraph::LEFT );

	sprintf( aLine, "%d", m_iEmittedThisLoop );
	graph.DrawText( aLine, CPoint( 1.0f, 0.0f, 0.0f ), 0x80ffffff, FunctionGraph::RIGHT );
#endif
}
