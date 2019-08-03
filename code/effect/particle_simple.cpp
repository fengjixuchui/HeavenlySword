//--------------------------------------------------
//!
//!	\file particle_simple.cpp
//!	Implementation of the various types of particle
//! handlers used by the simple particle system
//!
//--------------------------------------------------

#include "particle_simple.h"
#include "particle_types.h"
#include "particle_structs.h"
#include "particle_movement.h"
#include "psystem_simple.h"

//--------------------------------------------------
//!
//!	ParticleHandlerSimple::InstantiateFromDef
//! Static factory method for manufacturing particle handlers
//!
//--------------------------------------------------
ParticleHandlerSimple* ParticleHandlerSimple::Instantiate( const PSystemSimple* pOwner )
{
	ntAssert(pOwner);

	switch( pOwner->GetDefinition().m_particleType )
	{
	case PT_SIMPLE_SPRITE:			return NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PHS_SimpleSprite(pOwner);
	case PT_ROTATING_SPRITE:		return NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PHS_RotatingSprite(pOwner);
	case PT_WORLD_ALIGNED_QUAD:		return NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PHS_OrientedQuad(pOwner);
	case PT_AXIS_ALIGNED_RAY:		return NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PHS_AxisAlignedRay(pOwner);
	case PT_VELOCITY_ALIGNED_RAY:	return NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PHS_VelScaledRay(pOwner);
	default:
		ntError_p(0,("Unsupported particle type requested"));
		return NULL;
	}
}

//--------------------------------------------------
//!
//!	ParticleHandlerSimple::ParticleHandlerSimple
//!
//--------------------------------------------------
ParticleHandlerSimple::ParticleHandlerSimple( const PSystemSimple* pOwner ) :
	m_pOwner(pOwner),
	m_pPSystemDef(&pOwner->GetDefinition()),
	m_pSpawner(0),
	m_iCurrParticle(0),
	m_bUsingCPUParticle(false),
	m_bUsingPointSprites(false)
{
	ntAssert( m_pOwner );
	ntAssert( m_pPSystemDef );

	if (m_pPSystemDef->RequiresSorting())
		m_pQuads = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) QuadListSorted;
	else
		m_pQuads = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) QuadList;
}

//--------------------------------------------------
//!
//!	ParticleHandlerSimple::dtor
//!
//--------------------------------------------------
ParticleHandlerSimple::~ParticleHandlerSimple()
{
	NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pQuads );
}





//--------------------------------------------------
//!
//!	PHS_SimpleSprite::ctor
//!
//--------------------------------------------------
void PHS_SimpleSprite::ConstructBody()
{
	m_bUsingCPUParticle = m_pPSystemDef->RequiresCPUParticle();
	m_bUsingPointSprites = m_pPSystemDef->CanUsePointSprite();

	if (m_bUsingCPUParticle)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME,	"input.ageN" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,	"input.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,	"input.size" );			
		m_pQuads->GetCPUData().PushVertexElement( sizeof(CPUParticle_PS), 0 );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_simple_cpu" );
#else
		if (m_bUsingPointSprites)
			m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_cpu_point_vp.sho" );
		else
			m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_cpu_quad_vp.sho" );
#endif
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME, "input.birthTime" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,	"input.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_VELOCITY,	"input.vel" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,	"input.size" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZEVEL,	"input.sizeVel" );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_simple_gpu" );
#else
		if (m_bUsingPointSprites)
			m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_gpu_point_vp.sho" );
		else
			m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_gpu_quad_vp.sho" );
#endif
	}

#ifdef PLATFORM_PS3
	SetPixelShader();
	ntAssert_p( m_pVertexShader, ("Failed to find vertex shader") );
	ntAssert_p( m_pPixelShader, ("Failed to find pixel shader") );
#endif

	static CPUParticle_PS templateParticle( 1.0f );

	if (m_bUsingPointSprites)
	{
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), true );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME );
	}
	else if (m_pPSystemDef->m_eTexMode == PTM_RAND_TEXTURED)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_TEXTURE0, "input.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0, true );
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2, PE_TEXTURE0, "input.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0 );
	}

	if (m_bUsingCPUParticle)
		m_pParticlesCPU = (CPUParticle_PS*)m_pQuads->GetCPUData().GetVertex(0);
}

