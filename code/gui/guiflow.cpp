/***************************************************************************************************
*
*	DESCRIPTION		Contains functionality to map out the flow of a game
*
*	NOTES			The game flow is simply a description of an entire game layout.  Which user
*					screens are presented where, where levels, videos etc are to be found.
*
***************************************************************************************************/

// Includes
#include "guiflow.h"
#include "guiutil.h"
#include "guimanager.h"

/***************************************************************************************************
*
*	The bits below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CScreenHeader(); }

// Register this class under it's XML tag
bool g_bSCREENHEADER	= CGuiManager::Register( "SCREENHEADER", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::CScreenHeader
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CScreenHeader::CScreenHeader( void )
{
	// Clear the flags
	m_iScreenFlags = 0;

	// Initialise our name
	m_pcFileName = 0;

	// The skipback value defaults to zero
	m_iSkipback = 0;

	m_pcTag = NULL;
}


/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::~CScreenHeader
*
*	DESCRIPTION		Destruction - the destruction of child elements is dealt with by the underlying
*					CXMLElement class.
*
***************************************************************************************************/

CScreenHeader::~CScreenHeader( void )
{
	NT_DELETE( m_pcFileName );
	NT_DELETE_ARRAY(m_pcTag);
}

const char* CScreenHeader::GetTag() const
{
	return m_pcTag ? m_pcTag : "";
}

void CScreenHeader::SetTag(const char* pcTag)
{
	NT_DELETE_ARRAY(m_pcTag);
	m_pcTag = NULL;

	GuiUtil::SetString( pcTag, &m_pcTag );
}

NinjaLua::LuaObject CScreenHeader::GetScreenStore()
{
	if (m_obScreenStore.IsNil())
		m_obScreenStore = NinjaLua::LuaObject::CreateTable(CLuaGlobal::Get().State());

	return m_obScreenStore;
}

/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::GetOption
*
*	DESCRIPTION
*
***************************************************************************************************/

CScreenHeader* CScreenHeader::GetOption( int iOption )
{
	// Make sure that this is a sensible request
	ntAssert( (u_int)iOption <= m_obScreenHeaders.size() );

	// Create and initialise a return value
	CScreenHeader* pobScreenHeader = 0;

	// Loop to the option to hand it back
	int iScreenHeader = 1;
	for ( ntstd::List< CScreenHeader* >::iterator obIt = m_obScreenHeaders.begin(); obIt != m_obScreenHeaders.end(); ++obIt )
	{
		// If this is out option number have it
		if ( iScreenHeader == iOption )
		{
			pobScreenHeader = ( *obIt );
			break;
		}

		// Otherwise move along, there's nothing to see here
		++iScreenHeader;
	}

	// Check we found what we wanted
	ntAssert ( pobScreenHeader );

	return pobScreenHeader;
}


/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::ProcessAttribute
*
*	DESCRIPTION
*
***************************************************************************************************/

bool CScreenHeader::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CXMLElement::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "filename" ) == 0 )
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

		else if ( strcmp( pcTitle, "tag" ) == 0 )
		{
			SetTag(pcValue);
			return true;
		}

		return false;
	}

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::ProcessChild
*
*	DESCRIPTION
*
***************************************************************************************************/

bool CScreenHeader::ProcessChild( CXMLElement* pobChild )
{
	// Drop out if the data is shonky
	ntAssert( pobChild );
	UNUSED(pobChild);

	//lets not do this here as we ned to be able to sync children post load. This operation has been deferred to SyncChildren
/*
	// The screenheader on holds lists of other screen headers
	m_obScreenHeaders.push_back( ( CScreenHeader* )pobChild );
*/

	return true;
}

