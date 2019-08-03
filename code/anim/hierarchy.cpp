/***************************************************************************************************
*
*	$Header:: /game/hierarchy.cpp 34    6/08/03 8:31 Dean                                          $
*
*	Hierarchy Processing
*
*	CHANGES
*
*	18/2/2003	Dean	Created
*
***************************************************************************************************/

#include "anim/hierarchy.h"
#include "anim/transform.h"
#include "core/exportstruct_clump.h"
#include "core/timer.h"

// The joint flags from ATGs GpJoint MUST match our Transform flags.
static_assert_in_class( (uint32_t)TRANSF_WORLD_MATRIX_INVALID == (uint32_t)GpJoint::kWorldMatrixInvalid, These_must_have_the_same_value );

CHierarchy*	CHierarchy::m_gpobWorldHierarchy = NULL;


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::CreateWorld
*
*	DESCRIPTION		Creates a specialised hierarchy that represents the root of our world.
*
*	NOTES			If the world has already been created, then we bail..
*
***************************************************************************************************/

void	CHierarchy::CreateWorld( void )
{
	// Check that we don't have a world already in existence
	ntAssert( !m_gpobWorldHierarchy );	

	// Create the world hierarchy
	m_gpobWorldHierarchy = CHierarchy::Create();

	// Now make sure the hierarchy doesn't have any flags set (especially the user-created one),
	// and also ensure that the root transform knows it's actually the root of the world..
	m_gpobWorldHierarchy->SetFlags( NULL );
	m_gpobWorldHierarchy->m_pobTransformArray->SetFlags( TRANSF_IS_WORLD_ROOT );
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::DestroyWorld
*
*	DESCRIPTION		Destroys the world, ensuring that the static variable pointing to the world is
*					correctly cleared.
*
***************************************************************************************************/

void	CHierarchy::DestroyWorld( void )
{
	// Check that we have a world already in existence
	ntAssert( m_gpobWorldHierarchy );	

	// We cache the existing world hierarchy pointer, and clear the static, to ensure that 
	// the underlying call to CHierarchy::Destroy() doesn't trip up.
	CHierarchy*	pobTempHierarchy = m_gpobWorldHierarchy;
	m_gpobWorldHierarchy = NULL;

	// Destroy it.
	CHierarchy::Destroy( pobTempHierarchy );
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::Create
*
*	DESCRIPTION		A static function that creates a CHierarchy object using containing a single
*					transform. Used to group external transforms that would otherwise be attached
*					directly to the world. 
*
*	NOTES			You are free to change the root transform as you wish.. it doesn't need to be 
*					identity.
*
***************************************************************************************************/

CHierarchy*	CHierarchy::Create( void )
{
	CHierarchy*	pobHierarchy;

	// Allocate memory for the hierarchy, based on our known initial state
	int	iHierarchySize	=	sizeof( CHierarchy ) +							// CHierarchy object
							( 1 * sizeof( Transform ) ) +					// Single Transform representing the world (identity)
							( 1 * sizeof( uint8_t ) );						// Single element flags array.

	iHierarchySize		=	ROUND_POW2( iHierarchySize, 128 );				// Realign current size onto 128-byte boundary.
	iHierarchySize		+=	ROUND_POW2( 1 * sizeof( GpJoint ), 128 );		// Single element joint array.

//	iHierarchySize		=	ROUND_POW2( iHierarchySize, 128 );				// Realign current size onto 128-byte boundary.
//	iHierarchySize		+=	ROUND_POW2( 1 * sizeof( CMatrix ), 128 );		// Single element local matrix array.
	
	iHierarchySize		=	ROUND_POW2( iHierarchySize, 128 );				// Realign current size onto 128-byte boundary.
	iHierarchySize		+=	ROUND_POW2( 1 * sizeof( FwTransform ), 128 );	// Single element world matrix array.

	pobHierarchy = (CHierarchy *)NT_MEMALIGN_CHUNK( Mem::MC_ANIMATION, iHierarchySize, 128 );
	ntError( ( (uintptr_t)pobHierarchy & 127 ) == 0 );

	// Init GpSkeleton to NULL.
	pobHierarchy->m_flags							=	0;
	pobHierarchy->m_jointCount						=	1;
	pobHierarchy->m_pSkeletonDef					=	NULL;
	pobHierarchy->m_pJointLinkage					=	NULL;
	pobHierarchy->m_pJointBindPose					=	NULL;
	pobHierarchy->m_pJointFlagsArray				=	NULL;
	pobHierarchy->m_pJointArray						=	NULL;
	pobHierarchy->m_pWorldMatrix					=	NULL;
	pobHierarchy->m_pDrivenKeys						=	NULL;
	pobHierarchy->m_transform						=	FwTransform( FwMaths::kIdentity );

	// Initialse CHierarchy.
	pobHierarchy->m_pobParent						=	NULL;
	pobHierarchy->m_pobNextSibling					=	NULL;
	pobHierarchy->m_pobFirstChild					=	NULL;
	pobHierarchy->m_iFlags							=	HIERF_USER_CREATED;

	pobHierarchy->m_iTransformCount					=	1;
	pobHierarchy->m_pobTransformArray				=	( Transform* )( pobHierarchy + 1 );

	pobHierarchy->m_pJointFlagsArray				=	(uint8_t *)( pobHierarchy->m_pobTransformArray + pobHierarchy->m_iTransformCount );
	pobHierarchy->m_pJointArray						=	(GpJoint *)ROUND_POW2( (uintptr_t)( pobHierarchy->m_pJointFlagsArray + pobHierarchy->m_iTransformCount ), 128 );
	pobHierarchy->m_pWorldMatrix					=	(FwTransform *)ROUND_POW2( (uintptr_t)( pobHierarchy->m_pJointArray + pobHierarchy->m_iTransformCount ), 128 );

	pobHierarchy->m_pobSkinMatrixArray				=	NULL;
	pobHierarchy->m_pobClumpHeader					=	NULL;
	pobHierarchy->m_uiHierarchyKey					=	0;

	pobHierarchy->m_uiLastRenderedTick				=	0;

	pobHierarchy->m_BoneOffsetArray					=	NULL;

	pobHierarchy->m_pobSkinToBoneArray				=	NULL;
	pobHierarchy->m_pCharacterBoneToIndexArray		=	NULL;

	// Clear each world matrix.
	pobHierarchy->m_pWorldMatrix[ 0 ] = FwTransform( FwMaths::kIdentity );

	pobHierarchy->m_pJointFlagsArray[ 0 ] = TRANSF_WORLD_MATRIX_INVALID | TRANSF_WORLD_ROTATION_INVALID;
	pobHierarchy->m_pJointArray->m_rotation = FwQuat( FwMaths::kIdentity );
	pobHierarchy->m_pJointArray->m_scale = FwVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	pobHierarchy->m_pJointArray->m_translation = FwPoint( FwMaths::kZero );

	// Finally, initialise our transforms and other matrices..
	pobHierarchy->m_pobTransformArray->m_pobParent = NULL;
	pobHierarchy->m_pobTransformArray->m_pobNextSibling = NULL;
	pobHierarchy->m_pobTransformArray->m_pobFirstChild = NULL;
	pobHierarchy->m_pobTransformArray->m_pobExternalData = NULL;

	// This is an internal transform, associate it with the correct index.
	pobHierarchy->m_pobTransformArray->m_HierarchyIndex = 0;
	pobHierarchy->m_pobTransformArray->m_pobParentHierarchy = pobHierarchy;
	pobHierarchy->m_pobTransformArray->m_TransformData = NULL;
	pobHierarchy->m_pobTransformArray->m_obWorldRotation.SetIdentity();

	pobHierarchy->m_pobTransformArray->SetFlags( NULL );

	// Return the hierarchy pointer to the caller
	return pobHierarchy;
}

/***************************************************************************************************
*
*	FUNCTION		CHierarchy::Destroy
*
*	DESCRIPTION		A static function that destroys a CHierarchy object
*
***************************************************************************************************/
void	CHierarchy::Destroy( CHierarchy* pobHierarchy )
{
	ntAssert_p( pobHierarchy, ("Must provide a vaild heirarchy ptr here") );
	pobHierarchy->~CHierarchy();

	uint8_t* pData = (uint8_t*) pobHierarchy;
	NT_FREE_CHUNK( Mem::MC_ANIMATION, (uintptr_t)pData );
}

/***************************************************************************************************
*
*	FUNCTION		CHierarchy::Create
*
*	DESCRIPTION		A static function that creates a CHierarchy object using information from the
*					specified CClumpHeader. A normal constructor isn't an option for this class,
*					as it is a single allocation with a number of additional areas of data storage
*					tacked after the CHierarchy object itself.
*
***************************************************************************************************/
CHierarchy*	CHierarchy::Create( const CClumpHeader* pobClumpHeader )
{
	CHierarchy*	pobHierarchy;
	bool		bIsSkinned;

	// Validate inputs
	ntAssert( pobClumpHeader );

	// Are we skinned?
	if ( pobClumpHeader->m_pobSkinToBoneArray )
		bIsSkinned = true;
	else
		bIsSkinned = false;

	bool transformBindPose = false;
#ifdef PLATFORM_PS3
	if (pobClumpHeader -> m_uiVersionTag != NEW_CLUMP_VERSION)
#endif
	{
		transformBindPose = true;
	}

	// Allocate memory for the hierarchy, based on our known initial state
	int	iHierarchySize	=	sizeof( CHierarchy ) +																// CHierarchy object
							( pobClumpHeader->m_iNumberOfTransforms * sizeof( Transform ) );					// Single Transform representing the world (identity)

	iHierarchySize		+=	pobClumpHeader->m_iNumberOfTransforms * sizeof( uint8_t );							// 8-bit flag per transform.

	iHierarchySize		=	ROUND_POW2( iHierarchySize, 128 );													// Realign current size onto 128-byte boundary.
	iHierarchySize		+=	ROUND_POW2( pobClumpHeader->m_iNumberOfTransforms * sizeof( GpJoint ), 128 );		// Joint array.
	iHierarchySize		+=	ROUND_POW2( pobClumpHeader->m_iNumberOfTransforms * sizeof( FwTransform ), 128 );	// World matrix per transform.
	if (transformBindPose)
	{
		iHierarchySize		+=	ROUND_POW2( pobClumpHeader->m_iNumberOfTransforms * sizeof( GpJoint ), 128 );		// Joint bind-pose.
		iHierarchySize		+=	ROUND_POW2( pobClumpHeader->m_iNumberOfTransforms * sizeof( CPoint ), 128 );		// Bone-offset per bind-pose joint.
	}
	iHierarchySize		+=	ROUND_POW2( sizeof( GpSkeletonDef ), 16 );											// GpSkeletonDef space.

	if ( bIsSkinned )																							// If we're skinned, then we need room for 
		iHierarchySize	+= ( pobClumpHeader->m_iNumberOfTransforms * sizeof( CSkinMatrix ) );					// the skin matrices. Note this doesn't mean all meshes must be skinned!

	pobHierarchy = (CHierarchy *)NT_MEMALIGN_CHUNK( Mem::MC_ANIMATION, iHierarchySize, 128 );
	
	// Init GpSkeleton to NULL.
	pobHierarchy->m_flags							=	0;
	pobHierarchy->m_jointCount						=	pobClumpHeader->m_iNumberOfTransforms;
	pobHierarchy->m_pSkeletonDef					=	NULL;
	pobHierarchy->m_pJointLinkage					=	NULL;
	pobHierarchy->m_pJointBindPose					=	NULL;
	pobHierarchy->m_pJointFlagsArray				=	NULL;
	pobHierarchy->m_pJointArray						=	NULL;
	pobHierarchy->m_pWorldMatrix					=	NULL;
	pobHierarchy->m_pDrivenKeys						=	NULL;
	pobHierarchy->m_transform						=	FwTransform( FwMaths::kIdentity );

	// Initialse CHierarchy.
	pobHierarchy->m_pobParent						=	NULL;
	pobHierarchy->m_pobNextSibling					=	NULL;
	pobHierarchy->m_pobFirstChild					=	NULL;
	pobHierarchy->m_iFlags							=	HIERF_INVALIDATED_MAIN;

	pobHierarchy->m_iTransformCount					=	pobClumpHeader->m_iNumberOfTransforms;
	pobHierarchy->m_pobTransformArray				=	(Transform *)( pobHierarchy + 1 );

	pobHierarchy->m_pJointFlagsArray				=	(uint8_t *)( pobHierarchy->m_pobTransformArray + pobHierarchy->m_iTransformCount );
	pobHierarchy->m_pJointArray						=	(GpJoint *)ROUND_POW2( (uintptr_t)( pobHierarchy->m_pJointFlagsArray + pobHierarchy->m_iTransformCount ), 128 );
	pobHierarchy->m_pWorldMatrix					=	(FwTransform *)ROUND_POW2( (uintptr_t)( pobHierarchy->m_pJointArray + pobHierarchy->m_iTransformCount ), 128 );

	// Setup bind-pose data.
	if (transformBindPose)
	{
		pobHierarchy->m_pJointBindPose					=	(GpJoint *)ROUND_POW2( (uintptr_t)( pobHierarchy->m_pWorldMatrix + pobHierarchy->m_iTransformCount ), 128 );
		pobHierarchy->m_BoneOffsetArray					=	(CPoint *)ROUND_POW2( (uintptr_t)( pobHierarchy->m_pJointBindPose + pobHierarchy->m_iTransformCount ), 128 );
		pobHierarchy->m_pSkeletonDef					=	(GpSkeletonDef *)ROUND_POW2( (uintptr_t)( pobHierarchy->m_BoneOffsetArray + pobHierarchy->m_iTransformCount ), 16 );
	}
	else
	{
		pobHierarchy->m_pJointBindPose					=	(GpJoint *)(CBindPose*)pobClumpHeader->m_pobBindPoseArray; // first type-cast calls the conversion operator of the ntDiskPointer
		pobHierarchy->m_BoneOffsetArray					=	(CPoint*)ROUND_POW2( (uintptr_t)(pobHierarchy->m_pJointBindPose + pobHierarchy->m_iTransformCount), 128);
		pobHierarchy->m_pSkeletonDef					=	(GpSkeletonDef *)ROUND_POW2( (uintptr_t)( pobHierarchy->m_pWorldMatrix + pobHierarchy->m_iTransformCount ), 16 );
	}

	// Setup GpSkeletonDef.
	//pobHierarchy->m_pSkeletonDef					=	(GpSkeletonDef *)ROUND_POW2( (uintptr_t)( pobHierarchy->m_BoneOffsetArray + pobHierarchy->m_iTransformCount ), 16 );
	GpSkeletonDef *skel_def							=	const_cast< GpSkeletonDef * >( pobHierarchy->m_pSkeletonDef );
	skel_def->m_tag									=	FW_MAKE_TAG( 'S', 'K', '0', '6' );
	skel_def->m_skeletonKey							=	pobClumpHeader->m_uiHierarchyKey;
	skel_def->m_ownsMemory							=	0;
	skel_def->m_jointCount							=	(int16_t)pobClumpHeader->m_iNumberOfTransforms;
	*(int32_t *)&skel_def->m_jointLinkageArray		=	(intptr_t)( (CTransformLinkage *)pobClumpHeader->m_pobTransformLinkageArray ) - (intptr_t)( &skel_def->m_jointLinkageArray );
	*(int32_t *)&skel_def->m_jointBindPoseArray		=	(intptr_t)pobHierarchy->m_pJointBindPose - (intptr_t)( &skel_def->m_jointBindPoseArray );
	*(int32_t *)&skel_def->m_jointNameArray			=	(intptr_t)( pobHierarchy->m_pSkeletonDef->m_jointLinkageArray.Get() + pobHierarchy->m_iTransformCount ) - (intptr_t)( &skel_def->m_jointNameArray );
	*(int32_t *)&skel_def->m_drivenKeysDef			=	NULL;
	*(int32_t *)&skel_def->m_skeletalLod			=	NULL;
	skel_def->m_futureExpansion[ 0 ]				=	0;
	skel_def->m_futureExpansion[ 1 ]				=	0;
	skel_def->m_futureExpansion[ 2 ]				=	0;
	skel_def->m_futureExpansion[ 3 ]				=	0;

	if ( bIsSkinned )
		pobHierarchy->m_pobSkinMatrixArray			=	(CSkinMatrix *)( pobHierarchy->m_pSkeletonDef + 1 );
	else
		pobHierarchy->m_pobSkinMatrixArray			=	NULL;

	pobHierarchy->m_pobSkinToBoneArray				=	pobClumpHeader->m_pobSkinToBoneArray;
	pobHierarchy->m_pCharacterBoneToIndexArray		=	pobClumpHeader->m_pCharacterBoneToIndexArray;
	pobHierarchy->m_uiHierarchyKey					=	pobClumpHeader->m_uiHierarchyKey;
	pobHierarchy->m_uiLastRenderedTick				=	0;
	pobHierarchy->m_pobClumpHeader					=	pobClumpHeader;

	if (transformBindPose)
	{
		// Initialise our bind-pose data.
		const CBindPose *clump_bind_pose = pobClumpHeader->m_pobBindPoseArray;
		for ( int32_t i=0;i<pobHierarchy->m_iTransformCount;i++ )
		{
			pobHierarchy->m_BoneOffsetArray[ i ] = clump_bind_pose[ i ].m_obBoneOffset;
			const_cast< GpJoint * >( pobHierarchy->m_pJointBindPose )[ i ].m_scale			= FwVector4( 1.0f, 1.0f, 1.0f, 1.0f );
			const_cast< GpJoint * >( pobHierarchy->m_pJointBindPose )[ i ].m_rotation		= FwQuat( clump_bind_pose[ i ].m_obRotation.QuadwordValue() );
			const_cast< GpJoint * >( pobHierarchy->m_pJointBindPose )[ i ].m_translation	= FwPoint( clump_bind_pose[ i ].m_obTranslation.QuadwordValue() );
		}
	}


	// Work out where the pointers should start.
	pobHierarchy->m_pJointLinkage = (const GpJointLinkage *)( (CTransformLinkage *)pobClumpHeader->m_pobTransformLinkageArray );

	// Finally, initialise our transforms
	Transform *					pobTransform	= pobHierarchy->m_pobTransformArray;
	const GpJointLinkage *		pobLinkage		= pobHierarchy->m_pJointLinkage;
	const GpJoint *				pobBindPose		= pobHierarchy->m_pJointBindPose;

	for ( int32_t iLoop = 0; iLoop < pobHierarchy->m_iTransformCount; iLoop++ )
	{
		// Clear each world matrix.
		pobHierarchy->m_pWorldMatrix[ iLoop ] = FwTransform( FwMaths::kZero );

		pobHierarchy->m_pJointFlagsArray[ iLoop ]			= 0;
		pobHierarchy->m_pJointArray[ iLoop ].m_rotation		= pobBindPose->m_rotation;
		pobHierarchy->m_pJointArray[ iLoop ].m_scale		= pobBindPose->m_scale;
		pobHierarchy->m_pJointArray[ iLoop ].m_translation	= pobBindPose->m_translation;

		// This is an internal transform, associate it with the correct index.
		pobTransform->m_HierarchyIndex = iLoop;

		pobTransform->m_TransformData = NULL;

		// Pointers to owner & external data
		pobTransform->m_pobParentHierarchy	= pobHierarchy;
		pobTransform->m_pobExternalData		= NULL;

		// Create transform hierarchy linkage
		if ( pobLinkage->m_parentIndex >= 0 )
			pobTransform->m_pobParent		= &pobHierarchy->m_pobTransformArray[ pobLinkage->m_parentIndex ];
		else
			pobTransform->m_pobParent		= NULL;

		if ( pobLinkage->m_nextSiblingIndex >= 0 )
			pobTransform->m_pobNextSibling	= &pobHierarchy->m_pobTransformArray[ pobLinkage->m_nextSiblingIndex ];
		else
			pobTransform->m_pobNextSibling	= NULL;

		if ( pobLinkage->m_firstChildIndex >= 0 )
			pobTransform->m_pobFirstChild	= &pobHierarchy->m_pobTransformArray[ pobLinkage->m_firstChildIndex ];
		else
			pobTransform->m_pobFirstChild	= NULL;

		// Flags & validity information
		pobTransform->SetFlagBits( TRANSF_WORLD_MATRIX_INVALID | TRANSF_WORLD_ROTATION_INVALID );

		// Set matrices & rotations
		pobTransform->m_obWorldRotation.SetIdentity();

		pobTransform++;
		pobLinkage++;
		pobBindPose++;
	}

#ifndef _RELEASE
	pobHierarchy->m_DEBUG_SCALE_MAT.SetIdentity();
#endif

	// Return the hierarchy to our creator..
	return pobHierarchy;
}

/***************************************************************************************************
*
*	FUNCTION		CHierarchy::Destroy
*
*	DESCRIPTION		Destroys a hierarchy.. note that memory freeing is handled by an overloaded
*					delete operator.
*
***************************************************************************************************/

CHierarchy::~CHierarchy()
{
	ntAssert( this != m_gpobWorldHierarchy );		// World must be destroyed with DestroyWorld()

	Transform* pobTransform;

	// We need to make sure that we don't have any referenced transforms that exist outside of the
	// main hierarchy transform array.
	for ( int iLoop = 0; iLoop < GetTransformCount(); iLoop++ )
	{
		pobTransform = &m_pobTransformArray[ iLoop ];
		ntAssert( ( pobTransform->GetParent() == NULL )		|| ( IsEmbeddedTransform( pobTransform->GetParent() ) ) );
		ntAssert( ( pobTransform->GetNextSibling() == NULL )	|| ( IsEmbeddedTransform( pobTransform->GetNextSibling() ) ) );
		ntAssert( ( pobTransform->GetFirstChild() == NULL )	|| ( IsEmbeddedTransform( pobTransform->GetFirstChild() ) ) );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::ResetToBindPose
*
*	DESCRIPTION		A function to set all of the local space matricies on a heirarchy to their bind
*					pose values. 
*
*	INPUTS			bOverrideLocks		-	If 'true' then we reset all transforms irrespective of 
*											their locked state. If false, then we do not reset
*											transforms that are locked in either local or world
*											space. This function uses a default parameter of 'true'.
*
***************************************************************************************************/

void	CHierarchy::ResetToBindPose()
{
	Transform *					pobTransform	= m_pobTransformArray;
	const GpJoint *				pobBindPose		= m_pJointBindPose;

	ntError( pobBindPose != NULL );

	for ( int iLoop = 0; iLoop < m_iTransformCount; iLoop++ )
	{
		// Set matrices & rotations
		if ( pobTransform->m_HierarchyIndex == Transform::InvalidIndex )
		{
			ntError( pobTransform->GetFlags() & TRANSF_IS_EXTERNAL );
			ntError( pobTransform->m_TransformData != NULL );

			pobTransform->m_TransformData->m_LocalSpace = *pobBindPose;
			pobTransform->m_TransformData->m_obWorldMatrix.Clear();
		}
		else
		{
			ntError( pobTransform->m_HierarchyIndex >= 0 && pobTransform->m_HierarchyIndex < m_iTransformCount );
			ntError( !( pobTransform->GetFlags() & TRANSF_IS_EXTERNAL ) );
			ntError( pobTransform->m_TransformData == NULL );

			int32_t idx = pobTransform->m_HierarchyIndex;

			m_pJointArray[ idx ].m_rotation		= pobBindPose->m_rotation;
			m_pJointArray[ idx ].m_translation	= pobBindPose->m_translation;

			m_pWorldMatrix[ idx ] = FwTransform( FwMaths::kZero );
		}

		pobTransform->m_obWorldRotation.SetIdentity();

		// Move to the next transform
		pobTransform++;
		pobBindPose++;
	}

	// Mark the root transform for this hierarchy as invalid. This will propagate through all children.
	m_pobTransformArray->Invalidate();
};


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::GetCharacterBoneTransform
*
*	DESCRIPTION		For character-based hierarchies, this function allows direct access to a 
*					transform based on a character bone ID. If the bone ID is not in use by this
*					particular hierarchy, then we return NULL. Character specific bones must be 
*					retrieved by name, which is significantly more expensive.
*
*	INPUTS			eBoneID			-	A valid character bone ID.
*
*	NOTES			This function can only be called on character-based hierarchies.
*
***************************************************************************************************/

Transform*		CHierarchy::GetCharacterBoneTransform( CHARACTER_BONE_ID eBoneID ) const
{
	// We need to have bones for this to work...
	if(!GetCharacterBoneToIndexArray())
		return 0;

	int iTransformIndex = GetCharacterBoneToIndexArray()[ eBoneID ];
	if ( iTransformIndex >= 0 )
		return GetTransform( iTransformIndex );
	else
		return 0;
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::GetTransformIndex
*
*	DESCRIPTION		Retrieve the index of a transform based on its name.
*
*	INPUTS			obNameHash		-	A reference to a CHashedString object constructed from the
*										name.
*										If obNameHash is "ROOT", then this function always returns
*										index 0 (the root transform).
*
*	NOTES			Use like this : 
*
*					int iIndex = m_pobHierarchy->GetTransformIndex( CHashedString( "r_wrist" ) );
*
*					Of course, if you're using the same name over and over again, please seriously
*					consider caching the CHashedString object away - it's only 4 Bytes.
*
***************************************************************************************************/

int32_t	CHierarchy::GetTransformIndex( const CHashedString& obNameHash ) const
{
	const static CHashedString obROOT( "ROOT" );
	if( obNameHash == obROOT )
		return 0;

	return GetJointIndex( FwHashedString( obNameHash.GetHash() ) );
}

/***************************************************************************************************
*
*	FUNCTION		CHierarchy::GetTransformIndex
*
*	DESCRIPTION		Same as above.
*
***************************************************************************************************/
int32_t	CHierarchy::GetTransformIndexFromHash( uint32_t uiHash )
{
	/*const static CHashedString obROOT( "ROOT" );

	if( uiHash == obROOT.GetValue() )
		return 0;*/

	int32_t idx = GetJointIndex( FwHashedString( uiHash ) );
	if ( idx == -1 )
		return 0;

	return idx;
}

/***************************************************************************************************
*
*	FUNCTION		CHierarchy::AddChild
*
*	DESCRIPTION		Adds the specified hierarchy as a child of the current CHierarchy object. 
*
*	INPUTS			pobChild			-	A pointer to the proposed child hierarchy.
*
*	NOTES			If the proposed child already has a parent, then we fail. We don't support any
*					kind of automatic adoption of child nodes.
*
*					Returns false if the addition failed
*
***************************************************************************************************/
void CHierarchy::AddChild( CHierarchy* pobChild )
{
	// If the child already has a parent then we fail
	if ( pobChild->GetParent() )
	{
		ntAssert_p( 0, ( "Failed to add a child heirarchy, the child already has a parent" ) );
		user_warn_p( 0, ( "Failed to add a child heirarchy, the child already has a parent" ) );
	}

	// If the child is still linked to siblings then we fail
	else if ( pobChild->GetNextSibling() )
	{
		ntAssert_p( 0, ( "Failed to add a child heirarchy, the child is linked to siblings" ) );
		user_warn_p( 0, ( "Failed to add a child heirarchy, the child is linked to siblings" ) );
	}
	
	// Otherwise we are all set to go
	else
	{
		// Set the child parent pointer to point to 'this' and insert at the front of the list of siblings
		pobChild->m_pobParent = this;
		pobChild->m_pobNextSibling = m_pobFirstChild;
		m_pobFirstChild = pobChild;
	}
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::RemoveFromParent
*
*	DESCRIPTION		Removes this hierarchy from its parent. 
*
***************************************************************************************************/

void	CHierarchy::RemoveFromParent( void )
{
	ntAssert( GetParent() );

	CHierarchy* pobHierarchy	 = m_pobParent->m_pobFirstChild;
	CHierarchy* pobPrevHierarchy = NULL;
		
	while ( pobHierarchy != this )
	{
		// If we didn't find ourselves in our parent's list of hierarchies, bail..
		ntAssert( pobHierarchy != NULL );

		pobPrevHierarchy = pobHierarchy;
		pobHierarchy = pobHierarchy->m_pobNextSibling;		
	}
		
	// If we were chained to by another sibling, point it to any other siblings we might have been
	// before. If we had no prior siblings, then we must be setting the first of our siblings (if
	// any are defined) to be the first child of the parent.

	if ( pobPrevHierarchy )
		pobPrevHierarchy->m_pobNextSibling = m_pobNextSibling;
	else
		m_pobParent->m_pobFirstChild = m_pobNextSibling;

	// Finally clear our parent and sibling pointers
	m_pobParent = NULL;
	m_pobNextSibling = NULL;
}

/***************************************************************************************************
*
*	FUNCTION		CHierarchy::SetScaleMatrix
*
*	DESCRIPTION		Debug code so artists can rapidly preview scaled characters. DONOT USE FOR GAME!
*
*	NOTES			Scales dont work as it breaks our lighting and skinning.
*					This debug functionality is done this way as people always overrite the frikkin
*					local matrix of the root directly.
*					The observant among you will notice that GetLocalTransform() is now 'incorrect'
*					as it dosnt include this scale. Another reason we cant use this in game.
*
***************************************************************************************************/
#ifndef _RELEASE

void CHierarchy::SetScaleMatrix( const CMatrix& mat )
{
	m_DEBUG_SCALE_MAT = mat;
}

#endif

/***************************************************************************************************
*
*	FUNCTION		CHierarchy::Resynchronise
*
*	DESCRIPTION		Handles the resynchronisation of a heirarchy. When we find a hierarchy that is
*					marked as invalid, then we resynchronise all of the transforms that are 
*					associated with the invalid heirarchy. This processing can extend outside of the
*					bounds of the current CHierarchy object, but once we've dealt with invalidated
*					transforms we mark all child hierarchies as valid again. 
*
*	NOTES			This function recurses.. watch out.
*
***************************************************************************************************/

void	CHierarchy::Resynchronise( void )
{
#ifndef _RELEASE
	if (GetRootTransform() && !(GetRootTransform()->GetFlags() & TRANSF_IS_WORLD_ROOT))
		GetRootTransform()->SetLocalMatrix( m_DEBUG_SCALE_MAT * GetRootTransform()->GetLocalMatrix() );
#endif

	// If this hierarchy is invalid, then we need to resynchronise all transforms underneath dirty ones that we find. 
	if ( GetFlags() & HIERF_INVALIDATED_MAIN )
	{
		GetRootTransform()->Resynchronise();

		// Mark current and all child hierarchies as valid, and generate skin matrices.. (this will recurse)
		Finalise();
	}
	else
	{
		if ( GetFlags() & HIERF_INVALIDATED_EXTERNAL )
		{
			// If any of the main transforms point to external transforms, use that pointer as a
			// starting point for resynchronisation.
			for ( int iLoop = 0; iLoop < GetTransformCount(); iLoop++ )
			{
				for ( Transform* pobChild = GetTransform( iLoop )->m_pobFirstChild; pobChild != NULL; pobChild = pobChild->m_pobNextSibling )
				{
					// As soon as we find a child that's invalid, we can stop recursing down.. 
					if ( pobChild->GetFlags() & TRANSF_IS_EXTERNAL )
						pobChild->Resynchronise();
				}
			}
	
			// Clear the external invalidation bits			
			ClearFlagBits( HIERF_INVALIDATED_EXTERNAL );
		}

		// Once we're happy with this hierarchy, then we look through all children and resynchronise them..
		for ( CHierarchy* pobChild = m_pobFirstChild; pobChild != NULL; pobChild = pobChild->m_pobNextSibling )
		{
			pobChild->Resynchronise();
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::Finalise
*
*	DESCRIPTION		Marks the current hierarchy (as well as child hierarchies) as valid
*
*	NOTES			This function recurses.
*
***************************************************************************************************/

void	CHierarchy::Finalise( void )
{
	//ntAssert( GetFlags() & HIERF_INVALIDATED_MAIN );
	// 26/11/04 HC: This ntAssert has been commented out because of a situation where finalise is being called on
	// a hierarchy that didnt need it. This particular example was a spear that was being parented to the root
	// node of a statue, but the statue didnt invalidate its child (the spear hierarchy) because the statues root
	// didn't move or do anything to warrant it getting invalidated.

	// Mark this hierarchy as valid..
	ClearFlagBits( HIERF_INVALIDATED_MAIN | HIERF_INVALIDATED_EXTERNAL );

	// Mark all child hierarchies as valid too..
	for ( CHierarchy* pobChild = m_pobFirstChild; pobChild != NULL; pobChild = pobChild->m_pobNextSibling )
	{
		pobChild->Finalise();
	}
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::UpdateSkinMatrices
*
*	DESCRIPTION		Skin matrix updates are closely linked to rendering behaviour. Indeed, there
*					should be no instance in update-based code where skin matrices are being used. 
*					This means there's little (or no) point in computing skin matrices for any
*					hierarchies that are not rendered.
*
*					As such, the update of skin matrices have been separated from the actual
*					Finalise() operation.
*
*	NOTES			This function can only be called on hierarchies that are valid. 
*
***************************************************************************************************/

void	CHierarchy::UpdateSkinMatrices( void )
{
	ntAssert( !( GetFlags() & HIERF_INVALIDATED_MAIN ) );

	// Blat through the array fixing up skin-matrices for this hierarchy.
	if ( ( GetFlags() & HIERF_MAY_REQUIRE_SKIN_MATRIX_UPDATE ) )
	{
		ntAssert( m_pobSkinToBoneArray );
		if ( m_pobSkinToBoneArray )
		{
			CMatrix obSkinToRoot = m_pobTransformArray[0].GetWorldMatrixFast().GetAffineInverse();
			CMatrix	obMatrix;

			for ( int iLoop = 0; iLoop < m_iTransformCount; iLoop++ )
			{
				// TODO : This could do with inlining
				obMatrix = m_pobSkinToBoneArray[ iLoop ] * m_pobTransformArray[ iLoop ].GetWorldMatrixFast() * obSkinToRoot;
				obMatrix = obMatrix.GetTranspose();

				m_pobSkinMatrixArray[ iLoop ].m_obRow0 = obMatrix.Row( 0 );
				m_pobSkinMatrixArray[ iLoop ].m_obRow1 = obMatrix.Row( 1 );
				m_pobSkinMatrixArray[ iLoop ].m_obRow2 = obMatrix.Row( 2 );
			}
		}

		ClearFlagBits( HIERF_MAY_REQUIRE_SKIN_MATRIX_UPDATE );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::MarkAsRendered
*
*	DESCRIPTION		Marks the hierarchy as rendered during the current frame by storing the current
*					system tick value.
*
***************************************************************************************************/

void	CHierarchy::MarkAsRendered( void )
{
	m_uiLastRenderedTick = CTimer::Get().GetSystemTicks();
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::WasRenderedLastFrame
*
*	DESCRIPTION		Returns whether the hierarchy was rendered last frame by comparing stored 
*					system tick values to those currently held within CTimer.
*
*	RESULT			'true' if we were rendered last frame, else 'false'.
*
***************************************************************************************************/

bool	CHierarchy::WasRenderedLastFrame( void ) const
{
	return ( m_uiLastRenderedTick == ( CTimer::Get().GetSystemTicks() - 1 ) );
}


/***************************************************************************************************
*
*	FUNCTION		CHierarchy::Collapse
*
*	DESCRIPTION		Flattens a hierarchy so that all nodes are attached to the root. The world
*					matrix is essentially preserved as the local matrix is recalculated. Should
*					be used with caution since theres no way to restore the hierarchy to its
*					original state once it has been collapsed!
*
***************************************************************************************************/

void	CHierarchy::Collapse ()
{
	CHierarchy* pobParent=GetParent();

	CMatrix* pobWorldMatrix=NT_NEW_ARRAY_CHUNK( Mem::MC_ANIMATION ) CMatrix [m_iTransformCount];

	for(int iCount=1; iCount<m_iTransformCount; ++iCount)
	{
		pobWorldMatrix[iCount]=m_pobTransformArray[iCount].GetWorldMatrix();
	}

	for(int iCount=1; iCount<m_iTransformCount; ++iCount)
	{
		if (m_pobTransformArray[iCount].GetParent()!=m_pobTransformArray) // No point reparenting this transform if its already attached to the root
		{
			m_pobTransformArray[iCount].RemoveFromParent();

			m_pobTransformArray->AddChild(&m_pobTransformArray[iCount]);

			m_pobTransformArray[iCount].SetLocalMatrixFromWorldMatrix(pobWorldMatrix[iCount]);
		}
	}

	if (GetParent()==0) // Re-attach this hierarchy to its parent if necessary
	{
		pobParent->AddChild(this);
	}

	NT_DELETE_ARRAY_CHUNK( Mem::MC_ANIMATION, pobWorldMatrix );
}

/***************************************************************************************************
*
*	FUNCTION		CHierarchy::SetGpSkeletonRootFromParent
*
*	DESCRIPTION		Sets the root on the GpSkeleton root to the world matrix of the parent
*					transform of this hierarchy's root (if it has been parented).
*
***************************************************************************************************/
void CHierarchy::SetGpSkeletonRootFromParent()
{
	Transform *root = GetRootTransform();
	ntError( root != NULL );

	Transform *root_parent = root->GetParent();
	if ( root_parent == NULL )
	{
		GpSkeleton::SetTransform( FwTransform( FwMaths::valIdentityTag ) );
		return;
	}

	CQuat world_rot = root_parent->GetWorldRotation();
	CPoint world_trans = root_parent->GetWorldTranslation();

	FwQuat fw_rot( world_rot.QuadwordValue() );
	FwPoint fw_trans( world_trans.QuadwordValue() );

	FwTransform fw_world( fw_rot, fw_trans );
	GpSkeleton::SetTransform( fw_world );
}

/***************************************************************************************************
*
*	FUNCTION		CHierarchy::ResetGpSkeletonRoot
*
*	DESCRIPTION		Resets the root on the GpSkeleton root to the identity matrix.
*
***************************************************************************************************/
void CHierarchy::ResetGpSkeletonRoot( bool invalidate )
{
	if ( m_pJointLinkage != NULL && m_pJointFlagsArray != NULL )
	{
		GpSkeleton::SetTransform( FwTransform( FwMaths::valIdentityTag ), invalidate );
	}
}







