/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			Looks after front end sounds
*
***************************************************************************************************/

// Includes
#include "guimanager.h"
#include "guisound.h"

#include "audio/audiosystem.h"
#include "game/shellconfig.h"

#ifndef _RELEASE

//#define _GUI_SOUND_DEBUG

#endif // _RELEASE



/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* SoundConstructWrapper( void ) { return NT_NEW CGuiSound(); }

// Register this class under it's XML tag
bool g_bSOUND = CGuiManager::Register( "SOUND", &SoundConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSound::CGuiSound
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSound::CGuiSound( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSound::~CGuiSound
*
*	DESCRIPTION		Desruction
*
***************************************************************************************************/

CGuiSound::~CGuiSound( void )
{
	if ( m_pcFileName )
		NT_DELETE_ARRAY( m_pcFileName);
	
	if ( m_pcBankName )
		NT_DELETE_ARRAY( m_pcBankName);
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSound::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSound::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CXMLElement::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "sound" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcFileName );
		}

		if ( strcmp( pcTitle, "bank" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcBankName );
		}

		if ( strcmp( pcTitle, "type" ) == 0 )
		{
			return GuiUtil::SetFlags(pcValue, &astrSounds[0], &m_iSoundType);
		}

		return false;
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSound::ProcessEnd
*
*	DESCRIPTION		Called when we reach the end on the defining XML tag.  We know that we are fully 
*					defined here so we can do any final setup.
*
***************************************************************************************************/

bool CGuiSound::ProcessEnd( void )
{
	ntAssert( m_pcFileName );
	ntAssert( m_pcBankName );

	return true;
}

/***************************************************************************************************
*
*	The bits below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSoundList(); }

// Register this class under it's XML tag
bool g_bSOUNDLIST	= CGuiManager::Register( "SOUNDLIST", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::CScreenHeader
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSoundList::CGuiSoundList( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSoundList::~CGuiSoundList
*
*	DESCRIPTION		Destruction - the destruction of child elements is dealt with by the underlying
*					CXMLElement class.
*
***************************************************************************************************/

CGuiSoundList::~CGuiSoundList( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSoundList::ProcessAttribute
*
*	DESCRIPTION
*
***************************************************************************************************/

bool CGuiSoundList::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CXMLElement::ProcessAttribute( pcTitle, pcValue ) )
	{
		/*if ( strcmp( pcTitle, "filename" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcFileName );
		}

		else if ( strcmp( pcTitle, "skipback" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iSkipback );
		}

		else if ( strcmp( pcTitle, "flags" ) == 0 )
		{
			return GuiUtil::SetFlags( pcValue, &astrScreenFlags[0], &m_iScreenFlags );
		}

		return false;*/
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSoundList::ProcessChild
*
*	DESCRIPTION
*
***************************************************************************************************/

bool CGuiSoundList::ProcessChild( CXMLElement* pobChild )
{
	// Drop out if the data is shonky
	ntAssert( pobChild );

	// The screenheader on holds lists of other screen headers
	m_obSoundList.push_back( ( CGuiSound* )pobChild );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSoundList::ProcessEnd
*
*	DESCRIPTION		Called when we reach the end on the defining XML tag.  We know that we are fully 
*					defined here so we can do any final setup.
*
***************************************************************************************************/

bool CGuiSoundList::ProcessEnd( void )
{
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSoundList::GetSound
*
*	DESCRIPTION
*
***************************************************************************************************/

CGuiSound*	CGuiSoundList::GetSound (int iSound)
{
	// Create and initialise a return value
	CGuiSound* pobSound = 0;

	// Loop to the sound to hand it back
	for ( ntstd::List< CGuiSound* >::iterator obIt = m_obSoundList.begin(); obIt != m_obSoundList.end(); ++obIt )
	{
		// If this is our sound have it
		if ( ( *obIt )->m_iSoundType == iSound )
		{
			pobSound = ( *obIt );
			break;
		}
	}

	// Check we found what we wanted
	ntAssert ( pobSound );

	return pobSound;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSoundManager::CGuiSoundManager
*
*	DESCRIPTION
*
***************************************************************************************************/

CGuiSoundManager::CGuiSoundManager( void )
:	m_pobSoundList ( 0 )
{
	
}

CGuiSoundManager::~CGuiSoundManager( void )
{
	NT_DELETE (m_pobSoundList);
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSoundManager::Initialise
*
*	DESCRIPTION		Read in the sound/music definitions from xml
*
***************************************************************************************************/

void CGuiSoundManager::Initialise(void)
{
	m_bSoundFirst = g_ShellOptions->m_bGuiSoundFirst;

	// Import the sound definitions
	m_pobSoundList = static_cast< CGuiSoundList* >( CXMLParse::Get().CreateTree( "gui\\sounds.xml" ) );
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSoundManager::PlaySound
*
*	DESCRIPTION		
*
***************************************************************************************************/
bool CGuiSoundManager::PlaySound(int iSound)
{
	CGuiSound* pobSound = 0;
	pobSound = m_pobSoundList->GetSound(iSound);

	if (pobSound && AudioSystem::Get().Sound_Prepare(m_aiSoundRefs[iSound], pobSound->m_pcBankName, pobSound->m_pcFileName) )
	{
		AudioSystem::Get().Sound_Play( m_aiSoundRefs[iSound] );
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSoundManager::StopSound
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CGuiSoundManager::StopSound(int iSound)
{
	AudioSystem::Get().Sound_Stop( m_aiSoundRefs[iSound] );

	return true;
}

bool CGuiSoundManager::PlayMusic(int iSound)
{
	return PlaySound(iSound);
}

bool CGuiSoundManager::StopMusic(int iSound)
{
	return StopSound(iSound);
}
