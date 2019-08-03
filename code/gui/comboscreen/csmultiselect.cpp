/***************************************************************************************************
*
*	DESCRIPTION		This is a switch betweem n number of item returing the int that it is selected.
*					Only the selected one is rendered.
*
*	NOTES			
*
***************************************************************************************************/

#include "csmultiselect.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"

CSMultiSelect::CSMultiSelect()
: m_pCurrentItemCache( NULL )
, m_iCurrentItem( 0 )
, m_bHasChanged( false )
, m_bShowLeft( false )
, m_bShowRight( false )
{
	//init the arrow texture and size
	m_LeftArrow.SetTexture( "gui/frontend/textures/options/ow_left_arrow_colour_alpha.dds" );
	m_LeftArrow.SetWidth( (float)m_LeftArrow.GetTextureWidth() );
	m_LeftArrow.SetHeight( (float)m_LeftArrow.GetTextureHeight() );
	m_RightArrow.SetTexture( "gui/frontend/textures/options/ow_right_arrow_colour_alpha.dds" );
	m_RightArrow.SetWidth( (float)m_RightArrow.GetTextureWidth() );
	m_RightArrow.SetHeight( (float)m_RightArrow.GetTextureHeight() );

}

CSMultiSelect::~CSMultiSelect()
{
	//delete everything we have
	DeleteItems();
}

void	CSMultiSelect::MovePrev( void )
{
	//can we go back?
	if( m_iCurrentItem > 0 )
	{
	//update current pos
		--m_iCurrentItem;
	//cache the pointer to the prevoius one
		ntstd::List< CSListObjectBase* >::iterator Iter = m_ItemList.begin();
		for( int j = 0 ; j < m_iCurrentItem ; ++j )
		{
			Iter++;
		}
		m_pCurrentItemCache = (*Iter);
	//set has changed to true
		m_bHasChanged = true;

		if( 0 == m_iCurrentItem )
		{
			m_bShowLeft = false;
		}
		m_bShowRight = true;
	}
}

void	CSMultiSelect::MoveNext( void )
{
	//can we go forward?
	if( m_iCurrentItem < ( (int)m_ItemList.size() - 1 ) )
	{
	//update current pos
		++m_iCurrentItem;
	//cache the pointer to the next one
		ntstd::List< CSListObjectBase* >::iterator Iter = m_ItemList.begin();
		for( int j = 0 ; j < m_iCurrentItem ; ++j )
		{
			Iter++;
		}
		m_pCurrentItemCache = (*Iter);
	//set has chaged to true
		m_bHasChanged = true;

		if( m_iCurrentItem == ( (int)m_ItemList.size() - 1 ) )
		{
			m_bShowRight = false;
		}
		m_bShowLeft = true;
	}
}

void	CSMultiSelect::AddItem( CSListObjectBase* pObject )
{
	
	m_ItemList.push_back( pObject );
	//is this the first one?
	if( NULL == m_pCurrentItemCache ) 
	{
		m_pCurrentItemCache = pObject;
	}
	if( m_ItemList.size() > 1 )
	{
		m_bShowRight = true;
	}
}


//
// This will return a Zero based int. As per order they we added.
//
int		CSMultiSelect::GetCurrentSelection( void ) const
{
	return m_iCurrentItem;
}

void	CSMultiSelect::Render( void )
{
	if( NULL != m_pCurrentItemCache )
	{
		m_pCurrentItemCache->Render();
	}

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	if( true == m_bShowLeft )
	{
		m_LeftArrow.Render();
	}

	if ( true == m_bShowRight )
	{
		m_RightArrow.Render();
	}

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
}

void	CSMultiSelect::Update( void )
{
	//this doesn't do anything yet.
}

bool	CSMultiSelect::HasChanged( void ) const
{
	return m_bHasChanged;
}

void	CSMultiSelect::DeleteItems( void )
{
	m_pCurrentItemCache = NULL;
	ntstd::List< CSListObjectBase* >::iterator Iter = m_ItemList.begin();
	while( Iter != m_ItemList.end() )
	{
		delete (*Iter);
		Iter++;
	}
	m_ItemList.clear();
}

void	CSMultiSelect::ClearChangedState( void )
{
	m_bHasChanged = false;
}

void	CSMultiSelect::SetArrowPosition( const float fY, const float fLeftX, const float fRightX )
{
	m_fArrowsY = fY;
	m_fArrowsLeftX = fLeftX;
	m_fArrowsRightX = fRightX;

	m_LeftArrow.SetPosition( CPoint( fLeftX, fY, 0.0f ) );
	m_RightArrow.SetPosition( CPoint( fRightX, fY, 0.0f ) );
}
