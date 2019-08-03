//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_ANIMATOR_H
#define GP_ANIMATOR_H

#include <Fw/FwMaths/FwQuat.h>
#include <Fw/FwMaths/FwVector.h>
#include <Fw/FwMaths/FwPoint.h>
#include <Fw/FwMaths/FwTransform.h>

#ifdef __SPU__	
#	include <../Fp/FpAnim/FpAnimClip.h>
#else //__SPU__
#	include <Fp/FpAnim/FpAnimClip.h>
#endif//__SPU__

#ifdef __PPU__ // We need a WwsJob JobListMarker 
#	include <wwsbase/types.h>
#	include <jobsystem/helpers.h>
#	include <jobapi/joblist.h>
#endif//__PPU__

class	GpAnimation;
class	GpAnimator;
class	GpSkeleton;
class	GpJoint;
class	GpJointLinkage;

class	GpAnimNamePair;
class	GpAnimUserBindingDef;
class	GpAnimMirrorBindingDef;

class	GpAnimatorBatch;

class	MultiThreadSafeJobList;
class	SingleThreadJobList;


//--------------------------------------------------------------------------------------------------
/**
	@class			GpAnimNamePair
	
	@brief			A class used by GpAnimUserBindingDef to assist in the definition & storage of
					pairs of hashed names. The meaning of the names is dependent on the class that
					uses them. Please see the corresponding class documentation for more information.			
**/
//--------------------------------------------------------------------------------------------------

class	GpAnimNamePair
{
public:
	GpAnimNamePair( FwHashedString name1, FwHashedString name2 )
	{
		m_name[ 0 ] = name1;	
		m_name[ 1 ] = name2;
	}

	u64				GetPairValue( void ) const
	{
		return ( ( u64 )m_name[0].Get() << 32 ) | ( ( u64 )m_name[1].Get() );
	}

	FwHashedString	GetName( int index ) const
	{
		FW_ASSERT( ( index >= 0 ) && ( index <= 1 ) );
		return m_name[ index ];
	}

private:
	FwHashedString		m_name[ 2 ];
};

//--------------------------------------------------------------------------------------------------
/**
	@class			GpAnimUserBindingDef
	
	@brief			This object contains the information required to manage binding of specified
					user channels of data to a GpAnimator object. Following the header, an array 
					of GpAnimNamePair objects (m_count in size) exists. The array is sorted in
					ascending pair value (where a pair is represented by a 'u64' data type). The 
					item name exists in the upper 32-bits, and the channel name exists in the lower
					32-bits.
			
					No duplicates of the u64 combination are permitted.

	@note			Ideally this structure should be generated offline, but there are functions to
					generate runtime versions incase that is required. Note that these runtime
					functions do not perform duplicate validation though..		
**/
//--------------------------------------------------------------------------------------------------

class	GpAnimUserBindingDef
{
public:
	static	GpAnimUserBindingDef*	Create( GpAnimNamePair* pNamePairArray, int pairCount, void* pUserMemoryArea = NULL );
	static	int						QuerySizeInBytes( GpAnimNamePair* pNamePairArray, int pairCount );
	static	void					Destroy( GpAnimUserBindingDef* pUserBindingDef );
	static	u32						GetClassTag()	{ return FW_MAKE_TAG( 'A','U','0','1' ); }

	u32								GetTag( void ) const { return m_tag; }
	int								GetCount( void ) const { return m_count; }
	int								GetBindingIndex( FwHashedString itemName, FwHashedString channelName ) const;

	static	const	int	kInvalidIndex = -1;

private:
	GpAnimUserBindingDef()	{};
	~GpAnimUserBindingDef()	{};

	u32		m_tag;
	s16		m_ownsMemory;
	s16		m_count;
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpAnimMirrorSpec
	
	@brief			Namespace containing enumerations related to mirroring behaviour definition
**/
//--------------------------------------------------------------------------------------------------

namespace	GpAnimMirrorSpec
{
	enum	MirrorSpecBits
	{
		kQuatX		=	0,
		kQuatY		=	1,
		kQuatZ		=	2,
		kQuatW		=	3,

		kTransX		=	0,
		kTransY		=	1,
		kTransZ		=	2,
		kTransW		=	3,

