//--------------------------------------------------
//!
//!	\file psystem_complex.cpp
//!	implementation of complex particle system and its
//! definition object
///!
//--------------------------------------------------

#include "psystem_complex.h"
#include "effect_util.h"
#include "objectdatabase/dataobject.h"
#include "colour_function.h"
#include "functioncurve.h"
#include "psystem_debug.h"
#include "objectdatabase/neteditinterface.h"
#include "effect/psystem_utils.h"
#include "effect/particle_movement.h"

START_STD_INTERFACE( PSystemComplexDef )

	// standard params
	I2STRING	( m_texName,				TextureName )
	I2INT		( m_iRandomSeed,			RandomSeed )

	I2FLOAT		( m_fParticleLifetime,		ParticleLifetime )
	I2REFERENCE	( m_pParticleDef,			ParticleDef )

	I2VECTOR	( m_acceleration,			Acceleration )
	I2REFERENCE	( m_pBouncePlaneDef,		BouncePlane )
	I2FLOAT		( m_fRestitution,			BounceResititution )
	I2BOOL		( m_bUseRayCast,			UseHavok )
	I2BOOL		( m_bSortedParticles,		SortedParticles )
	I2BOOL		( m_bUseRandAtlasTex,		UseRandAtlasTex )
	I2FLOAT		( m_fCullRadius,			CullingRadius(m) )	// ED.CullingRadius(m)
	I2FLOAT		( m_fSortingPush,			SortingPush(m) )	// ED.SortingPush(m)

	I2REFERENCE_WITH_DEFAULT	( m_pDefaultEmitterDef, 	EmitterDef,			EmitterSimpleDef )
	I2REFERENCE					( m_pSpawnDef,				SpawnDef )

	// complex specific
	I2REFERENCE	( m_pAdvMovement, AdvancedMovement )
	I2REFERENCE_WITH_DEFAULT	( m_resources.m_pSizeMin, 	SizeFunctionMin,	FunctionCurve_User )
	I2REFERENCE_WITH_DEFAULT	( m_resources.m_pSizeMax, 	SizeFunctionMax,	FunctionCurve_User )
	I2REFERENCE_WITH_DEFAULT	( m_resources.m_pPalette,	ColourFunction,		ColourFunction_Lerp )
	
	// renderstate parameters
	I2ENUM		( m_renderStateDef.m_renderType,		RS.RenderMode, EFFECT_RENDER_TYPE )
	I2ENUM		( m_renderStateDef.m_blendMode,			RS.AlphaBlendMode, EFFECT_BLENDMODE )
	I2BOOL		( m_renderStateDef.m_bZWriteEnable,		RS.DepthWriteEnabled )
	I2BOOL		( m_renderStateDef.m_bAlphaTestEnable,	RS.AlphaTestEnabled )
	I2FLOAT		( m_renderStateDef.m_fAlphaTestRef,		RS.AlphaTestValue )
	I2ENUM		( m_renderStateDef.m_alphaTestFunc,		RS.AlphaTestFunction, EFFECT_CMPFUNC )
	I2REFERENCE	( m_renderStateDef.m_pTimeOfDayMod,		RS.TimeOfDayPalette )

	PUBLISH_PTR_CONTAINER_AS( m_obObjects, Objects )

	DECLARE_AUTOCONSTRUCT_CALLBACK( AutoConstruct );

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

//--------------------------------------------------
//!
//!	PSystemComplexResources::PSystemComplexResources
//!
//--------------------------------------------------
PSystemComplexResources::PSystemComplexResources() :
	m_pSizeMin(0),
	m_pSizeMax(0),
	m_pPalette(0)
{
	m_pParent = 0;
	EffectResourceMan::Get().RegisterResource( *this );
}

PSystemComplexResources::~PSystemComplexResources()
{
	EffectResourceMan::Get().ReleaseResource( *this );
}

