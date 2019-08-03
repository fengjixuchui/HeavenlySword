/***************************************************************************************************
*
*	$Header:: /game/hierarchy.h 26    6/08/03 8:31 Dean                                            $
*
*	Hierarchy Processing
*
*	CHANGES
*
*	18/2/2003	Dean	Created
*
***************************************************************************************************/

#ifndef	HIERARCHY_H_
#define	HIERARCHY_H_

#include "Fw/FwMaths/FwQuat.h"
#include "Fw/FwMaths/FwPoint.h"
#include "Fw/FwMaths/FwVector4.h"

#include "Gp/GpSkeleton/GpSkeleton.h"

#if !defined( CORE_HASH_H )
#	include "core/hash.h"
#endif

#if !defined( CORE_SMARTPTR_H )
#	include "core/smartptr.h"
#endif

#include "anim/transform.h"

// Forward references
class	CClumpHeader;
class	CBindPose;
class	CSkinMatrix;
class	CTransformLinkage;
class	OneChain;

#include "anim/CharacterBoneID.h"

// Flags for the CHierarchy object 
enum	HIERARCHY_FLAGS
{
	HIERF_USER_CREATED						= ( 1 << 0 ),
	HIERF_INVALIDATED_MAIN					= ( 1 << 1 ),
	HIERF_INVALIDATED_EXTERNAL				= ( 1 << 2 ),
	HIERF_MAY_REQUIRE_SKIN_MATRIX_UPDATE	= ( 1 << 3 ),
};

