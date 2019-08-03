/***************************************************************************************************
*
*	$Header:: /game/transform.cpp 21    14/07/03 17:20 Dean                                        $
*
*	Transform Processing
*
*	CHANGES
*
*	18/2/2003	Dean	Created
*
***************************************************************************************************/

#include "anim/transform.h"
#include "anim/hierarchy.h"



/*
static void CheckMatrix( const CMatrix &matrix )
{
	CDirection x = matrix.GetXAxis();
	CDirection y = matrix.GetYAxis();
	CDirection z = matrix.GetZAxis();

	ntError_p( fabsf( x.Dot( y ) ) < 0.05f, ("%f", x.Dot( y )) );
	ntError_p( fabsf( y.Dot( z ) ) < 0.05f, ("%f", y.Dot( z )) );
	ntError_p( fabsf( z.Dot( x ) ) < 0.05f, ("%f", z.Dot( x )) );

	ntError_p( fabsf( x.Length() - 1.0f ) < 0.1f, ("%f", x.Length()) );
	ntError_p( fabsf( y.Length() - 1.0f ) < 0.1f, ("%f", y.Length()) );
	ntError_p( fabsf( z.Length() - 1.0f ) < 0.1f, ("%f", z.Length()) );

	ntError( matrix.GetTranslation().Length() < 1000000.0f );
}
*/



/***************************************************************************************************
*
*	FUNCTION		Transform Constructor
*
*	DESCRIPTION		Initialises a external transform object.
*
***************************************************************************************************/
Transform::Transform	()
:	m_obWorldRotation	( CONSTRUCT_IDENTITY )
,	m_TransformData		( NT_NEW_CHUNK( Mem::MC_ANIMATION ) NonHierarchyTransformData )
,	m_pobParent			( NULL )
,	m_pobNextSibling	( NULL )
,	m_pobFirstChild		( NULL )
,	m_pobParentHierarchy( NULL )
,	m_pobExternalData	( NULL )
,	m_HierarchyIndex	( InvalidIndex )
{
	ntError( m_TransformData != NULL );
	m_TransformData->m_iFlags = TRANSF_IS_EXTERNAL | TRANSF_WORLD_MATRIX_INVALID | TRANSF_WORLD_ROTATION_INVALID;
	m_TransformData->m_LocalSpace.m_rotation = FwQuat( FwMaths::kIdentity );
	m_TransformData->m_LocalSpace.m_scale = FwVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_TransformData->m_LocalSpace.m_translation = FwPoint( FwMaths::kZero );
	m_TransformData->m_obWorldMatrix.SetIdentity();
}

/***************************************************************************************************
*
*	FUNCTION		Transform dtor.
*
***************************************************************************************************/
Transform::~Transform()
{
	//if( m_TransformData && (m_TransformData->m_iFlags & TRANSF_IS_EXTERNAL) )
	{
		ntError_p( !GetParent(), ("Transform being destroyed is still in a hierarchy\n") );
		ntError_p( !m_pobFirstChild, ("Transform being destroyed still has children\n"));
	}
	NT_DELETE_CHUNK( Mem::MC_ANIMATION, m_TransformData );
	m_TransformData = NULL;
}

