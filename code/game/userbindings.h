#ifndef _USERBINDINGS_H_
#define _USERBINDINGS_H_

//--------------------------------------------------
//!
//!	"special" lua binding function
//!	This is the palce where you bind all the special/debug/per-level function.
//!	Everybody who use that MUST be aware that this file will be clean up one day or the other.
//! The bindings function in that file should be here for special and temporary reasons
//!
//! ATTN! this class does nothing on PS3!
//!
//--------------------------------------------------



class CUserBindings
{
public:

#if defined( PLATFORM_PC )

	static void Register();

#elif defined( PLATFORM_PS3 )

	static void Register() {}

#endif
};

#endif // end of _USERBINDINGS_H_