		kNegate		=	4,
		
		kNegQuatX	=	kNegate + kQuatX,
		kNegQuatY	=	kNegate + kQuatY,
		kNegQuatZ	=	kNegate + kQuatZ,
		kNegQuatW	=	kNegate + kQuatW,

		kNegTransX	=	kNegate + kTransX,
		kNegTransY	=	kNegate + kTransY,
		kNegTransZ	=	kNegate + kTransZ,
		kNegTransW	=	kNegate + kTransW,
	};
	
	#define	GPANIM_MIRROR_QUAT( qx, qy, qz, qw )		( ( ( qx ) << 28 ) | ( ( qy ) << 24 ) | ( ( qz ) << 20 ) | ( ( qw ) << 16 ) )
	#define	GPANIM_MIRROR_TRANS( tx, ty, tz )			( ( ( tx ) << 12 ) | ( ( ty ) <<  8 ) | ( ( tz ) <<  4 ) | ( kTransW << 0 ) )
	#define	GPANIM_MIRROR( qx, qy, qz, qw, tx, ty, tz )	( GPANIM_MIRROR_QUAT( ( qx ), ( qy ), ( qz ), ( qw ) ) | GPANIM_MIRROR_TRANS( ( tx ), ( ty ), ( tz ) ) )

	enum	MirrorSpecs
	{
		kNotMirrored					= GPANIM_MIRROR( kQuatX, kQuatY, kQuatZ, kQuatW,			kTransX, kTransY, kTransZ ),		// No mirroring at all

		kBehaviouralNonPaired			= GPANIM_MIRROR( kNegQuatX, kQuatY, kQuatZ, kNegQuatW,		kNegTransX, kTransY, kTransZ ),		// Mirror rotation in yz, mirror translation in x
		kBehaviouralLinkedToNonPaired	= GPANIM_MIRROR( kNegQuatW, kQuatZ, kNegQuatY, kQuatX,		kNegTransX, kTransY, kTransZ ),		// Mirror rotation in yz, rotate around x by pi, mirror translation in x
		kBehaviouralPaired				= GPANIM_MIRROR( kQuatX, kQuatY, kQuatZ, kQuatW,			kNegTransX, kNegTransY, kTransZ ),	// Mirror translation in x and y

		kOrientation					= GPANIM_MIRROR( kQuatX, kNegQuatY, kNegQuatZ, kQuatW,		kNegTransX, kTransY, kTransZ ),		// Mirror rotation in yz, mirror translation in x
	};	
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpAnimMirrorPair
	
	@brief			A class used by GpAnimMirrorBindingDef to assist in the definition & storage of
					mirrored pairs, along with any mirroring flags that are required by the runtime.
**/
//--------------------------------------------------------------------------------------------------

class	GpAnimMirrorPair
{
public:
	GpAnimMirrorPair( FwHashedString name1, FwHashedString name2, u32 mirrorSpec = NULL )
	: m_mirrorPair( name1, name2 ), m_mirrorSpec( mirrorSpec )
	{
	}

	u64				GetPairValue( void ) const	{ return m_mirrorPair.GetPairValue(); }
	FwHashedString	GetName( int index ) const	{ return m_mirrorPair.GetName( index ); }
	u32				GetSpec( void ) const		{ return m_mirrorSpec; }

private:
	GpAnimNamePair	m_mirrorPair;
	u32				m_mirrorSpec;
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpAnimMirrorBindingDef
	
	@brief			This object contains the information required to manage mirroring of joints
					by a GpAnimation object. Following the header, an array of GpAnimNamePair objects
					(m_count in size) exists. And following that, there exists an array of 32-bit
					specification elements associated with each pair definition. The array is sorted
					in ascending pair value (where a pair is represented by a 'u64' data type). The
					original item/joint name exists in the upper 32-bits, and the mirrored item/joint
					name exists in the lower 32-bits. 

					Please be aware that if a pair for 'a' and 'b' exists, then there should be 
					a corresponding pair for 'b' and 'a' in the exported data structure, positioned
					in ascending pair value like any other. This only holds true for exported data
					though, in that if Create() is being called, the corresponding pair for 'b' and
					'a' will be created dynamically. See Create() documentation for more details.

