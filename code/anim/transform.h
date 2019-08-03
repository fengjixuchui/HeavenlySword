/***************************************************************************************************
*
*	$Header:: /game/transform.h 13    4/08/03 16:54 Dean                                           $
*
*	Transform Processing
*
*	CHANGES
*
*	18/2/2003	Dean	Created
*
***************************************************************************************************/

#ifndef	TRANSFORM_H_
#define	TRANSFORM_H_

#include "Gp/GpSkeleton/GpSkeleton.h"

class	CHierarchy;

// Flags for the Transform object 
enum	TRANSFORM_FLAGS
{
	TRANSF_WORLD_MATRIX_INVALID		= ( 1 << 0 ),		// 0x00000001 : Is world matrix invalid?
									
	// I've left as much space (from 1<<1 to 1<<3) as I can as a guard against ATGs GpJoint
	// defining more flags - we MUST NEVER clash with their flags and we MUST ALWAYS
	// make sure that TRANSF_WORLD_MATRIX_INVALID is equal to GpJoint::kWorldMatrixInvalid;
	// this is asserted for at compile-time in Hierarchy.cpp. [ARV].

	TRANSF_WORLD_ROTATION_INVALID	= ( 1 << 4 ),		// 0x00000010 : Is world rotation invalid?
//	TRANSF_LOCAL_MATRIX_INVALID		= ( 1 << 5 ),		// 0x00000020 : Is the local matrix invalid? Remember it's now calculated and cached from the decompose joint representation.

	TRANSF_IS_WORLD_ROOT			= ( 1 << 6 ),		// 0x00000040 : Set for the transform that's used to define the world root
	TRANSF_IS_EXTERNAL				= ( 1 << 7 ),		// 0x00000080 : Set if the Transform was externally created
};

