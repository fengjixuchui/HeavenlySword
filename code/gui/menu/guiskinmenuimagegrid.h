//-------------------------------------------------------------------------------------------------
//!
//!	\file	/game/guiskinmenuimagegrid.h
//!
//!	Definition of class to manage a grid of selectable images.
//!
//-------------------------------------------------------------------------------------------------
#ifndef _GUISKINMENUIMAGEGRID_H
#define _GUISKINMENUIMAGEGRID_H

// Includes
#include "gui/guitext.h"
#include "gui/guiunit.h"
#include "core/nt_std.h"
#include "movies/movieplayer.h"

// Forward declarations
class CGuiSkinMenuImageGridCell;

typedef struct _Size
{
	float m_fWidth;
	float m_fHeight;

	_Size();
	_Size( const float fWidth, const float fHeight );
} Size;

//------------------------------------------------------------------------------------------------
//!
//! CGuiSkinMenuImageGrid::CGuiSkinMenuImageGrid
//!
//------------------------------------------------------------------------------------------------
class CGuiSkinMenuImageGrid : public CGuiUnit
{
	typedef CGuiUnit super;
public:

	CGuiSkinMenuImageGrid();
	~CGuiSkinMenuImageGrid();

protected:

	virtual bool Render( void );

	virtual bool MoveLeftAction( int iPads );
	virtual bool MoveRightAction( int iPads );
	virtual bool MoveDownAction( int iPads );
	virtual bool MoveUpAction( int iPads );
	virtual bool BackAction( int iPads );
	virtual bool SelectAction( int iPads );
	virtual bool Update( void );

	virtual bool ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool ProcessChild( CXMLElement* pobChild );
	virtual bool ProcessEnd( void );

private:

	enum eCURSOR_DIRECTION
	{
		CG_UP = 0,
		CG_RIGHT,
		CG_DOWN,
		CG_LEFT
	};

	void RenderGridCells( void );
	void RenderSliderSprites( void );
	void RenderFullscreenBorderSprites( void );
	void RenderCursorSprite( void );
	void RenderFullscreenSprite( void );

	void EnableRender( const bool bEnable );
	void EnableNavBar( const bool bEnable );
	void EnableScreenFilter( const bool bEnable );

	void CreateCursorSprite( void );
	void CreateSliderBarSprites( void );
	void CreateFullscreenBorderSprites( void );

	void DeleteFullscreenBorderSprites( void );
	void DeleteSprites( void );
	void DeleteStringObjects( void );

	void UpdateCellSpriteRenderStates( void );
	void UpdateSliderBarSprites( void );
	void UpdateCursorPosition( void );

	void NullSpritePointers( void );
	void NullStringPointers( void );

	void GetCentre( CPoint& obCentre );
	int GetCurrentRow( void );
	CGuiSkinMenuImageGridCell* GetCell( const int iIndex );

	int CalculateNumberOfRows( const int iNumCells, const int iCellsPerRow ) const;
	void CalculateExtents( void );

	bool IncludeCellInGrid( CGuiSkinMenuImageGridCell* pobGridCell );
	void HandleGridNavigation( const eCURSOR_DIRECTION eCursorDirection );
	void Initialise( void );
	bool StartMovie( void );
	bool StopMovie( void );
	bool HideFullscreenImage( void );
	bool ShowFullscreenImage( void );
	bool DisplayingCellContent( void );
	void PostMovie( void );

	CPoint m_obLeftTop;
	CPoint m_obRightBottom;

	float m_fCellSpacing;
	float m_fSliderStartY;
	float m_fSliderEndY;
	float m_fSliderX;

	int m_iCellsPerRow;
	int m_iCurrentCell;
	int m_iNumCells;
	int m_iRowsToShow;
	int m_iFirstVisibleRow;
	int m_iLastVisibleRow;
	int m_iNumRows;
	int m_iLastRow;
	int m_iLastCell;

	Size m_CellSize;
	Size m_CursorSize;
	Size m_FilterSize;
	Size m_BorderSize;

	enum
	{
		IG_STRING_TEXTURE_NAME_CURSOR = 0,
		IG_STRING_FUNCTION_INCLUDE_CELL,
		IG_STRING_TEXTURE_NAME_FILTER_IMAGE,
		IG_STRING_TEXTURE_NAME_BORDER_IMAGE,
		IG_NUM_STRINGS
	};

	const char* m_apcString[ IG_NUM_STRINGS ];

	enum
	{
		GC_SPRITE_CURSOR = 0,

		CG_SPRITE_FULLSCREEN_BORDER_PART_TR,
		CG_SPRITE_FULLSCREEN_BORDER_PART_R,
		CG_SPRITE_FULLSCREEN_BORDER_PART_BR,
		CG_SPRITE_FULLSCREEN_BORDER_PART_B,
		CG_SPRITE_FULLSCREEN_BORDER_PART_BL,
		CG_SPRITE_FULLSCREEN_BORDER_PART_L,
		CG_SPRITE_FULLSCREEN_BORDER_PART_TL,
		CG_SPRITE_FULLSCREEN_BORDER_PART_T,
		CG_SPRITE_FULLSCREEN_BORDER_PART_CENTRE,

		GC_SPRITE_FULLSCREEN,

