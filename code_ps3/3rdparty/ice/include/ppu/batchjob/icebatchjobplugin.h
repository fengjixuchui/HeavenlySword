/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_BATCHJOBPLUGIN_H
#define ICE_BATCHJOBPLUGIN_H

#include "icebatchjob.h"

namespace Ice {
	namespace BatchJob {

		/*!	Plugin encapsulates a binary buffer in a batch job compatible plugin format,
		 *	which has a 3 quadword header, including a 2 quadword SN systems debugging header
		 *	and a 1 quadword Plugin header, plus following code and data.  Plugin
		 *	includes constructor methods to load a plugin from a file or to create one from a buffer
		 *	already in memory (statically linked or dynamically loaded by another method).
		 *
		 *	Batch job compatible plugins are generally linked with ice/batchjob/plugin/batchjob_plugincrt0,
		 *	which implements BatchJobPluginLoad() to store g_batchJobFunctionTable, zero the BSS section,
		 *	and call all global constructors, BatchJobPluginUnload() to call all global destructors, and
		 *	BatchJobPluginExecute() to set the position independent code register ($126) and then call
		 *	BatchJobPluginMain(), passing through arguments.
		 *
		 *	From a plugin programmer's standpoint, then, BatchJobPluginMain() is the entry point,
		 *	which must implement the Ice::BatchJob::DispatcherFunction prototype with position independent
		 *	code.  See ice/batchjob/plugin/Makefile for an example of how to build a batch job compatible
		 *	plugin from your source code.
		 *
		 *	To allow transparent usage of a Plugin in PPU batched mode, space is reserved
		 *	in the header for the runtime to set a PPU function to mimic calls to BatchJobPluginMain()
		 *	on the PPU.  The SetPpuBatchModeFunction() and GetPpuBatchModeFunction() methods are provided
		 *	to access this entry in the header.
		 */
		class Plugin
		{
			U8 const*			m_pData;
			U32					m_size;
			U32					m_flags;

			struct Header
			{
				U32		m_SNGUID0;		//SN systems header: GUID[0] (should be ila $2, (GUID.hword[0] << 2) | 0x0 == 0x42000002 | (GUID.hword[0] << 9))
				U32		m_SNGUID1;		//SN systems header: GUID[1] (should be ila $2, (GUID.hword[1] << 2) | 0x1 == 0x42000082 | (GUID.hword[0] << 9))
				U32		m_SNGUID2;		//SN systems header: GUID[2] (should be ila $2, (GUID.hword[2] << 2) | 0x2 == 0x42000102 | (GUID.hword[0] << 9))
				U32		m_SNGUID3;		//SN systems header: GUID[3] (should be ila $2, (GUID.hword[3] << 2) | 0x3 == 0x42000182 | (GUID.hword[0] << 9))

				U32		m_entryPoint;	//SN systems header: offset of BatchJobPluginExecute() function
				U32		m_bssOffset;	//SN systems header: offset to start of BSS section
				U32		m_bssSize;		//SN systems header: size of BSS section (which is created and zeroed by BatchJobPluginLoad() after upload)
				U32		m_flags;		//SN systems header: debugger flags == 0x40

				U32		m_loadEntryPoint;						// Plugin header: offset of BatchJobPluginLoad() function
				U32		m_unloadEntryPoint;						// Plugin header: offset of BatchJobPluginUnload() function
				DispatcherFunction m_pPpuBatchModeFunction;		// Plugin header: == 0, but may be set to a PPU equivalent function to BatchJobPluginMain()
				U32		m_version;								// Plugin header: == 1, version number of this Header (i.e. of the linker script used to build it)
			};

			static const unsigned kSnHeaderGuidShift = 9;
			static const U32 kSnHeaderGuidMask = (0xFFFF << kSnHeaderGuidShift);
			static const U32 kSnHeaderIla0 = 	0x42000002;
			static const U32 kSnHeaderIla1 = 	0x42000082;
			static const U32 kSnHeaderIla2 = 	0x42000102;
			static const U32 kSnHeaderIla3 = 	0x42000182;

