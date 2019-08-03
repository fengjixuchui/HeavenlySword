//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		Animation Clip Support

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FP_ANIM_CLIP_H
#define	FP_ANIM_CLIP_H

#include <Fw/FwMaths/FwVector4.h>
#include <Fw/FwMaths/FwQuat.h>
#include <Fw/FwMaths/FwVector.h>
#include <Fw/FwMaths/FwPoint.h>
#include <Fw/FwStd/FwHashedString.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			FpAnimKeyframeHermite
	
	@brief			Keyframe for cubic Hermite interpolation - C1 continuity.

	@note			Because this format assumes C1 continuity, the entire keyframe for evaluation
					comes from a combination of this structure and the data in the next keyframe.

					Discontinuities are represented by inserting null (duration=0) keyframes.
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimKeyframeHermite
{
public:
	FwVector4	m_value;					///< Value at start of the segment
	FwVector4	m_tangent;					///< Tangent (direction & speed) at start of the segment
};

//--------------------------------------------------------------------------------------------------
/**
	@class			FpAnimKeyframeHalfHermite
	
	@brief			Keyframe for half-precision cubic Hermite interpolation - C1 continuity.

	@note			Because this format assumes C1 continuity, the entire keyframe for evaluation
					comes from a combination of this structure and the data in the next keyframe.

					Discontinuities are represented by inserting null (duration=0) keyframes.
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimKeyframeHalfHermite
{
public:
	u128		m_valueTangent;				///< Value and tangent at start of the segment, packed
											///< into one quadword (V/V/V/V/T/T/T/T) containing half
											///< precision floating point data.
};

//--------------------------------------------------------------------------------------------------
/**
	@brief			Channel transform (principal component)
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimKeyframeTransform
{
public:
	inline		FpAnimKeyframeTransform( void ) {};
	FwVector4	TransformVal( FwVector4_arg vec ) const;
	FwVector4	TransformTan( FwVector4_arg vec ) const;

private:
	s16			m_mat[ 4 ][ 4 ];	// Orthonormal matrix
	FwVector4	m_trans;			// Fp32 for the moment. todo : improve
};

//--------------------------------------------------------------------------------------------------
/**
	@class		FpAnimTimeblockChannelHeader 

	@brief		Header used internally in a FpAnimTimeblock to describe per-channel data

	@see		FpAnimTimeblock	

	@warning	GpAnimatorEvalGather*.nga are hardcoded for this structure

**/
//--------------------------------------------------------------------------------------------------

class FpAnimTimeblockChannelHeader 
{
public:
	s32	m_bitIdxFirstKeyframe;			///< Bit index of 1st keyframe for this channel
	s16	m_numKeyframes;					///< Number of keyframes for this timeblock
	s16	m_idxFirstTimeInArray;			///< Index of 1st time value in the float array, followed by (m_numKeyframes - 1) consecutive values
};

