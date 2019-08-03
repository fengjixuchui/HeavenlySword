//--------------------------------------------------------------------------------------------------
/**
	@file		GpFaceShaderArg.h

	@brief		Class definition for the shader effect descriptor used for facial animation

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_FACE_SHADER_ARG_H
#define GP_FACE_SHADER_ARG_H

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			GpFaceShaderArg

	@brief			Descriptor of a shader effect parameter used in a facial anim expression function

	@note			Only floats & single components of float arrays are currently supported
**/
//--------------------------------------------------------------------------------------------------

class GpFaceShaderArg
{
public:
	// Construction & Destruction
	inline GpFaceShaderArg(FwHashedString meshName, FwHashedString paramName, u32 size);
	inline ~GpFaceShaderArg();

	// Access
	inline FwHashedString	GetMeshName() const;
	inline FwHashedString	GetParamName() const;
	inline u32				GetSize() const;

private:
	// Attributes
	FwHashedString	m_meshName;			///< hash name of mesh instance associated with the shader
	FwHashedString	m_paramName;		///< shader effect parameter name hash
	u32				m_size;				///< length of the parameter (in floats)
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceShaderArg constructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceShaderArg::GpFaceShaderArg(FwHashedString meshName, FwHashedString paramName, u32 size)
{
	m_meshName	= meshName;
	m_paramName	= paramName;
	m_size		= size;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			GpFaceShaderArg destructor
**/
//--------------------------------------------------------------------------------------------------

inline GpFaceShaderArg::~GpFaceShaderArg()
{
	// do nothing
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the name hash of the mesh associated with this shader
**/
//--------------------------------------------------------------------------------------------------

inline FwHashedString GpFaceShaderArg::GetMeshName() const
{
	return m_meshName;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the name hash of the shader effect parameter
**/
//--------------------------------------------------------------------------------------------------

inline FwHashedString GpFaceShaderArg::GetParamName() const
{
	return m_paramName;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the size in floats of this effect parameter
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpFaceShaderArg::GetSize() const
{
	return m_size;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_FACE_SHADER_ARG_H








