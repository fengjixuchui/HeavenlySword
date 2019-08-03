//--------------------------------------------------------------------------------------------------
/**
	@file		GcShader.inl

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_SHADER_INL
#define GC_SHADER_INL

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Query the amount of host or video memory required by the GcShader's resource data
				- i.e. the fragment shader code.

	In otherwords, the size of host or video memory allocated if a GcShader is created from the same resource.

	@note		Refer to the other GcShader::QueryResourceSizeInBytes() functions for full details.
**/
//--------------------------------------------------------------------------------------------------

inline int GcShader::QueryResourceSizeInBytes(const FwResourceHandle& hResource)
{
	return QueryResourceSizeInBytes(hResource.GetData());
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the underlying shader resource.
**/
//--------------------------------------------------------------------------------------------------

inline const GcShaderResource*	GcShader::GetResource() const
{
	return m_pHeader;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the buffer type of the fragment shader.

	This function only makes sense for fragment programs as vertex programs are simply
	referenced from the underlying shader resource.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::BufferType GcShader::GetBufferType() const
{
	return m_bufferType;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the memory context of the fragment shader.

	This function only makes sense for fragment programs as vertex programs are simply
	referenced from the underlying shader resource.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::MemoryContext GcShader::GetMemoryContext() const
{
	return m_memoryContext;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the data address for the fragment program microcode.

	This function is only valid to call for fragment shaders.
**/
//--------------------------------------------------------------------------------------------------

inline void* GcShader::GetDataAddress() const
{
	// check valid
	CheckAddressValid();

	// done
	return m_pCodeInstance;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the data address for the fragment program microcode.

	This function is only valid to call for fragment shaders.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetDataOffset() const
{
	// check valid
	CheckAddressValid();

	// done
	return m_codeOffset;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the fragment program control register.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetControl() const
{
	FW_ASSERT( GetType() == Gc::kFragmentProgram );
	return m_control;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Patches the fragment program constant at the given index.

	This function is only valid to call for fragment shaders.
**/
//--------------------------------------------------------------------------------------------------

inline void GcShader::PatchConstant( uint index, const float* pValues, uint rowOffset )
{
	const GcShaderResource::Constant* pConstant = m_pHeader->GetConstants() + index;
	FW_ASSERT( rowOffset < pConstant->m_rowCount );
	PatchConstant( index, pValues, rowOffset, pConstant->m_rowCount - rowOffset );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Patches the fragment program constant at the given index.

	This function is only valid to call for fragment shaders.
**/
//--------------------------------------------------------------------------------------------------

inline void GcShader::PatchConstant( uint index, 
									 float value, 
									 uint rowOffset )
{
	float pValues[] = { value, 0.0f, 0.0f, 0.0f };
	PatchConstant( index, pValues, rowOffset, 1 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Patches the fragment program constant at the given index.

	This function is only valid to call for fragment shaders.
**/
//--------------------------------------------------------------------------------------------------

inline void GcShader::PatchConstant( uint index, 
									 const FwVector4& vec, 
									 uint rowOffset )
{
	// alias the memory to floats
	union { v128 m_v; float m_f[4]; } adapter;
	adapter.m_v = vec.QuadwordValue();

	// call with a float array
	PatchConstant( index, adapter.m_f, rowOffset, 1 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Patches the fragment program constant at the given index.

	This function is only valid to call for fragment shaders.
**/
//--------------------------------------------------------------------------------------------------

inline void GcShader::PatchConstant( uint index, 
									 const FwMatrix44& mat, 
									 uint rowOffset, 
									 uint rowCount )
{
	FW_ASSERT( rowCount <= 4 );

	// alias the memory to floats
	union { v128 m_v[4]; float m_f[16]; } adapter;
	adapter.m_v[0] = mat.GetRow( 0 ).QuadwordValue();
	adapter.m_v[1] = mat.GetRow( 1 ).QuadwordValue();
	adapter.m_v[2] = mat.GetRow( 2 ).QuadwordValue();
	adapter.m_v[3] = mat.GetRow( 3 ).QuadwordValue();

	// call with a float array
	PatchConstant( index, adapter.m_f, rowOffset, rowCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the initial register count for fragment programs.

	This value is the register count initially assigned by the Cg compiler. This function is only 
	valid to call for fragment shaders.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetInitialRegisterCount() const
{
	FW_ASSERT( GetType() == Gc::kFragmentProgram );
	return m_pHeader->GetFragmentProgram()->m_control >> 24;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the register count for fragment programs.

	This function is only valid to call for fragment shaders.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetRegisterCount() const
{
	FW_ASSERT( GetType() == Gc::kFragmentProgram );
	return m_control >> 24; 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the register count for fragment programs.

	Be careful to only set a register equal to or higher than the register count assigned by 
	the Cg compiler!  This function is only valid to call for fragment shaders.
**/
//--------------------------------------------------------------------------------------------------

inline void GcShader::SetRegisterCount( uint registerCount )
{
	FW_ASSERT( GetType() == Gc::kFragmentProgram );
	FW_ASSERT( registerCount >= GetInitialRegisterCount() );
	m_control = ( m_control & 0x00FFFFFF ) | ( registerCount << 24 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the program type.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::ProgramType GcShader::GetType() const
{
	uint type = m_pHeader->m_programType;
	return ( Gc::ProgramType )type;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the number of vertex attributes on this shader.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetAttributeCount() const
{
	return m_pHeader->m_attributeCount;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the number of constants on this shader.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetConstantCount() const
{
	return m_pHeader->m_constantCount;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the number of samplers on this shader.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetSamplerCount() const
{
	return m_pHeader->m_samplerCount;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the hash of the attribute at the given index.
**/
//--------------------------------------------------------------------------------------------------

inline FwHashedString GcShader::GetAttributeHash( uint index ) const
{
	FW_ASSERT( index < m_pHeader->m_attributeCount );
	const GcShaderResource::Attribute* pAttribute = m_pHeader->GetAttributes() + index;
	return FwHashedString(pAttribute->m_hash);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the resource index for the attribute at the given index.

	This resource index is the physical vertex attribute that input data should be mapped to. This 
	can be used in GcKernel::SetStreamElement.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetAttributeResourceIndex( uint index ) const
{
	FW_ASSERT( index < m_pHeader->m_attributeCount );
	const GcShaderResource::Attribute* pAttribute = m_pHeader->GetAttributes() + index;
	return pAttribute->m_resource;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Finds the index of the attribute with the given hash.

	Returns the attribute index if found, or GcShader::kInvalidIndex if not found.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetAttributeIndex( FwHashedString hash ) const
{
	// loop over attributes until we get a match
	const GcShaderResource::Attribute* pAttributes = m_pHeader->GetAttributes();
	for( u16 i = 0; i < m_pHeader->m_attributeCount; ++i )
	{
		if( FwHashedString(pAttributes[i].m_hash) == hash )
			return i;
	}

	// no match
	return kInvalidIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Finds the resource index of the attribute with the given hash.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetAttributeResourceIndex( FwHashedString hash ) const
{
	// loop over attributes until we get a match
	const GcShaderResource::Attribute* pAttributes = m_pHeader->GetAttributes();
	for( u16 i = 0; i < m_pHeader->m_attributeCount; ++i )
	{
		if( FwHashedString(pAttributes[i].m_hash) == hash )
			return pAttributes[i].m_resource;
	}

	// no match
	FW_ASSERT_MSG( false, ( "Attribute with the given hash does not exist on the shader" ) );
	return kInvalidIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the hash for the given constant index.
**/
//--------------------------------------------------------------------------------------------------

inline FwHashedString GcShader::GetConstantHash( uint index ) const
{
	FW_ASSERT( index < m_pHeader->m_constantCount );
	const GcShaderResource::Constant* pConstant = m_pHeader->GetConstants() + index;
	return FwHashedString(pConstant->m_hash);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the constant at the given index is contiguous.

	Contiguous constants map directly to an array of constant registers. Non-contiguous constants
	are allocated haphazardly by the Cg compiler, and some array elements may not be allocated at
	all.
**/
//--------------------------------------------------------------------------------------------------

inline bool	GcShader::IsConstantContiguous( uint index ) const
{
	FW_ASSERT( index < m_pHeader->m_constantCount );
	const GcShaderResource::Constant* pConstant = m_pHeader->GetConstants() + index;
	return pConstant->IsContiguous();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the row resource index of the given constant at the given index.

	This resource index is the hardware constant register used by that row. This index can be used
	with GcKernel::SetVertexProgramConstant to set constants regardless of the active shader.

	Non-contiguous constants may not have all rows allocated. If the row is not allocated then 
	this function returns GcShader::kInvalidIndex.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetConstantResourceIndex( uint index, uint row ) const
{
	FW_ASSERT( index < m_pHeader->m_constantCount );
	const GcShaderResource::Constant* pConstant = m_pHeader->GetConstants() + index;
	if( pConstant->IsContiguous() )
		return pConstant->m_resourceStart + row;
	else
		return m_pHeader->GetResourceArray( pConstant->m_resourceArrayOffset )[row];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Finds the index of the constant with the given hash.

	Returns the constant index if found, or GcShader::kInvalidIndex if not found.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetConstantIndex( FwHashedString hash ) const
{
	// loop over attributes until we get a match
	const GcShaderResource::Constant* pConstants = m_pHeader->GetConstants();
	for( u16 i = 0; i < m_pHeader->m_constantCount; ++i )
	{
		if( FwHashedString(pConstants[i].m_hash) == hash )
			return i;
	}

	// no match
	return kInvalidIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the constant with the given hash is contiguous.

	Contiguous constants map directly to an array of constant registers. Non-contiguous constants
	are allocated haphazardly by the Cg compiler, and some array elements may not be allocated at
	all.
**/
//--------------------------------------------------------------------------------------------------

inline bool	GcShader::IsConstantContiguous( FwHashedString hash ) const
{
	// loop over attributes until we get a match
	const GcShaderResource::Constant* pConstants = m_pHeader->GetConstants();
	for( u16 i = 0; i < m_pHeader->m_constantCount; ++i )
	{
		if( FwHashedString(pConstants[i].m_hash) == hash )
			return pConstants[i].IsContiguous();
	}

	// not found
	FW_ASSERT_MSG( false, ( "Constant with the given hash does not exist on the shader" ) );
	return false;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the row resource index of the constant with the given hash.

	This resource index is the hardware constant register used by that row. This index can be used
	with GcKernel::SetVertexProgramConstant to set constants regardless of the active shader.

	Non-contiguous constants may not have all rows allocated. If the row is not allocated then 
	this function returns GcShader::kInvalidIndex.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetConstantResourceIndex( FwHashedString hash, uint row ) const
{
	// loop over attributes until we get a match
	const GcShaderResource::Constant* pConstants = m_pHeader->GetConstants();
	for( u16 i = 0; i < m_pHeader->m_constantCount; ++i )
	{
		if( FwHashedString(pConstants[i].m_hash) == hash )
		{
			if( pConstants[i].IsContiguous() )
				return pConstants[i].m_resourceStart + row;
			else
				return m_pHeader->GetResourceArray( pConstants[i].m_resourceArrayOffset )[row];
		}
	}

	// not found
	FW_ASSERT_MSG( false, ( "Constant with the given hash does not exist on the shader" ) );
	return kInvalidIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

inline FwHashedString GcShader::GetSamplerHash( uint index ) const
{
	FW_ASSERT( index < m_pHeader->m_samplerCount );
	const GcShaderResource::Sampler* pSampler = m_pHeader->GetSamplers() + index;
	return FwHashedString(pSampler->m_hash);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetSamplerResourceIndex( uint index ) const
{
	FW_ASSERT( index < m_pHeader->m_samplerCount );
	const GcShaderResource::Sampler* pSampler = m_pHeader->GetSamplers() + index;
	return pSampler->m_resource;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Finds the index of the sampler with the given hash.

	Returns the sampler index if found, or GcShader::kInvalidIndex if not found.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetSamplerIndex( FwHashedString hash ) const
{
	// loop over attributes until we get a match
	const GcShaderResource::Sampler* pSamplers = m_pHeader->GetSamplers();
	for( u16 i = 0; i < m_pHeader->m_samplerCount; ++i )
	{
		if( FwHashedString(pSamplers[i].m_hash) == hash )
			return i;
	}

	// no match
	return kInvalidIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Finds the resource of the sampler with the given hash.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcShader::GetSamplerResourceIndex( FwHashedString hash ) const
{
	// loop over attributes until we get a match
	const GcShaderResource::Sampler* pSamplers = m_pHeader->GetSamplers();
	for( u16 i = 0; i < m_pHeader->m_samplerCount; ++i )
	{
		if( FwHashedString(pSamplers[i].m_hash) == hash )
			return pSamplers[i].m_resource;
	}

	// no match
	FW_ASSERT_MSG( false, ( "Sampler with the given hash does not exist on the shader" ) );
	return kInvalidIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		
**/
//--------------------------------------------------------------------------------------------------

#ifndef ATG_DEBUG_MODE
inline void GcShader::CheckAddressValid() const {}
#endif

#endif // ndef GC_SHADER_INL

