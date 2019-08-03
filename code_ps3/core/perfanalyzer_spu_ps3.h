#ifndef	_PERFANALYZER_SPU_PS3_H
#define	_PERFANALYZER_SPU_PS3_H


#define PABM_SPU_WATER			(0x0001)
#define PABM_SPU_BATCH_RENDERER (0x0002)
#define PABM_SPU_RENDERER		(0x0003)
#define PABM_SPU_FLAGS			(0x0004)
#define PABM_SPU_ARMY			(0x0005)
#define PABM_SPU_ANIM			(0x0006)
#define PABM_SPU_BLENDSHAPES	(0x0007)
#define PABM_SPU_CLIPPER		(0x0008)
#define PABM_SPU_UPDATESKIN		(0x0009)
#define PABM_SPU_HAIR			(0x000A)
#define PABM_SPU_ARMY2			(0x000B)

#define INSERT_PA_BOOKMARK(bookmark) __asm__ volatile ("wrch $ch69, %0" :: "r" ((uint32_t)bookmark));

#endif 
