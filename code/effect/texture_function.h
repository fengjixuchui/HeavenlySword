//--------------------------------------------------
//!
//!	\file texture_function.h
//!	Objects that collect FunctionCurve_User objects
//! and approximate them into a texture or
//! float array.
//!
//--------------------------------------------------

#ifndef _TEXTURE_FUNCTION_H
#define _TEXTURE_FUNCTION_H

#include "gfx/texture.h"

class FunctionCurve_User;

//--------------------------------------------------
//!
//!	FunctionTableGenerator
//!
//--------------------------------------------------
class FunctionTableGenerator
{
public:
	~FunctionTableGenerator() { Reset(); }
	
	void Reset()
	{
		m_iFunctionCount = 0;
		m_functions.clear();
	}

	// note, this ptr could well be null, as it can be used to 
	// generate sparse float tables (where the user must be clever
	// with function indexes when sampling) or black bits in the texture
	void AddFunction( const FunctionCurve_User* pFunc )
	{
		if (pFunc)
			m_iFunctionCount++;
		m_functions.push_back( pFunc );
	}

	u_int GetTableSize()	const { return m_functions.size(); }
	u_int GetNumFunctions() const { return m_iFunctionCount; }

	Texture::Ptr GenerateTexture(	u_int iFunctionWidth, u_int iResolution,
									bool b8bit, bool bSaveToDisk, const char* pName = 0 );

	void GenerateFunctionTable(	u_int iResolution, float** ppTablePtr );

private:
	u_int	m_iFunctionCount;
	ntstd::List<const FunctionCurve_User*> m_functions;
};

//--------------------------------------------------
//!
//!	FunctionSampler
//! Provides CPU side linear filtering of a function
//!
//--------------------------------------------------
class FunctionSampler
{
public:
	FunctionSampler( float* pTable, u_int iResolution, u_int iNumFunctions ) :
		m_pTable(pTable),
		m_iResolution(iResolution),
		m_iSampleMax(iResolution-1),
		m_iNumFunctions(iNumFunctions) //btw, is perfectly valid to have 0 functions here
	{
		ntAssert(m_pTable);
		ntAssert(m_iResolution > 0);
	}

	inline float Sample_linearFilter( float fFunctionArgument, u_int iFunctionIndex ) const
	{
		ntError( (fFunctionArgument >= 0.0f) || (fFunctionArgument <= 1.0f) );
		ntError( iFunctionIndex < m_iNumFunctions );

		float lerpVal = fFunctionArgument * m_iSampleMax;
		float floored = floor(lerpVal);

		u_int iIndex1 = (u_int)floored;
		u_int iIndex2 = ntstd::Min( iIndex1+1, m_iSampleMax );

		lerpVal -= floored;
		
		ntAssert( iIndex1 <= m_iSampleMax );
		ntAssert( iIndex2 <= m_iSampleMax );

		u_int iBase = (m_iResolution * iFunctionIndex);

		float sample1 = m_pTable[ iBase + iIndex1 ];
		float sample2 = m_pTable[ iBase + iIndex2 ];
			
		return (sample1 * (1.0f - lerpVal)) + (sample2 * lerpVal);
	}

private:
	float*	m_pTable;
	u_int	m_iResolution;
	u_int	m_iSampleMax;
	u_int	m_iNumFunctions;
};

//--------------------------------------------------
//!
//!	FunctionObject
//! Object that wraps up table approximations to
//! functions. Hopefully this should be serialisable
//! so we dont have to generate on startup all the time.
//!
//--------------------------------------------------
class FunctionObject
{
public:
	enum FUNCTION_TABLE_TYPE
	{
		FTT_UNKNOWN,
		FTT_FLOAT_SIMPLE,	// float array simple
		FTT_FLOAT_SPARSE,	// float array sparse
		FTT_TEXTURE,		// texture based (can always be sparse)
	};

	FunctionObject() :
		m_eType(FTT_UNKNOWN),
		m_bRequireGeneration(true),
		m_pFuncTable(0),
		m_pCPUFunctions(0)
	{}

