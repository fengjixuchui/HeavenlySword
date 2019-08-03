// code to add dummy raise throw handlers when exceptions are turned off

#include <exception>
#if defined(PLATFORM_PC) && (_HAS_EXCEPTIONS == 0)

namespace std
{
	void _Throw( class std::exception const & except )
	{
		ntPrintf( "STD::exception : %s\n", except.what() );
		ntError_p( false, ("STD::exception throw") );
	}

	void (__cdecl* std::_Raise_handler)(class std::exception const & );
}

#elif defined(PLATFORM_PS3) && (_HAS_EXCEPTIONS == 0)

void std::exception::_Raise() const 
{
	ntError_p( false, ("STD::exception raise") );
}

#endif