					No duplicates of the u64 combination are permitted, and reuse of mirrored names
					is not permitted either. So, you cannot have 'a' mirrored to 'b' and also 'a' 
					mirrored to 'c'. That would be bad.

	@note			Ideally this structure should be generated offline, but there are functions to
					generate runtime versions incase that is required. Note that these runtime
					functions do not perform duplicate validation though..		
**/
//--------------------------------------------------------------------------------------------------

class	GpAnimMirrorBindingDef
{
public:
	static	GpAnimMirrorBindingDef*	Create( GpAnimMirrorPair* pMirrorPairArray, int pairCount, void* pUserMemoryArea = NULL );
	static	int						QuerySizeInBytes( GpAnimMirrorPair* pMirrorPairArray, int pairCount );
	static	void					Destroy( GpAnimMirrorBindingDef* pMirrorBindingDef );
	static	u32						GetClassTag()	{ return FW_MAKE_TAG( 'A','M','0','2' ); }

	u32								GetTag( void ) const { return m_tag; }
	int								GetCount( void ) const { return m_count; }
	FwHashedString					GetMirroredName( FwHashedString itemName ) const;

	const	GpAnimMirrorPair*		GetMirrorPairArray( void ) const { return ( GpAnimMirrorPair* )( this + 1 ); }

private:
	GpAnimMirrorBindingDef()	{};
	~GpAnimMirrorBindingDef()	{};

	u32		m_tag;
	s16		m_ownsMemory;
	s16		m_count;	
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpAnimJointMask
	
	@brief			Allows joints to be masked, meaning that they will not be subject to animation
					evaluation. Instead, disabled joints will contain bind pose data within the
					evaluation slot positions.

					This class exists to allow for pre-creation and easy setting of a number of joint
					masks. For example, ones associated with varying LOD settings. 

					The mask is only used when a skeleton is associated with a GpAnimator object,
					and will only prevent the evaluation of rotation, translation and scale channels.
					User channels associated with joint names will continue to be evaluated as before.

	@note			Remember that all joints and local->world matrices are output as normal. This
					masking system *only affects evaluation*.
**/
//--------------------------------------------------------------------------------------------------

class	GpAnimJointMask
{
public:
	GpAnimJointMask();
	GpAnimJointMask( const GpAnimJointMask& jointMask );

	// Assignment
	GpAnimJointMask& operator = ( const GpAnimJointMask& jointMask );

	// Query
	bool	IsEnabled( int jointIndex ) const;
	bool	AllEnabled( void ) const;

	// Modification
	void	DisableJoint( int jointIndex );
	void	EnableJoint( int jointIndex );
	void	Reset( void );
	void	Merge( const GpAnimJointMask& jointMask );
	
private:
	u32	m_jointMask[ 4 ];
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpAnimation
	
	@brief			This is effectively an instance of an FpAnimClipDef that is closely associated 
					with a single GpAnimator object. 	
**/
//--------------------------------------------------------------------------------------------------

class	GpAnimation
{
	friend	class GpAnimatorUpdate;

public:
	typedef	unsigned int	LocomotionKey;
	
	// Creation and destruction
	static	int				QuerySizeInBytes( const GpAnimator* pAnimator, const FpAnimClipDef* pAnimClipDef, const GpAnimMirrorBindingDef* pMirrorBindingDef = NULL );
	static	GpAnimation*	Create( const GpAnimator* pAnimator, const FpAnimClipDef* pClipDef, const GpAnimMirrorBindingDef* pMirrorBindingDef = NULL, void* pUserMemoryArea = NULL );
	static	void			Destroy( GpAnimation* pAnimation );

	// Retrieve information
	float					GetDuration( void ) const					{ return m_pAnimClipDef->GetDuration(); }

	// Access connected objects
	const FpAnimClipDef*	GetAnimClipDef( void ) const				{ return m_pAnimClipDef; };
	const u32				GetSizeSpuAnimClipDef( void ) const			{ return m_sizeSpuAnimClipDef; };
	const GpAnimator*		GetAnimator( void ) const					{ return m_pAnimator; };

