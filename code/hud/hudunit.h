/***************************************************************************************************
*
*	DESCRIPTION		The basic unit on which HUD items are based
*
*	NOTES			
*
***************************************************************************************************/

#ifndef	_HUDUNIT_H
#define	_HUDUNIT_H


// Includes

// Forward declarations
class CHudUnit;

/***************************************************************************************************
*
*	CLASS			CHudUnitDef
*
*	DESCRIPTION		The basic def unit for a HUD element.
*
***************************************************************************************************/

class CHudUnitDef
{
public:

	// A pure virtual call to be overridden by specific unit defintions
	virtual CHudUnit* CreateInstance( void ) const = 0;

	//! Construction Destruction - derive if you want one
	CHudUnitDef( void );
	virtual ~CHudUnitDef( void );
};

/***************************************************************************************************
*
*	CLASS			CHudUnit
*
*	DESCRIPTION		The basic unit for a HUD element.
*
***************************************************************************************************/

class CHudUnit
{
protected:
	
	// Unit States
	enum UNIT_STATE
	{
		STATE_ENTER,
		STATE_ACTIVE,
		STATE_EXIT,
		STATE_INACTIVE,

		UNIT_STATE_SIZE,
	}	m_eUnitState;

public:

	//! The biggies
	virtual bool	Initialise( void );
	virtual bool	Update( float fTimestep );
	virtual bool	Render( void );

	// State Updates
	virtual bool	BeginEnter( bool bForce = false );
	virtual bool	BeginExit( bool bForce = false );

	void			RemoveOnExit ( bool bExit = true ) { m_bRemoveOnExit = bExit; };

	//! Construction Destruction - derive if you want one
	CHudUnit( void );
	virtual ~CHudUnit( void );

	bool		IsActive( void ) const		{	return m_eUnitState == STATE_ACTIVE;	}
	bool		IsDeactive( void ) const	{	return m_eUnitState == STATE_INACTIVE;	}
protected:


	// State based updates - should probably be scripted at some point
	virtual void	UpdateEnter( float fTimestep );
	virtual void	UpdateInactive( float fTimestep );
	virtual void	UpdateActive( float fTimestep );
	virtual void	UpdateExit( float fTimestep );

	// State Setting code
	virtual void	SetStateEnter( void );
	virtual void	SetStateExit( void );
	virtual void	SetStateInactive( void );
	virtual void	SetStateActive( void );

	CPoint			m_obPosition;

	bool m_bRemoveOnExit;
};

#endif	//_HUDUNIT_H
