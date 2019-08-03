/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiskinmenubutton.h"
#include "gui/guimanager.h"
#include "gui/guitext.h"

#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gfx/graphicsdevice.h"
#include "effect/renderstate_block.h"

/***************************************************************************************************
*
*	The bobbins below allows us to register this class along with its XML tag before main is called.
*	This means that the parts constructing it don't have to know about it.
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper( void ) { return NT_NEW CGuiSkinMenuButton(); }

// Register this class under it's XML tag
bool g_bMENUBUTTON = CGuiManager::Register( "MENUBUTTON", &ConstructWrapper );

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuButton::CGuiSkinMenuButton
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiSkinMenuButton::CGuiSkinMenuButton( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuButton::~CGuiSkinMenuButton
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiSkinMenuButton::~CGuiSkinMenuButton( void )
{
}


/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuButton::ProcessAttribute
*
*	DESCRIPTION		Passes attribute values on to specific handlers based on their title.
*
***************************************************************************************************/

bool CGuiSkinMenuButton::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		return m_obAction.ProcessAttribute(pcTitle, pcValue);
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuButton::ProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*
***************************************************************************************************/

bool CGuiSkinMenuButton::ProcessEnd( void )
{
	// Call the base first
	return super::ProcessEnd();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuButton::PostProcessEnd
*
*	DESCRIPTION		Carries out setup tasks that are valid only after we know all attributes are set
*					and is safe to be called again if SetAttribute is used to modify any of the
*					objects properties.
*
***************************************************************************************************/

void CGuiSkinMenuButton::PostProcessEnd( void )
{
	// Set the button text colour based on the menu items selectable state.
	UpdateTextColourForSelectability();
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuButton::SelectAction
*
*	DESCRIPTION		Activate button
*
***************************************************************************************************/

bool CGuiSkinMenuButton::SelectAction( int iPads )
{
	UNUSED(iPads);

	// Only allow the selection action if selectable.
	if	( Selectable() )
	{
		m_obAction.TriggerAction(GetParentScreen());
	}

	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuButton::StartAction
*
*	DESCRIPTION		Activate button
*
***************************************************************************************************/

bool CGuiSkinMenuButton::StartAction( int iPads )
{
	return SelectAction(iPads);
}

/***************************************************************************************************
*
*	FUNCTION		CGuiSkinMenuButton::Render
*
*	DESCRIPTION		Render our label
*
***************************************************************************************************/

bool CGuiSkinMenuButton::Render()
{
	return super::Render();
	//RenderMenuText(TextString());
}


/***************************************************************************************************
*
*	DESCRIPTION		BG button code
*
***************************************************************************************************/

static CXMLElement* ConstructWrapper2( void ) { return NT_NEW CGuiSkinMenuBGButton(); }
// Register this class under it's XML tag
bool g_bMENUBGBUTTON = CGuiManager::Register( "MENUBGBUTTON", &ConstructWrapper2 );


CGuiSkinMenuBGButton::CGuiSkinMenuBGButton( void )
{
	m_bHasTexture = false;

	m_fXPad = 0.0f;
	m_fYPad = 0.0f;
}

CGuiSkinMenuBGButton::~CGuiSkinMenuBGButton( void )
{

}

bool CGuiSkinMenuBGButton::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if (strcmp("bgimage", pcTitle) == 0)
		{
			if (!pcValue || strcmp("", pcValue) == 0)
			{
				m_bHasTexture = false;
			}
			else
			{
				m_obBGImage.SetTexture(pcValue);
				m_bHasTexture = true;
			}
			return true;
		}
		else if (strcmp("bgpad", pcTitle) == 0)
		{
			return GuiUtil::SetFloats(pcValue, &m_fXPad, &m_fYPad);
		}
	}

	return true;
}

bool CGuiSkinMenuBGButton::ProcessEnd( void )
{
	super::ProcessEnd();

	//setup image from text...
	GuiExtents obExt;
	GetExtents(obExt);

	m_obBGImage.SetWidth( obExt.fWidth + 2*m_fXPad );
	m_obBGImage.SetHeight( obExt.fHeight + 2*m_fYPad );

	m_obBGImage.SetPosition(CPoint(obExt.fX + obExt.fWidth*0.5f, obExt.fY + obExt.fHeight*0.5f, 0.0f));

	return true;
}

bool CGuiSkinMenuBGButton::Render()
{
	if (m_bHasTexture)
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		m_obBGImage.SetColour( CVector(1.0f,1.0f,1.0f,ScreenFade()) );
		m_obBGImage.Render();
	}

	return super::Render();
}
