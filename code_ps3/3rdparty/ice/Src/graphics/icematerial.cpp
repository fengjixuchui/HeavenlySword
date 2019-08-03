/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */


#include "icematerial.h"
#include "icecg.h"
#include "icefx.h"
#include "iceeffects.h"

using namespace Ice;
using namespace Ice::Graphics;

static void (*g_environmentParameterSettingCallbackFunctionPtr)(U32, const ParameterBinding *) = NULL;

void Ice::Graphics::FixupMaterialDescriptor(MaterialDescriptor *materialDescriptor, Fx::Effect **uniqueEffectTable)
{
	// At load time, m_fxEffect contains an index to a unique table of effects. This needs
	// to be fixed up to a pointer to the effect.
	materialDescriptor->m_fxEffect = uniqueEffectTable[(U32)materialDescriptor->m_fxEffect];
	ICE_ASSERT(materialDescriptor->m_fxEffect);

	for (U32F iTechnique = 0; iTechnique < materialDescriptor->m_techniqueCount; iTechnique++) {
		MaterialTechnique *matTech = &materialDescriptor->m_techniques[iTechnique];

		// At load time, m_fxTechnique contains a pointer to a null-terminated string containing
		// the technique name. This needs to be fixed up to a pointer to the technique.
		const char *techniqueName = (const char *)matTech->m_fxTechnique;
		matTech->m_fxTechnique = Fx::GetNamedTechnique(materialDescriptor->m_fxEffect, techniqueName);
		ICE_ASSERT(matTech->m_fxTechnique);

		// We use the first pass only (in fact, it is an error to have more than one):
		const Fx::Pass *fxPass = Fx::GetFirstPass(matTech->m_fxTechnique);
		ICE_ASSERT(Fx::GetNextPass(fxPass) == NULL);

		// At load time, the parameter pointer in the parameter bindings point to a
		// "v:parameter_name" or "f:parameter_name" string (depending on whether the parameter
		// is a vertex or fragment program parameter). This needs to be fixed up to a pointer
		// to the parameter.
		ParameterBinding *paramBinding = matTech->m_materialParameterBindingTable;
		U32F parameterCount = matTech->m_materialParameterCount;
		for (U32F iParam = 0; iParam < parameterCount; iParam++) {
			const char *extendedParamName = (const char *)paramBinding->m_parameter;
			const char *paramName = extendedParamName + 2;
			bool isVertex = extendedParamName[0] == 'v' || extendedParamName[0] == 'V';
			const Cg::Program *cgProgram = isVertex ? Fx::GetPassVertexProgram(fxPass) :
				Fx::GetPassFragmentProgram(fxPass);
			paramBinding->m_parameter = Cg::GetNamedParameter(cgProgram, paramName);

			// A warning, not an ICE_ASSERT, since parameters often removed during debugging.
			if (paramBinding->m_parameter == NULL) {
				printf("WARNING - FixupMaterialDescriptor: didn't find %s program material parameter %s!\n",
				       isVertex ? "vertex" : "fragment", paramName);
			}

			paramBinding++;
		}
		paramBinding = matTech->m_envParameterBindingTable;
		parameterCount = matTech->m_envParameterCount;
		for (U32F iParam = 0; iParam < parameterCount; iParam++) {
			const char *extendedParamName = (const char *)paramBinding->m_parameter;
			const char *paramName = extendedParamName + 2;
			bool isVertex = extendedParamName[0] == 'v' || extendedParamName[0] == 'V';
			const Cg::Program *cgProgram = isVertex ? Fx::GetPassVertexProgram(fxPass) :
				Fx::GetPassFragmentProgram(fxPass);
			paramBinding->m_parameter = Cg::GetNamedParameter(cgProgram, paramName);

			// A warning, not an ICE_ASSERT, since parameters often removed during debugging.
			if (paramBinding->m_parameter == NULL) {
				printf("WARNING - FixupMaterialDescriptor: didn't find %s program environment parameter %s!\n",
				       isVertex ? "vertex" : "fragment", paramName);
			}

			paramBinding++;
		}
	}
};

void Ice::Graphics::RegisterEnvironmentParameterSettingCallback(void (*callbackFunctionPtr)(U32, const ParameterBinding *))
{
	g_environmentParameterSettingCallbackFunctionPtr = callbackFunctionPtr;
}

static void SetupMaterialInstance(MaterialInstance const *__restrict matInstance, MaterialTechnique const *__restrict matTechnique)
{
	// Set values of all material parameters
	U32F const parameterCount = matTechnique->m_materialParameterCount;
	ParameterBinding const *__restrict paramBinding = matTechnique->m_materialParameterBindingTable;
	for (U32F i = 0; i < parameterCount; ++i, ++paramBinding) 
	{
		const Cg::Parameter *cgParam = paramBinding->m_parameter;
		if (cgParam == NULL) 
			continue;
			
		void const *paramData = (char const *)matInstance + paramBinding->m_valueSource;
		if (Cg::GetParameterResourceClass(cgParam) == Cg::kResourceClassTextureUnit) 
			Cg::SetSamplerParameterValue(cgParam, *(Render::Texture **)paramData);
		else 
			Cg::SetParameterValues(cgParam, (float const *)paramData); // NOTE! These should really be baked in!
	}
}

void Ice::Graphics::SetupMaterial(MaterialInstance const *__restrict matInstance, MaterialTechnique const *__restrict matTechnique)
{
	ICE_ASSERT(matInstance);
	ICE_ASSERT(matTechnique);

	// Note: Switching of textures should not be dependant on technique?
	SetupMaterialInstance(matInstance, matTechnique);

	// Set values of all environment parameters.
	U64 const parameterCount = matTechnique->m_envParameterCount;
	ParameterBinding const *__restrict paramBinding = matTechnique->m_envParameterBindingTable;
	if (g_environmentParameterSettingCallbackFunctionPtr) 
		g_environmentParameterSettingCallbackFunctionPtr(parameterCount, paramBinding);

	// Set all state assignments and bind vertex and fragment programs for first (and only) pass.
	// NOTE - this has to happen AFTER all the parameters are set!
	Fx::SetPassState(Fx::GetFirstPass(matTechnique->m_fxTechnique));
};

