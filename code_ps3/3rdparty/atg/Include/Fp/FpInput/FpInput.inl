//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpInput : PS3 input manager 

	@warning	Temporary PC port. will be rewritten

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_INPUT_PS3_INL
#define FP_INPUT_PS3_INL

//--------------------------------------------------------------------------------------------------
//	FUNCTIONS DEFINITIONS
//--------------------------------------------------------------------------------------------------

FpPad* FpInput::GetPad(unsigned int numPad) 
{
	FpPad* pPad=NULL;

	if (numPad<kMaxPads)
	{
		pPad=&m_pads[numPad];
	}

	return pPad;
}

FpKeyboard* FpInput::GetKeyboard(unsigned int numKeyboard) 
{	
	FpKeyboard* pKeyboard=NULL;

	if (numKeyboard<kMaxKeyboards)
	{
		pKeyboard=&m_keyboards[numKeyboard];
	}

	return pKeyboard;
}

FpMouse* FpInput::GetMouse(unsigned int numMouse) 
{
	FpMouse* pMouse=NULL;

	if (numMouse<kMaxMice)
	{
		pMouse=&m_mice[numMouse];
	}

	return pMouse;
}

//--------------------------------------------------------------------------------------------------

#endif	//FP_INPUT_PS3_INL