		GC_SPRITE_SLIDER_BAR_TOP,
		GC_SPRITE_SLIDER_BAR_MIDDLE,
		GC_SPRITE_SLIDER_BAR_BOTTOM,
		GC_SPRITE_SLIDER,
		GC_SPRITE_SLIDER_UP_ARROW,
		GC_SPRITE_SLIDER_UP_ARROW_HIGHLIGHT,
		GC_SPRITE_SLIDER_DOWN_ARROW,
		GC_SPRITE_SLIDER_DOWN_ARROW_HIGHLIGHT,

		GC_NUM_SPRITE
	};

	ScreenSprite* m_apobSprite[ GC_NUM_SPRITE ];

	ntstd::List<CGuiSkinMenuImageGridCell*> m_obCellsAll;
	ntstd::List<CGuiSkinMenuImageGridCell*> m_obCells;

	MovieID m_MovieID;

	bool m_bRender;
	bool m_bCheckMovieIsPlaying;

	enum
	{
		IG_SLIDER_UP_ARROW = 0,
		IG_SLIDER_DOWN_ARROW,
		IG_NUM_SLIDER_ARROWS = 2
	};

	int m_aiSliderArrowSpritesToRender[ IG_NUM_SLIDER_ARROWS ];
};

class CGuiSkinMenuImageGridCell : public CXMLElement
{
	typedef CXMLElement super;

public:

	typedef struct _UVExtents
	{
		float fLeft;
		float fTop;
		float fRight;
		float fBottom;

		_UVExtents();

	} UVExtents;

	struct CellInit
	{
		int	m_iIndex;
		int m_iRow;
		float m_fLeft;
		float m_fTop;
		float m_fWidth;
		float m_fHeight;
		const char* m_pcTextureNameFilterImage;
		const char* m_pcTextureNameBorderImage;
		Size m_FilterSize;
		Size m_BorderSize;

		CellInit();
	};

	enum eState
	{
		GC_NONE = 0
	};

	enum { MAX_USER_INTS = 2 };

	CGuiSkinMenuImageGridCell();
	~CGuiSkinMenuImageGridCell();

	void Initialise( const CellInit& obInit );

	void SetState( const int iState );
	void SetIndex( const int iIndex );
	void SetRow( const int iRow );

	const int GetIndex( void ) const;
	const int GetRow( void ) const;
	void GetCentre( CPoint& obPos );

	void Move( const float fXMovement, const float fYMovement );
	bool OnScreen( const int iFirstVisibleRow, const int iLastVisibleRow );

	void SetUVHelper( ScreenSprite* pobSprite, const UVExtents& TextureUV,
		const uint32_t uiTextureWidth, const uint32_t uiTextureHeight );

	void EnableFilterSpriteRender( const bool bEnable );

	const char* GetFullScreenImageName( void );
	const char* GetMovieName( void );
	const int GetUserInt( const int iIntIndex );

	bool RenderMe( void );

protected:

	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );


private:

	// Functions
	ScreenSprite* CreateSpriteImage( void );
	ScreenSprite* CreateSpriteFilter( void );
	ScreenSprite* CreateSpriteBorder( void );

	void NullSpritePointers( void );
	void DeleteSprites( void );
	void RenderSprites( void );

	void NullStringArrayPointers( void );
	void DeleteStringArrays( void );

	void CreateString( void );
	void DestroyString( void );
	void RenderString( void );
	//

	int m_iUserInt[ MAX_USER_INTS ];
	int m_iIndex;
	int m_iRow;
	int m_iState;

	float m_fLeft;
	float m_fTop;

	bool m_bRenderFilterSprite;

	CPoint m_obCentre;

	// Sprites
	enum
	{
		GC_SPRITE_BORDER = 0,
		GC_SPRITE_IMAGE,
		GC_SPRITE_FILTER,
		GC_NUM_SPRITES
	};

	ScreenSprite* m_apobSprite[ GC_NUM_SPRITES ];
	//

	// Strings
	enum
	{
		GC_STRING_IMAGE_TEXTURE_NAME = 0,
		GC_STRING_FILTER_TEXTURE_NAME,
		GC_STRING_FULLSCREEN_TEXTURE_NAME,
		GC_STRING_BORDER_TEXTURE_NAME,
		GC_STRING_CELL_NAME,
		GC_MOVIE_NAME,
		GC_NUM_STRINGS
	};
	
	const char* m_apcStrings[ GC_NUM_STRINGS ];
	//

	// Sizes
	enum
	{
		GC_SPRITE_SIZE_BORDER = 0,
		GC_SPRITE_SIZE_IMAGE,
		GC_SPRITE_SIZE_FILTER,
		GC_NUM_SPRITE_SIZE
	};

	Size m_aSize[ GC_NUM_SPRITE_SIZE ];
	//

	// UV
	enum
	{
		GC_UV_FILTER = 0,
		GC_UV_IMAGE,
		GC_NUM_UV
	};
	
	UVExtents m_aUV[ GC_NUM_UV ];
	//

	CString* m_pobString;

	Transform*		m_pobBaseTransform;
	CPoint			m_BasePosition;
};

typedef CGuiSkinMenuImageGridCell CGuiGridCell;

#endif // _GUISKINMENUIMAGEGRID_H