/***************************************************************************************************
*
*	FUNCTION		Transform::AddChild
*
*	DESCRIPTION		Adds the specified transform as a child of the current Transform object. 
*
*	INPUTS			pobChild			-	A pointer to the proposed child transform.
*
*	NOTES			If the proposed child already has a parent, then we fail. We don't support any
*					kind of automatic adoption of child nodes.
*
***************************************************************************************************/
void Transform::AddChild( Transform *pobChild )
{
	// Check that we're not already a child
	ntAssert( !pobChild->m_pobParent && !pobChild->m_pobNextSibling );
	ntAssert( pobChild != this );		// DOH!

	// Set the child parent pointer to point to 'this' and connect it up as the new first child
	// (This is still fine when the current first child is NULL)
	pobChild->m_pobParent = this;
	pobChild->m_pobNextSibling = m_pobFirstChild;
	m_pobFirstChild = pobChild;

	// Hierarchy connection processing..
	if ( pobChild->GetFlags() & TRANSF_IS_EXTERNAL )
	{
		// When we want to prevent external transforms from being added to the world, uncomment
		// the following assertion:
		//ntAssert( !( GetParentHierarchy()->GetRootTransform()->GetFlags() & TRANSF_IS_WORLD_ROOT ) );

		// If the child is externally created, then we need to make sure that the transform
		// we're attaching it to has a hierarchy, and that the transform doesn't already have a parent
		// hierarchy set on it..
		ntAssert( GetParentHierarchy() );
		ntAssert( !pobChild->GetParentHierarchy() );
		pobChild->m_pobParentHierarchy = GetParentHierarchy();
	}
	else if	(
			( !pobChild->GetParentHierarchy()->GetParent() ) &&
			( pobChild->GetParentHierarchy() != GetParentHierarchy() )
			)
	{
		// If it's a transform that's a part of a hierarchy (ie not external), then we need to connect
		// hierarchies up too.. there has to be one, or things must have gone bad..
		ntAssert( GetParentHierarchy() );
		ntAssert( pobChild->GetParentHierarchy() );
		GetParentHierarchy()->AddChild( pobChild->GetParentHierarchy() );
	}

	// Now we're added, we need to invalidate..
	pobChild->Invalidate();
}


/***************************************************************************************************
*
*	FUNCTION		Transform::RemoveFromParent
*
*	DESCRIPTION		Removes this transform from its parent. 
*
***************************************************************************************************/
void Transform::RemoveFromParent()
{
	// Check that we have a parent transform and hierarchy
	ntAssert( GetParent() );
	ntAssert( GetParentHierarchy() );

	Transform* pobTransform	 = m_pobParent->m_pobFirstChild;
	Transform* pobPrevTransform = NULL;
		
	while ( pobTransform != this )
	{
		// If we didn't find ourselves in our parent's list of transforms, bail..
		ntAssert( pobTransform != NULL );

		pobPrevTransform = pobTransform;
		pobTransform = pobTransform->m_pobNextSibling;		
	}
	
	// If we were chained to by another sibling, point it to any other siblings we might have been
	// before. If we had no prior siblings, then we must be setting the first of our siblings (if
	// any are defined) to be the first child of the parent.
	if ( pobPrevTransform )
		pobPrevTransform->m_pobNextSibling = m_pobNextSibling;
	else
		m_pobParent->m_pobFirstChild = m_pobNextSibling;

	// Finally clear our parent and sibling pointer
	m_pobParent = NULL;
	m_pobNextSibling = NULL;

	// We need to clear the our associated hierarchy's GpSkeleton-root transform.
	m_pobParentHierarchy->ResetGpSkeletonRoot( true );

	// If the transform being removed is external, clear the parent hierarchy pointer. If this is
	// not the case, then the transform must be the linkage point between two hierarchies, so we 
	// need to separate them.
	if ( GetFlags() & TRANSF_IS_EXTERNAL )
		m_pobParentHierarchy = NULL;
	else if (GetParentHierarchy()->GetParent())
		GetParentHierarchy()->RemoveFromParent();

	// Now we've been removed, we need to invalidate
	Invalidate();
}

/***************************************************************************************************
*
*	FUNCTION		Transform::Invalidate
*
*	DESCRIPTION		When the local matrix for a transform has been modified by application code, this
*					function must be called in order to ensure that all child transforms are marked
*					as dirty.
*
*	NOTES			It might be the case that for all invalidated transforms we forcibly mark all
*					parent hierarchies as invalid too..
*
*					Also note that this function recurses.
*
***************************************************************************************************/
void Transform::Invalidate() const
{
	SetFlagBits( TRANSF_WORLD_MATRIX_INVALID | TRANSF_WORLD_ROTATION_INVALID );

	// Invalidate the hierarchy if present. We need to handle cases where transforms have no hierarchies,
	// or we wouldn't be able to set matrices on transforms prior to attaching them to a hierarchy.
	if ( m_pobParentHierarchy )
		m_pobParentHierarchy->Invalidate( ( GetFlags() & TRANSF_IS_EXTERNAL ) != 0 );

	for ( Transform* pobChild = m_pobFirstChild; pobChild != NULL; pobChild = pobChild->m_pobNextSibling )
	{
		// As soon as we find a child that's invalid, we can stop recursing down.. 
		if ( pobChild->IsWorldMatrixValid() )
			pobChild->Invalidate();
	}
}

