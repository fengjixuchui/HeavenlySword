//------------------------------------------------------------
//!
//! \file core/ntstdallocator.h
//! Replacement for std::Allocator<T> that A)Works B) uses our
//! debug memory library and system
//------------------------------------------------------------

#if !defined(CORE_NTSTDALLOCATOR_H)
#define CORE_NTSTDALLOCATOR_H

//------------------------------------------------------------
//!
//! \namespace ntstd
//! Where our  C++ std (and STL) library lives
//-------------------------------------------------------------
namespace ntstd {

	//---------------------------------------------------------
	//!
	//! Internal structure for std::allocator<T>.
	//! General template
	//!
	//---------------------------------------------------------
	template<class Ty>
		struct _Allocator_base
		{
			typedef Ty value_type;
		};

	//---------------------------------------------------------
	//!
	//! Internal structure for std::allocator<T>.
	//! Special Const special type
	//!
	//---------------------------------------------------------
	template<class Ty>
		struct _Allocator_base<const Ty>
		{
			typedef Ty value_type;
		};

	namespace _Allocator_destroy
	{
		inline void destruct( char* )
		{
			// do nothing
		}
		inline void destruct( wchar_t* )
		{
			// do nothing
		}

		template<typename T>
			void destruct(T* t)
			{
				// cos VS is being rubbish
#if defined(PLATFORM_PC)
				t;
#endif
				t->~T();
			}
	};



	template<class T, int C = Mem::MC_MISC>
	class allocator : public _Allocator_base<T>
	{
	public:
		typedef _Allocator_base<T> _base;		//!< base type
		typedef typename _base::value_type value_type;		//!< Identical to T.

		typedef size_t		size_type;			//!< A type that can represent the size of the largest object in the allocation model.
		typedef ptrdiff_t	difference_type;	//!< A type that can represent the difference between any two pointers in the allocation model.
		
		typedef value_type*			pointer;			//!< Pointer to T;
		typedef value_type const*	const_pointer;		//!< Pointer to const T.
		typedef value_type&			reference;			//!< Reference to T.
		typedef value_type const&	const_reference;	//!< Reference to const T.
	
		//! A struct to construct an allocator for a different type.
		template<class U> 
			struct rebind 
			{ 
				typedef allocator<U,C> other; 
			};

		//! address of val
		pointer address( reference val ) const
		{
			return (&val);
		}

		//! address of val
		const_pointer address( const_reference val ) const
		{
			return (&val);
		}

		//! default ctor
		allocator()
		{
			// do nothing
		}

		//! copy ctor
		allocator( const allocator<T,C>& )
		{
			// do nothing
		}

		//! ctor from related
		template<class U>
			allocator( const allocator<U,C>& )
		{
			// do nothing
		}

		//! assign from related
		template<class U>
			allocator<T,C>& operator=( const allocator<U,C>& )
		{
			// do nothing just return ourself
			return (*this);
		}

		//! deallocate via our memory system
		void deallocate( pointer ptr, size_type )
		{
#if defined( _HAVE_MEMORY_TRACKING )
			Mem::Free( (Mem::MEMORY_CHUNK)C, (uintptr_t)ptr, "ntstd::allocator", NT_FUNC_NAME, __LINE__ );
#else
			Mem::Free( (Mem::MEMORY_CHUNK)C, (uintptr_t)ptr );
#endif
		}

		//! allocate via our system
		pointer allocate( size_type count )
		{
#if defined( _HAVE_MEMORY_TRACKING )
			return reinterpret_cast<pointer>( Mem::Alloc( (Mem::MEMORY_CHUNK)C, count * sizeof(T) , __FILE__, NT_FUNC_NAME, __LINE__) );
#else
			return reinterpret_cast<pointer>( Mem::Alloc( (Mem::MEMORY_CHUNK)C, count * sizeof(T) ) );
#endif
		}

		//! allocate via our system (ignore hint)
		pointer allocate( size_type count, const void* )
		{
#if defined( _HAVE_MEMORY_TRACKING )
			return reinterpret_cast<pointer>( Mem::Alloc( (Mem::MEMORY_CHUNK)C, count * sizeof(T) , __FILE__, NT_FUNC_NAME, __LINE__) );
#else
			return reinterpret_cast<pointer>( Mem::Alloc( (Mem::MEMORY_CHUNK)C, count * sizeof(T) ) );
#endif
		}

		//! construct val at ptr (placement new)
		void construct( pointer ptr, const T& val )
		{
			NT_PLACEMENT_NEW (ptr) T(val);
		}

		//! destroy ptr (call dtor)
		void destroy( pointer ptr )
		{
			_Allocator_destroy::destruct( ptr );
		}

		//! maximum size you can allocate
		size_t max_size() const
		{	
			size_t count = (size_t)(-1) / sizeof (T);
			return (0 < count ? count : 1);
		}
	};

	// allocator TEMPLATE OPERATORS
	template<class T, int C, class U, int C2> inline
		bool operator==(const allocator<T,C>&, const allocator<U,C2>&)
	{	
		return (true);
	}

	template<class T, int C, class U, int C2> inline
		bool operator!=(const allocator<T,C>&, const allocator<U,C2>&)
	{	
		// test for allocator inequality (always false)
		return (false);
	}

	//---------------------------------------------------------
	//!
	//! void specailisation for std::allocator<void>.
	//!
	//---------------------------------------------------------
	template<> class allocator<void, 0>
	{	
	public:
		typedef void T;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T value_type;

		//! rebind allocator to different type
		template<class U>
			struct rebind
			{	
				typedef allocator<U,0> other;
			};

		//! ctor
		allocator()
		{	
			// do nothing
		}

		//! dtor
		allocator(const allocator<T, 0>&)
		{	// construct by copying (do nothing)
		}

		// ctor from related
		template<class U>
			allocator(const allocator<U,0>&)
			{	
			}

		// assign from related
		template<class U>
			allocator<T, 0>& operator=(const allocator<U,0>&)
			{	
				// do nothing
				return (*this);
			}
	};

} // end namespace std


#endif // CORE_NTSTDALLOCATOR_H

