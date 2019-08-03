/***************************************************************************************************
*
*	$Header:: /game/smartptr.h 8     16/06/03 14:38 Simonb                                         $
*
*	Smart Pointer Classes
*
*	CHANGES
*
*	5/3/2003	Simon Created
*
***************************************************************************************************/

#ifndef CORE_SMARTPTR_H
#define CORE_SMARTPTR_H

/***************************************************************************************************
*
*	CLASS			CNonCopyable
*
*	DESCRIPTION		A class that makes subclasses non-copyable.
*
***************************************************************************************************/

//! A marker class to inherit from to disallow copy-construction and assignment.
/*! This only declares the copy-constructor and assignment operator as private
	so that any classes that inherit it cannot generate a default copy-constructor
	or assignment operator. It therefor has no runtime overhead whatsoever.
*/
class CNonCopyable
{
public:
	//! Public constructor.
	CNonCopyable() {}

private:
	//! Private copy-constructor declaration.
	CNonCopyable(CNonCopyable const&);

	//! Private assignment operator declaration.
	CNonCopyable& operator=(CNonCopyable const&);
};

/***************************************************************************************************
*
*	CLASS			COperatorDeleter
*
*	DESCRIPTION		A class to destroy using NT_DELETE.
*
***************************************************************************************************/

//! A deleter that does so using NT_DELETE_CHUNK and a specified chunk type (C)
template<typename T, int C>
class COperatorDeleter
{
public:
	//! The class operator deletes a resource via NT_DELETE_CHUNK
	void operator()(T* const& pResource) 
	{ 
		typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
		(void) sizeof(type_must_be_complete);
		NT_DELETE_CHUNK( C, pResource ); // ANSI says safe even when 0
	}
};

/***************************************************************************************************
*
*	CLASS			CArrayOperatorDeleter
*
*	DESCRIPTION		A class to destroy using NT_DELETE_ARRAY
*
***************************************************************************************************/

//! A deleter that does so using NT_DELETE_ARRAY_CHUNK and a specified chunk type (C)
template<typename T, int C>
class CArrayOperatorDeleter
{
public:
	//! The class operator deletes a resource via NT_DELETE_ARRAY_CHUNK
	void operator()(T* const& pResource) 
	{ 
		typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
		(void) sizeof(type_must_be_complete);
		NT_DELETE_ARRAY_CHUNK( C, pResource ); // ANSI says safe even when 0
	}
};

/***************************************************************************************************
*
*	CLASS			CScopedBase
*
*	DESCRIPTION		The base scoped template to a resource.
*
***************************************************************************************************/

//! A generic base scoped smart pointer template.
/*! Use for any resources that will require deletion when it goes out of scope.
*/
template<typename T, typename Deleter>
class CScopedBase : CNonCopyable
{
public:
	//! Creates an empty smart pointer.
	CScopedBase() : m_pResource(0) {}

	//! Creates a smart pointer around the given resource.
	/*! The smart pointer assumes ownership of the resource and will
		free it when it goes out of scope.
	*/
	explicit CScopedBase(T* pResource) : m_pResource(pResource) {}

	//! Deletes the encapsulated resource.
	~CScopedBase()
	{
		Free();
	}

	//! Retrieves the contents of the smart pointer.
	/*! \return		The contents of the smart pointer, or 0 if it is empty.
	*/
	T* Get() const { return m_pResource; }

	//! Resets the contents of the smart pointer.
	/*! The previous contents of the smart pointer are deleted. Calling Reset
		with no arguments or an argument of 0 effectively will result in an
		empty smart pointer. The smart pointer will assume ownership of any 
		non-zero resource and will free it when it goes out of scope.
		\param pNewResource		The new contents of the smart pointer, or 0
								to create an empty smart pointer.
	*/
	void Reset(T* pNewResource = 0)
	{
		CScopedBase<T, Deleter> pobTemp(pNewResource);
		Swap(pobTemp);
	}

	//! Swaps the contents of this smart pointer with that of another.
	/*! This function is useful to alter the scoping of resource in a safe manner.
	*/
	void Swap(CScopedBase<T, Deleter>& pobSmartPtr)
	{
		T* pTemp = m_pResource;
		m_pResource = pobSmartPtr.m_pResource;
		pobSmartPtr.m_pResource = pTemp;
	}