/***************************************************************************************************
*
*	FUNCTION		Transform::Resynchronise
*
*	DESCRIPTION		This function looks in a transform hierarchy for a transform that is marked as
*					dirty. If one is found, then it forces a resynchronisation of the world matrices
*					in all children (see Transform::ForceResynchronise() for more details).
*
*	NOTES			This function recurses..
*
***************************************************************************************************/
void Transform::Resynchronise()
{
	if ( IsWorldMatrixValid() )
	{
		// The transform is valid, so recurse through the children..
		for ( Transform* pobChild = m_pobFirstChild; pobChild != NULL; pobChild = pobChild->m_pobNextSibling )
		{
			pobChild->Resynchronise();
		}
	}
	else
	{
		// We've found a transform that's invalid.. time to fixup *all* dirty child transforms.
		ForceResynchronise();
	}
}

/***************************************************************************************************
*
*	FUNCTION		Transform::ForceResynchronise
*
*	DESCRIPTION		When Transform::Resynchronise() finds a transform that is dirty, this function
*					is called to ensure that the transform and all associated children have a valid
*					world matrix. This processing includes extending into externally created transforms,
*					or to transforms in other hierarchies) 
*
*	NOTES			This function recurses..
*
***************************************************************************************************/
void Transform::ForceResynchronise()
{
	ntAssert( GetParent() != NULL );
	ntAssert( GetParent()->IsWorldMatrixValid() );
//	ntAssert( !IsWorldMatrixValid() );					// I've commented this out as Frank's hair stuff updates Transforms
														// from SPU without resetting any flags so in some cases the world-
														// matrix is invalid but marked as valid. I hate you Frank. No. Really.

	SetWorldMatrixDirectly( GetLocalMatrix() * GetParent()->GetWorldMatrix() );	// Make sure we write it back to the transform obj

	ClearFlagBits( TRANSF_WORLD_MATRIX_INVALID );							// ...and then mark the transform as valid.

	for ( Transform* pobChild = m_pobFirstChild; pobChild != NULL; pobChild = pobChild->m_pobNextSibling )
	{
		pobChild->ForceResynchronise();
	}
}


/***************************************************************************************************
*
*	FUNCTION		Transform::GetWorldMatrix
*
*	DESCRIPTION		This function returns the world matrix for the current transform. If the 
*					transform is classified as invalid, then all parents up to either the root or
*					the first non-dirty transform are stored in a static array, which is then
*					bounced through recalculating world matrices for all of the parent nodes. Those
*					transforms are then marked as valid.
*
*	NOTES			This is likely to be a prime candidate for inline ASM optimisation later in the
*					project.
*
***************************************************************************************************/
const CMatrix &Transform::GetWorldMatrix() const
{
	// If the matrix is invalid, recalculate it..
	if ( GetFlags() & TRANSF_WORLD_MATRIX_INVALID )
	{
		int							iLevel	= 0;
		const Transform*			pobCurrent = this;
		static const Transform*		apobTransform[ iMAX_TRANSFORM_DEPTH ];
		CMatrix						obAccMatrix;
		
		// Build an array of all dirty parents to this frame..
		do
		{
			ntAssert( iLevel < iMAX_TRANSFORM_DEPTH );
			apobTransform[ iLevel ] = pobCurrent;
			pobCurrent = pobCurrent->GetParent();
			iLevel++;
		} while ( ( pobCurrent ) && ( GetFlags() & TRANSF_WORLD_MATRIX_INVALID ) );

		// Make sure we're pointing to the last Transform that was added..
		iLevel--;

		// If pobCurrent is now NULL, then we're at the top of the chain.. so, set our current accumulated
		// matrix to identity. If we're mid-way through a hierarchy of transforms, take the current (valid)
		// world transformation as a starting point..

		if ( !pobCurrent )
			obAccMatrix.SetIdentity();
		else
			obAccMatrix = pobCurrent->GetWorldMatrixFast();

		// Ok.. now loop through all the dirty transforms in top-down order, and fix them up..
		do
		{
			obAccMatrix = apobTransform[ iLevel ]->GetLocalMatrix() * obAccMatrix;	// Create a world matrix for this transform
			apobTransform[ iLevel ]->SetWorldMatrixDirectly( obAccMatrix );			// Make sure we write it back to the transform object

			apobTransform[ iLevel ]->ClearFlagBits( TRANSF_WORLD_MATRIX_INVALID );	// ...and then mark the transform as valid.
		}
		while ( ( --iLevel ) >= 0 );

	}

	// Paranoia...
	ntError( !( GetFlags() & TRANSF_WORLD_MATRIX_INVALID ) );

	// Return the matrix to the caller
	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		ntError_p( GetFlags() & TRANSF_IS_EXTERNAL, ("Transforms with invalid hierarchy-indices must be external.") );
		return m_TransformData->m_obWorldMatrix;
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( m_pobParentHierarchy != NULL, ("Transforms with a parent hierarchy-index must have a parent hierarchy to index into.") );
		return *reinterpret_cast< CMatrix * >( m_pobParentHierarchy->m_pWorldMatrix + m_HierarchyIndex );
	}
}

