//------------------------------------------------------------------------------------------
//!
//!	\file PriorityCache.cpp
//!
//------------------------------------------------------------------------------------------

#include "anim/PriorityCache.h"

#include "anim/animation.h"
#include "anim/hierarchy.h"
#include "anim/transform.h"

#ifdef PLATFORM_PC
#	pragma warning( push )
#	pragma warning( disable : 4201 )	// Turn off stupid warning about nameless structs within unions.
#endif

namespace QHelp
{
	typedef ntstd::pair< Transform *, PriorityCache::Priority > DepthPair;		// Pair to hold a Transform and it's depth down the parent hierarchy.
	typedef ntstd::List< DepthPair > DepthPairQueue;
	
	inline void Add( DepthPairQueue &q, DepthPair item )
	{
		q.push_back( item );
	}

	inline DepthPair Remove( DepthPairQueue &q )
	{
		DepthPairQueue::const_iterator it = q.begin();
		if ( it == q.end() )
		{
			return DepthPair( NULL, 0 );
		}

		DepthPair res = *it;
		q.pop_front();

		return res;
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Return the priority for this combination of animation and hierarchy.
//!
//------------------------------------------------------------------------------------------
PriorityCache::Priority PriorityCache::CalculatePriority( const CAnimation *anim, const CHierarchy *hierarchy ) const
{
	const FpAnimClipDef *clip_def = anim->GetGpAnimation()->GetAnimClipDef();

/*
	ntPrintf( "anim short name hash = 0x%08X, hierarchy key = 0x%08X\n", anim->GetShortNameHash(), hierarchy->GetHierarchyKey() );
	ntPrintf( "Skeleton hashes, num joints = %i\n", hierarchy->GetGpSkeleton()->GetJointCount() );
	for ( int32_t i=0;i<hierarchy->GetGpSkeleton()->GetJointCount();i++ )
	{
		FwHashedString name = hierarchy->GetGpSkeleton()->GetJointName( i );
		ntPrintf( "Joint %i has hashed name 0x%08X\n", i, name.Get() );
	}
	

	ntPrintf( "\nAnim hashes, num joints = %i\n", clip_def->GetItemCount() );
	for ( int32_t i=0;i<clip_def->GetItemCount();i++ )
	{
		const FpAnimItem *item = clip_def->GetItemArray() + i;
		FwHashedString name = item->GetItemName();

		int32_t skel_item_idx = hierarchy->GetGpSkeleton()->GetJointIndex( name );
		int32_t hierarchy_depth = FindHierarchyDepth( hierarchy, skel_item_idx );

		ntPrintf( "Joint %i has hashed name 0x%08X = joint %i on skeleton at depth %i\n", i, name.Get(), skel_item_idx, hierarchy_depth );
	}
*/
	// Is the root present?
	if ( clip_def->GetItemIndex( AnimNames::root ) != -1 )
	{
		// Yes - no point in checking anything else...
		return Priority( 0 );
	}

	const GpSkeleton *skeleton = hierarchy->GetGpSkeleton();

	int32_t min_index = -1;

	for ( int16_t i=0;i<clip_def->GetItemCount();i++ )
	{
		const FpAnimItem *item = clip_def->GetItemArray() + i;
		ntError( item != NULL );

		FwHashedString item_name = item->GetItemName();

		int32_t skel_item_idx = skeleton->GetJointIndex( item_name );
		int32_t hierarchy_depth = FindHierarchyDepth( hierarchy, skel_item_idx );

		if ( hierarchy_depth != -1 )
		{
			// This FpAnimItem is present in the GpSkeleton as well.

			if ( min_index == -1 )
				min_index = hierarchy_depth;
			else
				min_index = min( min_index, hierarchy_depth );

			ntError( min_index != -1 );
		}
	}

	if ( min_index == -1 || min_index == 0 )
	{
		// Either this animation wasn't meant for this hierarchy (in which case we don't care
		// about the priority) or it's a proper, non-partial, animation, so return 0.
		return Priority( 0 );
	}

	return min_index;
}

int32_t PriorityCache::FindHierarchyDepth( const CHierarchy *hierarchy, int32_t joint_index ) const
{
	if ( joint_index == -1 )
		return -1;

	// Partial-anims are the only things that should get through to here.
	// We need to run through the hierarchy in a breadth-first manner to
	// find how far down it this joint index is.
	using namespace QHelp;
	DepthPairQueue queue;	// Need a queue to do breadth first traversal.

	// Start with the root.
	Add( queue, DepthPair( hierarchy->GetRootTransform(), 0 ) );

	DepthPair curr = QHelp::Remove( queue );
	ntError( hierarchy->IsEmbeddedTransform( curr.first ) );		// Root should always be embedded...

	while ( curr.first != NULL )
	{
		//
		//	If curr is active in this animation then return its priority.
		//

		// Grab the joint/item name from the animation.
		ntError( hierarchy->IsEmbeddedTransform( curr.first ) );
		int32_t transform_idx = curr.first->GetIndex();

		if ( transform_idx == joint_index )
		{
			return curr.second;
		}

		//
		//	Add all the children of the current transform to the queue.
		//
		ntError( curr.first != NULL );
		for ( Transform *child = curr.first->GetFirstChild(); child != NULL; child = child->GetNextSibling() )
		{
			// Add the child, incrementing the priority from its parent.
			if ( hierarchy->IsEmbeddedTransform( child ) )
			{
				Add( queue, DepthPair( child, curr.second + 1 ) );
			}
		}
		
		//
		//	Move onto the next transform.
		//
		curr = Remove( queue );
	}

	return -1;
}

//------------------------------------------------------------------------------------------
//!
//!	Return the hash for this combination of animation and hierarchy.
//!
//------------------------------------------------------------------------------------------
uint64_t PriorityCache::GetHash( const CAnimation *anim, const CHierarchy *hierarchy ) const
{
	union Combine
	{
		struct
		{
			uint32_t	m_AnimKey;
			uint32_t	m_HierarchyKey;
		};

		uint64_t		m_Hash;
	};
	Combine combine;
	combine.m_AnimKey			= anim->GetShortNameHash();
	combine.m_HierarchyKey	= hierarchy->GetHierarchyKey();

	return combine.m_Hash;
}

//------------------------------------------------------------------------------------------
//!
//!	Ctor.
//!
//------------------------------------------------------------------------------------------
PriorityCache::PriorityCache()
{
}

//------------------------------------------------------------------------------------------
//!
//!	Dtor.
//!
//------------------------------------------------------------------------------------------
PriorityCache::~PriorityCache()
{
}

#ifdef PLATFORM_PC
#	pragma warning( pop )
#endif








