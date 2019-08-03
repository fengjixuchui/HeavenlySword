/***************************************************************************************************
*
*	DESCRIPTION		Generic queue class.
*
*	NOTES
*
***************************************************************************************************/

#ifndef QUEUE_H_
#define QUEUE_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
template < typename Type >
class Queue 
{
	public:
		//
		//	Insertion/Removal to/from the queue.
		//
		void			AddItem		( const Type &item )
		{
			m_Data.push_back( item );
		}

		Type			RemoveItem	()
		{
			typename ntstd::List< Type >::iterator it( m_Data.begin() );
			Type head( *it );
			m_Data.erase( it );
			return head;
		}

	public:
		//
		//	Return the number of items in the queue.
		//
		size_t			GetNumItems	()	const
		{
			return m_Data.size();
		}

	public:
		//
		//	Ctors, assignment operator.
		//
		Queue() {}
		Queue( const Queue< Type > &copy )
		{
			*this = copy;
		}
		Queue< Type > &operator = ( const Queue< Type > &rhs )
		{
			m_Data = rhs.m_Data;
			return *this;
		}
		
	private:
		//
		//	Implement with a list for now.
		//
		ntstd::List< Type >	m_Data;
};

#endif	// !QUEUE_H_

