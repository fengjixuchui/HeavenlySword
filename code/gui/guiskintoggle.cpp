/***************************************************************************************************
*
*	DESCRIPTION		A temporary skin of the basic user interface toggle unit
*
*	NOTES			Just for testing out the basic structural functionality of the GUI
*
***************************************************************************************************/

// Includes
#include "guiskintoggle.h"
#include "guimanager.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinTestToggle(); }

// Register this class under it's XML tag
bool g_bTOGGLE = CGuiManager::Register( "TOGGLE", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestToggle::CGuiSkinTestToggle
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinTestToggle::CGuiSkinTestToggle( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinTestToggle::~CGuiSkinTestToggle
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinTestToggle::~CGuiSkinTestToggle( void )
{
}



