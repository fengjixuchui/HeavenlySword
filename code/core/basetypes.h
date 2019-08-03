#ifndef CORE_BASETYPES_H_
#define CORE_BASETYPES_H_

#if defined( PLATFORM_PC)
#	include "core/basetypes_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "core/basetypes_ps3.h"
#endif

static const size_t POINTER_SIZE_IN_BYTES = sizeof( intptr_t );

#endif //CORE_BASETYPES_H_