//--------------------------------------------------------------------------------------------------
/**
	@class		FpAnimTimeblock

	@brief		Timeblock - Contains time information to retrieve keyframes for *all* channel for a 
				given time window (curTimeblock.minTime <= t < nextTimeblock.minTime).
				
				There is a limited redundancy (only a keyframe index, not the keyframe itself) 
				to be able to retrieve keyframes for channels that are not updated at minTime.

				Contains :

				- An array of channelCount elements 

					class FpAnimTimeblockChannelHeader 
					{
						s32	m_idxFirstGlobalKeyframe;	///< Bitream index of 1st keyframe for this channel in global keyframe array for this type
						s16	m_numKeyframes;				///< Number of keyframes for this timeblock
						s16	m_idxInFloatArray;			///< Index of the first time value in the float array, followed by (m_numKeyframes - 1) consecutive values
					};

					Note that this channel header can be compressed more efficiently.

				- Starting on the next qword boundary immediatly after this (= statically 
					determined by number of channels in the animation), the array of float
					time values. All "idxInFloatArray" refer to this array.
			
			Example:

				Imagine we have the following situations : 
				- timeblock stating at t = 1.0f (because of the size limit of previous keyframe) 
				- two channels having keyframes at these 
					- channel 0: ... 0.95 1.05 1.20
					- channel 1: ... 0.95 1.0 1.25

				What the tool should generate is :

				{
					{ x, 3, 0 }			// 3 keyframe indices because 0.95 will be duplicated, 
										// since there's missing data between 1.0 and 1.05
					{ y, 2, 3 }			// 2 keyframe indices, no redundancy
				}
				{
					0.95, 1.05, 1.20,	// channel 0
					1.0, 1.25			// channel 1
				}

	@note	This object should be aligned to 16 bytes

	@note	Opaque data type - read doc above and tools code
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimTimeblock
{
public:
	enum
	{
		kMaxTimeblockSize = 4096		// arbitrary.. needs more test / balancing 
	};
	enum Type
	{
			kFloatTimeblock = 0,			///< full precision float time values
			kByteTimeblock,					///< quantised u8 time values
			kShortTimeblock,				///< quantised u16 time values 
	};

	// Simple accessors
	Type								GetType( void ) const							{ return ( Type ) m_type; }
	int									GetChannelCount( void ) const					{ return ( int ) m_animatedChannelCount; }
	const FpAnimTimeblockChannelHeader*	GetChannelHeaders( void ) const;					
	const FpAnimTimeblockChannelHeader*	GetChannelHeader( int idxChannelInTb ) const;					
	const float*						GetTimeArrayFloat( void ) const;
	const u8*							GetTimeArrayByte( void ) const;
	const u16*							GetTimeArrayShort( void ) const;

	float								GetTimeOffset( void ) const						{ return m_timeOffset; }
	float								GetTimeStep( void ) const						{ return m_timeStep; }

	// Operation (PPU evaluators)
	int									FindKeyframe( float& timeStart, float& timeEnd, const FpAnimTimeblockChannelHeader* FW_RESTRICT pTimeblockChannelHeader, const float timeValue ) const;

private:
	// Attributes
	u16		m_type;						///< Timeblock type
	s16		m_animatedChannelCount;		///< Animated (=non constant channels) stored in this timeblock
	float	m_timeOffset;				///< Time offset (0 in float mode) - note that it's different from start time in timeblock index (which is excluding repeated channels)
	float	m_timeStep;					///< Time step (0 in float mode)

	// Operations	
	const void*							GetTimeArray( void ) const;
};

//--------------------------------------------------------------------------------------------------
/**
	@class			FpAnimTimeblockIndex
	
	@brief			This is the entry that describe a timeblock; it's stored directly in the 
					FpAnimClipHeader header.

					Note that:
					- There is enough reundancy (indices, not actual data) to be able
					  to retrieve keyframes for all channels at a given time within one timeblock.
				    - There is no m_timeMax, as it is the m_timeMin of the next timeblock.

	@note			This structure is far too big, mostly because of biff writer limitations, it 
					can/should be optimised.
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimTimeblockIndex					
{
public:
	float						m_timeMin;		// Min time in this time block, we guarantee that 
												// there is enough redundancy for that minTime value
	FwOffset<FpAnimTimeblock>	m_blockStart;	// Start of this timeblock - could be made much smaller
	FwOffset<FpAnimTimeblock>	m_blockEnd;		// End label, to know the size of timeblock - could 
												// be made much smaller but currently limited by Biff writer
};

//--------------------------------------------------------------------------------------------------
/**
	@class			FpAnimChannelInfo
	
	@brief			This contains information relating to the channel that is not required by the 
					core update code. It should be used to compute channel indices.

	@note			This class can be unaligned in the array
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimChannelInfo
{
public:
	enum	KeyframeType
	{
		kInvalid = -1,				// Invalid.. used in tools processing.
		kConstantDirect,			// No keyframes.. channel takes value from constant channel array in FpAnimClipDef 
		kConstantIndirect,			// No keyframes.. channel takes value from constant channel array in FpAnimClipDef (indirected)
		kHermite,					// 2 x FwVector4's (startValue, startTangent) assuming C1 continuity (so we read two keys in to evaluate)
		kHalfHermite,				// Same as kHermite, but data is in half-precision floating point.
		kPackedHermite,				// Bitpacked hermite (needs an additioanl packing spec)
		kNumKeyframeTypes,
	};

	FwHashedString			GetChannelName( const class FpAnimClipDef& animClip ) const;
	int						GetGlobalChannelIndex( void ) const;

private:
	u8						m_channelNameIndex;			///< Index of the hashed string in parent object (to reduce size, heavily redundant)
	u8						m_globalChannelIndexByte0;	///< Index in the global list of FpAnimTimeblockChannelHeader objects (1st byte - to allow unaligned loads)
	u8						m_globalChannelIndexByte1;	///< Index in the global list of FpAnimTimeblockChannelHeader objects (2nd byte - to allow unaligned loads)
};

//--------------------------------------------------------------------------------------------------
/**
	@class			FpAnimKeyframePackingSpec
	
	@brief			This contains information needed to be able to unpack a channel of 
					FpAnimChannelInfo::kPackedHermite type.

					A packing spec is actually itself bitpacked. It's built of two similar arrays 
					of bits representing encoding of values and tangents for this channel.

					For each of them, the bitstream contains :
					- kNumBitsSignSpec bits		: numSignBits X
					- kNumBitsExponentSpec bits : numExponentBits X
					- kNumBitsMantissaSpec bits : numMantissaBits X
					- kNumBitsSignSpec bits		: numSignBits Y
					- kNumBitsExponentSpec bits	: numExponentBits Y
					- kNumBitsMantissaSpec bits	: numMantissaBits Y
					- kNumBitsSignSpec bits		: numSignBits Z
					- kNumBitsExponentSpec bits	: numExponentBits Z
					- kNumBitsMantissaSpec bits	: numMantissaBits Z
					- kNumBitsSignSpec bits		: numSignBits W
					- kNumBitsExponentSpec bits	: numExponentBits W
					- kNumBitsMantissaSpec bits	: numMantissaBits W					
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimKeyframePackingSpec
{
public:
	enum
	{
		kNumBitsSignSpec     = 1,		// 0..1 bits for sign - if fixed point means signed 
		kNumBitsExponentSpec = 4,		// 0..8 bits for biased exponent  - if 0 -> fixed point
		kNumBitsMantissaSpec = 5,		// 0..23 bits for mantissa

		kNumBitsSpec = kNumBitsSignSpec + kNumBitsExponentSpec + kNumBitsMantissaSpec // 10 bits per component
	};

	enum
	{
		kMaskRebuild   = 0x1F,
		kFlagRebuildX  = 0x18,	// rebuild x from smallest 3 (y,z,w) components of a unit quat
		kFlagRebuildY  = 0x14, // rebuild y from smallest 3 (x,z,w) components of a unit quat
		kFlagRebuildZ  = 0x12, // rebuild z from smallest 3 (x,y,w) components of a unit quat
		kFlagRebuildW  = 0x11, // rebuild w from smallest 3 (z,y,z) components of a unit quat

	};

	inline	u32		GetNumBits( void ) const		{ return ( m_kfNumBitsAndFlags & 0x01FF ); }
	inline	u32		GetFlags( void ) const			{ return ( ( m_kfNumBitsAndFlags & 0x7E00 ) >> 9 ); }

	void			DecodeKeyframe( FwVector4& valueKf, FwVector4& tanKf, const u8* pBitStreamBase, const u32 bitOffset ) const;

private:
	u16	m_kfNumBitsAndFlags;			// number of bits per keyframe (0..256) 
	u8  m_specValue[5];					// individual coding spec per component
	u8  m_specTangent[5];				// individual coding spec per component

	friend class AnimKeyframePackingSpec;
};

//--------------------------------------------------------------------------------------------------
/**
	@class			FpAnimItem
	
	@brief			
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimItem
{
public:
	FwHashedString					GetItemName( void ) const			{ return m_itemName; }
	s32								GetChannelCount( void ) const		{ return m_channelCount; }
	const	FpAnimChannelInfo*		GetChannelInfoArray( void ) const	{ return m_channelInfoArray.Get(); }
	
private:
	FwHashedString						m_itemName;
	s32									m_channelCount;
	FwOffset<const FpAnimChannelInfo>	m_channelInfoArray;
};


//--------------------------------------------------------------------------------------------------
/**
	@class			FpAnimClipDef
	
	@brief			
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimClipDef
{
public:
	enum	AnimClipDefFlags
	{
		kIsLocomoting	=	( 1 << 0 ),
	};

	class	LocomotionInfo
	{
	public:
		FwQuat		GetStartRotation( void ) const			{ return m_startRotation; }
		FwPoint		GetStartTranslation( void ) const		{ return m_startTranslation; }
		FwQuat		GetEndDeltaRotation( void ) const		{ return m_endDeltaRotation; }
		FwVector	GetEndDeltaTranslation( void ) const	{ return m_endDeltaTranslation; }

	private:
		FwQuat		m_startRotation;
		FwPoint		m_startTranslation;
		FwQuat		m_endDeltaRotation;
		FwVector	m_endDeltaTranslation;	
	};

	u32									GetTag( void ) const								{ return m_tag; }
	s32									GetFlags( void ) const								{ return m_flags; }
	float								GetDuration( void ) const							{ return m_duration; }
	s16									GetItemCount( void ) const							{ return m_itemCount; }
	s16									GetTimeblockCount( void ) const						{ return m_timeblockCount; }
	s16									GetChannelCount( void ) const						{ return m_channelCount; }
	s16									GetConstantChannelCount( void ) const				{ return GetChannelTypeCount( FpAnimChannelInfo::kConstantIndirect ) + GetChannelTypeCount( FpAnimChannelInfo::kConstantDirect ); }
	u32									GetSizeSpuDmaIn( void ) const						{ return ( u32 ) m_endSpuDmaIn.Get() - ( u32 ) this; }

	const	FwVector4*					GetConstantChannelArray( void ) const				{ return m_constantChannelArray.Get(); }
	const	u16*						GetConstantChannelIndexArray( void ) const			{ return m_constantChannelIndexArray.Get(); }
	const	FpAnimTimeblockIndex*		GetTimeblockIndexArray( void ) const				{ return m_timeblockIndexArray.Get(); }	

	const	LocomotionInfo*				GetLocomotionInfo( void ) const						{ return m_locomotionInfo.Get(); }
	const	FpAnimChannelInfo*			GetChannelInfoArray( void ) const					{ return m_channelInfoArray.Get(); }
	const	FpAnimItem*					GetItemArray( void ) const							{ return m_itemArray.Get(); }
	const	FpAnimKeyframeHermite*		GetKeyframeHermiteArray( void ) const				{ return m_keyframeHermiteArray.Get(); }
	const	FpAnimKeyframeHalfHermite*	GetKeyframeHalfHermiteArray( void ) const			{ return m_keyframeHalfHermiteArray.Get(); }
	const	u8*							GetKeyframePackedHermiteBitstream( void ) const		{ return m_keyframePackedHermiteBitstream.Get(); }
	const	FpAnimKeyframePackingSpec*	GetPackedHermitePackingSpecArray( void ) const		{ return m_packedHermitePackingSpecArray.Get(); }
	const	u16*						GetPackedHermiteSpecIndexArray( void ) const		{ return m_packedHermiteSpecIndexArray.Get(); }
	const	FpAnimKeyframeTransform*	GetPackedHermiteTransformArray( void ) const		{ return m_packedHermiteTransformArray.Get(); }
	const	u16*						GetPackedHermiteTransformIndexArray( void ) const	{ return m_packedHermiteTransformIndexArray.Get(); }
	const	FwHashedString*				GetHashedStringArray( void ) const					{ return m_hashedStringArray.Get(); }

	u16									GetChannelTypeCount( FpAnimChannelInfo::KeyframeType keyframeType ) const	{ return m_channelTypeCount[ keyframeType ]; }
	u16									GetChannelTypeStart( FpAnimChannelInfo::KeyframeType keyframeType ) const	{ return m_channelTypeStart[ keyframeType ]; }

	int									GetItemIndex( FwHashedString itemName ) const;
	int									GetItemChannelIndex( int itemIndex, FwHashedString channelName ) const;
	int									GetChannelIndex( FwHashedString itemName, FwHashedString channelName ) const;

	static u32							GetClassTag()								{ return FW_MAKE_TAG( 'A','C','1','0' ); }

	FwVector4							Evaluate( int channelIndex, float timeValue ) const;


	// Shared evaluation code - internal use only (PPU fallbacks)
	static FwVector4					EvaluateHermiteKeyframe(		const FpAnimTimeblock* FW_RESTRICT pTimeblock,
																		const FpAnimTimeblockChannelHeader* FW_RESTRICT pTimeblockChannelHeader,
																		const FpAnimKeyframeHermite* FW_RESTRICT pPPUKeyframeArray,
																		const float timeValue );
	static FwVector4					EvaluateHalfHermiteKeyframe(	const FpAnimTimeblock* FW_RESTRICT pTimeblock,
																		const FpAnimTimeblockChannelHeader* FW_RESTRICT pTimeblockChannelHeader,
																		const FpAnimKeyframeHalfHermite* FW_RESTRICT pPPUKeyframeArray,
																		const float timeValue );
	static FwVector4					EvaluatePackedHermiteKeyframe(	const FpAnimTimeblock* FW_RESTRICT pTimeblock,
																		const FpAnimTimeblockChannelHeader* FW_RESTRICT pTimeblockChannelHeader,
																		const u8* FW_RESTRICT pPPUKeyframeBitstream,
																		const FpAnimKeyframePackingSpec* FW_RESTRICT pPackingSpec,
																		const FpAnimKeyframeTransform* FW_RESTRICT pTransform,
																		const float timeValue );

	// Current limits - lots of room left for upcoming IK
	static const int	kMaxItems		=	128; // Important : must be multiple of 16 (for SPU code) : was 192
	static const int	kMaxChannels	=	592; // Important : must be multiple of 16 (for SPU code) : was 896

private:	
	u32											m_tag;
	s32											m_flags;
	float										m_duration;
	s16											m_itemCount;						// Item count	
	s16											m_timeblockCount;					// Timeblock count

	s16											m_channelCount;						// This is total channel count.. (inclusive of constant channels)
	s16											m_pad0;								// We can use this for something else later, if we want..
	FwOffset<const FwVector4>					m_constantChannelArray;				// Array containing constant channels data (used for both direct and indirect constant channels)
	FwOffset<const u16>							m_constantChannelIndexArray;		// Array containing indices for indereted constant channels data
	FwOffset<const FpAnimTimeblockIndex>		m_timeblockIndexArray;				// The FpAnimTimeblockIndex array

	FwOffset<const LocomotionInfo>				m_locomotionInfo;					// Optional data.. 	
	FwOffset<const u8>							m_endSpuDmaIn;						// SPU data ends here

	FwOffset<const FwHashedString>				m_hashedStringArray;				// Only used on PPU - array of hashed string used for channel names
	FwOffset<const FpAnimChannelInfo>			m_channelInfoArray;					// Only used on PPU 
	FwOffset<const FpAnimItem>					m_itemArray;						// Only used on PPU

	FwOffset<const FpAnimKeyframeHermite>		m_keyframeHermiteArray;				// Used both by PPU and SPU (gather DMA)
	FwOffset<const FpAnimKeyframeHalfHermite>	m_keyframeHalfHermiteArray;			// Used both by PPU and SPU (gather DMA)
	FwOffset<const u8>							m_keyframePackedHermiteBitstream;	// Used both by PPU and SPU (gather DMA)

	FwOffset<const FpAnimKeyframePackingSpec>	m_packedHermitePackingSpecArray;	// Packing spec dictionary (per packed channel / indirected)
	FwOffset<const u16>							m_packedHermiteSpecIndexArray;		// Indices for packed hermite packing specs

	FwOffset<const FpAnimKeyframeTransform>		m_packedHermiteTransformArray;		// Array of keyframe transforms (per packed channel / indirected)
	FwOffset<const u16>							m_packedHermiteTransformIndexArray;	// Indices for packed hermite packing keyframe transform

	u16											m_channelTypeCount[ FpAnimChannelInfo::kNumKeyframeTypes ];	// Number of channels for each keyframe type
	u16											m_channelTypeStart[ FpAnimChannelInfo::kNumKeyframeTypes ]; // Start channel index for each block of keyframe types.

	u32											m_pad[ 3 ];
};

// Make sure that this structure is a multiple of 16 bytes
FW_STATIC_ASSERT( ( sizeof( FpAnimClipDef ) % 16 ) == 0 );


//--------------------------------------------------------------------------------------------------
/**
	@class			FpAnimClip
	
	@brief			An object that manages data associated with instantiation of an animation clip.
**/
//--------------------------------------------------------------------------------------------------