	//! Standard equality operator.
	bool operator==(CScopedBase<T, Deleter> const& pobSmartPtr) const
	{
		return m_pResource == pobSmartPtr.m_pResource;
	}

	//! Standard inequality operator.
	bool operator!=(CScopedBase<T, Deleter> const& pobSmartPtr) const
	{
		return m_pResource != pobSmartPtr.m_pResource;
	}

private:
	//! Frees the contents of the smart pointer.
	/*! The smart pointer will be empty after this function returns.
	*/
	void Free()
	{
		m_obDeleter(m_pResource);
		m_pResource = 0;
	}

	//! The encapsulated resource.
	T* m_pResource;

	//! The deleter that will handle destroying the resource.
	Deleter m_obDeleter;
};

/***************************************************************************************************
*
*	CLASS			CSharedBase
*
*	DESCRIPTION		The base shared template to a resource.
*
***************************************************************************************************/

//! A generic base shared smart pointer template.
/*! Use for any resources that will require deletion when all pointers go out of scope.
*/
template<typename T, typename Deleter, int C = Mem::MC_MISC, typename CounterAllocator = ntstd::allocator<int, C> >
class CSharedBase
{
public:
	//! Creates an empty smart pointer.
	CSharedBase() : m_pResource(0), m_piCounter(0) {}

	//! Creates a smart pointer around the given resource.
	/*! The smart pointer assumes ownership of the resource and will
		free it when all pointers go out of scope.
	*/
	explicit CSharedBase(T* pResource) : m_pResource(pResource), m_piCounter(0)
	{
		if(pResource != 0)
		{
			m_piCounter = m_obCounterAllocator.allocate(1);
			*m_piCounter = 1;
		}
	}

	//! Copies the contents of an existing smart pointer.
	/*! If the pointer is non-empty then the reference count is incremented.
	*/
	CSharedBase(CSharedBase<T, Deleter> const& pobSmartPtr) 
	  : m_pResource(pobSmartPtr.m_pResource),
		m_piCounter(pobSmartPtr.m_piCounter)
	{
		if(m_piCounter)
			++*m_piCounter;
	}

	//! Copies the contents of an existing smart pointer.
	/*! If the pointer is non-empty then the reference count is incremented.
	*/
	CSharedBase<T, Deleter> const& operator=(CSharedBase<T, Deleter> const& pobSmartPtr)
	{
		CSharedBase<T, Deleter> pobTemp(pobSmartPtr);
		Swap(pobTemp);
		return *this;
	}
	
	//! Deletes the encapsulated resource if this is the last pointer to go out of scope.
	~CSharedBase()
	{
		Free();
	}

	//! Retrieves the contents of the smart pointer.
	/*! \return		The contents of the smart pointer, or 0 if it is empty.
	*/
	T* Get() const { return m_pResource; }

	//! Retrieves the current reference count on an instance.
	/*! \return		The current reference count, or 0 if the smart pointer is empty.
	*/
	int GetReferenceCount() const 
	{ 
		if(m_piCounter)
			return *m_piCounter;
		else
			return 0;
	}

	//! Resets the contents of the smart pointer.
	/*! The previous contents of the smart pointer are deleted if this is the last
		pointer with a reference to the contents. Calling Reset with no arguments 
		or an argument of 0 effectively will result in an empty smart pointer. The 
		smart pointer will assume ownership of any non-zero resource and will free
		it when all pointers go out of scope.
		\param pNewResource		The new contents of the smart pointer, or 0
								to create an empty smart pointer.
	*/
	void Reset(T* pNewResource = 0)
	{
		CSharedBase<T, Deleter> pobTemp(pNewResource);
		Swap(pobTemp);
	}

	//! Swaps the contents of this smart pointer with that of another.
	void Swap(CSharedBase<T, Deleter>& pobSmartPtr)
	{
		T* pTemp = m_pResource;
		m_pResource = pobSmartPtr.m_pResource;
		pobSmartPtr.m_pResource = pTemp;
		
		int* piTemp = m_piCounter;
		m_piCounter = pobSmartPtr.m_piCounter;
		pobSmartPtr.m_piCounter = piTemp;
	}

