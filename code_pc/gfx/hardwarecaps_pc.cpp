//--------------------------------------------------
//!
//!	\file hardwarecaps.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/hardwarecaps.h"
#include "gfx/dxerror_pc.h"
#include "gfx/graphicsdevice.h"

//--------------------------------------------------
//!
//!	HardwareCapabilities::HardwareCapabilities
//!
//--------------------------------------------------
HardwareCapabilities::HardwareCapabilities() :
	m_supportsShadowMapHardware(TS_UNKNOWN)
{
	if( FAILED(GetD3DDevice()->GetDeviceCaps( &m_CAPS ) ) )
	{
		user_error_p( false, ("This machine does not have valid D3D, check DirectX or drivers" ) );
	}
}

//--------------------------------------------------
//!
//!	HardwareCapabilities::IsValidRenderTargetFormat
//!
//--------------------------------------------------
bool HardwareCapabilities::IsValidRenderTargetFormat( D3DFORMAT type )
{
	if (m_validRT.find(type) != m_validRT.end())
		return true;
	
	if (m_invalidRT.find(type) != m_invalidRT.end())
		return false;

	if ( GraphicsDevice::Get().m_Platform.IsValidRenderTargetType( type ) )
	{
		m_validRT.insert(type);
		return true;
	}
	else
	{
		m_invalidRT.insert(type);
		return false;
	}
}

//--------------------------------------------------
//!
//!	HardwareCapabilities::IsValidTextureFormat
//!
//--------------------------------------------------
bool HardwareCapabilities::IsValidTextureFormat( D3DFORMAT type )
{
	if (m_validTex.find(type) != m_validTex.end())
		return true;
	
	if (m_invalidTex.find(type) != m_invalidTex.end())
		return false;

	if ( GraphicsDevice::Get().m_Platform.IsValidTextureType( type ) )
	{
		m_validTex.insert(type);
		return true;
	}
	else
	{
		m_invalidTex.insert(type);
		return false;
	}
}

//--------------------------------------------------
//!
//!	HardwareCapabilities::IsValidDepthFormat
//!
//--------------------------------------------------
bool HardwareCapabilities::IsValidDepthFormat( D3DFORMAT type )
{
	if (m_validDF.find(type) != m_validDF.end())
		return true;
	
	if (m_invalidDF.find(type) != m_invalidDF.end())
		return false;

	if ( GraphicsDevice::Get().m_Platform.IsValidDepthFormat( type ) )
	{
		m_validDF.insert(type);
		return true;
	}
	else
	{
		m_invalidDF.insert(type);
		return false;
	}

	ntError_p( 0, ( "Couldn't determine if depth format support is present or not" ) );
	return false;
}

//--------------------------------------------------
//!
//!	HardwareCapabilities::IsValidAlphaBlendRTFormat
//!
//--------------------------------------------------
bool HardwareCapabilities::IsValidAlphaBlendRTFormat( D3DFORMAT type )
{
	if (m_validART.find(type) != m_validART.end())
		return true;
	
	if (m_invalidART.find(type) != m_invalidART.end())
		return false;

	if (IsValidRenderTargetFormat(type))
	{
		if ( GraphicsDevice::Get().m_Platform.IsValidAlphaBlendRTFormat( type ) )
		{
			m_validART.insert(type);
			return true;
		}
		else
		{
			m_invalidART.insert(type);
			return false;
		}
	}
	m_invalidART.insert( type );
	return false;
}

//--------------------------------------------------
//!
//!	HardwareCapabilities::SupportsHardwareShadowMaps
//!
//--------------------------------------------------
bool HardwareCapabilities::SupportsHardwareShadowMaps() const
{
	if( m_supportsShadowMapHardware == TS_UNKNOWN )
	{
		if ( GraphicsDevice::Get().m_Platform.SupportsHardwareShadowMaps() )
			m_supportsShadowMapHardware = TS_TRUE;
		else
			m_supportsShadowMapHardware = TS_FALSE;
	}

	#ifdef _DEBUG_SHADOW
		return false;
	#else	
		return !!m_supportsShadowMapHardware;
	#endif // _DEBUG_SHADOW
}