//--------------------------------------------------
//!
//!	PHS_SimpleSprite::EmitParticle
//!	pos and vel may legally be adjusted here
//! returns the particle if we're CPU based
//!
//--------------------------------------------------
pos_vel_PD* PHS_SimpleSprite::EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel )
{
	pos_vel_PD* pResult = 0;

	if (m_iCurrParticle >= m_pQuads->GetNumQuads())
		m_iCurrParticle = 0;

	float fSizeStart = erandf( m_pPSystemDef->m_fSizeStartMax - m_pPSystemDef->m_fSizeStartMin ) + m_pPSystemDef->m_fSizeStartMin;
	float fSizeEnd = erandf( m_pPSystemDef->m_fSizeEndMax - m_pPSystemDef->m_fSizeEndMin ) + m_pPSystemDef->m_fSizeEndMin;
	float fSizeVel = fSizeEnd - fSizeStart;

	if (m_bUsingCPUParticle)
	{
		float fOffset = fCurrTime - fBirthTime;
		m_pParticlesCPU[ m_iCurrParticle ].Init( pos, vel, fSizeStart, fSizeVel );
		m_pParticlesCPU[ m_iCurrParticle ].OffsetInitialConditions( fOffset, fOffset / m_pPSystemDef->m_fParticleLifetime, m_pPSystemDef->m_acceleration );
		pResult = &m_pParticlesCPU[ m_iCurrParticle ];
	}
	else
	{
		GPUParticle vertData;
		vertData.Set( fBirthTime, pos, vel * m_pPSystemDef->m_fParticleLifetime, fSizeStart, fSizeVel );
		m_pQuads->SetGPUQuadInfo( m_iCurrParticle, &vertData, sizeof( GPUParticle ) );
	}

	m_iCurrParticle++;
	return pResult;
}

//--------------------------------------------------
//!
//!	Update
//!
//--------------------------------------------------
void PHS_SimpleSprite::Update( float fTimeDelta, float fTimeDeltaN )
{
	if (m_bUsingCPUParticle)
	{
		Iterative_ParticleMover mover( m_pPSystemDef->m_bUseRayCast,
										m_pPSystemDef->m_pBouncePlaneDef,
										m_pPSystemDef->m_fRestitution,
										m_pSpawner );

		mover.m_currAcc = m_pPSystemDef->m_acceleration;

		for (u_int i = 0; i < m_pQuads->GetNumQuads(); ++i )
		{
			m_pParticlesCPU[i].ageN += fTimeDeltaN;
			if ( m_pParticlesCPU[i].ageN < 1.0f )
			{
				mover.Update( fTimeDelta, m_pParticlesCPU[i] );
				PODParticle vertData( m_pParticlesCPU[i].ageN, m_pParticlesCPU[i].pos, m_pParticlesCPU[i].GetSize() );
				m_pQuads->SetGPUQuadInfo( i, &vertData, sizeof( PODParticle ) );
			}
			else
			{
				static PODParticle vertData( 1.0f, CPoint(0.0f, 0.0f, 0.0f), 0.0f );
				m_pQuads->SetGPUQuadInfo( i, &vertData, sizeof( PODParticle ) );
			}
		}
	}
}





//--------------------------------------------------
//!
//!	PHS_RotatingSprite::ctor
//!
//--------------------------------------------------
PHS_RotatingSprite::PHS_RotatingSprite( const PSystemSimple* pOwner ) :
	ParticleHandlerSimple(pOwner)
{
	m_pParticleDef = (ParticleDef_Rotating*)m_pPSystemDef->m_pParticleDef;
	
	m_bUsingCPUParticle = m_pPSystemDef->RequiresCPUParticle();
	m_bUsingPointSprites = false;

	if (m_bUsingCPUParticle)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME,	"particle.ageN" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,	"particle.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,	"particle.size" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_ROTSTART,	"particle.rot" );
		m_pQuads->GetCPUData().PushVertexElement( sizeof(CPUParticle_PS_Rot), 0 );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_simple_rot_cpu" );
#else
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_rot_cpu_quad_vp.sho" );
#endif
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME, "particle.birthTime" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,	"particle.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_VELOCITY,	"particle.vel" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,	"particle.size" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZEVEL,	"particle.sizeVel" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_ROTSTART,	"particle.rot" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_ROTVEL,		"particle.rotVel" );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_simple_rot_gpu" );
#else
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_rot_gpu_quad_vp.sho" );
#endif
	}

#ifdef PLATFORM_PS3
	SetPixelShader();
	ntAssert_p( m_pVertexShader, ("Failed to find vertex shader") );
	ntAssert_p( m_pPixelShader, ("Failed to find pixel shader") );
