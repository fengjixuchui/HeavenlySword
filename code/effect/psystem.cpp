//--------------------------------------------------
//!
//!	\file psystem.cpp
//!	core of the particle system
//!
//--------------------------------------------------

#include "psystem.h"
#include "objectdatabase/dataobject.h"
#include "effect_error.h"
#include "emission_function.h"
#include "particle_spawning.h"
#include "effect/psystem_utils.h"
#include "gfx/textureatlas.h"
#include "gfx/texturemanager.h"
#include "gfx/fxmaterial.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "colour_function.h"
#include "gfx/levellighting.h"

// for debug rendering only
#include "core/visualdebugger.h"
#include "objectdatabase/neteditinterface.h"

//--------------------------------------------------
//!
//!	ParticleSystemStdParams::RequiresBounceCode
//! Do we require bouncing against an arbitary plane, or havok ray checks?
//!
//--------------------------------------------------
ParticleSystemStdParams::ParticleSystemStdParams() :
	m_iRandomSeed( eseed() ),
	m_acceleration( CONSTRUCT_CLEAR ),
	m_fParticleLifetime( 1.0f ),
	m_particleType( PT_SIMPLE_SPRITE ),

	m_pSpawnDef( 0 ),
	m_pDefaultEmitterDef( 0 ),
	m_pParticleDef( 0 ),
	m_pBouncePlaneDef( 0 ),

	m_bUseRandAtlasTex( false ),
	m_bSortedParticles( false ),
	m_fCullRadius( 1.0f ),
	m_fSortingPush( 0.0f ),

	m_fRestitution( 1.0f ),
	m_bUseRayCast( false )
{}

