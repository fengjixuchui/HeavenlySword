//------------------------------------------------------------------------------------------
//!
//!	\file Commandresult.h
//!
//! \note This file should be included wherever a Command(function that can be invoked by 
//! registered keypress, external script etc) is declared within a class. The command
//! member function should return a COMMAND_RESULT. 
//!
//------------------------------------------------------------------------------------------

#ifndef _COMMAND_RESULT_H
#define _COMMAND_RESULT_H

// The command result enum

enum COMMAND_RESULT {
	CR_SUCCESS,
	CR_FAILED
};

#endif //_COMMAND_RESULT_H
