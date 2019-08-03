//--------------------------------------------------
//!
//!	\file psystem_simple.cpp
//!	implementation of simple particle system and its
//! definition object
//!
//--------------------------------------------------

#include "psystem_simple.h"
#include "effect_util.h"
#include "objectdatabase/dataobject.h"
#include "objectdatabase/neteditinterface.h"
#include "psystem_debug.h"
#include "effect/psystem_utils.h"

START_STD_INTERFACE( PSystemSimpleDef )

	// standard params
	I2STRING	( m_texName,				TextureName )
	I2INT		( m_iRandomSeed,			RandomSeed )

	I2FLOAT		( m_fParticleLifetime,		ParticleLifetime )
	I2REFERENCE	( m_pParticleDef,			ParticleDef )

	I2VECTOR	( m_acceleration,			Acceleration )		// ParticleForce
	I2REFERENCE	( m_pBouncePlaneDef,		BouncePlane )
	I2FLOAT		( m_fRestitution,			BounceResititution )
	I2BOOL		( m_bUseRayCast,			UseHavok )
	I2BOOL		( m_bSortedParticles,		SortedParticles )
	I2BOOL		( m_bUseRandAtlasTex,		UseRandAtlasTex)
	I2FLOAT		( m_fCullRadius,			CullingRadius(m) )	// ED.CullingRadius(m)
	I2FLOAT		( m_fSortingPush,			SortingPush(m) )	// ED.SortingPush(m)
	
	I2REFERENCE_WITH_DEFAULT	( m_pDefaultEmitterDef, 	EmitterDef,		EmitterSimpleDef )
	I2REFERENCE					( m_pSpawnDef,				SpawnDef )

	// simple specific
	I2FLOAT		( m_fSizeStartMin,			ParticleSizeStartMin )
	I2FLOAT		( m_fSizeStartMax,			ParticleSizeStartMax )
	I2FLOAT		( m_fSizeEndMin,			ParticleSizeEndMin )
	I2FLOAT		( m_fSizeEndMax,			ParticleSizeEndMax )

	I2LIGHT		( m_colStart,				ColourStart )
	I2LIGHT		( m_colEnd,					ColourEnd )
	I2FLOAT		( m_fAlphaStart,			AlphaStart )
	I2FLOAT		( m_fAlphaEnd,				AlphaEnd )

	// renderstate parameters
	I2ENUM		( m_renderStateDef.m_renderType,		RS.RenderMode, EFFECT_RENDER_TYPE )
	I2ENUM		( m_renderStateDef.m_blendMode,			RS.AlphaBlendMode, EFFECT_BLENDMODE )
	I2BOOL		( m_renderStateDef.m_bZWriteEnable,		RS.DepthWriteEnabled )
	I2BOOL		( m_renderStateDef.m_bAlphaTestEnable,	RS.AlphaTestEnabled )
	I2FLOAT		( m_renderStateDef.m_fAlphaTestRef,		RS.AlphaTestValue )
	I2ENUM		( m_renderStateDef.m_alphaTestFunc,		RS.AlphaTestFunction, EFFECT_CMPFUNC )
	I2REFERENCE	( m_renderStateDef.m_pTimeOfDayMod,		RS.TimeOfDayPalette )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

//--------------------------------------------------
//!
//!	PSystemSimpleDef ctor
//!
//--------------------------------------------------
PSystemSimpleDef::PSystemSimpleDef() :
	m_fSizeStartMin( 1.0f ),
	m_fSizeStartMax( 1.0f ),
	m_fSizeEndMin( 1.0f ),
	m_fSizeEndMax( 1.0f ),
	m_colStart( 1.0f, 1.0f, 1.0f, 1.0f ),
	m_colEnd( 1.0f, 1.0f, 1.0f, 1.0f ),
	m_fAlphaStart( 1.0f ),
	m_fAlphaEnd( 1.0f )
{}

//--------------------------------------------------
//!
//!	fill in any missing fields we have...
//!
//--------------------------------------------------
void PSystemSimpleDef::PostConstruct()
{
	LOAD_PROFILE( PSystemSimpleDef_PostConstruct )
	EstablishParticleType();
	ResolveTextureMode();
	LoadTexture();
}