//--------------------------------------------------
//!
//!	ParticleSystemStdParams::EstablishParticleType
//!
//--------------------------------------------------
void ParticleSystemStdParams::EstablishParticleType()
{
	m_particleType = PT_SIMPLE_SPRITE;

	if (m_pParticleDef != NULL)
	{
		const char* pType =  EffectUtils::GetInterfaceType( m_pParticleDef );

		if (stricmp(pType,"ParticleDef_Rotating")==0)
		{
			m_particleType = PT_ROTATING_SPRITE;
		}
		else if (stricmp(pType,"ParticleDef_WorldQuad")==0)
		{
			m_particleType = PT_WORLD_ALIGNED_QUAD;
		}
		else if (stricmp(pType,"ParticleDef_AxisQuad")==0)
		{
			m_particleType = PT_AXIS_ALIGNED_RAY;
		}
		else if (stricmp(pType,"ParticleDef_VelScaledRay")==0)
		{
			m_particleType = PT_VELOCITY_ALIGNED_RAY;
		}
		else
		{
			#ifndef _RELEASE
			char ntError[512];
			sprintf( ntError, "PARTICLES: particle definition %s is of unrecognised type %s\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( m_pParticleDef )), pType );
			EffectErrorMSG::AddDebugError( ntError );
			#endif
		}
	}
}

//--------------------------------------------------
//!
//!	ParticleSystemStdParams::RequiresBounceCode
//! Do we require bouncing against an arbitary plane, or havok ray checks?
//!
//--------------------------------------------------
bool	ParticleSystemStdParams::RequiresBounceCode() const
{
	return ((m_pBouncePlaneDef != NULL) || (m_bUseRayCast));
}

//--------------------------------------------------
//!
//!	ParticleSystemStdParams::RequiresSorting
//! Do we require inter-particle sorting?
//!
//--------------------------------------------------
bool	ParticleSystemStdParams::RequiresSorting() const
{
	if	(
		(m_bSortedParticles) &&						// asked for
		(!m_renderStateDef.m_bZWriteEnable) &&		// transparent
		(m_renderStateDef.m_blendMode == EBM_LERP)	// modulate only
		)
		return true;
	return false;
}
		
//--------------------------------------------------
//!
//!	ParticleSystemStdParams::RequiresCPUParticle
//! Do we need to be updated on the CPU or GPU
//!
//--------------------------------------------------
bool	ParticleSystemStdParams::RequiresCPUParticle() const
{
	if	(
		(RequiresBounceCode()) ||
		(!HardwareCapabilities::Get().SupportsVertexShader3()) ||
		(RequiresSorting()) ||
		(m_pSpawnDef)
		)
		return true;
	return false;
}

//--------------------------------------------------
//!
//!	ParticleSystemStdParams::CanUsePointSprite
//! Can we use the more efficient point sprite primitive?
//! 
//--------------------------------------------------
bool	ParticleSystemStdParams::CanUsePointSprite() const
{
	if	(
		(!HardwareCapabilities::Get().SupportsVertexShader3()) || // we use extra interpolators defined in SM3
		(m_renderStateDef.m_renderType == ERT_HDR_DEPTH_HAZED) || // not enough interpolators
		(m_bUseRandAtlasTex) // we use texcoord.z to store the rand in, hence no dice.
		)
		return false;

	return true;
}

//--------------------------------------------------
//!
//!	ParticleSystemStdParams::ResolveTextureMode
//! Get what type of texture mode we are
//!
//--------------------------------------------------
void	ParticleSystemStdParams::ResolveTextureMode()
{
	m_eTexMode = PTM_UNTEXTURED;

	// setup texture if any
	if ( !ntStr::IsNull(m_texName))
	{
		// are we using a texture atlas?
		const char* pName = ntStr::GetString(m_texName);
		if	(
			( TextureAtlasManager::Get().IsAtlas(pName) ) &&
			( TextureAtlasManager::Get().Exists(pName) )
			)
		{
			// yup, now get what type we are
			if (m_bUseRandAtlasTex)
				m_eTexMode = PTM_RAND_TEXTURED;
			else
				m_eTexMode = PTM_ANIM_TEXTURED;
		}
		// FIXME_WIL. When we pre-load particle resources, this should
		// change to Loaded_Neutral(). This is just a level load optimisation
		else if ( TextureManager::Get().Exists_Neutral(pName) )
		{
			m_eTexMode = PTM_SIMPLE_TEXTURED;
		}

		#ifndef _RELEASE
		if (m_eTexMode == PTM_UNTEXTURED)
		{
			static char aErrors[MAX_PATH];
			sprintf( aErrors, "Texture %s required by particle system %s does not exist", pName, GetDebugName() );
			EffectErrorMSG::AddDebugError( aErrors );
		}
		#endif
	}
}

//--------------------------------------------------
//!
//!	ParticleSystemStdParams::LoadTexture
//!	Load any additional resources (atlas, texture)
//!
//--------------------------------------------------
void	ParticleSystemStdParams::LoadTexture()
{
	if	(
		( m_eTexMode == PTM_RAND_TEXTURED ) ||
		( m_eTexMode == PTM_ANIM_TEXTURED )
		)
	{
		m_pAtlas = TextureAtlasManager::Get().GetAtlas( ntStr::GetString(m_texName) );
		ntAssert_p( m_pAtlas, ("Problem with texture atlas %s\n", ntStr::GetString(m_texName)) );

		// make sure theyre all the same size and shape,
		#ifndef _RELEASE
		u_int iNumTex = m_pAtlas->GetNumEntries();
		float fWidth = m_pAtlas->GetEntryByIndex(0)->GetWidth();
		for ( u_int i = 0; i < iNumTex; i++ )
		{
			ntError( m_pAtlas->GetEntryByIndex(i)->GetHeight() == 1.0f );
			ntError( m_pAtlas->GetEntryByIndex(i)->GetWidth() == fWidth );
		}
		#endif
	}
	else if ( m_eTexMode == PTM_SIMPLE_TEXTURED )
	{
		m_pTex = TextureManager::Get().LoadTexture_Neutral( ntStr::GetString(m_texName) );
	}
}


//--------------------------------------------------
//!
//!	ParticleSystem::ConstructInternal
//! Base particle system ctor
//!
//--------------------------------------------------
void ParticleSystem::ConstructInternal(	const ParticleSystemStdParams* pBaseDef,
										const CMatrix*		pFrame,
										const Transform*	pTransform,
										const EmitterDef*	pOverideEmit )
{
	m_pBaseDef = pBaseDef;
	m_pTransform = pTransform;
	m_pEmitter = 0;
	m_pSpawner = 0;
	m_pEmitterInitInfo = pOverideEmit ? pOverideEmit : m_pBaseDef->m_pDefaultEmitterDef;

	if (!m_pTransform)
		m_emitterFrameCurr = *pFrame;

	m_pBaseDef->m_resetSet.RegisterThingToReset(this);
}

ParticleSystem::~ParticleSystem()
{
	m_pBaseDef->m_resetSet.UnRegisterThingToReset(this);
}

//--------------------------------------------------
//!
//!	ParticleSystem::ResetBase
//! Called by derived classes
//! Establishes texture mode required by derived classes
//!
//--------------------------------------------------
void ParticleSystem::ResetBase( bool bInDestructor )
{
	m_fLastEmitTime = 0.0f;
	m_renderstates = m_pBaseDef->m_renderStateDef;
	m_renderstates.m_bDisablePolyCulling = true;
	m_iMaxParticles = 0;

	m_fCullingRadius = m_pBaseDef->m_fCullRadius;
	m_fSortingPush = m_pBaseDef->m_fSortingPush;
	m_iLastRand = m_pBaseDef->m_iRandomSeed;

	m_proceduralVel.Clear();
	m_proceduralCol = CVector(1.0f,1.0f,1.0f,1.0f);
	m_fEmitMultiplier = 1.0f;

	if (m_iLastRand == -1)
		m_iLastRand = erand();

	if (m_pEmitter)
	{
		m_pEmitter->GetDef().m_resetSet.UnRegisterThingToReset(this);
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pEmitter );
		m_pEmitter = 0;
	}

	if (m_pSpawner)
	{
		// force immediate clean of child effects if we're being reset, rather than dying
		m_pSpawner->Reset( !bInDestructor );
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pSpawner );
		m_pSpawner = 0;
	}
	
	if (!bInDestructor)
	{
		// create emitter
		ntAssert(m_pEmitterInitInfo);
		m_pEmitter = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) Emitter( *m_pEmitterInitInfo );
		m_pEmitter->GetDef().m_resetSet.RegisterThingToReset(this);

		// get initial transform
		if (m_pTransform)
		{
			m_emitterFrameCurr = m_pTransform->GetWorldMatrix();
			m_emitterPosOld = m_emitterPosCurr = m_pEmitter->GetCurrState().m_offset * m_emitterFrameCurr;
			m_emitterVelOld.Clear(); m_emitterVelCurr.Clear();
		}

		// get culling origin
		m_cullingOrigin = m_pEmitter->GetCurrState().m_offset * m_emitterFrameCurr;

		// calc max particles possible
		m_iMaxParticles = m_pEmitter->GetDef().CalcMaximumEmitted( m_pBaseDef->m_fParticleLifetime );

		// create spawn handler
		if (m_pBaseDef->m_pSpawnDef)
			m_pSpawner = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) ParticleSpawner( m_pBaseDef->m_pSpawnDef, m_iMaxParticles );
	}
}

