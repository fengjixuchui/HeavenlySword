//------------------------------------------------------------------------------------------
//!
//!	\file animevents.h
//!
//------------------------------------------------------------------------------------------

#ifndef REGISTERED_H_
#define REGISTERED_H_

template < class Type >
class Registered
{
	public:
		static Type *	GetFirst()	{ return m_ListHead; }
		Type *			GetNext	()	{ return m_Next; }
		Type *			GetPrev	()	{ return m_Prev; }

	public:
		Registered()
		:	m_Next( NULL )
		,	m_Prev( NULL )
		{
			AddToHeadOfList();
		}

		~Registered()
		{
			RemoveFromList();
			ntError( m_Next == NULL );
			ntError( m_Prev == NULL );
		}

	private:
		inline void AddToHeadOfList();
		inline void RemoveFromList();

	private:
		static Type *	m_ListHead;
		Type *			m_Next;
		Type *			m_Prev;
};

template < class Type >
Type *Registered< Type >::m_ListHead = NULL;

template < class Type >
void Registered< Type >::AddToHeadOfList()
{
	Type *me = static_cast< Type * >( this );

	ntError( m_Prev == NULL );
	ntError( m_Next == NULL );

	m_Next = m_ListHead;
	m_ListHead = me;

	if ( m_Next )
	{
		static_cast< Registered< Type > * >( m_Next )->m_Prev = me;
	}
}

template < class Type >
void Registered< Type >::RemoveFromList()
{
	if ( !m_Prev )
	{
		if ( m_Next )
		{
			static_cast< Registered< Type > * >( m_Next )->m_Prev = NULL;
		}

		if ( this == m_ListHead )
		{
			m_ListHead = m_Next;
		}
	}
	else
	{
		if ( m_Next )
		{
			static_cast< Registered< Type > * >( m_Next )->m_Prev = m_Prev;
		}
		static_cast< Registered< Type > * >( m_Prev )->m_Next = m_Next;
	}

	m_Prev = NULL;
	m_Next = NULL;
}

#endif // !REGISTERED_H_

