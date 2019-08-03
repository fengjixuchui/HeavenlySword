/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */


#ifndef ICE_MATERIAL_H
#define ICE_MATERIAL_H

#include "icebase.h"
#include "icemeshfrontend.h"

namespace Ice
{
	// Forward declarations
	namespace Cg
	{
		struct Parameter;
	}
	namespace Fx
	{
		struct Effect;
		struct Technique;
	}

	namespace Graphics
	{
		// Note - at load time m_parameter will contain a pointer to a null-terminated
		// "v:parameter_name" or "f:parameter_name" string (depending on whether it is a vertex
		// program or fragment program parameter). The loading code will fix this up to a
		// parameter pointer.
		struct ParameterBinding // 4-byte aligned
		{
			U32                   m_valueSource; ///< If material param, byte offset from start of material instance. If env param, env source enum.
			const Cg::Parameter   *m_parameter;
		};

		//! The mesh processing front-end function to call on this technique.
		enum MeshProcessingFrontEndFunction
		{
			kRenderGeneralPmVertexSet,
			kRenderGpuVertexSet,
			kRenderBasicVertexSet,
			kRenderDiscretePmVertexSet,
			kRenderContinuousPmVertexSet,
			kRenderDmVertexSet
		};

		// Note - at load time m_fxTechnique will contain a pointer to a null-terminated string
		// containing the name of the technique. The loading code will fix this up to the
		// technique pointer.
		struct MaterialTechnique    // 16-byte aligned
		{
			U16                                     m_materialParameterCount;           ///< Size of material parameter binding table.
			U16                                     m_envParameterCount;                ///< Size of environment parameter binding table.
			U8                                      m_meshProcessingFrontEndFunction;   ///< Uses the MeshProcessingFrontEndFunction enum.
			U8                                      m_attributeArrayFormatCount;        ///< Size of attribute array format table.
			U16                                     m_pad;
			U32                                     m_deltaStreamUsageBits;             ///< Delta streams used (high halfword has the count).
			U16                                     m_gpuSkinningConstantIndex;         ///< Vertex Program constant index of Skinning Matrices.
			U16                                     m_meshProcessingFlags;              ///< Uses Mesh::ProcessingFlags enum.
			Mesh::VertexInputInfo					m_vertexInputInfo;                  ///< Information on GPU vertex input mappings.
			const Fx::Technique                     *m_fxTechnique;                     ///< Pointer to ICEFX technique used.
			union {
				U16                                 *m_meshProcCommandList;             ///< Command list (for custom mesh program).
				Mesh::MeshProgramInfo				*m_meshProcInfo;                    ///< Mesh program info (for standard mesh processing).

			};
			ParameterBinding                        *m_materialParameterBindingTable;   ///< Material parameter binding table.
			ParameterBinding                        *m_envParameterBindingTable;        ///< Environment parameter binding table.
			Mesh::AttributeArrayFormatInfo			*m_attributeArrayFormatTable;       ///< Attribute format table, sorted by hardware index!
			U32                                     m_padding;
		};

		enum
		{
			kVariationNotPresent = 0xFF
		};

		// At load time m_fxEffect will contain an index into a table of unique effect binary
		// filenames (this pointer should not be written as patchable in the icewriter). The
		// loading code will fix this up after creating the effect structs.
		struct MaterialDescriptor   // 16-byte aligned
		{
			Mesh::FixedFormatInfo           m_fixedFormatInfo;
			U8                              *m_variationMappingTable;   ///< Variation -> technique table column (kVariationNotPresent if none).
			const Fx::Effect                *m_fxEffect;                ///< Pointer to ICEFX Effect used.
			U32                             m_techniqueCount;           ///< Number of techniques.
			U32                             m_variationCount;           ///< Variations used (columns in instance technique tables).
			U32                             m_pad;
			MaterialTechnique               m_techniques[0];            ///< Table of techniques.
		};

		// This struct is used in the material instance's technique table. Each of these
		// represents a given variation and LOD level (if shader LOD is used). The count and
		// mapping of variations (which represent the columns of the table) are found in the
		// material descriptor. If shader LOD is used, each row of the table represents a LOD
		// level. If shader LOD is not used, then row 0 is used for non-DM cases and row 1 is
		// used for DM (if present). Note that not all entries will use both flat and transition
		// members: DM, 'only shader LOD' and last LOD don't use the transition member, and the
		// flat member is often not used for intermediate LODs.
		enum
		{
			kInvalidTechnique = 0xFFFF
		};
		struct LodTechniqueIndexes // 4-byte aligned
		{
			U16                 m_flatLodTechniqueIndex;        ///< Technique for 'flat spot' on current LOD.
			U16                 m_transitionLodTechniqueIndex;  ///< Technique for transition from current LOD to next.
		};

		enum MaterialInstanceFlagBits
		{
			kMaterialInstanceShaderLod       = 1 << 0   ///< This material instance uses shader LOD
		};

		// This structure is immediately followed by the material parameter data, if any.
		struct MaterialInstance // 4-byte aligned
		{
			MaterialDescriptor  *m_materialDescriptor;              ///< The material descriptor for this instance.
			LodTechniqueIndexes *m_techniqueTable;                  ///< 2D table to select technique ID per variation, LOD and flat/transition.
			U32                 m_flags;                            ///< Uses the MaterialInstanceFlagBits enum.
			// Parameter data follows here! Offsets to it are relative to the start of the MaterialInstance struct.
		};

		// Performed at load time, this does name string lookups and fixes up the effect,
		// technique and parameter pointers. This could also be performed at tools time instead
		// if late binding is not desired.
		void FixupMaterialDescriptor(MaterialDescriptor *materialDescriptor, Fx::Effect **uniqueEffectTable);

		// Register an application callback for setting environment parameters (usually only done at startup).
		void RegisterEnvironmentParameterSettingCallback(void (*callbackFunctionPtr)(U32, const ParameterBinding *));

		// Performed at runtime, this binds all programs and textures and sets all parameter values.
		// It will call the registered application callback for setting environment parameters.
		void SetupMaterial(const MaterialInstance *matInstance, const MaterialTechnique *matTechnique);
	}
}

#endif // ICE_MATERIAL_H