//--------------------------------------------------
//!
//!	PSystemComplexResources::GenerateResources
//!
//--------------------------------------------------
void PSystemComplexResources::GenerateResources()
{
	ntError(m_pParent);
	ntError(m_pSizeMin);
	ntError(m_pSizeMax);
	ntError(m_pPalette);

	m_texPalette.Reset();

	bool bHDRPalette = (m_pParent->m_renderStateDef.m_renderType == ERT_HIGH_DYNAMIC_RANGE) ? true : false;
	m_texPalette = m_pPalette->GenerateTexture( ColourFunction::TEXGEN_NORMAL, 256, bHDRPalette, false );
	
	m_bCPUResources = m_pParent->RequiresCPUParticle();

	if (m_bCPUResources)
		m_sizeFunctions.Reset( FunctionObject::FTT_FLOAT_SIMPLE );
	else
		m_sizeFunctions.Reset( FunctionObject::FTT_TEXTURE );

	m_sizeFunctions.AddFunction( m_pSizeMin );
	m_sizeFunctions.AddFunction( m_pSizeMax );
	m_sizeFunctions.FlushCreation();

	ResourcesOutOfDate(); // this flushes any erronious refresh detects
	m_bRequireRefresh = false;
}

//--------------------------------------------------
//!
//!	PSystemComplexResources::ResourcesOutOfDate
//! per frame callback to see if we need regenerating
//!
//--------------------------------------------------
bool PSystemComplexResources::ResourcesOutOfDate() const
{
	if (m_pSizeMin->DetectCurveChanged())
		m_bRequireRefresh = true;

	if (m_pSizeMax->DetectCurveChanged())
		m_bRequireRefresh = true;

	if (m_pPalette->HasChanged())
		m_bRequireRefresh = true;

	return m_bRequireRefresh;
}




//--------------------------------------------------
//!
//!	PSystemComplexDef ctor
//!
//--------------------------------------------------
PSystemComplexDef::PSystemComplexDef() :
	m_pAdvMovement(0)
{
	m_resources.SetParent( this );
}

//--------------------------------------------------
//!
//! PSystemComplexDef::PostConstruct
//!	fill in any missing fields we have...
//!
//--------------------------------------------------
void PSystemComplexDef::PostConstruct()
{
	LOAD_PROFILE( PSystemComplexDef_PostConstruct )
	EstablishParticleType();
	ResolveTextureMode();
	LoadTexture();
}

//--------------------------------------------------
//!
//! PSystemComplexDef::AutoConstruct
//!	If editor is creating, set some good defaults
//!
//--------------------------------------------------
void PSystemComplexDef::AutoConstruct( const DataInterfaceField* pField )
{
	static const float aValues[] = { 1.0f, 1.0f };
	static const float aTimes[] = { 0.0f, 1.0f };

	if( pField->GetName() == "SizeFunctionMin" )
	{
		m_resources.m_pSizeMin->InitialiseToArrayValues( 2, aValues, aTimes );
	}
	else if( pField->GetName() == "SizeFunctionMax" )
	{
		m_resources.m_pSizeMax->InitialiseToArrayValues( 2, aValues, aTimes );
	}
}

