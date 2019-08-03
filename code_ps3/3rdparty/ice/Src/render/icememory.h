/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_RENDER_ICEMEMORY_H
#define ICE_RENDER_ICEMEMORY_H

namespace Ice
{
	namespace Render
	{
		struct CommandContext;
		
		U64 static const kMaxRegions = 0x10000;
		U64 static const kMaxTiles = 15;
		U64 static const kMaxDepthCullAreas = 8;
		U64 static const kCompressionTagSize = 2048;
		U64 static const kDepthCullSize = 0x300000; // It is at least this much. Need more information. 
		U32 static const kRegionTypeMask = 0xE0000000;
		U32 static const kRegionStartMask = 0x1FFFFFFF;

		enum RegionType
		{
			kUnused    = 0x00000000,
			kLinear    = 0x20000000,
			kTiled     = 0x40000000,
			kCull      = 0x60000000,
			kDirty     = 0x80000000,
			kLinearIo  = 0xA0000000,
			kTiledIo   = 0xC0000000,
			kTag       = 0xE0000000
		};

		struct Region
		{
			U32 m_typeAndStart;
			U32 m_size;
			Region *m_next;
		};

		struct Tile
		{
			Region *m_owner;
			struct
			{
				void Set(U32 index, U32 offset, TileContext context, U32 size, U32 pitch, CompressionFormat format, U32 base, U32 bank)
				{
					m_index = index;
					m_offset = offset;
					m_context = context;
					m_size = size;
					m_pitch = pitch;
					m_format = format;
					m_base = base;
					m_bank = bank;
				}

				U32 m_index;
				U32 m_offset;
				TileContext m_context;
				U32 m_size;
				U32 m_pitch;
				CompressionFormat m_format;
				U32 m_base;
				U32 m_bank;
			} m_parms;
		};
		
		struct CompressionTags
		{
			Region *m_owner;
			Region *m_tagArea;
		};

		struct DepthCullArea
		{
			Region *m_owner;
			Region *m_cullArea;
			struct
			{
				void Set(U32 index, U32 offset, U32 width, U32 height, U32 start, DepthStencilBufferFormat depthFmt, MultisampleMode ms, DepthCullDirection dir, DepthCullFormat cullFmt, ComparisonFunc func, U32 ref, U32 mask)
				{
					m_index = index;
					m_offset = offset;
					m_width = width;
					m_height = height;
					m_start = start;
					m_depthFmt = depthFmt;
					m_ms = ms;
					m_dir = dir;
					m_cullFmt = cullFmt;
					m_func = func;
					m_ref = ref;
					m_mask = mask;
				}
				
				U32 m_index;
				U32 m_offset;
				U32 m_width;
				U32 m_height;
				U32 m_start;
				DepthStencilBufferFormat m_depthFmt;
				MultisampleMode m_ms;
				DepthCullDirection m_dir;
				DepthCullFormat m_cullFmt;
				ComparisonFunc m_func;
				U32 m_ref;
				U32 m_mask;
			} m_parms;
		};

		extern Tile g_tiles[kMaxTiles];
		extern DepthCullArea g_depthCullAreas[kMaxDepthCullAreas];

		//! Initializes the main and video memory allcoation helpers.
		/*! \param ioBase      The base main memory address. 
		    \param ioSize      The size of the main memory window.
		    \param vmBase      The base video memory address.
		    \param vmSize      The size of the video memory window.
		    \param regions     A pointer to an array of regions for use in allocation management.
		    \param numRegions  The number of regions in the passed in array.
		*/
		void InitializeMemoryAllocators(void *ioBase, U32 ioSize, void *vmBase, U32 vmSize, Region *regions, U32 numRegions);

		//! Queries the future success of a main memory allocation that is viewable by the Gpu.
		/*! \param size  		The size of the memory to allocate, in bytes.
			\param alignment	Alignment (must be a power of two).
		    \return      		True if the allocation will succeed, false otherwise.
		*/
		bool QueryAllocateIoMemory(U32 size, U32 alignment = 0x400);

		//! Allocates main memory that is viewable by the Gpu.
		/*! \param size  The size of the memory to allocate, in bytes.
		    \return      If this function fails, the return value is nullptr.
		*/
		MALLOC_FUNCTION void *AllocateIoMemory(U32 size, U32 alignment = 0x400);
		