class	FpAnimClip : public FwNonCopyable
{
public:
	// Construction & destruction functions..
	static	int				QuerySizeInBytes( const FpAnimClipDef* pAnimClipDef );
	static	FpAnimClip*		Create( const FpAnimClipDef* pAnimClipDef, void* pUserMemoryArea = NULL );
	static	void			Destroy( FpAnimClip* pAnimClip );

	// Access to the original clip definition
	const FpAnimClipDef*	GetAnimClipDef( void ) const;

	// Pass through basic information from clip..
	float					GetDuration( void ) const;

	// Evaluate the clip at a given time
	void					Evaluate( float timeValue );

	// Access to items & evaluated data (for general use)
	int						GetChannelIndex( FwHashedString itemName, FwHashedString channelName ) const;
	const FwVector4*		GetChannelData( int channelIndex ) const;
	const FwVector4*		GetChannelData( FwHashedString itemName, FwHashedString channelName ) const;

	// Static helper for packed -> unpacked keyframe conversion
	static void				UnpackHalfHermite( u128 packedInput, FwVector4& unpackedOutputA, FwVector4& unpackedOutputB );
	
protected:
	FpAnimClip()	{};
	~FpAnimClip()	{};

	// Access to an item	
	const FpAnimItem*		GetItem( FwHashedString itemName ) const;

	const FpAnimClipDef*	m_pAnimClipDef;
	FwVector4*				m_pResultArray;

	// Does this anim clip own its own allocation, or is it created in user-controlled memory..
	bool					m_ownsMemory;
};

//--------------------------------------------------------------------------------------------------
//	INLINES
//--------------------------------------------------------------------------------------------------

#ifdef __SPU__
#	include <../Fp/FpAnim/FpAnimClip.inl>
#else //__SPU__
#	include <Fp/FpAnim/FpAnimClip.inl>
#endif//__SPU__

//--------------------------------------------------------------------------------------------------

#endif	// FP_ANIM_CLIP_H
