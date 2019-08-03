#ifndef _SPEEDTREEMANAGER_H_
#define _SPEEDTREEMANAGER_H_

//--------------------------------------------------
//!
//!	\file SpeedTreeManager.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------


//#include "core/explicittemplate.h"
//#include "gfx/graphicsdevice.h"
//#include "gfx/rendercontext.h"
//#include "camera/camman_public.h"
//#include "anim/transform.h"
//#include "core/exportstruct_clump.h"

#include "speedtree/SpeedTreeUtil_ps3.h"
#include "speedtree/SpeedTreeShaders_ps3.h"
#include "game/commandresult.h"
#include "core/nt_std.h"
#include "core/bitmask.h"

class CSpeedTreeForest;
class SpeedTreeXmlForest;
class CTreeRegister;
class CSpeedTreeWrapper;
class SpeedTreeXmlTree;

class SpeedTreeManager: public Singleton<SpeedTreeManager>
{
	friend class CSpeedTreeForest;
public:
	// speedtree tweak
	class SpeedTreeGlobalData
	{
	public:
		// global settings
		int		m_nNumWindMatrices;
		float	m_fNearLodFactor;
		float	m_fFarLodFactor;
		Vec3	m_afWindDirection;
	public:
		// ctor
		SpeedTreeGlobalData()
			:m_nNumWindMatrices(4)
			,m_fNearLodFactor(1.0f)
			,m_fFarLodFactor(15.0f)
			,m_afWindDirection (1.0f, 0.0f, 0.0f)
		{
			// nothing
		}
	};

	// speedtree observer
	struct SpeedTreeObserver
	{
		CPoint m_camPos;
		CDirection m_camDir;
		CPoint m_lightPos;
		CDirection m_lightDir;
	};

	// animate flags
	typedef enum
	{
		SPEEDTREE_TOGGLE_RENDER = BITFLAG(0),
		SPEEDTREE_TOGGLE_GAME_UPDATE = BITFLAG(1),
		SPEEDTREE_TOGGLE_VIEWPORT_UPDATE = BITFLAG(2),
		SPEEDTREE_TOGGLE_DEBUG_RENDER = BITFLAG(3),
		SPEEDTREE_M_TOGGLE_ALL_ON = BITFLAG(4)-1,
	} Speedtree_toggle;


public:
	//! constructor
	SpeedTreeManager();
	
	//! destructor
	~SpeedTreeManager();


	// set speedtree camera and dirty all the geometry
	static void SetSpeedtreeCamera(const CPoint& dir, const CDirection& pos);
	void SetSpeedtreeCameraToCamera(bool bForBillboard = false);
	void SetSpeedtreeCameraToLight(bool bForBillboard = false);

	// add remove forest
	void AddForest(const SpeedTreeXmlForest *);
	void RemoveForest(const SpeedTreeXmlForest *);

	// load an xml file
	void LoadXmlFile(const ntstd::String& xmlFilename);

	// ready for rendering from light
	void UpdateForLight();

	// ready for rendering from camera
	void UpdateForCamera();
	
	// game update
	void PerFrameUpdate(float fElapsedTime);

	// viewport update
	void PerViewportUpdate();
	
	// debug rendering: stat
	void DebugRender();
	
	// toggle speedtree renderable
	COMMAND_RESULT CommandToggleRender(const int& kind);

	// get render bit
	uint32_t GetRenderBitVector();

	// register key
	void RegisterKey();

	// get state
	SpeedTreeStat GetStat();

	// toggle speedtree renderable
	COMMAND_RESULT CommandToggleSpeedtree(const int& kind);
	
	// get speedtree global data
	const SpeedTreeGlobalData& GetGlobalData() const {return m_speedtreeData;}

	// speedtree data path
	const ntstd::String& GetDataPath() const {return m_speedTreeFolder;}

	// get observer
	const SpeedTreeObserver& GetObserver() const {return m_speedtreeObserver;}

	// get accum time
	float GetAccumTime() const {return m_fAccumTime;}

	bool GetResourcePath(const char* originalName, char* speedTreeName);

	void ResetRendering();
	void FinishRendering();
	void OnLevelUnload();

	void OnAreaLoad(CHashedString	resHash);
	void OnAreaUnload(CHashedString	resHash);

	unsigned int GetVRAMFootprint(unsigned int& vertexSize, unsigned int& indexSize, unsigned int& textureSize);

	COMMAND_RESULT DumpStats();

private:
	CSpeedTreeWrapper*  GetUniqueTree(const SpeedTreeXmlTree* treeDef);
	void				AddUniqueTree(CSpeedTreeWrapper* tree, const SpeedTreeXmlTree* treeDef);

protected:
	// forests container
	typedef ntstd::Map<const SpeedTreeXmlForest*,CSpeedTreeForest*> Forests;
	Forests m_forests;
	
	// time
	float m_fAccumTime;

	//render bit
	uint32_t m_ulRenderBitVector;
	
	// data folder
	ntstd::String m_speedTreeFolder;
	
	// animate flags
	BitMask<Speedtree_toggle> m_speedtreeToggle;
	
	// speedtree tweak
	SpeedTreeGlobalData m_speedtreeData;

	// camera and light directiopn
	SpeedTreeObserver	m_speedtreeObserver;
	CTreeRegister*		m_treeRegister;


}; // end of class SpeedTreeManager

#endif // end of _SPEEDTREEMANAGER_H_
