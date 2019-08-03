//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2006. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_ANIMATORBATCH_H
#define	GP_ANIMATORBATCH_H

#include <Gp/GpAnimator/GpAnimator.h>


//--------------------------------------------------------------------------------------------------
/**
	@class			GpAnimatorBatch
	
	@brief			A class used to group the update of several GpAnimator objects, in such a way as
					to maximise efficiency from the SPU job manager. For PC/PPU there may be a small
					instruction cache benefit, but probably nothing major.			
**/
//--------------------------------------------------------------------------------------------------

class	GpAnimatorBatch
{
public:	
	GpAnimatorBatch();
	~GpAnimatorBatch();
	
	// Batching control
	void	StartBatch( void );
	void	AddAnimator( GpAnimator* pAnimator );
	void	EndBatch( void );
	
	// Update start
	void	StartUpdate( GpAnimator::UpdateMode updateMode ); 
	void	StartUpdate( GpAnimator::UpdateMode updateMode, SingleThreadJobList* pWwsJobListST, int readyCount = 8 );
	void	StartUpdate( GpAnimator::UpdateMode updateMode, MultiThreadSafeJobList* pWwsJobListMT, int readyCount = 8 );

	// Update query
	void	WaitForUpdate( void );
	bool	IsUpdating( void ) const;

private:
	// Internal batch reset
	void	ResetBatch( void );

	// Interal update code ( shared between multithreaded and single threaded joblists ) 
	void	StartUpdateInternal( GpAnimator::UpdateMode updateMode, SingleThreadJobList* pWwsJobListST, MultiThreadSafeJobList* pWwsJobListMT, int readyCount );

	// Locking
	void	Lock( void );
	void	Unlock( void );
	bool	IsLocked( void ) const;

	enum	Flags
	{
		kIsLocked			= ( 1 << 0 ),
		kIsDefiningBatch	= ( 1 << 1 ),
	};

	GpAnimator*				m_pFirstAnimator;
	GpAnimator*				m_pLastAnimator;
	u32						m_flags;
};

// -------------------------------------------------------------------------------------------------

inline	GpAnimatorBatch::GpAnimatorBatch()
{
	m_pFirstAnimator	= NULL;
	m_pLastAnimator		= NULL;
	m_flags				= 0;
}

inline	GpAnimatorBatch::~GpAnimatorBatch()
{
	FW_ASSERT( !IsLocked() );
	ResetBatch();
}

inline 	void	GpAnimatorBatch::Lock( void )
{
	FW_ASSERT( !IsLocked() );
	m_flags |= kIsLocked;
}

inline	void	GpAnimatorBatch::Unlock( void )
{
	FW_ASSERT( IsLocked() );
	m_flags &= ~kIsLocked;
}

inline	bool	GpAnimatorBatch::IsLocked( void ) const
{
	return ( m_flags & kIsLocked );
}

inline	void	GpAnimatorBatch::StartBatch( void )
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( !( m_flags & kIsDefiningBatch ) );

	// Remove any other associations..
	ResetBatch();

	// Now mark that we're defining a batch
	m_flags |= kIsDefiningBatch;
}

inline	void	GpAnimatorBatch::AddAnimator( GpAnimator* pAnimator )
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( m_flags & kIsDefiningBatch );

	// If this is our first animator, make a note of the address..
	if ( m_pFirstAnimator == NULL )
		m_pFirstAnimator = pAnimator;
	
	// If we have a prior animator in the batch, link this new animator up to it..
	if ( m_pLastAnimator )
		m_pLastAnimator->SetNextBatchedAnimator( this, pAnimator );
	
	m_pLastAnimator = pAnimator;
}

inline	void	GpAnimatorBatch::EndBatch( void )
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( m_flags & kIsDefiningBatch );
	m_flags &= ~kIsDefiningBatch;
}


//--------------------------------------------------------------------------------------------------
/**
	@brief			Starts the update of a batch of animations. (overload for SingleThreadJobList)
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimatorBatch::StartUpdate(	GpAnimator::UpdateMode updateMode, 
												SingleThreadJobList* pWwsJobListST, 
												int readyCount )
{
	FW_ASSERT( ( updateMode == GpAnimator::kRunOnPPU ) || ( pWwsJobListST != NULL ) );
	StartUpdateInternal( updateMode, pWwsJobListST, NULL, readyCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Starts the update of a batch of animations. (overload for MultiThreadSafeJobList)

	@see			GpAnimatorBatch::StartUpdate( GpAnimator::UpdateMode updateMode, MultiThreadSafeJobList* pWwsJobListMT )
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimatorBatch::StartUpdate( GpAnimator::UpdateMode updateMode, 
											  MultiThreadSafeJobList* pWwsJobListMT,
											  int readyCount )
{
	FW_ASSERT( ( updateMode == GpAnimator::kRunOnPPU ) || ( pWwsJobListMT != NULL ) );
	StartUpdateInternal( updateMode, NULL, pWwsJobListMT, readyCount );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Starts the update of a batch of animations. (overload for PPU, without joblist)
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpAnimatorBatch::StartUpdate(	GpAnimator::UpdateMode updateMode )
{
	FW_ASSERT( updateMode == GpAnimator::kRunOnPPU );
	StartUpdateInternal( updateMode, NULL, NULL, 8 );
}

#endif	// GP_ANIMATORBATCH_H