	//! Standard equality operator.
	bool operator==(CSharedBase<T, Deleter> const& pobSmartPtr) const
	{
		return m_pResource == pobSmartPtr.m_pResource;
	}

	//! Standard inequality operator.
	bool operator!=(CSharedBase<T, Deleter> const& pobSmartPtr) const
	{
		return m_pResource != pobSmartPtr.m_pResource;
	}

private:
	//! Frees the contents of the smart pointer.
	/*! The smart pointer will be empty after this function returns.
	*/
	void Free()
	{
		if(m_piCounter != 0 && --*m_piCounter == 0)
		{
			m_obDeleter(m_pResource);
			m_pResource = 0;
			m_obCounterAllocator.deallocate(m_piCounter, 1);
			m_piCounter = 0;
		}
	}

	//! The encapsulated resource.
	T* m_pResource;

	//! The shared instance counter.
	typename CounterAllocator::pointer m_piCounter;

	//! The allocator for the the counter.
	CounterAllocator m_obCounterAllocator;

	//! The deleter that will handle deleting the resource.
	Deleter m_obDeleter;
};

/***************************************************************************************************
*
*	CLASS			CScopedPtr
*
*	DESCRIPTION		A scoped pointer to an instance.
*
***************************************************************************************************/

//! A simple scoped smart pointer template.
/*! Use for any resources that will require deletion via NT_DELETE when it goes out of scope.
*/
template<typename T, int C = Mem::MC_MISC>
class CScopedPtr : public CScopedBase<T, COperatorDeleter<T, C> >
{
public:
	//! Creates an empty scoped pointer.
	CScopedPtr() {}

	//! Creates a scoped pointer to manage the given resource.
	explicit CScopedPtr(T* pResource) : CScopedBase<T, COperatorDeleter<T, C> >(pResource) {}

	//! Overloaded operator to the smart pointer contents.
	T* operator*() const 
	{
		return this->Get();
	}
	
	//! Overloaded operator to the smart pointer contents.
	T* operator->() const
	{
		return this->Get();
	}

	operator bool() const { return this->Get() != 0; }
};

/***************************************************************************************************
*
*	CLASS			CSharedPtr
*
*	DESCRIPTION		A shared pointer to an instance.
*
***************************************************************************************************/

//! A simple shared smart pointer template.
/*! Use for any resources that will require deletion via NT_DELETE when all pointers go out of scope.
*/
template<typename T, int C = Mem::MC_MISC, typename CounterAllocator = ntstd::allocator<int,C> >
class CSharedPtr : public CSharedBase<T,  COperatorDeleter<T,C>, C, CounterAllocator>
{
public:
	//! Creates an empty shared pointer.
	CSharedPtr() {}

	//! Creates a shared pointer to manage the given resource.
	explicit CSharedPtr(T* pResource) : CSharedBase<T, COperatorDeleter<T, C>, C, CounterAllocator>(pResource) {}

	//! Overloaded operator to the smart pointer contents.
	T* operator*() const 
	{
		return this->Get();
	}
	
	//! Overloaded operator to the smart pointer contents.
	T* operator->() const
	{
		return this->Get();
	}

	//! Returns true if the smart pointer is non-empty.
	operator bool() const { return this->Get() != 0; }
};

/***************************************************************************************************
*
*	CLASS			CScopedArray
*
*	DESCRIPTION		A scoped pointer to an array.
*
***************************************************************************************************/

//! A simple scoped smart pointer template.
/*! Use for any resources that will require deletion via NT_DELETE_ARRAY when it goes out of scope.
*/
template<typename T, int C = Mem::MC_MISC >
class CScopedArray : public CScopedBase<T, CArrayOperatorDeleter<T,C> >
{
public:
	//! Creates an empty scoped array pointer.
	CScopedArray() {}

	//! Creates a scoped array pointer to manage the given resource.
	explicit CScopedArray(T* pResource) : CScopedBase<T, CArrayOperatorDeleter<T, C> >(pResource) {}

