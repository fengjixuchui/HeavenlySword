/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef _MATERIALINSTANCE_H
#define _MATERIALINSTANCE_H

#include "material.h"
#include "gfx/renderable.h"
#include "vertexdeclaration.h"
#include "shader.h"
#include "materialbase.h"

#include "gfx/depthhazeconsts.h"
#include "core/timer.h"

class CMeshVertexElement;
class CMeshInstance;
class CHierarchy;
class CSkinMatrix;

#ifdef PLATFORM_PC
#define USE_VERTEX_SHADER_CONSTANT_CACHE
#endif

#if defined( PLATFORM_PS3 )
#include "heresy/heresy_capi.h"
#endif

/***************************************************************************************************
*
*	CLASS			CMaterialPropertyIterator
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

class CMaterialPropertyIterator
{
public:
	CMaterialPropertyIterator( CMaterialProperty const* pobMaterialProperties, 
							   int iPropertyCount );

	//! Gets the property for the given tag, or null if not found.
	CMaterialProperty const* NextProperty( int iSemanticTag );

	// Resets the iterator.
	void Reset() { m_iPropertyIndex = 0; }

private:
	CMaterialProperty const* m_pobMaterialProperties;
	int	m_iNumberOfProperties;
	int	m_iPropertyIndex;
};

/***************************************************************************************************
*
*	CLASS			CMaterialInstance
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

class CMaterialInstance: public MaterialInstanceBase
{
public:
	//! Creates a material.
	CMaterialInstance( CMaterial const* pobMaterial, 
					   CMeshVertexElement * pobVertexElements, 
					   int iVertexElementCount );
	virtual ~CMaterialInstance(){};

	//! Gets the material used for this instance.
	const CMaterial* GetMaterial() const { return m_pobMaterial; }

	// Since on the PS3 properties can be stored in shader in any order we can't user the 'old' CMaterialPropertyIterator
	// anymore, so we're using a map instead to bind property semantic with its data
	const CMaterialProperty* const GetMaterialProperty(int semantic) const;
	struct MATERIAL_DATA_CACHE
	{
		bool bApplyPosReconstMatrix;
		Transform const* pobTransform;
		CMatrix obObjectToWorld;
		CMatrix obWorldToObject;
		CMatrix obReconstructionMatrix;
	};
#if defined( PLATFORM_PS3 )
	virtual void PatchProperties(	restrict Heresy_PushBuffer* pPB, const MATERIAL_DATA_CACHE& stCache ) const;
	void PatchSkinMatrices( restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, CHierarchy* pobHierarchy ) const;
#endif

protected:
	//! partially create a material for batched render sub-classes
	CMaterialInstance( CMaterial const* pobMaterial );

	//! Uploads skin matrices to the given base register.
	void UploadSkinMatrices( Shader* pVertexShader, CHierarchy* pobHierarchy, int iBaseRegister ) const;

	//! Binds all the shaders to the vertex elements.
	void BindShaders(enum  VERTEXSHADER_TRANSFORM_TYPE type, unsigned int* puiStreamElements = 0 );

	CMaterial const* m_pobMaterial;					//!< The source material.

#ifdef PLATFORM_PC
	CScopedArray<CVertexDeclaration> m_pobDeclarations;	//!< The declarations.
#endif

	unsigned int* m_puiStreamElements;				//!< what stream each vertex element uses




	static const int SKIN_START_REGISTER = 64;

	virtual void LoadVertexProperty(	Shader* pVertexShader,
										const SHADER_PROPERTY_BINDING* pstBinding,
										const MATERIAL_DATA_CACHE& stCache ) const;

	virtual void LoadPixelProperty(		Shader* pPixelShader,
										const SHADER_PROPERTY_BINDING* pstBinding,
										const MATERIAL_DATA_CACHE& stCache ) const;

	virtual void LoadTexture( SHADER_TEXTURE_BINDING const* pstBinding, MATERIAL_DATA_CACHE const& stCache ) const;

	virtual void BindProperties( Shader* pVertexShader, Shader* pPixelShader, MATERIAL_DATA_CACHE const& stCache ) const;
	void UnBindProperties( Shader* pVertexShader, Shader* pPixelShader ) const;

#if defined( PLATFORM_PS3 )
	restrict uint32_t*  BuildPropertiesPB( restrict Heresy_PushBuffer* pPB, restrict uint32_t* pPixelShaderLocation, const CShaderGraph* pGraph ) const;
	void BuildUnBindPropertiesPB( restrict Heresy_PushBuffer* pPB, const CShaderGraph* pGraph ) const;
	void FixupVertexShaderConstant(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const MATERIAL_DATA_CACHE& stCache ) const;
	void FixupPixelShaderConstant(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const MATERIAL_DATA_CACHE& stCache ) const;
	void FixupTexture(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch ) const;
#endif

	void SetVSConst( Shader* pVertexShader, int iRegister, const void* pData, int iQWStorage ) const;
	void SetPSConst( Shader* pPixelShader, int iRegister, const void* pData, int iQWStorage ) const;
	
	void SetVSConst( Shader* pVertexShader, int iRegister, CVectorBase const& obValue ) const { SetVSConst( pVertexShader, iRegister, &obValue, 1 ); }
	void SetVSConst( Shader* pVertexShader, int iRegister, CMatrix const& obValue, int iNumRegisters = 4 ) const { SetVSConst( pVertexShader, iRegister, &obValue, iNumRegisters ); }

	void SetPSConst( Shader* pPixelShader, int iRegister, CVectorBase const& obValue ) const { SetPSConst( pPixelShader, iRegister, &obValue, 1 ); }
	void SetPSConst( Shader* pPixelShader, int iRegister, CMatrix const& obValue, int iNumRegisters = 4 ) const { SetPSConst( pPixelShader, iRegister, &obValue, iNumRegisters ); }

	// 4K per material to try and speed up LoadVertexProperty
	void ResetVertexShaderConstantCache() const
	{
#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE
		m_iMinRegister = 0xFFFF;
		m_iMinSkinRegister = 0xFFFF;
		m_iMaxRegister = -1;
#endif
	}
	void UploadVertexShaderConstants() const;

#ifdef USE_VERTEX_SHADER_CONSTANT_CACHE
	mutable float m_fVertexConstantCache[256*4];
	mutable int m_iMinRegister;
	mutable int m_iMaxRegister;
	mutable int m_iMinSkinRegister;
	mutable int m_iNumSkinRegisters;
	mutable unsigned int m_uiVertexCacheTick; //!< is the cache valid
#endif
};

/***************************************************************************************************
*
*	CLASS			CGameMaterialInstance
*
*	DESCRIPTION		<Insert Here>
*
***************************************************************************************************/