#endif

	static CPUParticle_PS_Rot templateParticle( 1.0f );

	if (m_bUsingPointSprites)
	{
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), true );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME );
	}
	else if (m_pPSystemDef->m_eTexMode == PTM_RAND_TEXTURED)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_TEXTURE0, "particle.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0, true );
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2, PE_TEXTURE0, "particle.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0 );
	}

	if (m_bUsingCPUParticle)
		m_pParticlesCPU = (CPUParticle_PS_Rot*)m_pQuads->GetCPUData().GetVertex(0);
}

//--------------------------------------------------
//!
//!	PHS_RotatingSprite::EmitParticle
//!	pos and vel may legally be adjusted here 
//! returns the particle if we're CPU based
//!
//--------------------------------------------------
pos_vel_PD* PHS_RotatingSprite::EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel )
{
	pos_vel_PD* pResult = 0;

	if (m_iCurrParticle >= m_pQuads->GetNumQuads())
		m_iCurrParticle = 0;

	float fSizeStart = erandf( m_pPSystemDef->m_fSizeStartMax - m_pPSystemDef->m_fSizeStartMin ) + m_pPSystemDef->m_fSizeStartMin;
	float fSizeEnd = erandf( m_pPSystemDef->m_fSizeEndMax - m_pPSystemDef->m_fSizeEndMin ) + m_pPSystemDef->m_fSizeEndMin;
	float fSizeVel = fSizeEnd - fSizeStart;

	float fRotStart = m_pParticleDef->GetNewRotation();	// radians
	float fRotVel = m_pParticleDef->GetNewRotationVel();	// radians / sec

	if (m_bUsingCPUParticle)
	{
		float fOffset = fCurrTime - fBirthTime;
		m_pParticlesCPU[ m_iCurrParticle ].Init( pos, vel, fSizeStart, fSizeVel, fRotStart, fRotVel );
		m_pParticlesCPU[ m_iCurrParticle ].OffsetInitialConditions( fOffset, fOffset / m_pPSystemDef->m_fParticleLifetime, m_pPSystemDef->m_acceleration );
		pResult = &m_pParticlesCPU[ m_iCurrParticle ];
	}
	else
	{
		GPUParticle vertData;
		float fLifetime = m_pPSystemDef->m_fParticleLifetime;
		vertData.Set( fBirthTime, pos, vel * fLifetime, fSizeStart, fSizeVel, fRotStart, fRotVel * fLifetime );
		m_pQuads->SetGPUQuadInfo( m_iCurrParticle, &vertData, sizeof( GPUParticle ) );
	}

	m_iCurrParticle++;
	return pResult;
}

//--------------------------------------------------
//!
//!	Update
//!
//--------------------------------------------------
void PHS_RotatingSprite::Update( float fTimeDelta, float fTimeDeltaN )
{
	if (m_bUsingCPUParticle)
	{
		Iterative_ParticleMover mover( m_pPSystemDef->m_bUseRayCast,
										m_pPSystemDef->m_pBouncePlaneDef,
										m_pPSystemDef->m_fRestitution,
										m_pSpawner );

		mover.m_currAcc = m_pPSystemDef->m_acceleration;

		float fRotAcc = m_pParticleDef->GetRotationAcc();

		for (u_int i = 0; i < m_pQuads->GetNumQuads(); ++i )
		{
			m_pParticlesCPU[i].ageN += fTimeDeltaN;
			if ( m_pParticlesCPU[i].ageN < 1.0f )
			{
				mover.Update( fTimeDelta, m_pParticlesCPU[i] );

				m_pParticlesCPU[i].rot += m_pParticlesCPU[i].rotVel * fTimeDelta;
				m_pParticlesCPU[i].rotVel += fRotAcc * fTimeDelta;

				PODParticle vertData( m_pParticlesCPU[i].ageN, m_pParticlesCPU[i].pos, m_pParticlesCPU[i].GetSize(), m_pParticlesCPU[i].rot );
				m_pQuads->SetGPUQuadInfo( i, &vertData, sizeof( PODParticle ) );
			}
			else
			{
				static PODParticle vertData( 1.0f, CPoint(0.0f, 0.0f, 0.0f), 0.0f, 0.0f );
				m_pQuads->SetGPUQuadInfo( i, &vertData, sizeof( PODParticle ) );
			}
		}
	}
}




