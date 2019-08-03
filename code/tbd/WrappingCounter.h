/***************************************************************************************************
*
*	DESCRIPTION		A counter that wraps to zero instead of overflowing.
*
*	NOTES
*
***************************************************************************************************/

#ifndef WRAPPINGCOUNTER_H_
#define WRAPPINGCOUNTER_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	A counter that wraps to zero instead of overflowing.
//**************************************************************************************
template
<
	typename	CounterType,
	CounterType	MaxValue
>
class WrappingCounterT
{
	public:
		//
		//	Conversion to CounterType.
		//
		inline CounterType	Convert	()	const	{ return m_Counter; }

	public:
		//
		//	Overload increments and decrements.
		//
		inline WrappingCounterT &operator ++ ()		// Pre-increment.
		{
			m_Counter = m_Counter < MaxValue ? m_Counter+1 : CounterType( 0 );
			return *this;
		}

		inline WrappingCounterT operator ++ ( int )	// Post-increment.
		{
			WrappingCounterT temp( *this );
			m_Counter = m_Counter < MaxValue ? m_Counter+1 : CounterType( 0 );
			return temp;
		}

		inline WrappingCounterT &operator -- ()		// Pre-decrement.
		{
			m_Counter = m_Counter > CounterType( 0 ) ? m_Counter-1 : MaxValue;
			return *this;
		}

		inline WrappingCounterT operator -- ( int )	// Post-decrement.
		{
			WrappingCounterT temp( *this );
			m_Counter = m_Counter > CounterType( 0 ) ? m_Counter-1 : MaxValue;
			return temp;
		}

	public:
		//
		//	Overload comparison operators.
		//
		inline bool operator == ( const WrappingCounterT &rhs ) const
		{
			if ( m_Counter == rhs.m_Counter )
				return true;

			return false;
		}

		inline bool operator == ( CounterType rhs ) const
		{
			if ( m_Counter == rhs )
				return true;

			return false;
		}

		inline bool operator != ( const WrappingCounterT &rhs ) const
		{
			if ( m_Counter != rhs.m_Counter )
				return true;

			return false;
		}

		inline bool operator != ( CounterType rhs ) const
		{
			if ( m_Counter != rhs )
				return true;

			return false;
		}

	public:
		//
		//	Ctors, dtor, assignment operators.
		//
		explicit WrappingCounterT()
		:	m_Counter( CounterType( 0 ) )
		{}

		~WrappingCounterT() {}

		WrappingCounterT( const WrappingCounterT &copy )
		{
			m_Counter = copy.m_Counter;
		}

		WrappingCounterT &operator = ( const WrappingCounterT &rhs )
		{
			m_Counter = rhs.m_Counter;
			return *this;
		}

		WrappingCounterT &operator = ( CounterType rhs )
		{
			m_Counter = rhs;
			return *this;
		}

	private:
		//
		//	The counter object.
		//
		CounterType		m_Counter;
};

typedef WrappingCounterT< unsigned int, 0xffffffff >	WrappingCounter;

#endif	// !WRAPPINGCOUNTER_H_