		//! Releases main memory that is viewable by the Gpu.
		/*! \param ptr  A pointer to main memory previously returned by the AllocateIoMemory() function.
		*/
		void ReleaseIoMemory(void *ptr);

		//! Allocates tiled main memory that is viewable by the Gpu.
		/*! \param size  The size of the memory to allocate, in bytes. Must be a multiple of 64k.
		    \param pitch The pitch of the tile, in bytes.
		    \param bank  The bank of the tile. 
		    \return      If this function fails, the return value is nullptr.
		*/
		MALLOC_FUNCTION void *AllocateTiledIoMemory(U32 size, U32 pitch, U32 bank);
		
		//! Releases main memory that is viewable by the Gpu.
		/*! \param ptr  A pointer to main memory previously returned by the AllocateTiledIoMemory() function.
		*/
		void ReleaseTiledIoMemory(void *ptr);

		//! Queries the future success of a linear VRAM allocation.
		/*! \param size  		The size of the memory to allocate, in bytes.
			\param alignment	Alignment (must be a power of two).
		    \return      		True if the allocation will succeed, false otherwise.
		*/
		bool QueryAllocateLinearVideoMemory(U32 size, U32 alignment = 0x400);
		
		//! Allocates linear VRAM for vertex/index data, textures, and fragment programs.
		/*! \param size  The size of the memory to allocate, in bytes.
		    \return      If this function fails, the return value is nullptr.
		*/
		MALLOC_FUNCTION void *AllocateLinearVideoMemory(U32 size, U32 alignment = 0x400);
		
		//! Releases linear VRAM.
		/*! \param ptr  A pointer to linear VRAM previously returned by the AllocateLinearVideoMemory() function.
		*/
		void ReleaseLinearVideoMemory(void *ptr);

		//! Allocates tiled VRAM for vertex/index data, textures, and fragment programs.
		/*! \param size  The size of the memory to allocate, in bytes. Must be a multiple of 64k.
		    \param pitch The pitch of the tile, in bytes.
		    \param cfmt  The compression format of the tile.
		    \param bank  The bank of the tile. 
		    \return      If this function fails, the return value is nullptr.
		*/
		MALLOC_FUNCTION void *AllocateTiledVideoMemory(U32 size, U32 pitch, CompressionFormat cfmt, U32 bank);
		
		//! Releases tiled VRAM.
		/*! \param ptr  A pointer to tiled VRAM previously returned by the AllocateTiledVideoMemory() function.
		*/
		void ReleaseTiledVideoMemory(void *ptr);
		
		//! Gets the tiling parameters for a color surface.
		/** \param width        The width, in pixels, of the surface.
		    \param height       The height, in pixels, of the surface.
		    \param format       The pixel format of the surface.
		    \param multisample  The multisampling mode of the surface.
		    \param size         A pointer to a location that receives the size in bytes of the specified surface.
		    \param pitch        A pointer to a location that receives the row pitch in bytes of the specified surface.
		    \param cfmt         A pointer to a location that receives the compression format of the specified surface.
		*/
		void GetColorSurfaceTileParameters(U32 width, U32 height, ColorBufferFormat format, MultisampleMode multisample, U32 *size, U32 *pitch, CompressionFormat *cfmt);
		
		//! Gets the tiling parameters for a depth/stencil surface.
		/** \param width        The width, in pixels, of the surface.
		    \param height       The height, in pixels, of the surface.
		    \param format       The pixel format of the surface.
		    \param multisample  The multisampling mode of the surface.
		    \param size         A pointer to a location that receives the size in bytes of the specified surface.
		    \param pitch        A pointer to a location that receives the row pitch in bytes of the specified surface.
		    \param cfmt         A pointer to a location that receives the compression format of the specified surface.
		    \param cullWidth    A pointer to a location that recieves the depth cull area width of the specified surface.
		    \param cullHeight   A pointer to a location that recieves the depth cull area height of the specified surface.
		*/
		void GetDepthStencilSurfaceTileParameters(U32 width, U32 height, DepthStencilBufferFormat format, MultisampleMode multisample, U32 *size, U32 *pitch, CompressionFormat *cfmt, U32 *cullWidth, U32 *cullHeight);