	~FunctionObject()
	{
		Reset( FTT_UNKNOWN );
	}

	void Reset( FUNCTION_TABLE_TYPE eType,
				int iTableResolution = 256 )
	{
		m_generator.Reset();
		m_eType = eType;
		m_iTableResolution = iTableResolution;

		// clean old functions if required
		if (m_pFuncTable)
		{
			NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pFuncTable );
			m_pFuncTable = 0;
		}

		if (m_pCPUFunctions)
		{
			NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pCPUFunctions );
			m_pCPUFunctions = 0;
		}

		m_texFunctions.Reset();
		m_bRequireGeneration = true;
	};

	void AddFunction( const FunctionCurve_User* pFunc )
	{
		if (!pFunc)
		{
			ntError( m_eType == FTT_FLOAT_SPARSE );
		}

		if (m_eType == FTT_FLOAT_SPARSE)
		{
			u_int iIndex = pFunc ? m_generator.GetNumFunctions() : 0xffffffff;
			m_functionIndexTranlator[m_generator.GetTableSize()] = iIndex;
		}

		m_generator.AddFunction( pFunc );
	}

	// make our approximations
	inline void FlushCreation()
	{
		if (m_bRequireGeneration)
			GenerateFunctions();
	}

	// retrive function approximations as a texture
	const Texture::Ptr&	GetFunctionTex() const
	{
		ntAssert(m_eType == FTT_TEXTURE);
		ntAssert(!m_bRequireGeneration);
		return m_texFunctions;
	}

	// retrive function approximations as a table
	// (only trust the user with this if we know we're not a sparse array)
	const FunctionSampler& GetFunctionTable() const
	{
		ntAssert(m_eType == FTT_FLOAT_SIMPLE);
		ntAssert(!m_bRequireGeneration);
		return *m_pCPUFunctions;
	}

	// wrapper around sampling of CPU resources, incase we have a sparse table
	float Sample_linearFilter( float fFunctionArgument, u_int iFunctionIndex ) const
	{
		ntAssert(m_eType != FTT_TEXTURE);
		ntAssert(!m_bRequireGeneration);
		ntAssert(iFunctionIndex < m_generator.GetTableSize());

		if (m_eType == FTT_FLOAT_SPARSE)
		{
			ntstd::Map<u_int,u_int>::const_iterator it = m_functionIndexTranlator.find(iFunctionIndex);
			iFunctionIndex = 0xffffffff;

			if (it!=m_functionIndexTranlator.end())
				iFunctionIndex = it->second;

			if (iFunctionIndex == 0xffffffff)
				return 0.0f;
		}

		return m_pCPUFunctions->Sample_linearFilter( fFunctionArgument, iFunctionIndex );
	}

	FUNCTION_TABLE_TYPE GetType() const { return m_eType; }

private:
	void GenerateFunctions()
	{
		ntAssert( m_bRequireGeneration );
		ntAssert( m_generator.GetTableSize() > 0 );

		if (m_eType == FTT_TEXTURE)
			m_texFunctions = m_generator.GenerateTexture( 2, m_iTableResolution, false, false );
		else
		{
			m_generator.GenerateFunctionTable( m_iTableResolution, &m_pFuncTable );
			m_pCPUFunctions = NT_NEW_CHUNK( Mem::MC_EFFECTS ) FunctionSampler( m_pFuncTable, m_iTableResolution, m_generator.GetNumFunctions() );
		}

		m_bRequireGeneration = false;
	}

	FUNCTION_TABLE_TYPE		m_eType;
	int						m_iTableResolution;
	bool					m_bRequireGeneration;
	FunctionTableGenerator	m_generator;
	float*					m_pFuncTable;		// auto gen'd functions as floats
	Texture::Ptr			m_texFunctions;		// auto gen'd functions as texture
	FunctionSampler*		m_pCPUFunctions;	// CPU function sampler
	ntstd::Map<u_int,u_int>	m_functionIndexTranlator; // used if we're sparse
};

#endif