/***************************************************************************************************
*
*	FUNCTION		Transform::GetLocalMatrix
*
*	DESCRIPTION		Returns the local matrix associated with this transform.
*
***************************************************************************************************/
CMatrix Transform::GetLocalMatrix() const
{
	ntError_p( m_HierarchyIndex == InvalidIndex || m_pobParentHierarchy != NULL, ("If we have a valid hierarchy index then we must have a hierarchy to index into.") );
	return	m_HierarchyIndex == InvalidIndex ?
				CMatrix(	CQuat( m_TransformData->m_LocalSpace.m_rotation.QuadwordValue() ),
							CPoint( m_TransformData->m_LocalSpace.m_translation.QuadwordValue() ) ) :
				m_pobParentHierarchy->GetLocalMatrix( m_HierarchyIndex );
}

/***************************************************************************************************
*
*	FUNCTION		Transform::GetLocalTranslation
*
*	DESCRIPTION		Returns the local translation associated with this transform.
*
***************************************************************************************************/
CPoint Transform::GetLocalTranslation() const
{
	ntError_p( m_HierarchyIndex == InvalidIndex || m_pobParentHierarchy != NULL, ("If we have a valid hierarchy index then we must have a hierarchy to index into.") );
	return	m_HierarchyIndex == InvalidIndex ?
				CPoint( m_TransformData->m_LocalSpace.m_translation.QuadwordValue() ) :
				m_pobParentHierarchy->GetLocalTranslation( m_HierarchyIndex );
}

/***************************************************************************************************
*
*	FUNCTION		Transform::GetLocalTranslation
*
*	DESCRIPTION		Returns the local translation associated with this transform.
*
***************************************************************************************************/
CQuat Transform::GetLocalRotation() const
{
	ntError_p( m_HierarchyIndex == InvalidIndex || m_pobParentHierarchy != NULL, ("If we have a valid hierarchy index then we must have a hierarchy to index into.") );
	return	m_HierarchyIndex == InvalidIndex ?
				CQuat( m_TransformData->m_LocalSpace.m_rotation.QuadwordValue() ) :
				m_pobParentHierarchy->GetLocalRotation( m_HierarchyIndex );
}

/***************************************************************************************************
*
*	FUNCTION		Transform::GetLocalSpace
*
*	DESCRIPTION		Returns the local space associated with this transform.
*
***************************************************************************************************/
const GpJoint &Transform::GetLocalSpace() const
{
	ntError_p( m_HierarchyIndex == InvalidIndex || m_pobParentHierarchy != NULL, ("If we have a valid hierarchy index then we must have a hierarchy to index into.") );
	return	m_HierarchyIndex == InvalidIndex ?
				m_TransformData->m_LocalSpace :
				m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ];
}

/***************************************************************************************************
*
*	FUNCTION		Transform::GetWorldMatrixFast
*
*	DESCRIPTION		Returns the world-matrix associated with this transform without
*					validity checking.
*
***************************************************************************************************/
const CMatrix &Transform::GetWorldMatrixFast() const
{
	ntError_p( m_HierarchyIndex == InvalidIndex || m_pobParentHierarchy != NULL, ("If we have a valid hierarchy index then we must have a hierarchy to index into.") );
	ntAssert( !( GetFlags() & TRANSF_WORLD_MATRIX_INVALID ) );
	return m_HierarchyIndex == InvalidIndex ? m_TransformData->m_obWorldMatrix : m_pobParentHierarchy->GetWorldMatrixFast( m_HierarchyIndex );
}