	// Control blend weight scalar.. worry about how these are set later.
	void					SetBlendScalar( FwHashedString itemName, FwHashedString channelName, float scalar );
	float					GetBlendScalar( FwHashedString nameHash, FwHashedString channelName ) const;

	// Retrieve locomotion data
	FwQuat					GetLastRootRotation( void ) const			{ return m_lastRootRotation; }
	FwPoint					GetLastRootTranslation( void ) const		{ return m_lastRootTranslation; }
	FwQuat					GetRootRotationDelta( void ) const			{ return m_rootRotationDelta; }
	FwVector				GetRootTranslationDelta( void ) const		{ return m_rootTranslationDelta; }

	// Joint mask support
	void					SetJointMask( const GpAnimJointMask& jointMask )	{ m_jointMask = jointMask; }
	const GpAnimJointMask&	GetJointMask( void ) const							{ return m_jointMask; }

	// Acquire a locomotion key for use in ResetLocomotion() when resetTime != 0.0f
	LocomotionKey			GetLocomotionKey( void ) const;

	// Reset locomotion data
	void					ResetLocomotion( float resetTime = 0.0f, LocomotionKey locomotionKey = 0xffffffff );

	// Locking behaviour
	void					Lock( void );
	void					Unlock( void );
	bool					IsLocked( void ) const;

private:
	// We don't want people to be able to embed or construct this object themselves, as it has a complex allocation layout.
	GpAnimation() {};
	~GpAnimation() {};

	// Internal Flags
	enum	Flags
	{
		kOwnsMemory		= ( 1 << 0 ),
		kIsLocked		= ( 1 << 1 ),
		kIsLocomoting	= ( 1 << 2 ),
		kIsMirrored		= ( 1 << 3 ),
	};

	// This data is structured such that fields marked 'R/W' can be written back with a 128-byte transfer. Do not change without thinking about it.
	FwQuat						m_lastRootRotation;			// R/W	:	Locomotion : Last rotation of root joint
	FwPoint						m_lastRootTranslation;		// R/W	:	Locomotion : Last translation of root joint
	FwQuat						m_rootRotationDelta;		// R/W	:	Locomotion : Rotational delta for root joint
	FwVector					m_rootTranslationDelta;		// R/W	:	Locomotion : Translational delta for root joint
	FwQuat						m_rootEndDeltaRotation;		// R	:	Locomotion : End delta
	FwVector					m_rootEndDeltaTranslation;	// R	:	Locomotion : End delta (needed by SPU evaluation in read-only capacity)
	
	const GpAnimator*			m_pAnimator;				// This is a pointer to owning GpAnimator.
	const FpAnimClipDef*		m_pAnimClipDef;				// This is a pointer to the FpAnimClipDef
	u32							m_sizeSpuAnimClipDef;		// This is the size of the first part of the animclip we have to DMA-in on SPU

	s32							m_flags;					// 'owns memory', for example
	s32							m_numChannels;				// Total number of channels being animated (duplicate from anim clip, for local use)

	GpAnimJointMask				m_jointMask;				// Mask to prevent evaluation of joint (bit set = no evaluation)

	u32*						m_pMirrorSpecsArray;		// Pointer to mirroring specification array (or NULL if not mirroring)

	// Each of these blocks is 128-byte aligned in start and size 
	float*						m_pBlendWeightScalarArray;	// Pointer to blend weight scalars. 
	s16*						m_pLocalToSlotBindingArray;	// Pointer to binding indices (local channel index to evaluation slot channel index)

	u8							m_pad[ 108 ];				// Pad to 128 bytes
};

// Make sure that this structure is a multiple of 128 bytes
FW_STATIC_ASSERT( ( sizeof( GpAnimation ) % 128 ) == 0 );


//--------------------------------------------------------------------------------------------------
/**
	@class			GpAnimator
	
	@brief			Manages the update/processing of a number of GpAnimation objects, and the	
					application of skeletal and/or user data results.

					GpAnimator has two different ways to update. 

					- kRunOnPPU : (default) synchronous update on PPU (blocking / slow)
					- kRunOnSPU : asynchronous on SPU (faster)

					See documentation of GpAnimator::StartUpdate( )

	@note			You should attempt to update an animation (on PPU or SPU) while there is still
					a pending SPU update. Job markers are store in the GpAnimator object and will
					be overwritten/lost.
**/
//--------------------------------------------------------------------------------------------------

class	GpAnimator
{
	friend class GpAnimatorUpdate;
	friend class GpAnimatorBatch;

public:
	// Update Mode
	enum	UpdateMode
	{
		kRunOnPPU,
		kRunOnSPU,
	};