		//! Allocates VRAM specifically designated for a color surface.
		/** \param width        The width, in pixels, of the color surface.
		    \param height       The height, in pixels, of the color surface.
		    \param format       The pixel format of the color surface.
		    \param multisample  The multisampling mode of the color surface.
		    \param pitch        A pointer to a location that receives the row pitch of the allocated color surface.
		    \return             If this function fails, the return value is nullptr.
		*/
		static inline MALLOC_FUNCTION void *AllocateColorSurfaceVideoMemory(U32 width, U32 height, ColorBufferFormat format, MultisampleMode multisample, U32 *pitch)
		{
			U32 size;
			CompressionFormat cfmt;
			GetColorSurfaceTileParameters(width, height, format, multisample, &size, pitch, &cfmt);
			return AllocateTiledVideoMemory(size, *pitch, cfmt, 0);
		}

		//! Releases color surface VRAM.
		/*! \param ptr  A pointer to color surface VRAM previously returned by the AllocateColorSurfaceVideoMemory() function.
		*/
		static inline void ReleaseColorSurfaceVideoMemory(void *ptr)
		{
			ReleaseTiledVideoMemory(ptr);
		}

		//! Allocates a depth cull area for the specified parameters.
		/*! \param vmBasePtr    A pointer to a video memory address where the depth cull area should be allocated to.
		    \param width        The width of the depth cull area being allocated. This is typically equal to the depth buffer width.
		    \param height       The height of the depth cull area being allocated. This is typically equal to the depth buffer height.
		    \param multisample  The multisampling mode of the depth/stencil surface.
		    \return             This function returns true if the depth cull area was sucessfully allocated.
		*/
		bool AllocateDepthCullArea(const void *vmBasePtr, U32F width, U32F height, DepthStencilBufferFormat format, MultisampleMode multisample);

		//! Releases a depth cull area associated with the passed in pointer.
		/*! \param vmBasePtr  A pointer to a video memory address where the depth cull area should be released from.
		    \return           This function returns true if the depth cull area was sucessfully released.
		*/
		bool ReleaseDepthCullArea(const void *vmBasePtr);

		//! Allocates VRAM specifically designated for a depth/stencil surface.
		/*! \param width        The width, in pixels, of the depth/stencil surface.
		    \param height       The height, in pixels, of the depth/stencil surface.
		    \param format       The pixel format of the depth/stencil surface.
		    \param multisample  The multisampling mode of the depth/stencil surface.
		    \param pitch        A pointer to a location that receives the row pitch of the allocated depth/stencil surface.
		    \return             If this function fails, the return value is nullptr.
		*/
		static inline MALLOC_FUNCTION void *AllocateDepthStencilSurfaceVideoMemory(U32 width, U32 height, DepthStencilBufferFormat format, MultisampleMode multisample, U32 *pitch)
		{
			U32 size;
			CompressionFormat cfmt;
			U32 cullWidth, cullHeight;
			GetDepthStencilSurfaceTileParameters(width, height, format, multisample, &size, pitch, &cfmt, &cullWidth, &cullHeight);
			
			void *ptr = AllocateTiledVideoMemory(size, *pitch, cfmt, 1);
			if(ptr != NULL)
			{
				AllocateDepthCullArea(ptr, cullWidth, cullHeight, format, multisample);
			}
			return ptr;
		}

		//! Release depth/stencil surface VRAM.
		/*! \param ptr  A pointer to depth/stencil surface VRAM previously returned by the AllocateDepthStencilSurfaceVideoMemory() function.
		*/
		static inline void ReleaseDepthStencilSurfaceVideoMemory(void *ptr)
		{
			ReleaseDepthCullArea(ptr);
			ReleaseTiledVideoMemory(ptr);
		}

		//! Get the size of a previous allocation.
		/*! \param ptr  A pointer to the memory returned by a previous allocation using one of the Allocate...Memory() functions.
		    \param type	Memory region type - either kLinear, kTiled, kLinearIo, or kTiledIo.
			\return     Byte size of the allocation or zero if this function fails.
        */
		U32 GetAllocationSize(void *ptr, RegionType type);
	}
}

#endif // ICE_RENDER_ICEMEMORY_H