//--------------------------------------------------
//!
//!	refresh our internal state after editing
//!
//--------------------------------------------------
bool PSystemSimpleDef::EditorChangeValue( CallBackParameter param, CallBackParameter )
{
	CHashedString pName(param);
	const char* pStrName = ntStr::GetString( pName );

	PostConstruct();

	if (HASH_STRING_TEXTURENAME == pName || HASH_STRING_RANDOMSEED == pName || HASH_STRING_BOUNCEPLANE == pName
		|| HASH_STRING_USEHAVOK == pName || HASH_STRING_PARTICLELIFETIME == pName || HASH_STRING_SORTEDPARTICLES == pName
		|| HASH_STRING_USERANDATLASTEX == pName || HASH_STRING_SPAWNDEF == pName || HASH_STRING_PARTICLEDEF == pName
		|| HASH_STRING_EMITTERDEF == pName || (strstr(pStrName,"RS.")) )
	{
		m_resetSet.ResetThings();
	}

	return true;
}

//--------------------------------------------------
//!
//!	retrive our XML name
//!
//--------------------------------------------------
const char* PSystemSimpleDef::GetDebugName() const
{
	return EffectUtils::GetInterfaceName( (void*)this );
}







//--------------------------------------------------
//!
//!	PSystemSimple::PSystemSimple
//! ctors for simple particle systems
//!
//--------------------------------------------------
PSystemSimple::PSystemSimple( const PSystemSimpleDef& def, const CMatrix& frame, const EmitterDef* pOverideEmit ) :
	ParticleSystem( def, frame, pOverideEmit ),
	m_pDefinition( &def ),
	m_pParticles( 0 )
{
	Reset(false);
	PSystemDebugMan::Register(this);
}

PSystemSimple::PSystemSimple( const PSystemSimpleDef& def, const Transform& transform, const EmitterDef* pOverideEmit ) :
	ParticleSystem( def, transform, pOverideEmit ),
	m_pDefinition( &def ),
	m_pParticles( 0 )
{
	Reset(false);
	PSystemDebugMan::Register(this);
}

//--------------------------------------------------
//!
//!	PSystemSimple dtor
//!
//--------------------------------------------------
PSystemSimple::~PSystemSimple()
{
	Reset(true);
	PSystemDebugMan::Release(this);
}

//--------------------------------------------------
//!
//!	PSystemSimple::Reset
//!
//--------------------------------------------------
void PSystemSimple::Reset( bool bInDestructor )
{
	if (m_pParticles)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pParticles );
	}

	ParticleSystem::ResetBase(bInDestructor);

	if (!bInDestructor)
	{
		m_pParticles = ParticleHandlerSimple::Instantiate( this );
		m_pParticles->SetSpawner( m_pSpawner );
		m_renderstates.m_bPointSprite = m_pParticles->UsingPointSprites();
	}
}

//--------------------------------------------------
//!
//!	PSystemSimple::Update
//!
//--------------------------------------------------
bool PSystemSimple::UpdateEffect()
{
	float fTimeDelta = GetNextTimeDelta();

	// this is okay here as we're only using debug prims, which are delayed render...
	// it's here as render can be disabled completley if the right blend modes are set.
	if	(
		(EmitterEdited())||
		(CNetEditInterface::Get().GetSelected() == ObjectDatabase::Get().GetDataObjectFromPointer(m_pDefinition) )
		)
		DebugRender( true, true );

	// never update with a backwards delta
	if (fTimeDelta <= 0.0f)
		return m_bKillMeNow;

	PSystemDebugMan::StartSimpleUpdateTimer();

	m_bKillMeNow = UpdateParticleSystem( fTimeDelta );

	PSystemDebugMan::StopSimpleUpdateTimer();

	return m_bKillMeNow;
}

//--------------------------------------------------
//!
//!	Debug render everything
//!
//--------------------------------------------------
void PSystemSimple::DebugRender( bool bEditing, bool bText )
{
	ParticleSystem::DebugRenderEmitter(bEditing);

	if (bText)
	{
		const char* pCPUCost,* pVSCost,* pPSCost;
		PSystemUtils::GetApproxCost( *this, &pCPUCost, &pVSCost, &pPSCost );

		const char* pType = m_pParticles->UsingCPUParticles() ? "SimpleCPU" : "SimpleGPU";
		ParticleSystem::DebugRenderInfo(pCPUCost,pVSCost,pPSCost,PSystemUtils::GetTechniqueName(*this), pType);
	}
}
