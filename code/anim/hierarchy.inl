/***************************************************************************************************
*
*	DESCRIPTION		Inline function bodies for CHierarchy.
*
*	NOTES			This file should only be included from within hierarchy.h.
*
***************************************************************************************************/

#ifndef HIERARCHY_H_
#	error You must only include this file from within hierarchy.h
#endif	// !HIERARCHY_H_

inline const GpJoint *CHierarchy::GetBindPoseJoint( int32_t idx ) const
{
	ntError_p( m_pJointBindPose!= NULL, ("We have no bind-pose joint-array - not good.") );
	ntError_p( idx >= 0 && idx < m_iTransformCount, ("bind-pose joint index out of bounds.") );
	return m_pJointBindPose + idx;
}

inline CQuat CHierarchy::GetBindPoseJointRotation( int32_t idx ) const
{
	ntError_p( m_pJointBindPose != NULL, ("We have no bind-pose joint-array - not good.") );
	ntError_p( idx >= 0 && idx < m_iTransformCount, ("bind-pose joint index out of bounds.") );
	return CQuat( m_pJointBindPose[ idx ].m_rotation.QuadwordValue() );
}

inline CPoint CHierarchy::GetBindPoseJointTranslation( int32_t idx ) const
{
	ntError_p( m_pJointBindPose != NULL, ("We have no bind-pose joint-array - not good.") );
	ntError_p( idx >= 0 && idx < m_iTransformCount, ("bind-pose joint index out of bounds.") );
	return CPoint( m_pJointBindPose[ idx ].m_translation.QuadwordValue() );
}

inline CVector CHierarchy::GetBindPoseJointScale( int32_t idx ) const
{
	ntError_p( m_pJointBindPose != NULL, ("We have no bind-pose joint-array - not good.") );
	ntError_p( idx >= 0 && idx < m_iTransformCount, ("bind-pose joint index out of bounds.") );
	return CVector( m_pJointBindPose[ idx ].m_scale.QuadwordValue() );
}

inline const CPoint &CHierarchy::GetBoneOffsetForJoint( int32_t idx ) const
{
	ntError_p( m_BoneOffsetArray != NULL, ("We have no bind-pose bone-offset-array - not good.") );
	ntError_p( idx >= 0 && idx < m_iTransformCount, ("bind-pose joint index out of bounds.") );
	return m_BoneOffsetArray[ idx ];
}

inline const CClumpHeader *CHierarchy::GetClumpHeader() const
{
	ntAssert( m_pobClumpHeader );
	return m_pobClumpHeader;
}

inline int32_t CHierarchy::GetCharacterBoneTransformIndex( CHARACTER_BONE_ID eBoneID ) const
{
	ntAssert( GetCharacterBoneToIndexArray() );
	return	(int32_t)GetCharacterBoneToIndexArray()[ eBoneID ];
}

inline bool CHierarchy::DoesTransformExist( const CHashedString &obNameHash ) const
{
	return GetTransformIndex( obNameHash ) >= 0;
}

inline Transform *CHierarchy::GetTransform( int32_t iTransform ) const
{
	ntAssert( ( iTransform >= 0 ) && ( iTransform <= m_iTransformCount ) );
	return &m_pobTransformArray[ iTransform ];
}

inline Transform *CHierarchy::GetTransform( const CHashedString &obNameHash ) const
{
	int32_t iTransform = GetTransformIndex( obNameHash );
	ntAssert( iTransform >= 0 );
	return &m_pobTransformArray[ iTransform ];
}

inline Transform *CHierarchy::GetTransformFromHash( uint32_t uiHash ) 
{
	int32_t iTransform = GetTransformIndexFromHash( uiHash );
	ntAssert( iTransform >= 0 );
	return &m_pobTransformArray[ iTransform ];
}

inline bool CHierarchy::IsEmbeddedTransform( const Transform *pobTransform ) const
{
	int32_t	iOffset = int32_t( pobTransform - m_pobTransformArray );
	
	if ( ( iOffset >= 0 ) && ( iOffset < m_iTransformCount ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

inline int32_t CHierarchy::GetTransformIndex( const Transform *transform ) const
{
	ntError_p( IsEmbeddedTransform( transform ), ("You can only call this on an embedding transform of this hierarchy.") );
	return int32_t( transform - m_pobTransformArray );
}

inline void CHierarchy::Invalidate( bool bIsExternal )
{
	if ( bIsExternal )
	{
		SetFlagBits( HIERF_INVALIDATED_EXTERNAL | HIERF_MAY_REQUIRE_SKIN_MATRIX_UPDATE );
	}
	else
	{
		SetFlagBits( HIERF_INVALIDATED_MAIN | HIERF_MAY_REQUIRE_SKIN_MATRIX_UPDATE );
	}
}

//
//	Interface for Transform objects ONLY.
//
inline CMatrix CHierarchy::GetLocalMatrix( int32_t idx ) const
{
	ntError( !( m_pobTransformArray[ idx ].GetFlags() & TRANSF_IS_EXTERNAL ) );
	ntError_p( idx >= 0 && idx < m_iTransformCount, ("Invalid transform index. Out of bounds.") );

	return CMatrix(	CQuat( m_pJointArray[ idx ].m_rotation.QuadwordValue() ),
					CPoint( m_pJointArray[ idx ].m_translation.QuadwordValue() ) );
}

inline CPoint CHierarchy::GetLocalTranslation( int32_t idx ) const
{
	ntError( !( m_pobTransformArray[ idx ].GetFlags() & TRANSF_IS_EXTERNAL ) );
	ntError_p( idx >= 0 && idx < m_iTransformCount, ("Invalid transform index. Out of bounds.") );

	return CPoint( m_pJointArray[ idx ].m_translation.QuadwordValue() );
}

inline CQuat CHierarchy::GetLocalRotation( int32_t idx ) const
{
	ntError( !( m_pobTransformArray[ idx ].GetFlags() & TRANSF_IS_EXTERNAL ) );
	ntError_p( idx >= 0 && idx < m_iTransformCount, ("Invalid transform index. Out of bounds.") );

	return CQuat( m_pJointArray[ idx ].m_rotation.QuadwordValue() );
}

inline const CMatrix &CHierarchy::GetWorldMatrixFast( int32_t idx ) const
{
	ntError_p( idx >= 0 && idx < m_iTransformCount, ("Invalid transform index. Out of bounds.") );
	return *reinterpret_cast< CMatrix * >( m_pWorldMatrix + idx );
}