	//! Overloaded operator to the array smart pointer contents.
	T* operator*() const 
	{
		return this->Get();
	}
	
	//! Overloaded operator to the array smart pointer contents.
	T& operator[](int iIndex) const
	{
		return this->Get()[iIndex];
	}

	//! Returns true if the smart pointer is non-empty.
	operator bool() const { return this->Get() != 0; }
};

/***************************************************************************************************
*
*	CLASS			CSharedArray
*
*	DESCRIPTION		A shared pointer to an array.
*
***************************************************************************************************/

//! A simple shared smart pointer template.
/*! Use for any resources that will require deletion via NT_DELETE_ARRAY when all pointers go out of scope.
*/
template<typename T, int C = Mem::MC_MISC, typename CounterAllocator = ntstd::allocator<int,C> >
class CSharedArray : public CSharedBase<T, CArrayOperatorDeleter<T,C>, C, CounterAllocator>
{
public:
	//! Creates an empty shared array pointer.
	CSharedArray() {}

	//! Creats a shared array pointer around the given resource.
	explicit CSharedArray(T* pResource) : CSharedBase<T, CArrayOperatorDeleter<T, C>, C, CounterAllocator>(pResource) {}

	//! Overloaded operator to the array smart pointer contents.
	T* operator*() const 
	{
		return this->Get();
	}
	
	//! Overloaded operator to the array smart pointer contents.
	T& operator[](int iIndex) const
	{
		return this->Get()[iIndex];
	}

	//! Returns true if the smart pointer is non-empty.
	operator bool() const { return this->Get() != 0; }
};

/***************************************************************************************************
*
*	CLASS			CComPtr
*
*	DESCRIPTION		A minimal smart COM pointer.
*
***************************************************************************************************/

//! A COM pointer wrapper for those that like that sort of thing. 
/*! I expect this to be used mostly with the PC rendering side of the project, since all DirectX
	objects are accessed through COM interfaces.
*/
template<typename I>
class CComPtr
{
public:
	// Constructs an empty smart pointer.
	CComPtr() : m_pResource(0) {}

	//! Explicitly constructs a smart pointer around an existing resource.
	/*! The resource can be zero to create an empty smart pointer explicitly. If the 
		resource is non-zero it is not addref-ed, and the smart pointer will assume ownership.
	*/
	explicit CComPtr(I* pResource) : m_pResource(pResource) {}

	//! Releases the encapsulated resource, if it exists.
	~CComPtr()
	{
		SafeRelease();
	}

	//! Copy-constructs the smart pointer from another smart pointer.
	CComPtr(CComPtr<I> const& pobSmartPtr) : m_pResource(pobSmartPtr.m_pResource)
	{
		if(m_pResource)
			m_pResource->AddRef();
	}

	//! Assigns the contents of the smart pointer from an existing smart pointer.
	CComPtr<I>& operator=(CComPtr<I> const& pobSmartPtr)
	{
		CComPtr<I> pobCopy(pobSmartPtr);
		Swap(pobCopy);
		return *this;
	}

	//! Gets the contents of the smart pointer. The contents are not addref-ed during this call.
	I* Get() const { return m_pResource; }

	//! Gets the contents of the smart pointer. The contents are not addref-ed during this call.
	I* operator->() const { return m_pResource; }

	//! Releases the encapsulated resource, if it exists.
	void SafeRelease()
	{
		if(m_pResource)
		{
			m_pResource->Release();
			m_pResource = 0;
		}
	}

	//! Gets a pointer to the storage in this smart pointer. 
	/*! This function will complain if the contents are non-zero, since overwriting non-zero
		contents can cause a memory leak due the lack of Release.
	*/
	I** AddressOf()
	{
		ntAssert_p(!m_pResource, ("this should be 0 to avoid memory leakage"));
		return &m_pResource;
	}

	//! Swaps the contents with that of the argument.
	void Swap(CComPtr<I>& pobSmartPtr)
	{
		I* pTemp = m_pResource;
		m_pResource = pobSmartPtr.m_pResource;
		pobSmartPtr.m_pResource = pTemp;
	}