/***************************************************************************************************
*
*	FUNCTION		Transform::GetWorldRotation
*
*	DESCRIPTION		Returns the world-space rotation associated with this transform. This value
*					is cached and recalculated as necessary.	
*
*	NOTES			If the transform is classified as invalid, then a recalculation of the world
*					matrix must occur prior to the calculation of the quaternion rotation. So, bear
*					in mind that this could take a while if you're requesting a world rotation for
*					a transform that's at the bottom of a deep and invalid transform hierarchy.
*
***************************************************************************************************/
CQuat Transform::GetWorldRotation() const
{
	// If the rotation is invalid, recalculate it
	if ( GetFlags() & TRANSF_WORLD_ROTATION_INVALID )
	{
		m_obWorldRotation = CQuat( GetWorldMatrix() );
		ClearFlagBits( TRANSF_WORLD_ROTATION_INVALID );
	}
		
	// Return the rotation to the caller
	return m_obWorldRotation;
}

/***************************************************************************************************
*
*	FUNCTION		Transform::SetWorldMatrixDirectly
*
*	DESCRIPTION		Sets the world matrix of the transform without altering other matrices around.
*
***************************************************************************************************/
void Transform::SetWorldMatrixDirectly( const CMatrix &matrix ) const
{
	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		m_TransformData->m_obWorldMatrix = matrix;
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( m_pobParentHierarchy != NULL, ("An internal transform must always have a valid pointer to its parent hierarchy.") );
		ntError_p( !( GetFlags() & TRANSF_IS_EXTERNAL ), ("A transform with a valid hierarchy-index should never be external.") );
		ntError( m_HierarchyIndex >= 0 && m_HierarchyIndex < m_pobParentHierarchy->m_iTransformCount );
		*reinterpret_cast< CMatrix * >( m_pobParentHierarchy->m_pWorldMatrix + m_HierarchyIndex ) = matrix;
	}
}

/***************************************************************************************************
*
*	FUNCTION		Transform::SetLocalMatrixFromWorldMatrix
*
*	DESCRIPTION		This function sets the transforms local matrix such that the resultant world
*					matrix matches the parameter matrix.
*
*	INPUTS			obWorldMatrix		-		A reference to a CMatrix that holds the target world
*												matrix.
*
*	NOTES			Although this looks like a small (and quick) function, you really should be
*					aware that GetAffineInverse() is a relatively large function. And, if necessary,
*					this function may recurse upwards in order to get a valid world matrix from the
*					parent transform. So, don't go calling this hundreds of times a frame, please..
*
***************************************************************************************************/
void Transform::SetLocalMatrixFromWorldMatrix( const CMatrix &obWorldMatrix )
{
	// If we have no parent, or our parent is the world, then we can just set the local matrix directly
	if ( ( !GetParent() ) || ( GetParent()->GetFlags() & TRANSF_IS_WORLD_ROOT ) )
	{
		SetLocalMatrix( obWorldMatrix );
	}
	else
	{
		SetLocalMatrix( obWorldMatrix * GetParent()->GetWorldMatrix().GetAffineInverse() );	
	}
}