//--------------------------------------------------
//!
//!	ParticleSystem::RetrieveWorldInfo
//! gather all transform info required this frame
//!
//--------------------------------------------------
void ParticleSystem::RetrieveWorldInfo( float fTimeDelta )
{
	UNUSED(fTimeDelta);

	if	(
		( m_pTransform ) &&
		((m_bKillMeRequested) || (m_bKillMeNow))
		)
		m_pTransform = 0;

#ifndef _RELEASE
	m_fCullingRadius = m_pBaseDef->m_fCullRadius;
	m_fSortingPush = m_pBaseDef->m_fSortingPush;
#endif

	if (m_pTransform)
	{
		if (m_pEmitter->GetDef().m_bWorldSpaceEffect == false)
		{
			// note. for local space effects we donot bother with splines and the like
			// as there is no interpolation to be done.
			m_emitterFrameCurr = m_pTransform->GetWorldMatrix();
			m_cullingOrigin = m_pEmitter->GetCurrState().m_offset * m_emitterFrameCurr;
		}
		else
		{
			// buffer old info
			m_startQuat = CQuat(m_emitterFrameCurr);

			CDirection emitterVelOldOld = m_emitterVelOld;
			CPoint emitterPosOldOld = m_emitterPosOld;
			m_emitterVelOld = m_emitterVelCurr;
			m_emitterPosOld = m_emitterPosCurr;

			// get info
			m_emitterFrameCurr = m_pTransform->GetWorldMatrix();
			m_endQuat = CQuat(m_emitterFrameCurr);

			m_emitterPosCurr = m_pEmitter->GetCurrState().m_offset * m_emitterFrameCurr;
			m_emitterVelCurr = (m_emitterPosCurr ^ m_emitterPosOld) / fTimeDelta;

			// predict future velocity and thus position
			CDirection nextVel = (3.0f*m_emitterVelCurr) - (3.0f*m_emitterVelOld) + emitterVelOldOld;
			CPoint nextPos = m_emitterPosCurr + (nextVel*fTimeDelta);

			static const float gafBasisMat[4][4] =
			{
				{ -0.5f,  1.5f, -1.5f,  0.5f },
				{  1.0f, -2.5f,  2.0f, -0.5f },
				{ -0.5f,  0.0f,  0.5f,  0.0f },
				{  0.0f,  1.0f,  0.0f,  0.0f }
			};

			// generate some catmull coeffs for this frame
			for (int i = 0; i < 4; i++)
			{
				m_posSplineCoeffs[i].Clear();
				m_posSplineCoeffs[i] += CVector(emitterPosOldOld)	* gafBasisMat[i][0];
				m_posSplineCoeffs[i] += CVector(m_emitterPosOld)	* gafBasisMat[i][1];
				m_posSplineCoeffs[i] += CVector(m_emitterPosCurr)	* gafBasisMat[i][2];
				m_posSplineCoeffs[i] += CVector(nextPos)			* gafBasisMat[i][3];

				if (m_pEmitter->GetCurrState().m_bInheritTransVel)
				{
					m_velSplineCoeffs[i].Clear();
					m_velSplineCoeffs[i] += CVector(emitterVelOldOld)	* gafBasisMat[i][0];
					m_velSplineCoeffs[i] += CVector(m_emitterVelOld)	* gafBasisMat[i][1];
					m_velSplineCoeffs[i] += CVector(m_emitterVelCurr)	* gafBasisMat[i][2];
					m_velSplineCoeffs[i] += CVector(nextVel)			* gafBasisMat[i][3];
				}
			}

			m_cullingOrigin = m_emitterPosCurr;
		}
	}
}

