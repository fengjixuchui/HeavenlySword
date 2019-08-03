/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SERIALIZE2_VERSION_REGISTRY_H
#define HK_SERIALIZE2_VERSION_REGISTRY_H

#include <hkbase/class/hkClass.h>
#include <hkserialize/util/hkClassNameRegistry.h>

class hkObjectUpdateTracker;
template <typename T> class hkArray;
template <typename T> class hkStringMap;

/// Manages conversion between sdk versions.
/// Note that the registry has no concept of version numbers being greater
/// or less than one another. It just knows that it may call a function
/// to convert between two string identifiers.
class hkVersionRegistry : public hkSingleton<hkVersionRegistry>
{
	public:

			/// Signature of an update function.
		typedef hkResult (HK_CALL* UpdateFunction)(
			hkArray<hkVariant>& loadedObjects,
			hkObjectUpdateTracker& tracker );

			/// Single entry to update between specified versions.
		struct Updater
		{
			const char* fromVersion;
			const char* toVersion;
			UpdateFunction updateFunction;
		};

		/// Single entry to identify classes for specified version.
		struct ClassList
		{
			const char* version;
			hkClass*const* classes;
		};

			///
		hkVersionRegistry();

			///
		~hkVersionRegistry();

			/// Add an updater to the registry.
			/// Usually updaters are compiled in via StaticLinkedUpdaters, but
			/// dynamically loaded updaters may use this method.
		void registerUpdater( const Updater* updater );

			/// Find the sequence of updaters to convert between given versions.
			/// If there is a path between fromVersion and toVersion write the sequence
			/// of updaters needed into pathOut and return HK_SUCCESS.
			/// If no such path exists, return HK_FAILURE.
		hkResult getVersionPath( const char* fromVersion, const char* toVersion, hkArray<const Updater*>& pathOut ) const;

		const hkClassNameRegistry* getClassNameRegistry( const char* versionString );

	public:

			/// Available updaters.
		hkArray<const Updater*> m_updaters;

		/// Updaters available at compile time.
		static const Updater* StaticLinkedUpdaters[];

		/// List of versions and corresponding classes available at compile time.
		static const ClassList StaticLinkedClassList[];

	private:

		hkStringMap<hkClassNameRegistry*> m_versionToClassNameRegistryMap;
};

#endif // HK_SERIALIZE2_VERSION_REGISTRY_H

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
