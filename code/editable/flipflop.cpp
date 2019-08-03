
/***************************************************************************************************
*
*	DESCRIPTION		Implementation of the FlipFlop class
*
*	NOTES			This is the game side class for an editable series of on off states
*
***************************************************************************************************/

#include "editable/flipflop.h"

// A buffer for the flipflop to write it's text into
char CFlipFlop::g_acTextBuffer[CFlipFlop::m_giTextBufferSize];

/***************************************************************************************************
*
*	FUNCTION		CFlipFlop Constructor
*
*	DESCRIPTION		Default constructor
*
***************************************************************************************************/
CFlipFlop::CFlipFlop()
{
}

/***************************************************************************************************
*
*	FUNCTION		CFlipFlop Destructor
*
*	DESCRIPTION		Default destructor
*
***************************************************************************************************/
CFlipFlop::~CFlipFlop()
{
	Reset();
}

/***************************************************************************************************
*
*	FUNCTION		CFlipFlop::Build
*
*	DESCRIPTION		Build the flip values from a text string
*
***************************************************************************************************/
bool CFlipFlop::Build( const char* pcNewFlips )
{
	// Clear out all the old values
	Reset();

	// Check for a NULL input
	if (strcmp(pcNewFlips, "NULL") != 0)
	{
		// Copy the input text before breaking it
		#define FLIPCOPY_BUFFER 512 
		char cBuffer[FLIPCOPY_BUFFER];
		ntAssert(strlen(pcNewFlips) < FLIPCOPY_BUFFER-1);
		strcpy(cBuffer, pcNewFlips);

		// Parse the text of form "x1,w1/x2,w2/x3,w3"
		// Get the x part
		char * pcTokenX = strtok(cBuffer, ",");
		while (pcTokenX != 0)
		{
			// Get the w part
			char * pcTokenW = strtok(0, "/"); 
			// Create a NT_NEW flipper and fill it in
			CFlipper obFlipper;
			obFlipper.Set((float)atof(pcTokenX), (float)atof(pcTokenW));
			m_obFlipList.push_back(obFlipper);
			// Try again for the next x part
			pcTokenX = strtok(0, ","); 
		}
	}
	return true;
}

/***************************************************************************************************
*
*	FUNCTION		CFlipFlop::Reset
*
*	DESCRIPTION		Return the FlipFlop to an empty state
*
***************************************************************************************************/
void CFlipFlop::Reset()
{
	m_obFlipList.clear();
}


/***************************************************************************************************
*
*	FUNCTION		CFlipFlop::IsSet
*
*	DESCRIPTION		Test the CFlipFlop at a given time
*
***************************************************************************************************/
bool CFlipFlop::IsSet( float fValue, float fScale ) const
{
	if ((fValue >= 0.0f) && (fValue < fScale))
	{
		for ( FlipperContainerType::const_iterator obIt = m_obFlipList.begin(); obIt != m_obFlipList.end(); ++obIt )
		{
			if ((fValue >= obIt->X1(fScale)) && (fValue < obIt->X2(fScale)))
			{
				return true;
			}
		}
	}
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CFlipFlop::ToString
*
*	DESCRIPTION		Convert to a string
*
***************************************************************************************************/
char* CFlipFlop::ToString() const
{
	// Check for zero elements
	if ( m_obFlipList.size() == 0 )
	{
		return "NULL";
	}

	g_acTextBuffer[0] = 0;
	char acBuffer[64];

	for ( FlipperContainerType::const_iterator obIt = m_obFlipList.begin(); obIt != m_obFlipList.end(); ++obIt )
	{
		//Need to round values to nearest 0.5...
		sprintf( acBuffer, "%f,%f/", floor(0.5f+2.0f*(obIt->m_fStart*FLIPPER_INTERNAL_SCALE))/2.0f, floor(0.5f+2.0f*(obIt->m_fWidth*FLIPPER_INTERNAL_SCALE))/2.0f);
		strcat( g_acTextBuffer, acBuffer );
	}

	// If we have something in the buffer
	if ( g_acTextBuffer[0] != 0 )
	{
		// rub out the trailing /
		g_acTextBuffer[strlen(g_acTextBuffer)-1] = 0;
	}
	return g_acTextBuffer;
}


/***************************************************************************************************
*
*	FUNCTION		CFlipFlop::GetFirstValue
*
*	DESCRIPTION		Get the value of the first flip in the FlipFlop returns -1.0f if empty.
*
***************************************************************************************************/
float CFlipFlop::GetFirstValue( float fScale ) const
{	
	if ( !m_obFlipList.empty() )
		return m_obFlipList.begin()->X1( fScale ); 
	else
		return -1.0f;
}


/***************************************************************************************************
*
*	FUNCTION		CFlipFlop::GetFirstValueLength
*
*	DESCRIPTION		Get the length of the first flip in the FlipFlop returns -1.0f if empty.
*
***************************************************************************************************/
float CFlipFlop::GetFirstValueLength( float fScale ) const
{	
	if ( !m_obFlipList.empty() )
		return ( m_obFlipList.begin()->X2( fScale ) - m_obFlipList.begin()->X1( fScale ) ); 
	else
		return -1.0f;
}
