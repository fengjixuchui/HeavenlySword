/***************************************************************************************************
*
*	DESCRIPTION		This is a will render and update a list of item on the screen, scrolling should there be to many.
*
*	NOTES			
*
***************************************************************************************************/


/*
This is a list control^_^
You can add objects to it, one per 'Line'
All objects must be have their positioned controlled by the same 'Transform'
This is because once you make a Text string you can not reposition it with out recreating it,
meaning I would have to store all the strings and create every object every time it moves. 
Since I plan to do smooth per pixel scrolls, that would be a lot.
*/


#include "cslistcontrol.h"
#include "gfx/renderer.h"
#include "gfx/renderstates.h"
#include "gui/guisettings.h"
#include "gui/guimanager.h"

//This sets up the selection icon and the up and down arrows
CSListControl::CSListControl()
: m_iNumItems( 0 )
, m_iCurrentTopItem( 0 )
, m_iNumItemsPerPage( 0 )
, m_iCurrentSelection( 0 )
, m_fItemHeight( 0.0f )
, m_bShowUpArrow( true )
, m_bShowDownArrow( false )
{
	//Arrow textures
	m_UpArrow.SetTexture( "gui/frontend/textures/options/ow_up_arrow_colour_alpha.dds" );
	m_DownArrow.SetTexture( "gui/frontend/textures/options/ow_down_arrow_colour_alpha.dds" );
	
	//Get the 'Global' selection texture
	const GuiSettingsMenuSelectDef* pobSettings = CGuiManager::Get().GuiSettings()->MenuSelect();
	m_SelectionIcon.SetTexture( pobSettings->CursorTexture() );

	//set the texture and colour of the sprites.
	m_SelectionIcon.SetWidth( (float)m_SelectionIcon.GetTextureWidth() );
	m_SelectionIcon.SetHeight( (float)m_SelectionIcon.GetTextureHeight() );

	m_UpArrow.SetColour( CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0 );
	m_DownArrow.SetColour( CVector( 1.0f, 1.0f, 1.0f, 1.0f ), 0 );
	m_UpArrow.SetHeight( (float)m_UpArrow.GetTextureHeight() );
	m_UpArrow.SetWidth( (float)m_UpArrow.GetTextureWidth() );
	m_DownArrow.SetHeight( (float)m_DownArrow.GetTextureHeight() );
	m_DownArrow.SetWidth( (float)m_DownArrow.GetTextureWidth() );

	//Init the sperators
	m_Seperators.Init();
}

CSListControl::~CSListControl()
{
	//We will delete all things that are given to us
	DeleteAllChildren();
}

void CSListControl::Update( void )
{
	//update all the objects that are 'active' i.e. on screen.
	int iEnd = CalcCurrentItemRange();
	ntstd::List< CSListObjectBase* >::iterator itor = m_ItemList.begin();
	AdvanceIteratorToStart( itor );
	for ( int j = m_iCurrentTopItem ; j < iEnd ; ++j )
	{
		(*itor)->Update();
		itor++;
	}
}

void CSListControl::Render( void )
{
	//how many items to we have to render?
	int iEnd = CalcCurrentItemRange();
	if( iEnd > (int)m_ItemList.size() )
	{
		iEnd = m_ItemList.size();
	}

	//skip past all the items that we are not rendering
	ntstd::List< CSListObjectBase* >::iterator itor = m_ItemList.begin();
	AdvanceIteratorToStart( itor );

	//render 'active' ones
	for ( int j = m_iCurrentTopItem ; j < iEnd ; ++j )
	{
		(*itor)->Render();
		itor++;
	}

	//render arrows
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	//show the arrows if we need to
	if( m_bShowDownArrow )
	{
		m_DownArrow.Render();
	}
	if( m_bShowUpArrow ) 
	{
		m_UpArrow.Render();
	}

	//render the selection sprite
	m_SelectionIcon.Render();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	//draw the lines
	m_Seperators.Render();
}

void CSListControl::NumItemsPerPage( const int iNum )
{
	m_iNumItemsPerPage = iNum;
}

void CSListControl::AddItem( CSListObjectBase* pObject )
{
	//put it on the list
	m_ItemList.push_back( pObject );

	//we have one more
	++m_iNumItems;

	//if this is the first item
	if( 1 == m_iNumItems )
	{
		//move the selection  cursor to select it
		UpdateSelectionCursor();
	}

	//have we got more items than we can show at once
	if( m_iNumItems > m_iNumItemsPerPage )
	{
		//is this the first time we have gone over the on screen limit
		if( false == m_bShowDownArrow )
		{
			//show the scroll down arrow
			m_bShowDownArrow = true;
			//set the seperators to render for all onscreen items, to wrap the item
			m_Seperators.SetNumberToRender( m_iNumItemsPerPage + 1 );
		}
	}
	else
	{
		//wrap the seperators around the current number of items we have.
		m_Seperators.SetNumberToRender( m_iNumItems + 1 );
	}
}

