//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Skeletal System Definition

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_SKELETON_H
#define GP_SKELETON_H

#include <Fw/FwMaths/FwQuat.h>
#include <Fw/FwMaths/FwPoint.h>
#include <Fw/FwMaths/FwVector.h>
#include <Fw/FwMaths/FwTransform.h>
#include <Fw/FwStd/FwHashedString.h>

// We need to include this to make sure we use the same limits as FpAnimClipDef..
#ifdef __SPU__
#	include <../Fp/FpAnim/FpAnimClip.h>
#else //__SPU__
#	include <Fp/FpAnim/FpAnimClip.h>
#endif//__SPU__

class	GpDrivenKeysDef;
class	GpDrivenKeys;

//--------------------------------------------------------------------------------------------------
/**
	@class			GpSkeletonJointDef
	
	@brief			A class used to build runtime skeletal definitions. 	
**/
//--------------------------------------------------------------------------------------------------

class	GpSkeletonJointDef
{
	friend class GpSkeletonDef;

public:
	GpSkeletonJointDef();

	void		AddChild( GpSkeletonJointDef& child );
	void		SetName( FwHashedString name );
	void		SetRotation( FwQuat_arg rotation );
	void		SetTranslation( FwPoint_arg translation );
	void		SetScale( FwVector4_arg scale );
	
protected:
	void		ComputeJointInfo( int& jointCount );			// Compute joint count, and also joint index data.

private:
	int									m_index;				// Skeletal hierarchy index (filled in during GpSkeletonDef::Create())
	FwHashedString						m_name;					// Hash of joint name
	GpSkeletonJointDef*					m_pParent;				// Pointer to parent
	GpSkeletonJointDef*					m_pNextSibling;			// Pointer to next sibling 
	GpSkeletonJointDef*					m_pFirstChild;			// Pointer to first child
	FwQuat								m_rotation;				// Default/bind rotation from parent
	FwPoint								m_translation;			// Default/bind translation from parent
	FwVector4							m_scale;				// Default/bind scale
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpJointLinkage
	
	@brief			This structure defines the linkage between joints, defined as indices into the
					hierarchy array itself.			
**/
//--------------------------------------------------------------------------------------------------

class GpJointLinkage : public FwNonCopyable
{
public:
	static	const int	kInvalidJointIndex	= -1;

	enum	JointLinkageFlags
	{
		kParentScaleCompensate = ( 1 << 0 )						 
	};

	s16									m_flags;
	s16									m_parentIndex;
	s16									m_nextSiblingIndex;	
	s16									m_firstChildIndex;
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpJointName
	
	@brief			Applications need to access joints by name, so this allows us to have a sorted
					array of hashed strings, mapping to hierarchy joint indices.			
**/
//--------------------------------------------------------------------------------------------------

class	GpJointName
{
	friend class GpSkeletonDef;

public:
	FwHashedString		GetName( void ) const	{ return m_name; }		
	s32					GetIndex( void ) const	{ return m_index; }		

//private:
	FwHashedString		m_name;					// Hashed name for the joint.
	s32					m_index;				// Index into skeletal hierarchy.
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpJoint
	
	@brief			Joints.. they have rotation, translation, and scale.. that's all. 		
**/
//--------------------------------------------------------------------------------------------------

class GpJoint
{
	friend class	GpSkeleton;
	friend class	GpSkeletonDef;

public:
	enum	JointFlags
	{
		kWorldMatrixInvalid = ( 1 << 0 )												
	};
	
	FwQuat			GetRotation( void )		const			{ return m_rotation; }
	FwPoint			GetTranslation( void )	const			{ return m_translation; }
	FwVector4		GetScale( void )		const			{ return m_scale; }

	FwQuat			m_rotation;
	FwPoint			m_translation;
	FwVector4		m_scale;
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpSkeletalLod
	
	@brief			
**/
//--------------------------------------------------------------------------------------------------

class GpSkeletalLod : public FwNonCopyable
{
public:
	u32									GetNumLevels( void ) const		{ return m_numLevels; }
	u16									GetLevelCount( uint i ) const	{ FW_ASSERT( i < m_numLevels ); return m_levelCounts[i]; }
	const u16 *							GetRemapTables( void ) const	{ return &m_levelCounts[m_numLevels]; }

private:
	u32									m_numLevels;
	u16									m_levelCounts[1];
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpSkeletonDef
	
	@brief			This is the exported skeletal definition.			
**/
//--------------------------------------------------------------------------------------------------

class GpSkeletonDef : public FwNonCopyable
{
public:
	static	GpSkeletonDef*		Create( GpSkeletonJointDef& rootJoint, void* pMemory = NULL );
	static	int					QuerySizeInBytes( GpSkeletonJointDef& rootJoint, int* pJointCount = NULL );
	static	void				Destroy( GpSkeletonDef* pSkeletonDef );
		
	u32							GetTag( void ) const					{ return m_tag; }
	u32							GetSkeletonKey( void ) const			{ return m_skeletonKey; }
	s32							GetJointCount( void ) const				{ return m_jointCount; }
	const	GpJointLinkage*		GetJointLinkageArray( void ) const		{ return m_jointLinkageArray.Get(); }
	const	GpJoint*			GetJointBindPoseArray( void ) const		{ return m_jointBindPoseArray.Get(); }
	const	GpJointName*		GetJointNameArray( void ) const			{ return m_jointNameArray.Get(); }
	const	GpDrivenKeysDef*	GetDrivenKeysDef( void ) const			{ return m_drivenKeysDef.Get(); }
	const	GpSkeletalLod*		GetSkeletalLod( void ) const			{ return m_skeletalLod.Get(); }