//--------------------------------------------------
//!
//!	ParticleSystem::UpdateParticleSystem
//! update the particle system
//!
//--------------------------------------------------
bool ParticleSystem::UpdateParticleSystem( float fTimeDelta )
{
	ntAssert(m_pEmitter);
	ntAssert(fTimeDelta >= 0.0f);

	// get latest info for emitter
	RetrieveWorldInfo( fTimeDelta );

	// buffer the old rand, push on our one
	int iBufferRand = eseed();
	erands( m_iLastRand );

	// update the emitter.
	bool bFinished = m_pEmitter->Update<ParticleSystem>( this, m_bKillMeRequested, fTimeDelta, m_fEmitMultiplier );

	// kill ourselves if we're supposed to auto destruct
	if	(
		(bFinished) && 
		((m_pEmitter->GetAge() - m_fLastEmitTime) >= m_pBaseDef->m_fParticleLifetime)
		)
		m_bKillMeNow = true;

	// pass back control to derived object
	UpdateParticles( fTimeDelta );

	// now update our spawner if we have one
	if (m_pSpawner)
		m_pSpawner->Update();

	// update our local rand, push back the old one
	m_iLastRand = eseed();
	erands( iBufferRand );

	return m_bKillMeNow;
}

//--------------------------------------------------
//!
//!	ParticleSystem::RetriveNewPosAndVel
//! Get a position and velocity for an emitted particle
//!
//--------------------------------------------------
void ParticleSystem::EmitParticle( float fNormalisedT )
{
	// retrive position and velocity
	CPoint		position;
	CDirection	velocity;

	if (m_pEmitter->GetDef().m_bWorldSpaceEffect == false)
	{
		// simple case just get position and possibly back transform our velocity.
		// from world to object space (not sure there's an awful lot of point to that)

		position = m_pEmitter->GetCurrState().GetNewPosition();
		velocity = m_pEmitter->GetCurrState().GetNewVelocity();

		if (m_pEmitter->GetDef().m_bSpawnDirInWorld)
			velocity = velocity * m_emitterFrameCurr.GetAffineInverse();
	}
	else if (m_pTransform)
	{
		CMatrix interpolated;
		interpolated.SetFromQuat( CQuat::Slerp( m_startQuat, m_endQuat, fNormalisedT ) );

		// evaluate our posSpline for the frame translation
		interpolated.SetTranslation( GetSmoothPos(fNormalisedT) );

		// generate initial position and velocity
		position = m_pEmitter->GetCurrState().GetNewPositionNoOffset() * interpolated;
		
		if (m_pEmitter->GetDef().m_bSpawnDirInWorld)
			velocity = m_pEmitter->GetCurrState().GetNewVelocity();
		else
			velocity = m_pEmitter->GetCurrState().GetNewVelocity() * interpolated;

		// evaluate our velSpline for smooth additional velocities
		if (m_pEmitter->GetCurrState().m_bInheritTransVel)
			velocity += GetSmoothVel(fNormalisedT) * m_pEmitter->GetCurrState().m_fInheritVelScalar;
	}
	else
	{
		position = m_pEmitter->GetCurrState().GetNewPosition() * m_emitterFrameCurr;

		if (m_pEmitter->GetDef().m_bSpawnDirInWorld)
			velocity = m_pEmitter->GetCurrState().GetNewVelocity();
		else
			velocity = m_pEmitter->GetCurrState().GetNewVelocity() * m_emitterFrameCurr;
	}

	// calc actual emission time
	m_fLastEmitTime = ((m_pEmitter->GetAge() - m_pEmitter->GetOldAge()) * fNormalisedT)
						+ m_pEmitter->GetOldAge();

	// get our derived class to give us a particle to track
	pos_vel_PD* pNewParticle = SpawnNewParticle( m_pEmitter->GetAge(), m_fLastEmitTime, position, velocity + m_proceduralVel );

	if (m_pSpawner)
	{
		ntAssert_p( pNewParticle, ("Must have a CPU particle for spawner to track") );
		m_pSpawner->NewParticleToTrack( pNewParticle );
	}
}

