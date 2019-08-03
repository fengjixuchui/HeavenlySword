//--------------------------------------------------
//!
//!	\file psystem_utils.h
//!	misc helper functions for particle systems
//!
//--------------------------------------------------

#ifndef _PSYSTEM_UTILS_H
#define _PSYSTEM_UTILS_H

class QuadList;
class PSystemSimple;
class PSystemComplex;
class RenderStateBlock;
class Transform;
class EmitterDef;

//--------------------------------------------------
//!
//!	PARTICLE_TEXTURE_MODE
//! Are we textured, animated or what?
//!
//--------------------------------------------------
enum PARTICLE_TEXTURE_MODE
{
	PTM_UNTEXTURED,			// no tex
	PTM_SIMPLE_TEXTURED,	// single tex
	PTM_ANIM_TEXTURED,		// animate through a texture dict
	PTM_RAND_TEXTURED,		// random choice in texture dict
};

//--------------------------------------------------
//!
//!	PSystemUtils
//!
//--------------------------------------------------
class PSystemUtils
{
public:
	static const char* GetTechniqueName( u_int iTechnique );
	static const char* GetTechniqueName(const PSystemSimple& effect);
	static const char* GetTechniqueName(const PSystemComplex& effect);

	static void GetApproxCost(const PSystemSimple& effect, const char** ppCPU, const char** ppVS, const char** ppPS );
	static void GetApproxCost(const PSystemComplex& effect, const char** ppCPU, const char** ppVS, const char** ppPS );

#ifdef PLATFORM_PC
	static void SetFXTechique( ID3DXEffect* pFX, PARTICLE_TEXTURE_MODE eTexMode, const RenderStateBlock& rs );
#endif

	static void InitialiseQuadList( QuadList& quads, void* pCPUTemplate, u_int iBirthSlot, u_int iTextureSlot = 0xffffffff, bool bSetRand = false );
	static u_int ConstructParticleEffect( void* pDef, const CMatrix& frame, const EmitterDef* pEmitterOveride = 0 );
	static u_int ConstructParticleEffect( void* pDef, const Transform* pTransform, const EmitterDef* pEmitterOveride = 0 );
	static u_int ConstructParticleEffect( void* pDef, const CHashedString& pEntName, const CHashedString& pTransformName, const EmitterDef* pEmitterOveride = 0 );

private:
	static u_int GetTechniqueID( PARTICLE_TEXTURE_MODE eTexMode, const RenderStateBlock& rs );
	static const char* GetVSCost( u_int iScore );
	static const char* GetPSCost( u_int iScore );
	static const char* GetCPUCost( u_int iScore );
};

#endif // _PSYSTEM_UTILS_H
