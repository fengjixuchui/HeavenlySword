
//----------------------------------------------------------------------------------------
//! 
//! \filename core/PriorityQ.h
//! 
//----------------------------------------------------------------------------------------
#ifndef PRIORITYQ_H_
#define PRIORITYQ_H_

//
//	PriorityQ class.
//
//	The Traits template parameter should be a structure containing a typedef for "Item" type
//	and a static comparison function "LessThanOrEqual" taking two "Item"s as parameters and returning a bool.
//
template < class Traits >
class PriorityQ
{
	public:
		//
		//	Some typedefs.
		//
		typedef PriorityQ< Traits >				This;
		typedef typename Traits::Item			Item;

	public:
		//
		//	Returns true if the PQ is empty, false otherwise.
		//
		bool IsEmpty() const { return ( mHeap.size() == 0 ); }

	public:
		//
		//	Inserts an item into the PQ.
		//
		void Insert( const Item &v )
		{
			mHeap.push_back( v );
			FixUp( mHeap.size() - 1 );
		}

	public:
		//
		//	Removes the maximum item from the PQ and returns it.
		//
		Item DeleteMaximum()
		{
			ntError_p( mHeap.size() > 0, ("There is nothing to delete.") );

			Item temp( mHeap[ 0 ] );
			mHeap[ 0 ] = mHeap[ mHeap.size() - 1 ];
			mHeap[ mHeap.size() - 1 ] = temp;

			FixDown( 0, mHeap.size() - 1 );

			Item ret_i( mHeap[ mHeap.size() - 1 ] );
			mHeap.erase( --mHeap.end() );

			return ret_i;
		}

	public:
		//
		//	Returns the maximum item in the PQ without removing it.
		//
		const Item &GetMaximum() const
		{
			return mHeap[ 0 ];
		}

	public:
		//
		//	GetItems() returns the array of items that make up the PQ.
		//
		const ntstd::Vector< Item > &	GetItems() const	{ return mHeap; }
		ntstd::Vector< Item > &			GetItems()			{ return mHeap; }		// If you alter positions or priorities then the Q won't work anymore!

		//
		//	Returns the number of items in the queue.
		//
		uint32_t GetNumItems() const { return mHeap.size(); }

	public:
		//
		//	Debug function to verify the heap.
		//
		bool CheckHeap()
		{
			for ( uint32_t i=1;i<mHeap.size();i++ )
			{
				// Check that each child has a lesser value than that of its parent.
				if ( !Traits::LessThanOrEqual( mHeap[ i ], mHeap[ GetParent( i ) ] ) )
				{
					return false;
				}
			}

			return true;
		}

	public:
		//
		//	Ctor, dtor.
		//
		PriorityQ()
		{}

		PriorityQ( uint32_t pre_alloc_size )
		{
			mHeap.reserve( pre_alloc_size );
		}

		~PriorityQ()
		{}

	private:
		//
		// Helper functions to heapify the array from different locations...
		//

		// Bottom-up heapify.
		void FixUp( uint32_t k )
		{
			while ( k > 0 && Traits::LessThanOrEqual( mHeap[ GetParent( k ) ], mHeap[ k ] ) )
			{
				Item temp = mHeap[ k ];
				mHeap[ k ] = mHeap[ GetParent( k ) ];
				mHeap[ GetParent( k ) ] = temp;

				k = GetParent( k );
			}
		}

		// Top-down heapify.
		void FixDown( uint32_t k, uint32_t N )
		{
			uint32_t j;
			while ( GetFirstChild( k ) < N )
			{
				j = GetFirstChild( k );
				if ( j < N-1 && Traits::LessThanOrEqual( mHeap[ j ], mHeap[ j + 1 ] ) )
					j++;

				if ( !Traits::LessThanOrEqual( mHeap[ k ], mHeap[ j ] ) )
					break;

				Item temp( mHeap[ k ] );
				mHeap[ k ] = mHeap[ j ];
				mHeap[ j ] = temp;

				k = j;
			}
		}

		// Index manipulation functions to give parent from index and first child from index.
		inline uint32_t GetParent		( uint32_t k )	const	{ return ( k - 1 ) / 2; }
		inline uint32_t GetFirstChild	( uint32_t k )	const	{ return 2 * k + 1; }

	private:
		//
		// Aggregated members.
		//
		ntstd::Vector< Item >	mHeap;			// The heap array.
};

#endif // !PRIORITYQ_H_