//--------------------------------------------------
//!
//!	ParticleSystem::UploadStdParameters
//! upload local parameters... these are specific to this particle effect
//!
//--------------------------------------------------
#ifdef PLATFORM_PC

void ParticleSystem::UploadStdParameters( ID3DXEffect* pFX )
{
	ntAssert(pFX);

	EffectUtils::SetGlobalFXParameters(pFX);

	// these should be based on whether the effect is in local transform space or world space...
	CMatrix objectToWorld( CONSTRUCT_IDENTITY );

	if (m_pEmitter->GetDef().m_bWorldSpaceEffect == false)
		objectToWorld = m_emitterFrameCurr;

	FX_SET_VALUE_VALIDATE( pFX, "m_objectWorld", &objectToWorld, sizeof(CMatrix) );
	FX_SET_VALUE_VALIDATE( pFX, "m_worldViewProj", &RenderingContext::Get()->m_worldToScreen, sizeof(CMatrix) );

	if (m_renderstates.m_renderType == ERT_HDR_DEPTH_HAZED)
	{
		CMatrix worldToObject( objectToWorld.GetAffineInverse() );

		FX_SET_VALUE_VALIDATE( pFX, "m_worldView", &RenderingContext::Get()->m_worldToView, sizeof(CMatrix) );

		CPoint viewPos = RenderingContext::Get()->GetEyePos() * worldToObject;
		FX_SET_VALUE_VALIDATE( pFX, "m_viewPosition_objectS", &viewPos, sizeof(float) * 3 );

		CVector temp( RenderingContext::Get()->m_sunDirection * worldToObject );
		FX_SET_VALUE_VALIDATE( pFX, "m_sunDir_objectS", &temp, sizeof(float) * 3 );
	}

	// age
	float fAge = m_pEmitter->GetAge();
	FX_SET_VALUE_VALIDATE( pFX, "m_emitterAge", &fAge, sizeof(float) );

	D3DXHANDLE h;

	// textures
	switch( m_pBaseDef->m_eTexMode )
	{
	case PTM_SIMPLE_TEXTURED:
		{
			FX_GET_HANDLE_FROM_NAME( pFX, h, "m_diffuse0" );
			pFX->SetTexture( h, m_pBaseDef->m_pTex->m_Platform.Get2DTexture() );
		}
		break;

	case PTM_ANIM_TEXTURED:
	case PTM_RAND_TEXTURED:
		{
			FX_GET_HANDLE_FROM_NAME( pFX, h, "m_diffuse0" );
			pFX->SetTexture( h, m_pBaseDef->m_pAtlas->GetAtlasTexture()->m_Platform.Get2DTexture() );

			float fNumTextures = _R(m_pBaseDef->m_pAtlas->GetNumEntries());
			float fTexWidth = m_pBaseDef->m_pAtlas->GetEntryByIndex(0)->GetWidth();

			FX_SET_VALUE_VALIDATE( pFX, "m_TA_numTex", &fNumTextures, sizeof(float) );
			FX_SET_VALUE_VALIDATE( pFX, "m_TA_TexWidth", &fTexWidth, sizeof(float) );
		}
		break;
	}

	// Time of day global colour modifier
	CVector TODModifier(1.0f,1.0f,1.0f,1.0f);
	if (m_renderstates.m_pTimeOfDayMod)
	{
		float fTime = LevelLighting::Get().GetTimeOfDayN();
		TODModifier = m_renderstates.m_pTimeOfDayMod->GetColour( fTime );
	}
	TODModifier *= m_proceduralCol;
	FX_SET_VALUE_VALIDATE( pFX, "m_TODModifier", &TODModifier, sizeof(float)*4 );
}

