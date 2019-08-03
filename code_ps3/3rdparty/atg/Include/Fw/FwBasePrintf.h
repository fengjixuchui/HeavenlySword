//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Replacement for C Standard Library 'printf', allowing additional filtering control.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_BASE_PRINTF_H
#define	FW_BASE_PRINTF_H

// -------------------------------------------------------------------------------------------------

class	FwBasePrintfCtrl
{
public:
	// Typedef used for our output handler
	typedef void ( *OutputHandler )( const char* pFormat, va_list argList );

	static	OutputHandler	SetHandler( OutputHandler pHandler );
	static	void			InvokeHandler( const char* pFormat, va_list argList );
	
private:
	static	void			DefaultHandler( const char* pFormat, va_list argList );

	static	OutputHandler	ms_pHandler;							///< Pointer to current handler
};

///< Sets output handler.
inline	FwBasePrintfCtrl::OutputHandler	FwBasePrintfCtrl::SetHandler( OutputHandler pHandler )
{
	OutputHandler oldHandler = ms_pHandler;
	ms_pHandler = pHandler;
	return oldHandler;
}

///< Sets output handler.
inline	void	FwBasePrintfCtrl::InvokeHandler( const char* pFormat, va_list argList )
{
	ms_pHandler( pFormat, argList );
}

// ---- Global functions

inline	void	FwBaseVPrintf( const char* pFormat, va_list argList )
{
	FwBasePrintfCtrl::InvokeHandler( pFormat, argList );
}

#ifdef	__GNUC__
void			FwBasePrintf( const char* pFormat, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
#endif	//__GNUC__

inline	void	FwBasePrintf( const char* pFormat, ... ) 
{
	va_list argList;
	va_start( argList, pFormat );
	FwBaseVPrintf( pFormat, argList );
	va_end( argList );
}

// -------------------------------------------------------------------------------------------------
// Framework-specific printf() replacement.

#ifdef	ATG_PRINTF_ENABLED

extern	bool		g_FwPrintfEnable;

inline	void		FwEnablePrintf( void )	{	g_FwPrintfEnable = true;	}
inline	void		FwDisablePrintf( void )	{	g_FwPrintfEnable = false;	}

// Helper (and internal.. not for project use) to reduce inlining overheads
void				FwPrintfCore( const char* pFormat, ... );

#define	FwPrintf	FwPrintfCore

#else

inline	void			FwEnablePrintf( void )			{}
inline	void			FwDisablePrintf( void )			{}

// Because VS.NET 2003 doesn't support variadic macros, we need to have an empty inline here..
#ifdef	__GNUC__
#define					FwPrintf( fmt, ... )			( ( void )0 )
#else
__forceinline	void	FwPrintf( const char*, ... )	{}
#endif	

#endif	// ATG_PRINTF_ENABLED

#endif	// FW_BASE_PRINTF_H
