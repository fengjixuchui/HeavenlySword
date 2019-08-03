//-----------------------------------------------------------------------------
//!
//!	\file /game/guiskinmenuimagegrid.cpp
//!
//! Implementation of class to manage a grid of selectable images.
//!
//-----------------------------------------------------------------------------

//! Includes
#include "guiskinmenuimagegrid.h"
#include "gfx/renderer.h"
#include "gui/guisettings.h"
#include "gui/guisound.h"
#include "objectdatabase/dataobject.h"
#include "gui/guilua.h"
#include "lua/ninjalua.h"
#include "gui/guimanager.h"
#include "gui/guitext.h"
#include "anim/hierarchy.h"
#include "movies/movieinstance.h"
#include "gui/guiscreen.h"

#define TRUTH_STRING( bEnable )	( bEnable?"true":"false" )

//-----------------------------------------------------------------------------------------
//
//	The bobbins below allows us to register this class along with its XML tag before main
//  is called. This means that the parts constructing it don't have to know about it.
//
//-----------------------------------------------------------------------------------------
static CXMLElement* ConstructWrapperImageGrid( void ) { return NT_NEW CGuiSkinMenuImageGrid(); }

static CXMLElement* ConstructWrapperImageGridCell( void ) { return NT_NEW CGuiSkinMenuImageGridCell(); }

// Register this class under it's XML tag
bool g_bMENUIMAGEGRID = CGuiManager::Register( "MENUIMAGEGRID", &ConstructWrapperImageGrid );
bool g_bMENUIMAGEGRIDCELL = CGuiManager::Register( "MENUIMAGEGRIDCELL", &ConstructWrapperImageGridCell );

