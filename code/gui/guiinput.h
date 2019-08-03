/***************************************************************************************************
*
*	DESCRIPTION		Filters user input into a form that is meaningful to the user interface
*
*	NOTES			Basically this code further abstracts any input from the pad code into UI
*					relevant commands - e.g. Up, Down, Select, Back, Start...
*
***************************************************************************************************/

#ifndef _GUIINPUT_H
#define _GUIINPUT_H

// Includes
#include "input\inputhardware.h"
#include "guiutil.h"

/***************************************************************************************************
*
*	CLASS			CGuiInput
*
*	DESCRIPTION		Filters actual pad input into actions that the gui may respond to.
*
*					The command names are abstracted from actual controller button names.  This
*					class will need to be written on a per platform basis, but the interface to 
*					this class should not.  For example on the xbox a back action would be issued
*					by pressing the B or BACK button.  On other platforms it may link only to a 
*					single button press.
*
*					Timing issues have been simplified to one timer per controller.  As far as the
*					gui is concerned each pad will be able to offer a single action, after which it
*					will be disabled for a fixed amount of time until it may offer another.
*
*					Actions performed on a pad will be indentified in a bitwise manner.  As an
*					example, if the is a left action on the third pad only then the return value for
*					'MoveLeft' will be ( 0x01 << 2 ).  This shift is based on the controller number
*					enum - however the GUI code will never reference a specific pad, it will work
*					simply from the PAD_NUM value.
*
***************************************************************************************************/

class CGuiInput : public Singleton<CGuiInput>
{
public:

	//! Construction Destruction
	CGuiInput( void )
	:	m_bLocalisation ( false )
	,	m_eSelectPress ( PAD_FACE_3 )
	,	m_eCancelPress ( PAD_FACE_1 )
	,	m_bInputBlocked ( false )
	{}
	~CGuiInput( void ) {}

	void	Initialise( void );
	void	Update( void );

	//! Movement
	int		MoveDown( bool bGetRaw = false );
	int		MoveUp( bool bGetRaw = false );
	int		MoveLeft( bool bGetRaw = false );
	int		MoveRight( bool bGetRaw = false );

	//! Actions
	int		StartPress( bool bGetRaw = false );
	int		SelectPress( bool bGetRaw = false );
	int		Back( bool bGetRaw = false );
	int		Delete( bool bGetRaw = false );
	int		AnyAction( bool bGetRaw = false );

	//! Pad Status monitoring
	void	UpdatePadStatus( void );
	int		PadStatusChange( void );
	int		GetCurrentStatus( void );
	bool	GetPadCurrentStatus( PAD_NUMBER ePad );

	enum ACCEPTS_INPUT
	{
		ACCEPTS_UNKNOWN = 0,
		ACCEPTS_NONE	= (1 << 0),
		ACCEPTS_ANY		= (1 << 1),
		ACCEPTS_LEFT	= (1 << 2),
		ACCEPTS_RIGHT	= (1 << 3),
		ACCEPTS_UP		= (1 << 4),
		ACCEPTS_DOWN	= (1 << 5),
		ACCEPTS_START	= (1 << 6),
		ACCEPTS_SELECT	= (1 << 7),
		ACCEPTS_BACK	= (1 << 8),
		ACCEPTS_DELETE	= (1 << 9),
		ACCEPTS_ALL		= ACCEPTS_ANY|ACCEPTS_LEFT|ACCEPTS_RIGHT|ACCEPTS_UP|ACCEPTS_DOWN|ACCEPTS_START|ACCEPTS_SELECT|ACCEPTS_BACK|ACCEPTS_DELETE
	};
	static const CStringUtil::STRING_FLAG ms_astrAcceptsInputFlags[];

	static bool AcceptsInput(int iFlags, ACCEPTS_INPUT input)	{ return (iFlags&input) == input; }

	bool InputBlocked() const { return m_bInputBlocked; }
	void BlockInput(bool bBlock) { m_bInputBlocked = bBlock; }

protected:

	// Menu button localisation
	bool m_bLocalisation;
	PAD_BUTTON m_eSelectPress;
	PAD_BUTTON m_eCancelPress;

	//! Helpers
	void	InitialiseTimers( void );
	bool	PadMovement( PAD_NUMBER ePadNumber, PAD_BUTTON eDigitalDirection, float fAnalogueAngle, bool bGetRaw = false );
	bool	PadAnyAnalogueMovement( PAD_NUMBER ePadNumber, bool bGetRaw = false );
	bool	PadAction( PAD_NUMBER ePadNumber, u_int uiPadButtons, bool bGetRaw = false );

	//! Members
	CGuiTimer	m_aobPadTimers[PAD_NUM];
	bool		m_abPadStatus[PAD_NUM];

	bool		m_bInputBlocked;
};

#endif // _GUIINPUT_H
