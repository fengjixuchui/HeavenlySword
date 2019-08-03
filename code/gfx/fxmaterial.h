//--------------------------------------------------
//!
//!	\file fxmaterial.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _FX_MATERIAL_H
#define _FX_MATERIAL_H

#include "vertexdeclaration.h"
#include "materialbase.h"
#include "core/semantics.h"
#include "core/bitmask.h"

typedef CComPtr<ID3DXEffectPool> FXPoolHandle;

class CMeshVertexElement;
class CMaterialProperty;
class Transform;
class CMatrix;

//--------------------------------------------------
//!
//! Handle class for an ID3DXEffect object
//!
//--------------------------------------------------
class FXHandle : public CComPtr<ID3DXEffect>
{
public:
	FXHandle() : m_pPool(0) {};
	FXHandle( const char* pFXFile, const char* pName, ID3DXEffectPool* pPool = NULL );
	FXHandle(const FXHandle& handle) :
		CComPtr<ID3DXEffect>(handle)
	{
		strcpy( m_pName, handle.m_pName );
		strcpy( m_pFileName, handle.m_pFileName );
		m_modDate = handle.m_modDate;
		m_pPool = handle.m_pPool;
	}

	FXHandle& operator=(const FXHandle& handle)
	{
		CComPtr<ID3DXEffect> cpy(handle);
		Swap(cpy);

		strcpy( m_pName, handle.m_pName );
		strcpy( m_pFileName, handle.m_pFileName );
		m_modDate = handle.m_modDate;
		m_pPool = handle.m_pPool;

		return *this;
	}

	const char* GetName() const { return m_pName; }
	const char* GetFileName() const { return m_pFileName; }

	void ReloadMe( bool bForceRecompile = false );
	bool IsOutOfDate() const;

private:
	char m_pName[ MAX_PATH ];
	char m_pFileName[ MAX_PATH ];
	time_t m_modDate;
	ID3DXEffectPool* m_pPool;
};

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
class FXMaterial
{
public:
	FXMaterial( const char* pFXFile, const char* pName );

	enum FX_MAT_TECHNIQUE
	{
		FX_STATIC_NOT_SHADOWED = 0,
		FX_STATIC_GET_SHADOW_TERM,
		FX_STATIC_DEPTH_OUTPUT,
		FX_STATIC_RECIEVE_SHADOW,
		FX_STATIC_RECIEVE_SHADOW_SMH,

		FX_SKINNED_NOT_SHADOWED,
		FX_SKINNED_GET_SHADOW_TERM,
		FX_SKINNED_DEPTH_OUTPUT,
		FX_SKINNED_RECIEVE_SHADOW,
		FX_SKINNED_RECIEVE_SHADOW_SMH,

		FX_BATCHED_NOT_SHADOWED,
		FX_BATCHED_GET_SHADOW_TERM,
		FX_BATCHED_DEPTH_OUTPUT,
		FX_BATCHED_RECIEVE_SHADOW,
		FX_BATCHED_RECIEVE_SHADOW_SMH,

		FX_MAT_MAX_TECHNIQUES,
	};

	static const char* GetTechniqueName( FX_MAT_TECHNIQUE eTechnique )
	{
		switch (eTechnique)
		{
		case FX_STATIC_NOT_SHADOWED:		return "notshadowed";
		case FX_STATIC_GET_SHADOW_TERM:		return "getshadowterm";
		case FX_STATIC_DEPTH_OUTPUT:		return "depth_output";
		case FX_STATIC_RECIEVE_SHADOW:		return "recieve_shadow";
		case FX_STATIC_RECIEVE_SHADOW_SMH:	return "recieve_shadowSMH";

		case FX_SKINNED_NOT_SHADOWED:		return "skinned_notshadowed";
		case FX_SKINNED_GET_SHADOW_TERM:	return "skinned_getshadowterm";
		case FX_SKINNED_DEPTH_OUTPUT:		return "skinned_depth_output";
		case FX_SKINNED_RECIEVE_SHADOW:		return "skinned_recieve_shadow";
		case FX_SKINNED_RECIEVE_SHADOW_SMH:	return "skinned_recieve_shadowSMH";

		case FX_BATCHED_NOT_SHADOWED:		return "batched_notshadowed";
		case FX_BATCHED_GET_SHADOW_TERM:	return "batched_getshadowterm";
		case FX_BATCHED_DEPTH_OUTPUT:		return "batched_depth_output";
		case FX_BATCHED_RECIEVE_SHADOW:		return "batched_recieve_shadow";
		case FX_BATCHED_RECIEVE_SHADOW_SMH:	return "batched_recieve_shadowSMH";

		default:
			ntError( 0 );
			return NULL;
		}
	}

	// body of these are static so NON-material instances can use the same upload code
	static void UploadGlobalParameters( const FXHandle& handle );
	static void UploadObjectParameters( const FXHandle& handle,
										const CMatrix& objectToWorld,
										const CMatrix& worldToObject );