//--------------------------------------------------
//!
//!	refresh our internal state after editing
//!
//--------------------------------------------------
bool PSystemComplexDef::EditorChangeValue( CallBackParameter param, CallBackParameter )
{
	CHashedString pName(param);
	const char* pStrName = ntStr::GetString( pName );

	if( HASH_STRING_SIZEFUNCTIONMIN == pName || HASH_STRING_SIZEFUNCTIONMAX == pName || HASH_STRING_COLOURFUNCTION == pName
		|| m_resources.HasCPUParticleResources() != RequiresCPUParticle())
	{
		m_resources.GenerateResources();
	}

	PostConstruct();

	if ( HASH_STRING_TEXTURENAME == pName || HASH_STRING_RANDOMSEED == pName || HASH_STRING_BOUNCEPLANE == pName 
		|| HASH_STRING_USEHAVOK == pName || HASH_STRING_PARTICLELIFETIME == pName || HASH_STRING_SORTEDPARTICLES == pName
		|| HASH_STRING_USERANDATLASTEX == pName || HASH_STRING_SPAWNDEF == pName || HASH_STRING_PARTICLEDEF == pName
		|| HASH_STRING_EMITTERDEF == pName || HASH_STRING_ADVANCEDMOVEMENT == pName || (strstr(pStrName,"RS.")) )
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
const char* PSystemComplexDef::GetDebugName() const
{
	return EffectUtils::GetInterfaceName( (void*)this );
}




//--------------------------------------------------
//!
//!	PSystemComplex ctor
//!
//--------------------------------------------------
PSystemComplex::PSystemComplex( const PSystemComplexDef& def, const CMatrix& frame, const EmitterDef* pOverideEmit ) :
	ParticleSystem( def, frame, pOverideEmit ),
	m_pDefinition( &def ),
	m_pParticles( 0 )
{
	Reset(false);
	PSystemDebugMan::Get().Register(this);
}

//--------------------------------------------------
//!
//!	PSystemComplex ctor
//!
//--------------------------------------------------
PSystemComplex::PSystemComplex( const PSystemComplexDef& def, const Transform& transform, const EmitterDef* pOverideEmit ) :
	ParticleSystem( def, transform, pOverideEmit ),
	m_pDefinition( &def ),
	m_pParticles( 0 )
{
	Reset(false);
	PSystemDebugMan::Get().Register(this);
}

//--------------------------------------------------
//!
//!	PSystemComplex dtor
//!
//--------------------------------------------------
PSystemComplex::~PSystemComplex()
{
	Reset(true);
	PSystemDebugMan::Get().Release(this);
}

//--------------------------------------------------
//!
//!	reset
//!
//--------------------------------------------------
void PSystemComplex::Reset( bool bInDestructor )
{
	if (m_pParticles)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pParticles );
	}

	ParticleSystem::ResetBase(bInDestructor);

	if (!bInDestructor)
	{
		m_pParticles = ParticleHandlerComplex::Instantiate( this ); 
		m_pParticles->SetSpawner( m_pSpawner );
		m_renderstates.m_bPointSprite = m_pParticles->UsingPointSprites();
	}
}

//--------------------------------------------------
//!
//!	PSystemComplex::WaitingForResources
//!
//--------------------------------------------------
bool PSystemComplex::WaitingForResources() const
{
	if (m_pDefinition->m_resources.ResourcesOutOfDate())
		return true;

	if	(
		(m_pDefinition->m_pAdvMovement) &&
		(m_pDefinition->m_pAdvMovement->m_resources.ResourcesOutOfDate())
		)
		return true;

	return false;
}

//--------------------------------------------------
//!
//!	Update
//!
//--------------------------------------------------
bool PSystemComplex::UpdateEffect()
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

	PSystemDebugMan::StartFunctionalUpdateTimer();

	m_bKillMeNow = UpdateParticleSystem( fTimeDelta );

	PSystemDebugMan::StopFunctionalUpdateTimer();

	return m_bKillMeNow;
}

//--------------------------------------------------
//!
//!	Debug render everything
//!
//--------------------------------------------------
void PSystemComplex::DebugRender( bool bEditing, bool bText )
{
	ParticleSystem::DebugRenderEmitter(bEditing);

	if (bText)
	{
		const char* pCPUCost,* pVSCost,* pPSCost;
		PSystemUtils::GetApproxCost( *this, &pCPUCost, &pVSCost, &pPSCost );

		const char* pType = m_pParticles->UsingCPUParticles() ? "ComplexCPU" : "ComplexGPU";
		ParticleSystem::DebugRenderInfo(pCPUCost,pVSCost,pPSCost,PSystemUtils::GetTechniqueName(*this), pType);
	}
}


