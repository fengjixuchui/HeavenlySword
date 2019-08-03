/*----------------------------------------------------------------------
Bugslayer Column - MSDN Magazine - John Robbins
Copyright (c) 2000 - 2001 John Robbins -- All rights reserved.

The minidump function wrappers.  MiniDumpWriteDump can easily be called
to write out a dump of the current process.  The only problem is that
the dump for the current thread will show that it's coming from deep
inside MiniDumpWriteDump.  Unfortunately, with DBGHELP.DLL being updated
each release of WinDBG, we are missing symbols.  That means it's nearly
impossible to walk the stack back to your code.  These wrappers fix that
by spawning a thread to do the write.  While not a perfect solution, at
least you'll get a cleaner dump.  Keep in mind you might have to walk
the stack quite a bit to get back to your code, especially if used from
something like my SUPERASSERT (v2) dialog.

Note that these functions do NO checking if the older broken versions of
the mini dump commands are there.  The broken versions would hang if
called inside the process space.  Any version of DBGHELP.DLL from WinDBG
3.0.20 or later, or Windows XP will work just fine.

Modified by Deano for HS purposes
----------------------------------------------------------------------*/

#ifndef _MINIDUMP_H
#define _MINIDUMP_H

#include "dbghelp.h"


namespace MiniDump
{
//////////////////////////////////////////////////////////////////////*/
// The return values for CreateCurrentProcMiniDump.
typedef enum tag_BSUMDRET
{
    // Everything worked.
    eDUMP_SUCCEEDED           ,
    // DBGHELP.DLL could not be found at all in the path.
    eDBGHELP_NOT_FOUND        ,
    // The mini dump exports are not in the version of DBGHELP.DLL
    // in memory.
    eDBGHELP_MISSING_EXPORTS  ,
    // A parameter was bad.
    eBAD_PARAM                ,
    // Unable to open the dump file requested.
    eOPEN_DUMP_FAILED         ,
    // MiniDumpWriteDump failed.  Call GetLastError to see why.
    eMINIDUMPWRITEDUMP_FAILED ,
    // Death ntError.  Thread failed to crank up.
    eDEATH_ERROR              ,
    // The invalid ntError value.
    eINVALID_ERROR            ,
} BSUMDRET ;


/*----------------------------------------------------------------------
FUNCTION        :   CreateCurrentProcessMiniDump
DISCUSSION      :
    Creates a minidump of the current process.
PARAMETERS      :
    eType       - The type of mini dump to do.
    szFileName  - The complete path and filename to write the dump.
                  Traditionally, the extension for dump files is .DMP.
                  If the file exists, it will be overwritten.
    dwThread    - The optional id of the thread that crashed.
    pExceptInfo - The optional exception information.  This can be NULL
                  to indicate no exception information is to be added
                  to the dump.
RETURNS         :
    FALSE - Mini dump functions are not available.
    TRUE  - Mini dump functions are there.
----------------------------------------------------------------------*/
BSUMDRET __stdcall
    CreateCurrentProcessMiniDumpA ( MINIDUMP_TYPE        eType      ,
                                    char *               szFileName ,
                                    uint32_t                dwThread   ,
                                    EXCEPTION_POINTERS * pExceptInfo ) ;

BSUMDRET __stdcall
    CreateCurrentProcessMiniDumpW ( MINIDUMP_TYPE        eType      ,
                                    WCHAR_T *            szFileName ,
                                    uint32_t                dwThread   ,
                                    EXCEPTION_POINTERS * pExceptInfo ) ;

#ifdef UNICODE
#define CreateCurrentProcessMiniDump CreateCurrentProcessMiniDumpW
#else
#define CreateCurrentProcessMiniDump CreateCurrentProcessMiniDumpA
#endif



} // end namespace 
#endif