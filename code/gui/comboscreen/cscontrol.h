/***************************************************************************************************
*
*	DESCRIPTION		This is the class that does all the fancy stuff for the combo screen.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef CSCONTROL_H
#define CSCONTROL_H

#include "gui/guiunit.h"
#include "anim/transform.h"
#include "cslistcontrol.h"
#include "cslistobjecttri.h"
#include "csmultiselect.h"
#include "objectdatabase/dataobject.h"
#include "core/nt_std.h"
#include "game/checkpointmanager.h"
#include "cslistobjecttext.h"
#include "game/combatstyle.h"
#include "cslistobjectnew.h"


struct CS_MOVE_INFO_PARENT
{
	ntstd::String			cpName;
	ntstd::String			cpMoveDesc;
	ComboUnlockItemData*	pComboUnlockData;
};

struct CS_MOVE_INFO_BASIC : public CS_MOVE_INFO_PARENT
{
	HAS_INTERFACE( CS_MOVE_INFO_BASIC )
	
	void RegisterBasicMove( void );
};

struct CS_MOVE_INFO_SPEED  : public CS_MOVE_INFO_PARENT
{
	HAS_INTERFACE( CS_MOVE_INFO_SPEED )

	void RegisterSpeedMove( void );
};

struct CS_MOVE_INFO_RANGE  : public CS_MOVE_INFO_PARENT
{
	HAS_INTERFACE( CS_MOVE_INFO_RANGE )

	void RegisterRangeMove( void );
};

struct CS_MOVE_INFO_POWER  : public CS_MOVE_INFO_PARENT
{
	HAS_INTERFACE( CS_MOVE_INFO_POWER )

	void RegisterPowerMove( void );
};

struct CS_MOVE_INFO_AERIAL  : public CS_MOVE_INFO_PARENT
{
	HAS_INTERFACE( CS_MOVE_INFO_AERIAL )

	void RegisterPowerMove( void );
};

struct CS_MOVE_INFO_SUPERSTYLE  : public CS_MOVE_INFO_PARENT
{
	HAS_INTERFACE( CS_MOVE_INFO_SUPERSTYLE )

	void RegisterPowerMove( void );
};

class CSControl : public CGuiUnit
{
	typedef CGuiUnit super;
public:
	// Construction Destruction
	CSControl( void );
	virtual ~CSControl( void );

   	//! The biggies
	virtual bool	Render( void );

	//! Movement Commands - these should remain empty in this abstract class
	virtual bool	MoveLeftAction( int iPads );	
	virtual bool	MoveRightAction( int iPads );
	virtual bool	MoveDownAction( int iPads );	
	virtual bool	MoveUpAction( int iPads );	

	// State Updates
	virtual bool	BeginEnter( bool bForce = false );
	virtual bool	BeginIdle( bool bForce = false );
	virtual bool	BeginFocus( bool bForce = false );
	virtual bool	BeginExit( bool bForce = false );

	enum LayoutType { BASIC = 0, AERIAL, RANGE, POWER, MISC, SPEED, SUPERSTYLE };

protected:
	virtual void	UpdateIdle( void );
	virtual bool	ProcessEnd( void );
	void SetUpWindows( void );

	void	FlushListBox();
	void	FillListBoxWithBasic();
	void	FillListBoxWithSpeed();
	void	FillListBoxWithRange();
	void	FillListBoxWithPower();
	void	FillListBoxWithAerial();
	void	FillListBoxWithSuper();
	void	FillListFromPointer( ntstd::List< CS_MOVE_INFO_PARENT* >* pList, LayoutType eType );

	void	FillMultiControlWithBasic();
	void	AddStancesToMultiControl();

	CSListObjectTri*	BuildTriFromStrings( const char* cpName, const char* cpMoveList, bool bNew, float fOffset );
	CSListObjectTri*	Build3LineFromStrings( const char* cpName, const char* cpMoveList, bool bNew, float fOffset );
	CSListObjectTri*	Build2LineFromStrings( const char* cpName, const char* cpMoveList, bool bNew, float fOffset );
	CSListObjectTri*	BuildSuperListLineFromStrings( const char* cpName, const char* cpMoveList, bool bNew, float fOffset );

	Transform			m_TempTransform;			///< Transform for the list box item, prob should have a name change ;)
	Transform			m_MultiTranform;			///< Trandform for the multi control
	CSListControl		m_ListControl;				///< The list box
	CSMultiSelect		m_MultiControl;				///< The multi selection
	CSListObjectText	m_Info;						///< The info box at the bottom

	ScreenSprite m_aobBackgroundWindow1[9];			///< Sprites for window 1 Main
	ScreenSprite m_aobBackgroundWindow2[9];			///< Sprites for window 2 Info

	bool m_bHasSeenBasic;							///< Keep track to see if they have viewed a page
	bool m_bHasSeenRange;
	bool m_bHasSeenSpeed;
	bool m_bHasSeenPower;
	bool m_bHasSeenAerial;
	bool m_bHasSeenSuper;

}; //end class CSControl.

#endif //CSCONTROL_H