	static	u32					GetClassTag( void )						{ return FW_MAKE_TAG( 'S','K','0','6' ); }

//private:
	static	void	PopulateFromJointData( GpSkeletonJointDef& joint, GpJointLinkage* pJointLinkage, GpJoint* pJointBindPose, GpJointName* pAnimClipBindObject );
	static	void	SortJointNameObjects( GpJointName* pJointNameArray, int arrayStart, int arrayEnd );

	u32									m_tag;
	u32									m_skeletonKey;
	s16									m_ownsMemory;
	s16									m_jointCount;
	FwOffset<const GpJointLinkage>		m_jointLinkageArray;
	FwOffset<const GpJoint>				m_jointBindPoseArray;
	FwOffset<const GpJointName>			m_jointNameArray;	
	FwOffset<const GpDrivenKeysDef>		m_drivenKeysDef;
	FwOffset<const GpSkeletalLod>		m_skeletalLod;
	u32									m_futureExpansion[ 4 ];
};


//--------------------------------------------------------------------------------------------------
/**
	@class			GpSkeleton
	
	@brief			This is a runtime instantiation of a GpSkeletonDef object.		
**/
//--------------------------------------------------------------------------------------------------

class GpSkeleton : public FwNonCopyable
{
public:
	// Note: We allow skeletons with multiple root joints. This constant is a convenience for single root skeletons.
	static	const int		kRootJointIndex	= 0;

	// Creation and destruction
	static	int				QuerySizeInBytes( const GpSkeletonDef* pSkeletonDef );
	static	GpSkeleton*		Create( const GpSkeletonDef* pSkeletonDef, void* pUserMemoryArea = NULL );
	static	void			Destroy( GpSkeleton* pSkeleton );

	// Data synchronisation
	void					Synchronise( void );

	// Master skeleton transform
	const FwTransform&		GetTransform( void ) const;
	void					SetTransform( const FwTransform& trans, bool invalidate = true );
	
	// Retrieve joint count
	int						GetJointCount( void ) const;
	
	// Retrieve joint index from a name
	int						GetJointIndex( FwHashedString jointName ) const;

	// Retrieve a joint name from an index
	FwHashedString			GetJointName( int jointIndex ) const;

	// Retrieve joint (for flag modification), and individual components
	GpJoint*				GetJoint( int jointIndex ) const;
	FwQuat					GetRotation( int jointIndex ) const;
	FwPoint					GetTranslation( int jointIndex ) const;
	FwVector4				GetScale( int jointIndex ) const;
	
	// Retrieve bind pose information
	FwQuat					GetBindRotation( int jointIndex ) const;
	FwPoint					GetBindTranslation( int jointIndex ) const;
	FwVector4				GetBindScale( int jointIndex ) const;

	// Retrieve joint indices for parents, children, and next siblings
	int						GetParentIndex( int jointIndex ) const;
	int						GetFirstChildIndex( int jointIndex ) const;
	int						GetNextSiblingIndex( int jointIndex ) const;

	// Set data for a joint
	void					SetRotation( int jointIndex, FwQuat_arg rot );
	void					SetTranslation( int jointIndex, FwPoint_arg trans );
	void					SetScale( int jointIndex, FwVector4_arg scale );

	// Retrieve local & world space matrices for a joint
	FwTransform				GetLocalMatrix( int jointIndex ) const;
	const FwTransform&		FindWorldMatrix( int jointIndex ) const;

	// Get skeletal definition structure
	const GpSkeletonDef*	GetSkeletonDef( void ) const;

	// Get driven keys instantation
	GpDrivenKeys*			GetDrivenKeys( void ) const;

	// Get skeleton key
	u32						GetSkeletonKey( void ) const;

	// Fast access for skinning
	const FwTransform*		GetWorldMatrixArrayUnsafe( void ) const;

	// Get flags array
	u8*						GetFlagsArray( void ) const;

	// Invalidate a joint (and all children)
	void					Invalidate( int jointIndex );

	// Mark all joints as valid (this is for restricted use by animation code.. do not touch!)
	void					MarkAsValid( void );

	// Locking behaviour
	void					Lock( void );
	void					Unlock( void );
	bool					IsLocked( void ) const;
	
//private:
	// We don't want anyone using constructors/destructors here..
	GpSkeleton()	{};
	~GpSkeleton()	{};

	enum
	{
		kOwnsMemory	= ( 1 << 0 ),
		kIsLocked	= ( 1 << 1 ),
	};

	static const int	kMaxJointDepth	= 64;

	s32							m_flags;
	s32							m_jointCount;

	const	GpSkeletonDef*		m_pSkeletonDef;			// Pointers to R/O data within GpSkeleton
	const	GpJointLinkage*		m_pJointLinkage;
	const	GpJoint*			m_pJointBindPose;

	u8*							m_pJointFlagsArray;		// Storage for joint flags

	GpJoint*					m_pJointArray;			// Storage for joint information (128 byte aligned start/size)
	mutable FwTransform*		m_pWorldMatrix;			// Storage for world matrices (128 byte aligned start/size );

	GpDrivenKeys*				m_pDrivenKeys;			// Driven key information

	FwTransform					m_transform;			// Master position/orientation of skeleton
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#ifdef __SPU__
#	include <../Gp/GpSkeleton/GpSkeleton.inl>
#else
#	include <Gp/GpSkeleton/GpSkeleton.inl>
#endif //__SPU__

#endif // GP_SKELETON_H
