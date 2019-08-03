//------------------------------------------------------------------------------------------
//!
//!	\file syncdcombat.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_SYNCDCOMBAT_H
#define	_SYNCDCOMBAT_H

// Forward declarations
class CStrike;
class CEntity;

//------------------------------------------------------------------------------------------
//!
//!	SyncdCombat
//!	Used for coordinating strikes - so we know when they are going to arrive on a 
//! particular character.  Hopefully i can grow the functionality of this component to 
//! manage all combat interactions and hence provide a single point of access for the 
//! network code.
//!
//------------------------------------------------------------------------------------------
class SyncdCombat : public Singleton< SyncdCombat >
{
public:

	// Construction destruction
	SyncdCombat( void );
	~SyncdCombat( void );

	// Some update action - doesn't need a time step for now
	void Update( void );

	// When we send strikes we can send it here - then we know that it will be
	// processed on the frame after it was sent, rather than either the same or the next
	void PostStrike( CStrike* pobStrike );

private:

	// A list of the movements we need to kick off
	ntstd::List< CStrike* > m_obStrikesToSend;
};

#endif // _SYNCDCOMBAT_H
