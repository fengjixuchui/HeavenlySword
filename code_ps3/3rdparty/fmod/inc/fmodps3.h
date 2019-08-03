/* ========================================================================================= */
/* FMOD PS3 Specific header file. Copyright (c), Firelight Technologies Pty, Ltd. 1999-2006. */
/* ========================================================================================= */

#ifndef _FMODPS3_H
#define _FMODPS3_H

#include "fmod.h"


/*
[STRUCTURE] 
[
    [DESCRIPTION]
    Use this structure with System::init to set the information required for ps3
    initialisation.

    Pass this structure in as the "extradriverdata" parameter in System::init.

    [REMARKS]
    force5point1 and attenuateDDLFE are to address issues in the Sony audio libraries.
    These should always be set to 1 until the issues have been resolved by Sony.

    [PLATFORMS]
    PS3

    [SEE_ALSO]
    System::init
]
*/
typedef struct FMOD_PS3_EXTRADRIVERDATA
{
    const void *spu_mixer_elfname_or_spursdata;    /* Path and name of the FMOD SPU mixer self file. If using SPURS, this is a pointer to the SPURS elf data.*/
    const void *spu_streamer_elfname_or_spursdata; /* Path and name of the FMOD SPU stream self file.  If using SPURS, this is a pointer to the SPURS elf data.*/
    int         spu_priority_mixer;                /* SPU thread priority of the mixer. (highest: 16, lowest: 255, default: 16). Ignored if using SPURS. */
    int         spu_priority_at3;                  /* SPU thread priority of the AT3 decoder (highest: 16, lowest: 255, default: 200) */
    int         spu_priority_streamer;             /* SPU thread priority of the MPEG decoder (highest: 16, lowest: 255, default: 200). Ignored if using SPURS. */

    void        *spurs;                            /* Pointer to SPURS instance. (Set this to NULL if not using SPURS) */

    void        *rsx_pool;                         /* Pointer to start of RSX memory pool */
    unsigned int rsx_pool_size;                    /* Size of RSX memory pool to use */

    int         force5point1;                      /* Force output to 5.1 if 7.1 is detected. (Please refer to PS3 section of FMOD documentation) */
    int         attenuateDDLFE;                    /* Attenuate LFE channel of Dolby Digital output by -10dB. (Please refer to PS3 section of FMOD documentation)*/

} FMOD_PS3_EXTRADRIVERDATA;

#endif