/***************************************************************************************************
*
*	FUNCTION		Transform::SetLocalMatrix
*
*	DESCRIPTION		Set the local matrix to the given matrix.
*
***************************************************************************************************/
void Transform::SetLocalMatrix( const CMatrix &obMatrix )
{
	/*
	ntAssert(obMatrix.GetTranslation().Length()<1000.0f);
	obMatrix.CheckAffineTransform();
	ntAssert(fabsf(obMatrix.GetXAxis().Length()-1.0f)<EPSILON);
	ntAssert(fabsf(obMatrix.GetYAxis().Length()-1.0f)<EPSILON);
	ntAssert(fabsf(obMatrix.GetZAxis().Length()-1.0f)<EPSILON);
	*/

//	CheckMatrix( obMatrix );

	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		ntError_p( GetFlags() & TRANSF_IS_EXTERNAL, ("Transforms with invalid hierarchy-indices must be external.") );
		m_TransformData->m_LocalSpace.m_rotation = FwQuat( CQuat( obMatrix ).QuadwordValue() );
		m_TransformData->m_LocalSpace.m_translation = FwPoint( obMatrix.GetTranslation().QuadwordValue() );

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadword( m_TransformData->m_LocalSpace.m_rotation.QuadwordValue() );
			FwMathsInternal::ValidateQuadwordXYZ( m_TransformData->m_LocalSpace.m_translation.QuadwordValue() );
#		endif
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( m_pobParentHierarchy != NULL, ("Transforms with a parent hierarchy-index must have a parent hierarchy to index into.") );

		// Update the local joint info.
		m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_translation = FwPoint( obMatrix.GetTranslation().QuadwordValue() );
		m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_rotation = FwQuat( CQuat( obMatrix ).QuadwordValue() );

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadword( m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_rotation.QuadwordValue() );
			FwMathsInternal::ValidateQuadwordXYZ( m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_translation.QuadwordValue() );
#		endif
	}

	Invalidate();
}

/***************************************************************************************************
*
*	FUNCTION		Transform::SetLocalTranslation
*
*	DESCRIPTION		Set the local matrix translation to be the given point.
*
***************************************************************************************************/
void Transform::SetLocalTranslation( const CPoint &obPoint )
{
	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		ntError_p( GetFlags() & TRANSF_IS_EXTERNAL, ("Transforms with invalid hierarchy-indices must be external.") );
		m_TransformData->m_LocalSpace.m_translation = FwPoint( obPoint.QuadwordValue() );

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadwordXYZ( m_TransformData->m_LocalSpace.m_translation.QuadwordValue() );
#		endif
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( m_pobParentHierarchy != NULL, ("Transforms with a parent hierarchy-index must have a parent hierarchy to index into.") );

		// Update the local joint translation.
		m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_translation = FwPoint( obPoint.QuadwordValue() );

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadwordXYZ( m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_translation.QuadwordValue() );
#		endif
	}

	Invalidate();
}

/***************************************************************************************************
*
*	FUNCTION		Transform::SetLocalRotation
*
*	DESCRIPTION		Set the local matrix rotation to be the given quaternion.
*
***************************************************************************************************/
void Transform::SetLocalRotation( const CQuat &rot )
{
	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		ntError_p( GetFlags() & TRANSF_IS_EXTERNAL, ("Transforms with invalid hierarchy-indices must be external.") );
		m_TransformData->m_LocalSpace.m_rotation = FwQuat( rot.QuadwordValue() );

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadword( m_TransformData->m_LocalSpace.m_rotation.QuadwordValue() );
#		endif
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( m_pobParentHierarchy != NULL, ("Transforms with a parent hierarchy-index must have a parent hierarchy to index into.") );

		// Update the local joint translation.
		m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_rotation = FwQuat( rot.QuadwordValue() );

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadword( m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_rotation.QuadwordValue() );
#		endif
	}

	Invalidate();
}

/***************************************************************************************************
*
*	FUNCTION		Transform::SetLocalSpace
*
*	DESCRIPTION		Set the local space.
*
***************************************************************************************************/
void Transform::SetLocalSpace( const CQuat &rot, const CPoint &pos )
{
	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		ntError_p( GetFlags() & TRANSF_IS_EXTERNAL, ("Transforms with invalid hierarchy-indices must be external.") );
		m_TransformData->m_LocalSpace.m_rotation = FwQuat( rot.QuadwordValue() );
		m_TransformData->m_LocalSpace.m_translation = FwPoint( pos.QuadwordValue() );

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadword( m_TransformData->m_LocalSpace.m_rotation.QuadwordValue() );
			FwMathsInternal::ValidateQuadwordXYZ( m_TransformData->m_LocalSpace.m_translation.QuadwordValue() );
#		endif
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( m_pobParentHierarchy != NULL, ("Transforms with a parent hierarchy-index must have a parent hierarchy to index into.") );

		// Update the local joint translation.
		m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_rotation = FwQuat( rot.QuadwordValue() );
		m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_translation = FwPoint( pos.QuadwordValue() );

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadword( m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_rotation.QuadwordValue() );
			FwMathsInternal::ValidateQuadwordXYZ( m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_translation.QuadwordValue() );
#		endif
	}

	Invalidate();
}

