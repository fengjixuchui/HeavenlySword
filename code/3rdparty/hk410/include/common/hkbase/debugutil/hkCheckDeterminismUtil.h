/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKBASE_HKDEBUGUTIL_CHECK_DETERMINISM_UTIL_H
#define HKBASE_HKDEBUGUTIL_CHECK_DETERMINISM_UTIL_H


extern struct hkCheckDeterminismUtil g_checkDeterminismUtil;


class hkIstream;
class hkOstream;

	// set the next line to 1 if you want to check the engine for determinism
#define HK_CHECK_DETERMINISM_ENABLE_ST_CHECKS 0

	// not used
#define HK_CHECK_DETERMINISM_ENABLE_MT_CHECKS 0

	/// This is a small helper class allowing to check the determinism of
	/// a system. Basically it works by running an application twice:
	/// First a binary logfile is created and the second time the system
	/// checks against this binary logfile.
struct hkCheckDeterminismUtil
{
	public:

		 hkCheckDeterminismUtil();
		~hkCheckDeterminismUtil();

		static hkCheckDeterminismUtil& getInstance() { return g_checkDeterminismUtil; }

			/// sets this utility to write mode. Call at startup of your test
			/// Make sure to call finish() at the end
		void startWriteMode(const char* filename = "hkDeterminismCheckfile.bin");

			/// sets this utility to check mode. Call at startup of your test
			/// Make sure to call finish() at the end
		void startCheckMode(const char* filename = "hkDeterminismCheckfile.bin");

		/// check an array of objects from a single threaded section of your program
		template<typename TYPE>
		static HK_FORCE_INLINE void HK_CALL checkSt( TYPE* object, int numObjects = 1 )
		{
			if ( HK_CHECK_DETERMINISM_ENABLE_ST_CHECKS )
			{
				getInstance().checkImpl( object, sizeof( TYPE ) * numObjects );
			}
		}

		/// check a simple type object from a single threaded section of your program
		template<typename TYPE>
		static HK_FORCE_INLINE void HK_CALL checkSt( TYPE object )
		{
			if ( HK_CHECK_DETERMINISM_ENABLE_ST_CHECKS )
			{
				getInstance().checkImpl( &object, sizeof( TYPE ));
			}
		}

		/// check an array of objects in a multi threaded environment
		template<typename TYPE>
		static HK_FORCE_INLINE void HK_CALL checkMt( TYPE* object, int numObjects = 1 )
		{
			if ( HK_CHECK_DETERMINISM_ENABLE_ST_CHECKS )
			{
				getInstance().checkImpl( object, sizeof(TYPE ) * numObjects );
			}
		}

		/// check a simple type object in a multi threaded environment
		template<typename TYPE>
		static HK_FORCE_INLINE void HK_CALL checkMt( TYPE object )
		{
			if ( HK_CHECK_DETERMINISM_ENABLE_ST_CHECKS )
			{
				getInstance().checkImpl( &object, sizeof(TYPE ));
			}
		}


		void checkImpl(const void* object, int size);

			/// Call this function at the end of your write/check run. This closes the open files
		void finish();

	protected:
		hkIstream *m_inputStream;
		hkOstream *m_outputStream;
};


#endif // HKBASE_HKDEBUGUTIL_CHECK_DETERMINISM_UTIL_H

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