/***************************************************************************************************
*	
*	CLASS			CHierarchy
*
*	DESCRIPTION		This object defines a single hierarchy. A hierarchy is essentially a management
*					structure that refers to an array of Transform objects. In all circumstances
*					the array is laid out in the order of processing as defined by the recursive
*					use of the 'm_pobNextSibling' and 'm_pobFirstChild' member variables within the
*					Transform objects. This particular layout system is extremely important as it
*					allows us to directly access other arrays of transform-related data using the
*					same positional offset as the Transform object has in the main array of 
*					transforms. 
*
*					A CHierarchy object also points directly to read-only elements defined within
*					the loaded model data. Currently this data includes the CBindPose array, an
*					array of matrices representing skin->bone transformation, and also some data
*					to facilitate quick lookup of character specific transform information  within
*					the arrays described above. More information on this mechanism will be in the
*					animation system notes.
*
*	NOTES		
*
*		Hierarchy Dirtying
*		------------------
*
*			As can be seen, each hierarchy contains linkage pointers structured in the same way
*			as those used in the Transform object above. So, each hierarchy is fully aware of 
*			child hierarchies. Obviously this approach will cost a little when connecting and 
*			removing hierarchies, but the benefit of this approach is that a resynchronisation
*			of the scene (ie checking that all Transform objects have valid world transforms) 
*			incurs the minimum possible cost.
*
*			When a transform is modified (and is made dirty - see Transform notes), the parent
*			hierarchy is flagged as dirty too (via the same mechanism described above).
*
*			At resynchronisation we bounce through the hierarchy of hierarchies. When we encounter a
*			hierarchy that is dirty we iterate through all transforms associated with that specific
*			hierarchy. When we encounter a transform that is dirty, we update all transforms that
*			are linked as children. This includes transforms that may have been manually attached,
*			and those that exist within child hierarchies. When we dirty a transform, we automatically
*			mark the parent hierarchy as invalid too.
*
*			When we need a transform in-engine (ie before resynchronisation), we can retrieve it
*			directly if it's valid. If it's	invalid, then we need to go all the way up the parent
*			hierarchy while caching pointers to transforms, then we bounce back down the call-chain
*			updating their transforms and marking them as valid again. 
*
*
*		Transform Ownership Identification
*		----------------------------------
*		
*			When we're computing our skinning matrices we need to be *very* careful that we only
*			consider Transform objects that are an integral part of our hierarchy array. Manually
*			attached Transform objects should *not* be processed, as it would break the ordering
*			relationship between the transform array and the skin-to-bone array. Similarly, attached
*			hierarchies	should not be processed. We can avoid the inclusion of Transform objects
*			outside of the original hierarchy array by simply performing an address comparison. It's
*			simple,	and it should be fast.
*
*
*		Attaching User-Created Transforms to Hierarchies
*		------------------------------------------------
*
*			When adding user-created transforms to a hierarchy, the m_pobHierarchy field within
*			the Transform object will be set to point to the parent hierarchy. We can determine
*			whether a transform is really 'owned' by a hierarchy by performing the identification
*			process described above. When we are destroying a hierarchy, we need to check that no
*			other transforms are connected to it. This process will ensure that child hierarchies
*			are destroyed first too, as their transforms will be identified as external.
*
*
*		Missing Clump Frames, and Dealing With Self-Locomoting Actions
*		--------------------------------------------------------------
*
*			After an extensive chat with John, Ben and Simon, it was decided that we should not 
*			have an extra transform acting as a parent for the root of a hierarchy. You may recall
*			that this extra transform was referred to as the 'Clump Frame' in RenderWare.. it was
*			this transform that effectively defined *where* a hierarchy was positioned. In our new
*			system, as we've decided that we're not having this extra transform, we need to be a
*			little more creative. Basically for things that locomote via animation, we will be 
*			marking the root local matrix as locked, moving it manually based on deltas that are
*			supplied by the animation system. These deltas will be calculated for the root frame 
*			*regardless* of whether the underlying local matrix is marked as locked. I think this
*			is right.. but we might need to flesh this particular system out a bit as we go on.
*
*
*		Sample Hierarchy Processing
*		---------------------------
*
*			Here's an example of how a the linkage associated with CTransforms and CHierarchy objects
*			is processed recursively. Note that the processing order of nodes must *not* change, as
*			the additional arrays of data needed by the system must be in the same order as native
*			hierarchy processing.
*
*			void CHierarchy::ExampleHierarchyProcess( void )
*			{
*				for ( CHierarchy* pobChild = m_pobFirstChild; pobChild != NULL; pobChild = pobChild->m_pobNextSibling )
*				{
*					pobChild->ExampleHierarchyProcess();
*				}
*			}
*
*			Hierarchy processing will start at a hierarchy object that represents the world.
*
***************************************************************************************************/
ALIGNTO_PREFIX( 128 ) class CHierarchy : private GpSkeleton
{
	public:
		//
		//	World construction & access
		//
		static	void		CreateWorld	();																		// Creates the world hierarchy root with a single transform
		static	void		DestroyWorld();																		// Destroys the world, but it must be empty before destruction.
		static	CHierarchy *GetWorld	()							{ return m_gpobWorldHierarchy; }			// Use this to get to the world hierarchy

		//
		//	Construction & destruction
		//
		static	CHierarchy *Create		();																		// Create a container hierarchy with one identity transform present.
		static	CHierarchy *Create		( const CClumpHeader *pstClumpHeader );									// Create a hierarchy from core data.
		static	void		Destroy		( CHierarchy* pobHierarchy );											// Destroys a hierarchy

	public:
		//
		//	Accessors
		//
		int32_t				GetFlags		() const				{ return m_iFlags; }						// Return flags
		void				SetFlags		( int32_t iFlags )		{ m_iFlags = iFlags; }						// Sets flags
		void				SetFlagBits		( int32_t iFlagBits )	{ m_iFlags |= iFlagBits; }					// Sets flag bits
		void				ClearFlagBits	( int32_t iFlagBits )	{ m_iFlags &= ~iFlagBits; }					// Clears flag bits

		const CClumpHeader *GetClumpHeader	() const;															// Returns a const pointer to the clump header used to construct this hierarchy

		CHierarchy *		GetParent					() const	{ return m_pobParent; }						// Returns a non-const pointer to parent hierarchy, or NULL
		CHierarchy *		GetNextSibling				() const	{ return m_pobNextSibling; }				// Returns a non-const pointer to next sibling hierarchy, or NULL
		CHierarchy *		GetFirstChild				() const	{ return m_pobFirstChild; }					// Returns a non-const pointer to first child hierarchy, or NULL
		uint32_t			GetHierarchyKey				() const	{ return m_uiHierarchyKey; }				// Returns the hierarchy key
		int32_t				GetTransformCount			() const	{ return m_iTransformCount; }				// Returns the number of transforms in the arrays below..		
		Transform *			GetRootTransform			() const	{ return m_pobTransformArray; }				// Returns a non-const pointer to the root transform for this hierarchy.
		CSkinMatrix *		GetSkinMatrixArray			() const	{ return m_pobSkinMatrixArray; }			// Returns a non-const pointer to the skin matrix array, or NULL if not skinned.
		const int8_t *		GetCharacterBoneToIndexArray() const	{ return m_pCharacterBoneToIndexArray; }	// Returns a const pointer to the character bone->index array.
		
		const GpJoint *		GetBindPoseJoint			( int32_t idx )						const;				// Access to bind pose array.. 
		CQuat				GetBindPoseJointRotation	( int32_t idx ) 					const;
		CPoint				GetBindPoseJointTranslation	( int32_t idx ) 					const;
		CVector				GetBindPoseJointScale		( int32_t idx ) 					const;
		const CPoint &		GetBoneOffsetForJoint		( int32_t idx )						const;
		const CPoint *		GetBoneOffsetArray			()									const	{ return m_BoneOffsetArray; }

		int32_t				GetTransformIndex			( const CHashedString &obNameHash ) const;				// Returns a transform index based on the name of the transform
		int32_t				GetTransformIndexFromHash	( uint32_t uiHash );									// Returns a transform index based on the name of the transform

		bool				DoesTransformExist			( const CHashedString &obNameHash ) const;
		Transform *			GetTransform				( int32_t iTransform )				const;				// Returns a specific transform based on supplied index. 
		Transform *			GetTransform				( const CHashedString &obNameHash ) const;
		Transform *			GetTransformFromHash		( uint32_t uiHash );
		bool				IsEmbeddedTransform			( const Transform *pobTransform )	const;				// Returns whether a transform is part of the hierarchy transform array
		int32_t				GetTransformIndex			( const Transform *transform )		const;

		Transform *			GetCharacterBoneTransform		( CHARACTER_BONE_ID eBoneID ) const;				// Return a transform based on the bone ID specified. May return NULL.
		int32_t				GetCharacterBoneTransformIndex	( CHARACTER_BONE_ID eBoneID ) const;				// Return a transform index based on the bone ID specified. May return -1 if not found.

		// I know this is bad (m_pobSkinToBoneArray is a private member) but I have no other way to pass this stuff to my SPU Module, any idea? (Marco)
		const CMatrix*		GetSkinToBoneArray() const { return m_pobSkinToBoneArray; }

		void				SetGpSkeletonRootFromParent	();
		void				ResetGpSkeletonRoot			( bool invalidate );

	public:
		//
		//	Update & Invalidation
		//
		static	void		UpdateWorld			()				{ m_gpobWorldHierarchy->Resynchronise(); }		// Updates invalid transforms in all hierarchies

		void				ResetToBindPose		();																// Resets a hierarchy back to its bind pose.
		void				UpdateSkinMatrices	();																// Updates skin matrices
		void				MarkAsRendered		();																// Marks a hierarchy as being used in a rendering operation
		bool				WasRenderedLastFrame()	const;														// Returns whether a hierarchy was used in a rendering operation in the last frame.

		void				Collapse			();																// Re-attach all the nodes in a hierarchy to the root, thus flattening it

#		ifndef _RELEASE
			// we DO NOT support scales in the game. This is debug code only so dont use it for anything else!
			void			SetScaleMatrix		( const CMatrix &scale );
#		endif

	public:
		//
		//	For CAnimator use ONLY. DO NOT CALL.
		//
		GpSkeleton *		GetGpSkeleton		()				{ return static_cast< GpSkeleton * >( this ); }
		const GpSkeleton *	GetGpSkeleton		()	const		{ return static_cast< const GpSkeleton * >( this ); }

	private:
		//
		//	The Transform object can look inside CHierarchy.
		//
		friend	class Transform;
		friend	class OneChain;		// Bad Frank!

	protected:
		//
		//	Protected Invalidate function - only callable internally or from a Transform object.
		//
		void				Invalidate			( bool bIsExternal );										// Marks the hierarchy as invalid

	private:
		//
		//	Ctor, dtor.
		//
		//	Nobody should be able to create a CHierarchy object without using our Create()
		//	methods, nor destroy without using Destroy().
		//
		CHierarchy	() {}
		~CHierarchy	();

	private:
		//
		//	Internal functions.
		//

		// Hierarchy connection
		void				AddChild			( CHierarchy* pobChild );								// Adds pobChild as a child of this hierarchy object
		void				RemoveFromParent	();														// Removes this hierarchy from any attached parent.

		// Internal functions for resynchronisation
		void				Resynchronise		();														// Resynchronise a hierarchy if necessary..
		void				Finalise			();														// Mark a hierarchy and all child hierarchies as valid, and generate skin matrices.

	private:
		//
		//	Transform-class visible helper functions.
		//
		CMatrix 			GetLocalMatrix		( int32_t idx )		const;
		CPoint 				GetLocalTranslation	( int32_t idx )		const;
		CQuat 				GetLocalRotation	( int32_t idx )		const;
		const CMatrix &		GetWorldMatrixFast	( int32_t idx )		const;

	private:
		//
		//	Data members.
		//
		static CHierarchy *	m_gpobWorldHierarchy;			// This doesn't contribute to the size of CHierarchy as an object. It's here so we can get access to the world hierarchy.

		CHierarchy *		m_pobParent;					// This is a pointer to the parent, or NULL.
		CHierarchy *		m_pobNextSibling;				// This is a pointer to the next sibling, or NULL.
		CHierarchy *		m_pobFirstChild;				// This is a pointer to the first child, or NULL. 
		int32_t				m_iFlags;						// Flags.. 

		int32_t				m_iTransformCount;				// Number of transforms/bones we have in this hierarchy. This does not include additionally added transforms. Only used to determine size of arrays.
		Transform *			m_pobTransformArray;			// Pointer to the root Transform in this hierarchy. Must not be NULL. ***ROOT TRANSFORM WILL ALWAYS BE INDEX 0***
		CSkinMatrix *		m_pobSkinMatrixArray;			// Pointer to array of final skinning matrices (4x3 transpose format) that are used by rendering (or NULL if not skinned)
		uint32_t			m_uiHierarchyKey;				// A 32-bit hash generated by the names of the transforms used to generate the hierarchy. This can be used
															// on non-character animations to stop playing them on incorrect hierarchies.

		uint32_t			m_uiLastRenderedTick;			// System tick when this hierarchy was last used in a rendering operation

		CPoint *			m_BoneOffsetArray;

		// We have some pointers to commonly used data inside the clump. I figure it's good for data locality..
		const CMatrix *		m_pobSkinToBoneArray;			// R/O pointer to skin->bone matrices in root data (or NULL if there are no skinned components). Must be in same order as transforms. Can be NULL.
		const int8_t *		m_pCharacterBoneToIndexArray;	// R/O pointer to character bone->index array.. (NULL for non-character objects).
		const CClumpHeader *m_pobClumpHeader;				// R/O pointer to the CClumpHeader used to construct this hierarchy.

#		ifndef _RELEASE
			CMatrix			m_DEBUG_SCALE_MAT;				// dont use it!
#		endif
}
ALIGNTO_POSTFIX( 128 );

