//----------------------------------------------------------------------------------------
//! 
//! \filename core/loader_callback_ps3.h
//! 
//----------------------------------------------------------------------------------------
#ifndef LOADER_CALLBACK_PS3_H_
#define LOADER_CALLBACK_PS3_H_

namespace Loader
{
	//
	//	Callback stage enum - passed to the callback to notify of different stages of completion.
	//
	enum CallbackStage
	{
		LCS_SYNCPOINT				= 0,

		LCS_NUM_CALLBACK_STAGES
	};

	//
	//	Declare the callback type.
	//
	typedef void (*Callback)( CallbackStage, volatile void * );
}

#endif // !LOADER_CALLBACK_PS3_H_
