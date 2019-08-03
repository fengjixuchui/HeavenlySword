//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		Animation Clip Support

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FP_ANIM_CLIP_INL
#define	FP_ANIM_CLIP_INL

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTIONS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the name (as a hashed string ) of this channel

	@param			animClip	reference to the animClip, since hashed strings are now stored as
								indices, and the table is in the FpAnimClipDef object
**/
//--------------------------------------------------------------------------------------------------

inline	FwHashedString	FpAnimChannelInfo::GetChannelName( const FpAnimClipDef& animClip ) const
{ 
	return animClip.GetHashedStringArray()[ m_channelNameIndex ]; 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Index in the global list of FpAnimTimeblockChannelHeader objects

	@note			Since the structure is not aligned on half boundaries we must rebuild the word
					by hand
**/
//--------------------------------------------------------------------------------------------------

inline	int				FpAnimChannelInfo::GetGlobalChannelIndex( void ) const
{ 
#if	FW_BIG_ENDIAN 
	return ( ( int )m_globalChannelIndexByte0 << 8 ) | ( ( int )m_globalChannelIndexByte1 ); 
#elif FW_LITTLE_ENDIAN
	return ( ( int )m_globalChannelIndexByte1 << 8 ) | ( ( int )m_globalChannelIndexByte0 ); 
#else
	#error	endianness not defined! (check Fw.h)
#endif 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return a pointer to the channel headers.
	
					Channel headers are immediatly following the timeblock header
**/
//--------------------------------------------------------------------------------------------------

inline const FpAnimTimeblockChannelHeader*	FpAnimTimeblock::GetChannelHeaders( void ) const					
{ 
	uintptr_t hdrEnd = FwAlign( sizeof( *this ) + ( uintptr_t ) this, 4U ); 	

	return ( const FpAnimTimeblockChannelHeader* ) hdrEnd; 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return a pointer to a given channel header.
	
	@param			idxChannelInTb	Note that this not the "global" channel index - 0 = 1st animated
									channel.					
**/
//--------------------------------------------------------------------------------------------------

inline const FpAnimTimeblockChannelHeader*	FpAnimTimeblock::GetChannelHeader( int idxChannelInTb ) const					
{ 
	FW_ASSERT( idxChannelInTb >=0 && idxChannelInTb < m_animatedChannelCount );

	uintptr_t hdrEnd = FwAlign( sizeof( *this ) + ( uintptr_t ) this, 4U ); 	

	return ( ( const FpAnimTimeblockChannelHeader* ) hdrEnd ) + idxChannelInTb; 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return a pointer to the time array in the time block

					Array is always aligned to 16 bytes for SPU search code.
**/
//--------------------------------------------------------------------------------------------------

inline const void*	FpAnimTimeblock::GetTimeArray( void ) const
{
	const FpAnimTimeblockChannelHeader* pHeaderEnd = GetChannelHeaders() + m_animatedChannelCount; 
	
	uintptr_t hdrEnd = FwAlign( ( uintptr_t ) pHeaderEnd, 16U ); 

	return ( const void* ) hdrEnd; 
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return a pointer to the time array in the time block

					Array is always aligned to 16 bytes for SPU search code.
**/
//--------------------------------------------------------------------------------------------------

inline const float*	FpAnimTimeblock::GetTimeArrayFloat( void ) const
{
	FW_ASSERT( GetType() == kFloatTimeblock );
	
	return ( const float* ) GetTimeArray();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return a pointer to the time array in the time block

					Array is always aligned to 16 bytes for SPU search code.
**/
//--------------------------------------------------------------------------------------------------

inline const u8*	FpAnimTimeblock::GetTimeArrayByte( void ) const
{
	FW_ASSERT( GetType() == kByteTimeblock );
	
	return ( const u8* ) GetTimeArray();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return a pointer to the time array in the time block

					Array is always aligned to 16 bytes for SPU search code.
**/
//--------------------------------------------------------------------------------------------------

inline const u16*	FpAnimTimeblock::GetTimeArrayShort( void ) const
{
	FW_ASSERT( GetType() == kShortTimeblock );
	
	return ( const u16* ) GetTimeArray();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a pointer to the FpAnimClipDef object that this clip refers to.

	@result			A read-only pointer to the underlying FpAnimClipDef object.
**/
//--------------------------------------------------------------------------------------------------

inline const FpAnimClipDef*	FpAnimClip::GetAnimClipDef( void ) const
{	
	return m_pAnimClipDef;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the duration of the animation that this clip refers to.

	@result			Duration of the clip, in seconds.
**/
//--------------------------------------------------------------------------------------------------

inline float FpAnimClip::GetDuration( void ) const
{
	return m_pAnimClipDef->GetDuration();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns the channel index for the specified item/channel name pair.

	@result			Channel index, or -1 if not found.
**/
//--------------------------------------------------------------------------------------------------

inline int	FpAnimClip::GetChannelIndex( FwHashedString itemName, FwHashedString channelName ) const
{
	return m_pAnimClipDef->GetChannelIndex( itemName, channelName );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve a pointer to the result of clip evaluation using item and channel names

	@param			channelIndex	A channel index obtained from GetChannelIndex().
	@param			pResult			Read-only pointer to the evaluated FwVector4.
**/
//--------------------------------------------------------------------------------------------------

inline	const FwVector4*	FpAnimClip::GetChannelData( int channelIndex ) const
{
	FW_ASSERT( ( channelIndex >= 0 ) && ( channelIndex < m_pAnimClipDef->GetChannelCount() ) );
	return &m_pResultArray[ channelIndex ];
}

//--------------------------------------------------------------------------------------------------

#endif	// FP_ANIM_CLIP_INL