	//! Returns true if the smart pointer is non-empty.
	operator bool() const { return m_pResource != 0; }

private:
	I* m_pResource;	//!< The smart pointer contents.
};

//
//	Similar to CComPtr (instruments AddRef and Release) but doesn't have a user
//	defined conversion to bool like CComPtr. The UDC for CComPtrs is bad because
//	you can't do something like: if ( ccom_ptr_1 == ccom_ptr_2 ) { ... } because
//	this is effectively saying: if ( ( ccom_ptr_1.m_pResource != 0 ) == ( ccom_ptr_2.m_pResource != 0 ) )
//	which isn't what you want really.
//
template < typename Type >
class IntrusiveRefCountPtr
{
	public:
		//
		//	Ctors, dtor.
		//
		IntrusiveRefCountPtr() : m_Resource( NULL ) {}
		explicit IntrusiveRefCountPtr( Type *resource ) : m_Resource( resource ) {}
		~IntrusiveRefCountPtr() { if ( m_Resource != NULL ) { m_Resource->Release(); m_Resource = NULL; } }

		IntrusiveRefCountPtr( const IntrusiveRefCountPtr &ptr )
		:	m_Resource( ptr.m_Resource )
		{
			if ( m_Resource != NULL )
			{
				m_Resource->AddRef();
			}
		}

		IntrusiveRefCountPtr &operator = ( const IntrusiveRefCountPtr &ptr )
		{
			IntrusiveRefCountPtr copy( ptr );
			Type *temp = m_Resource;
			m_Resource = copy.m_Resource;
			copy.m_Resource = temp;

			return *this;
		}

	public:
		//
		//	Get the internal pointer.
		//
		Type *	GetPtr		() const { return m_Resource; }

	public:
		//
		//	Operator overloads.
		//
		Type *	operator -> () const { return m_Resource; }
		Type &	operator *	() const { return *m_Resource; }

		bool operator ! () const { return m_Resource == NULL; }

		inline bool operator == ( const IntrusiveRefCountPtr &rhs )	const	{ return m_Resource == rhs.m_Resource; }
		inline bool operator != ( const IntrusiveRefCountPtr &rhs )	const	{ return m_Resource != rhs.m_Resource; }

		inline friend bool operator == ( const IntrusiveRefCountPtr &lhs, const Type *rhs )		{ return lhs.m_Resource == rhs; }
		inline friend bool operator == ( const Type *lhs, const IntrusiveRefCountPtr &rhs )		{ return lhs == rhs.m_Resource; }

		inline friend bool operator != ( const IntrusiveRefCountPtr &lhs, const Type *rhs )		{ return lhs.m_Resource != rhs; }
		inline friend bool operator != ( const Type *lhs, const IntrusiveRefCountPtr &rhs )		{ return lhs != rhs.m_Resource; }

		template < typename U >
		inline friend bool operator == ( const IntrusiveRefCountPtr &lhs, const U *rhs )		{ return lhs.m_Resource == rhs; }
		template < typename U >
		inline friend bool operator == ( const U *lhs, const IntrusiveRefCountPtr &rhs )		{ return lhs == rhs.m_Resource; }

		template < typename U >
		inline friend bool operator != ( const IntrusiveRefCountPtr &lhs, const U *rhs )		{ return lhs.m_Resource != rhs; }
		template < typename U >
		inline friend bool operator != ( const U *lhs, const IntrusiveRefCountPtr &rhs )		{ return lhs != rhs.m_Resource; }

	private:
		//
		//	Funkiness to handle "if ( ptr )" style test. UDCs get matched last.
		//
		class Tester
		{
			private:
				void operator delete( void * );
		};

	public:
		operator Tester * () const
		{
			if ( m_Resource == NULL )
			{
				return NULL;
			}

			static Tester test;
			return &test;
		}

	private:
		Type *m_Resource;
};

// This property must hold for this class to be useful in the animation system on SPUs.
static_assert_in_class( sizeof( IntrusiveRefCountPtr< int32_t > ) == sizeof( intptr_t ), IntrusiveRefCountPtr_must_be_same_size_as_raw_pointer );

#endif // ndef _SMARTPTR_H
