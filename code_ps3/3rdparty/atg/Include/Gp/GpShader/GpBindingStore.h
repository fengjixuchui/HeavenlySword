//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Binding store to automate the updating of shader parameters.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef INCL_GPBINDINGSTORE_H
#define INCL_GPBINDINGSTORE_H

#include <Fw/FwStd/FwHashedString.h>
#include <Fw/FwStd/FwStdMap.h>
#include <Fw/FwStd/FwStdSharedPtr.h>
#include <Gc/GcTexture.h>
#include <Gc/GcShader.h>
#include <Fw/FwMaths/FwVector4.h>
#include <Fw/FwMaths/FwMatrix44.h>

//--------------------------------------------------------------------------------------------------
/**
	@class		

	@brief		Stores data and textures by hash for use in any number of shaders.

	This class is a temporary helper class for use in ATG samples. It is not supposed to be an
	example of efficient constant and texture management and should be used for prototyping only,
	and preferably not at all.
**/
//--------------------------------------------------------------------------------------------------

class GpBindingStore
{
public:
	void Clear();

	void SetConstant( FwHashedString hash, const float* pValues, int rows );

	void SetConstant( FwHashedString hash, float value );
	void SetConstant( FwHashedString hash, const FwVector4& value );
	void SetConstant( FwHashedString hash, const FwMatrix44& value );

	void SetTexture( FwHashedString hash, const GcTextureHandle& hTexture );

	void UpdateShader( GcShaderHandle& hShader ) const;

private:
	class Constant
	{
	public:
		Constant();
		Constant( const float* pValues, int rows );

		const float* GetValues() const;

	private:
		FwStd::SharedArrayPtr< float > m_pValues;
	};

	FwStd::Map< FwHashedString, Constant > m_constants;
	FwStd::Map< FwHashedString, GcTextureHandle > m_textures; 
};

inline GpBindingStore::Constant::Constant()
{
}

inline GpBindingStore::Constant::Constant( const float* pValues, int rows )
  :	m_pValues( FW_NEW float[ rows*4 ] )
{
	FwMemcpy( m_pValues.Get(), pValues, rows*4*sizeof( float ) );
}

inline const float* GpBindingStore::Constant::GetValues() const
{
	return m_pValues.Get();
}

#endif // ndef INCL_GPBINDINGSTORE_H