//--------------------------------------------------
//!
//!	PHS_OrientedQuad::ctor
//!
//--------------------------------------------------
PHS_OrientedQuad::PHS_OrientedQuad( const PSystemSimple* pOwner ) :
	PHS_SimpleSprite(pOwner,true)
{
	m_pParticleDef = (ParticleDef_WorldQuad*)m_pPSystemDef->m_pParticleDef;
	m_bUsingCPUParticle = m_pPSystemDef->RequiresCPUParticle();

	if (m_bUsingCPUParticle)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME, "particle.ageN" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,	"particle.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,	"particle.size" );			
		m_pQuads->GetCPUData().PushVertexElement( sizeof(CPUParticle_PS), 0 );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_simple_orientquad_cpu" );
#else
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_orient_cpu_quad_vp.sho" );
#endif
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME, "particle.birthTime" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,	"particle.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_VELOCITY,	"particle.vel" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,	"particle.size" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZEVEL,	"particle.sizeVel" );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_simple_orientquad_gpu" );
#else
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_orient_gpu_quad_vp.sho" );
#endif
	}

#ifdef PLATFORM_PS3
	SetPixelShader();
	ntAssert_p( m_pVertexShader, ("Failed to find vertex shader") );
	ntAssert_p( m_pPixelShader, ("Failed to find pixel shader") );
#endif

	static CPUParticle_PS templateParticle( 1.0f );

	if (m_pPSystemDef->m_eTexMode == PTM_RAND_TEXTURED)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_TEXTURE0, "particle.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0, true );
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2, PE_TEXTURE0, "particle.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );		
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0 );
	}

	if (m_bUsingCPUParticle)
		m_pParticlesCPU = (CPUParticle_PS*)m_pQuads->GetCPUData().GetVertex(0);
}




//--------------------------------------------------
//!
//!	PHS_AxisAlignedRay::ctor
//!
//--------------------------------------------------
PHS_AxisAlignedRay::PHS_AxisAlignedRay( const PSystemSimple* pOwner ) :
	PHS_SimpleSprite(pOwner,true)
{
	m_pParticleDef = (ParticleDef_AxisQuad*)m_pPSystemDef->m_pParticleDef;
	m_bUsingCPUParticle = m_pPSystemDef->RequiresCPUParticle();

	if (m_bUsingCPUParticle)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME, "particle.ageN" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,	"particle.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,	"particle.size" );			
		m_pQuads->GetCPUData().PushVertexElement( sizeof(CPUParticle_PS), 0 );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_simple_axisray_cpu" );
#else
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_axisray_cpu_vp.sho" );
#endif
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME, "particle.birthTime" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,	"particle.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_VELOCITY,	"particle.vel" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,	"particle.size" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZEVEL,	"particle.sizeVel" );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_simple_axisray_gpu" );
#else
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_axisray_gpu_vp.sho" );
#endif
	}

#ifdef PLATFORM_PS3
	SetPixelShader();
	ntAssert_p( m_pVertexShader, ("Failed to find vertex shader") );
	ntAssert_p( m_pPixelShader, ("Failed to find pixel shader") );
#endif

	static CPUParticle_PS templateParticle( 1.0f );

	if (m_pPSystemDef->m_eTexMode == PTM_RAND_TEXTURED)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_TEXTURE0, "particle.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0, true );
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2, PE_TEXTURE0, "particle.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0 );
	}

	if (m_bUsingCPUParticle)
		m_pParticlesCPU = (CPUParticle_PS*)m_pQuads->GetCPUData().GetVertex(0);
}




//--------------------------------------------------
//!
//!	PHS_VelScaledRay::ctor
//!
//--------------------------------------------------
PHS_VelScaledRay::PHS_VelScaledRay( const PSystemSimple* pOwner ) :
	ParticleHandlerSimple(pOwner),
	m_fLastTimeInterval(0.0f)
{
	m_pParticleDef = (ParticleDef_VelScaledRay*)m_pPSystemDef->m_pParticleDef;
	m_bUsingCPUParticle = m_pPSystemDef->RequiresCPUParticle();

	if (m_bUsingCPUParticle)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME,		"particle.ageN" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,		"particle.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,		"particle.size" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_VELINTERVAL,	"particle.velInterval" );
		m_pQuads->GetCPUData().PushVertexElement( sizeof(CPUParticle_PS), 0 );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_velscaleray_cpu" );
#else
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_velscaleray_cpu_vp.sho" );
#endif
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_BIRTH_TIME, "particle.birthTime" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_POSITON,	"particle.pos" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_VELOCITY,	"particle.vel" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZESTART,	"particle.size" );
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT1, PE_SIZEVEL,	"particle.sizeVel" );

#ifdef PLATFORM_PC
		m_ppFX = EffectResourceMan::Get().RetrieveFXHandle( "psystem_velscaleray_gpu" );
#else
		m_pVertexShader = DebugShaderCache::Get().LoadShader( "psystem_simple_velscaleray_gpu_vp.sho" );
