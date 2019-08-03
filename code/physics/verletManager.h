#ifndef _VERLETMANAGER_H_
#define _VERLETMANAGER_H_

#if defined(PLATFORM_PC)
#include "physics/verletManager_pc.h"
#elif defined( PLATFORM_PS3)
#include "physics/verletManager_ps3.h"
#endif

#endif // end of _VERLETMANAGER_H_
