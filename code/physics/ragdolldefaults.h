//**************************************************************************************
//
//	ragdolldefaults.h
//	
//**************************************************************************************

#ifndef RAGDOLLDEFAULTS_H_
#define RAGDOLLDEFAULTS_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	Default filename for the generic ragdoll - different atm for PC/PS3.
//**************************************************************************************
#if defined( PLATFORM_PC )
	static const char *sDefaultRagdollClumpFilename = "data/default.ragdoll.clump";
#elif defined( PLATFORM_PS3 )
	static const char *sDefaultRagdollClumpFilename = "data/default.ragdoll.clump_ps3";
#else
#	error Unknown platform!
#endif

#endif // !RAGDOLLDEFAULTS_H_
