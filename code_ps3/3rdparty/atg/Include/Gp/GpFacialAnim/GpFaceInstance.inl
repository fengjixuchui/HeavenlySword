//--------------------------------------------------------------------------------------------------
/**
	@file		GpFaceInstance.inl

	@brief		contains the inline function definitions for the GpFaceInstance class

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_FACE_INSTANCE_INL_H
#define GP_FACE_INSTANCE_INL_H

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			calculate the amount of memory required for a face instance object

	@param			hResource	- handle to .face file resource
	@param			pExprSet	- expression set for handle -> blendweight/orients mapping

	@return			size in bytes required to store the instance
**/
//--------------------------------------------------------------------------------------------------

inline int GpFaceInstance::QuerySizeInBytes( const FwResourceHandle& hResource, 
											 GpFaceExprSet* pExprSet )
{
	return QuerySizeInBytes( (const u8*)hResource.GetData(), 
							 (size_t)hResource.GetSize(), 
							 pExprSet );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			instantiates from a loaded .face file, allowing the user to specify when & where
					the memory is allocated

	@param			pFaceFile	- pointer to .face resource file in memory
	@param			fileSize	- size of .face resource file in bytes
	@param			pExprSet	-	expression set for handle -> blendweight/orients mapping
	@param			pMem		-	address at which to construct this instance or NULL

	@return			handle to a GpFaceInstance instance
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceInstanceHandle GpFaceInstance::Create( const FwResourceHandle& hResource,
													GpFaceExprSet* pExprSet,
													void* pMem )
{
	GpFaceInstanceHandle hFace = Create( (const u8*)hResource.GetData(),
										 (size_t)hResource.GetSize(),
										 pExprSet,
										 pMem );
	hFace->m_hResource = hResource;
	return hFace;
}


//--------------------------------------------------------------------------------------------------
/**
	@brief			clamp value to range

		provided here since the expression functions call the MEL builtin equivalent
					
	@param			min - minimum value of range
	@param			max - maximum value of range
	@param			val - value to clamp
**/
//--------------------------------------------------------------------------------------------------

inline float GpFaceInstance::Clamp(float min, float max, float val) const
{
	if (val < min)	return min;
	if (val > max)	return max;
	return val;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			look up the name of a rig handle using it's index

		this provides the mapping between rig handle names & their indices
**/
//--------------------------------------------------------------------------------------------------

inline const FwHashedString	GpFaceInstance::GetRigHandleName( u32 handleIndex ) const
{
	return m_pRigHandleNames[ handleIndex ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return pointer to the expression set

		the expressions in the set map from rig handles to blendweights, joint orientations
		and shader parameters
**/
//--------------------------------------------------------------------------------------------------

inline const GpFaceExprSet* GpFaceInstance::GetExprSet() const
{
	return m_pRigMapping;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get the number of rig handles
**/
//--------------------------------------------------------------------------------------------------

inline u16 GpFaceInstance::GetNumRigHandles() const
{
	return m_numRigHandles;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get a reference to the rig handle from it's index

	@param			handleIndex - index of handle
**/
//--------------------------------------------------------------------------------------------------

inline FwVector4& GpFaceInstance::GetHandle( u32 handleIndex ) const
{
	FW_ASSERT( handleIndex < m_numRigHandles );
	return m_pRigHandleValues[ handleIndex ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set the current value of the rig handle

	@param			handleIndex - index of handle
	@param			value - new value of the rig handle
**/
//--------------------------------------------------------------------------------------------------

inline void GpFaceInstance::SetHandle( u32 handleIndex, FwVector4_arg value ) const
{
	FW_ASSERT( handleIndex < m_numRigHandles );
	m_pRigHandleValues[ handleIndex ] = value;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			reset the rig handles to their default values
**/
//--------------------------------------------------------------------------------------------------

inline void GpFaceInstance::SetHandlesToDefaults() const
{
	for (int i=0; i<m_numRigHandles; i++)
		m_pRigHandleValues[i] = m_pRigHandleDefaults[i];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get default value of a particular rig handle

	@param			handleIndex - index of rig handle
**/
//--------------------------------------------------------------------------------------------------

inline const FwVector4& GpFaceInstance::GetHandleDefault( u32 handleIndex ) const
{
	FW_ASSERT( handleIndex < m_numRigHandles );
	return m_pRigHandleDefaults[ handleIndex ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get the index of the specified joint angle argument

	@param			jointIndex - joint angle argument index

	@return 		index of joint relative to the skeleton it is a member of
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceInstance::GetJointAngleIndex( u32 jointIndex ) const
{
	FW_ASSERT( m_pRigMapping );
	FW_ASSERT( jointIndex < m_pRigMapping->GetNumJointAngleArgs() );
	return m_pRigJointAngleIndices[ jointIndex ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get the index of the specified joint trans argument

	@param			jointIndex - joint angle argument index

	@return 		index of joint relative to the skeleton it is a member of
**/
//--------------------------------------------------------------------------------------------------

inline s32 GpFaceInstance::GetJointTransIndex( u32 jointIndex ) const
{
	FW_ASSERT( m_pRigMapping );
	FW_ASSERT( jointIndex < m_pRigMapping->GetNumJointTransArgs() );
	return m_pRigJointTransIndices[ jointIndex ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			fetch an entry from the Euler angle cache
												
	@param			entryIndex - entry index in the cache

	@return 		a 4 vector with the corresponding angles
**/
//--------------------------------------------------------------------------------------------------

inline FwVector4 GpFaceInstance::GetEulerCacheEntry( u32 entryIndex ) const
{
	FW_ASSERT( m_pRigMapping );
	FW_ASSERT( entryIndex < m_pRigMapping->GetNumJointAngleArgs() );
	return m_pRigEulerCache[ entryIndex ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			fetch an entry from the translate cache
												
	@param			entryIndex - entry index in the cache

	@return 		a 4 vector with the corresponding translation
**/
//--------------------------------------------------------------------------------------------------

inline FwVector4 GpFaceInstance::GetTransCacheEntry( u32 entryIndex ) const
{
	FW_ASSERT( m_pRigMapping );
	FW_ASSERT( entryIndex < m_pRigMapping->GetNumJointTransArgs() );
	return m_pRigTransCache[ entryIndex ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return true if this joint has a joint orientation defined, false otherwise
												
	@param			jointIndex - index of joint
**/
//--------------------------------------------------------------------------------------------------

inline bool GpFaceInstance::JointAngleHasOrient( u32 jointIndex ) const
{
	FW_ASSERT( m_pRigMapping );
	return m_pRigMapping->JointAngleHasOrient( jointIndex );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return true if this joint has a rotate axis defined, false otherwise
												
	@param			jointIndex - index of joint
**/
//--------------------------------------------------------------------------------------------------

inline bool GpFaceInstance::JointAngleHasRotAxis( u32 jointIndex ) const
{
	FW_ASSERT( m_pRigMapping );
	return m_pRigMapping->JointAngleHasRotAxis( jointIndex );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return a joint's orientation as a quaternion
												
	@param			jointIndex - index of joint

	@note			assumes the joint has a valid joint orientation
**/
//--------------------------------------------------------------------------------------------------

inline FwQuat GpFaceInstance::GetJointAngleOrient( u32 jointIndex ) const
{
	FW_ASSERT( m_pRigMapping );
	return m_pRigJointOrients[ m_pRigMapping->GetJointAngleOrientIndex( jointIndex ) ];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return a joint's rotate axis as a quaternion
												
	@param			jointIndex - index of joint

	@note			assumes the joint has a valid rotate axis
**/
//--------------------------------------------------------------------------------------------------

inline FwQuat GpFaceInstance::GetJointAngleRotAxis( u32 jointIndex ) const
{
	FW_ASSERT( m_pRigMapping );
	return m_pRigJointOrients[ m_pRigMapping->GetJointAngleRotAxisIndex( jointIndex ) ];
}

//--------------------------------------------------------------------------------------------------

#endif // GP_FACE_INSTANCE_INL_H