	static const char* GetFXPropertyTagString( int iPropertyTag )
	{
		switch(iPropertyTag)
		{
		case PROPERTY_ALPHATEST_THRESHOLD:	return "m_alphatestRef" ;
		case PROPERTY_DIFFUSE_COLOUR0:		return "m_diffuseColour0" ;
		case PROPERTY_DIFFUSE_COLOUR1:		return "m_diffuseColour1" ;
		case PROPERTY_DIFFUSE_COLOUR2:		return "m_diffuseColour2" ;
		case PROPERTY_FRESNEL_EFFECT:		return "m_fresnelStrength" ;
		case PROPERTY_REFLECTANCE_COLOUR:	return "m_reflectedColour" ;
		case PROPERTY_SPECULAR_COLOUR:		return "m_specularColour" ;
		case PROPERTY_SPECULAR_POWER:		return "m_specularPower" ;

		case TEXTURE_NORMAL_MAP:			return "m_normalMap" ;
		case TEXTURE_DIFFUSE0:				return "m_diffuse0" ;
		case TEXTURE_DIFFUSE1:				return "m_diffuse1" ;
		case TEXTURE_DIFFUSE2:				return "m_diffuse2" ;

		default:
			ntError_p( 0, ("Unsupported property semantics used") );
			return 0;
		}
	}

	const char* GetName() const { return m_pEffectResource.GetName(); }
	const CHashedString& GetNameHash() const { return m_nameHash; }
	const FXHandle& GetEffect() const { return m_pEffectResource; }

	void RefreshEffect() { m_pEffectResource.ReloadMe(); }
private:
	FXHandle m_pEffectResource;
	CHashedString m_nameHash;
public:
	typedef enum
	{
		F_ISDEBUGSKINNING  = BITFLAG(2),
		F_ISSKIN  = BITFLAG(3),
	} State;
	typedef BitMask<State> Mask;
private:
	Mask m_mask;
public:
	const Mask& GetMask() const {return m_mask;}
};

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
class FXMaterialInstance: public MaterialInstanceBase
{
public:
	//! Creates a material from the given material property table.
	FXMaterialInstance( const FXMaterial* pMaterial, 

						   CMeshVertexElement* pVertexElements, 
						   int iVertexElementCount, 

						   const CMaterialProperty* pProperties, 
						   int iPropertyCount );

	//! Called before rendering vertex data.
	void PreRender( const Transform* pTransform, bool bIsReceivingShadow, void const* ) const;

	//! Called after rendering vertex data.
	void PostRender() const;

	// shadow rendering stuff TODO
	virtual void PreRenderDepth( Transform const* pobTransform, bool bShadowProject, void const* ) const;
	virtual void PostRenderDepth( bool bShadowProject ) const;

	virtual void PreRenderShadowRecieve( Transform const* pobTransform, void const* ) const;
	virtual void PostRenderShadowRecieve() const;

	void BindShaders();
	const FXMaterial* GetEffectMaterial() {return m_pMaterial;}
private:
	void InternalPreRender( const Transform* pTransform, bool bDepth, bool bShadowProject, bool bShadowRecieve, bool bPickupShadows ) const;

	void	UploadGlobalParameters() const;
	void	UploadObjectParameters( const Transform* pTransform, const CMatrix& worldToObject ) const;
	void	UploadMaterialParameters( const CMatrix& worldToObject ) const;

#ifdef PLATFORM_PC
	CVertexDeclaration		m_pDecl;		//!< The declaration.
#endif

	const FXMaterial*	m_pMaterial;	//!< The source material.
};

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
class FXMaterialManager : public Singleton<FXMaterialManager>
{
public:
	FXMaterialManager();
	~FXMaterialManager();
	void DebugUpdate();

	FXMaterial*	FindMaterial( const char* pName );
	FXMaterial*	FindMaterial( const CHashedString& name );

	void ForceRecompile();

	time_t GetExeTime() const { return m_exeModDate; }

	static bool		s_bUseFXSafety;

private:
	FXMaterial*	FindInternal( const char* pName );
	FXMaterial*	FindInternal( const CHashedString& name );

	typedef ntstd::List<FXMaterial*, Mem::MC_GFX> FXMaterialList;
	FXMaterialList m_materials;

	FXMaterial	m_missingLambert;
	FXMaterial	m_missingMetallic;
	FXMaterial	m_missingPhong;
	FXMaterial	m_missingUnknown;

	time_t	m_exeModDate;
};

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------

#ifdef PLATFORM_PC // FIXME_WIL

#define		FX_GET_HANDLE_FROM_NAME( effect, handle, name )					\
			handle = effect->GetParameterByName( NULL, name );				\
			ntAssert_p( handle, ("missing effect parameter %s\n", name ) );

#define		FX_SET_VALUE_VALIDATE( effect, name, data, Bytes )								\
			{																				\
				HRESULT hr = effect->SetValue( name, data, Bytes );							\
				ntAssert_p( !FAILED(hr), ("Failed to set effect parameter ", name, "\n" ) );	\
				UNUSED(hr);																	\
			}

#define		FX_SET_VALUE_RAW( effect, name, data, Bytes )									\
			{																				\
				HRESULT hr = effect->SetFloatArray( name, (float*)data, 4 * Bytes );		\
				ntAssert_p( !FAILED(hr), ("Failed to set effect parameter ", name, "\n" ) );	\
				UNUSED(hr);																	\
			}

#else

#define FX_GET_HANDLE_FROM_NAME(var1,var2,var3)		UNUSED(var1); UNUSED(var2); UNUSED(var3);
#define FX_SET_VALUE_VALIDATE(var1,var2,var3,var4)	UNUSED(var1); UNUSED(var2); UNUSED(var3); UNUSED(var4);
#define FX_SET_VALUE_RAW(var1,var2,var3,var4)		UNUSED(var1); UNUSED(var2); UNUSED(var3); UNUSED(var4);

#endif // PLATFORM_PC

#endif // end _FX_MATERIAL_H
