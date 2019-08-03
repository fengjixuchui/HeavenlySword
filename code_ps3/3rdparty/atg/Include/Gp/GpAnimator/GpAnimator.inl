//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Animation System : Inlines

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_ANIMATOR_INL
#define GP_ANIMATOR_INL

//--------------------------------------------------------------------------------------------------
/**
	@brief		Lock access to animation data
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimation::Lock( void )
{
	FW_ASSERT( !( m_flags & kIsLocked ) );
	m_flags |= kIsLocked;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Unlock access to animation data
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimation::Unlock( void )
{
	FW_ASSERT( m_flags & kIsLocked );
	m_flags &= ~kIsLocked;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Query animation lock status
**/
//--------------------------------------------------------------------------------------------------

inline	bool	GpAnimation::IsLocked( void ) const
{
	return ( m_flags & kIsLocked );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Default constructor for GpAnimJointMask.

	@note		All joints are enabled after this call
**/
//--------------------------------------------------------------------------------------------------

inline	GpAnimJointMask::GpAnimJointMask()
{
	Reset();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Copy constructor for GpAnimJointMask	
**/
//--------------------------------------------------------------------------------------------------

inline	GpAnimJointMask::GpAnimJointMask( const GpAnimJointMask& jointMask )
{
	m_jointMask[ 0 ] = jointMask.m_jointMask[ 0 ];
	m_jointMask[ 1 ] = jointMask.m_jointMask[ 1 ];
	m_jointMask[ 2 ] = jointMask.m_jointMask[ 2 ];
	m_jointMask[ 3 ] = jointMask.m_jointMask[ 3 ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Assignment operator for GpAnimJointMask	
**/
//--------------------------------------------------------------------------------------------------

inline	GpAnimJointMask& GpAnimJointMask::operator = ( const GpAnimJointMask& jointMask )
{
	m_jointMask[ 0 ] = jointMask.m_jointMask[ 0 ];
	m_jointMask[ 1 ] = jointMask.m_jointMask[ 1 ];
	m_jointMask[ 2 ] = jointMask.m_jointMask[ 2 ];
	m_jointMask[ 3 ] = jointMask.m_jointMask[ 3 ];
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Query whether the specified skeletal joint is enabled.
**/
//--------------------------------------------------------------------------------------------------

inline	bool	GpAnimJointMask::IsEnabled( int jointIndex ) const
{
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < FpAnimClipDef::kMaxItems ) );
	return m_jointMask[ jointIndex >> 5 ] & ( 1 << ( jointIndex & 0x1f ) );	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Query whether all joints are enabled.
**/
//--------------------------------------------------------------------------------------------------

inline	bool	GpAnimJointMask::AllEnabled( void ) const
{
	u32	workMask = m_jointMask[ 0 ] & m_jointMask[ 1 ] & m_jointMask[ 2 ] & m_jointMask[ 3 ];
	return workMask == 0xffffffff;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Disable animation evaluation for the specified skeletal joint index 
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimJointMask::DisableJoint( int jointIndex )
{
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < FpAnimClipDef::kMaxItems ) );
	m_jointMask[ jointIndex >> 5 ] &= ~( 1 << ( jointIndex & 0x1f ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Enable animation evaluation for the specified skeletal joint index 
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimJointMask::EnableJoint( int jointIndex )
{
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < FpAnimClipDef::kMaxItems ) );
	m_jointMask[ jointIndex >> 5 ] |= ( 1 << ( jointIndex & 0x1f ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Reset joint mask to a state where all joints are evaluated.
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimJointMask::Reset( void )
{
	m_jointMask[ 0 ] = 0xffffffff;
	m_jointMask[ 1 ] = 0xffffffff;
	m_jointMask[ 2 ] = 0xffffffff;
	m_jointMask[ 3 ] = 0xffffffff;	
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Merge specified joint mask with current values.
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimJointMask::Merge( const GpAnimJointMask& jointMask )
{
	m_jointMask[ 0 ] &= jointMask.m_jointMask[ 0 ];
	m_jointMask[ 1 ] &= jointMask.m_jointMask[ 1 ];
	m_jointMask[ 2 ] &= jointMask.m_jointMask[ 2 ];
	m_jointMask[ 3 ] &= jointMask.m_jointMask[ 3 ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Retrieve an opaque user channel index for a given item & channel name
	
	@param		itemName			Item name
	@param		channelName			Channel name

	@result		Valid (opaque) user channel index, or GpAnimUserBindingDef::kInvalidIndex if the
				item/channel pair does not exist.
	
	@note		Do not cache this value in off-line tools! It is dynamic, based on skeletal layout!
**/
//--------------------------------------------------------------------------------------------------

inline	int	GpAnimator::GetUserChannelIndex( FwHashedString itemName, FwHashedString channelName ) const
{
	FW_ASSERT( m_pUserBindingDef );
	int	bindingIndex = m_pUserBindingDef->GetBindingIndex( itemName, channelName );
	return bindingIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Retrieve a pointer to user channel data, using an opaque user channel index.
	
	@param		userChannelIndex	Index, acquired via GetUserChannelIndex()

	@result		A pointer to an FwVector4 containing user channel data, or NULL if the specified
				item/channel pair does not exist.
**/
//--------------------------------------------------------------------------------------------------

inline	const FwVector4*	GpAnimator::GetUserChannel( int userChannelIndex ) const
{
	FW_ASSERT( m_pUserBindingDef );
	FW_ASSERT( ( userChannelIndex >= 0 ) && ( userChannelIndex < ( int )m_userChannelCount ) );
	return m_pUserChannelArray + userChannelIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Retrieve a pointer to user channel data, using the item & channel names
	
	@param		itemName			Item name
	@param		channelName			Channel name

	@result		A pointer to an FwVector4 containing user channel data, or NULL if the specified
				item/channel pair does not exist.
**/
//--------------------------------------------------------------------------------------------------

inline	const FwVector4*	GpAnimator::GetUserChannel( FwHashedString itemName, FwHashedString channelName ) const
{
	FW_ASSERT( m_pUserBindingDef );
	int	channelIndex = GetUserChannelIndex( itemName, channelName );
	if ( channelIndex >= 0 )
	{
		return GetUserChannel( channelIndex );
	}
	return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Retrieve user channel start index (for use by GpAnimation & GpAnimator only!)
**/
//--------------------------------------------------------------------------------------------------

inline	int		GpAnimator::GetUserChannelStart( void ) const
{
	return m_userChannelStart;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Lock access to animator data
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimator::Lock( void )
{
	FW_ASSERT( !( m_flags & kIsLocked ) );
	m_flags |= kIsLocked;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Unlock access to animator data
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimator::Unlock( void )
{
	FW_ASSERT( m_flags & kIsLocked );
	m_flags &= ~kIsLocked;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Query animator lock status
**/
//--------------------------------------------------------------------------------------------------

inline	bool	GpAnimator::IsLocked( void ) const
{
	return ( m_flags & kIsLocked );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Retrieve pointer to the owning GpAnimatorBatch
**/
//--------------------------------------------------------------------------------------------------

inline	GpAnimatorBatch*	GpAnimator::GetAnimatorBatch( void )
{
	return m_pAnimatorBatch;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Retrieve pointer to next animation in batch
**/
//--------------------------------------------------------------------------------------------------

inline	GpAnimator*	GpAnimator::GetNextBatchedAnimator( void )
{
	return m_pNextBatchedAnimator;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Set pointer to owning batch, and next animation in batch
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimator::SetNextBatchedAnimator( GpAnimatorBatch* pAnimatorBatch, GpAnimator* pAnimator )
{
	m_pAnimatorBatch		= pAnimatorBatch;
	m_pNextBatchedAnimator	= pAnimator;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Starts the animator's update process - overload for SingleThreadJoblist 

	@param			updateMode			Determines whether update runs on PPU or SPU (see below)
	@param			pWwsJobListST		WWS Job joblist (user has reponsibility to manage/reset it), 
										mandatory for use in SPU update mode. 
	@param			readyCount			SPURS ready count, either kRequestAllSpus or kRequestOneSpu	

					GpAnimator has two different ways to update:

					- kRunOnPPU (default) : synchronous update on PPU (blocking / slow)
						- Call StartUpdate( kRunOnPPU )
						- WaitForUpdate() will always return instantly if object has been updated on PPU.
					    - IsUpdating() will always return false if object has been updated on PPU.

					- kRunOnSPU : asynchronous on SPU (faster)
						- Call StartUpdate( kRunOnSPU,  pWwsJobList )
						- Stall (PPU side) on  WaitForUpdate( )
						- You can also query completion status using IsUpdating( void )

	@note			You should not attempt to update an animation (on PPU or SPU) while there's still
					a pending SPU update. Job markers are store in the GpAnimator object and will
					be overwritten/lost. 

					You cannot use kRunOnSPU on the PC target. You have no SPUs, so don't be daft.
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimator::StartUpdate( UpdateMode updateMode, SingleThreadJobList* pWwsJobListST, 
										 int readyCount )
{
	FW_ASSERT( ( updateMode == kRunOnPPU ) || ( pWwsJobListST != NULL ) );
	StartUpdateInternal( updateMode, pWwsJobListST, NULL, readyCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Starts the animator's update process - overload for MultiThreadSafeJoblist

	@see			Documentation of GpAnimator::StartUpdate( UpdateMode updateMode, SingleThreadJobList* pWwsJobListST, SingleThreadJobList::ReadyCountValues readyCount  )
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimator::StartUpdate( UpdateMode updateMode, MultiThreadSafeJobList* pWwsJobListMT,
										 int readyCount )
{
	FW_ASSERT( ( updateMode == kRunOnPPU ) || ( pWwsJobListMT != NULL ) );
	StartUpdateInternal( updateMode, NULL, pWwsJobListMT, readyCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Starts the animator's update process - overload for PPU without joblist

	@see			Documentation of GpAnimator::StartUpdate( UpdateMode updateMode, SingleThreadJobList* pWwsJobListST, SingleThreadJobList::ReadyCountValues readyCount  )
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimator::StartUpdate( UpdateMode updateMode )
{
	FW_ASSERT( updateMode == kRunOnPPU );
	StartUpdateInternal( updateMode, NULL, NULL, 8 );
}

#endif // GP_ANIMATOR_INL 
