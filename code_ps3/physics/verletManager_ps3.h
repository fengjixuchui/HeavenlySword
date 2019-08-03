#ifndef _VERLETMANAGER_PS3_H_
#define _VERLETMANAGER_PS3_H_



//--------------------------------------------------
//!
//!	\file verletManager.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "core/bitmask.h"
#include "game/commandresult.h"

class Transform;
class CMatrix;
class GlobalWind;
class Template_Flag;

namespace Physics
{
	class VerletInstance;
	class VerletInstanceDef;
	class VerletMaterialInstanceDef;
	class VerletGameLink;
	class RenderableVerletSystem;


	//--------------------------------------------------
	//!
	//!	\file verletManager.h
	//!	keep track of all the verlet systems, take care about the rendering too
	//!
	//--------------------------------------------------
	class VerletManager: public Singleton<VerletManager>
	{
	public:
		typedef enum
		{
			RESET_SIMULATION = BITFLAG(0),
			SPU_BREAKPOINT = BITFLAG(1),
			SPU_BREAKPOINT_AUX = BITFLAG(2),
			DEBUG_RENDERING = BITFLAG(3),
			RENDERING = BITFLAG(4),
			IS_PAUSED = BITFLAG(5),
		} Flags;
	private:
		// verlet list
		typedef ntstd::List<Physics::VerletInstance*> VerletList;
		VerletList m_verletList;

		// flags
		BitMask<Flags> m_flags;

		// standard falga material binder (lambert1n double sided)
		VerletGameLink* m_pGameLink;
	public:
		// get game link
		const VerletGameLink& GetGameLink() const {return *m_pGameLink;}
		// get flags
		bool GetFlags(Flags f) {return m_flags[f];}
		//! constructor
		VerletManager();
		//! destructor
		~VerletManager();
		//! reset
		void Reset();
		// toggle flags
		COMMAND_RESULT CommandToggleFlags(const int& kind);
		// update (doesn't so much for now)
		void Update(float fTimeDelta);
		//register a verlet
		void Register(VerletInstance*);
		void Unregister(VerletInstance*);

		/////////////////////////////////////////
		// creating flags
		VerletInstance* CreateASimpleGenericVerletSystem(Transform* pParentTransform, Template_Flag* pTemplate = 0);
		VerletInstance* CreateASimpleGenericVerletSystemWithOffset(Transform* pParentTransform, const CMatrix& offset);

		/////////////////////////////////////////
		// rendering command
		void RendererUpdate();

		// Default Global Wind accessor
		GlobalWind* GetDefaultGlobalWind() const {return m_pobDefaultGlobalWind;}

		// Is the system enabled?
		static bool IsEnabled() {return m_gVerletEnable;}

		// Set the enabled state
		static void SetEnabled(bool bEnabled) {m_gVerletEnable = bEnabled;}


		// Register a new GlobalWind Object
		void AddWind(GlobalWind* pobWind)
		{
			// Validate pointer
			ntAssert(pobWind);
			// Add to list
			m_obWindList.push_back(pobWind);
		}

		// Register a new GlobalWind Object
		void RemoveWind(GlobalWind* pobWind)
		{
			// Validate pointer
			ntAssert(pobWind);
			// Remove from list
			m_obWindList.remove(pobWind);
		}

		// Get gravity value to be used by verlet systems
		CVector GetGravity() const {return m_obGravity;}

	private:

		static bool m_gVerletEnable;

		// procedural (eg: no artist control) rectangular flags
		VerletInstance* CreateRectangularFlag(Transform* pParentTransform, const VerletInstanceDef& def, const VerletMaterialInstanceDef& matDef);
		// DebugRendering of all verlets
		void DebugRender();
		// register key
		void ResetPosition();
		// register key
		void RegisterKey();
		// default global wind
		GlobalWind*	m_pobDefaultGlobalWind;
		// List of all wind objects
		typedef ntstd::List<GlobalWind*> WindList;
		ntstd::List<GlobalWind*> m_obWindList;  

		// The gravity values used by all verlets
		CVector m_obGravity;

	}; // end of class VerletSingleton
}


#endif // end of _VERLETMANAGER_PS3_H_