class CGameMaterialInstance : public CMaterialInstance
{
public:
	//! Creates a material from the given material property table.
	CGameMaterialInstance( CMaterial const* pobMaterial, 
						   CMeshVertexElement * pobVertexElements, 
						   int iVertexElementCount, 
						   CMaterialProperty const* pobProperties, 
						   int iPropertyCount );


	//! Called before rendering colour buffers.
	virtual void PreRender( Transform const* pobTransform, bool bRecieveShadows, void const* pDecompMatrix = NULL ) const;
	//! Called after rendering colour buffers.
	void PostRender() const;

	//! called before rendering depth (shadow map) buffers.
	void PreRenderDepth( Transform const* pobTransform, bool bShadowProject, void const* pDecompMatrix = NULL ) const;
	//! called after rendering depth (shadow map) buffers.
	void PostRenderDepth( bool bShadowProject ) const;

	//! called before rendering the recieving shadow map buffer.
	void PreRenderShadowRecieve( Transform const* pobTransform, void const* pDecompMatrix = NULL ) const;
	//! called afterrendering the recieving shadow map buffer.
	void PostRenderShadowRecieve() const;

#if defined( PLATFORM_PS3 )
	//! called before rendering depth (shadow map) buffers.
	bool BuildPreRenderDepthPB( restrict Heresy_PushBuffer* pPB, const VBHandle* pVB, bool bShadowProject ) const;
	//! called after rendering depth (shadow map) buffers.
	bool BuildPostRenderDepthPB( Heresy_PushBuffer* pPB, const VBHandle* pVB, bool bShadowProject ) const;

	//! Called before rendering colour buffers.
	bool BuildPreRenderPB( restrict Heresy_PushBuffer* pPB, const VBHandle* pVB, uint32_t uiStreamCount,  bool bShadowRecieve ) const;
	//! Called after rendering colour buffers.
	bool BuildPostRenderPB( Heresy_PushBuffer* pPB, const VBHandle* pVB, uint32_t uiStreamCount ) const;
#endif

protected:
	//! partially create a material for batched render sub-classes
	CGameMaterialInstance( CMaterial const* pobMaterial );

	static int GetTechniqueIndex( bool bReceiveShadow );

	mutable int m_iTechniqueIndex;
};

#endif // ndef _MATERIALINSTANCE_H
