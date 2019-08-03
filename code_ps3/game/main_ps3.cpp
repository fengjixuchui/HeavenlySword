//------------------------------------------------------------------------------------------
//!
//!	\file main_ps3.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/shellmain.h"
#include "input/inputhardware.h"
#include "core/memman.h"
#include "core/fileio_ps3.h"
#include <libsntuner.h>
#include <cell/sysmodule.h>
#include "sys/process.h"
#include "sys/paths.h"
#include <sys/timer.h>

//extern void ForceLinkFunction2();
extern void ForceLinkFunction4();
extern void ForceLinkFunction5();
extern void ForceLinkFunction6();
extern void ForceLinkFunction7();
extern void ForceLinkFunction8();
extern void ForceLinkFunction9();
extern void ForceLinkFunction10();
extern void ForceLinkFunction11();
extern void ForceLinkFunction12();
extern void ForceLinkFunction13();
extern void ForceLinkFunction14();
extern void ForceLinkFunction15();
extern void ForceLinkFunction16();
extern void ForceLinkFunction17();
extern void ForceLinkFunction18();
extern void ForceLinkFunction19();
extern void ForceLinkFunction20();
extern void ForceLinkFunction21();
extern void ForceLinkFunction22();
extern void ForceLinkFunction23();
extern void ForceLinkFunction24();
extern void ForceLinkFunction25();
extern void ForceLinkFunction26();
extern void ForceLinkFunction27();

extern void ForceLinkFunctionHUD();

extern void ForceLinkFunctionPushable();
extern void ForceLinkFunctionRigidBody();
extern void ForceLinkFunctionThrown();
extern void ForceLinkFunctionTraverser();
extern void ForceLinkFunctionColapsableAnimated();
extern void	ForceLinkFunctionCollapsableHierarchy();
extern void ForceLinkFunctionBoulder();
extern void ForceLinkFunctionEntityFire();
extern void ForceLinkFunctionBSSkin();
extern void ForceLinkFunctionSimpleSwitch();
extern void ForceLinkFunctionSimpleDoor();
extern void ForceLinkFunctionButtonMash();
extern void ForceLinkFunctionSwitch();
extern void ForceLinkFunctionTwoWaySwitch();
extern void ForceLinkFunctionHoistStatue();
extern void ForceLinkFunctionLadder();
extern void ForceLinkFunctionSpear();
extern void ForceLinkFunctionBarrage();
extern void ForceLinkFunctionCheckpoint();
extern void ForceLinkFunctionArmy();
extern void ForceLinkFunctionTurretPoint();
extern void ForceLinkFunctionLookAtInfo();
extern void ForceLinkFunctionKite();
extern void ForceLinkFunctionProjectile();
extern void ForceLinkFunctionRangedWeapon();
extern void ForceLinkFunctionAnimated();
extern void ForceLinkFunctionParticleEmitter();
extern void ForceLinkFunctionCatapult();
extern void ForceLinkFunctionCatapultRock();
extern void ForceLinkFunctionTurretWeapon();
extern void ForceLinkFunctionSpeedGrass();
extern void ForceLinkFunctionWaterInstanceDef();
extern void ForceLinkFunctionArmyMessageHub();

class EarlyStaticInit
{
public:
    EarlyStaticInit()
    {
		ntPrintf( "STATIC INITIALISE - LOADING PRX.\n" );
		cellSysmoduleInitialize();

		int32_t res = cellSysmoduleLoadModule(CELL_SYSMODULE_GCM_SYS);
        ntError(res >= 0);

        res = cellSysmoduleLoadModule(CELL_SYSMODULE_IO);
        ntError(res >= 0);

        res = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
        ntError(res >= 0);

        res = cellSysmoduleLoadModule(CELL_SYSMODULE_SPURS);
        ntError(res >= 0);

        res = cellSysmoduleLoadModule(CELL_SYSMODULE_USBD);
        ntError(res >= 0);

        res = cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
        ntError(res >= 0);

        res = cellSysmoduleLoadModule(CELL_SYSMODULE_SYNC);
        ntError(res >= 0);

        res = cellSysmoduleLoadModule(CELL_SYSMODULE_NETCTL);
        ntError(res >= 0);

        res = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL);
        ntError(res >= 0);
    }

	static void UnloadModules()
	{
		int32_t res;
		res = cellSysmoduleUnloadModule( CELL_SYSMODULE_SYSUTIL );
		if ( res != CELL_OK )
			printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_SYSUTIL ) = 0x%x\n", res );

		res = cellSysmoduleUnloadModule( CELL_SYSMODULE_NETCTL );
		if ( res != CELL_OK )
			printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_NETCTL ) = 0x%x\n", res );

		res = cellSysmoduleUnloadModule( CELL_SYSMODULE_SYNC );
		if ( res != CELL_OK )
			printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_SYNC ) = 0x%x\n", res );

		res = cellSysmoduleUnloadModule( CELL_SYSMODULE_FS );
		if ( res != CELL_OK )
			printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_FS ) = 0x%x\n", res );

		res = cellSysmoduleUnloadModule( CELL_SYSMODULE_USBD );
		if ( res != CELL_OK )
			printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_USBD ) = 0x%x\n", res );

		res = cellSysmoduleUnloadModule( CELL_SYSMODULE_SPURS );
		if ( res != CELL_OK )
			printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_SPURS ) = 0x%x\n", res );

		res = cellSysmoduleUnloadModule( CELL_SYSMODULE_NET );
		if ( res != CELL_OK )
			printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_NET ) = 0x%x\n", res );

		res = cellSysmoduleUnloadModule( CELL_SYSMODULE_IO );
		if ( res != CELL_OK )
			printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_IO ) = 0x%x\n", res );

		res = cellSysmoduleUnloadModule( CELL_SYSMODULE_GCM_SYS );
		if ( res != CELL_OK )
			printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_GCM_SYS ) = 0x%x\n", res );

		cellSysmoduleFinalize();
	}
};

