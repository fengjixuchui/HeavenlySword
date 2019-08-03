/***************************************************************************************************
*
*	DESCRIPTION		This is a image render in the Combo Screen list.
*
*	NOTES			
*
***************************************************************************************************/
#if 0
#include "cslistobjectcombobuttons.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"


CSListObjectComboButtons::CSListObjectComboButtons( void )
{

}

CSListObjectComboButtons::~CSListObjectComboButtons( void )
{
}

	//overides
void CSListObjectComboButtons::Render( void )
{
	if( NULL != m_pOffsetTransform )
	{
		m_CrossButtonSprites.SetPosition( m_pOffsetTransform->GetWorldTranslation() + CPoint( m_fX, m_fY, 0.0f ) );
	}
	
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
	m_CrossButtonSprites.Render();
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
}

void CSListObjectComboButtons::Update( void )
{

}

	//Construct combo images
void CSListObjectComboButtons::SetTexture( const char* ccTextureName )
{
	m_CrossButtonSprites.SetTexture( ccTextureName );
	m_CrossButtonSprites.SetColour( CVector( 1.0f, 1.0f, 1.0f, 1.0f ) );
	m_CrossButtonSprites.SetHeight( m_fHeight );
	m_CrossButtonSprites.SetWidth( m_fWidth );
	
}

#endif