#endif
	}

#ifdef PLATFORM_PS3
	SetPixelShader();
	ntAssert_p( m_pVertexShader, ("Failed to find vertex shader") );
	ntAssert_p( m_pPixelShader, ("Failed to find pixel shader") );
#endif

	static CPUParticle_PS templateParticle( 1.0f );

	if (m_pPSystemDef->m_eTexMode == PTM_RAND_TEXTURED)
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT3, PE_TEXTURE0, "particle.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0, true );
	}
	else
	{
		m_pQuads->GetGPUData().PushVertexElement( VD_STREAM_TYPE_FLOAT2, PE_TEXTURE0, "particle.texcoord" );
		m_pQuads->Initialise( m_pOwner->GetMaxParticles(), false );
		PSystemUtils::InitialiseQuadList( *m_pQuads, &templateParticle, PE_BIRTH_TIME, PE_TEXTURE0 );
	}

	if (m_bUsingCPUParticle)
		m_pParticlesCPU = (CPUParticle_PS*)m_pQuads->GetCPUData().GetVertex(0);
}

//--------------------------------------------------
//!
//!	PHS_VelScaledRay::EmitParticle
//!	pos and vel may legally be adjusted here 
//! returns the particle if we're CPU based
//!
//--------------------------------------------------
pos_vel_PD* PHS_VelScaledRay::EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel )
{
	pos_vel_PD* pResult = 0;

	if (m_iCurrParticle >= m_pQuads->GetNumQuads())
		m_iCurrParticle = 0;

	float fSizeStart = erandf( m_pPSystemDef->m_fSizeStartMax - m_pPSystemDef->m_fSizeStartMin ) + m_pPSystemDef->m_fSizeStartMin;
	float fSizeEnd = erandf( m_pPSystemDef->m_fSizeEndMax - m_pPSystemDef->m_fSizeEndMin ) + m_pPSystemDef->m_fSizeEndMin;
	float fSizeVel = fSizeEnd - fSizeStart;

	if (m_bUsingCPUParticle)
	{
		float fOffset = fCurrTime - fBirthTime;
		m_pParticlesCPU[ m_iCurrParticle ].Init( pos, vel, fSizeStart, fSizeVel );
		m_pParticlesCPU[ m_iCurrParticle ].OffsetInitialConditions( fOffset, fOffset / m_pPSystemDef->m_fParticleLifetime, m_pPSystemDef->m_acceleration );
		pResult = &m_pParticlesCPU[ m_iCurrParticle ];
	}
	else
	{
		GPUParticle vertData;
		vertData.Set( fBirthTime, pos, vel * m_pPSystemDef->m_fParticleLifetime, fSizeStart, fSizeVel );
		m_pQuads->SetGPUQuadInfo( m_iCurrParticle, &vertData, sizeof( GPUParticle ) );
	}

	m_iCurrParticle++;
	return pResult;
}

//--------------------------------------------------
//!
//!	PHS_VelScaledRay::Update
//!
//--------------------------------------------------
void PHS_VelScaledRay::Update( float fTimeDelta, float fTimeDeltaN )
{
	m_fLastTimeInterval = fTimeDelta;

	if (m_bUsingCPUParticle)
	{
		Iterative_ParticleMover mover( m_pPSystemDef->m_bUseRayCast,
										m_pPSystemDef->m_pBouncePlaneDef,
										m_pPSystemDef->m_fRestitution,
										m_pSpawner );

		mover.m_currAcc = m_pPSystemDef->m_acceleration;

		float fTimeScale = m_pParticleDef->m_bFixedTime ? (1.0f / 30.0f) : fTimeDelta;

		for (u_int i = 0; i < m_pQuads->GetNumQuads(); ++i )
		{
			m_pParticlesCPU[i].ageN += fTimeDeltaN;
			if ( m_pParticlesCPU[i].ageN < 1.0f )
			{
				mover.Update( fTimeDelta, m_pParticlesCPU[i] );
				PODParticle vertData( m_pParticlesCPU[i].ageN, m_pParticlesCPU[i].pos, m_pParticlesCPU[i].GetSize(), m_pParticlesCPU[i].vel * fTimeScale );
				m_pQuads->SetGPUQuadInfo( i, &vertData, sizeof( PODParticle ) );
			}
			else
			{
				static PODParticle vertData( 1.0f, CPoint(0.0f, 0.0f, 0.0f), 0.0f, CDirection(0.0f,0.0f,0.0f) );
				m_pQuads->SetGPUQuadInfo( i, &vertData, sizeof( PODParticle ) );
			}
		}
	}
}
