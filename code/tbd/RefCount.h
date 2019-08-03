/***************************************************************************************************
*
*	DESCRIPTION		Base class to provide reference counting functionality.
*
*	NOTES
*
***************************************************************************************************/

#ifndef REFCOUNT_H_
#define REFCOUNT_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	Base class to provide reference counting functionality.
//**************************************************************************************
class RefCount 
{
	public:
		//
		//	Reference count manipulation.
		//
				// Increment the ref-count, returns the reference count.
		int		AddRef			()	const	{ return ++m_RefCount; }

				// Decrements the ref-count and deletes the memory if appropriate.
		void	Release			()	const
		{
			ntAssert_p( m_RefCount > 0, ("This object should already be deleted?") );
			--m_RefCount;

			if ( m_RefCount == 0 )
			{
				NT_DELETE( this );
			}
		}

	public:
		//
		//	Get the reference count, does not modify the ref-count.
		//
		int			GetRefCount		()	const	{ return m_RefCount; }

	protected:
		//
		//	Protected ctor, dtor.
		//
		RefCount() : m_RefCount( 1 )
		{}

		virtual ~RefCount()
		{
			ntAssert_p( m_RefCount == 0, ("Deleting a reference counted object whose ref-count is not zero!") );
		}

	private:
		//
		//	The reference count itself.
		//
		mutable int			m_RefCount;		// This is mutable because you want to be able to AddRef
											// and Release pointers to constant objects.
};

#endif	// !REFCOUNT_H_

