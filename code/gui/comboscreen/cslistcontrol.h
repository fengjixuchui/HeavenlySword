/***************************************************************************************************
*
*	DESCRIPTION		This is a will render and update a list of item on the screen, scrolling should there be to many.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef CSLISTCONTROL_H
#define CSLISTCONTROL_H

#include "core/nt_std.h"
#include "anim/transform.h"
#include "cslistobjectbase.h"
#include "effect/screensprite.h"
#include "cslistseperator.h"

class CSListControl
{
public:
   CSListControl();
   ~CSListControl();

   //This will update all the children in the list
   void Update( void );

   //this will render all the current visible children, draw the speperator lines, and the up and down arrows
   void Render( void );

   //this sets how many items to display at once
   void NumItemsPerPage( const int iNum );

   //inset an item into the list, auto incrementing
   void AddItem( CSListObjectBase* pObject );

   //this tells the control to scroll down, returning false if this is an invalid request.
   bool	ScrollDown( void );

   //this tells the control to scroll down, returning false if this is an invalid request.
   bool ScrollUp( void );

   //move to the first item in the list
   void JumpToStart( void );

   //this will cal delete on all the pointers held by the list
   void DeleteAllChildren( void );

   //this sets the transform that all the items added use. This is prodded by the control to scroll things up and down.
   void SetControlingTransform( Transform* pTransform );

   //when me move from one object to the next how far do we have to move
   void SetItemHeight( const float& fHeight );

   //set the locations to render the up and down arrows
   void SetUpAndDownArrowItems ( float fXPos, float fTopY, float fBottomY );

   //set the top corner of the control. 
   void SetTopXAndYOfControl( const float& fXpos, const float& fYpos );
   
   //this will return the current item height.
   float GetItemHeight( void );

   //this will update the seperators to reflect the current state of the control
   void MoveSeperators( void );

   //move the selection cursor and scroll the control to match
   void UpdateSelectionCursor( void );

   //this will scroll the display to place the selected item in the middle if possible
   void CenterItemsAroundSelectionCursor( void );

protected:

	//this will calc what item is the top and what item is the bottom
	int CalcCurrentItemRange( void );
	//convience fumction to iterate over the current list of item in the list to the first item to be rendered.
	void AdvanceIteratorToStart( ntstd::List< CSListObjectBase* >::iterator &itor );
	

	ntstd::List< CSListObjectBase* >	m_ItemList;						///< This holds all the items in the list

	Transform*							m_pObjectControlTransform;		///< The transform that all objects are bound to
	ScreenSprite						m_UpArrow;						///< The up arrow sprite
	ScreenSprite						m_DownArrow;					///< The down arrow sprite
	ScreenSprite						m_SelectionIcon;				///< The current selection arrow thing sprite
	CSListSeperator						m_Seperators;					///< The seperators wrapper class
	int									m_iNumItems;					///< How many items are currently in the list
	int									m_iCurrentTopItem;				///< The item that is the first visible item on the list
	int									m_iNumItemsPerPage;				///< How many items to show at one time.
	int									m_iCurrentSelection;			///< The item that is being selects and is at the middle of the screen were possible
	float								m_fItemHeight;					///< How high each item is
	float								m_fX;							///< The X position of the left side of the control
	float								m_fY;							///< The Y position of the top side of the control
	bool								m_bShowUpArrow;					///< Do we render the up arrow.
	bool								m_bShowDownArrow;				///< Do we render the down arrow.

};

#endif //#ifndef CSLISTCONTROL_H
