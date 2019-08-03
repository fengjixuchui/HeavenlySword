/***************************************************************************************************
*
*	Physics representation loading
*
*	CHANGES
*
*	8/4/2004		Ben		Created
*
***************************************************************************************************/

#ifndef	_PHYSICSLOADER_H
#define	_PHYSICSLOADER_H

#include <hkbase\config\hkConfigVersion.h>
#include "physics/havokincludes.h"

#include <hkbase/hkBase.h>
#include <hkdynamics/world/hkPhysicsSystem.h>
#include <hkserialize/util/hkLoader.h>
#include "collisionbitfield.h"
#include "logicgroup.h"
#include <hkcollide/shape/hkShape.h>

const uint32_t INVALID_MATERIAL = 0;

namespace Physics
{
	class CPhysicsLoader;
	class PhysicsDataRef;

	class PhysicsData
	{
	public:
		PhysicsData(const CHashedString& name) : m_physicsSystem(0), m_name(name), m_refCount(0)
			 {};


		struct CloningParams
		{
			EntityCollisionFlag m_collisionFlags;
			FilterExceptionFlag m_filterExceptionFlags;
			bool m_static;
			bool m_largeSmallInteractable;

			CloningParams() : m_static(false), m_largeSmallInteractable(false) 
			{ 
				m_collisionFlags.base = 0;
				m_filterExceptionFlags.base = 0; 
			};
		};

		void CloneIntoLogicGroup(LogicGroup& dest, const CloningParams& params) const;

        // BE WARE!!! It is recomendet to fill logic group by CloneIntoLogicGroupUse
		// The following two function can be used in special cases as ragdolls... 
		const hkPhysicsSystem& GetPhysicsSystem() const {return *m_physicsSystem;};
		typedef ntstd::Vector<uint32_t, MC_PHYSICS> TransformHashes;
		const TransformHashes& GetTransformHashes() const {return m_transfromHash;};

	protected:		
		void AddRef() {m_refCount++;};
		void Release();

		hkLoader m_loader; // all memory allocated for loaded packfile is in loader.
		hkPhysicsSystem * m_physicsSystem;
		TransformHashes m_transfromHash;

		CHashedString m_name; // name of the source file
		int m_refCount;  // reference counter

		friend class CPhysicsLoader;
		friend class PhysicsDataRef;
	};



	// Loader singleton its self
	class CPhysicsLoader : public Singleton< CPhysicsLoader >
	{
	public:
		CPhysicsLoader() {};
		~CPhysicsLoader() {ntAssert(m_cache.size() == 0);};

		void Clear();

		// This will return the anim data for the specified anim file. If the file is already loaded,
		// it'll return the exisiting pointer - the file won't be reloaded.
		PhysicsData* LoadPhysics_Neutral	( const char *pNeutralName );
		PhysicsData* LoadPhysics_Platform	( const char *pPlatformName );

		// Retrieve the platform specific name for this physics file
		static void	MakePlatformPhysicsName( const char* pNeutralName, char* pPlatformName );

		// Change the extension of the filename to physics one
		static ntstd::String AlterFilenameExtension( const char * const filename );

		// unload physics file ... (basically remove it from memory)
		void Unload(const CHashedString & name);

	private:
#ifdef BINARY_PHYSICS_LOADER
		static void FinishShape(hkShape * shape);
#endif

		typedef ntstd::Map< uint32_t, PhysicsData * >	CacheMap;

		CacheMap	m_cache;
	};
};

#endif	//_PHYSICSLOADER_H