bool CSListControl::ScrollDown( void )
{
	ntAssert( NULL != m_pObjectControlTransform );

	//can we scroll down?
	if( m_iCurrentSelection >= m_iNumItems )
	{
		//return no, so we can play a implementation specific sound upon failure.
		return false;
	}

	//move to the next item then
	++m_iCurrentSelection;

	//make the item we just selected the center were possible
	CenterItemsAroundSelectionCursor();

	//make sure the cursor is there
	UpdateSelectionCursor();

	//did we just select the last item
	if( m_iCurrentSelection == m_iNumItems ) 
	{
		//hide the scroll down arrow to inform the user they are at the bottom
		m_bShowDownArrow = false;
	}
	//since we have gone down we must be able to go up, so inform the user of the fact
	m_bShowUpArrow = true;

	//return true so we can play a implementation specific sound upon sucess.
	return true;
}

bool CSListControl::ScrollUp( void )
{
	ntAssert( NULL != m_pObjectControlTransform );
	
	//can we scroll up
	if( m_iCurrentSelection <= 0 )
	{
		//return no, so we can play a implementation specific sound upon failure.
		return false;
	}

	//move to the prevoius item then
	--m_iCurrentSelection;

	//make the item we just selected the center were possible
	CenterItemsAroundSelectionCursor();

	//make sure the cursor is there
	UpdateSelectionCursor();

	//did we just select the first item?
	if( 0 == m_iCurrentSelection ) 
	{
		//hide the up arrow to inform the user they are at the top
		m_bShowUpArrow = false;
	}

	//since we have moved up we must be able to move back down, so inform the user of the fact.
	m_bShowDownArrow = true;

	//return true so we can play a implementation specific sound upon sucess.
	return true;
	
}

void CSListControl::SetControlingTransform( Transform* pTransform )
{
	m_pObjectControlTransform = pTransform;
}									   


void CSListControl::JumpToStart( void )
{
	//make the current top item and slection the first one
	m_iCurrentTopItem = 0;
	m_iCurrentSelection = 0;

	//do not scroll the items at all
	CPoint Temp = m_pObjectControlTransform->GetLocalTranslation();
	Temp.Y() = 0.0f;
	m_pObjectControlTransform->SetLocalTranslation( Temp );
	
	//make the seperators refelct this
	MoveSeperators();
}

void CSListControl::DeleteAllChildren( void )
{
	//get all items
	ntstd::List< CSListObjectBase* >::iterator itor = m_ItemList.begin();
	//for each element
	while ( itor != m_ItemList.end() )
	{
		//delete it
		CSListObjectBase* pTemp = (*itor);
		delete pTemp;
		//next
		itor++;
	}
	//clean up the list
	m_ItemList.clear();
	//we are empty
	m_iNumItems = 0;
	//no scrolling for you
	m_bShowDownArrow = false;
	m_bShowUpArrow = false;
}

//given the current top item what is the index of the last one we should render.
int CSListControl::CalcCurrentItemRange( void )
{
	//what would be the natural ending should we have a full screens worth of items
	int iProperEnd = m_iCurrentTopItem + m_iNumItemsPerPage;
	//can we meet that demand?
	if( iProperEnd > m_iNumItems )
	{
		//no return the actual end
		return m_iNumItems;
	}
	//yes, thats okay
	return iProperEnd;
}

void CSListControl::AdvanceIteratorToStart( ntstd::List< CSListObjectBase* >::iterator &itor )
{
	//from start to current
	for ( int j = 0 ; j < m_iCurrentTopItem ; ++j )
	{
		//skip
		itor++;
	}
}

void CSListControl::SetItemHeight( const float& fHeight )
{
	//set the gap
	m_fItemHeight = fHeight;
	//move any sperators to reflect this change
	MoveSeperators();
}

void CSListControl::SetUpAndDownArrowItems ( float fXPos, float fTopY, float fBottomY  )
{
	//move em
	m_UpArrow.SetPosition( CPoint( fXPos, fTopY, 0.0f ) );
	m_DownArrow.SetPosition( CPoint( fXPos, fBottomY, 0.0f ) );
}

