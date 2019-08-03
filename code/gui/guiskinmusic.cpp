/***************************************************************************************************
*
*	DESCRIPTION		A usable screen unit that contains background music functionality
*
*	NOTES			
*
***************************************************************************************************/
// Includes
#include "guiskinmusic.h"
#include "guisound.h"
#include "guimanager.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinMusic(); }

// Register this class under it's XML tag
bool g_bMUSIC = CGuiManager::Register( "MUSIC", &ConstructWrapper );

int CGuiSkinMusic::s_iUseCount = 0;

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMusic::CGuiSkinMusic
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinMusic::CGuiSkinMusic( void )
{
	m_iMusicType = CGuiSound::MUSIC_MAINMENU;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMusic::~CGuiSkinMusic
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinMusic::~CGuiSkinMusic( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMusic::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinMusic::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CGuiUnit::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( stricmp( pcTitle, "type" ) == 0 )
		{
			return GuiUtil::SetFlags( pcValue, &astrSounds[0], &m_iMusicType );
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMusic::ProcessEnd
*
*	DESCRIPTION		This is called when the closing tag on an XML element is parsed.  
***************************************************************************************************/

bool CGuiSkinMusic::ProcessEnd( void )
{
	// Call the base first
	CGuiUnit::ProcessEnd();

	return true;
}

void CGuiSkinMusic::SetStateIdle( void )
{
	CGuiUnit::SetStateIdle();

	// Only start music if we have not already done so
	/*if (s_iUseCount == 0)
	{
		CGuiSoundManager::Get().PlayMusic( m_iMusicType );
	}
	++s_iUseCount;*/
}

void CGuiSkinMusic::SetStateDead( void )
{
	CGuiUnit::SetStateDead();

	/*--s_iUseCount;
	ntAssert( s_iUseCount >= 0);

	// Only stop music if we have not already done so
	if (s_iUseCount == 0)
	{
		CGuiSoundManager::Get().StopMusic( m_iMusicType );
	}*/
}
