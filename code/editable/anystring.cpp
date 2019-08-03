//------------------------------------------------------------------------------------------
//!
//!	\file anystring.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "editable/anystring.h"
#include "objectdatabase/dataobject.h"

START_STD_INTERFACE( AnyString )
	PUBLISH_VAR_AS( m_obString, String )
END_STD_INTERFACE
