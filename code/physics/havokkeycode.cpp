#ifndef HAVOK_KEYCODE_H
#define HAVOK_KEYCODE_H

#include "config.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#include <hkbase\config\hkConfigVersion.h>
#include "physics/havokincludes.h"

//
// Enter your keycode below in the form: HAVOK_KEYCODE("<code>", <value>)
// Where <code> is your key-code and <value> is your client key-value.
// The client key-value should only be specified with a valid client key-code
// and left at its default value of 0x0 otherwise.
// Please note that when running a simulation with the eval version of the SDK,
// the simulation will pause for about 1 second every 5 minutes.
//

// Convenience Macro for keycodes
#include <hkbase/hkBase.h>
#include <hkserialize/version/hkVersionRegistry.h>

#if defined(HK_PLATFORM_GC)
#	define KEYCODE_ATTRIBUES __attribute__((section(".sdata")))
#else
#	define KEYCODE_ATTRIBUES
#endif

#define HAVOK_KEYCODE(code, value)					\
	extern const char         HK_KEYCODE[] KEYCODE_ATTRIBUES = code;	\
	extern const unsigned int HK_KEYVALUE  KEYCODE_ATTRIBUES = value

// FINAL CLIENT Code this will NEVER change we can now actuall ship the game!!
HAVOK_KEYCODE("CLIENT.PhAn.NinjaTheory", 0x76373eb2);


// Generate a custom list to trim memory requirements
//#define HK_CLASSES_FILE <demos/classlist/hkCompleteClasses.h>
//#include <hkserialize/util/hkBuiltinTypeRegistry.cxx>
// Generate a custom list to trim memory requirements
#define HK_COMPAT_FILE <hkcompat/hkCompatVersions.h>
#include <hkcompat/hkCompat_All.cxx>

#endif // !_PS3_RUN_WITHOUT_HAVOK_BUILD
#endif // !HAVOK_KEYCODE_H