#else

void ParticleSystem::UploadStdParameters(	Shader& vertexShader,
											Shader& pixelShader,
											bool bPointSprite,
											bool bCPUpartices )
{
	// these are globals
	//-------------------------------------------------------

	CMatrix ImageTransform( CONSTRUCT_IDENTITY );
	if (m_pBaseDef->m_renderStateDef.m_renderType == ERT_LOW_DYNAMIC_RANGE)
		ImageTransform = RenderingContext::Get()->m_postProcessingMatrix;
	
	pixelShader.SetPSConstantByName( "g_ImageTransform", ImageTransform, 4 );

	// particle type (vertex shader) specific globals
	//-------------------------------------------------------
	
	if	(
		(m_pBaseDef->m_particleType == PT_SIMPLE_SPRITE) ||
		(m_pBaseDef->m_particleType == PT_ROTATING_SPRITE)
		)
	{
		CMatrix camMat = RenderingContext::Get()->m_pViewCamera->GetViewTransform()->GetWorldMatrix();

		if (bPointSprite)
		{
			// should only be required by point sprites
			CPoint cameraZ( camMat.GetZAxis() );
			cameraZ.W() = -( camMat.GetTranslation().Dot( cameraZ ) );
			vertexShader.SetVSConstantByName( "g_cameraZ", cameraZ );

			CVector vpScalars(	Renderer::Get().m_targetCache.GetWidthScalar(),
								Renderer::Get().m_targetCache.GetHeightScalar(),
								0.0f, 0.0f );

			vertexShader.SetVSConstantByName( "g_vpScalars", vpScalars );
		}
		else
		{
			// should only be required by quads
			vertexShader.SetVSConstantByName( "g_cameraUnitAxisX", camMat.GetXAxis() );
			vertexShader.SetVSConstantByName( "g_cameraUnitAxisY", camMat.GetYAxis() );
		}
	}
	else if	(
			(m_pBaseDef->m_particleType == PT_AXIS_ALIGNED_RAY) ||
			(m_pBaseDef->m_particleType == PT_VELOCITY_ALIGNED_RAY)
			)
	{
		CPoint eyePos = RenderingContext::Get()->GetEyePos();
		vertexShader.SetVSConstantByName( "g_eyePos", eyePos );
	}

	// these should be based on whether the effect is in local transform space or world space...
	CMatrix objectToWorld( CONSTRUCT_IDENTITY );

	if (m_pEmitter->GetDef().m_bWorldSpaceEffect == false)
		objectToWorld = m_emitterFrameCurr;

	vertexShader.SetVSConstantByName( "m_objectWorld", objectToWorld, 4 );
	vertexShader.SetVSConstantByName( "m_worldViewProj", RenderingContext::Get()->m_worldToScreen, 4 );

	// age - only required by GPU based psystems
	if (!bCPUpartices)
	{
		CVector Age( m_pEmitter->GetAge(), 0.0f, 0.0f, 0.0f );
		vertexShader.SetVSConstantByName( "m_emitterAge", Age );
	}

	int iGammaCorrect = (m_pBaseDef->m_renderStateDef.m_renderType == ERT_HIGH_DYNAMIC_RANGE) ? Gc::kGammaCorrectSrgb : 0;

	// textures
	switch( m_pBaseDef->m_eTexMode )
	{
	case PTM_SIMPLE_TEXTURED:
		{
			m_pBaseDef->m_pTex->m_Platform.GetTexture()->SetGammaCorrect( iGammaCorrect );
			Renderer::Get().SetTexture( 0, m_pBaseDef->m_pTex );
			Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
			Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );
		}
		break;

	case PTM_ANIM_TEXTURED:
	case PTM_RAND_TEXTURED:
		{
			CVector NumTextures( _R(m_pBaseDef->m_pAtlas->GetNumEntries()), 0.0f, 0.0f, 0.0f );
			pixelShader.SetPSConstantByName( "m_TA_numTex", NumTextures );

			CVector TexWidth( m_pBaseDef->m_pAtlas->GetEntryByIndex(0)->GetWidth(), 0.0f, 0.0f, 0.0f );
			pixelShader.SetPSConstantByName( "m_TA_TexWidth", TexWidth );

			m_pBaseDef->m_pAtlas->GetAtlasTexture()->m_Platform.GetTexture()->SetGammaCorrect( iGammaCorrect );
			Renderer::Get().SetTexture( 0, m_pBaseDef->m_pAtlas->GetAtlasTexture() );
			Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
			Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_BILINEAR );

		}
		break;

	default:
		break;
	}

	// Time of day global colour modifier
	CVector TODModifier(1.0f,1.0f,1.0f,1.0f);
	if (m_renderstates.m_pTimeOfDayMod)
	{
		float fTime = LevelLighting::Get().GetTimeOfDayN();
		TODModifier = m_renderstates.m_pTimeOfDayMod->GetColour( fTime );
	}

	pixelShader.SetPSConstantByName( "m_TODModifier", TODModifier );
}

