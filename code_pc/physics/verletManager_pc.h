#ifndef _VERLETMANAGER_PC_H_
#define _VERLETMANAGER_PC_H_



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
	class RenderableVerletSystem;
	class VerletInstance;

	//--------------------------------------------------
	//!
	//!	\file verletManager.h
	//!	keep track of al the verlet, take care about the rendering too
	//!
	//--------------------------------------------------
	class VerletManager: public Singleton<VerletManager>
	{
	public:
		typedef enum
		{
			RENDERING = BITFLAG(4),
			IS_PAUSED = BITFLAG(5),
		} Flags;
	private:
		// flags
		BitMask<Flags> m_flags;
		//VerletGameLink* m_pGameLink;
	public:
		// get flags
		bool GetFlags(Flags f) {return m_flags[f];}
		//! constructor
		VerletManager();
		//! destructor
		~VerletManager();
		//! reset
		void Reset() {}
		// toggle flags
		COMMAND_RESULT CommandToggleFlags(const int& kind);
		// update (doesn't so much for now)
		void Update( float );
		// register key
		void RegisterKey();

		/////////////////////////////////////////
		// creating flags
		VerletInstance* CreateASimpleGenericVerletSystem(Transform* pParentTransform, Template_Flag* pTemplate = 0);
		VerletInstance* CreateASimpleGenericVerletSystemWithOffset(Transform* pParentTransform, const CMatrix& offset);

		// rendering command
		void RendererUpdate();

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

	private:

		// Enable the verlet system or not
		static bool m_gVerletEnable;

		// List of all wind objects
		typedef ntstd::List<GlobalWind*> WindList;
		ntstd::List<GlobalWind*> m_obWindList; 

	}; // end of class VerletSingleton
}


#endif // end of _VERLETMANAGER_PC_H_