//-----------------------------------------------------------------------------------------
//!
//!	CellInit::CellInit()
//!
//! Constructor for cell initialization structure.
//!
//-----------------------------------------------------------------------------------------
CGuiSkinMenuImageGridCell::CellInit::CellInit() :
	m_iIndex( 0 ),
	m_iRow( 0 ),
	m_fLeft( 0.0f ),
	m_fTop( 0.0f ),
	m_fWidth( 0.0f ),
	m_fHeight( 0.0f ),
	m_pcTextureNameFilterImage( NULL ),
	m_pcTextureNameBorderImage( NULL ),
	m_FilterSize( _Size( 0.0f, 0.0f ) ),
	m_BorderSize( _Size( 0.0f, 0.0f ) )
{
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::CGuiSkinMenuImageGrid()
//!
//! Image grid constructor.
//!
//-----------------------------------------------------------------------------------------
CGuiSkinMenuImageGrid::CGuiSkinMenuImageGrid() :
	m_obLeftTop( 0.0f, 0.0f, 0.0f ),
	m_obRightBottom( 0.0f, 0.0f, 0.0f ),
	m_fCellSpacing( 20.0f ),
	m_fSliderStartY( 0.0f ),
	m_fSliderEndY( 0.0f ),
	m_fSliderX( 0.0f ),
	m_iCellsPerRow( 1 ),
	m_iCurrentCell( 0 ),
	m_iNumCells( 0 ),
	m_iRowsToShow( 1 ),
	m_iFirstVisibleRow( 0 ),
	m_iLastVisibleRow( 0 ),
	m_iNumRows( 0 ),
	m_iLastRow( 0 ),
	m_iLastCell( 0 ),
	m_CellSize( _Size( 0.0f, 0.0f ) ),
	m_CursorSize( _Size( 0.0f, 0.0f ) ),
	m_FilterSize( _Size( 0.0f, 0.0f ) ),
	m_BorderSize( _Size( 0.0f, 0.0f ) ),
	m_MovieID( 0 ),
	m_bRender( false ),
	m_bCheckMovieIsPlaying( false )
{
	NullSpritePointers();
	NullStringPointers();

	m_aiSliderArrowSpritesToRender[ IG_SLIDER_UP_ARROW ] = GC_SPRITE_SLIDER_UP_ARROW;
	m_aiSliderArrowSpritesToRender[ IG_SLIDER_DOWN_ARROW ] = GC_SPRITE_SLIDER_DOWN_ARROW;
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::NullSpritePointers()
//!
//! Null the sprite pointers.
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::NullSpritePointers( void )
{
	for ( int iSprite = 0; iSprite < GC_NUM_SPRITE; iSprite++ )
	{
		m_apobSprite[ iSprite ] = NULL;
	}
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::NullStringPointers()
//!
//! Null the string pointers.
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::NullStringPointers( void )
{
	for ( int iLoop = 0; iLoop < IG_NUM_STRINGS; iLoop++ )
	{
		m_apcString[ iLoop ] = NULL;
	}
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::~CGuiSkinMenuImageGrid()
//!
//! Image grid destructor.
//!
//-----------------------------------------------------------------------------------------
CGuiSkinMenuImageGrid::~CGuiSkinMenuImageGrid()
{
	StopMovie();

	// Destroy all loaded grid cells.
	// XMLParse will destroy the CGuiSkinMenuImageGridCell objects created during XML file processing.
	for ( ntstd::List<CGuiSkinMenuImageGridCell*>::iterator obIt = m_obCellsAll.begin(); obIt != m_obCellsAll.end(); )
	{
		// Remove from list.
		obIt = m_obCellsAll.erase( obIt );
	}

	// Destroy working list.
	for ( ntstd::List<CGuiSkinMenuImageGridCell*>::iterator obIt = m_obCells.begin(); obIt != m_obCells.end(); )
	{
		// Remove from list.
		obIt = m_obCells.erase( obIt );
	}

	DeleteStringObjects();
	DeleteSprites();
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::DeleteSprites()
//!
//! Delete all the sprite objects.
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::DeleteSprites( void )
{
	for ( int iSprite = 0; iSprite < GC_NUM_SPRITE; iSprite++ )
	{
		if ( m_apobSprite[ iSprite ] )
		{
			NT_DELETE( m_apobSprite[ iSprite ] );
			m_apobSprite[ iSprite ] = NULL;
		}
	}
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::DeleteStringsObjects()
//!
//! Delete the string objects.
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::DeleteStringObjects( void )
{
	for ( int iLoop = 0; iLoop < IG_NUM_STRINGS; iLoop++ )
	{
		if ( m_apcString[ iLoop ] )
		{
			NT_DELETE_ARRAY( m_apcString[ iLoop ] );
			m_apcString[ iLoop ] = NULL;
		}
	}
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::Initialise()
//!
//! Initialise function, responsible for calculating all internal variables, populating
//! grid cells, creating all sprites.
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::Initialise( void )
{
	// Build a new list containing only valid items.
	for ( ntstd::List<CGuiSkinMenuImageGridCell*>::iterator obIt = m_obCellsAll.begin(); obIt != m_obCellsAll.end(); obIt++ )
	{
		if ( IncludeCellInGrid( (*obIt) ) )
		{
			m_obCells.push_back( (*obIt) );
		
			m_iNumCells++;
		}
	}

	// Calculate index for the last cell.
	if ( m_iNumCells > 0 )
	{
		m_iLastCell = m_iNumCells-1;
	}

	m_iNumRows = CalculateNumberOfRows( m_iNumCells, m_iCellsPerRow );

	// Calculate the last row.
	if ( m_iNumRows > 0 )
	{
		m_iLastRow = m_iNumRows-1;
	}

	// Calculate the last visible row.
	if ( m_iRowsToShow > 1)
	{
		m_iLastVisibleRow = min( (m_iFirstVisibleRow + (m_iRowsToShow-1)), m_iLastRow );
	}

	// Calculate extents for the visible grid, LT, RB.
	CalculateExtents();

	// Populate cell list.
	if ( m_iCellsPerRow > 0 )
	{
		int iRow, iCol;
		float fLeft, fTop;

		int iIndex = 0;

		for ( ntstd::List<CGuiSkinMenuImageGridCell*>::iterator obIt = m_obCells.begin(); obIt != m_obCells.end(); obIt++ )
		{
			struct CGuiSkinMenuImageGridCell::CellInit obCellInit;

			if ( IncludeCellInGrid( (*obIt) ) )
			{
				// Calculate cell position in the grid.
				iRow = (iIndex/m_iCellsPerRow);
				iCol = iIndex-(iRow*m_iCellsPerRow);
				fLeft = m_obLeftTop.X() + ((float)iCol * (m_CellSize.m_fWidth + m_fCellSpacing));
				fTop = m_obLeftTop.Y() + ((float)iRow * (m_CellSize.m_fHeight + m_fCellSpacing));

				// Populate the cell init structure.
				obCellInit.m_iIndex = iIndex;
				obCellInit.m_iRow = iRow;
				obCellInit.m_fLeft = fLeft;
				obCellInit.m_fTop = fTop;
				obCellInit.m_fWidth = m_CellSize.m_fWidth;
				obCellInit.m_fHeight = m_CellSize.m_fHeight;
				obCellInit.m_pcTextureNameFilterImage = m_apcString[IG_STRING_TEXTURE_NAME_FILTER_IMAGE ];
				obCellInit.m_pcTextureNameBorderImage = m_apcString[IG_STRING_TEXTURE_NAME_BORDER_IMAGE ];
				obCellInit.m_BorderSize = m_BorderSize;
				obCellInit.m_FilterSize = m_FilterSize;

				(*obIt)->Initialise( obCellInit );

				iIndex++;
			}
		}
	}

	CreateCursorSprite();
	UpdateCursorPosition();

	UpdateCellSpriteRenderStates();

	CreateSliderBarSprites();
	UpdateSliderBarSprites();

	EnableRender( true );
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::CreateFullscreenBorderSprites()
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::CreateFullscreenBorderSprites( void )
{
	// Create the sprites
	//------------------------------------------------------------------------------------------------
	char acPartName[ 256 ];
	int iCount = 0;

	for ( int iSpriteIndex = CG_SPRITE_FULLSCREEN_BORDER_PART_TR; iSpriteIndex <= CG_SPRITE_FULLSCREEN_BORDER_PART_CENTRE; iSpriteIndex++ )
	{
		ScreenSprite* pobSprite = NT_NEW ScreenSprite;
		
		if ( pobSprite )
		{
			sprintf( acPartName, "gui/frontend/textures/art_gallery/fullscreen_image_border/ow_border_colour_alpha_nomip_%d.dds", iCount );

			pobSprite->SetTexture( acPartName );
			pobSprite->SetWidth( (float)pobSprite->GetTextureWidth() );
			pobSprite->SetHeight( (float)pobSprite->GetTextureHeight() );
			
			m_apobSprite[ iSpriteIndex ] = pobSprite;
		
			iCount++;
		}
	}

	// Calc some useful values
	//-------------------------------------------------------------------------------------------------
	float fImageWidth = 512.0f;
	float fImageHeight = 512.0f;

	float fHalfImageWidth = fImageWidth*0.5f;
	float fHalfImageHeight = fImageHeight*0.5f;

	uint32_t uiCornerWidth = m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_TR ]->GetTextureWidth();
	uint32_t uiCornerHeight = m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_TR ]->GetTextureHeight();

	uint32_t uiHalfCornerWidth = uiCornerWidth/2;
	uint32_t uiHalfCornerHeight = uiCornerHeight/2;


	// Do the centre sprite
	//----------------------------------------------------------------------------------------
	CPoint obCentrePos;
	GetCentre( obCentrePos );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_CENTRE ]->SetPosition( obCentrePos );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_CENTRE ]->SetWidth( fImageWidth );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_CENTRE ]->SetHeight( fImageHeight );


	// Do the corner sprites
	//----------------------------------------------------------------------------------------
	float fXOffset = fHalfImageWidth + (float)uiHalfCornerWidth;
	float fYOffset = fHalfImageHeight + (float)uiHalfCornerHeight;

	// Top right
	float fX = obCentrePos.X() + fXOffset;
	float fY = obCentrePos.Y() - fYOffset;
	CPoint obTRPos( fX, fY, 0.0f );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_TR ]->SetPosition( obTRPos );

	// Bottom right
	fX = obCentrePos.X() + fXOffset;
	fY = obCentrePos.Y() + fYOffset;
	CPoint obBRPos( fX, fY, 0.0f );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_BR ]->SetPosition( obBRPos );

	// Bottom Left
	fX = obCentrePos.X() - fXOffset;
	fY = obCentrePos.Y() + fYOffset;
	CPoint obBLPos( fX, fY, 0.0f );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_BL ]->SetPosition( obBLPos );

	// Top Left
	fX = obCentrePos.X() - fXOffset;
	fY = obCentrePos.Y() - fYOffset;
	CPoint obTLPos( fX, fY, 0.0f );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_TL ]->SetPosition( obTLPos );

	
	// Do the edge sprites
	//----------------------------------------------------------------------------------------
	fXOffset = fHalfImageWidth + (float)uiHalfCornerWidth;
	fYOffset = fHalfImageHeight + (float)uiHalfCornerHeight;

	// Top
	fX = obCentrePos.X();
	fY = obCentrePos.Y() - fYOffset;
	CPoint obTPos( fX, fY, 0.0f );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_T ]->SetPosition( obTPos );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_T ]->SetWidth( fImageWidth );

	// Right
	fX = obCentrePos.X() + fXOffset;
	fY = obCentrePos.Y();
	CPoint obRPos( fX, fY, 0.0f );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_R ]->SetPosition( obRPos );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_R ]->SetHeight( fImageHeight );

	// Bottom
	fX = obCentrePos.X();
	fY = obCentrePos.Y() + fYOffset;
	CPoint obBPos( fX, fY, 0.0f );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_B ]->SetPosition( obBPos );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_B ]->SetWidth( fImageWidth );

	// Left
	fX = obCentrePos.X() - fXOffset;
	fY = obCentrePos.Y();
	CPoint obLPos( fX, fY, 0.0f );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_L ]->SetPosition( obLPos );
	m_apobSprite[ CG_SPRITE_FULLSCREEN_BORDER_PART_L ]->SetHeight( fImageHeight );
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::DeleteFullscreenBorderSprites()
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::DeleteFullscreenBorderSprites( void )
{
	for ( int iSpriteIndex = CG_SPRITE_FULLSCREEN_BORDER_PART_TR; iSpriteIndex <= CG_SPRITE_FULLSCREEN_BORDER_PART_CENTRE; iSpriteIndex++ )
	{
		if ( m_apobSprite[ iSpriteIndex ] )
		{
			NT_DELETE( m_apobSprite[ iSpriteIndex ] );

			m_apobSprite[ iSpriteIndex ] = NULL;
		}
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::CreateCursorSprite
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::CreateCursorSprite( void )
{
	if ( m_apcString[ IG_STRING_TEXTURE_NAME_CURSOR ] )
	{
		ScreenSprite* pobSprite = NT_NEW ScreenSprite;
		if ( pobSprite )
		{
			pobSprite->SetTexture( m_apcString[ IG_STRING_TEXTURE_NAME_CURSOR ] );
			pobSprite->SetWidth( m_CursorSize.m_fWidth );
			pobSprite->SetHeight( m_CursorSize.m_fHeight );
		
			m_apobSprite[ GC_SPRITE_CURSOR ] = pobSprite;
		}
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::CreateSliderBarSprites
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::CreateSliderBarSprites( void )
{
	uint32_t uiSliderBarTopHeight = 0;
	uint32_t uiSliderBarBottomHeight = 0;
	uint32_t uiSliderBarTopHalfHeight = 0;
	uint32_t uiSliderBarBottomHalfHeight = 0;
	float fSliderBarX = m_obRightBottom.X() + ( (CGuiManager::Get().BBWidth() - m_obRightBottom.X()) * 0.5f );

	// Create top sprite.
	ScreenSprite* pobSprite = NT_NEW ScreenSprite;
	CPoint obPosSliderBarTop = m_obLeftTop;
	if ( pobSprite )
	{
		pobSprite->SetTexture( "gui/frontend/textures/common/scroll_bar/ow_scroll_bar_top_colour_alpha_nomip.dds" );
	
		uiSliderBarTopHeight = pobSprite->GetTextureHeight();
		uiSliderBarTopHalfHeight = uiSliderBarTopHeight/2;

		obPosSliderBarTop = CPoint( fSliderBarX, m_obLeftTop.Y() + (float)uiSliderBarTopHalfHeight, 0.0f );
		pobSprite->SetPosition( obPosSliderBarTop );
		pobSprite->SetWidth( (float)pobSprite->GetTextureWidth() );
		pobSprite->SetHeight( (float)uiSliderBarTopHeight );
	
		m_apobSprite[ GC_SPRITE_SLIDER_BAR_TOP ] = pobSprite;
	}
	
	// Create bottom sprite.
	pobSprite = NT_NEW ScreenSprite;
	CPoint obPosSliderBarBottom = m_obLeftTop;
	if ( pobSprite )
	{
		pobSprite->SetTexture( "gui/frontend/textures/common/scroll_bar/ow_scroll_bar_bottom_colour_alpha_nomip.dds" );

		uiSliderBarBottomHeight = pobSprite->GetTextureHeight();
		uiSliderBarBottomHalfHeight = uiSliderBarBottomHeight/2;

		obPosSliderBarBottom = CPoint( fSliderBarX, m_obRightBottom.Y() - (float)uiSliderBarBottomHalfHeight, 0.0f );
		pobSprite->SetPosition( obPosSliderBarBottom );
		pobSprite->SetWidth( (float)pobSprite->GetTextureWidth() );
		pobSprite->SetHeight( (float)uiSliderBarBottomHeight );

		m_apobSprite[ GC_SPRITE_SLIDER_BAR_BOTTOM ] = pobSprite;
	}

	// Create middle sprite.
	pobSprite = NT_NEW ScreenSprite;
	if ( pobSprite )
	{
		pobSprite->SetTexture( "gui/frontend/textures/common/scroll_bar/ow_scroll_bar_middle_colour_alpha_nomip.dds" );

		float fYOpenEndOfSliderBarTop = obPosSliderBarTop.Y() + (float)uiSliderBarTopHalfHeight;
		float fYOpenEndOfSliderBarBottom = obPosSliderBarBottom.Y() - (float)uiSliderBarBottomHalfHeight;

		float fLength = (fYOpenEndOfSliderBarBottom-fYOpenEndOfSliderBarTop);

		CPoint obPos( fSliderBarX, (obPosSliderBarTop.Y() + obPosSliderBarBottom.Y()) * 0.5f, 0.0f );
		pobSprite->SetPosition( obPos );
		pobSprite->SetWidth( (float)pobSprite->GetTextureWidth() );
		pobSprite->SetHeight( fLength );

		m_apobSprite[ GC_SPRITE_SLIDER_BAR_MIDDLE ] = pobSprite;
	}

	// Create slider.
	m_fSliderStartY = obPosSliderBarTop.Y() + 13.0f;
	m_fSliderEndY = obPosSliderBarBottom.Y() - 9.0f;
	m_fSliderX = fSliderBarX;

	pobSprite = NT_NEW ScreenSprite;
	if ( pobSprite )
	{
		pobSprite->SetTexture( "gui/frontend/textures/common/scroll_bar/ow_scroll_bar_colour_alpha_nomip.dds" );
	
		CPoint obPos( m_fSliderX, m_fSliderStartY, 0.0f );
		pobSprite->SetPosition( obPos );
		pobSprite->SetWidth( (float)pobSprite->GetTextureWidth() );
		pobSprite->SetHeight( (float)pobSprite->GetTextureHeight() );

		m_apobSprite[ GC_SPRITE_SLIDER ] = pobSprite;
	}

	// Create the top arrow.
	pobSprite = NT_NEW ScreenSprite;
	if ( pobSprite )
	{
		pobSprite->SetTexture( "gui/frontend/textures/common/scroll_bar/ow_up_arrow_colour_alpha_nomip.dds" );
		
		float fY = obPosSliderBarTop.Y() - ( uiSliderBarTopHalfHeight + ((float)pobSprite->GetTextureHeight() * 0.3f) );
		CPoint obPos( m_fSliderX, fY, 0.0f );
		pobSprite->SetPosition( obPos );
		pobSprite->SetWidth( (float)pobSprite->GetTextureWidth() );
		pobSprite->SetHeight( (float)pobSprite->GetTextureHeight() );

		m_apobSprite[ GC_SPRITE_SLIDER_UP_ARROW ] = pobSprite;

		// Create the top arrow highlight.
		pobSprite = NT_NEW ScreenSprite;
		if ( pobSprite )
		{
			pobSprite->SetTexture( "gui/frontend/textures/common/scroll_bar/ow_up_arrow_highlight_colour_alpha_nomip.dds" );
			pobSprite->SetPosition( obPos );
			pobSprite->SetWidth( (float)pobSprite->GetTextureWidth() );
			pobSprite->SetHeight( (float)pobSprite->GetTextureHeight() );
			
			m_apobSprite[ GC_SPRITE_SLIDER_UP_ARROW_HIGHLIGHT ] = pobSprite;
		}
	}

	// Create the bottom arrow.
	pobSprite = NT_NEW ScreenSprite;
	if ( pobSprite )
	{
		pobSprite->SetTexture( "gui/frontend/textures/common/scroll_bar/ow_down_arrow_colour_alpha_nomip.dds" );

		float fY = obPosSliderBarBottom.Y() + ( uiSliderBarBottomHalfHeight + ((float)pobSprite->GetTextureHeight() * 0.3f) );
		CPoint obPos( m_fSliderX, fY, 0.0f );
		pobSprite->SetPosition( obPos );
		pobSprite->SetWidth( (float)pobSprite->GetTextureWidth() );
		pobSprite->SetHeight( (float)pobSprite->GetTextureHeight() );

		m_apobSprite[ GC_SPRITE_SLIDER_DOWN_ARROW ] = pobSprite;

		// Create the bottom arrow highlight.
		pobSprite = NT_NEW ScreenSprite;
		if ( pobSprite )
		{
			pobSprite->SetTexture( "gui/frontend/textures/common/scroll_bar/ow_down_arrow_highlight_colour_alpha_nomip.dds" );
			pobSprite->SetPosition( obPos );
			pobSprite->SetWidth( (float)pobSprite->GetTextureWidth() );
			pobSprite->SetHeight( (float)pobSprite->GetTextureHeight() );

			m_apobSprite[ GC_SPRITE_SLIDER_DOWN_ARROW_HIGHLIGHT ] = pobSprite;
		}
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::UpdateSliderBarSprites
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::UpdateSliderBarSprites( void )
{
	if ( m_apobSprite[ GC_SPRITE_SLIDER ] )
	{
		if ( (m_iRowsToShow > 1) && (m_iNumRows > 1) )
		{
			float fSliderMovementRange = m_fSliderEndY - m_fSliderStartY;

			float fMovementVal = fSliderMovementRange * (m_iFirstVisibleRow/(float)(m_iNumRows-m_iRowsToShow));

			CPoint obPos( m_fSliderX, m_fSliderStartY + fMovementVal, 0.0f );
			m_apobSprite[ GC_SPRITE_SLIDER ]->SetPosition( obPos );
		}
	}

	// Update which arrows to render.
	if ( m_iFirstVisibleRow > 0 )
	{
		m_aiSliderArrowSpritesToRender[ IG_SLIDER_UP_ARROW ] = GC_SPRITE_SLIDER_UP_ARROW_HIGHLIGHT;
	}
	else
	{
		m_aiSliderArrowSpritesToRender[ IG_SLIDER_UP_ARROW ] = GC_SPRITE_SLIDER_UP_ARROW;
	}

	if ( m_iLastVisibleRow < m_iLastRow )
	{
		m_aiSliderArrowSpritesToRender[ IG_SLIDER_DOWN_ARROW ] = GC_SPRITE_SLIDER_DOWN_ARROW_HIGHLIGHT;
	}
	else
	{
		m_aiSliderArrowSpritesToRender[ IG_SLIDER_DOWN_ARROW ] = GC_SPRITE_SLIDER_DOWN_ARROW;
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::IncludeCellInGrid
//!
//! Determines if the cell should be included in the grid.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::IncludeCellInGrid( CGuiSkinMenuImageGridCell* pobGridCell )
{
	if ( m_apcString[ IG_STRING_FUNCTION_INCLUDE_CELL ] != NULL )
	{
		// Find the function.
		NinjaLua::LuaFunction luaFunc = CGuiLua::GetLuaFunction( m_apcString[ IG_STRING_FUNCTION_INCLUDE_CELL ] );

		if ( !luaFunc.IsNil() )
		{
			// Convert into bool returning function.
			NinjaLua::LuaFunctionRet<bool> luaFuncRet( luaFunc );

			int iChapter = pobGridCell->GetUserInt( 0 );	
			int iCheckpoint = pobGridCell->GetUserInt( 1 );	

			return luaFuncRet( iChapter, iCheckpoint );
		}
		else
		{
			ntPrintf("CGuiSkinMenuImageGrid::IncludeCellInGrid: Failed to find lua function \"%s\"\n", m_apcString[ IG_STRING_FUNCTION_INCLUDE_CELL ] );
		}
	}

	// Assume the cell is to be added.
	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::Update
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::Update( void )
{
	// Logic to handle movie that plays to the end and is not interupted by the player.
	if ( m_bCheckMovieIsPlaying )
	{
		MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_MovieID );
		
		// movie will be null when not playing.
		if ( NULL == movie )
		{
			// Movie must have finished so do post movie processing.
			PostMovie();
		}
	}

	return super::Update();
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::Render
//!
//! Render all grid sprites.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::Render( void )
{
	if ( m_bRender )
	{
		//Render grid components.		
		RenderGridCells();
		RenderCursorSprite();
		RenderSliderSprites();
	}

	RenderFullscreenBorderSprites();
	RenderFullscreenSprite();

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::EnableRender
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::EnableRender( const bool bEnable )
{
	m_bRender = bEnable;
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::RenderGridCells()
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::RenderGridCells( void )
{
	for ( ntstd::List<CGuiSkinMenuImageGridCell*>::iterator obIt = m_obCells.begin(); obIt != m_obCells.end(); obIt++ )
	{
		if ( (*obIt)->OnScreen( m_iFirstVisibleRow, m_iLastVisibleRow ) )
		{
			(*obIt)->RenderMe();
		}
	}
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::RenderSliderSprites()
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::RenderSliderSprites( void )
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	for ( int iSprite = GC_SPRITE_SLIDER_BAR_TOP; iSprite <= GC_SPRITE_SLIDER; iSprite++ )
	{
		if ( m_apobSprite[ iSprite ] )
		{
			m_apobSprite[ iSprite ]->Render();
		}
	}

	m_apobSprite[ m_aiSliderArrowSpritesToRender[ IG_SLIDER_UP_ARROW ] ]->Render();
	m_apobSprite[ m_aiSliderArrowSpritesToRender[ IG_SLIDER_DOWN_ARROW ] ]->Render();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::RenderFullscreenBorderSprites()
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::RenderFullscreenBorderSprites( void )
{
	if ( m_apobSprite[ GC_SPRITE_FULLSCREEN ] )
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

		for ( int iSpriteIndex = CG_SPRITE_FULLSCREEN_BORDER_PART_TR; iSpriteIndex <= CG_SPRITE_FULLSCREEN_BORDER_PART_CENTRE; iSpriteIndex++ )
		{
			if ( m_apobSprite[ iSpriteIndex ] )
			{
				m_apobSprite[ iSpriteIndex ]->Render();
			}
		}

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	}
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::RenderCursorSprite()
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::RenderCursorSprite( void )
{
	if ( m_apobSprite[ GC_SPRITE_CURSOR ] )
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

		m_apobSprite[ GC_SPRITE_CURSOR ]->Render();

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	}
}

//-----------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::RenderFullscreenSprite()
//!
//-----------------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::RenderFullscreenSprite( void )
{
	if ( m_apobSprite[ GC_SPRITE_FULLSCREEN ] )
	{
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

		m_apobSprite[ GC_SPRITE_FULLSCREEN ]->Render();

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::BackAction
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::BackAction( int iPads )
{
	if ( DisplayingCellContent() )
	{
		if ( m_apobSprite[ GC_SPRITE_FULLSCREEN ] )
		{
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_BACK);
			HideFullscreenImage();
		}
		else if ( MoviePlayer::Get().GetMoviePtrFromID( m_MovieID ) )
		{
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_BACK);
			StopMovie();
		}
	
		return true;
	}
	
	return super::BackAction( iPads );
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::MoveLeftAction
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::MoveLeftAction( int iPads )
{
	UNUSED(iPads);

	HandleGridNavigation( CG_LEFT );

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::MoveRightAction
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::MoveRightAction( int iPads )
{
	UNUSED(iPads);

	HandleGridNavigation( CG_RIGHT );

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::MoveDownAction
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::MoveDownAction( int iPads )
{
	UNUSED(iPads);

	HandleGridNavigation( CG_DOWN );

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::MoveUpAction
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::MoveUpAction( int iPads )
{
	UNUSED(iPads);

	HandleGridNavigation( CG_UP );

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::SelectAction
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::SelectAction( int iPads )
{
	UNUSED(iPads);

	if ( DisplayingCellContent() )
	{
		// Treat as back action if displaying cell content.
		BackAction( iPads );
	}
	else
	{
		// Start display of sell content.
		if ( GetCell( m_iCurrentCell )->GetFullScreenImageName() )
		{
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);
			ShowFullscreenImage();
		}
		else if ( GetCell( m_iCurrentCell )->GetMovieName() )
		{
			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_SELECT);
			StartMovie();
		}
	}

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::ShowFullscreenImage
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::ShowFullscreenImage( void )
{
	if ( NULL == m_apobSprite[ GC_SPRITE_FULLSCREEN ] )
	{
		ScreenSprite* pobSprite = NT_NEW ScreenSprite;

		if ( pobSprite )
		{
			pobSprite->SetTexture( GetCell( m_iCurrentCell )->GetFullScreenImageName() );
			pobSprite->SetWidth( 512.0f );
			pobSprite->SetHeight( 512.0f );

			CPoint obPos;
			GetCentre( obPos );

			pobSprite->SetPosition( obPos );

			m_apobSprite[ GC_SPRITE_FULLSCREEN ] = pobSprite;

			CreateFullscreenBorderSprites();
		
			EnableRender( false );
			EnableScreenFilter( false );
			EnableNavBar( false );

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::HideFullscreenImage
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::HideFullscreenImage( void )
{
	if ( m_apobSprite[ GC_SPRITE_FULLSCREEN ] )
	{
		NT_DELETE( m_apobSprite[ GC_SPRITE_FULLSCREEN ] );
		m_apobSprite[ GC_SPRITE_FULLSCREEN ] = NULL;

		DeleteFullscreenBorderSprites();

		EnableRender( true );
		EnableScreenFilter( true );
		EnableNavBar( true );

		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::StartMovie
//!
//! Start movie playback.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::StartMovie( void )
{
	MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_MovieID );

	if ( NULL == movie )
	{
		// Create the movie
		movie = MovieInstance::Create( GetCell( m_iCurrentCell )->GetMovieName(), 0 );

		if ( NULL != movie )
		{
			movie->SetPosition( CPoint( 0.5f, 0.5f, 0.0f ) );

			const float max_height = 720.0f;
			const float movie_height = 720.0f;
			
			movie->SetSize( CDirection( 1.0f, float( movie_height ) / max_height, 0.0f ) );

			MoviePlayer::Get().AddMovie( movie );

			m_MovieID = MoviePlayer::Get().GetMovieIDFromPtr( movie );

			m_bCheckMovieIsPlaying = true;

			EnableRender( false );

			EnableNavBar( false );
			EnableScreenFilter( false );

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::StopMovie
//!
//! Stop movie playback.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::StopMovie( void )
{
	MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_MovieID );
	if ( NULL != movie )
	{
		if ( MoviePlayer::Get().IsPlaying( movie ) )
		{
			MoviePlayer::Get().RemoveMovie( movie );

			PostMovie();

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::PostMovie
//!
//! Perform any actions post movie.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::PostMovie( void )
{
	// Activate grid rendering.
	EnableRender( true );

	// Activate nav bar.
	EnableNavBar( true );

	EnableScreenFilter( true );

	// Stop movie playing check.
	m_bCheckMovieIsPlaying = false;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::EnableNavBar
//!
//! Enable/Disable the navbar.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::EnableNavBar( const bool bEnable )
{
	CGuiUnit* pobNavBar = GetParentScreen()->FindChildUnit( "navbar" );

	if ( pobNavBar )
	{
		pobNavBar->SetAttribute("allowrender", TRUTH_STRING( bEnable ) );
		pobNavBar->SetAttribute("showscriptoverlay", TRUTH_STRING( bEnable ) );
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::EnableScreenFilter
//!
//! Enable/Disable the screen filter object.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::EnableScreenFilter( const bool bEnable )
{
	GetParentScreen()->SetAttribute( "filter", TRUTH_STRING( bEnable ) );
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::DisplayingCellContent
//!
//!	Returns TRUE if the current cells content is being displayed, ie fullscreen
//! image or movie/
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::DisplayingCellContent( void )
{
	// check for full screen image.
	if ( m_apobSprite[ GC_SPRITE_FULLSCREEN ] )
	{
		return true;
	}

	// Check for movie playing.
	MovieInstancePtr movie = MoviePlayer::Get().GetMoviePtrFromID( m_MovieID );
	if ( MoviePlayer::Get().IsPlaying( movie ) )
	{
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::HandleGridNavigation
//!
//! Update the current cell, deal with grid movement and updating of all
//! screen components. Play movement sounds.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::HandleGridNavigation( const eCURSOR_DIRECTION eCursorDirection )
{
	// Disable navigation if we are viewing a cells content.
	if ( DisplayingCellContent() )
	{
		return;
	}

	// Calc next cell based on direction user selected.
	switch( eCursorDirection )
	{
	case CG_UP:
		if ( m_iCurrentCell >= m_iCellsPerRow )
		{
			m_iCurrentCell -= m_iCellsPerRow;

			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_UP);
		}
		break;

	case CG_RIGHT:
		if ( m_iCurrentCell < m_iLastCell )
		{
			m_iCurrentCell++;

			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_DOWN );
		}
		break;

	case CG_DOWN:
		if ( m_iCurrentCell < m_iLastCell )
		{
			m_iCurrentCell += m_iCellsPerRow;
		
			// It is possible that the last row might have less than
			// m_iCellsPerRow cells.  In this case we must check to
			// make sure m_iCurrentCell does not overrun.
			if ( m_iCurrentCell > m_iLastCell )
			{
				m_iCurrentCell = m_iLastCell;
			}

			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_DOWN );
		}
		break;

	case CG_LEFT:
		if ( m_iCurrentCell > 0 )
		{
			m_iCurrentCell--;

			CGuiSoundManager::Get().PlaySound(CGuiSound::ACTION_UP );
		}
		break;

	default:
		ntPrintf( "CGuiSkinMenuImageGrid::HandleGridNavigation() - Invalid movement direction.\n" );
		break;
	}

	// Calculate new row.
	int iRow = m_iCurrentCell/m_iCellsPerRow;

	// Calc required grid movement.
	float fYMovement = 0.0f;

	if ( iRow < m_iFirstVisibleRow )
	{
		// Going up...
		m_iFirstVisibleRow--;
		m_iLastVisibleRow--;

		fYMovement = (m_CellSize.m_fHeight + m_fCellSpacing);
	}
	else if	( (iRow > m_iLastVisibleRow) && (iRow <= m_iLastRow) )
	{
		// Going down...
		m_iFirstVisibleRow++;
		m_iLastVisibleRow++;

		fYMovement = -(m_CellSize.m_fHeight + m_fCellSpacing);
	}

	// Deal with any grid movement.
	if ( fYMovement != 0.0f )
	{
		for ( ntstd::List<CGuiSkinMenuImageGridCell*>::iterator obIt = m_obCells.begin(); obIt != m_obCells.end(); obIt++ )
		{
			(*obIt)->Move( 0.0f, fYMovement );
		}
	}

	UpdateCursorPosition();
	UpdateCellSpriteRenderStates();
	UpdateSliderBarSprites();
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::UpdateCellSpriteRenderStates
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::UpdateCellSpriteRenderStates( void )
{
	for ( ntstd::List<CGuiSkinMenuImageGridCell*>::iterator obIt = m_obCells.begin(); obIt != m_obCells.end(); obIt++ )
	{
		// Enable filter sprite render by default.
		(*obIt)->EnableFilterSpriteRender( true );

		// Disable filter render if this cell is under the cursor.
		if ( (*obIt)->GetIndex() == m_iCurrentCell )
		{
			(*obIt)->EnableFilterSpriteRender( false );
		}
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::UpdateCursorPosition
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::UpdateCursorPosition( void )
{
	// Position the cursor.
	CGuiSkinMenuImageGridCell* pobCell = GetCell( m_iCurrentCell );
	if ( pobCell )
	{
		CPoint obCentre;
		pobCell->GetCentre( obCentre );
		m_apobSprite[ GC_SPRITE_CURSOR ]->SetPosition( obCentre );
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::GetCell
//!
//! Get pointer to the specified grid cell.
//!
//------------------------------------------------------------------------------
CGuiSkinMenuImageGridCell* CGuiSkinMenuImageGrid::GetCell( const int iIndex )
{
	for ( ntstd::List<CGuiSkinMenuImageGridCell*>::iterator obIt = m_obCells.begin(); obIt != m_obCells.end(); obIt++ )
	{
		if ( (*obIt)->GetIndex() == iIndex )
		{
			return (*obIt);
		}
	}

	return NULL;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::ProcessAttribute
//!
//! Process all the attributes.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "cellsize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_CellSize.m_fWidth, &m_CellSize.m_fHeight );
		}
		else if ( strcmp( pcTitle, "cellspacing" ) == 0 )
		{
			return GuiUtil::SetFloat( pcValue, &m_fCellSpacing );
		}
		else if ( strcmp( pcTitle, "cellsperrow" ) == 0 )
		{
			bool bRetVal = GuiUtil::SetInt( pcValue, &m_iCellsPerRow );
			ntAssert_p( m_iCellsPerRow > 0, ("CGuiSkinMenuImageGrid::ProcessAttribute() - m_iCellsPerRow should be > 0.\n") );
			return bRetVal;
		}
		else if ( strcmp( pcTitle, "rowstoshow" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iRowsToShow );
		}
		else if ( strcmp( pcTitle, "cursortexture" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcString[ IG_STRING_TEXTURE_NAME_CURSOR ] );
		}
		else if ( strcmp( pcTitle, "cursorsize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_CursorSize.m_fWidth, &m_CursorSize.m_fHeight );
		}
		else if ( strcmp( pcTitle, "includecellingridcheck" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcString[ IG_STRING_FUNCTION_INCLUDE_CELL ] );
		}
		else if ( strcmp( pcTitle, "filterimage" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcString[IG_STRING_TEXTURE_NAME_FILTER_IMAGE ] );
		}
		else if ( strcmp( pcTitle, "filtersize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_FilterSize.m_fWidth, &m_FilterSize.m_fHeight );
		}
		else if ( strcmp( pcTitle, "borderimage" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcString[IG_STRING_TEXTURE_NAME_BORDER_IMAGE ] );
		}
		else if ( strcmp( pcTitle, "bordersize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_BorderSize.m_fWidth, &m_BorderSize.m_fHeight );
		}
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::ProcessChild
//!
//! Process all child elements.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::ProcessChild( CXMLElement* pobChild )
{
	ntAssert ( pobChild );

	m_obCellsAll.push_back( (CGuiSkinMenuImageGridCell*)pobChild );

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::ProcessEnd
//!
//! End of attribute processing.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGrid::ProcessEnd( void )
{
	// Call the base first
	super::ProcessEnd();

	Initialise();

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::CalculateExtents
//!
//! Calc left,top,right,bottom extents for the grid.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::CalculateExtents( void )
{
	float fWidth = m_CellSize.m_fWidth;
	float fHeight = m_CellSize.m_fHeight;

	if ( m_iCellsPerRow > 1 )
	{
		fWidth = (m_iCellsPerRow*m_CellSize.m_fWidth)+((m_iCellsPerRow-1)*m_fCellSpacing);
	}

	if ( m_iRowsToShow > 1 )
	{
		fHeight = (m_iRowsToShow*m_CellSize.m_fHeight)+(m_iRowsToShow-1)*m_fCellSpacing;
	}

	m_obLeftTop = CPoint( m_BasePosition.X()-(fWidth*0.5f), m_BasePosition.Y()-(fHeight*0.5f), 0.0f );
	m_obRightBottom = CPoint( m_obLeftTop.X() + fWidth, m_obLeftTop.Y() + fHeight, 0.0f );
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::GetCentre
//!
//! Get centre of the entire grid.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGrid::GetCentre( CPoint& obCentre )
{
	obCentre = CPoint( (m_obLeftTop + m_obRightBottom) * CPoint( 0.5f, 0.5f, 1.0f ) );
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::CalculateNumberOfRows
//!
//! Calculate the number of rows in the grid.
//!
//------------------------------------------------------------------------------
int CGuiSkinMenuImageGrid::CalculateNumberOfRows( const int iNumCells, const int iCellsPerRow ) const
{
	int iRows = 0;
		
	if ( iCellsPerRow > 0 )
	{
		// Calculate complete rows.
		iRows = iNumCells/iCellsPerRow;

		// Detect incomplete row.
		if ( iNumCells%iCellsPerRow )
		{
			iRows++;
		}
	}

	return iRows;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGrid::GetCurrentRow
//!
//! Get row for current cell.
//!
//------------------------------------------------------------------------------
int CGuiSkinMenuImageGrid::GetCurrentRow( void )
{
	int iRow = 0;

	if ( m_iCellsPerRow > 0 )
	{
		// Calculate complete rows.
		iRow = m_iCurrentCell/m_iCellsPerRow;

		// Detect incomplete row.
		if ( m_iCurrentCell%m_iCellsPerRow )
		{
			iRow++;
		}
	}

	return iRow;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::CGuiSkinMenuImageGridCell
//!
//! Constructor for the CGuiSkinMenuImageGridCell class.
//!
//------------------------------------------------------------------------------
CGuiSkinMenuImageGridCell::CGuiSkinMenuImageGridCell() :
	m_iIndex( -1 ),
	m_iRow( 0 ),
	m_iState( CGuiSkinMenuImageGridCell::GC_NONE ),

	m_fLeft( 0.0f ),
	m_fTop( 0.0f ),

	m_bRenderFilterSprite( true ),
	m_obCentre( CPoint(0.0f,0.0f,0.0f) ),
	m_pobString( NULL ),
	m_pobBaseTransform( NULL )
{
	NullSpritePointers();
	NullStringArrayPointers();
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::NullSpritePointers
//!
//! Null all the sprite pointers.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::NullSpritePointers( void )
{
	for ( int iLoop = 0; iLoop < GC_NUM_SPRITES; iLoop++ )
	{
		m_apobSprite[ iLoop ] = NULL;
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::NullStringArrayPointers
//!
//! Null the string array pointers.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::NullStringArrayPointers( void )
{
	for ( int iLoop = 0; iLoop < GC_NUM_STRINGS; iLoop++ )
	{
		m_apcStrings[ iLoop ] = NULL;
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::~CGuiSkinMenuImageGridCell
//!
//! Destructor for the CGuiSkinMenuImageGridCell class.
//!
//------------------------------------------------------------------------------
CGuiSkinMenuImageGridCell::~CGuiSkinMenuImageGridCell()
{
	DeleteSprites();
	DeleteStringArrays();
	DestroyString();
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::DeleteSprites
//!
//! Delete all the sprites.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::DeleteSprites( void )
{
	for ( int iLoop = 0; iLoop < GC_NUM_SPRITES; iLoop++ )
	{
		if ( m_apobSprite[ iLoop ] )
		{
			NT_DELETE( m_apobSprite[ iLoop ] );
			m_apobSprite[ iLoop ] = NULL;
		}
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::DeleteStringArrays
//!
//! Delete all the string arrays.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::DeleteStringArrays( void )
{
	for ( int iLoop = 0; iLoop < GC_NUM_STRINGS; iLoop++ )
	{
		if ( m_apcStrings[ iLoop ] )
		{
			NT_DELETE_ARRAY( m_apcStrings[ iLoop ] );
			m_apcStrings[ iLoop ] = NULL;
		}
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::Initialise
//!
//! Initialise and position cell sprites.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::Initialise( const CellInit& obCellInit )
{
	ScreenSprite* pobSprite = NULL;

	m_iIndex = obCellInit.m_iIndex;
	m_iRow = obCellInit.m_iRow;

	// Calculate centre position for the sprite.
	m_obCentre = CPoint( obCellInit.m_fLeft + (obCellInit.m_fWidth * 0.5f), obCellInit.m_fTop + (obCellInit.m_fHeight * 0.5f), 0.0f );

	// Initialise image thumbnail sprite.
	pobSprite = CreateSpriteImage();
	if ( pobSprite )
	{
		pobSprite->SetTexture( m_apcStrings[ GC_STRING_IMAGE_TEXTURE_NAME ] );
		pobSprite->SetWidth( obCellInit.m_fWidth );
		pobSprite->SetHeight( obCellInit.m_fHeight );
		pobSprite->SetPosition( m_obCentre );

		SetUVHelper( pobSprite, m_aUV[ GC_UV_IMAGE ], pobSprite->GetTextureWidth(), pobSprite->GetTextureHeight() );
	}

	// Initialise the border sprite
	pobSprite = CreateSpriteBorder();
	if ( pobSprite )
	{
		// Border texture can be set on a cell by cell basis if required.
		if ( m_apcStrings[ GC_STRING_BORDER_TEXTURE_NAME ] )
		{
			pobSprite->SetTexture( m_apcStrings[ GC_STRING_BORDER_TEXTURE_NAME ] );
			pobSprite->SetWidth( m_aSize[ GC_SPRITE_SIZE_BORDER ].m_fWidth );
			pobSprite->SetHeight( m_aSize[ GC_SPRITE_SIZE_BORDER ].m_fHeight );
		}
		else if ( obCellInit.m_pcTextureNameBorderImage )
		{
			pobSprite->SetTexture( obCellInit.m_pcTextureNameBorderImage );
			pobSprite->SetWidth( obCellInit.m_BorderSize.m_fWidth );
			pobSprite->SetHeight( obCellInit.m_BorderSize.m_fHeight );
		}

		pobSprite->SetPosition( m_obCentre );
	}

	// Initialise filter sprite.
	pobSprite = CreateSpriteFilter();
	if ( pobSprite )
	{	
		// Filter texture can be set on a cell by cell basis if required.
		if ( m_apcStrings[ GC_STRING_FILTER_TEXTURE_NAME ] )
		{
			pobSprite->SetTexture( m_apcStrings[ GC_STRING_FILTER_TEXTURE_NAME ] );
			pobSprite->SetWidth( m_aSize[ GC_SPRITE_SIZE_FILTER ].m_fWidth );
			pobSprite->SetHeight( m_aSize[ GC_SPRITE_SIZE_FILTER ].m_fHeight );
		}
		else if ( obCellInit.m_pcTextureNameFilterImage )
		{
			pobSprite->SetTexture( obCellInit.m_pcTextureNameFilterImage );
			pobSprite->SetWidth( obCellInit.m_FilterSize.m_fWidth );
			pobSprite->SetHeight( obCellInit.m_FilterSize.m_fHeight );
		}

		pobSprite->SetPosition( m_obCentre );

		SetUVHelper( pobSprite, m_aUV[ GC_UV_FILTER ], pobSprite->GetTextureWidth(), pobSprite->GetTextureHeight() );
	}

	CreateString();
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::CreateSpriteImage
//!
//! Create sprite used to display the cell image.
//!
//------------------------------------------------------------------------------
ScreenSprite* CGuiSkinMenuImageGridCell::CreateSpriteImage( void )
{
	if ( m_apobSprite[ GC_SPRITE_IMAGE ] )
	{
		NT_DELETE( m_apobSprite[ GC_SPRITE_IMAGE ] );
	}

	m_apobSprite[ GC_SPRITE_IMAGE ] = NT_NEW ScreenSprite;

	return m_apobSprite[ GC_SPRITE_IMAGE ];
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::CreateSpriteFilter
//!
//! Create sprite used to display the cell image filter.
//!
//------------------------------------------------------------------------------
ScreenSprite* CGuiSkinMenuImageGridCell::CreateSpriteFilter( void )
{
	if ( m_apobSprite[ GC_SPRITE_FILTER ] )
	{
		NT_DELETE( m_apobSprite[ GC_SPRITE_FILTER ] );
	}

	m_apobSprite[ GC_SPRITE_FILTER ] = NT_NEW ScreenSprite;

	return m_apobSprite[ GC_SPRITE_FILTER ];
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::CreateSpriteBorder
//!
//! Create sprite used to display the border round each image.
//!
//------------------------------------------------------------------------------
ScreenSprite* CGuiSkinMenuImageGridCell::CreateSpriteBorder( void )
{
	if ( m_apobSprite[ GC_SPRITE_BORDER ] )
	{
		NT_DELETE( m_apobSprite[ GC_SPRITE_BORDER ] );
	}

	m_apobSprite[ GC_SPRITE_BORDER ] = NT_NEW ScreenSprite;

	return m_apobSprite[ GC_SPRITE_BORDER ];
}

//-----------------------------------------------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::SetUVHelper
//!
//!	Set texture UV for a screen sprite.  Expects fLeft, fTop, fRight, fBottom to be in pixels not 0..1.
//!
//-----------------------------------------------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::SetUVHelper( ScreenSprite* pobSprite, const UVExtents& TextureUV,
											const uint32_t uiTextureWidth, const uint32_t uiTextureHeight )
{
	if ( pobSprite )
	{
		float fNormalisedLeft = 0.0f;
		float fNormalisedTop = 0.0f;

		float fNormalisedRight = 1.0f;
		float fNormalisedBottom = 1.0f;

		if ( fabsf( TextureUV.fLeft - TextureUV.fRight ) )
		{
			fNormalisedLeft = TextureUV.fLeft/(float)uiTextureWidth;
			fNormalisedTop = TextureUV.fTop/(float)uiTextureHeight;
		}

		if ( fabsf( TextureUV.fTop - TextureUV.fBottom ) )
		{
			fNormalisedRight = TextureUV.fRight/(float)uiTextureWidth;
			fNormalisedBottom = TextureUV.fBottom/(float)uiTextureHeight;
		}

		pobSprite->SetUV( fNormalisedTop, fNormalisedLeft, fNormalisedBottom, fNormalisedRight );
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::Render
//!
//! Render the cell.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGridCell::RenderMe( void )
{
	Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );

	RenderSprites();
	RenderString();

	Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::RenderSprites
//!
//! Render all the sprites.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::RenderSprites( void )
{
	for ( int iLoop = 0; iLoop < GC_NUM_SPRITES; iLoop++ )
	{
		if ( m_apobSprite[ iLoop ] )
		{
			if ( (GC_SPRITE_FILTER == iLoop) &&  (false == m_bRenderFilterSprite) )
			{
				continue;
			}
			
			m_apobSprite[ iLoop ]->Render();
		}
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::SetIndex
//!
//! Set cell index.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::SetIndex( const int iIndex )
{
	m_iIndex = iIndex;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::GetIndex
//!
//! Get cell index.
//!
//------------------------------------------------------------------------------
const int CGuiSkinMenuImageGridCell::GetIndex( void ) const
{
	return m_iIndex;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::SetRow
//!
//! Set row the cell is on.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::SetRow( const int iRow )
{
	m_iRow = iRow;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::GetRow
//!
//! Get row the cell is on.
//!
//------------------------------------------------------------------------------
const int CGuiSkinMenuImageGridCell::GetRow( void ) const
{
	return m_iRow;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::GetCentre
//!
//! Get centre position of the cell.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::GetCentre( CPoint& obPos )
{
	obPos = m_obCentre;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::Move
//!
//! Move the cell by the specified amount.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::Move( const float fXMovement, const float fYMovement )
{
	// Calculate new centre position.
	m_obCentre = CPoint( m_obCentre.X() + fXMovement, m_obCentre.Y() + fYMovement, 0.0f );

	// Move the sprites.
	for ( int iLoop = 0; iLoop < GC_NUM_SPRITES; iLoop++ )
	{
		if ( m_apobSprite[ iLoop ] )
		{
			m_apobSprite[ iLoop ]->SetPosition( m_obCentre );
		}
	}

	// Move the string object.
	if ( m_pobString )
	{
		m_BasePosition = m_obCentre;
		CMatrix obLocalMatrix( m_pobBaseTransform->GetLocalMatrix() );
		obLocalMatrix.SetTranslation( m_BasePosition );
		m_pobBaseTransform->SetLocalMatrix( obLocalMatrix );
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::OnScreen
//!
//! Is the cell on screen.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGridCell::OnScreen( const int iFirstVisibleRow, const int iLastVisibleRow )
{
	if ( m_iRow < iFirstVisibleRow )
	{
		return false;
	}

	if ( m_iRow > iLastVisibleRow )
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::EnableFilterSpriteRender
//!
//! Activate/Deactivate rendering of the filter sprite.
//!
//------------------------------------------------------------------------------
void CGuiSkinMenuImageGridCell::EnableFilterSpriteRender( const bool bEnable )
{
	m_bRenderFilterSprite = bEnable;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::ProcessAttribute
//!
//! Process cell attributes during construction.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGridCell::ProcessAttribute( const char* pcTitle, const char* pcValue )
{
	if ( !super::ProcessAttribute( pcTitle, pcValue ) )
	{
		if ( strcmp( pcTitle, "userint1" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iUserInt[ 0 ] );
		}
		else if ( strcmp( pcTitle, "userint2" ) == 0 )
		{
			return GuiUtil::SetInt( pcValue, &m_iUserInt[ 1 ] );
		}
		else if ( strcmp( pcTitle, "image" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcStrings[ GC_STRING_IMAGE_TEXTURE_NAME ] );
		}
		else if ( strcmp( pcTitle, "imageuv" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_aUV[ GC_UV_IMAGE ].fLeft,&m_aUV[ GC_UV_IMAGE ].fTop,
				&m_aUV[ GC_UV_IMAGE ].fRight,&m_aUV[ GC_UV_IMAGE ].fBottom );
		}
		else if ( strcmp( pcTitle, "fullscreenimage" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcStrings[ GC_STRING_FULLSCREEN_TEXTURE_NAME ] );
		}
		else if ( strcmp( pcTitle, "filterimage" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcStrings[ GC_STRING_FILTER_TEXTURE_NAME ] );
		}
		else if ( strcmp( pcTitle, "filterimageuv" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_aUV[ GC_UV_FILTER ].fLeft,&m_aUV[ GC_UV_FILTER ].fTop,
				&m_aUV[ GC_UV_FILTER ].fRight,&m_aUV[ GC_UV_FILTER ].fBottom );
		}
		else if ( strcmp( pcTitle, "filtersize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_aSize[ GC_SPRITE_SIZE_FILTER ].m_fWidth, &m_aSize[ GC_SPRITE_SIZE_FILTER ].m_fHeight );
		}
		else if ( strcmp( pcTitle, "borderimage" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcStrings[ GC_STRING_BORDER_TEXTURE_NAME ] );
		}
		else if ( strcmp( pcTitle, "bordersize" ) == 0 )
		{
			return GuiUtil::SetFloats( pcValue, &m_aSize[ GC_SPRITE_SIZE_BORDER ].m_fWidth, &m_aSize[ GC_SPRITE_SIZE_BORDER ].m_fHeight );
		}
		else if ( strcmp( pcTitle, "text" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcStrings[ GC_STRING_CELL_NAME ] );
		}
		else if	( strcmp( pcTitle, "moviename" ) == 0 )
		{
			return GuiUtil::SetString( pcValue, &m_apcStrings[ GC_MOVIE_NAME ] );
		}
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::ProcessEnd
//!
//! Called when all attributes have been processed.
//!
//------------------------------------------------------------------------------
bool CGuiSkinMenuImageGridCell::ProcessEnd( void )
{
	// Call the base first
	super::ProcessEnd();

	return true;
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::GetFullScreenImageName
//!
//! Get the name of the full screen image associated with this cell.
//!
//------------------------------------------------------------------------------
const char* CGuiSkinMenuImageGridCell::GetFullScreenImageName( void )
{
	return m_apcStrings[ GC_STRING_FULLSCREEN_TEXTURE_NAME ];
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::GetMovieName
//!
//! Get the name of the movie associated with this cell.
//!
//------------------------------------------------------------------------------
const char* CGuiSkinMenuImageGridCell::GetMovieName( void )
{
	return m_apcStrings[ GC_MOVIE_NAME ];
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::GetUserInt
//!
//! Get user data associated with this cell.
//!
//------------------------------------------------------------------------------
const int CGuiSkinMenuImageGridCell::GetUserInt( const int iIndex )
{
	if ( iIndex >= 0 && iIndex < MAX_USER_INTS )
	{
		return m_iUserInt[ iIndex ];
	}

	return 0;
}

void CGuiSkinMenuImageGridCell::CreateString( void )
{
	if ( NULL == m_apcStrings[ GC_STRING_CELL_NAME ] )
	{
		return;
	}

	// Find out some info about the font to be used.
	CStringDefinition obStrDef;
	obStrDef.m_pobFont = CFontManager::Get().GetFont( CGuiManager::Get().GuiSettings()->BodyFont() );
	obStrDef.m_fOverallScale = 0.5f;

	// Create a matrix with the point
	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	obBaseMatrix.SetTranslation( m_obCentre );

	// Now set our transform object out of all that
	m_pobBaseTransform = NT_NEW Transform();
	m_pobBaseTransform->SetLocalMatrix( obBaseMatrix );

	CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobBaseTransform );

	m_pobString = CStringManager::Get().MakeString( m_apcStrings[ GC_STRING_CELL_NAME ], obStrDef, m_pobBaseTransform, CGuiUnit::RENDER_SCREENSPACE );

	CVector obColour( 1.0f, 1.0f, 1.0f, 1.0f );
	m_pobString->SetColour( obColour );
}

void CGuiSkinMenuImageGridCell::DestroyString( void )
{
	if	( m_pobString )
	{
		CStringManager::Get().DestroyString( m_pobString );
	}

	// Deal with the base transform
	if ( m_pobBaseTransform )
	{
		m_pobBaseTransform->RemoveFromParent();
		NT_DELETE( m_pobBaseTransform );
	}
}

void CGuiSkinMenuImageGridCell::RenderString( void )
{
	if ( m_pobString )
	{
		m_pobString->Render();
	}
}

//------------------------------------------------------------------------------
//!
//!	CGuiSkinMenuImageGridCell::_UVExtents
//!
//! Constructor for the uv extents struct.
//!
//------------------------------------------------------------------------------
CGuiSkinMenuImageGridCell::_UVExtents::_UVExtents() :
	fLeft( 0.0f ),
	fTop( 0.0f ),
	fRight( 0.0f ),
	fBottom( 0.0f )
{
}

//------------------------------------------------------------------------------
//!
//!	::_Size
//!
//! Constructor _Size struct.
//!
//------------------------------------------------------------------------------
_Size::_Size( void ) :
	m_fWidth( 0.0f ),
	m_fHeight( 0.0f )
{
}

//------------------------------------------------------------------------------
//!
//!	::_Size
//!
//! Constructor _Size struct.
//!
//------------------------------------------------------------------------------
_Size::_Size( const float fWidth, const float fHeight ) :
	m_fWidth( fWidth ),
	m_fHeight( fHeight )
{
}
