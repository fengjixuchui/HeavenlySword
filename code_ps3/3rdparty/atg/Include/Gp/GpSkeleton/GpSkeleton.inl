//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Skeletal System Definition : Inlines

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_SKELETON_INL
#define GP_SKELETON_INL


//--------------------------------------------------------------------------------------------------
/**
	@brief		Constructs a GpSkeletonJointDef.
**/
//--------------------------------------------------------------------------------------------------

inline	GpSkeletonJointDef::GpSkeletonJointDef() : 
		m_index( -1 ),
		m_pParent( NULL ),
		m_pNextSibling( NULL ),
		m_pFirstChild( NULL ),
		m_rotation( FwMaths::kIdentity ),
		m_translation( FwMaths::kZero ),
		m_scale( 1.0f, 1.0f, 1.0f, 1.0f )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Adds the specified GpSkeletonJointDef as a child of the current GpSkeletonJointDef object.
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpSkeletonJointDef::AddChild( GpSkeletonJointDef& child )
{
	FW_ASSERT( !child.m_pParent && !child.m_pNextSibling );

	child.m_pParent			= this;
	child.m_pNextSibling	= m_pFirstChild;
	m_pFirstChild			= &child;
} 

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the name of the current object
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpSkeletonJointDef::SetName( FwHashedString name )
{
	m_name = name;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the rotation of the current object
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpSkeletonJointDef::SetRotation( FwQuat_arg rotation )
{
	m_rotation = rotation;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the translation of the current object
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpSkeletonJointDef::SetTranslation( FwPoint_arg translation )
{
	m_translation = translation;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the scale of the current object
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpSkeletonJointDef::SetScale( FwVector4_arg scale )
{
	m_scale = scale;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return a const reference to the transform for the skeleton
**/
//--------------------------------------------------------------------------------------------------

inline	const FwTransform&	GpSkeleton::GetTransform( void ) const
{
	return m_transform;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets the transform for the skeletal, invalidating if necessary
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpSkeleton::SetTransform( const FwTransform& trans, bool invalidate )
{
	FW_ASSERT( !IsLocked() );

	m_transform = trans;
	if ( invalidate )
		Invalidate( kRootJointIndex );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return how many joints are present in this skeleton
**/
//--------------------------------------------------------------------------------------------------

inline	int	GpSkeleton::GetJointCount( void ) const
{
	return m_jointCount;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return a pointer to a GpJoint object for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	GpJoint*	GpSkeleton::GetJoint( int jointIndex ) const
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return &m_pJointArray[ jointIndex ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the local rotation for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		GpSkeleton::GetRotation( int jointIndex ) const
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return m_pJointArray[ jointIndex ].m_rotation;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the local translation for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		GpSkeleton::GetTranslation( int jointIndex ) const
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return m_pJointArray[ jointIndex ].m_translation;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the local scale for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	GpSkeleton::GetScale( int jointIndex ) const
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return m_pJointArray[ jointIndex ].m_scale;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the local bind pose rotation for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	FwQuat		GpSkeleton::GetBindRotation( int jointIndex ) const
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return m_pJointBindPose[ jointIndex ].m_rotation;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the local bind pose translation for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	FwPoint		GpSkeleton::GetBindTranslation( int jointIndex ) const
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return m_pJointBindPose[ jointIndex ].m_translation;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the local bind pose scale for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	FwVector4	GpSkeleton::GetBindScale( int jointIndex ) const
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return m_pJointBindPose[ jointIndex ].m_scale;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the parent joint index for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline int			GpSkeleton::GetParentIndex( int jointIndex ) const
{
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return m_pJointLinkage[ jointIndex ].m_parentIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the first child joint index for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline int			GpSkeleton::GetFirstChildIndex( int jointIndex ) const
{
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return m_pJointLinkage[ jointIndex ].m_firstChildIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the next sibling joint index for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline int			GpSkeleton::GetNextSiblingIndex( int jointIndex ) const
{
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	return m_pJointLinkage[ jointIndex ].m_nextSiblingIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets local rotation for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	void		GpSkeleton::SetRotation( int jointIndex, FwQuat_arg rot )
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	m_pJointArray[ jointIndex ].m_rotation = rot;
	Invalidate( jointIndex );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets local translation for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	void		GpSkeleton::SetTranslation( int jointIndex, FwPoint_arg trans )
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	m_pJointArray[ jointIndex ].m_translation = trans;
	Invalidate( jointIndex );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Sets local scale for the specific joint index
**/
//--------------------------------------------------------------------------------------------------

inline	void		GpSkeleton::SetScale( int jointIndex, FwVector4_arg scale )
{
	FW_ASSERT( !IsLocked() );
	FW_ASSERT( ( jointIndex >= 0 ) && ( jointIndex < m_jointCount ) );
	m_pJointArray[ jointIndex ].m_scale = scale;
	Invalidate( jointIndex );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return a pointer to the skeleton definition structure for this skeleton
**/
//--------------------------------------------------------------------------------------------------

inline const GpSkeletonDef*	GpSkeleton::GetSkeletonDef( void ) const
{
	return m_pSkeletonDef;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return a pointer to the driven keys structure for this skeleton
**/
//--------------------------------------------------------------------------------------------------

inline GpDrivenKeys*	GpSkeleton::GetDrivenKeys( void ) const
{	
	return m_pDrivenKeys;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Return the skeleton key for this skeleton (formed from joint hashes & indices)
**/
//--------------------------------------------------------------------------------------------------

inline u32	GpSkeleton::GetSkeletonKey( void ) const
{
	return m_pSkeletonDef->GetSkeletonKey();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a pointer to world matrix array

	@note			Assumes Synchronise() already called

	@note			unweildy name intentional, to discourage misuse
**/
//--------------------------------------------------------------------------------------------------

inline const FwTransform*	GpSkeleton::GetWorldMatrixArrayUnsafe( void ) const
{
	FW_ASSERT( !IsLocked() );
	return m_pWorldMatrix;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a pointer to the internal flags array

	@internal
**/
//--------------------------------------------------------------------------------------------------

inline u8*	GpSkeleton::GetFlagsArray( void ) const
{
	return m_pJointFlagsArray;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Mark all joints as valid

	@note		This should not be called by user code!
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpSkeleton::MarkAsValid( void )
{
	FW_ASSERT( !IsLocked() );

	for ( int jointLoop = 0; jointLoop < m_jointCount; jointLoop++ )
		m_pJointFlagsArray[ jointLoop ] &= ~GpJoint::kWorldMatrixInvalid;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Lock access to skeleton data
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpSkeleton::Lock( void )
{
	FW_ASSERT( !( m_flags & kIsLocked ) );
	m_flags |= kIsLocked;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Unlock access to skeleton data
**/
//--------------------------------------------------------------------------------------------------

inline	void	GpSkeleton::Unlock( void )
{
	FW_ASSERT( m_flags & kIsLocked );
	m_flags &= ~kIsLocked;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief		Query skeletal lock status
**/
//--------------------------------------------------------------------------------------------------

inline	bool	GpSkeleton::IsLocked( void ) const
{
	return ( m_flags & kIsLocked );
}

#endif // GP_SKELETON_INL
