//--------------------------------------------------------------------------------------------------
/**
	@file		GcTexture.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_TEXTURE_H
#define GC_TEXTURE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gc/Gc.h>
#include <Gc/GcResource.h>
#include <Fw/FwResource.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class

	@brief			Generic texture object.
**/
//--------------------------------------------------------------------------------------------------

//! Texture object.
class GcTexture : public GcResource
{
public:

	// Query Functions

	static int	QuerySizeInBytes();

	static int	QueryResourceSizeInBytes(	Gc::TexType		type,
											uint			mipCount,
											uint			width,
											uint			height,
											uint			depth,
											Gc::TexFormat	format,
											uint			pitch);
	
	static int	QueryResourceSizeInBytes(const FwResourceHandle& hResource, bool forceLinear = false);

	static int	QueryResourceSizeInBytes(const void* pResource, size_t size, bool forceLinear = false);


	// Create Functions

	static GcTextureHandle Create( Gc::TexType type, 
								   uint mipCount, 
								   uint width, 
								   uint height, 
								   uint depth, 
								   Gc::TexFormat format, 
								   bool isSignedExpand,
								   uint pitch,
								   Gc::BufferType bufferType = Gc::kStaticBuffer,
								   void* pClassMemory = NULL,
								   Gc::MemoryContext memoryContext = Gc::kVideoMemory );

	static GcTextureHandle Create( const GcRenderBufferHandle& hRenderBuffer, 
								   void* pClassMemory = NULL );


	// Create from DDS resource

	static GcTextureHandle Create( const FwResourceHandle& hResource, 
								   void* pClassMemory = NULL, 
								   Gc::MemoryContext memoryContext = Gc::kVideoMemory,
								   bool forceLinear = false );

	static GcTextureHandle Create( const void* pResource, 
								   size_t size, 
								   void* pClassMemory = NULL, 
								   Gc::MemoryContext memoryContext = Gc::kVideoMemory, 
								   bool forceLinear = false );

#if defined(ATG_DEBUG_MODE) || defined(ATG_PC_PLATFORM)
	void				SaveToDDS( const char* pFileName ) const;
#endif


	// Access

	Gc::TexType			GetType() const;
	uint				GetMipLevelCount() const;
	uint				GetWidth() const;
	uint				GetHeight() const;
	uint				GetDepth() const;
	Gc::TexFormat		GetFormat() const;
	uint				GetPitch() const;
	bool				IsSwizzled() const;

	void				SetWrapS( Gc::TexWrapMode wrapMode );
	void				SetWrapT( Gc::TexWrapMode wrapMode );
	void				SetWrapR( Gc::TexWrapMode wrapMode );
	void				SetBorderColour( float red, float green, float blue, float alpha );
	void				SetFilter( Gc::TexFilter magFilter, Gc::TexFilter minFilter );
	void				SetKernelMode( Gc::KernelMode mode );
	void				SetMaxAnisotropy( Gc::AnisotropyLevel maxAniso );
	void				SetLevelRange( float minLevel, float maxLevel );
	void				SetLevelBias( float levelBias );
	void				SetCompareFunc( Gc::CmpFunc compareFunc );
	void				SetSignedExpand( bool isSignedExpand );
	void				SetNormalised( bool isNormalised );
	void				SetFormat( Gc::TexFormat format );
	void				SetGammaCorrect( int flags );
	void				SetDimensions( uint width, uint height );

	Gc::TexWrapMode		GetWrapS() const;
	Gc::TexWrapMode		GetWrapT() const;
	Gc::TexWrapMode		GetWrapR() const;
	Gc::TexFilter		GetMagFilter() const;
	Gc::TexFilter		GetMinFilter() const;
	Gc::AnisotropyLevel	GetMaxAnisotropy() const;
	float				GetLevelBias() const;
	bool				IsSignedExpand() const;
	bool				IsNormalised() const;

	
	// Clone PPU structures but share data

	GcTextureHandle		Clone( void* pClassMemory = NULL );


	// Ice::Render texture access
	
	Ice::Render::Texture&	GetIceTexture();
	
	
	// Sub-texture access
		
	GcTextureHandle		GetCubeFace( Gc::TexCubeFace face, void* pClassMemory = NULL );
	
	GcTextureHandle		GetMipLevel( uint mipLevel, void* pClassMemory = NULL );
	GcTextureHandle		GetCubeMipLevel( Gc::TexCubeFace face, uint mipLevel, void* pClassMemory = NULL );
	GcTextureHandle		GetVolumeMipLevel( uint sliceIndex, uint mipLevel, void* pClassMemory = NULL );

	
	// Texture Data Interface

	bool				QueryGetNewScratchMemory() const;
	void				GetNewScratchMemory();
	void				SetDataAddress( void* pUserAddress );


private:

	// Enumerations
	
	// Endian conversion type depending on texel component size.
	enum EndianConversion { kEndianConvertNone, kEndianConvertAsU16s, kEndianConvertAsU32s };

	// Helper Functions
	
	static uint			CalcMemoryAllocationSize(	Gc::TexType		type,
													uint			mipCount,
													uint			width,
													uint			height,
													uint			depth,
													Gc::TexFormat	format,
													uint			pitch,
													uint*			pBytesPerFace = 0 );

	static void			ExtractDDSHeaderInfo(	const void*			pResource,		// Inputs
												bool				forceLinear,
												
												Gc::TexType*		pType,			// Outputs
												uint*				pMipCount,
												uint*				pWidth,
												uint*				pHeight,
												uint*				pDepth,
												Gc::TexFormat*		pFormat,
												bool*				pIsSwizzed,
												bool*				pIsCompressed,
												EndianConversion*	pEndianConversion,
												uint*				pPitch);
										
	GcTextureHandle		GenericGetMipLevel( uint faceIndex, uint sliceIndex, uint mipLevel, void* pMem );

	void				UpdateIceTextureAddress();

	// Construction & Destruction

	GcTexture( Gc::TexType type, 
			   uint mipCount, 
			   uint width, 
			   uint height, 
			   uint depth, 
			   Gc::TexFormat format, 
			   bool isSignedExpand, 
			   uint pitch, 
			   uint bytesPerFace,
			   Gc::BufferType bufferType, 
			   bool ownsClassMemory );

	// Internal Create function

	static GcTextureHandle Create( Gc::TexType type, 
								   uint mipCount, 
								   uint width, 
								   uint height, 
								   uint depth, 
								   Gc::TexFormat format, 
								   bool isSignedExpand, 
								   uint pitch, 
								   uint bytesPerFace,
								   Gc::BufferType bufferType, 
								   void* pClassMemory );

	// Attributes

	Gc::TexType				m_type;				//!< The texture type.
	Ice::Render::Texture	m_texture;			//!< The ICE texture representation.
	uint					m_bytesPerFace;		//!< The number of bytes between cube faces (or zero if not a cube map).

private:

	// Friends

	friend class GcContext;

	// Static variables

	static const unsigned kTextureControlSignedExpandBit	= 0x00001000;
	static const unsigned kTextureFormatUnnormalisedBit		= 0x00004000;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gc/GcTexture.inl>

//--------------------------------------------------------------------------------------------------

#endif // GC_TEXTURE_H

