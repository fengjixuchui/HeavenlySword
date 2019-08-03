/***************************************************************************************************
*
*	DESCRIPTION		This is a switch betweem n number of item returing the int that it is selected.
*					Only the selected one is rendered.
*					Objects are switched in an as added order. 
*
*	NOTES			
*
***************************************************************************************************/

#ifndef CSMULTISELECT_H
#define CSMULTISELECT_H

#include "core/nt_std.h"
#include "cslistobjectbase.h"
#include "effect/screensprite.h"

class CSMultiSelect 
{
public:
			CSMultiSelect();
	virtual ~CSMultiSelect();

	void	MovePrev( void );
	void	MoveNext( void );
	void	AddItem( CSListObjectBase* pObject );
	int		GetCurrentSelection( void ) const;
	void	Render( void );
	void	Update( void );
	bool	HasChanged( void ) const;
	void	DeleteItems( void );
	void	ClearChangedState( void );
	void	SetArrowPosition( const float fY, const float fLeftX, const float fRightX );

protected:

	ntstd::List< CSListObjectBase* >	m_ItemList;					///< The list of all items we can switch between
	ScreenSprite						m_LeftArrow;				///< The can move left arrow
	ScreenSprite						m_RightArrow;				///< The can move right arrow
	CSListObjectBase*					m_pCurrentItemCache;		///< The current item we have selected, use to make life easy
	int									m_iCurrentItem;				///< What number is the item we currently have selected
	float								m_fArrowsY;					///< The y pos of the left and right arrows
	float								m_fArrowsLeftX;				///< X pos of the left arrow
	float								m_fArrowsRightX;			///< X pos of the right arrow
	bool								m_bHasChanged;				///< Has the user changed the selection
	bool								m_bShowLeft;				///< Should we show the left arrow
	bool								m_bShowRight;				///< Should we show the right arrrow	

}; //end class CSMultiSelect

#endif //CSMULTISELECT_H