void CScreenHeader::SyncChildren()
{
	m_obScreenHeaders.clear();
	for( ntstd::List< CXMLElement* >::iterator obIt = GetChildren().begin(); obIt != GetChildren().end(); ++obIt)
	{
		m_obScreenHeaders.push_back( ( CScreenHeader* )(*obIt) );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::ProcessEnd
*
*	DESCRIPTION		Called when we reach the end on the defining XML tag.  We know that we are fully 
*					defined here so we can do any final setup.
*
***************************************************************************************************/

bool CScreenHeader::ProcessEnd( void )
{
	// Make sure that we have a filename at the very least
	ntAssert( m_pcFileName );

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::ProcessSkipBack
*
*	DESCRIPTION		Sets an attribute that allows game flow to drop back through the game heirachy
*					when this screen is finished with.
*
***************************************************************************************************/

bool CScreenHeader::ProcessSkipBack( const char* pcValue )
{
	// Scan the value for an int
	int iResult = sscanf( pcValue, "%d", &m_iSkipback ); 

	// Make sure we extracted one value
	ntAssert( iResult == 1 );
	UNUSED( iResult );

	// Return true if successful
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::ProcessFlags
*
*	DESCRIPTION		Sets up the screen property flags from a string. Multiple flags in the string
*					should be seperated by spaces.
*
*					We start from the end of the string, working backwards to find tokens, comparing
*					each with a string representation of the enumeration value.
*
***************************************************************************************************/

bool CScreenHeader::ProcessFlags( const char* pcValue )
{
	// Get the bits represented in the string
	m_iScreenFlags = CStringUtil::GetBitsFromString( pcValue, astrScreenFlags );

	// If we are here, yet no flags have been recognised then there is something wrong
	ntAssert( m_iScreenFlags != 0 );

	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::GetNextScreenP
*
*	DESCRIPTION		This returns the next screen that should be presented to the user.
*
***************************************************************************************************/

CScreenHeader*	CScreenHeader::GetNextScreenP( int iOption )
{
	// If we have been asked for an option number - get that
	if ( iOption > 0 )
	{
		return GetOption( iOption );
	}

	// If we are a screen that skips back multiple screens - give that
	else if ( m_iSkipback > 0 )
	{
		// Starting from ourself, skip back
		CScreenHeader* pScreenReturn = GetBackScreenP(m_iSkipback);

		return pScreenReturn;
	}

	// Otherwise we better hope that there is only one child available to return
	else
	{
		ntAssert( m_obScreenHeaders.size() == 1  );
		return GetOption( 1 );
	}
}


/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::GetBackScreenP
*
*	DESCRIPTION		This returns the next screen that should be presented to the user.
*
***************************************************************************************************/

CScreenHeader*	CScreenHeader::GetBackScreenP( int iScreensBack )
{
	CScreenHeader *pobBackScreen = this;

	// We have to move the default one screen back
	if (iScreensBack == 0)
		iScreensBack = 1;

	for (int iBack = 0; iBack < iScreensBack; iBack++)
	{
		// Make sure that this element has a parent element
		ntAssert( GetParent() );
		pobBackScreen = ( CScreenHeader* )pobBackScreen->GetParent();
	}

	// Assuming we are A O K give it to them
	return pobBackScreen;
}

/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::FindForwardScreenP
*
*	DESCRIPTION		This returns the next screen with type in iScreenFlags.
*					Breath first search of child screens
*
***************************************************************************************************/

CScreenHeader*	CScreenHeader::FindForwardScreenP( int iScreenFlags )
{
	CScreenHeader *pobReturnScreen = 0;

	// Loop to the option to hand it back
	for ( ntstd::List< CScreenHeader* >::iterator obIt = m_obScreenHeaders.begin(); obIt != m_obScreenHeaders.end(); ++obIt )
	{
		// If this screen is the correct type return it.
		if ( (*obIt)->m_iScreenFlags & iScreenFlags )
		{
			pobReturnScreen = ( *obIt );
			break;
		}
	}

	return pobReturnScreen;
}

/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::FindBackScreenP
*
*	DESCRIPTION		This returns the parent screen with type in iScreenFlags.
*					Search up parent chain
*
***************************************************************************************************/

CScreenHeader*	CScreenHeader::FindBackScreenP( int iScreenFlags )
{
	CScreenHeader *pobReturnScreen = (CScreenHeader*)GetParent();;
		
	// We got all the way to the top without finding the right screen
	if (! pobReturnScreen)
		return 0;

	if ( pobReturnScreen->m_iScreenFlags & iScreenFlags )
		return pobReturnScreen;
	else
		return pobReturnScreen->FindBackScreenP( iScreenFlags );
}

CScreenHeader* CScreenHeader::FindForwardScreenP( const char* pcTag)
{
	CScreenHeader *pobReturnScreen = 0;

	// Loop to the option to hand it back
	for ( ntstd::List< CScreenHeader* >::iterator obIt = m_obScreenHeaders.begin(); obIt != m_obScreenHeaders.end(); ++obIt )
	{
		// If this screen is the correct type return it.
		if ( strcmp((*obIt)->GetTag(), pcTag) == 0 )
		{
			pobReturnScreen = ( *obIt );
			break;
		}
	}

	return pobReturnScreen;
}

CScreenHeader* CScreenHeader::FindBackScreenP( const char* pcTag )
{
	CScreenHeader *pobReturnScreen = (CScreenHeader*)GetParent();;

	if (!pobReturnScreen)
		return NULL;
		
	// we found it!
	if ( strcmp(pcTag, pobReturnScreen->GetTag()) == 0 )
		return pobReturnScreen;

	return pobReturnScreen->FindBackScreenP( pcTag );
}

/***************************************************************************************************
*
*	The bits below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapperSG( void ) { return NT_NEW CScreenGroup(); }

// Register this class under it's XML tag
bool g_bSCREENGROUP = CGuiManager::Register( "SCREENGROUP", &ConstructWrapperSG );

/***************************************************************************************************
*
*	FUNCTION		CScreenGroup::CScreenGroup
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CScreenGroup::CScreenGroup( void )
{
	m_pcFileName = NULL;
	m_pcTag = NULL;
}


/***************************************************************************************************
*
*	FUNCTION		CScreenGroup::~CScreenGroup
*
*	DESCRIPTION		Destruction - the destruction of child elements is dealt with by the underlying
*					CXMLElement class.
*
***************************************************************************************************/

CScreenGroup::~CScreenGroup( void )
{
	NT_DELETE_ARRAY( m_pcFileName );
	NT_DELETE_ARRAY( m_pcTag );
}

/***************************************************************************************************
*
*	FUNCTION		CScreenHeader::ProcessAttribute
*
*	DESCRIPTION
*
***************************************************************************************************/

bool CScreenGroup::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !CXMLElement::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "filename" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcFileName );
		}
		else if ( strcmp( pcTitle, "tag" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_pcTag);
		}

		return false;
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CScreenGroup::ProcessEnd
*
*	DESCRIPTION		Called when we reach the end on the defining XML tag.  We know that we are fully 
*					defined here so we can do any final setup.
*
***************************************************************************************************/

bool CScreenGroup::ProcessEnd( void )
{
	ntAssert( m_pcFileName );
	ntAssert( m_pcTag );		// group must have a tag?

	return true;
}
