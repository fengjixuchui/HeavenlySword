/***************************************************************************************************
*
*	DESCRIPTION		This will render the new word and the medalion behind it.
*
*	NOTES			
*
***************************************************************************************************/

#include "cslistobjectnew.h"
#include "gui/guisettings.h"

#include "gfx/renderer.h"
#include "gfx/renderstates.h"

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::CSListObjectBase
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
CSListObjectNew::CSListObjectNew()
{
	m_pStr = NULL;
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::CSListObjectBase
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/
CSListObjectNew::~CSListObjectNew()
{
	if( NULL != m_pStr )
	{
		CStringManager::Get().DestroyString( m_pStr );
		m_pStr = NULL;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::Init
*
*	DESCRIPTION		Creates the text string and the sprite image
*
***************************************************************************************************/
bool CSListObjectNew::Init( void )
{
	//step 1 make sure we have a transform, and have not inited before
	ntError( NULL != m_pOffsetTransform );
	ntError( NULL == m_pStr );

	//step 2 make the string
	CStringDefinition StrDef;
	StrDef.m_fBoundsHeight = 32.0f;
	StrDef.m_fBoundsWidth = 128.0f;
	StrDef.m_fXOffset = m_fX;
	StrDef.m_fYOffset = m_fY;
	StrDef.m_fZOffset = 0.0f;

	const char* pcFont = CGuiManager::Get().GuiSettings()->BodyFont();
	StrDef.m_pobFont = CFontManager::Get().GetFont(pcFont);

	StrDef.m_eHJustify = CStringDefinition::JUSTIFY_CENTRE;
	StrDef.m_eVJustify = CStringDefinition::JUSTIFY_MIDDLE;

	m_pStr = CStringManager::Get().MakeString( "COMBOMENU_NEW", StrDef, m_pOffsetTransform, CGuiUnit::RENDER_SCREENSPACE );
	ntAssert( NULL != m_pStr );
	CVector TempColour( 0.0f, 0.0f, 0.0f, 1.0f );
	m_pStr->SetColour( TempColour );

	//step 3 make the sprite
	m_Medalion.SetTexture( "gui/frontend/textures/comboscreen/ow_combonewtab_colour_alpha_nomip.dds" );
	m_Medalion.SetColour( CVector( 1.0f, 1.0f, 1.0f, 1.0f ) );
	m_Medalion.SetHeight( 32.0f );
	m_Medalion.SetWidth( 128.0f );
	m_Medalion.SetPosition( m_pOffsetTransform->GetWorldTranslation() + CPoint( m_fX, m_fY, 0.0f ) );

	return true;

}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::HasMoved
*
*	DESCRIPTION		Moves the sprite to match the  position of the text.
*
***************************************************************************************************/
void CSListObjectNew::HasMoved( void )
{
	m_Medalion.SetPosition( m_pOffsetTransform->GetWorldTranslation() + CPoint( m_fX, m_fY, 0.0f ) );
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::Render
*
*	DESCRIPTION		This draws the sprite and the text.
*
***************************************************************************************************/
void CSListObjectNew::Render( void )
{
	
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
	m_Medalion.Render();
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	if( NULL != m_pStr )
	{
		m_pStr->Render();
	}
}

