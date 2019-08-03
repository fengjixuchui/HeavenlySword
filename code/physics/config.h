
#ifndef _DYNAMICS_CONFIG_INC
#define _DYNAMICS_CONFIG_INC

#define USE_HAVOK_MULTI_THREADING
#define USE_ASYCHRONOUS_SIMULATION

#ifdef USE_ASYCHRONOUS_SIMULATION
#define SYNC_ASYNCHRONOUS_SIMULATION // asynchronous simulation, but time step is synchronized with frame step
#endif

#define BINARY_PHYSICS_LOADER // loads physics files in binarized file format
#define PHYSICS_LOADER_410 // unfortunatelly binary files are not compatible between Havok 4.0.0 a Havok 4.1.

#ifdef PLATFORM_PS3
#	define USE_HAVOK_ON_SPUS
#endif

#ifdef _PS3_RUN_WITHOUT_HAVOK_BUILD
typedef unsigned short	hkUint16;
typedef unsigned int	hkUint32;
typedef short			hkInt16;
typedef int				hkInt32;
#endif // _PS3_RUN_WITHOUT_HAVOK_BUILD

#define HK_NEW					new
#define HK_DELETE(n)			{ delete n; }
#define HK_DELETE_ARRAY(n)		{ delete[] n; }

#define MC_PHYSICS				Mem::MC_HAVOK
#define GATSO_PHYSICS_START     CGatso::Start
#define GATSO_PHYSICS_STOP       CGatso::Stop

#define LARGE_INTERACTABLE_VOLUME_THRESHOLD 0.75f
#define MAX_VEL_FOR_DEAD_RAGDOLL 10.0f // when ragdoll is swithced to DEAD state, all velocities must be under this value. 
                                       // If they are not, they are renormalized to have maximal one on this val

#endif //_DYNAMICS_CONFIG_INC
