
/***************************************************************************************************
*
*	DESCRIPTION		Implementation of the FlipFlop class
*
*	NOTES			This is the game side class for an editable series of on off states
*
***************************************************************************************************/

#ifndef _FLIP_FLOP_H
#define _FLIP_FLOP_H

// Forward refs
class CFlipFlop;

/***************************************************************************************************
*
*	CLASS			CFlipper
*
*	DESCRIPTION		An individual state within a CFlipFlop
*
***************************************************************************************************/

#define FLIPPER_INTERNAL_SCALE 100.0f

class CFlipper
{
public:
typedef

	friend class CFlipFlop;

	// Constructor
	CFlipper() {};

	// Destructor
	~CFlipper() {};

	// Set the flipper to a particular time and width
	void Set(float fStart, float fWidth)
	{
		m_fStart = fStart/FLIPPER_INTERNAL_SCALE;
		m_fWidth = fWidth/FLIPPER_INTERNAL_SCALE;
	}

	// Get the flipper values unscaled
	void Get(float& fStart, float& fWidth) const
	{
		fStart = m_fStart * FLIPPER_INTERNAL_SCALE;
		fWidth = m_fWidth * FLIPPER_INTERNAL_SCALE;
	}

	// Get the lower time
	float X1(float fScale) const
	{
		return m_fStart*fScale;
	}

	// Get the upper time
	float X2(float fScale) const
	{
		return (m_fStart + m_fWidth)*fScale;
	}

private:
	// The starting time 0.0-1.0
	float m_fStart;
	// The width 0.0-1.0
	float m_fWidth;
};


/***************************************************************************************************
*
*	CLASS			CFlipFlop
*
*	DESCRIPTION		This is the game side class for an editable series of on off states
*
***************************************************************************************************/

class CFlipFlop
{
public:
	typedef ntstd::Vector<CFlipper> FlipperContainerType;

	// Construction destruction
	CFlipFlop();
	~CFlipFlop();

	// Build from text
	bool Build( const char* pcNewFlips );

	// Reset all flip values
	void Reset();

	/*******************************************************************************
	*	The stuff below is all const access to the data in the flip flop
	*******************************************************************************/

	// Test the flipflop at a particular point in time
	bool IsSet( float fValue, float fScale ) const;

	// Create a text version of this FlipFlop
	char* ToString() const;

	// Info acessors for parsing flipflop internals
	int	GetNumFlippers( void ) const { return m_obFlipList.size(); }
	
	// Get the value of the first flip in the FlipFlop returns -1.0f if empty
	float GetFirstValue( float fScale ) const;

	// Get the length of the first flip in the FlipFlop returns -1.0f if empty
	float GetFirstValueLength( float fScale ) const;

	FlipperContainerType::const_iterator Begin( void ) const	{ return m_obFlipList.begin(); }
	FlipperContainerType::const_iterator End( void ) const		{ return m_obFlipList.end(); }

private:

	// List of flippers
	FlipperContainerType m_obFlipList;

	// Text buffer to write text version into
	static char g_acTextBuffer[];

	// Size of the text buffer
	static const int m_giTextBufferSize = 1024;
};


#endif //_FLIP_FLOP_H
