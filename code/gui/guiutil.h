/***************************************************************************************************
*
*	DESCRIPTION		Useful snippets for GUI functionality
*
*	NOTES
*
***************************************************************************************************/

#ifndef	_GUIUTIL_H
#define	_GUIUTIL_H

class CStringDefinition;
class CFont;

/***************************************************************************************************
*
*	CLASS			CGuiTimer
*
*	DESCRIPTION		A very simple class for checking whether a set amount of time has passed.
*					Sort of a countdowny thing.
*
***************************************************************************************************/

class CGuiTimer
{
public:

	// Construction Destruction
	CGuiTimer( void ) : m_fCountdownTime(0.0f) {  }
	~CGuiTimer( void ) {}

	// Simple functionality - set, update, check
	void	Set( float fCountdownTime );
	void	Update( void );
	bool	Passed( void );

	// Other useful bits, reset to initial value, get the time, get the proportion since initial time
	void	Reset( void );
	float	Time( void );
	float	NormalisedTime( void);
	
private:

	// Holds the countdown time remaining
	float	m_fCountdownTime;

	// The timers initial time so it can do a reset
	float	m_fInitialTime;
};

namespace GuiUtil
{
	bool SetFloat(const char* pcValue, float* const pFloat);
	bool SetFloats(const char* pcValue, float* const pFloatA, float* const pFloatB);
	bool SetFloats(const char* pcValue, float* const pFloatA, float* const pFloatB , float* const pFloatC);
	bool SetFloats(const char* pcValue, float* const pFloatA, float* const pFloatB, float* const pFloatC, float* const pFloatD);
	bool SetString( const char* pcValue, const char** const ppcString );
	bool SetInt(const char* pcValue, int* const pInt);
	bool SetFlags( const char* pcValue, const CStringUtil::STRING_FLAG* pastrFlags, int* const piFlags );
	bool SetFlags( const WCHAR_T* pwcValue, const CStringUtil::STRING_FLAG_W* const pastrFlags, int* const piFlags );
	bool SetEnum( const char* pcValue, const CStringUtil::STRING_FLAG* pastrFlags, int* const piFlags );
	bool SetEnum( const WCHAR_T* pwcValue, const CStringUtil::STRING_FLAG_W* const pastrFlags, int* const piFlags );
	bool SetBool( const char* pcValue, bool* const pBool );

	u_int CalculateHash( const char* pwcIdentifier );
	u_int CalculateHash( const WCHAR_T* pwcIdentifier );

	//renders in screen space
	void RenderString(float fX, float fY, CStringDefinition* pobDef, const char* str);
	CFont *GetFontFromDescription(const char *strFontDescription);
	CFont *GetFontAbstracted(const char *pcFontName);
}

struct GuiExtents
{
	float fX;
	float fY;
	float fWidth;
	float fHeight;
};

#endif // _GUIUTIL_H
