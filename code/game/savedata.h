#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#if defined( PLATFORM_PC )
#	include "game/savedata_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "game/savedata_ps3.h"
#endif

#endif //SAVE_DATA_H