/***************************************************************************************************
*
*	FUNCTION		Transform::SetLocalSpace
*
*	DESCRIPTION		Set the local space.
*
***************************************************************************************************/
void Transform::SetLocalSpace( const GpJoint &space )
{
	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		ntError_p( GetFlags() & TRANSF_IS_EXTERNAL, ("Transforms with invalid hierarchy-indices must be external.") );
		m_TransformData->m_LocalSpace = space;

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadword( m_TransformData->m_LocalSpace.m_rotation.QuadwordValue() );
			FwMathsInternal::ValidateQuadwordXYZ( m_TransformData->m_LocalSpace.m_translation.QuadwordValue() );
#		endif
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( m_pobParentHierarchy != NULL, ("Transforms with a parent hierarchy-index must have a parent hierarchy to index into.") );

		// Update the local joint translation.
		m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ] = space;

#		ifdef _DEBUG
			FwMathsInternal::ValidateQuadword( m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_rotation.QuadwordValue() );
			FwMathsInternal::ValidateQuadwordXYZ( m_pobParentHierarchy->m_pJointArray[ m_HierarchyIndex ].m_translation.QuadwordValue() );
#		endif
	}

	Invalidate();
}

/***************************************************************************************************
*
*	FUNCTION		Transform:: Flags functions.
*
*	DESCRIPTION		Functions handling setting, clearing and retrieving of flags for transforms.
*
***************************************************************************************************/
int32_t Transform::GetFlags() const
{
	ntError_p( m_HierarchyIndex == InvalidIndex || m_pobParentHierarchy != NULL, ("If we have a valid hierarchy index then we must have a hierarchy to index into.") );
	return m_HierarchyIndex == InvalidIndex ? m_TransformData->m_iFlags : m_pobParentHierarchy->m_pJointFlagsArray[ m_HierarchyIndex ];
}

void Transform::SetFlags( int32_t iFlags ) const
{
	ntError_p( m_HierarchyIndex == InvalidIndex || m_pobParentHierarchy != NULL, ("If we have a valid hierarchy index then we must have a hierarchy to index into.") );
	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		ntError_p( m_TransformData->m_iFlags & TRANSF_IS_EXTERNAL, ("Transforms with an invalid index should always be external.") );
		m_TransformData->m_iFlags = iFlags;
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( iFlags >= 0 && iFlags < 256, ("Flags is stored at a uint8_t type - out of range.") );
		m_pobParentHierarchy->m_pJointFlagsArray[ m_HierarchyIndex ] = (uint8_t)iFlags;
	}
}

void Transform::SetFlagBits( int32_t iFlagBits ) const
{
	ntError_p( m_HierarchyIndex == InvalidIndex || m_pobParentHierarchy != NULL, ("If we have a valid hierarchy index then we must have a hierarchy to index into.") );
	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		ntError_p( m_TransformData->m_iFlags & TRANSF_IS_EXTERNAL, ("Transforms with an invalid index should always be external.") );
		m_TransformData->m_iFlags |= iFlagBits;
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( iFlagBits >= 0 && iFlagBits < 256, ("Flags is stored at a uint8_t type - out of range.") );
		m_pobParentHierarchy->m_pJointFlagsArray[ m_HierarchyIndex ] |= (uint8_t)iFlagBits;
	}
}

void Transform::ClearFlagBits( int32_t iFlagBits ) const
{
	ntError_p( m_HierarchyIndex == InvalidIndex || m_pobParentHierarchy != NULL, ("If we have a valid hierarchy index then we must have a hierarchy to index into.") );
	if ( m_HierarchyIndex == InvalidIndex )
	{
		ntError( m_TransformData != NULL );
		ntError_p( m_TransformData->m_iFlags & TRANSF_IS_EXTERNAL, ("Transforms with an invalid index should always be external.") );
		m_TransformData->m_iFlags &= ~iFlagBits;
	}
	else
	{
		ntError( m_TransformData == NULL );
		ntError_p( iFlagBits >= 0 && iFlagBits < 256, ("Flags is stored at a uint8_t type - out of range.") );
		m_pobParentHierarchy->m_pJointFlagsArray[ m_HierarchyIndex ] &= ~((uint8_t)iFlagBits);
	}
}



