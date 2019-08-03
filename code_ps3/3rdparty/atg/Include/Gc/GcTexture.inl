//--------------------------------------------------------------------------------------------------
/**
	@file		GcTexture.inl

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_TEXTURE_INL
#define GC_TEXTURE_INL

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief		Query the amount of host or video memory required by the GcTexture's resource data
				- i.e. the texture image data.

	In otherwords, the size of host or video memory allocated if a GcTexture is created from the same resource.

	@note		Refer to the other GcTexture::QueryResourceSizeInBytes() functions for full details.
**/
//--------------------------------------------------------------------------------------------------

inline int GcTexture::QueryResourceSizeInBytes(const FwResourceHandle& hResource, bool forceLinear)
{
	return QueryResourceSizeInBytes(hResource.GetData(), hResource.GetSize(), forceLinear);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture type: 2D, 3D or Cube.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::TexType GcTexture::GetType() const
{
	return m_type;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture mip level count.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcTexture::GetMipLevelCount() const
{
	return ( m_texture.m_format >> 16 ) & 0xf;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture width.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcTexture::GetWidth() const
{
	return m_texture.GetWidth();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture height.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcTexture::GetHeight() const
{
	return m_texture.GetHeight();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture depth.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcTexture::GetDepth() const
{
	return ( m_texture.m_size2 >> 20 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture format.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::TexFormat GcTexture::GetFormat() const
{
	return ( Gc::TexFormat )m_texture.GetFormat();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture pitch.
**/
//--------------------------------------------------------------------------------------------------

inline uint GcTexture::GetPitch() const
{
	return m_texture.GetLinearImagePitch();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the texture is swizzled.
**/
//--------------------------------------------------------------------------------------------------

inline bool GcTexture::IsSwizzled() const
{
	return ( m_texture.GetLinearImagePitch() == 0 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the u direction texture wrap mode.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetWrapS( Gc::TexWrapMode wrapMode )
{
	m_texture.SetWrapMode( Ice::Render::kTexcoordS, ( Ice::Render::WrapMode )wrapMode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the v direction texture wrap mode.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetWrapT( Gc::TexWrapMode wrapMode )
{
	m_texture.SetWrapMode( Ice::Render::kTexcoordT, ( Ice::Render::WrapMode )wrapMode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the w direction texture wrap mode.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetWrapR( Gc::TexWrapMode wrapMode )
{
	m_texture.SetWrapMode( Ice::Render::kTexcoordR, ( Ice::Render::WrapMode )wrapMode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the texture border colour.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetBorderColour( float red, float green, float blue, float alpha )
{
	int bgra =    ( min( 255, max( 0, int(  blue*255.0f ) ) ) << 24 )
				| ( min( 255, max( 0, int( green*255.0f ) ) ) << 16 )
				| ( min( 255, max( 0, int(   red*255.0f ) ) ) << 8 )
				| ( min( 255, max( 0, int( alpha*255.0f ) ) ) << 0 );

	m_texture.SetBorderColor( bgra );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the magnification and minification filter.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetFilter( Gc::TexFilter magFilter, Gc::TexFilter minFilter )
{
	FW_ASSERT( magFilter == Gc::kFilterConvolutionKernelMag || magFilter == Gc::kFilterLinear || magFilter == Gc::kFilterNearest );
	m_texture.SetFilterMode( ( Ice::Render::FilterMode )magFilter, ( Ice::Render::FilterMode )minFilter );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the convolution kernel mode.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetKernelMode( Gc::KernelMode mode )
{
	m_texture.SetKernelMode( ( Ice::Render::KernelMode )mode );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the maximum anisotropy level of the texture reads.

	An anisotropy level of 1 effectively disables anisotropic filtering.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetMaxAnisotropy( Gc::AnisotropyLevel maxAniso )
{
	m_texture.SetMaxAnisotropy( ( Ice::Render::AnisotropyLevel )maxAniso );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the level range to use when using mip maps.

	This must be a subset of the range [0, 15].
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetLevelRange( float minLevel, float maxLevel )
{
	int minFixed = int( minLevel*256.0f );
	int maxFixed = int( maxLevel*256.0f );

	m_texture.SetLevelRange( minFixed, maxFixed );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the level bias when choosing mip maps.

	For the best performance/quality tradeoff, this should be set to 0.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetLevelBias( float levelBias )
{
	int biasFixed = int( levelBias*256.0f );

	m_texture.SetLevelBias( biasFixed );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the texture compare function for shadow mapping.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetCompareFunc( Gc::CmpFunc compareFunc )
{
	// reverse the sense of less and greater
	Gc::CmpFunc depthFunc;
	switch( compareFunc )
	{
	case Gc::kLess:		depthFunc = Gc::kGreater; break;
	case Gc::kLEqual:	depthFunc = Gc::kGEqual; break;
	case Gc::kGreater:	depthFunc = Gc::kLess; break;
	case Gc::kGEqual:	depthFunc = Gc::kLEqual; break;
	default:			depthFunc = compareFunc; break;
	}

	// set on the texture
	m_texture.SetDepthFunc( ( Ice::Render::ComparisonFunc )depthFunc );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets whether or not the result is expanded from [0,1] to [-1,1] range.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetSignedExpand( bool isSignedExpand )
{
	if( isSignedExpand )
		m_texture.EnableSignedExpand();
	else
		m_texture.DisableSignedExpand();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets whether or not texture reads are done with normalised coordinates.

	This defaults to true for all texture types. Support for unnormalised reads is not completely
	known, they may only work for 2D textures and a subset of texture formats.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetNormalised( bool isNormalised )
{
	if( isNormalised )
	{
		// clear the flag
		m_texture.m_format &= ~kTextureFormatUnnormalisedBit;
	}
	else
	{
		// check our state
		FW_ASSERT_MSG( m_type == Gc::kTexture2D, ( "Non-normalised texture reads can only be enabled on 2D textures." ) );
		FW_ASSERT_MSG( GetMipLevelCount() == 1, ( "Non-normalised texture reads can only be enabled on single-level textures." ) );

		// enable the flag
		m_texture.m_format |= kTextureFormatUnnormalisedBit;
	}
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the gamma-correction flags for this texture.

	This can only be used on 8 bits per component textures. Use combinations of the flags from
	Gc::GammaCorrectionFlags.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetGammaCorrect( int flags )
{
	m_texture.SetGammaCorrect( flags );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the dimensions of this texture.

	No attempt is made to validate this dimensions against the underlying storage.  Use this
	function at your own risk!
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetDimensions( uint width, uint height )
{
	FW_ASSERT( width <= 4096 && height <= 4096 );
	m_texture.m_size1 = ( width << 16 ) | height;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the surface format of the texture
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetFormat( Gc::TexFormat format )
{
	m_texture.m_swizzle = (m_texture.m_swizzle & ~0x1ffff) | (format & 0x1ffff);
	m_texture.m_format = (m_texture.m_format & ~0x1f00) | ((format >> 16) & 0x1f00);
	m_texture.m_filter = (m_texture.m_filter & ~0xf0000000) | ((format << 8) & 0xf0000000);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture u direction wrap mode.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GcTexture::GetWrapS() const
{
	return ( Gc::TexWrapMode )( m_texture.m_control1 & 0xf );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture v direction wrap mode.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GcTexture::GetWrapT() const
{
	return ( Gc::TexWrapMode )( ( m_texture.m_control1 >> 8 ) & 0xf );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture w direction wrap mode.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GcTexture::GetWrapR() const
{
	return ( Gc::TexWrapMode )( ( m_texture.m_control1 >> 16 ) & 0xf );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture magnification filter.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GcTexture::GetMagFilter() const
{
	return ( Gc::TexFilter )( ( m_texture.m_filter >> 24 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture minification filter.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GcTexture::GetMinFilter() const
{
	return ( Gc::TexFilter )( ( m_texture.m_filter >> 16 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the texture minification filter.
**/
//--------------------------------------------------------------------------------------------------

inline Gc::AnisotropyLevel GcTexture::GetMaxAnisotropy() const
{
	return ( Gc::AnisotropyLevel )( ( m_texture.m_control2 >> 4 ) & 0xf );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets the mip level bias.
**/
//--------------------------------------------------------------------------------------------------

inline float GcTexture::GetLevelBias() const
{
	struct { int x:13; } signExtend;
	
    int	biasFixed = m_texture.m_filter & 0x1fff;
	biasFixed = signExtend.x = biasFixed;

	return biasFixed / 256.0f;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if signed expansion is enabled.
**/
//--------------------------------------------------------------------------------------------------

inline bool GcTexture::IsSignedExpand() const
{
	return ( m_texture.m_control1 & kTextureControlSignedExpandBit ) != 0;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns true if the texture reads are done with normalised coordinates.
**/
//--------------------------------------------------------------------------------------------------

inline bool GcTexture::IsNormalised() const
{
	return ( m_texture.m_format & kTextureFormatUnnormalisedBit ) == 0;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Returns the Ice::Render representation of the texture.
**/
//--------------------------------------------------------------------------------------------------
	
inline Ice::Render::Texture& GcTexture::GetIceTexture()
{
	return m_texture;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Creates a single-level texture from the given mip level of a 2D texture.

	The results of this operation are undefined if the texture is not a 2D texture. Once a surface
	has been extracted, it can be used to create a GcRenderBuffer for rendering into.
**/
//--------------------------------------------------------------------------------------------------

inline GcTextureHandle GcTexture::GetMipLevel( uint mipLevel, void* pClassMemory )
{
	FW_ASSERT( m_type == Gc::kTexture2D );
	return GenericGetMipLevel( 0, 0, mipLevel, pClassMemory );	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Creates a single-level texture from the given cube face and mip level.

	The results of this operation are undefined if the texture is not a cube texture. Once a surface
	has been extracted, it can be used to create a GcRenderBuffer for rendering into.
**/
//--------------------------------------------------------------------------------------------------

inline GcTextureHandle GcTexture::GetCubeMipLevel( Gc::TexCubeFace face, uint mipLevel, void* pClassMemory )
{
	FW_ASSERT( m_type == Gc::kTextureCube );
	return GenericGetMipLevel( face, 0, mipLevel, pClassMemory ); 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Creates a single-level texture from the given volume slice index and mip level.

	The results of this operation are undefined if the texture is not a volume texture. Once a 
	surface	has been extracted, it can be used to create a GcRenderBuffer for rendering into.
**/
//--------------------------------------------------------------------------------------------------

inline GcTextureHandle GcTexture::GetVolumeMipLevel( uint sliceIndex, uint mipLevel, void* pClassMemory )
{
	FW_ASSERT( m_type == Gc::kTexture3D );
	return GenericGetMipLevel( 0, sliceIndex, mipLevel, pClassMemory ); 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Gets new scratch memory for this texture object.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::GetNewScratchMemory() 
{
	// allocate the scratch memory
	GcResource::AllocateScratchMemory( GetSize(), Gc::kTextureAlignment );

	// update the ice texture
	UpdateIceTextureAddress();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets a new custom texture data address.
**/
//--------------------------------------------------------------------------------------------------

inline void GcTexture::SetDataAddress( void* pUserAddress )
{
	// update the buffer
	GcResource::SetUserAddress( pUserAddress );

	// update the ice texture
	UpdateIceTextureAddress();
}

#endif // ndef GC_TEXTURE_INL