			// disallow copy, as we'd be unable to track ownership of m_pData without complicated reference counting
			Plugin( Plugin const& ) : m_pData(NULL), m_size(0), m_flags(0) {}
			Plugin const& operator=( Plugin const& ) { return *this; }

		public:
			/// Construct a default (NULL) Plugin.
			Plugin() : m_pData(NULL), m_size(0), m_flags(0) {}
			/// Construct a Plugin from an existing data buffer; Asserts that the buffer is a valid plugin.
			/// NOTE: Plugin doesn't quite treat pData as const, as if you call SetPpuBatchModeFunction(),
			/// it stores a pointer into a 4 byte space in the header reserved by the plugin linker script.
			Plugin( void const* pData, U32 size );

			/// Returns a pointer to this plugin's binary data
			void const* GetData() const { return m_pData; }
			/// Returns the size of this plugin's binary data in bytes which is the size that must be DMA'd;
			/// NOTE: for PPU batched mode plugins, only 48 bytes is actually allocated,
			/// though GetDataSize() still returns the spoofed size of the SPU plugin.
			U32 GetDataSize() const { return m_size; }

			/// Returns the SN systems GUID of this plugin, extracted from the SNHeader
			U64 GetSnGuid() const;

			/// Returns the byte offset of the entry point to the BatchJobPluginExecute() function
			U32 GetEntryPoint() const { return ((Header const*)m_pData)->m_entryPoint; }
			/// Returns the offset to the BSS section for this plugin.
			U32 GetBssOffset() const { return ((Header const*)m_pData)->m_bssOffset; }
			/// Returns the size of the BSS section for this plugin.  The BSS section is a
			/// data section that is zeroed when the plugin is loaded.
			U32 GetBssSize() const { return ((Header const*)m_pData)->m_bssSize; }
			/// Returns the SN systems header flags
			U32 GetFlags() const { return ((Header const*)m_pData)->m_flags; }

			/// Returns the byte offset of the entry point to the BatchJobPluginLoad() function
			U32 GetLoadEntryPoint() const { return ((Header const*)m_pData)->m_loadEntryPoint; }
			/// Returns the byte offset of the entry point to the BatchJobPluginUnload() function
			U32 GetUnloadEntryPoint() const { return ((Header const*)m_pData)->m_unloadEntryPoint; }
			/// Returns a pointer to the PpuBatchModeFunction set with SetPpuBatchModeFunction()
			DispatcherFunction GetPpuBatchModeFunction() const { return ((Header const*)m_pData)->m_pPpuBatchModeFunction; }
			/// Returns the linker script version of this plugin
			U32 GetVersion() const { return ((Header const*)m_pData)->m_version; }

			/// Returns the offset to the uninitialized data section for this plugin,
			/// which lies between the end of the binary and the start of the BSS section.
			U32 GetUninitializedDataOffset() const { return m_size; }
			/// Returns the size of the uninitialized data section for this plugin
			U32 GetUninitializedDataSize() const { return GetBssOffset() - m_size; }
			/// Returns the SPU local store size required for this plugin, which includes its
			/// uploaded code and data plus the size of its uninitialized and BSS sections.
			U32 GetLocalStoreSize() const { return GetBssOffset() + GetBssSize(); }

			/// Returns true if this plugin has a valid buffer with a valid SN systems header
			bool IsValid() const;

			/// In PPU batch mode, we replace the SPU call to BatchJobPluginMain with an PPU call
			/// to the function specified to SetPpuBatchModeFunction(), which is stored to an unused
			/// U32 of data in the plugin header.  The arguments passed to this function will be
			/// identical to the arguments passed to the SPU function.
			/// NOTE: PPU batch mode has no need to spoof the SPU calls to BatchJobPluginLoad or
			/// BatchJobPluginUnload.
			void SetPpuBatchModeFunction( DispatcherFunction pPpuBatchModeFunction ) { if (IsValid()) { ((Header*)m_pData)->m_pPpuBatchModeFunction = pPpuBatchModeFunction; } else { ICE_ASSERT(IsValid()); } }
		};

	}
}

#endif
