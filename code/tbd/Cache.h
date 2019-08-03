/***************************************************************************************************
*
*	DESCRIPTION		Class to represent a generic cache.
*
*	NOTES
*
***************************************************************************************************/

#ifndef CACHE_H_
#define CACHE_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
template
<
	typename	Type,
	int			CacheSize
>
class Cache
{
	public:
		//
		//	Existence in/Insertion to cache
		//
		bool		Exists	( const Type &obj )					const
		{
			for ( typename CacheArray::const_iterator it = m_CacheArray.begin();
				  it != m_CacheArray.end();
				  ++it )
			{
				if ( ( *it ) == obj )
				{
					return true;
				}
			}

			return false;
		}

		bool		Get		( Type &obj )					const
		{
			for ( typename CacheArray::const_iterator it = m_CacheArray.begin();
				  it != m_CacheArray.end();
				  ++it )
			{
				if ( ( *it ) == obj )
				{
					obj = *it;
					return true;
				}
			}

			return false;
		}

		void		Insert	( const Type &obj )
		{
			ntError( !Exists( obj ) );

			if ( m_CacheArray.size() == CacheSize )
			{
				RemoveItem();
			}

			m_CacheArray.push_back( obj );
			m_CounterArray.push_back( m_Counter++ );
		}

	public:
		//
		//	Ctors, dtor, assignment operator.
		//
		Cache()
		:	m_Counter( 0 )
		{}

		~Cache()
		{}

		Cache( const Cache &copy )
		{
			*this = copy;
		}

		Cache &operator = ( const Cache &rhs )
		{
			m_CacheArray = rhs.m_CacheArray;
			m_CounterArray = rhs.m_CounterArray;
			m_Counter = rhs.m_Counter;
			return *this;
		}

	private:
		//
		//	Helper function to remove the "oldest" item from the map.
		//
		void	RemoveItem	()
		{
			ntError( m_CacheArray.size() > 0 );

			int64_t oldest_count( -1 );
			CounterArray::iterator oldest_counter_it;
			typename CacheArray::iterator oldest_object_it;

			CounterArray::iterator counter_it;
			typename CacheArray::iterator cache_it;
			for (	counter_it = m_CounterArray.begin(),
					cache_it = m_CacheArray.begin();

					counter_it != m_CounterArray.end(),
					cache_it != m_CacheArray.end();

					counter_it++,
					cache_it++ )
			{
				if ( oldest_count < *counter_it )
				{
					oldest_count = *counter_it;
					oldest_counter_it = counter_it;
					oldest_object_it = cache_it;
				}
			}

			ntError( oldest_count >= 0 );

			m_CacheArray.erase( oldest_object_it );
			m_CounterArray.erase( oldest_counter_it );
		}

	private:
		//
		//	Aggregated members.
		//
		typedef ntstd::Vector< Type >		CacheArray;
		typedef ntstd::Vector< int64_t >	CounterArray;

		CacheArray		m_CacheArray;
		CounterArray	m_CounterArray;

		int64_t			m_Counter;
};

#endif	// !CACHE_H_