	// Flags
	enum	Flags
	{
		kAutoApplyDeltas				= ( 1 << 0 ),
		kDeltasAvailable				= ( 1 << 1 ),

		// These flag bits are private.. please don't mess with them.
		kOwnsMemory						= ( 1 << 2 ),
		kIsLocked						= ( 1 << 3 ),
		kCommandListBuild				= ( 1 << 4 ),
		kCommandListOutputs				= ( 1 << 5 ),
		kCommandListWritesSkeleton		= ( 1 << 6 ),

		// This flag is not used at this time.. but it's here so I don't forget that - at some
		// point in the future - it may be required. By default the evaluation slot space is
		// initialised to the bind pose. If a skeleton is only being partially updated by animation
		// with the rest of the skeleton being updated by code, then it may be that having the 
		// animation system blat over your non-animated joints with bind pose data is a 'bad thing'.
		kInitialiseSlotsWithJointData	= ( 1 << 7 )
	};

	// Flags for evaluation command
	enum	EvalFlags
	{
		kDisableLocomotion			= ( 1 << 0 ),
	};

	// Creation and destruction
	static	int					QuerySizeInBytes( GpSkeleton* pSkeleton, GpAnimUserBindingDef* pUserBindingDef = NULL, int commandListSize = kDefaultCommandListSize );
	static	GpAnimator*			Create( GpSkeleton* pSkeleton, GpAnimUserBindingDef* pUserBindingDef = NULL, void* pUserMemoryArea = NULL, int commandListSize = kDefaultCommandListSize );
	static	void				Destroy( GpAnimator* pAnimator );

	// Accessors
	GpSkeleton*					GetSkeleton( void ) const					{ return m_pSkeleton; }
	const GpAnimUserBindingDef*	GetUserBindingDef( void ) const				{ return m_pUserBindingDef; }
	FwQuat						GetRootRotationDelta( void ) const			{ FW_ASSERT( !IsLocked() ); return m_rootRotationDelta; };
	FwVector					GetRootTranslationDelta( void ) const		{ FW_ASSERT( !IsLocked() ); return m_rootTranslationDelta; };

	// Flags
	int							GetFlags( void ) const						{ FW_ASSERT( !IsLocked() ); return m_flags; }				// Return flags
	void						SetFlags( int flags )						{ FW_ASSERT( !IsLocked() ); m_flags = flags; }				// Sets flags
	void						SetFlagBits( int flagBits )					{ FW_ASSERT( !IsLocked() ); m_flags |= flagBits; }			// Sets flag bits
	void						ClearFlagBits( int flagBits )				{ FW_ASSERT( !IsLocked() ); m_flags &= ~flagBits; }			// Clears flag bits

	// We need to let people get user data
	int							GetUserChannelIndex( FwHashedString itemName, FwHashedString channelName ) const;
	const FwVector4*			GetUserChannel( int userChannelIndex ) const;
	const FwVector4*			GetUserChannel( FwHashedString itemName, FwHashedString channelName ) const;

	// Command List
	void						StartCommandList( void );
	void						AddCmdEvaluate( int destSlot, GpAnimation* pAnimation, float time, int loopCount = 0, int evalFlags = 0 );
	void						AddCmdBlend( int destSlot, int lhsSlot, int rhsSlot, float alpha );
	void						AddCmdCompose( int destSlot, int lhsSlot, int rhsSlot );
	void						AddCmdInvert( int destSlot, int srcSlot );
	void						AddCmdApplyDelta( int destSlot, int baseSlot, int deltaSlot );
	void						AddCmdInputSkeletalJoints( int destSlot );
	void						AddCmdOutput( int srcSlot );
	void						AddCmdOutputSkeletalJoints( int srcSlot, GpJoint* pDestAddress );
	void						AddCmdOutputSkeletalTransforms( int srcSlot, FwTransform* pDestAddress );
	void						AddCmdOutputUserChannels( int srcSlot, FwVector4* pDestAddress );
	void						EndCommandList( void );

