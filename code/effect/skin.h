//--------------------------------------------------
//!
//!	\file skin.h
//!	Contains the interface for skin functions.
//!
//--------------------------------------------------

#include "effect/functioncurve.h"
#include "effect/texture_function.h"
#include "effect/effect_resourceman.h"
#include "effect/colour_function.h"

class SkinDef : public EffectResource
{
	HAS_INTERFACE( SkinDef )

	public:
		//
		//	Function texture access.
		//
		const Texture::Ptr& GetFunctionTex		()	const		{ return m_SkinFunctions.GetFunctionTex(); }

	public:
		//
		//	Set all the parameters for the skin shader.
		//
		void				SetSkinParameters	( const FXHandle &effect, const CMatrix &worldToObject )	const;

	public:
		//
		//	Implementation of EffectResource.
		//
		virtual void 		GenerateResources	();
		virtual bool 		ResourcesOutOfDate	() const;

	public:
		//
		//	Post construction function - auto-add ourselves to the effect resource manager.
		//
		void				OnPostConstruct		()				{}

	public:
		//
		//	Ctor, dtor.
		//
		SkinDef	()	{ EffectResourceMan::Get().RegisterResource( *this ); }
		~SkinDef()	{ EffectResourceMan::Get().ReleaseResource( *this ); }

	public:
		//
		//	Exposed members.
		//
		FunctionCurve_User *	m_RednessFunc;
		FunctionCurve_User *	m_TranslucencyFacingFunc;
		FunctionCurve_User *	m_TranslucencyLuminanceFunc;
		FunctionCurve_User *	m_PeachFuzzFunc;
		FunctionCurve_User *	m_DirectSpecularFunc;
		FunctionCurve_User *	m_GlancingSpecularFunc;

		float					m_DiffuseMultiplier;

		CVector					m_RednessColour;
		CVector					m_TranslucencyColour;
		CVector					m_PeachFuzzColour;
		CVector					m_DirectSpecularColour;
		CVector					m_GlanceSpecularColour;

		ColourFunction_Palette *m_RednessTOD;
		ColourFunction_Palette *m_TranslucencyTOD;
		ColourFunction_Palette *m_PeachFuzzTOD;
		ColourFunction_Palette *m_DirectSpecularTOD;
		ColourFunction_Palette *m_GlanceSpecularTOD;

		float					m_DirectSpecularRoughness;
		float					m_GlanceSpecularPower;
		float					m_GlanceLightZDistance;
		float					m_GlanceSpecularMaxDistance;

		bool					m_ShowDiffusePass;
		bool					m_ShowRednessPass;
		bool					m_ShowTranslucencyPass;
		bool					m_ShowPeachFuzzPass;
		bool					m_ShowDirectSpecularPass;
		bool					m_ShowGlanceSpecularPass;

	private:
		//
		//	Aggregated members.
		//
		FunctionObject			m_SkinFunctions;
};