/***************************************************************************************************
*	
*	CLASS			Transform
*
*	DESCRIPTION		This class is the building block of our entire world. Transform objects can be
*					seen as forming a hierarchy through the linkage pointers defined at the start
*					of the object.
*
*	NOTES
*
*		Transform Dirtying
*		------------------
*
*			When we update a transform object, we mark the current transform as dirty. We also set
*			the parent hierarchy to dirty too, and recurse thorough any child transforms. This
*			includes transforms that extend outside	of the hierarchy that the current transform may
*			be a part of. 
*
***************************************************************************************************/
ALIGNTO_PREFIX( 16 ) class Transform
{
	public:
		//
		//	Connection.
		//
		void			AddChild		( Transform *pobChild );											// Adds pobChild as a child of this Transform object
		void			RemoveFromParent();																	// Removes this transform from any attached parent.

	public:
		//
		//	Accessors.
		//
		int32_t			GetFlags		()						const;										// Return flags
		void			SetFlags		( int32_t iFlags )		const;										// Sets flags
		void			SetFlagBits		( int32_t iFlagBits )	const;										// Sets flag bits
		void			ClearFlagBits	( int32_t iFlagBits )	const;										// Clears flag bits

		Transform *		GetParent		() 						const		{ return m_pobParent; }			// Returns a non-const pointer to parent transform, or NULL
		Transform *		GetNextSibling	() 						const		{ return m_pobNextSibling; }	// Returns a non-const pointer to next sibling transform, or NULL
		Transform *		GetFirstChild	() 						const		{ return m_pobFirstChild; }		// Returns a non-const pointer to first child transform, or NULL

		const CMatrix &	GetWorldMatrix		() 					const;										// Returns the transforms world matrix. Note: May recurse upwards if update required
		const CPoint &	GetWorldTranslation () 					const		{ return GetWorldMatrix().GetTranslation(); }

		CMatrix 		GetLocalMatrix		()					const;										// Returns a const reference to the transforms local matrix
		CPoint			GetLocalTranslation () 					const;
		CQuat 			GetLocalRotation	() 					const;
		const GpJoint &	GetLocalSpace		()					const;

		void			SetLocalMatrix		( const CMatrix &obMatrix );									// Returns a non-const reference to the transforms local matrix
		void			SetLocalTranslation	( const CPoint &obPoint );
		void			SetLocalRotation	( const CQuat &rot ); 
		void			SetLocalSpace		( const CQuat &rot, const CPoint &pos );
		void			SetLocalSpace		( const GpJoint &space );
		void			SetLocalMatrixFromWorldMatrix( const CMatrix &obWorldMatrix );						// Sets the local matrix such that the resultant world matrix holds that specified by obWorldMatrix

		// Returns the current world matrix. Note: This does *NOT* perform validity checking!!
		const CMatrix &	GetWorldMatrixFast	() 					const;
		CQuat			GetWorldRotation	() 					const;											// Returns a quaternion representing world rotation (calculates and caches if necessary)

		CHierarchy *	GetParentHierarchy	() 					const		{ return m_pobParentHierarchy; }	// Returns a non-const pointer to parent hierarchy, or NULL

		void *			GetExternalData		() 					const		{ return m_pobExternalData; }		// Returns a pointer to any additional data.
		void			SetExternalData		( void *pvData )				{ m_pobExternalData = pvData; }		// Sets a pointer to any additional data.

		// Returns whether the matrix/rotation is valid or not..
		bool			IsWorldMatrixValid	() 					const		{ return !( GetFlags() & TRANSF_WORLD_MATRIX_INVALID ); }
		bool			IsWorldRotationValid() 					const		{ return !( GetFlags() & TRANSF_WORLD_ROTATION_INVALID ); }

		int32_t			GetIndex			()					const		{ ntError( m_pobExternalData == NULL  ); return m_HierarchyIndex; }

	public:
		//
		//	Hierarchy-driven resynchronisation
		//
		void			ForceResynchronise	();						// This actually does the resynchronisation on a Transform, performing recursive resyncs on children too.
		void			Resynchronise		();						// Starting point for resynchronisation processing in dirty hierarchies

	public:
		//
		//	Ctor.
		//
		Transform();
		~Transform();

	private:
		//
		//	Hierarchies and Animators can look in here.
		//
		friend	class CHierarchy;
		friend	class CAnimator;

	private:
		//
		//	Invalidation.
		//
		void			Invalidate			() const;				// Invalidates this transforms world-space data.. this invalidates all child transforms too.

	private:
		//
		//	Internal helper functions.
		//
		void			SetWorldMatrixDirectly	( const CMatrix &matrix ) const;

	private:
		//
		//	Data members.
		//
		static const int32_t		iMAX_TRANSFORM_DEPTH = 48;

		//
		//	These three members are only used for external transforms - for internal
		//	transforms the hierarchy-index is used to look up the relevant information
		//	from the parent hierarchy's arrays.
		//
		mutable	CQuat				m_obWorldRotation;				// World rotation formed from m_obWorldMatrix. The value is cached as appropriate.

		struct NonHierarchyTransformData
		{
			GpJoint					m_LocalSpace;					// Quat+Pos representation of local space.
			mutable	CMatrix			m_obWorldMatrix;				// World space matrix.
			mutable int32_t			m_iFlags;
		};
		NonHierarchyTransformData *	m_TransformData;

		Transform *					m_pobParent;					// Linkage and flags.
		Transform *					m_pobNextSibling;		
		Transform *					m_pobFirstChild;		

		mutable	CHierarchy *		m_pobParentHierarchy;			// Pointer to the parent hierarchy, or NULL
		void *						m_pobExternalData;				// A pointer to additional/external data. 

		static const int32_t		InvalidIndex = -1;
		int32_t						m_HierarchyIndex;				// Index in parent hierarchy's arrays (local matrix etc...).
}
ALIGNTO_POSTFIX( 16 );

// Include inlines.
#include "anim/transform.inl"

#endif	// !TRANSFORM_H_