	// Update
	void						StartUpdate( UpdateMode updateMode );
	void						StartUpdate( UpdateMode updateMode, SingleThreadJobList* pWwsJobListST, int readyCount = 8 );
	void						StartUpdate( UpdateMode updateMode, MultiThreadSafeJobList* pWwsJobListMT, int readyCount = 8 );

	void						WaitForUpdate( void );
	bool						IsUpdating( void ) const;

	// Locking behaviour
	void						Lock( void );
	void						Unlock( void );
	bool						IsLocked( void ) const;

	// Retrieve user channel start index (this is for GpAnimation use.. not yours!)
	int							GetUserChannelStart( void ) const;

	// Skinning matrices support
	void						SetSkinningInvBindTransforms( const FwTransform* pSkinningInvTransforms )	{ m_pSkinningInvBindTransforms = pSkinningInvTransforms; }
	void						SetSkinningTransforms( FwTransform* pSkinningTransforms )					{ m_pSkinningTransforms = pSkinningTransforms; }
	const FwTransform*			GetSkinningInvBindTransforms( void ) const									{ return m_pSkinningInvBindTransforms; }
	FwTransform* 				GetSkinningTransforms( void )												{ return m_pSkinningTransforms; }

	// Joint mask support
	void						SetJointMask( const GpAnimJointMask& jointMask )	{ m_jointMask = jointMask; }
	const GpAnimJointMask&		GetJointMask( void ) const							{ return m_jointMask; }
	
	// Get size of SPU data ( to allocate temporary buffer )
	static	size_t				GetGpAnimatorUpdateSpuSize( void );

	// Constants..
	static	const	int	kNumberOfSlots			= 4;
	static	const	int	kDefaultCommandListSize	= 32;

private:
	GpAnimator()	{}
	~GpAnimator()	{}

	// Internal pre-update processing, that is shared between GpAnimator and GpAnimatorBatch
	void						PrepareBeforeUpdate( void );
	void						PrepareAfterWait( void );
	void						InvokeCpuBasedUpdate( void );

	// Internal update ( shared by StartUpdate overload for SingleThreadJobList and MultiThreadSafeJobList )
	void						StartUpdateInternal( UpdateMode updateMode, SingleThreadJobList* pWwsJobListST, MultiThreadSafeJobList* pWwsJobListMT, int readyCount );

	// Batching
	GpAnimatorBatch*			GetAnimatorBatch( void );
	GpAnimator*					GetNextBatchedAnimator( void );
	void						SetNextBatchedAnimator( GpAnimatorBatch* pAnimatorBatch, GpAnimator* pAnimator );

#ifdef __PPU__
	// Internal job management
	void						AddJob( SingleThreadJobList* pWwsJobListST, MultiThreadSafeJobList* pWwsJobListMT, JobListPrivate::ReadyCountValues readyCount );
	
	// Internal functions to be able to use the JobListMarker class. 
	inline const JobListMarker&	GetJobListMarker( void ) const						{ return *( ( JobListMarker* ) &m_bytesWwsJobMarker[ 0 ] ); }
	inline bool					IsJobListMarkerNull( void ) const					{ return 0ULL == *reinterpret_cast<const u64*>( &m_bytesWwsJobMarker[ 0 ] ); }
	inline void					SetJobListMarker( const JobListMarker marker )		{ *reinterpret_cast<JobListMarker*>( &m_bytesWwsJobMarker[ 0 ] ) = marker ; }
	inline void					SetJobListMarkerNull( void )						{ *reinterpret_cast<u64*>( &m_bytesWwsJobMarker[ 0 ] ) = 0ULL ; }
	void						StallForJobListMarker( void )						{ if ( !IsJobListMarkerNull() ) { GetJobListMarker().StallForJobMarker(); } }
#endif//__PPU__

	enum	commandTypes
	{
		kEvaluate,					
		kBlend,
		kCompose,
		kInvert,
		kApplyDelta,	
		kInputSkeletalJoints,		
		kOutput,					
		kOutputSkeletalJoints,		
		kOutputSkeletalTransforms,	
		kOutputUserChannels,
	};