#endif

//--------------------------------------------------
//!
//!	ParticleSystem::EmitterEdited
//!
//--------------------------------------------------
bool ParticleSystem::EmitterEdited() const
{
	if (CNetEditInterface::Get().GetSelected() == ObjectDatabase::Get().GetDataObjectFromPointer(&m_pEmitter->GetDef()) )
		return true;

	return false;
}

//--------------------------------------------------
//!
//!	ParticleSystem::DebugRenderEmitter
//!
//--------------------------------------------------
void ParticleSystem::DebugRenderEmitter( bool bEditing )
{
	m_pEmitter->GetCurrState().DebugRender( m_emitterFrameCurr,
							m_pEmitter->GetDef().m_bSpawnDirInWorld );

	if (bEditing)
	{
		m_pEmitter->DebugRender();
		DebugRenderCullVolume();
	}
}

//--------------------------------------------------
//!
//!	ParticleSystem::DebugRenderInfo
//!
//--------------------------------------------------
void ParticleSystem::DebugRenderInfo(	const char* pCPUCost,
										const char* pVSCost,
										const char* pPSCost,
										const char* pTechnique,
										const char* pType )
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D( m_cullingOrigin, 0xffffffff, 0, "%s (%d) (%s)", m_pBaseDef->GetDebugName(), m_iMaxParticles, pTechnique );
	g_VisualDebug->Printf3D( m_cullingOrigin, 0.f, 12.f, 0xffffffff, 0, "CPU(%s) GPUVS(%s) GPUPS(%s)", pCPUCost, pVSCost, pPSCost );
	g_VisualDebug->Printf3D( m_cullingOrigin, 0.f, 24.f, 0xffffffff, 0, "(%s)", pType );
#endif
}




