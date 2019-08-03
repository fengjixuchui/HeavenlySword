//**************************************************************************************
//
//	ntStreamFactory_ps3.h
//	
//	PS3 replacement for the Havok hkStreambufFactory singleton class.
//
//**************************************************************************************

#ifndef NTSTREAMFACTORY_PS3_
#define NTSTREAMFACTORY_PS3_

#ifndef PLATFORM_PS3
#	error This header file is a PS3 header!
#endif // !PLATFORM_PS3

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include <hkbase/hkBase.h>
#include <hkbase/stream/hkStreambufFactory.h>

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************
class hkStreamWriter;
class hkStreamReader;

//**************************************************************************************
//	Ninja Theory custom Havok stream factory.
//**************************************************************************************
namespace Physics
{
	class ntStreamFactory : public hkStreambufFactory
	{
		public:
			//
			//	Implement hkStreambufFactory's methods.
			//
			virtual hkStreamWriter *	openWriter	( const char *name );
			virtual hkStreamReader *	openReader	( const char *name );
			virtual hkStreamWriter *	openConsole	( StdStream s );
	};
}

#endif // !NTSTREAMFACTORY_PS3_