	// This defines our animation command packet.
	struct	AnimCommand
	{
		s8				m_commandId;
		s8				m_arg0;
		s8				m_arg1;
		s8				m_arg2;
		float			m_arg3;
		void*			m_pDataAddress;	
	};

	// This data is structured such that fields marked 'R/W' can be written back with a 128-byte transfer. Do not change without thinking about it.
	s32							m_flags;						// R/W	:	Flags
	GpJoint*					m_pSkeletalJointArray;			// R	:	Pointer to skeletal joint array, in hierarchy order. Or NULL.
	FwTransform*				m_pSkeletalTransformArray;		// R	:	Pointer to skeletal transform array, in hierarchy order. Or NULL.
	u8*							m_pSkeletalJointFlagsArray;		// R	:	Pointer to skeletal joint flags. Or NULL.
	FwVector4*					m_pUserChannelArray;			// R	:	Pointer to user channel array. Order to be confirmed. Or NULL.

	FwQuat						m_rootRotationDelta;			// R/W	:	Locomotion		:	Rotational delta for root joint
	FwVector					m_rootTranslationDelta;			// R/W	:	Locomotion		:	Translational delta for root joint
	FwQuat						m_skeletalRootJointRotation;	// R/W	:	Locomotion		:	Saves us from referring to skeletal root joint rotation
	FwTransform					m_skeletalRootTransform;		// R	:	Synchronisation	:	Saves us referring to skeleton in animation code.
	FwPoint						m_skeletalRootJointTranslation;	// R	:	Locomotion		:	Saves us from referring to skeletal root joint translation

	// Command structure
	s32							m_commandCount;					// Number of commands in m_commandList
	s32							m_commandListSize;				// Size of command list pointed to by m_pCommandList
	AnimCommand*				m_pCommandList;					// Pointer to command list (128 byte start/size aligned)

	// We need this stuff kicking around
	s32							m_userChannelStart;				// Save us computing this every time..
	s32							m_userChannelCount;				// Number of user channels we're dealing with
	const GpAnimUserBindingDef*	m_pUserBindingDef;				// Pointer to user binding definition

	// We need this information from the skeleton too.. 
	s32							m_skeletalJointCount;			// Number of skeletal joints.
	GpSkeleton*					m_pSkeleton;					// Pointer to related skeleton, or NULL.
	const GpJoint*				m_pSkeletalJointBindPoseArray;	// Pointer to skeletal bind pose, or NULL.
	const GpJointLinkage*		m_pSkeletalJointLinkageArray;	// Pointer to skeletal joint linkage, or NULL.

	// Joint mask (global, affects all animations)
	GpAnimJointMask				m_jointMask;					// Mask to prevent evaluation of joint (bit set = no evaluation)

	// Support for ICE::mesh-style skinning matrices
	const FwTransform*			m_pSkinningInvBindTransforms;	// Pointer to inverse bind matrices(input, or NULL.
	FwTransform*				m_pSkinningTransforms;			// Pointer to skin matrices (output), or NULL.

	// Linkage for GpAnimationBatch processing
	GpAnimatorBatch*			m_pAnimatorBatch;				// Pointer to owning GpAnimator batch, or NULL.
	GpAnimator*					m_pNextBatchedAnimator;			// Pointer to next animation in batch, or NULL.

	u8							m_pad[ 24 ];

	// Pad this to have 128-byte size..? This is also used as the WwsJob command buffer..
	u8							m_bytesWwsJobCommandList[ 96 ];		// WwsJob commandlist
	u8							m_bytesWwsJobMarker[ 16 ];			// WwsJob marker (note: we can't use a union as the joblist marker is a C++ object with mandatory constructor)
	u32							m_paramsWwsJob[ 4 ];				// WwsJob parameters jor JobMain
};

// Make sure that this structure is a multiple of 128 bytes
FW_STATIC_ASSERT( ( sizeof( GpAnimator ) % 128 ) == 0 );

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#ifdef __SPU__
#	include <../Gp/GpAnimator/GpAnimator.inl>
#else//__SPU__
#	include <Gp/GpAnimator/GpAnimator.inl>
#endif//__SPU__

#endif	// GP_ANIMATOR_H