void CSListControl::MoveSeperators( void )
{
	//from the position of the items were do we put the bars, in the y
	const static float kBarYOffset = 20.0f;

	//clip the lines to the number of items we have to wrap
	int iNumLinesToUpdate = m_iNumItemsPerPage;
	if( iNumLinesToUpdate > m_iNumItems )
	{
		iNumLinesToUpdate  = m_iNumItems;
	}
	//if we have none to update then return
	if( iNumLinesToUpdate <= 0 )
	{
		return;
	}

	//add one to cover the top, becaues 1 item has 2 lines^_^  do that above
	//++iNumLinesToUpdate;

	CPoint Temp = m_pObjectControlTransform->GetLocalTranslation();
	Temp += CPoint( 0.0f, m_fY + ( (float)m_iCurrentTopItem * m_fItemHeight ), 0.0f );
	//place it in the center of the screen
	Temp.X() = 640.0f;
	//set the first one to first position - 10
	m_Seperators.SetPosition( Temp - CPoint( 0.0f, kBarYOffset, 0.0f ), 0 );
	//set 2nd one to first position + height -10 
	m_Seperators.SetPosition( Temp + CPoint( 0.0f, m_fItemHeight - kBarYOffset, 0.0f ), 1 );
	//..nth pos of 2nd + height
	if( iNumLinesToUpdate <= 1 )
	{
		//we only have the 2 lines to do so exit
		return;
	}

	for( int j = 2 ; j <= iNumLinesToUpdate ; ++j )
	{
		//update lines
		m_Seperators.SetPosition( Temp + CPoint( 0.0f, ( m_fItemHeight * j ) - kBarYOffset, 0.0f ), j );
	}
}

void CSListControl::SetTopXAndYOfControl( const float& fXpos, const float& fYpos )
{
	m_fX = fXpos;
	m_fY = fYpos;
}

float CSListControl::GetItemHeight( void )
{
	return m_fItemHeight;
}

void CSListControl::UpdateSelectionCursor( void )
{

	//here i make the asumption that all 1 items have the same offset from the top of the item
	//ie. if we have two lines in the control
	//	this gap is always the same ->						BBBBBBBB
	//										AAAAAAA			BBBBBBBB
	//										AAAAAAA			BBBBBBBB
	//														BBBBBBBB
	ntstd::List< CSListObjectBase* >::iterator itor = m_ItemList.begin();

	float fFirstX, fFirstY;

	(*itor)->GetPosition( fFirstX, fFirstY );

	//advnance to the first renderable item
	for ( int j = 0 ; j < m_iCurrentTopItem ; ++j )
	{
		itor++;
	}

	//store its position
	float fTopX, fTopY;
	(*itor)->GetPosition( fTopX, fTopY );

	//advance to the item we have selected
	for( int j = m_iCurrentTopItem + 1; j < m_iCurrentSelection ; ++j )
	{
		itor++;
	}

	//get is position
	float fX, fY;
	(*itor)->GetPosition( fX, fY );

	//get it relative by sutacting the pos of the first item
	fX -= fTopX;
	fY -= fTopY;

	//get it into screen position by adding the position of the list control
	fX += m_fX;
	fY += m_fY;

	//offset it the distance the selection point in the 'Item' is from the top of each 'Item'
	fX += fFirstX - m_fX;
	fY += fFirstY - m_fY;

	//offset the cursor by the Global amount so it is in the same relative position as all the other selections in the game
	const GuiSettingsMenuSelectDef* pobSettings = CGuiManager::Get().GuiSettings()->MenuSelect();
	CVector Temp = pobSettings->CursorOffset();
	Temp += CVector( fX, fY, 0.0f, 0.0f );

	//put it there, finally ^_^
	m_SelectionIcon.SetPosition( CPoint( Temp.X(), Temp.Y(), 0.0f ) );
}

void CSListControl::CenterItemsAroundSelectionCursor( void )
{
	int iFinalOffset = 0;
	//calc the desired middle of the onscreen section
	int iMiddleItem = m_iNumItemsPerPage / 2;

	//calc end point cliping
	int iEndPointClipping = m_iNumItems - iMiddleItem;

	if( m_iCurrentSelection <= iMiddleItem )
	{
		//then we are not scrollbale anymore so set us to the top
		iFinalOffset = 0;
	}
	else if ( m_iCurrentSelection >= iEndPointClipping )
	{
		//we can not scroll down the list anymore, so set us to the bottom
		iFinalOffset = m_iNumItems - m_iNumItemsPerPage;
	}
	else 
	{
		//the slection is in the middle and we have to move 
		iFinalOffset = m_iCurrentSelection - iMiddleItem;
	}

	m_iCurrentTopItem = iFinalOffset;

	//scroll all items to show the item we want in the best way
	CPoint Temp = m_pObjectControlTransform->GetLocalTranslation();
	Temp.Y() = -( (float)iFinalOffset * m_fItemHeight );
	m_pObjectControlTransform->SetLocalTranslation( Temp );

}
