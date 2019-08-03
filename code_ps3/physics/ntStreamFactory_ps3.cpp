//**************************************************************************************
//
//	ntStreamFactory_ps3.cpp
//	
//	PS3 replacement for the Havok hkStreambufFactory singleton class.
//
//**************************************************************************************

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "physics/config.h"
#include "physics/ntStreamFactory_ps3.h"

#include "physics/ntStreamReader_ps3.h"		// Include our custom reader.
#include "physics/ntStreamWriter_ps3.h"		// Include our custom writer.
#include "physics/ntConsoleWriter_ps3.h"	// Include our console stream writer.

#include "physics/ntBufferedStreamReader_ps3.h"

#define USE_BUFFERED_READS

//**************************************************************************************
//	
//**************************************************************************************
hkStreamWriter *Physics::ntStreamFactory::openWriter( const char *name )
{
	return HK_NEW ntStreamWriter( name );
}

//**************************************************************************************
//	
//**************************************************************************************
hkStreamReader *Physics::ntStreamFactory::openReader( const char *name )
{
#	ifdef USE_BUFFERED_READS
		return HK_NEW ntBufferedStreamReader( name );
#	else
		return HK_NEW ntStreamReader( name );
#	endif
}

//**************************************************************************************
//	
//**************************************************************************************
hkStreamWriter *Physics::ntStreamFactory::openConsole( StdStream s )
{
	return HK_NEW ntConsoleWriter( s );
}




