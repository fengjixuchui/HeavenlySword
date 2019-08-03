//------------------------------------------------------------------------------------------
//!
//!	\file PriorityCache.h
//!
//------------------------------------------------------------------------------------------

#ifndef	PRIORITY_CACHE_H_
#define	PRIORITY_CACHE_H_

class	CAnimation;
class	CHierarchy;

class PriorityCache : public Singleton< PriorityCache >
{
	public:
		//
		//	Cache access.
		//
		typedef int32_t Priority;
		inline Priority	GetPriority		( const CAnimation *anim, const CHierarchy *hierarchy );

	public:
		//
		//	Ctor, dtor.
		//
		PriorityCache();
		~PriorityCache();

	private:
		//
		//	Prevent copying or assignment.
		//
		PriorityCache( const PriorityCache & )				NOT_IMPLEMENTED;
		PriorityCache &operator = ( const PriorityCache & )	NOT_IMPLEMENTED;

	private:
		//
		//	Helper functions.
		//
		typedef uint64_t HashValue;
		uint64_t	GetHash				( const CAnimation *anim, const CHierarchy *hierarchy )	const;
		Priority	CalculatePriority	( const CAnimation *anim, const CHierarchy *hierarchy )	const;
		int32_t		FindHierarchyDepth	( const CHierarchy *hierarchy, int32_t joint_index )	const;

	private:
		//
		//	Internal data.
		//
		typedef ntstd::Map< HashValue, Priority >	PriorityCacheType;
		PriorityCacheType m_Cache;
};

PriorityCache::Priority PriorityCache::GetPriority( const CAnimation *anim, const CHierarchy *hierarchy )
{
	HashValue hash = GetHash( anim, hierarchy );

	PriorityCacheType::const_iterator it = m_Cache.find( hash );
	if ( it == m_Cache.end() )
	{
		Priority priority = CalculatePriority( anim, hierarchy );
		m_Cache.insert( PriorityCacheType::value_type( hash, priority ) );

		return priority;
	}

	return it->second;
}

#endif // !PRIORITY_CACHE_H_