// Include our inlined function bodies.
#include "anim/hierarchy.inl"

/***************************************************************************************************
*	
*	CLASS			CSkinMatrix
*
*	DESCRIPTION		This class is essentially a representation of the 4x3 matrix used by the vertex
*					shaders that perform soft skinning.
*
*	NOTES
*
*		Generating CSkinMatrix contents
*		-------------------------------
*
*			The contents of this matrix are formed by multiplying the local space matrix by	the
*			corresponding skin-to-bone transformation matrix. The resultant matrix is then transposed,
*			with the bottom row (which was originally the 4th column) being	dropped. So, each 
*			skin matrix only takes up 3 constants within the vertex shader.
*
*		
*		Having More Than 44 Bones
*		-------------------------
*
*			We have an array of 44 bones within the vertex shader constant register set. As such
*			it's impossible for us to have a single mesh that references more than 44 bones. But
*			because we can have a separate list of bone indices for each mesh we can have multiple
*			meshes that reference outside of the 44 bone range. So, for example, you could have a
*			character that used 44 bones for the body, and have a separate mesh that used a different
*			subset of 10 bones for it's rendering. We just need to make sure that a mesh has a 
*			count of how many skinned bones it uses, as well as an array of indices that contain
*			the real index of a CSkinMatrix within the array (that, again, matches the hierarchy
*			array layout). We can efficiently upload bones to vertex shader constants using the
*			BeginState()/EndState() Xbox API calls. Prefetching can be used to reduce any memory
*			bandwidth/stalling issues.
*
***************************************************************************************************/
ALIGNTO_PREFIX( 16 ) class CSkinMatrix
{
	public:
		CVector		m_obRow0;
		CVector		m_obRow1;
		CVector		m_obRow2;
}
ALIGNTO_POSTFIX( 16 );


#endif	// !HIERARCHY_H_
