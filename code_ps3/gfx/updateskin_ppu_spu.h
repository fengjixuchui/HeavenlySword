/***************************************************************************************************
*
* Defines and constants that control the flow of PPU and SPU updateskin module code
*
***************************************************************************************************/

#ifndef UPDATESKIN_PPU_SPU_H_
#define UPDATESKIN_PPU_SPU_H_

#define UPDATESKIN_SPU_ELF	"updateskin_spu_ps3.mod"

#include "core/doubleenderframeallocator.h"

class SPUTask;

// that's just a stupid container class
class SpuUpdateSkin 
{
public:

	struct HierarchyData
	{
		uint32_t m_ui32TransformCount;
		const void* m_pSkinToBoneArray;
		const void* m_pTransformArray;
		const void* m_pSkinMatrixArray;
	};

	static void Initialise(void);
	static void Destroy(void);
	static void UpdateSkin(void);
	static void Sync(void);

private:
	static void PrepareSPUData( void* _visibleRenderables );	

private:
	static bool							m_bInitialized;
	static bool							m_bSync;

	static uint32_t						maxUpdateCount;
	static uint32_t						currentHierarchy;
	
	static SPUTask*						m_pUpdateSkinTask;
	static HierarchyData*				m_pHierarchyData;

	static uintptr_t					m_pJobsMemory;
	static DoubleEnderFrameAllocatorC	m_pDoubleBufferedJobsMem;
};

#endif //UPDATESKIN_PPU_SPU_H_