static EarlyStaticInit ps3EarlyStaticInit __attribute__((init_priority(101)));

/***************************************************************************************************
*
*	FUNCTION		SN Object Loader support
*
***************************************************************************************************/
char snLoadRequest = 0;
void snSafeLoadPointBreak (void)
{
    asm("nop");
}
void snSafeLoadPoint (void)
{
    if (snLoadRequest)
        snSafeLoadPointBreak();
}


/***************************************************************************************************
*
*	FUNCTION		main
*
*	DESCRIPTION		The fun starts here :)
*
***************************************************************************************************/

int main(int argc, char* argv[] )
{
	//ForceLinkFunction2();
	//ForceLinkFunction3();
	ForceLinkFunction4();
	ForceLinkFunction10();
	ForceLinkFunction11();
	ForceLinkFunction12();
	ForceLinkFunction13();
	ForceLinkFunction14();
	ForceLinkFunction15();
	ForceLinkFunction16();
	ForceLinkFunction17();
	ForceLinkFunction18();
	ForceLinkFunction19();
	ForceLinkFunction20();
	ForceLinkFunction21();
	ForceLinkFunction22();
	ForceLinkFunction23();
	ForceLinkFunction24();
	ForceLinkFunction25();
	ForceLinkFunction26();
	ForceLinkFunction27();

	ForceLinkFunctionHUD();

	ForceLinkFunctionPushable();
	ForceLinkFunctionRigidBody();
	ForceLinkFunctionThrown();
	ForceLinkFunctionTraverser();
	ForceLinkFunctionColapsableAnimated();
	ForceLinkFunctionCollapsableHierarchy();
	ForceLinkFunctionBoulder();
	ForceLinkFunctionEntityFire();

	ForceLinkFunctionBSSkin();
	ForceLinkFunctionSimpleDoor();
	ForceLinkFunctionSwitch();
	ForceLinkFunctionSimpleSwitch();
	ForceLinkFunctionTwoWaySwitch();
	ForceLinkFunctionButtonMash();
	ForceLinkFunctionHoistStatue();
    ForceLinkFunctionBarrage();
	ForceLinkFunctionCheckpoint();
	ForceLinkFunctionTurretPoint();
	ForceLinkFunctionLookAtInfo();
	ForceLinkFunctionKite();
	ForceLinkFunctionProjectile();
    ForceLinkFunctionRangedWeapon();
	ForceLinkFunctionAnimated();
	ForceLinkFunctionParticleEmitter();
	ForceLinkFunctionCatapult();
	ForceLinkFunctionCatapultRock();
	ForceLinkFunctionTurretWeapon();
	ForceLinkFunctionSpeedGrass();
	ForceLinkFunctionWaterInstanceDef();

	ForceLinkFunctionArmy();

	ForceLinkFunctionArmyMessageHub();

	Mem::Init();

	if( !FwMem::IsInitialised() )
	{
		static FwMemConfig g_projectMemConfig = { Mem::ATG_AllocCallback, Mem::ATG_FreeCallback };
		FwMem::Initialise( &g_projectMemConfig );
	}

#if defined(_PROFILING)
	//snTunerInit();
#endif

	{
		// Create the game
		ShellMain obGame;

		// This is really just validation and it uses new so do a bit later than we used...
		//ALEXEY_TODO:
		//CHashedString::Initialise();
	
		// Update the game
		while( obGame.Update() )
		{
#if !defined( _GOLD_MASTER )
			snSafeLoadPoint();
#endif
		}
	}

	FwMem::Shutdown();

	FileManager::Kill();
	Mem::Kill();

	// If we're going to use sys_process_exit then we won't get static destruction,
	// so make sure all the PRX modules are unloaded before we quit!
	EarlyStaticInit::UnloadModules();

	// ps3run can pick up this string to bail out faster	
	ntPrintf("[[PS3EXIT]]\n");
	// This forces a quit of ps3run
	ntPrintf( "\x1A\n" );
/*
	while ( true )
	{
		sys_timer_usleep( 30 );
	}
*/
	sys_process_exit( 0 );
/*
	const char self_path[] = SYS_DEV_HDD0"/game/hs/USRDIR/hs_rel.self"; // /dev_flash/vsh/module/vsh.self
	const sys_addr_t data = 0;
	const int main_ppu_thread_priority = 1001;
	const uint64_t flags = SYS_PROCESS_PRIMARY_STACK_SIZE_64K;

	sys_game_process_exitspawn( self_path, NULL, NULL, data, 0, main_ppu_thread_priority, flags );
*/
	return 0;
}


